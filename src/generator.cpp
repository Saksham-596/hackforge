#include "hackforge/generator.hpp"
#include <algorithm>
#include <numeric>
#include <unordered_map>

namespace hackforge {

Generator::Generator(const Schema& schema, uint64_t seed) 
    : schema_(schema), rng_(seed) {}

InputAST Generator::generate() {
    InputAST ast;
    std::unordered_map<std::string, long long> symbol_table;

    for (const auto& var : schema_.variables) {
        if (var.type == "int") {
            long long val = gen_int(var.min, var.max);
            ast.variables.push_back({var.name, val});
            symbol_table[var.name] = val;
        } 
        else if (var.type == "string") {
            ast.variables.push_back({var.name, gen_string(var.min, var.max)});
        }
        else if (var.type == "array<int>") {
            long long len = symbol_table[var.length_ref.value()];
            ast.variables.push_back({var.name, gen_array(len, var.min, var.max)});
        } 
        else if (var.type == "permutation") {
            long long len = symbol_table[var.length_ref.value()];
            ast.variables.push_back({var.name, gen_permutation(len)});
        }
    }
    return ast;
}

long long Generator::gen_int(long long min, long long max) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double p = dist(rng_);
    if (p < 0.1) return min;
    if (p < 0.2) return max;
    if (p < 0.3 && min <= 0 && 0 <= max) return 0;
    if (p < 0.4 && min <= 1 && 1 <= max) return 1;
    if (p < 0.5 && min <= -1 && -1 <= max) return -1;
    
    std::uniform_int_distribution<long long> uni(min, max);
    return uni(rng_);
}

std::vector<long long> Generator::gen_array(long long length, long long min, long long max) {
    std::vector<long long> arr(length);
    if (length == 0) return arr;

    std::uniform_int_distribution<long long> val_dist(min, max);
    std::uniform_int_distribution<int> mode_dist(0, 4);
    int mode = mode_dist(rng_);

    if (mode == 0) { // All equal
        long long val = val_dist(rng_);
        std::fill(arr.begin(), arr.end(), val);
    } else { // Random base
        for (auto& x : arr) x = val_dist(rng_);
        if (mode == 1) std::sort(arr.begin(), arr.end()); // Sorted
        if (mode == 2) std::sort(arr.rbegin(), arr.rend()); // Reverse sorted
        if (mode == 3 && length > 2) { // Nearly sorted (1 swap)
            std::sort(arr.begin(), arr.end());
            std::uniform_int_distribution<size_t> idx(0, length - 2);
            std::swap(arr[idx(rng_)], arr[idx(rng_) + 1]);
        }
    }
    return arr;
}

std::string Generator::gen_string(long long min_len, long long max_len) {
    std::uniform_int_distribution<long long> len_dist(min_len, max_len);
    long long len = len_dist(rng_);
    if (len == 0) return "";

    std::string s(len, 'a');
    std::uniform_int_distribution<int> char_dist('a', 'z');
    std::uniform_int_distribution<int> mode_dist(0, 2);
    int mode = mode_dist(rng_);

    if (mode == 0) { // Repeated
        char c = char_dist(rng_);
        std::fill(s.begin(), s.end(), c);
    } else {
        for (auto& c : s) c = char_dist(rng_);
        if (mode == 1) { // Palindrome
            for (long long i = 0; i < len / 2; ++i) {
                s[len - 1 - i] = s[i];
            }
        }
    }
    return s;
}

std::vector<long long> Generator::gen_permutation(long long length) {
    std::vector<long long> p(length);
    std::iota(p.begin(), p.end(), 1); // 1 to N
    
    std::uniform_int_distribution<int> mode(0, 2);
    int m = mode(rng_);
    if (m == 0) std::shuffle(p.begin(), p.end(), rng_); // Random
    if (m == 1) std::reverse(p.begin(), p.end());       // Reversed

    return p;
}

} // namespace hackforge