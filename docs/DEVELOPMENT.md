# TRIFACT Development Environment

[English](DEVELOPMENT.md) | [한국어](DEVELOPMENT_KR.md)

> **Status: Active development setup**
>
> The current codebase is a research prototype. It does not implement a signature scheme and must not be used for security.

## Toolchain

The reference implementation uses:

- C17 without compiler-specific language extensions;
- CMake 3.24 or later for configuration and builds;
- CTest for test execution;
- `clang-format` for source formatting;
- compiler warnings treated as errors;
- GCC and Clang on Linux;
- MSVC on Windows;
- GitHub Actions for automated checks.

The initial toolchain has no third-party C library dependency. Solver libraries are added separately only when the corresponding analysis commit begins.

C was selected for the reference implementation to keep data representation, allocation, integer bounds, and canonical byte processing explicit. The project remains a research prototype; using C does not make the design suitable for production.

## Repository Layout

```text
.github/workflows/ci.yml  formatting, build, and test jobs
include/trifact/          public C headers
src/                      reference implementation
tests/                    CTest executables
docs/                     English specifications and Korean counterparts
analysis/                 later solvers, experiments, and benchmark outputs
CMakeLists.txt            build definition
.clang-format             source formatting rules
```

This commit contains only a buildable library skeleton and a toolchain smoke test. Hypergraph types and algorithm behavior begin in the following commits.

## Windows Setup

Required tools:

- Visual Studio Build Tools with the Desktop development with C++ workload;
- CMake available from Command Prompt or PowerShell.

Although the workload name contains C++, TRIFACT configures the project with `LANGUAGES C` and compiles C17 source files.

From the repository root:

```powershell
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

The default Visual Studio generator selects MSVC. Generated files remain under `build/`.

## Linux Setup

With GCC:

```bash
cmake -S . -B build -DCMAKE_C_COMPILER=gcc -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

With Clang:

```bash
cmake -S . -B build-clang -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build-clang --parallel
ctest --test-dir build-clang --output-on-failure
```

## Formatting

Check the current C files with:

```bash
clang-format --dry-run --Werror include/trifact/trifact.h src/trifact.c tests/test_toolchain.c
```

Apply formatting with:

```bash
clang-format -i include/trifact/trifact.h src/trifact.c tests/test_toolchain.c
```

New C headers and source files must be added to the CI formatting command in the same commit that creates them. A later tooling commit may replace the explicit list with a checked manifest.

## Compiler Checks

GCC and Clang builds enable:

```text
-Wall
-Wextra
-Wpedantic
-Werror
-Wconversion
-Wsign-conversion
-Wshadow
-Wstrict-prototypes
```

MSVC builds enable:

```text
/W4
/WX
```

Warnings are not suppressed globally. A necessary local suppression requires a comment explaining the exact compiler diagnostic and why the code remains correct.

## CI Contract

CI performs four independent checks:

1. source formatting on Linux;
2. build and test with GCC on Linux;
3. build and test with Clang on Linux;
4. build and test with MSVC on Windows.

All jobs must pass before continuing. CI has read-only repository-content permission and does not publish packages, generate keys, create releases, or modify the repository.

## Test Rules

Every implementation commit must add or update tests for its behavior. Tests must be:

- deterministic;
- independent of network access;
- explicit about expected failures;
- free of real secret material;
- valid under all three compilers;
- free of undefined behavior;
- safe under the parameter and allocation limits.

Tests return a nonzero process status on failure so CTest can report the failed executable.

## Integer and Memory Rules

The implementation uses fixed-width integer types from `<stdint.h>` for encoded protocol values. `size_t` is used for in-memory sizes and indices after checked conversion.

Every calculation involving counts, byte lengths, products, or allocation sizes must check overflow before allocation. Parser input is not trusted. Partial output objects must be cleaned up on failure.

Dynamic allocation is added only where object lifetime and cleanup ownership are documented in the public header or internal module contract.

## Dependency Rules

New dependencies require a specific research need.

Before adding one:

1. identify the module that requires it;
2. confirm that the C standard library is insufficient;
3. define how CMake locates the dependency;
4. record the accepted dependency release range;
5. add tests for missing and present dependency cases;
6. record native-library and solver settings required for reproduction.

SAT or exact-cover dependencies belong to analysis targets and must not leak into the mathematical core's public interface.

## Generated Files

Do not commit build directories, CMake state, compiler objects, libraries, executables, debug databases, editor state, or log files.

Raw benchmark data is not a disposable build artifact. Later analysis specifications determine which inputs and results must be committed for reproduction.

## Current Completion Condition

This setup is complete when:

- the static library compiles as C17;
- the toolchain smoke test passes through CTest;
- the source satisfies the formatting rule;
- GCC, Clang, and MSVC builds treat warnings as errors;
- CI runs on Linux and Windows;
- no hypergraph, KeyGen, proof, or signing algorithm is included prematurely.
