# HackForge 🔨

HackForge is a deterministic, evolutionary adversarial testing engine for competitive programming. 

Standard randomized testing (like generating arrays of random numbers) almost never triggers algorithmic edge cases. HackForge uses a structured **YAML Schema**, an **Evolutionary Fuzzer**, and a **Delta-Debugging Minimizer** to actively hunt for inputs that cause Time Limit Exceeded (TLE) or Runtime Error (Crash) verdicts in your code.

## 🚀 The "I Just Want to Test My Code" Guide
You don't need to understand ASTs or mutations to use this. If you are grinding for Candidate Master and keep getting TLE on Test 42, here is how you break your code locally.

**Step 1: Write your target code**
Save your solution as a standard C++ file (e.g., `my_solution.cpp`) and compile it:
```bash
g++ my_solution.cpp -o my_solution
```

**Step 2: Write a Schema**
Tell HackForge what a valid test case looks like. Create a file called `schema.yaml`:
```yaml
name: array_test
variables:
  - name: n
    type: int
    min: 1
    max: 5000
  - name: a
    type: array<int>
    length: n
    min: -10000
    max: 10000
```

**Step 3: Unleash HackForge**
Run a fuzzing campaign to find the worst-case input for your code:
```bash
./build/hackforge run --target ./my_solution --schema schema.yaml --iterations 1000 --output artifacts/
```

Check `artifacts/best_case.txt`. If your algorithm has a hidden O(N^2) trap, HackForge just found it.

---

## 🧠 How the Engine Works (Code Flow)

HackForge is built on a modular pipeline designed for speed and deterministic execution.

1. **Schema Parsing (`schema.cpp`):** Reads the YAML file and establishes the strict mathematical rules (bounds, dependencies) for the input.
2. **Generation (`generator.cpp`):** An MT19937_64 RNG engine generates initial Abstract Syntax Trees (ASTs) that strictly obey the schema.
3. **Execution (`executor.cpp`):** The engine forks a child process, pipes the serialized AST as standard input, and measures algorithmic CPU time, Wall time, and memory usage via POSIX APIs.
4. **Fitness & Corpus (`fitness.cpp` & `corpus.cpp`):** The engine scores the execution. Crashes and TLEs are prized. High-scoring inputs are saved to a Tournament-Selection Corpus.
5. **Evolutionary Mutation (`mutator.cpp`):** The Fuzzer grabs the best inputs from the Corpus and mutates them (e.g., sorting arrays, reversing, boundary testing). **Cascading Logic** ensures that if an integer `n` is mutated, any array depending on `n` is safely resized without violating the schema.
6. **Minimization (`minimizer.cpp`):** Once a pathological input is found, a delta-debugging algorithm zeros out chunks of the input to find the smallest, most readable test case that still breaks the target.

---

## 📂 Project Architecture & File Index

### Technologies Used
* **Core Engine:** C++20
* **Build System:** CMake, CTest
* **Dependencies:** `yaml-cpp` (Fetched dynamically at build)
* **Upcoming Infrastructure:** Node.js (API), Next.js (UI), Docker (Containerization)

### Directory Structure
```text
hackforge/
├── CMakeLists.txt              # Core build configuration
├── include/hackforge/          # C++ Header declarations
│   ├── ast.hpp                 # Defines InputAST and internal Variables
│   ├── corpus.hpp              # Bounded tournament-selection memory
│   ├── executor.hpp            # POSIX process runner & resource tracker
│   ├── fitness.hpp             # CPU-weighted scoring logic
│   ├── fuzzer.hpp              # The main evolutionary loop
│   ├── generator.hpp           # Deterministic structure builder
│   ├── minimizer.hpp           # Delta-debugging reduction engine
│   ├── mutator.hpp             # Schema-preserving mutation logic
│   └── schema.hpp              # YAML validation rules
├── src/                        # C++ Implementations
│   ├── ast.cpp                 # AST Serialization and text parsing
│   ├── cli.cpp                 # CLI argument routing 
│   ├── corpus.cpp              # Deduplication and tournament logic
│   ├── executor.cpp            # fork(), exec(), getrusage() internals
│   ├── fitness.cpp             # Mathematical evaluation of OS noise vs Algorithmic load
│   ├── fuzzer.cpp              # 25/75 Gen/Mutate campaign router
│   ├── generator.cpp           # Type-specific distribution engines
│   ├── main.cpp                # The core binary entry point
│   ├── minimizer.cpp           # Shrinks adversarial cases
│   ├── mutator.cpp             # Dynamic bounded array/string/int manipulators
│   └── schema.cpp              # yaml-cpp ingestion
├── benchmarks/                 # Intentionally vulnerable targets to test the engine
│   ├── bad_quicksort.cpp       # Lomuto O(N^2) trap
│   ├── graph_stress.cpp        # Bellman-Ford dense graph trap
│   ├── hidden_quadratic.cpp    # Magic number O(N^2) trap
│   └── schemas/                # YAML schemas for the benchmarks
└── tests/                      # Automated CTest Suites
    ├── fixtures/               # Tiny YAML files for parser testing
    ├── helpers/                # Dummy binaries (sleep, crash, burn CPU)
    └── test_*.cpp              # Individual unit and integration tests
```

---

## 💻 Developer Commands

**1. Clean Build the Engine**
```bash
rm -rf build && cmake -S . -B build && cmake --build build -j
```

**2. Run the CTest Suite**
```bash
ctest --test-dir build -V
```

**3. Run a Fuzzing Campaign**
```bash
./build/hackforge run --target <binary> --schema <yaml> --iterations 1000 --timeout 2000 --seed 1337 --output artifacts/
```

**4. Replay a Specific Input**
```bash
./build/hackforge replay --target <binary> --input artifacts/best_case.txt
```

**5. Minimize a Discovered Exploit**
```bash
./build/hackforge minimize --target <binary> --schema <yaml> --input artifacts/best_case.txt --output artifacts/minimized.txt
```