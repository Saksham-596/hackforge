#include "hackforge/ast.hpp"
#include <sstream>
#include <algorithm>

namespace hackforge {
std::string InputAST::serialize() const {
    std::stringstream ss;
    for (const auto& var : variables) {
        std::visit([&ss](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, long long>) {
                ss << arg << "\n";
            } else if constexpr (std::is_same_v<T, std::vector<long long>>) {
                for (size_t i = 0; i < arg.size(); ++i) {
                    ss << arg[i] << (i + 1 == arg.size() ? "" : " ");
                }
                ss << "\n";
            } else if constexpr (std::is_same_v<T, std::string>) {
                ss << arg << "\n";
            }
        }, var.value);
    }
    return ss.str();
}

InputAST InputAST::parse(const Schema& schema, const std::string& text) {
    InputAST ast;
    std::istringstream iss(text);
    for (const auto& schema_var : schema.variables) {
        if (schema_var.type == "int") {
            long long val; iss >> val;
            ast.variables.push_back({schema_var.name, val});
        } else if (schema_var.type == "string") {
            std::string val; iss >> val;
            ast.variables.push_back({schema_var.name, val});
        } else if (schema_var.type == "array<int>" || schema_var.type == "permutation") {
            long long len = 0;
            for (const auto& v : ast.variables) {
                if (v.name == schema_var.length_ref.value()) {
                    len = std::get<long long>(v.value); break;
                }
            }
            std::vector<long long> arr(len);
            for (long long i = 0; i < len; ++i) iss >> arr[i];
            ast.variables.push_back({schema_var.name, arr});
        }
    }
    return ast;
}

bool InputAST::validate(const Schema& schema, std::string* error) const {
    (void)error; // Suppress unused parameter warning
    for (const auto& schema_var : schema.variables) {
        auto it = std::find_if(variables.begin(), variables.end(),
                               [&](const Variable& v) { return v.name == schema_var.name; });
        if (it == variables.end()) return false;

        if (schema_var.type == "int") {
            if (!std::holds_alternative<long long>(it->value)) return false;
            long long val = std::get<long long>(it->value);
            if (val < schema_var.min || val > schema_var.max) return false;
        } 
        else if (schema_var.type == "string") {
            if (!std::holds_alternative<std::string>(it->value)) return false;
            const auto& str = std::get<std::string>(it->value);
            if (static_cast<long long>(str.size()) < schema_var.min || static_cast<long long>(str.size()) > schema_var.max) return false;
        }
        else if (schema_var.type == "array<int>" || schema_var.type == "permutation") {
            if (!std::holds_alternative<std::vector<long long>>(it->value)) return false;
            const auto& arr = std::get<std::vector<long long>>(it->value);
            
            auto len_it = std::find_if(variables.begin(), variables.end(),
                                       [&](const Variable& v) { return v.name == schema_var.length_ref.value(); });
            if (len_it == variables.end() || !std::holds_alternative<long long>(len_it->value)) return false;
            long long expected_len = std::get<long long>(len_it->value);
            if (static_cast<long long>(arr.size()) != expected_len) return false;

            if (schema_var.type == "array<int>") {
                for (long long val : arr) {
                    if (val < schema_var.min || val > schema_var.max) return false;
                }
            } else {
                std::vector<long long> check = arr;
                std::sort(check.begin(), check.end());
                for (long long i = 0; i < expected_len; ++i) {
                    if (check[i] != i + 1) return false; 
                }
            }
        }
    }
    return true;
}
} // namespace hackforge
