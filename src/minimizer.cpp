#include "hackforge/minimizer.hpp"
#include "hackforge/executor.hpp"
#include "hackforge/fitness.hpp"
#include "hackforge/ast.hpp"
#include <fstream>
#include <iostream>

namespace hackforge {
void Minimizer::minimize(const std::string& target, const Schema& schema, const std::string& input_file, const std::string& output_file) {
    std::ifstream in(input_file);
    std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    
    InputAST ast = InputAST::parse(schema, text);
    auto baseline_res = Executor::run(target, ast.serialize(), 5000.0);
    FitnessScore best_fit(baseline_res);

    std::cout << "[*] Minimizing. Baseline CPU Sec: " << best_fit.cpu_seconds << "\n";

    // Basic Delta Debugging: Replace elements with 0 to simplify the adversarial input
    for (auto& var : ast.variables) {
        if (std::holds_alternative<std::vector<long long>>(var.value)) {
            auto& arr = std::get<std::vector<long long>>(var.value);
            // Step size prevents taking hours on massive arrays. 
            size_t step = std::max<size_t>(1, arr.size() / 20); 
            for (size_t i = 0; i < arr.size(); i += step) {
                long long orig = arr[i];
                arr[i] = 0; // Attempt to simplify
                
                auto res = Executor::run(target, ast.serialize(), 5000.0);
                FitnessScore new_fit(res);
                
                bool keep = false;
                if (best_fit.crashed && new_fit.crashed) keep = true;
                else if (best_fit.timed_out && new_fit.timed_out) keep = true;
                else if (new_fit.cpu_seconds >= best_fit.cpu_seconds * 0.90) keep = true; // Retain 90% algorithmic load
                
                if (keep) {
                    best_fit = new_fit;
                } else {
                    arr[i] = orig; // Revert
                }
            }
        }
    }
    
    std::ofstream out(output_file);
    out << ast.serialize();
    std::cout << "[+] Minimized case saved to " << output_file << "\n";
}
}
