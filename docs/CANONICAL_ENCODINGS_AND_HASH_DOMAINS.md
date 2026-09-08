# TRIFACT Canonical Encodings and Hash Domains

[English](CANONICAL_ENCODINGS_AND_HASH_DOMAINS.md) | [한국어](CANONICAL_ENCODINGS_AND_HASH_DOMAINS_KR.md)

> **Status: Normative encoding specification**
>
> This document fixes canonical binary framing, strict decoding, randomness expansion, and domain-separated hash inputs. It does not define the internal proof body of a signature.

## 1. Purpose

TRIFACT requires every implementation to produce the same bytes for the same logical object. Structural equality alone is insufficient because keys, messages, commitments, and challenges are eventually hashed.

This specification defines:

- unsigned integer encoding;
- length-prefixed byte strings and sequences;
- parameter identifier encoding;
- canonical record framing;
- edge, edge-list, label-vector, public-key, and secret-key encoding;
- message and outer signature framing;
- strict rejection of malformed or non-canonical inputs;
- deterministic randomness expansion;
- the shared hash and XOF primitive;
- all currently required domain strings and their ordered inputs;
- mandatory parameter, public-key, and message binding.

## 2. Byte and Integer Conventions

`byte` means an integer in `[0,255]`. Concatenation is written `||`. All unsigned integers use fixed-width big-endian encoding.

```text
U8(x)   one byte
U16(x)  two bytes, most significant byte first
U32(x)  four bytes, most significant byte first
U64(x)  eight bytes, most significant byte first
```

An encoder rejects a value outside the selected width. A decoder accepts exactly the required number of bytes and does not accept signed, variable-width, decimal, hexadecimal-text, or little-endian alternatives.

Examples:

```text
U8(5)          = 05
U16(258)       = 0102
U32(3)         = 00000003
U64(256)       = 0000000000000100
```

## 3. Length-Prefixed Bytes

For a byte string `X`, define:

```text
Blob(X) = U32(len(X)) || X
```

The maximum encodable payload length is `2^32 - 1`, but every parser applies the smaller resource limit from the selected parameter descriptor before allocation.

Examples:

```text
Blob(empty) = 00000000
Blob("abc") = 00000003 616263
```

The quoted text in examples is encoded as ASCII. Unless a field explicitly says otherwise, protocol objects are bytes and undergo no character conversion or normalization.

## 4. Canonical Sequences

For an ordered sequence of byte strings `X[0], ..., X[k-1]`, define:

```text
Sequence(X) = U32(k) || Blob(X[0]) || ... || Blob(X[k-1])
```

Sequence order is significant. A schema must state how elements are ordered. A decoder rejects an element count or payload length that exceeds its resource limit before allocating memory.

Sets are never encoded by iteration order. Their elements must first be converted to their specified canonical order and then encoded as a sequence.

## 5. Restricted Names

Parameter identifiers, record names, and proof component names use only ASCII.

A restricted name must match:

```text
[a-z][a-z0-9-]{0,31}
```

Its encoding is:

```text
Name(s) = U8(len(s)) || ASCII(s)
```

Names are case-sensitive. Uppercase text, underscores, whitespace, Unicode, empty names, and names longer than 32 bytes are rejected.

Concrete parameter identifiers are registered by the research-parameter specification. A decoded but unregistered identifier is invalid.

## 6. Canonical Records

A record has one restricted name and an ordered list of numbered fields.

```text
Field(id, payload) = U16(id) || Blob(payload)

Record(name, fields) =
    Name(name)
    || U16(field_count)
    || Field(id_0, payload_0)
    || ...
    || Field(id_t, payload_t)
```

Canonical record rules are:

- field identifiers are in `[1,65535]`;
- identifiers are strictly increasing;
- no identifier occurs twice;
- the schema fixes every required field and its exact type;
- missing, repeated, out-of-order, and unknown fields are rejected;
- fields are never silently ignored;
- a record parser must consume its complete input.

The record name prevents one object type from being decoded as another. Field numbers provide unambiguous framing; they are not an extension mechanism for accepting unspecified data.

## 7. Parameter Identifier Encoding

The canonical encoding of a `ParameterId` is:

```text
EncodeParameterId(parameter_id) = Name(parameter_id)
```

The decoder performs both syntactic name validation and registry lookup. There is no fallback parameter and no prefix matching.

## 8. Edge Encoding

For an edge `e = (a,b,c)`, define:

```text
EncodeEdge(e) = U32(a) || U32(b) || U32(c)
```

Before encoding and after decoding, the edge must satisfy:

```math
0\le a<b<c<n.
```

The encoder does not sort an invalid input on the caller's behalf. A differently ordered triple is rejected rather than normalized during serialization.

## 9. Canonical Edge-List Encoding

