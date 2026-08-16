#pragma once
#include "hackforge/ast.hpp"
#include "hackforge/fitness.hpp"
#include <vector>
#include <random>

namespace hackforge {
struct CorpusEntry {
    InputAST ast;
    FitnessScore fitness;
    std::string serialized;
};

class Corpus {
public:
    Corpus(size_t max_size);
    bool try_add(const InputAST& ast, const FitnessScore& fitness);
    CorpusEntry select_parent(std::mt19937_64& rng) const;
    const CorpusEntry& get_best() const;
    const std::vector<CorpusEntry>& get_all() const;
    bool empty() const;
    size_t size() const;
private:
    size_t max_size_;
    std::vector<CorpusEntry> entries_;
};
}
