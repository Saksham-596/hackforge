#include "hackforge/generator.hpp"
#include <iostream>

using namespace hackforge;
int failures = 0;

#define ASSERT_TRUE(cond) if (!(cond)) { std::cerr << "Assertion failed: " << #cond << " at " << __LINE__ << "\n"; failures++; }

int main() {
    std::cout << "[*] Running Generator Tests & Demo\n";

    // Build Demo Schema
    Schema schema;
    schema.name = "demo_schema";
    schema.variables.push_back({"n", "int", 1, 100, std::nullopt});
    schema.variables.push_back({"arr", "array<int>", -10, 10, "n"});
    schema.variables.push_back({"str", "string", 3, 10, std::nullopt});
    schema.variables.push_back({"perm", "permutation", 1, 1, "n"});

    // Requirement #16: Determinism Test
    Generator gen1(schema, 42);
    Generator gen2(schema, 42);
    auto ast1 = gen1.generate();
    auto ast2 = gen2.generate();
    ASSERT_TRUE(ast1.serialize() == ast2.serialize()); // Must perfectly match

    // Requirement #17/19: Generate -> Validate -> Serialize
    Generator gen3(schema, 1337);
    for (int i = 0; i < 50; ++i) {
        auto ast = gen3.generate();
        std::string err;
        ASSERT_TRUE(ast.validate(schema, &err));
    }

    if (failures > 0) return 1;
    std::cout << "[+] All Generator tests passed!\n";
    return 0;
}