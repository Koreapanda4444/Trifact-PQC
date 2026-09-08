# TRIFACT Key Generation and Signing Interfaces

[English](KEY_AND_SIGNING_INTERFACES.md) | [한국어](KEY_AND_SIGNING_INTERFACES_KR.md)

> **Status: Normative interface specification**
>
> This document fixes the logical objects and information flow between key generation, signing, and verification. It does not instantiate the proof system or define byte encodings.

## 1. Purpose

The design draft represented the secret key only as a factor-label vector `c` while exposing the signing interface as `Sign(sk, M)`. That interface was incomplete: the signer also needs the public hypergraph, its canonical edge indices, the parameter identifier, and fresh randomness.

This specification resolves that gap by defining:

- the logical contents of public and secret keys;
- the parameter lookup contract;
- explicit randomness inputs;
- bounded KeyGen failure behavior;
- the mapping from generated factor labels to canonical edge indices;
- the information available to the signer;
- the success and failure behavior of `KeyGen`, `Sign`, and `Verify`.

## 2. Scope Boundary

This document defines logical values and interfaces. It intentionally does not define:

- byte encodings or field widths;
- hash or XOF choices and domain strings;
- concrete research parameter values;
- the proof system, transcript, challenge, or signature fields;
- production hardening or external API bindings.

Those items belong to later specifications. No signature implementation may invent them from this document.

## 3. Common Types

For a resolved parameter identifier, let:

```math
V=[n]=\{0,\ldots,n-1\},
\qquad
m=\frac{dn}{3}.
```

The logical types are:

```text
ParameterId   opaque registered identifier
Vertex        integer in [0, n)
Edge          ordered triple (a, b, c) with a < b < c
EdgeList      lexicographically increasing array of m unique edges
FactorLabel   integer in [0, d)
LabelVector   array of m factor labels
Message       finite byte string
Signature     opaque protocol object defined by the proof specification
```

The public and secret key objects are:

```text
PublicKey {
    parameter_id: ParameterId,
    edges: EdgeList
}

SecretKey {
    public_key: PublicKey,
    labels: LabelVector
}
```

The vertex set is implicit from the parameter descriptor and is not stored as a second list.

## 4. Parameter Resolution

Every operation begins by resolving `parameter_id` through one common registry:

```text
ResolveParameters(parameter_id) -> Result<ParameterDescriptor, UnknownParameter>
```

The descriptor supplies every limit needed by that operation, including at least:

```text
ParameterDescriptor {
    n
    d
    keygen_attempt_limit
    message_size_limit
    protocol_configuration   // unavailable until the proof layer is fixed
}
```

The research-parameter specification assigns the identifiers and concrete limits. An identifier is a configuration selector, not a claim of cryptographic strength.

The rules are:

- `KeyGen` receives a parameter identifier directly.
- `Sign` obtains the identifier from `sk.public_key`.
- `Verify` obtains the identifier from `pk`.
- an unknown, disabled, or incomplete parameter descriptor causes failure;
- an implementation must not substitute a default descriptor.

## 5. Randomness Interface

Key generation and signing receive randomness explicitly:

```text
RandomSource.read(length) -> Result<byte[length], RandomnessFailure>
```

The source must return fresh bytes or an explicit error. It must not return a short value, repeat a previous buffer silently, or fall back to a predictable source.

Two logical randomness roles are kept separate:

```text
keygen_rng   sampling factors and the vertex permutation
sign_rng     generating the salt and all proof randomness
```

The exact deterministic expansion and domain separation are defined with the encoding and hash domains. Until then, implementations must not assume that raw bytes can be reused across the two roles.

Research tools may inject a deterministic source for reproducible tests. A deterministic seed is sensitive because it can reproduce a secret key. The seed is not part of `pk` or `sk` and must not be published with a key intended to remain secret.

## 6. Public-Key Object

The logical public key is:

```math
pk=(\mathsf{parameter\_id},E_{\mathrm{can}}),
```

where `E_can` is the canonical edge list from the R3HFR specification.

A structurally valid public key must satisfy all of the following:

1. the parameter identifier resolves successfully;
2. the resolved `n` is divisible by three and the resolved `(n,d)` is in the allowed domain;
3. every edge contains exactly three distinct vertices in `[n]`;
4. every edge is internally sorted;
5. the edge array is strictly lexicographically increasing;
6. the array contains exactly `m = dn/3` edges;
7. every vertex has degree exactly `d`.

Public-key validation does not attempt to find a factorization. Deciding whether an arbitrary structurally valid key has a witness is the recovery problem itself. Honest `KeyGen` guarantees that its output has a witness.

## 7. Secret-Key Object

The logical secret key is:

```math
sk=(pk,c),
```

not merely `c`.

The embedded public key gives the signer:

