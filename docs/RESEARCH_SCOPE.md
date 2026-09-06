# TRIFACT Research Scope and Decision Gates

[English](RESEARCH_SCOPE.md) | [한국어](RESEARCH_SCOPE_KR.md)

> **Status: Active research policy**
>
> Passing a gate permits the next research phase. It does not prove cryptographic security.

## 1. Purpose

TRIFACT investigates whether the Random 3-Uniform Hypergraph Factorization Recovery problem (R3HFR) can support an experimental post-quantum digital signature.

The project must answer the underlying-problem question before investing in a complete proof system or signature implementation:

> Is recovering any valid 1-factorization difficult on average for hypergraphs sampled by TRIFACT KeyGen?

The project ends with one of two useful outcomes:

1. a reproducible negative result explaining why the current R3HFR construction should be abandoned; or
2. a research prototype supported by explicit definitions, baseline cryptanalysis, tests, measurements, and clearly stated security gaps.

## 2. In Scope

The project includes:

- a precise definition of the R3HFR instance distribution;
- a precise witness relation and factor-label equivalence rule;
- canonical representations for hypergraphs, keys, and transcripts;
- a deterministic reference relation validator;
- a reproducible KeyGen implementation for research instances;
- SAT, exact-cover, peeling, local-swap, automorphism, and statistical attack baselines;
- empirical measurement of recovery cost and witness density;
- a concrete MPC-in-the-Head proof construction, only after R3HFR passes the viability gate;
- a reference `KeyGen`, `Sign`, and `Verify` interface, only after the proof protocol is fully specified;
- correctness, negative, mutation, and parser tests;
- reproducible size and runtime measurements;
- English canonical documentation with matching Korean translations.

## 3. Out of Scope

The following are not goals of the current project:

- production deployment or protection of real data;
- a NIST submission or any other standardization submission;
- claims of Category 1, 3, or 5 security before concrete cryptanalysis;
- claims that worst-case NP-hardness proves average-case security;
- invention of a second new zero-knowledge protocol when an established proof family can be instantiated;
- optimized SIMD, GPU, embedded, or hardware implementations;
- constant-time or side-channel-hardened production code;
- stable interoperability formats or backward-compatibility guarantees;
- formal certification, third-party audit, or implementation compliance claims;
- a claim that TRIFACT is the first construction of its kind without a complete prior-art review;
- license selection during the current documentation phase.

Items outside this scope may be considered later only after the underlying problem and proof design remain viable.

## 4. Required Work Order

Work proceeds in this order:

1. formalize the problem, witness equivalence, interfaces, encodings, and research parameters;
2. implement and test the mathematical reference core;
3. implement multiple independent recovery attacks;
4. measure attack scaling and record a viability decision;
5. instantiate the proof protocol only after a `GO` decision;
6. expose the signature API only after the proof specification and circuit agree;
7. publish a research evaluation with explicit limitations.

Proof or signature implementation must not be used to avoid, postpone, or hide an unresolved R3HFR result.

## 5. Gate A — Specification Ready

### Requirements

Before implementing the mathematical core, the documentation must define:

- the exact KeyGen sampling distribution;
- all validity and rejection conditions;
- labeled witnesses and equivalence under global factor-label permutations;
- the attacker's success condition;
- public-key and secret-key contents;
- the information available to `Sign`;
- canonical ordering and byte encoding rules;
- research-only parameter profiles and resource limits.

### Pass Condition

Two independent implementations following the documents should be able to produce compatible objects without inventing additional protocol rules.

### Failure Action

If any required rule is ambiguous or contradictory, implementation remains blocked and the specification is corrected first.

## 6. Gate B — Reference Core Ready

### Requirements

The reference core must demonstrate that:

- every successful KeyGen output is 3-uniform, simple, and `d`-regular;
- the number of edges is exactly `dn/3`;
- the generated witness passes the native relation validator;
- factor-label permutation preserves witness validity;
- canonical sorting preserves the edge-to-label mapping;
- fixed seeds reproduce identical research instances;
- malformed and non-canonical encodings are rejected without crashes.

### Pass Condition

All invariant, boundary, negative, and round-trip tests pass on every research profile.

### Failure Action

Attack measurements remain blocked until the reference core is corrected, because an invalid generator or validator would invalidate all later results.

## 7. Gate C — R3HFR Viability

This is the primary Go/No-Go gate.

### Required Evidence

The evaluation must include:

- at least three increasing parameter points for every reported scaling trend;
- at least 30 independently seeded instances per parameter point, unless a complete exhaustive result is available;
- the same saved instance set for all comparable solvers;
- SAT and at least one structurally different recovery method;
- enumeration of witness classes on tractable instances after quotienting global label permutations;
- tests for local factor swaps, automorphisms, and sampler-dependent statistics;
- recorded wall time, peak memory, timeout, success status, and recovered-witness validity;
- raw results, environment information, solver settings, commands, and seeds sufficient for reproduction.

