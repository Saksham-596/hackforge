#include "hackforge/mutator.hpp"
#include "hackforge/generator.hpp"
#include <algorithm>

namespace hackforge {

Mutator::Mutator(const Schema& schema, uint64_t seed) : schema_(schema), rng_(seed) {}

bool Mutator::mutate(InputAST& ast) {
    if (ast.variables.empty()) return false;
    std::uniform_int_distribution<size_t> var_dist(0, ast.variables.size() - 1);
    size_t idx = var_dist(rng_);
    auto& var = ast.variables[idx];

    SchemaVariable sch_var;
    for (const auto& sv : schema_.variables) {
        if (sv.name == var.name) { sch_var = sv; break; }
    }

    if (sch_var.type == "int") {
        long long& val = std::get<long long>(var.value);
        val = mutate_int(val, sch_var.min, sch_var.max);
        
        Generator helper_gen(schema_, rng_());
        for (size_t i = idx + 1; i < ast.variables.size(); ++i) {
            SchemaVariable dep_sch;
            for (const auto& sv : schema_.variables) {
                if (sv.name == ast.variables[i].name) { dep_sch = sv; break; }
            }
            if (dep_sch.length_ref && dep_sch.length_ref.value() == var.name) {
                if (dep_sch.type == "array<int>") {
                    auto& arr = std::get<std::vector<long long>>(ast.variables[i].value);
                    while(arr.size() > static_cast<size_t>(val)) arr.pop_back();
                    while(arr.size() < static_cast<size_t>(val)) arr.push_back(helper_gen.gen_int(dep_sch.min, dep_sch.max));
                } else if (dep_sch.type == "permutation") {
                    ast.variables[i].value = helper_gen.gen_permutation(val);
                }
            }
        }
    } 
    else if (sch_var.type == "array<int>") {
        mutate_array(std::get<std::vector<long long>>(var.value), sch_var.min, sch_var.max);
    }
    else if (sch_var.type == "string") {
        mutate_string(std::get<std::string>(var.value), sch_var.min, sch_var.max);
    }
    else if (sch_var.type == "permutation") {
        mutate_permutation(std::get<std::vector<long long>>(var.value));
    }
    return true;
}

long long Mutator::mutate_int(long long val, long long min, long long max) {
    std::uniform_int_distribution<int> op_dist(0, 4);
    int op = op_dist(rng_);
    if (op == 0 && val < max) return val + 1;
    if (op == 1 && val > min) return val - 1;
    if (op == 2) return min;
    if (op == 3) return max; // Explicitly bias towards maximum limits for algorithmic stress
    std::uniform_int_distribution<long long> r(min, max);
    return r(rng_);
}

void Mutator::mutate_array(std::vector<long long>& arr, long long min, long long max) {
    if (arr.empty()) return;
    std::uniform_int_distribution<int> op_dist(0, 5);
    int op = op_dist(rng_);

    if (op == 0) std::reverse(arr.begin(), arr.end());
    else if (op == 1) std::sort(arr.begin(), arr.end());
    else if (op == 2) std::sort(arr.begin(), arr.end(), std::greater<long long>());
    else if (op == 3 && arr.size() >= 2) {
        std::uniform_int_distribution<size_t> idx(0, arr.size() - 2);
        std::swap(arr[idx(rng_)], arr[idx(rng_) + 1]);
    }
    else if (op == 4) { // Fill with constant (targets duplicate-heavy vulnerabilities)
        std::uniform_int_distribution<long long> val(min, max);
        std::fill(arr.begin(), arr.end(), val(rng_));
    } else { 
        std::uniform_int_distribution<size_t> idx(0, arr.size() - 1);
        std::uniform_int_distribution<long long> val(min, max);
        arr[idx(rng_)] = val(rng_);
    }
}

void Mutator::mutate_string(std::string& s, long long min_len, long long max_len) {
    if (s.empty() && min_len == 0 && max_len > 0) { s.push_back('a'); return; }
    if (s.empty()) return;
    std::uniform_int_distribution<int> op(0, 3);
    int choice = op(rng_);
    if (choice == 0) { 
        std::uniform_int_distribution<size_t> idx(0, s.size() - 1);
        std::uniform_int_distribution<int> ch('a', 'z');
        s[idx(rng_)] = ch(rng_);
    } else if (choice == 1) { 
        std::reverse(s.begin(), s.end());
    } else if (choice == 2 && static_cast<long long>(s.size()) > min_len) {
        std::uniform_int_distribution<size_t> idx(0, s.size() - 1);
        s.erase(idx(rng_), 1);
    } else if (choice == 3 && static_cast<long long>(s.size()) < max_len) {
        std::uniform_int_distribution<size_t> idx(0, s.size());
        std::uniform_int_distribution<int> ch('a', 'z');
        s.insert(s.begin() + idx(rng_), ch(rng_));
    }
}

void Mutator::mutate_permutation(std::vector<long long>& p) {
    if (p.size() < 2) return;
    std::uniform_int_distribution<size_t> idx(0, p.size() - 1);
    std::swap(p[idx(rng_)], p[idx(rng_)]); 
}

} // namespace hackforge