Let the canonical edge list be:

```math
E_{\mathrm{can}}=(e_0,\ldots,e_{m-1}).
```

Its encoding is:

```text
EncodeEdgeList(E_can) =
    U32(m)
    || EncodeEdge(e_0)
    || ...
    || EncodeEdge(e_(m-1))
```

The decoder resolves `(n,d)` from the parameter identifier before accepting the list and requires:

- `m = dn/3` exactly;
- every edge is valid and internally sorted;
- `e_i < e_(i+1)` in lexicographic order;
- no duplicate edge;
- every vertex has degree exactly `d`;
- `n`, `d`, and `m` fit their specified limits.

The edge list has no alternative sparse, textual, set, adjacency, or compressed canonical form.

## 10. Label-Vector Encoding

For a label vector `c = (c_0, ..., c_(m-1))`, define:

```text
EncodeLabelVector(c) =
    U32(m)
    || U32(c_0)
    || ...
    || U32(c_(m-1))
```

Every label must lie in `[0,d)`. The `i`-th label corresponds to the `i`-th edge of the embedded canonical public key.

The encoder preserves the supplied factor names. It does not apply `CanonLabel`. Globally permuted valid witnesses therefore have different secret-key encodings even though they belong to the same witness equivalence class.

## 11. Public-Key Encoding

The canonical public-key bytes are:

```text
EncodePublicKey(pk) = Record(
    "trifact-public-key",
    [
        (1, EncodeParameterId(pk.parameter_id)),
        (2, EncodeEdgeList(pk.edges))
    ]
)
```

Encoding requires `ValidatePublicKey(pk)` to succeed. Decoding applies the same structural validation after parsing.

The parameter identifier is part of the public key. Two identical edge lists under different identifiers are different public keys and have different encodings.

## 12. Secret-Key Encoding

The canonical secret-key bytes are:

```text
EncodeSecretKey(sk) = Record(
    "trifact-secret-key",
    [
        (1, EncodePublicKey(sk.public_key)),
        (2, EncodeLabelVector(sk.labels))
    ]
)
```

Encoding requires `ValidateSecretKey(sk)` to succeed. Decoding validates the embedded public key, label-vector length and range, and the complete R3HFR relation.

A secret-key decoder returns one self-contained `SecretKey`. It does not accept an external public key as a substitute for the embedded object.

## 13. Message Encoding

`Sign` and `Verify` receive an exact message byte string `M`. Its canonical transcript representation is:

```text
EncodeMessage(M) = Blob(M)
```

The interface performs no text decoding, Unicode normalization, newline conversion, JSON parsing, file metadata inclusion, or prehash selection.

An empty message is a valid byte string unless a later protocol rule explicitly excludes it. The configured message-size limit is checked before allocating or hashing the complete input.

## 14. Outer Signature Encoding

The proof specification will define `protocol_body`. The outer signature framing is fixed now:

```text
EncodeSignature(signature) = Record(
    "trifact-signature",
    [
        (1, EncodeParameterId(signature.parameter_id)),
        (2, signature.salt),
        (3, EncodeProtocolBody(signature.protocol_body))
    ]
)
```

The salt is exactly 32 bytes. The signature parameter identifier must equal `pk.parameter_id`; `Verify` rejects a mismatch.

`EncodeProtocolBody` must itself be one canonical record whose schema is fixed by the proof specification. Until that schema exists, no complete signature encoding exists and signing remains blocked.

## 15. Transcript Component Encoding

Every structured transcript component uses `Record`. Every repeated collection uses `Sequence` in the order specified by the proof protocol.

The following ordering rules already apply:

- repetition indices increase from `0`;
- within a repetition, party indices increase from `0`;
- commitments are encoded in `(repetition_index, party_index)` order;
- opened views follow challenge order, with ties broken by party index;
- a component carrying an index includes that index in its own record;
- maps, hash tables, filesystem order, and thread completion order are never serialization orders.

The proof specification must assign record names and field identifiers for every commitment, view, opening, challenge, and auxiliary value. It must not replace this framing with an implementation-specific serializer.

## 16. Strict Decoding

A decoder follows parse, bound, validate, and consume-all semantics:

1. read only enough bytes to determine the next bounded field;
2. check declared lengths and counts against parameter and global limits;
3. reject before allocation when a limit is exceeded;
4. parse fields in the required order;
5. validate canonical form and mathematical ranges;
6. reject if any byte remains after the object ends.

At minimum, decoding rejects:

- truncated integers, names, blobs, records, and sequences;
- invalid ASCII names;
- unregistered parameter identifiers;
- zero, duplicate, decreasing, missing, or unknown record field identifiers;
- inconsistent counts or lengths;
- integer overflow and arithmetic overflow in `m = dn/3`;
- invalid, duplicate, or unsorted edges;
- out-of-range labels or wrong label count;
- an invalid secret witness;
- a signature parameter mismatch;
- oversized inputs before large allocation;
- trailing bytes.

