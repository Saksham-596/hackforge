#pragma once
#include "hackforge/schema.hpp"
#include "hackforge/ast.hpp"
#include <random>

namespace hackforge {

class Mutator {
public:
    Mutator(const Schema& schema, uint64_t seed);

    // Mutates the AST in place. Resolves downstream constraints automatically.
    // Returns true if a mutation was applied.
    bool mutate(InputAST& ast);

private:
    Schema schema_;
    std::mt19937_64 rng_;

    long long mutate_int(long long val, long long min, long long max);
    void mutate_array(std::vector<long long>& arr, long long min, long long max);
    void mutate_string(std::string& s, long long min, long long max);
    void mutate_permutation(std::vector<long long>& p);
};

} // namespace hackforge