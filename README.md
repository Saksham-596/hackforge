# HackForge

HackForge is a C++20 adversarial test generation and stress-testing engine for
competitive-programming solutions. Given a target executable and a small input
schema, it generates valid tests, mutates promising cases, executes the target,
measures runtime/resources, and evolves toward pathological inputs.

Normal random testing often misses worst-case competitive-programming inputs
because the bad cases are structured: sorted arrays for poor quicksort pivots,
long repeated strings for hidden quadratic scans, dense graphs for O(VE)
algorithms, and so on. HackForge keeps the input structure intact while it
searches, so mutations stay valid instead of becoming random bytes.

## Status

This repository contains a working backend MVP:

- C++20 core engine with CMake build files.
- Dependency-free YAML-subset schema parser.
- Structural input AST and stdin serialization.
- Schema-aware generators for integers, arrays, strings, and permutations.
- Schema-preserving mutators for boundaries, sorted/reversed arrays, long runs,
  alternating extremes, string patterns, and permutation perturbations.
- POSIX executor using `fork`, `execv`, pipes, `wait4`, timeout kill, CPU timing,
  exit/signal reporting, and `getrusage` memory reporting where available.
- Fitness, ranked corpus, deterministic seeding, artifact persistence.
- CLI commands: `run`, `replay`, `minimize`, and `inspect`.
- Benchmarks for bad quicksort, hidden quadratic string scanning, and graph
  stress.
- Lightweight automated tests covering the core MVP.

HackForge is a stress-testing executor, not a secure sandbox. Do not run
untrusted programs with it expecting isolation.

## Architecture

```text
Schema
  -> Generator
  -> InputCase AST
  -> Mutator
  -> Executor
  -> Measurement
  -> Fitness
  -> Corpus
  -> Evolutionary Search
  -> Artifacts / Replay / Minimization
```

Main modules live under `include/hackforge/` and `src/`:

```text
schema      YAML-subset schema parsing and reference bounds
ast         structural input values, serialization, validation, repair
generator   CP-shaped seed generation
mutator     schema-preserving mutations
executor    POSIX process execution and measurement
fitness     runtime/resource/crash scoring
corpus      ranking, deduplication, persistence
fuzzer      evolutionary search loop
minimizer   basic structure-aware testcase reduction
cli         run/replay/minimize/inspect commands
```

## Build

Primary build:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

This development shell did not have `cmake` installed, so the repository also
includes a fallback build script that uses the system C++ compiler directly:

```bash
scripts/build_local.sh
./build/hackforge_tests ./build/benchmarks/targets/bad_quicksort
```

Verified locally:

```text
Built HackForge in /Users/sakshampal/hackforge-1/build
All HackForge tests passed
```

## Quick Start

Inspect a schema:

```bash
./build/hackforge inspect --schema benchmarks/schemas/quicksort.yaml
```

Run a search campaign:

```bash
./build/hackforge run \
  --target ./build/benchmarks/targets/bad_quicksort \
  --schema benchmarks/schemas/quicksort.yaml \
  --iterations 300 \
  --timeout 20 \
  --seed 123 \
  --output artifacts/quicksort_tle_demo
```

Replay the best saved case:

```bash
./build/hackforge replay \
  --target ./build/benchmarks/targets/bad_quicksort \
  --input artifacts/quicksort_tle_demo/best_case.txt \
  --timeout 20
```

Minimize it while preserving the timeout:

```bash
./build/hackforge minimize \
  --target ./build/benchmarks/targets/bad_quicksort \
  --schema benchmarks/schemas/quicksort.yaml \
  --input artifacts/quicksort_tle_demo/best_case.txt \
  --timeout 20 \
  --output artifacts/quicksort_tle_demo/minimized_case.txt
```

## Schema Example

```yaml
name: bad_quicksort

variables:
  - name: n
    type: int
    min: 1
    max: 7000

  - name: a
    type: array<int>
    length: n
    min: -1000000
    max: 1000000
```

Supported MVP field types:

- `int`
- `array<int>`
- `string`
- `permutation`

References such as `length: n` and `max: n` are resolved from earlier integer
fields. If a mutation changes `n`, dependent fields are repaired to remain
valid.

## Benchmarks

Benchmark targets are under `benchmarks/targets/` with schemas under
`benchmarks/schemas/`.

Measured on the local macOS development environment in this repository:

```text
bad_quicksort, 300 iterations, timeout 20 ms, seed 123
Executions:   300
Exec/sec:     112
Best runtime: 20.595 ms
Best verdict: TLE
Saved:        artifacts/quicksort_tle_demo/best_case.txt
```

Replay of the saved quicksort case:

```text
Verdict:      TLE
Runtime:      20.671 ms
CPU time:     20.027 ms
Max RSS:      1.6 MB
Signal:       9
```

Structured minimization of that case:

```text
Baseline:     20.896 ms (TLE)
Minimized:    20.461 ms (TLE)
Attempts:     120
Changed:      yes
```

Additional benchmark sanity runs:

```text
hidden_quadratic, 150 iterations, timeout 50 ms, seed 77
Executions:   150
Exec/sec:     115
Best runtime: 51.501 ms
Best verdict: TLE

graph_stress, 100 iterations, timeout 50 ms, seed 88
Executions:   100
Exec/sec:     56
Best runtime: 51.291 ms
Best verdict: TLE
```

Timing varies by machine and load. These numbers are examples from one local
run, not hard-coded test assertions.

## Determinism

The generator uses `std::mt19937_64` and `--seed`. Two one-iteration CLI runs
with `--seed 4242` produced byte-identical `best_case.txt` files:

```text
cmp artifacts/determinism_a/best_case.txt artifacts/determinism_b/best_case.txt
exit code: 0
```

Full multi-iteration campaigns can still differ slightly in ranking because
runtime measurements contain normal OS scheduling noise.

## macOS and Linux Notes

The MVP uses portable POSIX primitives and works on macOS:

- wall-clock timing via `std::chrono`
- CPU time and max RSS via `wait4`/`getrusage`
- timeout enforcement by killing the child process
- optional address-space memory limit when `RLIMIT_AS` is supported

Linux-specific enhancements such as `perf_event_open`, stronger resource
controls, and hardware performance counters are intentionally not mandatory.
HackForge never fakes unavailable hardware measurements.

## Repository Structure

```text
.
├── CMakeLists.txt
├── README.md
├── include/hackforge/
├── src/
├── tests/
├── benchmarks/
│   ├── targets/
│   └── schemas/
├── scripts/
├── docs/
├── examples/
└── artifacts/        # generated, ignored by git
```

## Limitations

- The schema language is intentionally small; it is not a constraint solver.
- The executor is not a security sandbox.
- The minimizer is basic and currently strongest for array/permutation inputs.
- Memory reporting depends on what the host OS exposes through `getrusage`.
- The search strategy is a simple evolutionary/hill-climbing loop, not a
  research-grade genetic algorithm.

## Future Work

- Optional Linux `perf_event_open` instruction counters.
- Richer schema constraints for graphs and paired records.
- More mutation operators and corpus import/export.
- Coverage-guided or differential-testing modes.
- Frontend/API layer after the backend stabilizes.
