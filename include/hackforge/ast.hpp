#pragma once
#include "hackforge/schema.hpp"
#include <string>
#include <vector>
#include <variant>

namespace hackforge {
using Value = std::variant<long long, std::vector<long long>, std::string>;
struct Variable {
    std::string name;
    Value value;
};
struct InputAST {
    std::vector<Variable> variables;
    bool validate(const Schema& schema, std::string* error = nullptr) const;
    std::string serialize() const;
    
    // Parses plain text back into AST structures for the minimizer
    static InputAST parse(const Schema& schema, const std::string& text);
};
}
