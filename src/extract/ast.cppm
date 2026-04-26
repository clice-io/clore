module;

#include "llvm/Support/Error.h"
#include "llvm/Support/xxhash.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Comment.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Index/USRGeneration.h"
#include "clang/Lex/Lexer.h"

export module extract:ast;

import std;
import :compiler;
import :model;
import support;

export namespace clore::extract {

struct ASTError {
    std::string message;
};

struct ExtractedRelation {
    SymbolID from;
    SymbolID to;
    bool is_call = false;         ///< true = call edge
    bool is_inheritance = false;  ///< true = inheritance edge (from=derived, to=base)
};

struct ASTResult {
    std::vector<SymbolInfo> symbols;
    std::vector<ExtractedRelation> relations;
    std::vector<std::string> dependencies;
};

auto extract_symbols(const CompileEntry& entry) -> std::expected<ASTResult, ASTError>;

}  // namespace clore::extract

// ── implementation ──────────────────────────────────────────────────

namespace clore::extract {

namespace {

auto compute_symbol_id(const clang::Decl* decl) -> SymbolID {
    llvm::SmallString<128> usr;
    if(clang::index::generateUSRForDecl(decl, usr)) {
        return SymbolID{.hash = 0};
    }
    std::uint64_t hash = llvm::xxHash64(usr);
    // Use xxh3 (a different algorithm) so the signature provides independent
    // entropy for collision disambiguation instead of being a truncated copy
    // of the xxHash64 hash.
    std::uint32_t signature = static_cast<std::uint32_t>(llvm::xxh3_64bits(usr));
    return SymbolID{.hash = hash, .signature = signature};
}

auto classify_decl(const clang::NamedDecl* decl) -> SymbolKind {
    if(llvm::isa<clang::NamespaceDecl>(decl))
        return SymbolKind::Namespace;
    if(llvm::isa<clang::CXXRecordDecl>(decl)) {
        auto* record = llvm::cast<clang::CXXRecordDecl>(decl);
        if(record->isClass())
            return SymbolKind::Class;
        if(record->isStruct())
            return SymbolKind::Struct;
        if(record->isUnion())
            return SymbolKind::Union;
        return SymbolKind::Class;
    }
    if(llvm::isa<clang::RecordDecl>(decl))
        return SymbolKind::Struct;
    if(llvm::isa<clang::EnumDecl>(decl))
        return SymbolKind::Enum;
    if(llvm::isa<clang::EnumConstantDecl>(decl))
        return SymbolKind::EnumMember;
    if(llvm::isa<clang::CXXMethodDecl>(decl))
        return SymbolKind::Method;
    if(llvm::isa<clang::FunctionDecl>(decl))
        return SymbolKind::Function;
    if(llvm::isa<clang::VarDecl>(decl))
        return SymbolKind::Variable;
    if(llvm::isa<clang::FieldDecl>(decl))
        return SymbolKind::Field;
    if(llvm::isa<clang::TypeAliasDecl>(decl) || llvm::isa<clang::TypedefDecl>(decl))
        return SymbolKind::TypeAlias;
    if(llvm::isa<clang::ConceptDecl>(decl))
        return SymbolKind::Concept;
    if(llvm::isa<clang::TemplateDecl>(decl))
        return SymbolKind::Template;
    return SymbolKind::Unknown;
}

auto get_access_string(clang::AccessSpecifier access) -> std::string {
    switch(access) {
        case clang::AS_public: return "public";
        case clang::AS_protected: return "protected";
        case clang::AS_private: return "private";
        default: return "";
    }
}

auto print_template_parameters(const clang::TemplateParameterList* params,
                               const clang::PrintingPolicy& policy) -> std::string {
    if(params == nullptr)
        return {};

    std::string param_str;
    llvm::raw_string_ostream os(param_str);
    os << "<";
    bool first = true;
    for(auto* param: *params) {
        if(!first)
            os << ", ";
        first = false;
        param->print(os, policy);
    }
    os << ">";
    return param_str;
}

auto apply_described_template_info(const clang::NamedDecl* decl,
                                   const clang::PrintingPolicy& policy,
                                   SymbolInfo& info) -> void {
    const clang::TemplateParameterList* params = nullptr;

    if(auto* concept_decl = llvm::dyn_cast<clang::ConceptDecl>(decl)) {
        params = concept_decl->getTemplateParameters();
    } else if(auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl)) {
        if(auto* tmpl = record->getDescribedClassTemplate()) {
            params = tmpl->getTemplateParameters();
        }
    } else if(auto* func = llvm::dyn_cast<clang::FunctionDecl>(decl)) {
        if(auto* tmpl = func->getDescribedFunctionTemplate()) {
            params = tmpl->getTemplateParameters();
        }
    } else if(auto* type_alias = llvm::dyn_cast<clang::TypeAliasDecl>(decl)) {
        if(auto* tmpl = type_alias->getDescribedAliasTemplate()) {
            params = tmpl->getTemplateParameters();
        }
    } else if(auto* var = llvm::dyn_cast<clang::VarDecl>(decl)) {
        if(auto* tmpl = var->getDescribedVarTemplate()) {
            params = tmpl->getTemplateParameters();
        }
    }

