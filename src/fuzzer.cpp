#include "hackforge/fuzzer.hpp"
#include "hackforge/generator.hpp"
#include "hackforge/mutator.hpp"
#include "hackforge/executor.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace hackforge {
Fuzzer::Fuzzer(const Schema& schema, const std::string& target, uint64_t seed, double timeout_ms)
    : schema_(schema), target_(target), seed_(seed), timeout_(timeout_ms), corpus_(50) {}

void Fuzzer::run(int iterations, const std::string& out_dir) {
    std::mt19937_64 rng(seed_);
    Generator gen(schema_, seed_);
    Mutator mut(schema_, seed_);

    std::filesystem::create_directories(out_dir + "/corpus");

    for (int i = 0; i < iterations; ++i) {
        InputAST current_ast;
        std::uniform_real_distribution<double> p(0.0, 1.0);
        
        if (corpus_.empty() || p(rng) < 0.25) {
            current_ast = gen.generate();
        } else {
            current_ast = corpus_.select_parent(rng).ast;
            mut.mutate(current_ast);
        }

        std::string input_str = current_ast.serialize();
        auto res = Executor::run(target_, input_str, timeout_);
        
        stats_.total_executions++;
        if (res.crashed) stats_.crashes++;
        if (res.timed_out) stats_.timeouts++;

        FitnessScore fit(res);
        if (corpus_.try_add(current_ast, fit)) {
            stats_.best_runtime = std::max(stats_.best_runtime, fit.cpu_seconds);
            
            // Save the corpus trace
            std::ofstream out_case(out_dir + "/corpus/case_" + std::to_string(stats_.total_executions) + ".txt");
            out_case << input_str;
        }
    }

    if (!corpus_.empty()) {
        const auto& best = corpus_.get_best();
        std::ofstream out(out_dir + "/best_case.txt");
        out << best.serialized;
        
        std::ofstream meta(out_dir + "/metadata.json");
        meta << "{\n  \"seed\": " << seed_ << ",\n  \"executions\": " << stats_.total_executions
             << ",\n  \"crashes\": " << stats_.crashes << ",\n  \"timeouts\": " << stats_.timeouts
             << ",\n  \"best_cpu_sec\": " << best.fitness.cpu_seconds 
             << ",\n  \"best_wall_sec\": " << best.fitness.wall_seconds << "\n}\n";

        std::ofstream txt(out_dir + "/metadata.txt");
        txt << "HackForge Campaign Summary\n==========================\n"
            << "Target: " << target_ << "\n"
            << "Total Executions: " << stats_.total_executions << "\n"
            << "Best Algorithmic CPU Time: " << best.fitness.cpu_seconds << "s\n"
            << "Best Gross Wall Time: " << best.fitness.wall_seconds << "s\n";
            
        std::ofstream csv(out_dir + "/corpus.csv");
        csv << "rank,crashed,timed_out,cpu_seconds,wall_seconds,max_rss_kb\n";
        int r = 1;
        for (const auto& entry : corpus_.get_all()) {
            csv << r++ << "," << entry.fitness.crashed << "," << entry.fitness.timed_out << "," 
                << entry.fitness.cpu_seconds << "," << entry.fitness.wall_seconds << "," << entry.fitness.max_rss_kb << "\n";
        }
    }
}
}
