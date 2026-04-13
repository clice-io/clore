#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace clore::testing {

struct CompDbEntry {
    std::filesystem::path directory;
    std::filesystem::path file;
    std::vector<std::string> arguments;
};

inline auto json_escape(std::string_view value) -> std::string {
    std::string escaped;
    escaped.reserve(value.size());

    for(char ch : value) {
        if(ch == '\\' || ch == '"') {
            escaped.push_back('\\');
        }
        escaped.push_back(ch);
    }

    return escaped;
}

inline void write_compile_commands(const std::filesystem::path& path,
                                   const std::vector<CompDbEntry>& entries) {
    std::ofstream out(path);
    out << "[\n";

    for(std::size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        if(i > 0) {
            out << ",\n";
        }

        out << "  {\n";
        out << "    \"directory\": \"" << json_escape(entry.directory.string()) << "\",\n";
        out << "    \"file\": \"" << json_escape(entry.file.string()) << "\",\n";
        out << "    \"arguments\": [";

        for(std::size_t arg_index = 0; arg_index < entry.arguments.size(); ++arg_index) {
            if(arg_index > 0) {
                out << ", ";
            }
            out << "\"" << json_escape(entry.arguments[arg_index]) << "\"";
        }

        out << "]\n";
        out << "  }";
    }

    out << "\n]";
}

}  // namespace clore::testing
