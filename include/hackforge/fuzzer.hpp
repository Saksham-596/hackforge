#pragma once
#include "hackforge/schema.hpp"
#include "hackforge/corpus.hpp"
#include <string>
#include <cstdint>

namespace hackforge {
struct CampaignStats {
    int total_executions = 0;
    int timeouts = 0;
    int crashes = 0;
    double best_runtime = 0.0;
};

class Fuzzer {
public:
    Fuzzer(const Schema& schema, const std::string& target_bin, uint64_t seed, double timeout_ms);
    void run(int iterations, const std::string& out_dir);

private:
    Schema schema_;
    std::string target_;
    uint64_t seed_;
    double timeout_;
    Corpus corpus_;
    CampaignStats stats_;
};
}