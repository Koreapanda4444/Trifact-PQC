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
- `clang-format` 18 for source formatting;
- compiler warnings treated as errors;
- GCC and Clang on Linux;
- MSVC on Windows;
- GitHub Actions for automated checks.

The initial toolchain has no third-party C library dependency. Solver libraries are added separately only when the corresponding analysis commit begins.

C was selected for the reference implementation to keep data representation, allocation, integer bounds, and canonical byte processing explicit. The project remains a research prototype; using C does not make the design suitable for production.

## Repository Layout

| Path | Purpose |
|---|---|
| `.github/workflows/ci.yml` | Formatting, strict builds, tests, sanitizers, and static analysis |
| `include/trifact/` | Public core, relation, incidence, witness, hash, entropy, research, and sampling APIs |
| `src/` | C17 implementation and private production heap wrapper |
| `tests/` | Deterministic tests, exhaustive oracle, invariants, and test allocator |
| `docs/` | English specifications and matching Korean documents |
| `analysis/` | Later solvers, experiments, and benchmark outputs |
| `CMakeLists.txt` | Build definition and optional diagnostic checks |
| `.clang-format` | Source formatting rules |

The codebase implements owning canonical hypergraphs and label vectors, the native R3HFR relation validator, validated vertex incidence indexes, and witness-label normalization and equivalence. [Core API](CORE_API.md) documents their ownership, errors, and eight core CTest targets. [Primitive and Randomness Providers](CRYPTO_PROVIDERS.md) documents the implemented SHAKE256, domain hashes, OS entropy, research streams, and sampling. [Cryptographic Test Vectors](CRYPTO_VECTORS.md) records independent expectations. There are 20 CTest targets on Linux and 19 on Windows; the additional Linux test substitutes `getrandom` only in its own executable. Windows links the native system `bcrypt` library. KeyGen, codecs, recovery solvers, proofs, and signatures remain later work.

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

## Sanitizers and Static Analysis

ASan and UBSan are optional and disabled in normal builds. On supported GCC or Clang toolchains, run:

```bash
cmake -S . -B build-sanitizers -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DTRIFACT_ENABLE_SANITIZERS=ON
cmake --build build-sanitizers --parallel
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ctest --test-dir build-sanitizers --output-on-failure
```

The sanitizer configuration checks compiler and linker support. Instrumentation and frame pointers propagate to the production core, test allocator build, and test executables. Unsupported requests fail configuration explicitly. CI uses Linux GCC and Clang with leak detection and stops on the first sanitizer diagnostic. Windows MSVC retains the normal strict build and tests.

GCC analysis requires a compiler supporting `-fanalyzer`; findings remain errors under the existing warning policy:

```bash
cmake -S . -B build-analysis -DCMAKE_C_COMPILER=gcc -DBUILD_TESTING=OFF -DTRIFACT_ENABLE_GCC_ANALYZER=ON
cmake --build build-analysis --parallel
```

Requesting this option with a different compiler or unsupported GCC fails configuration. Clang configurations expose a separate core-analysis target with text diagnostics and analyzer findings treated as errors:

```bash
cmake -S . -B build-clang-analysis -DCMAKE_C_COMPILER=clang -DBUILD_TESTING=OFF
cmake --build build-clang-analysis --target trifact-clang-analysis
```

Compiler analysis and sanitizer coverage are implementation checks, not evidence of cryptographic security. Production-only builds with `BUILD_TESTING=OFF` do not compile or link the fault-injection allocator.

## Formatting

Check the current C files with:

```bash
git ls-files -z -- '*.c' '*.h' | xargs -0 clang-format-18 --dry-run --Werror
```

Apply formatting with:

```bash
git ls-files -z -- '*.c' '*.h' | xargs -0 clang-format-18 -i
```

CI installs the selected formatter on Ubuntu 24.04 and checks every tracked C source and header, including private and test headers. Newly staged or committed C files automatically enter this manifest. Format new untracked files explicitly before staging them.

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

Warnings are not suppressed globally. A necessary local suppression must be justified in both development documents. Source files contain no code comments.

## CI Contract

CI performs eight independent checks:

1. source formatting on Linux;
2. build and test with GCC on Linux;
3. build and test with Clang on Linux;
4. build and test with MSVC on Windows;
5. ASan, UBSan, and leak checks with GCC on Linux;
6. ASan, UBSan, and leak checks with Clang on Linux;
7. GCC core static analysis;
8. Clang core static analysis.

All jobs must pass before an update is considered complete. CI has read-only repository-content permission and does not publish packages, generate keys, create releases, or modify the repository.

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

Dynamic allocation is added only where object lifetime and cleanup ownership are documented in the bilingual Core API or internal module contract.

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
- all registered CTest targets pass, including core and cryptographic oracles, retained vectors, entropy failures, and allocation-failure tests;
- GCC and Clang sanitizer builds pass with leak detection;
- GCC and Clang core static analysis reports no findings;
- no KeyGen, proof, or signing implementation is included prematurely.