- the exact parameter identifier;
- the canonical public hypergraph;
- the edge count and canonical edge indices;
- the public input that must be bound into the proof transcript.

The label vector satisfies:

```math
|c|=|E_{\mathrm{can}}|=m
```

and `c_i` labels the public edge at canonical index `i`.

A valid secret key must satisfy:

```math
R_{\mathrm{R3HFR}}(pk.edges,c)=1.
```

Embedding `pk` duplicates public information in stored key material, but it removes an ambiguous external dependency and makes `Sign(sk, M, rng)` self-contained.

## 8. Key-Pair Consistency

For every successful KeyGen result `(pk, sk)`:

```text
sk.public_key == pk
```

is exact logical equality, and:

```math
R_{\mathrm{R3HFR}}(pk.edges,sk.labels)=1.
```

An API that imports a public key and secret key separately must compare the complete public-key objects before using them together. Comparing only `n`, `d`, an edge count, or an external filename is insufficient.

The helper:

```text
PublicKeyFromSecretKey(sk) -> pk
```

returns the embedded public key without regenerating or modifying it.

## 9. Canonical Edge-to-Label Remapping

KeyGen initially constructs labeled pairs before the public edge list is sorted:

```text
(generated_edge, factor_label)
```

For factor `j`, every generated edge from that factor is paired with label `j`. After applying the vertex permutation, each pair is transformed as follows:

```text
remapped_pairs = []

for each generated pair (edge, label):
    transformed = sort_vertices(apply_vertex_permutation(edge))
    remapped_pairs.append((transformed, label))

sort remapped_pairs by transformed edge

for i from 0 to m - 1:
    E_can[i] = remapped_pairs[i].edge
    c[i]     = remapped_pairs[i].label
```

The edge and its label move as one pair. Sorting the edges and leaving the label array in generation order is invalid.

Duplicate transformed edges cannot occur after a candidate has passed duplicate-edge rejection, because the vertex permutation is bijective. If a duplicate is nevertheless detected at this stage, the implementation reports an internal invariant failure rather than silently repairing the candidate.

The factor labels themselves are not renamed during this remapping. Global label normalization such as `CanonLabel(c)` is an analysis operation, not part of KeyGen.

## 10. KeyGen Interface

The interface is:

```text
KeyGen(parameter_id, keygen_rng) -> Result<KeyPair, KeyGenError>

KeyPair {
    public_key: PublicKey,
    secret_key: SecretKey
}
```

Its logical procedure is:

1. resolve the parameter identifier;
2. validate that the descriptor is complete for KeyGen;
3. for each attempt up to `keygen_attempt_limit`:
   1. use `keygen_rng` to sample the complete ordered tuple of `d` factors;
   2. reject the complete tuple if any unordered edge occurs more than once;
   3. use `keygen_rng` to sample the vertex permutation;
   4. apply the canonical edge-to-label remapping;
   5. construct `pk` and `sk = (pk, c)`;
   6. validate the public-key structure and the witness relation;
   7. return the key pair;
4. return `SamplingExhausted` if no attempt succeeds.

Rejected attempts consume randomness. A new attempt samples a new complete factor tuple and a new vertex permutation. It must not reuse part of a rejected tuple.

## 11. KeyGen Errors

`KeyGenError` distinguishes at least:

```text
UnknownParameter
InvalidParameterDescriptor
RandomnessFailure
SamplingExhausted
InternalInvariantFailure
```

The public result contains no partial key material.

The implementation must not respond to failure by:

- changing the parameter identifier;
- lowering `d`;
- extending the attempt limit without the caller's knowledge;
- keeping only noncolliding factors;
- replacing individual colliding edges;
- returning an unchecked key pair.

Diagnostic tools may record rejection counts and internal causes, but ordinary failure output must not expose secret intermediate candidates.

## 12. Sign Interface

The interface is:

```text
Sign(secret_key, message, sign_rng) -> Result<Signature, SignError>
```

`message` is an exact byte string. Text conversion, file reading, and character normalization occur outside this interface.

Before proof generation, the signer performs:

1. resolve `secret_key.public_key.parameter_id`;
2. validate the public-key structure;
3. check the label-vector length and range;
4. validate the full R3HFR witness relation;
5. enforce the message-size and protocol resource limits;
6. construct the signing context from the embedded public key and message;
7. obtain fresh salt and proof randomness from `sign_rng`;
8. invoke the proof-layer signing procedure once that procedure is specified.

The signature object remains opaque here. Its fields cannot be fixed until the proof and transcript specifications exist.

## 13. Signing Context

The logical context passed to the future proof layer is:

```text
SigningContext {
    parameter_id
    public_key
    message
}
```

The witness passed to the proof layer is:

```text
Witness {
    labels   // indexed by public_key.edges
}
```

