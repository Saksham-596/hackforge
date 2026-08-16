#include "hackforge/ast.hpp"
#include <iostream>

using namespace hackforge;
int failures = 0;

#define ASSERT_TRUE(cond) if (!(cond)) { std::cerr << "Assertion failed: " << #cond << " at " << __LINE__ << "\n"; failures++; }
#define ASSERT_FALSE(cond) if (cond) { std::cerr << "Assertion failed: " << #cond << " at " << __LINE__ << "\n"; failures++; }
#define ASSERT_EQ(val1, val2) if ((val1) != (val2)) { std::cerr << "Assertion failed: " << (val1) << " != " << (val2) << " at " << __LINE__ << "\n"; failures++; }

void test_serialization() {
    InputAST ast;
    ast.variables.push_back({"n", 5LL});
    ast.variables.push_back({"a", std::vector<long long>{3, 1, 4, 2, 5}});
    
    std::string expected = "5\n3 1 4 2 5\n";
    ASSERT_EQ(ast.serialize(), expected);
}

void test_validation() {
    Schema schema;
    schema.name = "array_sort";
    schema.variables.push_back({"n", "int", 1, 10, std::nullopt});
    schema.variables.push_back({"a", "array<int>", -100, 100, "n"});

    InputAST ast;
    std::string err;

    // Valid AST
    ast.variables = {{"n", 5LL}, {"a", std::vector<long long>{3, 1, 4, 2, 5}}};
    ASSERT_TRUE(ast.validate(schema, &err));

    // Invalid: Scalar out of bounds
    ast.variables = {{"n", 15LL}, {"a", std::vector<long long>{3, 1, 4, 2, 5}}};
    ASSERT_FALSE(ast.validate(schema, &err));

    // Invalid: Array length mismatch
    ast.variables = {{"n", 5LL}, {"a", std::vector<long long>{1, 2}}};
    ASSERT_FALSE(ast.validate(schema, &err));

    // Invalid: Array element out of bounds
    ast.variables = {{"n", 2LL}, {"a", std::vector<long long>{101, -200}}};
    ASSERT_FALSE(ast.validate(schema, &err));
}

int main() {
    std::cout << "[*] Running AST Tests\n";
    test_serialization();
    test_validation();
    
    if (failures > 0) return 1;
    std::cout << "[+] All AST tests passed!\n";
    return 0;
}