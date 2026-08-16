#include "hackforge/corpus.hpp"
#include <algorithm>
#include <stdexcept>

namespace hackforge {
Corpus::Corpus(size_t max_size) : max_size_(max_size) {}

bool Corpus::try_add(const InputAST& ast, const FitnessScore& fitness) {
    std::string ser = ast.serialize();
    for (const auto& e : entries_) {
        if (e.serialized == ser) return false; // Deduplicate
    }
    entries_.push_back({ast, fitness, ser});
    std::sort(entries_.begin(), entries_.end(), [](const CorpusEntry& a, const CorpusEntry& b) {
        return a.fitness > b.fitness; // Descending
    });
    if (entries_.size() > max_size_) entries_.pop_back();
    return true;
}

CorpusEntry Corpus::select_parent(std::mt19937_64& rng) const {
    if (entries_.empty()) throw std::runtime_error("Corpus empty");
    std::uniform_int_distribution<size_t> dist(0, entries_.size() - 1);
    size_t best_idx = dist(rng);
    for (int i = 0; i < 2; ++i) { // Tournament size 3
        size_t candidate = dist(rng);
        if (candidate < best_idx) best_idx = candidate; 
    }
    return entries_[best_idx];
}

const CorpusEntry& Corpus::get_best() const { return entries_.front(); }
const std::vector<CorpusEntry>& Corpus::get_all() const { return entries_; }
bool Corpus::empty() const { return entries_.empty(); }
size_t Corpus::size() const { return entries_.size(); }
}
