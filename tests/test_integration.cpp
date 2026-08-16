#include "hackforge/fuzzer.hpp"
#include <iostream>
#include <filesystem>

using namespace hackforge;
int main(int argc, char** argv) {
    if (argc < 3) return 1;
    Schema schema = Schema::load(argv[2]);
    Fuzzer fuzzer(schema, argv[1], 42, 50.0); // 50ms timeout. sleep_helper sleeps 10s.
    fuzzer.run(10, "build/integration_test_out"); // 10 loops
    
    if (!std::filesystem::exists("build/integration_test_out/best_case.txt")) return 1;
    std::cout << "[+] Integration End-to-End Passed!\n";
    return 0;
}