# TRIFACT

[English](README.md) | [한국어](README_KR.md)

> **Experimental and unproven research project. Not for production use.**

TRIFACT explores a proposed post-quantum digital signature design based on the Random 3-Uniform Hypergraph Factorization Recovery problem (R3HFR).

The project is currently in the design and feasibility-validation stage. It does not claim post-quantum security, EUF-CMA security, standardized security levels, or suitability for protecting real data.

## Research direction

- Formalize the R3HFR instance distribution and witness relation.
- Build a small, reproducible reference model.
- Evaluate SAT, exact-cover, peeling, symmetry, and structural recovery attacks.
- Continue to the proof and signature layers only if the underlying problem survives the defined viability gate.

## Repository layout

- `docs/` — design specifications, decisions, and evaluation reports
- `analysis/` — R3HFR solvers, experiments, and benchmarks
- `include/` — public C headers
- `src/` — C17 reference implementation
- `tests/` — correctness, negative, and regression tests

## Development

The prototype uses C17 and CMake with automated GCC, Clang, and MSVC builds. See [Development Environment](docs/DEVELOPMENT.md).

## Documentation

English documents are canonical. Korean translations are provided in matching files with the `_KR` suffix.

## Security

TRIFACT is research software built around a new and unproven computational assumption. Do not use it in production or rely on it for security.