Malformed input produces a normal decoding failure. It must not cause a panic, partial object, implicit repair, or fallback parser.

## 17. Decode-and-Reencode Rule

For each supported object type and every accepted byte string `B`:

```text
Encode(Decode(B)) == B
```

Conversely, for every valid logical object `X`:

```text
Decode(Encode(X)) == X
```

If a parser could accept two byte strings for one logical object, at least one of those strings is non-canonical and must be rejected.

## 18. Hash and XOF Primitive

TRIFACT uses `SHAKE256` for the shared research hash and XOF interface.

For an ordered list of already canonical field bytes `F`, define:

```text
HashFrame(domain, F) =
    ASCII("TRIFACT-HASH")
    || Blob(ASCII(domain))
    || Sequence(F)

XofFrame(domain, F) =
    ASCII("TRIFACT-XOF")
    || Blob(ASCII(domain))
    || Sequence(F)
```

Then define:

```text
H32(domain, F) = SHAKE256(HashFrame(domain, F), 32 bytes)

XOF(domain, F, length) =
    SHAKE256(XofFrame(domain, F) || U32(length), length bytes)

XOFStream(domain, F) =
    the byte stream emitted by SHAKE256(XofFrame(domain, F))
```

`XOF` accepts lengths in `[0, 2^32 - 1]` subject to smaller resource limits. Including the requested length makes separate fixed-length requests distinct. `XOFStream` is used only where a sequential consumer is explicitly specified.

Domain strings are exact case-sensitive ASCII constants from the registry below. Unregistered domains are not accepted by protocol code.

## 19. Digest and Context Definitions

Define:

```text
pk_digest = H32(
    "TRIFACT/pk-digest",
    [EncodePublicKey(pk)]
)

message_digest = H32(
    "TRIFACT/message-digest",
    [EncodeMessage(M)]
)

context_digest = H32(
    "TRIFACT/signing-context",
    [
        EncodeParameterId(pk.parameter_id),
        pk_digest,
        message_digest
    ]
)
```

The parameter identifier appears both inside the public key and as an explicit context field. This redundancy is intentional: every signing and verification transcript is visibly scoped to one registered parameter configuration.

No signature may derive its challenge from the message alone, from a public-key filename, or from a shortened non-canonical representation.

## 20. KeyGen Randomness Expansion

`KeyGen` obtains exactly 32 bytes once:

```text
keygen_seed = keygen_rng.read(32)
```

For zero-based attempt index `a`, define an independent attempt stream:

```text
attempt_stream[a] = XOFStream(
    "TRIFACT/keygen-attempt",
    [
        EncodeParameterId(parameter_id),
        keygen_seed,
        U32(a)
    ]
)
```

The attempt limit must fit `U32`. Factor permutations are sampled from this stream in increasing factor index, followed by the vertex permutation. A rejected attempt discards the rest of its stream. The next attempt uses `a + 1` and is unaffected by how many bytes the rejected attempt consumed.

To draw a uniform integer from `[0,q)` for `1 <= q <= 2^32`, consume the next eight stream bytes as `x = U64(bytes)` and set:

```math
L=2^{64}-(2^{64}\bmod q).
```

Accept `x` when `x < L` and return `x mod q`; otherwise draw another eight bytes. A permutation uses descending Fisher-Yates order and calls this rule with `q = i + 1` for `i = n-1, ..., 1`.

This rule removes modulo bias and makes a supplied `keygen_seed` reproducible across implementations.

## 21. Signing Randomness Expansion

`Sign` obtains exactly 32 fresh bytes once after validating the secret key and message limits:

```text
sign_seed = sign_rng.read(32)
```

It derives the public salt:

```text
salt = XOF(
    "TRIFACT/sign-salt",
    [context_digest, sign_seed],
    32
)
```

Proof components derive independent bytes with:

```text
ProofBytes(repetition, component_name, length) = XOF(
    "TRIFACT/proof-expand",
    [
        context_digest,
        salt,
        sign_seed,
        U32(repetition),
        Name(component_name)
    ],
    length
)
```

The proof specification registers every `component_name` and required length. Reusing one component name for two meanings is forbidden.

The seed is never included in the signature. If `sign_rng.read(32)` fails or returns the wrong length, signing fails without a signature.

## 22. Commitment and Challenge Domains

For a canonical encoded party view and its commitment randomizer, define:

```text
view_commitment = H32(
    "TRIFACT/view-commitment",
    [
        context_digest,
        U32(repetition_index),
        U32(party_index),
        EncodeView(view),
        commitment_randomizer
    ]
)
```

