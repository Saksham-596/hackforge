#include "hackforge/fitness.hpp"
#include <cmath>

namespace hackforge {
FitnessScore::FitnessScore(const ExecutionResult& res) {
    crashed = res.crashed;
    timed_out = res.timed_out;
    cpu_seconds = res.cpu_seconds;
    wall_seconds = res.wall_seconds;
    max_rss_kb = res.max_rss_kb;
}

bool FitnessScore::operator>(const FitnessScore& other) const {
    if (crashed != other.crashed) return crashed;
    if (timed_out != other.timed_out) return timed_out;
    
    // Primary Metric: CPU seconds. This filters out OS fork/exec scheduling noise 
    // and explicitly isolates algorithmic complexity loops.
    if (std::abs(cpu_seconds - other.cpu_seconds) > 0.005) {
        return cpu_seconds > other.cpu_seconds;
    }
    
    // Fallback if CPU times are practically identical
    return wall_seconds > other.wall_seconds; 
}
}