    if(params != nullptr) {
        info.is_template = true;
        info.template_params = print_template_parameters(params, policy);
    }
}

auto build_function_signature(const clang::FunctionDecl* func, const clang::PrintingPolicy& policy)
    -> std::string {
    if(llvm::isa<clang::CXXConstructorDecl>(func) || llvm::isa<clang::CXXDestructorDecl>(func)) {
        std::string result = func->getQualifiedNameAsString();
        result += "(";
        bool first = true;
        for(auto* param: func->parameters()) {
            if(!first) {
                result += ", ";
            }
            first = false;
            std::string param_str;
            llvm::raw_string_ostream param_os(param_str);
            param->print(param_os, policy);
            result += param_str;
        }
        result += ")";
        return result;
    }
    return func->getType().getAsString(policy);
}

auto get_doc_comment(const clang::ASTContext& ctx, const clang::Decl* decl) -> std::string {
    auto* comment = ctx.getRawCommentForDeclNoCache(decl);
    if(!comment)
        return "";
    return clore::support::ensure_utf8(comment->getRawText(ctx.getSourceManager()).str());
}

auto should_extract_named_decl(const clang::ASTContext& context, const clang::NamedDecl* decl)
    -> bool {
    if(decl == nullptr || decl->isImplicit()) {
        return false;
    }

    auto loc = decl->getLocation();
    if(loc.isInvalid()) {
        return false;
    }

    if(context.getSourceManager().isInSystemHeader(loc)) {
        return false;
    }

    if(classify_decl(decl) == SymbolKind::Unknown) {
        return false;
    }

    if(decl->getDeclName().isEmpty()) {
        return false;
    }

    if(llvm::isa<clang::TemplateDecl>(decl) && !llvm::isa<clang::ConceptDecl>(decl)) {
        return false;
    }

    if(llvm::isa<clang::ClassTemplateSpecializationDecl>(decl)) {
        return false;
    }

    return true;
}

struct SourceSnippetBounds {
    std::uint32_t offset = 0;
    std::uint32_t length = 0;
    std::uint64_t file_size = 0;
    std::uint64_t content_hash = 0;
};

constexpr std::uint64_t kSourceSnippetHashOffsetBasis = 14695981039346656037ULL;
constexpr std::uint64_t kSourceSnippetHashPrime = 1099511628211ULL;

auto hash_source_snippet_bytes(std::string_view bytes) -> std::uint64_t {
    auto hash = kSourceSnippetHashOffsetBasis;
    for(auto ch: bytes) {
        hash ^= static_cast<std::uint64_t>(static_cast<unsigned char>(ch));
        hash *= kSourceSnippetHashPrime;
    }
    return hash;
}

auto get_source_snippet_bounds(const clang::ASTContext& ctx, const clang::Decl* decl)
    -> SourceSnippetBounds {
    auto& sm = ctx.getSourceManager();
    auto range = decl->getSourceRange();
    if(range.isInvalid())
        return {};

    auto begin_loc = sm.getSpellingLoc(range.getBegin());
    auto end_token_loc = sm.getSpellingLoc(range.getEnd());
    if(begin_loc.isInvalid() || end_token_loc.isInvalid()) {
        return {};
    }

    auto file_id = sm.getFileID(begin_loc);
    if(file_id.isInvalid() || file_id != sm.getFileID(end_token_loc)) {
        return {};
    }

    auto begin_offset = sm.getFileOffset(begin_loc);

    // getSourceRange().getEnd() points to the start of the last token.
    // Advance past the last token so we capture closing braces, semicolons, etc.
    auto end_loc = clang::Lexer::getLocForEndOfToken(end_token_loc, 0, sm, ctx.getLangOpts());
    if(end_loc.isInvalid() || file_id != sm.getFileID(sm.getSpellingLoc(end_loc))) {
        return {};
    }
    auto end_offset = sm.getFileOffset(end_loc);

    if(end_offset <= begin_offset) {
        end_offset = sm.getFileOffset(end_token_loc);
    }

    auto buffer = sm.getBufferDataOrNone(file_id);
    if(!buffer.has_value() || buffer->empty())
        return {};

    if(begin_offset >= buffer->size())
        return {};
    if(end_offset > buffer->size()) {
        end_offset = buffer->size();
    }

    auto length = end_offset - begin_offset;
    if(length > std::numeric_limits<std::uint32_t>::max()) {
        length = std::numeric_limits<std::uint32_t>::max();
    }

    return SourceSnippetBounds{
        .offset = static_cast<std::uint32_t>(begin_offset),
        .length = static_cast<std::uint32_t>(length),
        .file_size = static_cast<std::uint64_t>(buffer->size()),
        .content_hash =
            hash_source_snippet_bytes(std::string_view(buffer->data() + begin_offset, length)),
    };
}

auto make_source_location(const clang::SourceManager& sm, clang::SourceLocation loc)
    -> SourceLocation {
    if(loc.isInvalid())
        return {};
    auto presumed = sm.getPresumedLoc(loc);
    if(presumed.isInvalid())
        return {};
    return SourceLocation{
        .file = presumed.getFilename(),
        .line = presumed.getLine(),
        .column = presumed.getColumn(),
    };
}

struct LexicalContextInfo {
    std::string enclosing_namespace;
    std::string parent_name;
    SymbolKind parent_kind = SymbolKind::Unknown;
    std::optional<SymbolID> parent_id;
};

auto join_qualified_segments(const std::vector<std::string>& segments) -> std::string {
    if(segments.empty())
        return {};

    std::string joined;
    for(std::size_t index = 0; index < segments.size(); ++index) {
        if(index > 0) {
            joined += "::";
        }
        joined += segments[index];
    }
    return joined;
}

auto describe_lexical_context(const clang::DeclContext* decl_context) -> LexicalContextInfo {
    LexicalContextInfo info;
    std::vector<std::string> namespace_segments;

    for(auto* current = decl_context; current != nullptr && !current->isTranslationUnit();
        current = current->getParent()) {
        if(auto* namespace_decl = llvm::dyn_cast<clang::NamespaceDecl>(current)) {
            if(info.parent_name.empty()) {
                info.parent_name = namespace_decl->getQualifiedNameAsString();
                info.parent_kind = SymbolKind::Namespace;

                auto parent_id = compute_symbol_id(namespace_decl);
                if(parent_id.is_valid()) {
                    info.parent_id = parent_id;
                }
            }

            auto namespace_name = namespace_decl->getNameAsString();
            if(namespace_name.empty()) {
                namespace_name = "(anonymous namespace)";
            }
            namespace_segments.push_back(std::move(namespace_name));
            continue;
        }

        if(!info.parent_name.empty()) {
            continue;
        }

        auto* named = llvm::dyn_cast<clang::NamedDecl>(current);
        if(named == nullptr || named->getDeclName().isEmpty()) {
            continue;
        }

        auto parent_kind = classify_decl(named);
        if(parent_kind == SymbolKind::Namespace) {
            continue;
        }

        info.parent_name = named->getQualifiedNameAsString();
        info.parent_kind = parent_kind;

        auto parent_id = compute_symbol_id(named);
        if(parent_id.is_valid()) {
            info.parent_id = parent_id;
        }
    }

    std::reverse(namespace_segments.begin(), namespace_segments.end());
    info.enclosing_namespace = join_qualified_segments(namespace_segments);
    return info;
}

auto collect_dependency_files(const clang::SourceManager& source_manager)
    -> std::vector<std::string> {
    std::vector<std::string> files;
    std::unordered_set<std::string> seen;

    for(auto it = source_manager.fileinfo_begin(); it != source_manager.fileinfo_end(); ++it) {
        auto file_ref = it->first;
        auto path = file_ref.getFileEntry().tryGetRealPathName().str();
        if(path.empty()) {
            path = file_ref.getName().str();
        }
        if(path.empty()) {
            continue;
        }

        // Skip system headers: they bloat the dependency snapshot and
        // trigger unnecessary cache invalidation on system updates.
        auto file_id = source_manager.translateFile(&file_ref.getFileEntry());
        if(file_id.isValid()) {
            auto loc = source_manager.getLocForStartOfFile(file_id);
            if(source_manager.isInSystemHeader(loc)) {
                continue;
            }
        }

        auto normalized = std::filesystem::path(path).lexically_normal().generic_string();
        if(seen.insert(normalized).second) {
            files.push_back(std::move(normalized));
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

enum class RelationKind : std::uint8_t { Call, Reference, Inheritance };

struct RelationEdge {
    SymbolID from;
    SymbolID to;
    RelationKind kind;
};

auto edge_hash(SymbolID from, SymbolID to, RelationKind kind) -> std::uint64_t {
    auto mix = [](std::uint64_t& hash, auto value) {
        hash ^= std::hash<std::remove_cvref_t<decltype(value)>>{}(value) + 0x9e3779b97f4a7c15ULL +
                (hash << 6) + (hash >> 2);
    };

    std::uint64_t h = std::hash<std::uint64_t>{}(from.hash);
    mix(h, from.signature);
    mix(h, to.hash);
    mix(h, to.signature);
    mix(h, static_cast<std::uint8_t>(kind));
    return h;
}

class SymbolExtractorVisitor : public clang::RecursiveASTVisitor<SymbolExtractorVisitor> {
public:
    clang::ASTContext& context;
    std::vector<SymbolInfo>& symbols;
    std::vector<RelationEdge>& relations;
    std::string main_file;

    std::vector<SymbolID> enclosing_stack;
    std::unordered_set<std::uint64_t> seen_edges;

    SymbolExtractorVisitor(clang::ASTContext& ctx,
                           std::vector<SymbolInfo>& syms,
                           std::vector<RelationEdge>& rels,
                           std::string main_file) :
        context(ctx), symbols(syms), relations(rels), main_file(std::move(main_file)) {}

    bool shouldVisitImplicitCode() const {
        return false;
    }

    bool shouldVisitTemplateInstantiations() const {
        return false;
    }

    bool VisitNamedDecl(clang::NamedDecl* decl) {
        auto& sm = context.getSourceManager();
        if(!should_extract_named_decl(context, decl)) {
            return true;
        }

        auto kind = classify_decl(decl);
        auto id = compute_symbol_id(decl);
        if(!id.is_valid())
            return true;

        SymbolInfo info;
        info.id = id;
        info.kind = kind;
        info.name = decl->getNameAsString();
        info.qualified_name = decl->getQualifiedNameAsString();
        auto lexical_context = describe_lexical_context(decl->getDeclContext());
        info.enclosing_namespace = std::move(lexical_context.enclosing_namespace);
        info.declaration_location = make_source_location(sm, decl->getLocation());

        if(auto* func = llvm::dyn_cast<clang::FunctionDecl>(decl)) {
            if(func->isThisDeclarationADefinition()) {
                info.definition_location = make_source_location(sm, func->getLocation());
            }
            info.signature = build_function_signature(func, context.getPrintingPolicy());
            if(info.signature.empty()) {
                info.signature = info.qualified_name;
            }
        } else if(auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl)) {
            if(record->isThisDeclarationADefinition()) {
                info.definition_location = make_source_location(sm, record->getLocation());
                for(auto& base: record->bases()) {
                    auto* base_type = base.getType()->getAsCXXRecordDecl();
                    if(base_type) {
                        auto base_id = compute_symbol_id(base_type);
                        if(base_id.is_valid()) {
                            info.bases.push_back(base_id);
                            auto h = edge_hash(id, base_id, RelationKind::Inheritance);
                            if(seen_edges.insert(h).second) {
                                relations.push_back(RelationEdge{
                                    .from = id,
                                    .to = base_id,
                                    .kind = RelationKind::Inheritance,
                                });
                            }
                        }
                    }
                }
            }
        } else if(auto* tag = llvm::dyn_cast<clang::TagDecl>(decl)) {
            if(tag->isThisDeclarationADefinition()) {
                info.definition_location = make_source_location(sm, tag->getLocation());
            }
        }

        info.access = get_access_string(decl->getAccess());
        info.doc_comment = get_doc_comment(context, decl);
        {
            auto bounds = get_source_snippet_bounds(context, decl);
            info.source_snippet_offset = bounds.offset;
            info.source_snippet_length = bounds.length;
            info.source_snippet_file_size = bounds.file_size;
            info.source_snippet_hash = bounds.content_hash;
            // Do not eagerly copy the full text into memory; it will be resolved
            // on demand via resolve_source_snippet() during evidence building.
        }
        apply_described_template_info(decl, context.getPrintingPolicy(), info);

        info.lexical_parent_name = std::move(lexical_context.parent_name);
        info.lexical_parent_kind = lexical_context.parent_kind;
        if(lexical_context.parent_id.has_value()) {
            info.parent = *lexical_context.parent_id;
        }

        symbols.push_back(std::move(info));
        return true;
    }

    bool TraverseFunctionDecl(clang::FunctionDecl* decl) {
        auto id = should_extract_named_decl(context, decl) ? compute_symbol_id(decl) : SymbolID{};
        if(id.is_valid()) {
            enclosing_stack.push_back(id);
        }
        bool result =
            clang::RecursiveASTVisitor<SymbolExtractorVisitor>::TraverseFunctionDecl(decl);
        if(id.is_valid()) {
            enclosing_stack.pop_back();
        }
        return result;
    }

    bool TraverseCXXMethodDecl(clang::CXXMethodDecl* decl) {
        auto id = should_extract_named_decl(context, decl) ? compute_symbol_id(decl) : SymbolID{};
        if(id.is_valid()) {
            enclosing_stack.push_back(id);
        }
        bool result =
            clang::RecursiveASTVisitor<SymbolExtractorVisitor>::TraverseCXXMethodDecl(decl);
        if(id.is_valid()) {
            enclosing_stack.pop_back();
        }
        return result;
    }

    bool can_record_relation_from(clang::SourceLocation loc) const {
        if(enclosing_stack.empty()) {
            return false;
        }

        auto& sm = context.getSourceManager();
        if(loc.isInvalid() || sm.isInSystemHeader(loc)) {
            return false;
        }

        return true;
    }

    bool try_record_relation(clang::SourceLocation loc, SymbolID to, RelationKind kind) {
        if(!can_record_relation_from(loc) || !to.is_valid()) {
            return true;
        }

        auto from = enclosing_stack.back();
        if(from == to) {
            return true;
        }

        auto h = edge_hash(from, to, kind);
        if(seen_edges.insert(h).second) {
            relations.push_back(RelationEdge{.from = from, .to = to, .kind = kind});
        }
        return true;
    }

    bool VisitCallExpr(clang::CallExpr* expr) {
        auto* callee = expr->getDirectCallee();
        if(!callee || !should_extract_named_decl(context, callee))
            return true;

        auto callee_id = compute_symbol_id(callee);
        return try_record_relation(expr->getBeginLoc(), callee_id, RelationKind::Call);
    }

    bool VisitDeclRefExpr(clang::DeclRefExpr* expr) {
        auto* referenced = llvm::dyn_cast<clang::NamedDecl>(expr->getDecl());
        if(!referenced || !should_extract_named_decl(context, referenced))
            return true;

        if(llvm::isa<clang::FunctionDecl>(referenced))
            return true;

        auto ref_id = compute_symbol_id(referenced);
        return try_record_relation(expr->getBeginLoc(), ref_id, RelationKind::Reference);
    }

    bool VisitMemberExpr(clang::MemberExpr* expr) {
        auto* member = expr->getMemberDecl();
        if(!member || !should_extract_named_decl(context, member))
            return true;

        if(llvm::isa<clang::FunctionDecl>(member))
            return true;

        auto member_id = compute_symbol_id(member);
        return try_record_relation(expr->getBeginLoc(), member_id, RelationKind::Reference);
    }
};

class SymbolExtractorConsumer : public clang::ASTConsumer {
public:
    std::vector<SymbolInfo>& symbols;
    std::vector<RelationEdge>& relations;
    std::string main_file;

    SymbolExtractorConsumer(std::vector<SymbolInfo>& syms,
                            std::vector<RelationEdge>& rels,
                            std::string main_file) :
        symbols(syms), relations(rels), main_file(std::move(main_file)) {}

    void HandleTranslationUnit(clang::ASTContext& context) override {
        SymbolExtractorVisitor visitor(context, symbols, relations, main_file);
        visitor.TraverseDecl(context.getTranslationUnitDecl());
    }
};

class SymbolExtractorAction : public clang::ASTFrontendAction {
public:
    std::vector<SymbolInfo>& symbols;
    std::vector<RelationEdge>& relations;

    SymbolExtractorAction(std::vector<SymbolInfo>& syms, std::vector<RelationEdge>& rels) :
        symbols(syms), relations(rels) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance&,
                                                          llvm::StringRef file) override {
        return std::make_unique<SymbolExtractorConsumer>(symbols, relations, file.str());
    }
};

}  // namespace

auto extract_symbols(const CompileEntry& entry) -> std::expected<ASTResult, ASTError> {
    if(entry.arguments.empty()) {
        return std::unexpected(
            ASTError{.message = std::format("empty argument list for file: {}", entry.file)});
    }

    ASTResult result;
    std::vector<RelationEdge> raw_relations;

    auto instance = create_compiler_instance(entry);
    if(!instance) {
        return std::unexpected(ASTError{
            .message = std::format("failed to create compiler instance for file: {}", entry.file)});
    }

    // Force extraction-only mode so compile commands that normally emit
    // objects/PCMs are treated as pure semantic analysis.
    auto& frontend_opts = instance->getInvocation().getFrontendOpts();
    frontend_opts.ProgramAction = clang::frontend::ParseSyntaxOnly;
    frontend_opts.OutputFile.clear();
    frontend_opts.ModuleOutputPath.clear();

    SymbolExtractorAction action(result.symbols, raw_relations);
    if(!action.BeginSourceFile(*instance, instance->getFrontendOpts().Inputs[0])) {
        return std::unexpected(ASTError{
            .message = std::format("failed to begin AST extraction for file: {}", entry.file)});
    }

    if(auto error = action.Execute()) {
        llvm::consumeError(std::move(error));
        action.EndSourceFile();
        return std::unexpected(
            ASTError{.message = std::format("AST extraction failed for file: {}", entry.file)});
    }

    action.EndSourceFile();
    result.dependencies = collect_dependency_files(instance->getSourceManager());

    result.relations.reserve(raw_relations.size());
    for(auto& edge: raw_relations) {
        result.relations.push_back(ExtractedRelation{
            .from = edge.from,
            .to = edge.to,
            .is_call = (edge.kind == RelationKind::Call),
            .is_inheritance = (edge.kind == RelationKind::Inheritance),
        });
    }

    return result;
}

}  // namespace clore::extract