Every recovered witness must be checked by the native relation validator. A solver-reported assignment that fails the relation is not a successful recovery.

Timeouts are recorded as censored observations, not as proof of hardness.

### Decision Outcomes

#### `GO`

Use `GO` only when:

- no direct polynomial-time recovery method has been found;
- independent attack families show increasing resource cost over the tested parameter grid;
- larger research instances reach the predefined resource limits rather than remaining uniformly easy;
- no tested statistical signal or local operation provides repeatable low-cost full recovery;
- alternative witness classes do not make recovery effectively trivial;
- all experiments are reproducible from stored inputs and commands.

A `GO` decision means only that the current construction did not immediately fail against the implemented baselines. It is not evidence of a security level.

#### `REDESIGN`

Use `REDESIGN` when the core relation remains potentially useful but a repairable design choice causes failure, including:

- leakage caused by a specific sampling rule;
- excessive witness density caused by a parameter choice;
- avoidable canonicalization or representation leakage;
- unacceptable key, circuit, or proof-size estimates that may be reduced without changing the core assumption.

The changed design must return to the earliest affected gate. Previous measurements cannot be reused as evidence for the modified distribution without rerunning them.

#### `NO-GO`

Use `NO-GO` when any of the following is found:

- a direct polynomial-time factorization algorithm for the KeyGen distribution;
- a repeatable structural attack whose cost remains low as parameters increase;
- a practical SAT, exact-cover, peeling, or combined attack with insufficient empirical growth;
- a statistical method that reliably exposes enough factor information to complete recovery cheaply;
- small local swaps or abundant equivalent witnesses that collapse the intended search cost;
- parameters grow public keys or proof costs faster than they grow attack cost;
- security would require hiding KeyGen, the instance distribution, or other public design details.

A `NO-GO` decision ends proof and signature development for the current R3HFR construction. The project records the negative result and reproduction procedure.

## 8. Gate D — Proof Protocol Ready

This gate is evaluated only after Gate C returns `GO`.

### Requirements

The proof specification must fix:

- the number of virtual MPC parties;
- the sharing method and computation domain;
- the exact relation circuit;
- party randomness and view contents;
- commitment inputs and hash/XOF choices;
- challenge space and derivation;
- the exact set of opened views;
- verifier consistency checks;
- per-repetition soundness error and total repetition count;
- salt and transcript binding;
- the boundaries of the Fiat-Shamir, QROM, proof-of-knowledge, and EUF-CMA arguments.

### Pass Condition

The native relation and proof circuit agree on all generated and mutated test inputs, and the document contains no protocol choice that must be invented by the implementation.

### Failure Action

The signature API remains blocked until the proof protocol is complete and internally consistent.

## 9. Gate E — Research Release Ready

### Requirements

- `KeyGen`, `Sign`, and `Verify` use the documented formats and transcript rules;
- deterministic test hooks produce reproducible known-answer vectors;
- wrong messages, wrong keys, truncation, extension, component substitution, and transcript mutation are rejected;
- parsers fail closed on malformed input and enforce explicit size limits;
- fuzz and mutation tests complete without crashes or uncontrolled allocation;
- public-key, secret-key, signature-size, KeyGen, Sign, and Verify measurements are published;
- the security document distinguishes tested properties, assumptions, open questions, and unsupported claims;
- English and Korean documents describe the same decisions.

### Pass Condition

All required tests and reproducibility checks pass, and the resulting artifact is clearly labeled as an experimental research prototype that must not protect real data.

## 10. Measurement Rules

All cryptanalytic measurements follow these rules:

- use immutable input sets identified by parameter profile and seed;
- use the same timeout and resource accounting for comparable solvers;
- record solver failures separately from timeouts and invalid recovered witnesses;
- preserve raw per-instance results rather than only averages;
- report median and tail behavior, not only the fastest result;
- align global factor labels before comparing a recovered witness with a planted witness;
- count factorization equivalence classes separately from raw labeled witnesses;
- document hardware, operating system, runtime, solver, and dependency information;
- rerun affected experiments after changes to the generator, relation, encoding, or solver model.

## 11. Change Control

- English documents are canonical; Korean translations use the `_KR` suffix.
- Security-relevant decisions must be recorded in the repository rather than only in commit messages or conversations.
- A change to KeyGen distribution, witness relation, canonical encoding, transcript binding, or proof challenge invalidates all results that depend on the changed rule.
- A gate may be reopened whenever new evidence contradicts its previous decision.
- A `GO` decision never prevents later abandonment.
- Licensing is intentionally deferred until the project has a clearer implementation and contribution structure.

## 12. Current Project State

The project is currently before Gate A.

The next required documents must formalize:

1. the R3HFR distribution and witness equivalence;
2. key-generation and signing interfaces;
3. canonical encodings and hash domains;
4. research parameter profiles and resource limits.

No proof implementation, signature implementation, or security-level claim is authorized by the current state.
