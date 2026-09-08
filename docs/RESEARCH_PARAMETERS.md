# TRIFACT Research Parameters and Resource Limits

[English](RESEARCH_PARAMETERS.md) | [한국어](RESEARCH_PARAMETERS_KR.md)

> **Status: Normative research profile specification**
>
> These profiles exist for correctness checks and attack experiments. They do not represent cryptographic security levels and must not be used to protect real data.

## 1. Purpose

The R3HFR distribution, key interfaces, and canonical encodings require concrete bounds before implementation begins. Unbounded sampling, parsing, or solver execution would make results irreproducible and could turn malformed input into uncontrolled resource consumption.

This specification defines:

- the registered research parameter identifiers;
- concrete `(n,d,m)` values;
- the purpose of each profile;
- KeyGen attempt limits;
- key and message size limits;
- parser-wide safety limits;
- recovery-solver time and memory limits;
- a reproducible benchmark seed set;
- rules for recording failure, timeout, and censored results.

## 2. Non-Security Naming

The only registered parameter identifiers are:

```text
toy
small
medium
```

These names describe experiment scale only. They do not correspond to bit security, NIST categories, classical work factors, quantum work factors, or deployment recommendations.

Implementations and reports must not rename these profiles to terms such as `secure`, `recommended`, `128`, `192`, or `256`.

If every profile is easy to solve, the correct result is that the current design fails its viability gate. The profiles must not be enlarged repeatedly until a solver happens to time out.

## 3. Common Mathematical Requirements

Every registered profile satisfies:

```math
n\ge6,
\qquad
n\equiv0\pmod3,
\qquad
2\le d\le\binom{n-1}{2},
\qquad
m=\frac{dn}{3}.
```

All vertices and labels fit `U32`. Every successful public key is a simple `d`-regular 3-uniform hypergraph on `V=[n]` with exactly `m` canonical edges.

These syntactic conditions do not imply that the selected distribution is difficult to factor.

## 4. Profile Registry

| Parameter ID | `n` | `d` | `m = dn/3` | Primary purpose |
| --- | ---: | ---: | ---: | --- |
| `toy` | 12 | 3 | 12 | Invariants, mutation tests, complete witness-class enumeration |
| `small` | 24 | 4 | 32 | Solver development, cross-checking, and statistical smoke tests |
| `medium` | 60 | 6 | 120 | Largest initial recovery and scaling experiments |

The registry is exact. A parameter identifier resolves to one row and cannot override individual fields at runtime.

Custom `(n,d)` values may be accepted by isolated exploratory tools, but they are not registered parameters, cannot be encoded as project keys, and cannot be mixed into the official benchmark summary.

## 5. Intended Use of `toy`

The `toy` profile is the smallest common correctness target.

Required uses include:

- KeyGen and codec golden tests;
- relation-validator positive and negative cases;
- canonical edge-label remapping tests;
- factor-label permutation tests;
- SAT and exact-cover solver bring-up;
- complete enumeration of labeled witnesses when feasible;
- quotienting witness counts by global label permutation;
- local-swap and automorphism test fixtures.

A fast recovery on `toy` is expected and carries no security conclusion. A solver that cannot recover `toy` instances is not ready for larger-profile measurements.

## 6. Intended Use of `small`

The `small` profile is the integration target for independent attack implementations.

Required uses include:

- comparison of SAT, exact-cover, and peeling outputs;
- validation that every recovered witness passes the native relation;
- seed reproducibility across implementations;
- malformed-input and parser resource tests;
- preliminary factor-swap and automorphism measurements;
- statistical feature collection before the largest initial runs.

Complete enumeration is optional and may be attempted only under the separate enumeration limit. A timeout is recorded, not converted into an assumed witness count.

## 7. Intended Use of `medium`

The `medium` profile is the largest initial profile used before the R3HFR viability decision.

Required uses include:

- recovery-cost comparison against `toy` and `small`;
- wall-time and peak-memory measurement;
- full baseline solver runs on the fixed seed set;
- planted-distribution statistical tests;
- detection of scaling that remains flat or grows too slowly.

