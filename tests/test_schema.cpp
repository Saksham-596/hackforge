#include "hackforge/schema.hpp"
#include <iostream>

using namespace hackforge;
int failures = 0;

#define EXPECT_THROW(stmt, err_str) \
    try { stmt; std::cerr << "[-] Expected exception for: " << #stmt << "\n"; failures++; } \
    catch (const std::exception& e) { \
        if (std::string(e.what()).find(err_str) == std::string::npos) { \
            std::cerr << "[-] Wrong exception message: " << e.what() << "\n"; failures++; \
        } \
    }

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_schema <fixtures_dir>\n";
        return 1;
    }
    std::string fix_dir = std::string(argv[1]) + "/";

    std::cout << "[*] Running Schema Tests\n";
    
    // 1. Valid Integer
    auto schema1 = Schema::load(fix_dir + "valid_int.yaml");
    if (schema1.name != "simple_int" || schema1.variables[0].type != "int") failures++;

    // 2. Valid Array
    auto schema2 = Schema::load(fix_dir + "valid_array.yaml");
    if (schema2.variables[1].type != "array<int>" || schema2.variables[1].length_ref != "n") failures++;

    // 3. Invalid Type
    EXPECT_THROW(Schema::load(fix_dir + "invalid_type.yaml"), "unsupported type");

    // 4. Min > Max
    EXPECT_THROW(Schema::load(fix_dir + "invalid_range.yaml"), "min > max");

    // 5. Unknown array length reference
    EXPECT_THROW(Schema::load(fix_dir + "invalid_length_ref.yaml"), "not found before it");

    if (failures > 0) return 1;
    std::cout << "[+] All Schema tests passed!\n";
    return 0;
}