#include "hackforge/mutator.hpp"
#include "hackforge/generator.hpp"
#include <iostream>

using namespace hackforge;
int failures = 0;
#define ASSERT_TRUE(cond) if (!(cond)) { std::cerr << "Assertion failed: " << #cond << " at " << __LINE__ << "\n"; failures++; }

int main() {
    std::cout << "[*] Running Mutator Tests\n";

    Schema schema;
    schema.name = "mut_schema";
    schema.variables.push_back({"size", "int", 2, 5, std::nullopt});
    schema.variables.push_back({"a", "array<int>", -10, 10, "size"});
    schema.variables.push_back({"p", "permutation", 1, 1, "size"});

    Generator gen(schema, 123);
    Mutator mut(schema, 456);

    InputAST ast = gen.generate();
    std::string err;

    // Requirement #13/19: Generate -> Mutate -> Validate -> Serialize (1000 rounds)
    for (int i = 0; i < 1000; ++i) {
        mut.mutate(ast);
        ASSERT_TRUE(ast.validate(schema, &err));
    }

    if (failures > 0) return 1;
    std::cout << "[+] All Mutator tests passed!\n";
    return 0;
}