`medium` is not a candidate deployment parameter. A timeout on `medium` is a censored measurement and not evidence of hardness.

## 8. Parameter Descriptor

Resolving a registered identifier returns:

```text
ParameterDescriptor {
    parameter_id
    n
    d
    m
    keygen_attempt_limit
    message_size_limit
    public_key_size
    secret_key_size
    signature_input_limit
    recovery_timeout_seconds
    recovery_memory_limit_mib
    enumeration_timeout_seconds
    enumeration_memory_limit_mib
    protocol_configuration
}
```

The descriptor is immutable. `m` is stored for convenience but must be recomputed with checked arithmetic and compared to `dn/3` when the registry is loaded.

For all current profiles:

```text
protocol_configuration = unavailable
```

Therefore `Sign` returns `IncompleteProtocolConfiguration`, and `Verify` cannot accept a protocol body. This restriction remains until the viability gate permits proof-layer work and the proof specification supplies a complete configuration.

## 9. KeyGen Attempt Limit

Every current profile uses:

```text
keygen_attempt_limit = 256
```

Attempts are indexed `0` through `255`. Each attempt uses the independent `TRIFACT/keygen-attempt` stream defined by the encoding specification.

An attempt is rejected only when the complete factor tuple contains a duplicate unordered edge. Internal invariant failures are errors, not ordinary rejections.

If attempt `255` is rejected, KeyGen returns `SamplingExhausted`. It must not start attempt `256`, change the profile, reduce `d`, or retain a partial tuple.

The implementation records the number of attempts used for research diagnostics. That count is not part of `pk` or `sk`.

## 10. Expected Rejection Diagnostic

For two independently sampled uniform 1-factors on `n` vertices, the expected number of common edges is:

```math
\frac{2n}{3(n-1)(n-2)}.
```

Across `d` independently sampled factors, the expected number of pairwise edge collisions before conditioning is:

```math
\binom d2\frac{2n}{3(n-1)(n-2)}.
```

This value is a diagnostic expectation, not an exact rejection probability because multiple collision events are dependent.

Implementations must report observed attempts per successful key. A large disagreement between implementations using identical seeds indicates a sampling or canonicalization bug and blocks later benchmarks.

## 11. Exact Key Sizes

Under the canonical encoding specification, the current profiles have the following exact key sizes:

| Parameter ID | Encoded public key | Encoded secret key |
| --- | ---: | ---: |
| `toy` | 185 bytes | 270 bytes |
| `small` | 427 bytes | 592 bytes |
| `medium` | 1,484 bytes | 2,001 bytes |

The values include record names, field framing, the embedded public key in the secret key, edge counts, and label counts.

An encoded key with any other length is rejected before full object construction. Exact length does not replace structural validation.

## 12. Key-Size Derivation

For `m` edges, the edge-list payload length is:

```math
4+12m.
```

For `m` labels, the label-vector payload length is:

```math
4+4m.
```

Using the canonical record framing, the public-key sizes are:

```text
toy:     41 + 12m = 185
small:   43 + 12m = 427
medium:  44 + 12m = 1,484
```

The constant differs because the parameter identifier length differs.

The secret-key size is:

```math
37+|EncodePublicKey(pk)|+4m.
```

These equations are mandatory codec test assertions.

## 13. Message and Signature Input Limits

Every current profile uses:

```text
message_size_limit  = 1,048,576 bytes
signature_input_limit = 67,108,864 bytes
```

The message limit is an API and parser bound, not a recommended application message size.

The signature limit is only a hard outer allocation cap reserved for later proof experiments. It is not an expected signature size and does not authorize a signature implementation before the proof configuration exists.

An input exceeding either limit is rejected before hashing the complete object or allocating a buffer of the declared size.

## 14. General Parser Limits

All key, signature, and transcript parsers apply these limits in addition to profile-specific exact sizes:

| Resource | Limit |
| --- | ---: |
| Single `Blob` payload | 67,108,864 bytes |
| Elements in one `Sequence` | 1,048,576 |
| Fields in one `Record` | 64 |
| Nested record depth | 8 |
| Restricted-name length | 32 bytes |
| Total signature input | 67,108,864 bytes |
| Diagnostic solver output captured per instance | 16,777,216 bytes |

The smaller applicable limit always wins. For example, the public-key parser uses the exact profile key size rather than the general `Blob` limit.

Length and count arithmetic uses checked operations. A parser rejects overflow, multiplication overflow, impossible nesting, and over-limit declarations before allocation.

## 15. Recovery Solver Limits

The standard single-instance recovery limits are:

| Parameter ID | Wall-clock timeout | Peak memory limit | Reference thread count |
| --- | ---: | ---: | ---: |
| `toy` | 10 seconds | 512 MiB | 1 |
| `small` | 60 seconds | 1,024 MiB | 1 |
| `medium` | 300 seconds | 2,048 MiB | 1 |

The timeout starts immediately before the solver receives the public instance and ends when a complete candidate witness or terminal status is returned.

Instance loading, native witness validation, and result serialization are timed separately. Reports include both solver time and end-to-end time.

A solver that uses additional threads must be reported as a separate configuration. Results from different thread counts are not combined into one scaling curve.

## 16. Enumeration Limits

Witness-class enumeration uses separate limits:

| Parameter ID | Enumeration status | Wall-clock timeout | Peak memory limit |
| --- | --- | ---: | ---: |
| `toy` | Required when feasible | 300 seconds | 2,048 MiB |
| `small` | Optional exploratory run | 900 seconds | 4,096 MiB |
| `medium` | Disabled in the initial grid | — | — |

An enumerator must distinguish:

- complete enumeration;
- timeout;
- memory limit;
- solver failure;
- invalid emitted witness;
- externally interrupted run.

Only complete enumeration yields an exact witness-class count. Partial counts are lower bounds and must be labeled as such.

## 17. Fixed Benchmark Instance Set

Every official initial benchmark uses exactly 30 generated instances per profile.

For index `i` in `[0,29]`, the 32-byte benchmark seed is:

```text
BenchmarkSeed(i) = 28 zero bytes || U32(i)
```

The instance identifiers are:

```text
toy-000     through toy-029
small-000   through small-029
medium-000  through medium-029
```

For each profile and index, a fixed-seed `RandomSource` returns `BenchmarkSeed(i)` for the single 32-byte request made by KeyGen. The canonical KeyGen expansion then determines all attempts and permutations.

These public deterministic seeds exist only for reproducible attack research. They must not be used to generate a secret key intended to remain secret.

## 18. Separation of Generator and Solver Inputs

The benchmark generator may retain:

- the profile identifier;
- benchmark index and seed;
- public key;
- planted label vector;
- attempt count and rejection diagnostics.

The recovery solver receives only:

- the profile identifier already contained in `pk`;
- the canonical public key bytes;
- its declared resource configuration.

The solver must not receive the seed, planted label vector, attempt stream, generation order, or rejection trace. Publishing those values for reproducibility does not make them valid solver inputs.

Recovered output succeeds only when the native relation validator accepts it. Equality with the planted witness is optional diagnostic information and is not the success condition.

## 19. Benchmark Run Rules

For a comparable solver run:

1. generate or load the fixed 30-instance set;
2. verify every public key and planted witness before timing attacks;
3. run every compared solver on the same public instances;
4. apply the profile's timeout, memory, and thread limits;
5. validate every claimed recovery with the native relation validator;
6. preserve raw per-instance results before aggregation;
7. report environment, command, solver settings, and source commit;
8. never delete hard instances or replace exhausted KeyGen seeds silently.

Solver order should be rotated or randomized independently of the instance seed when system load could bias results. The chosen schedule is recorded.

## 20. Required Per-Instance Result

Every recovery result records at least:

```text
instance_id
parameter_id
n
d
m
solver_name
solver_configuration
thread_count
wall_time
end_to_end_time
peak_memory
status
candidate_witness_present
candidate_witness_valid
recovered_class_id_if_available
environment_id
```