For every index `i`, the proof circuit interprets `labels[i]` as the label of `public_key.edges[i]`. No module may reorder edges independently after this point.

The complete parameter identifier, public key, and message must be bound into the non-interactive transcript. The exact byte construction is defined by the canonical-encoding and hash-domain specification.

## 14. Sign Errors and Randomness Consumption

`SignError` distinguishes at least:

```text
UnknownParameter
IncompleteProtocolConfiguration
InvalidSecretKey
MessageTooLarge
RandomnessFailure
ProofGenerationFailure
InternalInvariantFailure
```

Signing returns either one complete signature or an error. It never returns a partial transcript.

If signing fails after consuming randomness, the consumed bytes are discarded. Retrying requires fresh randomness. An implementation must not resume from, reuse, or expose a partially generated proof transcript.

Signing is stateless with respect to earlier signatures. This requirement does not imply deterministic signing.

## 15. Verify Interface

The external interface is total:

```text
Verify(public_key, message, signature) -> boolean
```

It returns `false` for every invalid input and does not require the caller to classify failures.

Verification performs, in order:

1. resolve the public key's parameter identifier;
2. validate the canonical public-key structure;
3. enforce message, signature, and protocol resource limits;
4. validate the signature object's canonical form once defined;
5. reconstruct the verification context from the supplied public key and exact message bytes;
6. recompute every transcript-bound value;
7. verify the proof according to the later proof specification;
8. return `true` only if every check succeeds.

Internal research tools may expose a diagnostic result, but their accepted set must be identical to the boolean interface.

Verification must not attempt to recover a factorization and must not trust parameter or public-key data copied from inside a signature when it conflicts with the supplied `pk`.

## 16. Validation Helpers

Implementations expose or internally share the following reference checks:

```text
ValidatePublicKey(pk) -> Result<(), PublicKeyError>
ValidateSecretKey(sk) -> Result<(), SecretKeyError>
ValidateRelation(pk.edges, sk.labels) -> Result<(), RelationError>
PublicKeyFromSecretKey(sk) -> pk
```

`ValidateSecretKey` calls both `ValidatePublicKey` and `ValidateRelation`. `KeyGen` and `Sign` use the same reference validators used by tests; they do not maintain a weaker private definition.

The later proof circuit must accept exactly the same witness relation as `ValidateRelation`.

## 17. Equivalent Secret Keys

For any global factor-label permutation `sigma`, the two secret-key objects

```math
(pk,c)
\quad\text{and}\quad
(pk,\sigma\cdot c)
```

are different logical objects but equivalent witnesses for the same public key.

Both must pass secret-key validation and both may be used for signing. The interface does not require recovery or preservation of the planted factor names.

The first-occurrence representative `CanonLabel(c)` is used when experiments compare equivalence classes. It is not automatically substituted during signing and is not an additional validity condition.

## 18. Required Interface Tests

Later implementations must test at least:

- `sk.public_key` is exactly the `pk` returned by KeyGen;
- the label vector remains aligned after vertex permutation and edge sorting;
- sorting edges without labels causes relation validation to fail on a suitable test fixture;
- every successful KeyGen result passes both public-key and secret-key validation;
- a deterministic test source reproduces the same logical key pair;
- a randomness error produces no key or signature;
- attempt exhaustion produces `SamplingExhausted` without parameter fallback;
- replacing the embedded public key invalidates the secret key;
- changing one public edge, one label, or the parameter identifier is detected;
- globally permuting all factor labels preserves secret-key validity;
- `Sign` rejects an invalid secret key before proof generation;
- `Verify` rejects unknown parameters and all context mismatches;
- the boolean and diagnostic verification paths accept the same inputs.

## 19. Deferred Decisions

The following remain deliberately unresolved:

- the concrete parameter identifiers and their numeric limits;
- the canonical bytes for public keys, secret keys, messages, and signatures;
- deterministic expansion from seeds;
- hash and XOF domains;
- the proof protocol and its configuration;
- the concrete `Signature` object;
- transcript challenge derivation and soundness parameters.

Until those decisions are fixed, the interfaces are specification-only. In particular, an opaque `Signature` type is not permission to choose an arbitrary proof system during implementation.

## 20. Normative Summary

The corrected interfaces are:

```text
KeyGen(parameter_id, keygen_rng)
    -> Result<{ public_key, secret_key }, KeyGenError>

secret_key = {
    public_key,
    labels indexed by public_key.edges
}

Sign(secret_key, message, sign_rng)
    -> Result<Signature, SignError>

Verify(public_key, message, signature)
    -> boolean
```

The signer obtains the complete public context from the secret key. Canonical edge sorting always moves each edge together with its factor label. Parameters and randomness are explicit inputs, and every failure is reported without silent fallback.