Let `ordered_commitments` contain every 32-byte view commitment in `(repetition_index, party_index)` order. Define:

```text
commitment_root = H32(
    "TRIFACT/commitment-root",
    [
        context_digest,
        Sequence(ordered_commitments)
    ]
)
```

The challenge byte stream is:

```text
challenge_stream = XOFStream(
    "TRIFACT/challenge",
    [
        context_digest,
        salt,
        commitment_root,
        EncodeChallengeAux(challenge_aux)
    ]
)
```

`EncodeView` and `EncodeChallengeAux` are canonical records defined by the proof specification. If no auxiliary value is needed, that specification must define one explicit empty record rather than omit the field.

The proof specification defines how unbiased challenges are sampled from `challenge_stream`. It may not hash a different subset of the context.

## 23. Domain Registry

The exact registered domains are:

| Domain | Primitive | Ordered inputs | Purpose |
| --- | --- | --- | --- |
| `TRIFACT/pk-digest` | `H32` | encoded public key | Compact public-key binding |
| `TRIFACT/message-digest` | `H32` | encoded message | Exact message binding |
| `TRIFACT/signing-context` | `H32` | parameter ID, public-key digest, message digest | Shared signing context |
| `TRIFACT/keygen-attempt` | `XOFStream` | parameter ID, keygen seed, attempt index | Reproducible KeyGen sampling |
| `TRIFACT/sign-salt` | `XOF` | context digest, signing seed | Public per-signature salt |
| `TRIFACT/proof-expand` | `XOF` | context digest, salt, signing seed, repetition index, component name | Independent proof randomness |
| `TRIFACT/view-commitment` | `H32` | context digest, repetition index, party index, encoded view, randomizer | Party-view commitment |
| `TRIFACT/commitment-root` | `H32` | context digest, ordered commitments | Transcript commitment root |
| `TRIFACT/challenge` | `XOFStream` | context digest, salt, commitment root, encoded challenge auxiliary data | Fiat-Shamir challenge stream |

The table is normative. Spelling, capitalization, separators, primitive choice, input order, and framing must match exactly.

Adding a domain requires a specification change. Concatenating an informal suffix to an existing domain is not allowed.

## 24. Binding Requirements

Every signature challenge is transitively bound to:

- the registered parameter identifier;
- the complete canonical public key;
- the exact message length and bytes;
- the 32-byte signature salt;
- all ordered commitments;
- all challenge auxiliary data defined by the proof protocol.

Changing any bound value requires recomputing the context, commitments where applicable, and challenge.

The verifier reconstructs `pk_digest`, `message_digest`, and `context_digest` from its own `pk` and `M`. It never accepts those digests as trusted signature inputs.

## 25. Required Encoding Tests

Implementations must include golden-byte and negative tests for at least:

- every fixed-width integer boundary;
- empty and nonempty blobs and sequences;
- valid and invalid restricted names;
- public-key and secret-key round trips;
- different edge or field orders;
- duplicate edges and record fields;
- missing and unknown record fields;
- truncated input at every byte position;
- every accepted input reencoding to identical bytes;
- trailing-byte rejection;
- oversized declared lengths rejected before allocation;
- label vectors that are valid, out of range, misaligned, or the wrong length;
- distinct parameter identifiers producing distinct public-key digests;
- one-bit changes in the public key or message changing the signing context;
- identical KeyGen seeds reproducing identical logical key pairs;
- different attempt indices producing different KeyGen streams;
- identical signing inputs and signing seeds reproducing identical derived salt and proof bytes;
- any changed signing context producing different derived values;
- every registered domain producing output distinct from every other domain on equal field bytes.

Hash and XOF test vectors must be generated from this specification and committed with the implementation tests before the codec is considered complete.

## 26. Explicit Non-Claims

This specification does not claim that:

- canonical encoding makes R3HFR difficult;
- `SHAKE256` alone proves the signature secure;
- a 32-byte digest establishes an overall security level for TRIFACT;
- domain separation fills gaps in the proof of knowledge or Fiat-Shamir analysis;
- the undefined protocol body can be implemented safely before the proof specification exists.

It only removes byte-level ambiguity and cross-context reuse from the research design.

## 27. Normative Summary

TRIFACT uses fixed-width big-endian integers, length-prefixed bytes, strictly ordered records, canonical edge order, and consume-all decoding.

The key encodings are:

```text
public key = Record("trifact-public-key", parameter ID, edge list)
secret key = Record("trifact-secret-key", complete public key, label vector)
```

The shared signing context is:

```text
context_digest = H32(
    "TRIFACT/signing-context",
    parameter ID || public-key digest || message digest
)
```

Every commitment and challenge includes that context through its registered domain. Non-canonical inputs, unknown fields, resource-limit violations, and trailing bytes are rejected.
