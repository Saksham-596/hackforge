#pragma once
#include "hackforge/executor.hpp"

namespace hackforge {
struct FitnessScore {
    bool crashed = false;
    bool timed_out = false;
    double cpu_seconds = 0.0;
    double wall_seconds = 0.0;
    long max_rss_kb = 0;

    FitnessScore() = default;
    explicit FitnessScore(const ExecutionResult& res);
    
    // Higher is strictly "more adversarial"
    bool operator>(const FitnessScore& other) const;
};
}
