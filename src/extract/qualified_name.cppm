module;

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

export module extract:qualified_name;

export namespace clore::extract {

auto split_top_level_qualified_name(std::string_view qualified_name)
    -> std::vector<std::string>;

auto join_qualified_name_parts(const std::vector<std::string>& parts,
                               std::size_t count) -> std::string;

auto namespace_prefix_from_qualified_name(std::string_view qualified_name)
    -> std::string;

}  // namespace clore::extract

namespace clore::extract {

auto split_top_level_qualified_name(std::string_view qualified_name)
    -> std::vector<std::string> {
    std::vector<std::string> parts;
    std::string current;
    current.reserve(qualified_name.size());

    std::size_t template_depth = 0;
    for(std::size_t index = 0; index < qualified_name.size(); ++index) {
        auto ch = qualified_name[index];
        if(ch == '<') {
            template_depth++;
            current.push_back(ch);
            continue;
        }
        if(ch == '>') {
            if(template_depth > 0) {
                template_depth--;
            }
            current.push_back(ch);
            continue;
        }
        if(ch == ':' && template_depth == 0 && index + 1 < qualified_name.size() &&
           qualified_name[index + 1] == ':') {
            parts.push_back(current);
            current.clear();
            ++index;
            continue;
        }
        current.push_back(ch);
    }

    if(!current.empty()) {
        parts.push_back(std::move(current));
    }

    return parts;
}

auto join_qualified_name_parts(const std::vector<std::string>& parts,
                               std::size_t count) -> std::string {
    std::string joined;
    auto safe_count = count < parts.size() ? count : parts.size();
    for(std::size_t index = 0; index < safe_count; ++index) {
        if(index > 0) {
            joined += "::";
        }
        joined += parts[index];
    }
    return joined;
}

auto namespace_prefix_from_qualified_name(std::string_view qualified_name)
    -> std::string {
    auto parts = split_top_level_qualified_name(qualified_name);
    if(parts.size() <= 1) {
        return {};
    }
    return join_qualified_name_parts(parts, parts.size() - 1);
}

}  // namespace clore::extract