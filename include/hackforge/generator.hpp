#pragma once
#include "hackforge/schema.hpp"
#include "hackforge/ast.hpp"
#include <random>

namespace hackforge {

class Generator {
public:
    Generator(const Schema& schema, uint64_t seed);
    InputAST generate();

    // Exposed internally for mutation engine fallbacks
    long long gen_int(long long min, long long max);
    std::vector<long long> gen_array(long long length, long long min, long long max);
    std::string gen_string(long long min_len, long long max_len);
    std::vector<long long> gen_permutation(long long length);

private:
    Schema schema_;
    std::mt19937_64 rng_;
};

} // namespace hackforge