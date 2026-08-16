#include "hackforge/fuzzer.hpp"
#include "hackforge/minimizer.hpp"
#include "hackforge/executor.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>

using namespace hackforge;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: hackforge <run|replay|minimize|inspect> [args...]\n";
        return 1;
    }
    std::string cmd = argv[1];
    std::map<std::string, std::string> args;
    for (int i = 2; i < argc; i += 2) {
        if (i + 1 < argc) args[argv[i]] = argv[i + 1];
    }

    if (cmd == "run") {
        Schema schema = Schema::load(args["--schema"]);
        int iters = args.count("--iterations") ? std::stoi(args["--iterations"]) : 1000;
        double timeout = args.count("--timeout") ? std::stod(args["--timeout"]) : 2000.0;
        uint64_t seed = args.count("--seed") ? std::stoull(args["--seed"]) : 1337;
        
        std::cout << "[*] Starting HackForge Campaign...\n";
        Fuzzer fuzzer(schema, args["--target"], seed, timeout);
        fuzzer.run(iters, args["--output"]);
        std::cout << "[+] Campaign Complete. Artifacts in " << args["--output"] << "\n";
    } 
    else if (cmd == "replay") {
        std::ifstream in(args["--input"]);
        std::string input((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        auto res = Executor::run(args["--target"], input, 5000.0);
        std::cout << "Verdict: " << res.verdict << "\nWall Time: " << res.wall_seconds << "s\nMax RSS: " << res.max_rss_kb << " KB\n";
    }
    else if (cmd == "minimize") {
        Schema schema = Schema::load(args["--schema"]);
        Minimizer::minimize(args["--target"], schema, args["--input"], args["--output"]);
    }
    else if (cmd == "inspect") {
        std::ifstream in(args["--input"]);
        std::string input((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        std::cout << "[*] Input Size: " << input.size() << " bytes\n" << input << "\n";
    }
    return 0;
}
