cmake_minimum_required(VERSION 3.14)
project(HackForge CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

enable_testing()

include(FetchContent)
FetchContent_Declare(yaml-cpp GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git GIT_TAG yaml-cpp-0.9.0)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(yaml-cpp)

# Phase 1: Executor
add_library(hackforge_executor src/executor.cpp)
target_include_directories(hackforge_executor PUBLIC include)

# Phase 2-4: Core Engine
add_library(hackforge_core 
    src/schema.cpp src/ast.cpp src/generator.cpp src/mutator.cpp
    src/fitness.cpp src/corpus.cpp src/fuzzer.cpp src/minimizer.cpp src/cli.cpp
)
target_include_directories(hackforge_core PUBLIC include)
target_link_libraries(hackforge_core PRIVATE yaml-cpp hackforge_executor)

# CLI Binary
add_executable(hackforge src/main.cpp)
target_link_libraries(hackforge PRIVATE hackforge_core)

# Benchmarks
add_executable(bad_quicksort benchmarks/bad_quicksort.cpp)
add_executable(hidden_quadratic benchmarks/hidden_quadratic.cpp)
add_executable(graph_stress benchmarks/graph_stress.cpp)

# Test Helpers
add_executable(check_input tests/helpers/check_input.cpp)
add_executable(exit_nonzero tests/helpers/exit_nonzero.cpp)
add_executable(sleep_helper tests/helpers/sleep_helper.cpp)
add_executable(cpu_burn tests/helpers/cpu_burn.cpp)

# Tests
add_executable(test_executor tests/test_executor.cpp)
target_link_libraries(test_executor PRIVATE hackforge_executor)
add_test(NAME ExecutorTests COMMAND test_executor $<TARGET_FILE:check_input> $<TARGET_FILE:exit_nonzero> $<TARGET_FILE:sleep_helper> $<TARGET_FILE:cpu_burn>)

add_executable(test_schema tests/test_schema.cpp)
target_link_libraries(test_schema PRIVATE hackforge_core)
add_test(NAME SchemaTests COMMAND test_schema "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures")

add_executable(test_ast tests/test_ast.cpp)
target_link_libraries(test_ast PRIVATE hackforge_core)
add_test(NAME ASTTests COMMAND test_ast)

add_executable(test_generator tests/test_generator.cpp)
target_link_libraries(test_generator PRIVATE hackforge_core)
add_test(NAME GeneratorTests COMMAND test_generator)

add_executable(test_mutator tests/test_mutator.cpp)
target_link_libraries(test_mutator PRIVATE hackforge_core)
add_test(NAME MutatorTests COMMAND test_mutator)

add_executable(test_integration tests/test_integration.cpp)
target_link_libraries(test_integration PRIVATE hackforge_core)
add_test(NAME IntegrationTests COMMAND test_integration $<TARGET_FILE:sleep_helper> "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/schemas/quicksort.yaml")