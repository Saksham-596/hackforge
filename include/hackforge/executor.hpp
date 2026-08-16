#pragma once
#include <string>

namespace hackforge {

struct ExecutionResult {
    bool launched = false;
    bool timed_out = false;
    bool crashed = false;

    int exit_code = -1;
    int signal = 0;

    double wall_seconds = 0.0;
    double cpu_seconds = 0.0;

    long max_rss_kb = 0;

    std::string error;
    std::string verdict;
};

class Executor {
public:
    // Executes the target binary with standard POSIX constructs.
    // Handles stdin feeding safely and enforces strict time limits.
    static ExecutionResult run(
        const std::string& target_path,
        const std::string& input,
        double timeout_ms
    );
};

} // namespace hackforge