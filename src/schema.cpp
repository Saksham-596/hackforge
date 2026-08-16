#include "hackforge/schema.hpp"
#include <yaml-cpp/yaml.h>
#include <stdexcept>
#include <algorithm>

namespace hackforge {

Schema Schema::load(const std::filesystem::path& path) {
    Schema schema;
    YAML::Node root;
    try {
        root = YAML::LoadFile(path.string());
    } catch (const YAML::Exception& e) {
        throw std::runtime_error("Failed to parse YAML file '" + path.string() + "': " + e.what());
    }

    if (!root["name"]) throw std::runtime_error("Invalid schema: missing 'name' field.");
    schema.name = root["name"].as<std::string>();

    if (!root["variables"] || !root["variables"].IsSequence()) {
        throw std::runtime_error("Invalid schema: missing or invalid 'variables' section.");
    }

    for (const auto& node : root["variables"]) {
        SchemaVariable var;
        if (!node["name"]) throw std::runtime_error("Invalid schema: variable missing 'name'.");
        var.name = node["name"].as<std::string>();

        if (!node["type"]) throw std::runtime_error("Invalid schema: variable '" + var.name + "' missing 'type'.");
        var.type = node["type"].as<std::string>();

        if (var.type != "int" && var.type != "array<int>" && var.type != "string" && var.type != "permutation") {
            throw std::runtime_error("Invalid schema: variable '" + var.name + "' has unsupported type '" + var.type + "'.");
        }

        // Ints, Arrays, and Strings require min/max limits
        if (var.type == "int" || var.type == "array<int>" || var.type == "string") {
            if (!node["min"] || !node["max"]) throw std::runtime_error("Invalid schema: variable '" + var.name + "' must have 'min' and 'max'.");
            var.min = node["min"].as<long long>();
            var.max = node["max"].as<long long>();
            if (var.min > var.max) throw std::runtime_error("Invalid schema: variable '" + var.name + "' has min > max.");
        } else {
            var.min = 1; var.max = 1; // Permutations ignore this safely
        }

        if (var.type == "array<int>" || var.type == "permutation") {
            if (!node["length"]) throw std::runtime_error("Invalid schema: collection '" + var.name + "' missing 'length' reference.");
            var.length_ref = node["length"].as<std::string>();

            auto it = std::find_if(schema.variables.begin(), schema.variables.end(),
                                   [&](const SchemaVariable& v) { return v.name == var.length_ref.value(); });
            if (it == schema.variables.end()) {
                throw std::runtime_error("Invalid schema: length reference '" + var.length_ref.value() + "' not found before it.");
            }
            if (it->type != "int") {
                throw std::runtime_error("Invalid schema: length reference '" + var.length_ref.value() + "' is not of type 'int'.");
            }
        }
        schema.variables.push_back(var);
    }
    return schema;
}

} // namespace hackforge