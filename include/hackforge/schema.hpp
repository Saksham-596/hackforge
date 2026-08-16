#pragma once
#include <string>
#include <vector>
#include <optional>
#include <filesystem>

namespace hackforge {

struct SchemaVariable {
    std::string name;
    std::string type; // "int", "array<int>", "string", "permutation"
    long long min;    // Value min, or length min for strings
    long long max;    // Value max, or length max for strings
    std::optional<std::string> length_ref; // Used by array/permutation
};

struct Schema {
    std::string name;
    std::vector<SchemaVariable> variables;

    static Schema load(const std::filesystem::path& path);
};

} // namespace hackforge