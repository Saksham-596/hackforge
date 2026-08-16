#include "hackforge/executor.hpp"
#include <iostream>
#include <string>

using namespace hackforge;

int failures = 0;

#define ASSERT_TRUE(cond) \
    if (!(cond)) { \
        std::cerr << "Assertion failed: " << #cond << " at " << __LINE__ << "\n"; \
        failures++; \
    }

#define ASSERT_EQ(val1, val2) \
    if ((val1) != (val2)) { \
        std::cerr << "Assertion failed: " << #val1 << " == " << #val2 << " at " << __LINE__ \
                  << " (" << (val1) << " != " << (val2) << ")\n"; \
        failures++; \
    }

void test_normal_execution(const std::string& bin) {
    std::cout << "[*] Running test_normal_execution\n";
    auto res = Executor::run(bin, "hello\n", 5000.0);
    ASSERT_TRUE(res.launched);
    ASSERT_TRUE(!res.timed_out);
    ASSERT_TRUE(!res.crashed);
    ASSERT_EQ(res.verdict, "OK");
    ASSERT_EQ(res.exit_code, 0);
    ASSERT_TRUE(res.wall_seconds >= 0.0);
    ASSERT_TRUE(res.cpu_seconds >= 0.0);
}

void test_stdin(const std::string& bin) {
    std::cout << "[*] Running test_stdin\n";
    
    // Correct input -> Helper exits 0
    auto res1 = Executor::run(bin, "hello", 5000.0);
    ASSERT_EQ(res1.exit_code, 0);

    // Wrong input -> Helper exits 1 (Treated as Crash)
    auto res2 = Executor::run(bin, "wrong", 5000.0);
    ASSERT_EQ(res2.exit_code, 1);
    ASSERT_EQ(res2.verdict, "CRASH"); 
}

void test_nonzero_exit(const std::string& bin) {
    std::cout << "[*] Running test_nonzero_exit\n";
    auto res = Executor::run(bin, "", 5000.0);
    ASSERT_TRUE(res.crashed);
    ASSERT_EQ(res.exit_code, 42);
    ASSERT_EQ(res.verdict, "CRASH");
}

void test_timeout(const std::string& bin) {
    std::cout << "[*] Running test_timeout\n";
    auto res = Executor::run(bin, "", 200.0); // 200ms
    ASSERT_TRUE(res.launched);
    ASSERT_TRUE(res.timed_out);
    ASSERT_EQ(res.verdict, "TLE");
    ASSERT_TRUE(res.wall_seconds >= 0.2);
    ASSERT_TRUE(res.wall_seconds < 0.6); // Parent correctly didn't hang
}

void test_cpu_wall_measurement(const std::string& bin) {
    std::cout << "[*] Running test_cpu_wall_measurement\n";
    auto res = Executor::run(bin, "", 5000.0);
    ASSERT_TRUE(res.launched);
    ASSERT_EQ(res.verdict, "OK");
    ASSERT_TRUE(res.wall_seconds > 0.0);
    ASSERT_TRUE(res.cpu_seconds > 0.0);
    std::cout << "    Wall: " << res.wall_seconds << "s, CPU: " << res.cpu_seconds 
              << "s, MaxRSS: " << res.max_rss_kb << " KB\n";
}

void test_invalid_executable() {
    std::cout << "[*] Running test_invalid_executable\n";
    auto res = Executor::run("/path/to/nonexistent/executable", "", 1000.0);
    ASSERT_TRUE(!res.launched);
    ASSERT_TRUE(res.error.find("exec() failed") != std::string::npos ||
                res.error.find("fork() failed") != std::string::npos);
}

int main(int argc, char** argv) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <check_in_bin> <exit_bin> <sleep_bin> <cpu_burn_bin>\n";
        return 1;
    }

    test_normal_execution(argv[1]);
    test_stdin(argv[1]);
    test_nonzero_exit(argv[2]);
    test_timeout(argv[3]);
    test_cpu_wall_measurement(argv[4]);
    test_invalid_executable();

    if (failures > 0) {
        std::cerr << "\n[-] " << failures << " tests failed!\n";
        return 1;
    }

    std::cout << "\n[+] All tests passed successfully!\n";
    return 0;
}