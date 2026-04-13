module;

#include <algorithm>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

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
#include "llvm/Support/Error.h"

export module extract:ast;

import :compdb;
import :model;
import :symbol;
import :tooling;
import support;

export namespace clore::extract {

struct ASTError {
    std::string message;
};

struct ExtractedRelation {
    SymbolID from;
    SymbolID to;
    bool is_call;  ///< true = call edge, false = reference edge
};

struct ASTResult {
    std::vector<SymbolInfo> symbols;
    std::vector<ExtractedRelation> relations;
    std::vector<std::string> dependencies;
};

auto extract_symbols(const CompileEntry& entry, std::uint32_t max_snippet_bytes)
    -> std::expected<ASTResult, ASTError>;

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
    return SymbolID{.hash = hash};
}

auto classify_decl(const clang::NamedDecl* decl) -> SymbolKind {
    if(llvm::isa<clang::NamespaceDecl>(decl)) return SymbolKind::Namespace;
    if(llvm::isa<clang::CXXRecordDecl>(decl)) {
        auto* record = llvm::cast<clang::CXXRecordDecl>(decl);
        if(record->isClass()) return SymbolKind::Class;
        if(record->isStruct()) return SymbolKind::Struct;
        if(record->isUnion()) return SymbolKind::Union;
        return SymbolKind::Class;
    }
    if(llvm::isa<clang::RecordDecl>(decl)) return SymbolKind::Struct;
    if(llvm::isa<clang::EnumDecl>(decl)) return SymbolKind::Enum;
    if(llvm::isa<clang::EnumConstantDecl>(decl)) return SymbolKind::EnumMember;
    if(llvm::isa<clang::CXXMethodDecl>(decl)) return SymbolKind::Method;
    if(llvm::isa<clang::FunctionDecl>(decl)) return SymbolKind::Function;
    if(llvm::isa<clang::VarDecl>(decl)) return SymbolKind::Variable;
    if(llvm::isa<clang::FieldDecl>(decl)) return SymbolKind::Field;
    if(llvm::isa<clang::TypeAliasDecl>(decl) || llvm::isa<clang::TypedefDecl>(decl))
        return SymbolKind::TypeAlias;
    if(llvm::isa<clang::ConceptDecl>(decl)) return SymbolKind::Concept;
    if(llvm::isa<clang::TemplateDecl>(decl)) return SymbolKind::Template;
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
    if(params == nullptr) return {};

    std::string param_str;
    llvm::raw_string_ostream os(param_str);
    os << "<";
    bool first = true;
    for(auto* param : *params) {
        if(!first) os << ", ";
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

auto get_doc_comment(const clang::ASTContext& ctx, const clang::Decl* decl) -> std::string {
    auto* comment = ctx.getRawCommentForDeclNoCache(decl);
    if(!comment) return "";
    return clore::support::ensure_utf8(
        comment->getRawText(ctx.getSourceManager()).str());
}

auto get_source_snippet(const clang::ASTContext& ctx, const clang::Decl* decl,
                        std::uint32_t max_bytes) -> std::string {
    auto& sm = ctx.getSourceManager();
    auto range = decl->getSourceRange();
    if(range.isInvalid()) return "";

    auto begin = sm.getPresumedLoc(range.getBegin());
    auto end = sm.getPresumedLoc(range.getEnd());
    if(begin.isInvalid() || end.isInvalid()) return "";

    auto begin_offset = sm.getFileOffset(range.getBegin());

    // getSourceRange().getEnd() points to the start of the last token.
    // Advance past the last token so we capture closing braces, semicolons, etc.
    auto end_loc = clang::Lexer::getLocForEndOfToken(
        range.getEnd(), 0, sm, ctx.getLangOpts());
    auto end_offset = sm.getFileOffset(end_loc);

    if(end_offset <= begin_offset) {
        // Fallback: use original end
        end_offset = sm.getFileOffset(range.getEnd());
    }

    constexpr std::size_t utf8_slack_bytes = 4;
    const auto max_excerpt_bytes = static_cast<std::size_t>(max_bytes) + utf8_slack_bytes;
    if(static_cast<std::size_t>(end_offset - begin_offset) > max_excerpt_bytes) {
        end_offset = begin_offset + static_cast<decltype(end_offset)>(max_excerpt_bytes);
    }

    auto file_id = sm.getFileID(range.getBegin());
    auto buffer = sm.getBufferData(file_id);
    if(buffer.empty()) return "";

    if(begin_offset >= buffer.size()) return "";
    if(end_offset > buffer.size()) {
        end_offset = buffer.size();
    }

    std::string result(buffer.substr(begin_offset, end_offset - begin_offset));

    // Normalize \r\n to \n to avoid double-spaced lines in markdown
    std::string normalized;
    normalized.reserve(result.size());
    for(std::size_t i = 0; i < result.size(); ++i) {
        if(result[i] == '\r' && i + 1 < result.size() && result[i + 1] == '\n') {
            continue;  // skip \r before \n
        }
        normalized += result[i];
    }

    return clore::support::truncate_utf8(normalized, max_bytes);
}

auto make_source_location(const clang::SourceManager& sm, clang::SourceLocation loc)
    -> SourceLocation {
    if(loc.isInvalid()) return {};
    auto presumed = sm.getPresumedLoc(loc);
    if(presumed.isInvalid()) return {};
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
    if(segments.empty()) return {};

    std::string joined;
    for(std::size_t index = 0; index < segments.size(); ++index) {
        if(index > 0) {
            joined += "::";
        }
        joined += segments[index];
    }
    return joined;
}

auto describe_lexical_context(const clang::DeclContext* decl_context)
    -> LexicalContextInfo {
    LexicalContextInfo info;
    std::vector<std::string> namespace_segments;

    for(auto* current = decl_context;
        current != nullptr && !current->isTranslationUnit();
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

        auto normalized = std::filesystem::path(path).lexically_normal().generic_string();
        if(seen.insert(normalized).second) {
            files.push_back(std::move(normalized));
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

enum class RelationKind : std::uint8_t { Call, Reference };

struct RelationEdge {
    SymbolID from;
    SymbolID to;
    RelationKind kind;
};

auto edge_hash(SymbolID from, SymbolID to, RelationKind kind) -> std::uint64_t {
    auto h = std::hash<std::uint64_t>{}(from.hash);
    h ^= std::hash<std::uint64_t>{}(to.hash) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    h ^= std::hash<std::uint8_t>{}(static_cast<std::uint8_t>(kind)) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    return h;
}

class SymbolExtractorVisitor : public clang::RecursiveASTVisitor<SymbolExtractorVisitor> {
public:
    clang::ASTContext& context;
    std::vector<SymbolInfo>& symbols;
    std::vector<RelationEdge>& relations;
    std::string main_file;
    std::uint32_t max_snippet_bytes;

    std::vector<SymbolID> enclosing_stack;
    std::unordered_set<std::uint64_t> seen_edges;

    SymbolExtractorVisitor(clang::ASTContext& ctx, std::vector<SymbolInfo>& syms,
                           std::vector<RelationEdge>& rels,
                           std::string main_file, std::uint32_t max_bytes)
        : context(ctx), symbols(syms), relations(rels),
          main_file(std::move(main_file)), max_snippet_bytes(max_bytes) {}

    bool shouldVisitImplicitCode() const { return false; }
    bool shouldVisitTemplateInstantiations() const { return false; }

    bool VisitNamedDecl(clang::NamedDecl* decl) {
        if(decl->isImplicit()) return true;

        auto& sm = context.getSourceManager();
        auto loc = decl->getLocation();
        if(loc.isInvalid()) return true;

        if(sm.isInSystemHeader(loc)) return true;

        auto kind = classify_decl(decl);
        if(kind == SymbolKind::Unknown) return true;

        if(decl->getDeclName().isEmpty()) return true;

        if(llvm::isa<clang::TemplateDecl>(decl) &&
           !llvm::isa<clang::ConceptDecl>(decl)) {
            return true;
        }
        if(llvm::isa<clang::ClassTemplateSpecializationDecl>(decl)) return true;

        auto id = compute_symbol_id(decl);
        if(!id.is_valid()) return true;

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
            std::string sig;
            llvm::raw_string_ostream os(sig);
            func->print(os, context.getPrintingPolicy());
            info.signature = std::move(sig);
        } else if(auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl)) {
            if(record->isThisDeclarationADefinition()) {
                info.definition_location = make_source_location(sm, record->getLocation());
                for(auto& base : record->bases()) {
                    auto* base_type = base.getType()->getAsCXXRecordDecl();
                    if(base_type) {
                        auto base_id = compute_symbol_id(base_type);
                        if(base_id.is_valid()) {
                            info.bases.push_back(base_id);
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
        info.source_snippet = get_source_snippet(context, decl, max_snippet_bytes);
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
        auto id = compute_symbol_id(decl);
        if(id.is_valid()) enclosing_stack.push_back(id);
        bool result = clang::RecursiveASTVisitor<SymbolExtractorVisitor>::TraverseFunctionDecl(decl);
        if(id.is_valid()) enclosing_stack.pop_back();
        return result;
    }

    bool TraverseCXXMethodDecl(clang::CXXMethodDecl* decl) {
        auto id = compute_symbol_id(decl);
        if(id.is_valid()) enclosing_stack.push_back(id);
        bool result = clang::RecursiveASTVisitor<SymbolExtractorVisitor>::TraverseCXXMethodDecl(decl);
        if(id.is_valid()) enclosing_stack.pop_back();
        return result;
    }

    bool VisitCallExpr(clang::CallExpr* expr) {
        if(enclosing_stack.empty()) return true;

        auto& sm = context.getSourceManager();
        if(sm.isInSystemHeader(expr->getBeginLoc())) return true;

        auto* callee = expr->getDirectCallee();
        if(!callee) return true;

        auto callee_id = compute_symbol_id(callee);
        if(!callee_id.is_valid()) return true;

        auto from = enclosing_stack.back();
        if(from == callee_id) return true;

        auto h = edge_hash(from, callee_id, RelationKind::Call);
        if(seen_edges.insert(h).second) {
            relations.push_back(RelationEdge{
                .from = from, .to = callee_id, .kind = RelationKind::Call});
        }
        return true;
    }

    bool VisitDeclRefExpr(clang::DeclRefExpr* expr) {
        if(enclosing_stack.empty()) return true;

        auto& sm = context.getSourceManager();
        if(sm.isInSystemHeader(expr->getBeginLoc())) return true;

        auto* referenced = llvm::dyn_cast<clang::NamedDecl>(expr->getDecl());
        if(!referenced) return true;

        if(llvm::isa<clang::FunctionDecl>(referenced)) return true;

        auto ref_id = compute_symbol_id(referenced);
        if(!ref_id.is_valid()) return true;

        auto from = enclosing_stack.back();
        if(from == ref_id) return true;

        auto h = edge_hash(from, ref_id, RelationKind::Reference);
        if(seen_edges.insert(h).second) {
            relations.push_back(RelationEdge{
                .from = from, .to = ref_id, .kind = RelationKind::Reference});
        }
        return true;
    }

    bool VisitMemberExpr(clang::MemberExpr* expr) {
        if(enclosing_stack.empty()) return true;

        auto& sm = context.getSourceManager();
        if(sm.isInSystemHeader(expr->getBeginLoc())) return true;

        auto* member = expr->getMemberDecl();
        if(!member) return true;

        if(llvm::isa<clang::FunctionDecl>(member)) return true;

        auto member_id = compute_symbol_id(member);
        if(!member_id.is_valid()) return true;

        auto from = enclosing_stack.back();
        if(from == member_id) return true;

        auto h = edge_hash(from, member_id, RelationKind::Reference);
        if(seen_edges.insert(h).second) {
            relations.push_back(RelationEdge{
                .from = from, .to = member_id, .kind = RelationKind::Reference});
        }
        return true;
    }
};

class SymbolExtractorConsumer : public clang::ASTConsumer {
public:
    std::vector<SymbolInfo>& symbols;
    std::vector<RelationEdge>& relations;
    std::string main_file;
    std::uint32_t max_snippet_bytes;

    SymbolExtractorConsumer(std::vector<SymbolInfo>& syms, std::vector<RelationEdge>& rels,
                            std::string main_file, std::uint32_t max_bytes)
        : symbols(syms), relations(rels), main_file(std::move(main_file)),
          max_snippet_bytes(max_bytes) {}

    void HandleTranslationUnit(clang::ASTContext& context) override {
        SymbolExtractorVisitor visitor(context, symbols, relations, main_file, max_snippet_bytes);
        visitor.TraverseDecl(context.getTranslationUnitDecl());
    }
};

class SymbolExtractorAction : public clang::ASTFrontendAction {
public:
    std::vector<SymbolInfo>& symbols;
    std::vector<RelationEdge>& relations;
    std::uint32_t max_snippet_bytes;

    SymbolExtractorAction(std::vector<SymbolInfo>& syms, std::vector<RelationEdge>& rels,
                          std::uint32_t max_bytes)
        : symbols(syms), relations(rels), max_snippet_bytes(max_bytes) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& ci,
                                                          llvm::StringRef file) override {
        return std::make_unique<SymbolExtractorConsumer>(symbols, relations, file.str(),
                                                         max_snippet_bytes);
    }
};

}  // namespace

auto extract_symbols(const CompileEntry& entry, std::uint32_t max_snippet_bytes)
    -> std::expected<ASTResult, ASTError> {
    if(entry.arguments.empty()) {
        return std::unexpected(ASTError{
            .message = std::format("empty argument list for file: {}", entry.file)});
    }

    ASTResult result;
    std::vector<RelationEdge> raw_relations;

    auto instance = create_compiler_instance(entry);
    if(!instance) {
        return std::unexpected(ASTError{
            .message = std::format("failed to create compiler instance for file: {}",
                                   entry.file)});
    }

    // Force extraction-only mode so compile commands that normally emit objects/PCMs
    // are treated as pure semantic analysis.
    auto& frontend_opts = instance->getInvocation().getFrontendOpts();
    frontend_opts.ProgramAction = clang::frontend::ParseSyntaxOnly;
    frontend_opts.OutputFile.clear();

    SymbolExtractorAction action(result.symbols, raw_relations, max_snippet_bytes);
    if(!action.BeginSourceFile(*instance, instance->getFrontendOpts().Inputs[0])) {
        return std::unexpected(ASTError{
            .message = std::format("failed to begin AST extraction for file: {}",
                                   entry.file)});
    }

    if(auto error = action.Execute()) {
        llvm::consumeError(std::move(error));
        action.EndSourceFile();
        return std::unexpected(ASTError{
            .message = std::format("AST extraction failed for file: {}", entry.file)});
    }

    action.EndSourceFile();
    result.dependencies = collect_dependency_files(instance->getSourceManager());

    result.relations.reserve(raw_relations.size());
    for(auto& edge : raw_relations) {
        result.relations.push_back(ExtractedRelation{
            .from = edge.from,
            .to = edge.to,
            .is_call = (edge.kind == RelationKind::Call),
        });
    }

    return result;
}

}  // namespace clore::extract