`status` is exactly one of:

```text
success
unsolved
timeout
memory-limit
solver-error
invalid-witness
interrupted
```

An invalid witness is never converted to `unsolved` or `success`. Missing measurements are represented explicitly rather than as zero.

## 21. Timeout and Censoring Rules

A timeout means only that a configured run did not finish within its wall-clock limit.

Reports must:

- count timeouts separately from solver errors;
- treat timeout runtimes as right-censored observations;
- avoid reporting the timeout value as the actual solve time;
- show success rate together with runtime summaries;
- avoid averaging successful runs alone without stating that selection;
- keep the configured limit unchanged across comparable instances.

Increasing a timeout after inspecting one difficult seed creates a new solver configuration and must be reported separately.

## 22. KeyGen Failure Rules in Datasets

If a fixed benchmark seed reaches `SamplingExhausted`, the generator records a failed dataset construction for that exact instance identifier.

It must not:

- increment the seed until KeyGen succeeds;
- substitute a random seed;
- change `d` or the attempt limit;
- omit the instance from the denominator;
- reuse an instance from another profile.

Dataset generation remains blocked until the cause is understood and the specification is deliberately changed.

## 23. Statistical Experiment Limits

Statistics collected from the planted distribution use the same 30 public instances as recovery experiments unless a study declares a separate sample set in advance.

Every statistic records:

- the exact profile and instance identifiers;
- whether it uses public data only;
- any comparison distribution and its sampler;
- random seeds used by the analysis itself;
- correction for multiple tested statistics when applicable;
- raw values before summary or plotting.

A statistic that uses the planted labels is an explanatory diagnostic, not a public-key-only distinguisher.

## 24. Expansion Policy

The initial grid may be expanded only after the three current profiles have complete results or a documented blocker.

A new profile requires:

- a new registered lowercase identifier;
- exact `n`, `d`, and derived `m`;
- a stated experimental purpose;
- KeyGen rejection measurements;
- exact encoded key sizes;
- parser, timeout, memory, and enumeration limits;
- an independent 30-instance seed namespace;
- updates to both language documents and all registry tests.

New profiles do not retroactively change old benchmark results. Custom exploratory parameters remain outside the official grid.

## 25. Gate Interpretation

The initial profile grid is sufficient to detect immediate collapse, implementation errors, flat scaling, obvious statistical leakage, and trivial witness multiplicity.

It is not sufficient to establish cryptographic security. In particular:

- solving all three profiles cheaply is strong evidence for `NO-GO` or `REDESIGN`;
- a solver timeout is not evidence for `GO` by itself;
- a `GO` decision requires agreement across independent attack families and all previously defined gate evidence;
- larger experiments may be required when the three-point trend is inconclusive;
- no proof or signature implementation begins before the viability decision permits it.

## 26. Required Parameter Tests

Implementations must test at least:

- registry lookup accepts exactly `toy`, `small`, and `medium`;
- unknown and modified-case identifiers are rejected;
- every row satisfies the mathematical requirements;
- checked recomputation of `m` equals the stored value;
- exact encoded public-key and secret-key sizes match the table;
- attempt indices stop at `255`;
- attempt exhaustion returns `SamplingExhausted`;
- the same benchmark seed reproduces the same key pair and attempt count;
- different benchmark indices produce different attempt streams;
- public solvers receive no seed or planted witness field;
- parser limits are applied before allocation;
- timeout, memory-limit, solver-error, and invalid-witness statuses remain distinct;
- unavailable protocol configuration blocks signing on every profile.

## 27. Normative Summary

The initial research grid is:

```text
toy     n=12  d=3  m=12
small   n=24  d=4  m=32
medium  n=60  d=6  m=120
```

Every profile uses at most 256 KeyGen attempts, messages of at most 1 MiB, and the fixed 30-instance benchmark seed set. Recovery runs use profile-specific time and memory limits and treat timeouts as censored results.

All proof configurations remain unavailable. These profiles support R3HFR research only and make no security claim.
