# TRIFACT Primitive and Randomness Providers

[English](CRYPTO_PROVIDERS.md) | [한국어](CRYPTO_PROVIDERS_KR.md)

## Backend and acquisition

The research baseline selects a built-in, portable C17 SHAKE256 backend implementing the FIPS 202 Keccak-f[1600] permutation. It uses 24 rounds, a 136-byte rate, a 512-bit capacity, and the SHAKE byte suffix `0x1f`. Lane-byte conversion uses explicit shifts rather than native byte order or unaligned casts. This is a reference implementation, not a validated cryptographic module.

No third-party C library, package download, runtime backend discovery, or fallback is needed. A normal source checkout and the existing CMake toolchains acquire the complete backend. Alternative providers require an explicit reviewed change, the same state and error contracts, and the retained reference corpus. Dependency versions and acquisition would be documented before integrating such a provider.

## SHAKE256 lifecycle

The public wrapper owns an opaque context. Creation publishes an absorbing context or clears the output slot on failure. Its only successful transition is `absorbing -> squeezing`, through explicit finalization. Absorb is allowed only before finalization; squeeze only afterward. Repeated finalization, absorb after finalization, and squeeze before finalization return `INVALID_STATE`, without changing the context or caller bytes.

Zero-length absorb and squeeze accept a `NULL` byte pointer in the appropriate phase. A nonzero length requires a pointer to that many bytes. Empty and binary messages require no text conversion. Squeeze calls continue the same stream across rate boundaries; output length is not an input to raw SHAKE256. A one-shot wrapper is equivalent to create, absorb, finalize, squeeze, and destroy. Argument or allocation errors leave one-shot output unchanged.

Contexts are independently owned, contain no global mutable state, and are not shared concurrently. Caller buffers must not alias context storage. Destruction accepts `NULL` and clears context state through volatile byte writes before release. Repeated destruction requires nulling the caller's owned pointer. The clear routine does not promise deletion of compiler-created or external copies.

## Framed hashing boundary

The existing encoding specification remains authoritative: exact `TRIFACT-HASH` and `TRIFACT-XOF` prefixes, `Blob(domain)`, `Sequence(fields)`, and the fixed-XOF trailing `U32(output_length)`. The nine registered domain names and their H32, fixed-XOF, or stream modes are exact. Unknown domains and mismatched primitive modes fail before hashing.

Fields are already canonical bytes supplied by their owning codec or protocol layer. The primitive wrapper validates pointers, U32 counts and lengths, and total framing arithmetic; it does not implement future key or proof codecs or assert that an arbitrary field list is a valid protocol object. Hash32 produces exactly 32 bytes. Fixed XOF lengths are at most `UINT32_MAX`; stream reads continue without appending a requested length. Higher layers enforce their approved resource limits before calling the primitive.

The public `trifact/hash.h` registry uses `trifact_domain_t`; `trifact_domain_name` returns a borrowed exact name and `trifact_domain_from_name` accepts only an exact registered name. A failed lookup preserves its output enum. `trifact_hash_field_t` borrows each byte span for the duration of a call. `trifact_hash32`, `trifact_xof`, and `trifact_xof_stream_create` enforce the corresponding registry mode. A created stream is already finalized and is owned and destroyed through the SHAKE256 API. The primitive accepts empty field lists and empty fields; these have distinct encodings. It streams checked framing without allocating a concatenated message.

## Entropy callback

An owning provider borrows a callback and its caller-owned context. The callback receives writable temporary storage, its remaining capacity, and a received-count slot. `OK` requires `1 <= received <= requested`; short successful reads are accumulated. `INTERRUPTED` requires zero received bytes and is retried only within the explicit caller-supplied interruption budget for that exact-read call. Other statuses, zero progress, or excessive counts permanently fail the provider.

Exact reads stage all bytes privately and publish them only after complete success. Callback failure clears temporary storage and leaves caller output unchanged. An allocation or argument failure consumes no entropy and does not poison a usable provider. A failed provider returns `RANDOMNESS_FAILURE` on every later read, including zero-length reads. A ready zero-length read needs no buffer, allocation, or callback invocation. There is no fallback to time, process identifiers, `rand`, or a research seed.

Callbacks must return in bounded time and respect the provided capacity. The wrapper can validate reported counts but cannot make an unsafe callback safe. Providers are individually owned and used serially; callback contexts must outlive them. A separate exact-read source interface lets samplers consume either an entropy provider or an explicitly selected research stream.

`trifact/rng.h` exposes `trifact_entropy_provider_create`, `trifact_entropy_read_exact`, and null-safe destruction. The constructor accepts the interruption limit; zero means an interruption immediately closes the provider. `trifact_entropy_source` returns a borrowed `{read, context}` adapter usable until provider destruction. Passing a null provider returns an empty source. A custom `trifact_random_source_t` reader must deliver the entire requested span on `OK` and preserve output on error; a sampler cannot repair a callback that violates this contract. An entropy callback's failure status is mapped to `RANDOMNESS_FAILURE`, and reported progress is never accepted together with `INTERRUPTED`.

## Operating-system adapters

Linux uses blocking `getrandom(..., 0)`, limits each callback request to 256 bytes, translates `EINTR` to `INTERRUPTED`, and delegates bounded retry and short-read assembly to the common provider. All other failures close the provider. Windows uses `BCryptGenRandom(NULL, ..., BCRYPT_USE_SYSTEM_PREFERRED_RNG)` and links the system `bcrypt` library. Neither adapter adds a device-file fallback. An unsupported platform reports `PLATFORM_UNAVAILABLE`.

`trifact_entropy_provider_create_system` selects the compiled platform adapter and the caller's interruption budget. It clears its output slot on errors. The `bcrypt` dependency is a Windows system library linked by CMake, with no package download. The OS test requests 513 bytes to cross the adapter chunk boundary and verifies guarded output and repeat reads; it makes no statistical quality claim.

## Research streams and sampling

Research mode is explicit and deterministic. It accepts a 32-byte seed, a registered research parameter name, and a U32 attempt index, using precisely `TRIFACT/keygen-attempt` with `[Name(parameter_id), seed, U32(attempt)]`. It obtains no system entropy. It represents reproducible public experiments, not an entropy source for secret production keys. The seed is absorbed and is not retained as a separate caller-visible key component.

Uniform draws consume eight stream bytes in big-endian order. For `1 <= q <= 2^32`, they accept the largest initial interval of U64 values whose size is divisible by `q`, then return the residue. Rejected tail values consume new bytes. The caller explicitly supplies a positive draw limit; exhaustion returns `SAMPLING_EXHAUSTED` without publishing a result. This bound is an implementation termination budget, not a replacement for the approved 256 KeyGen attempts.

Permutations use descending Fisher-Yates over `[0,n)`, with that same uniform rule and explicit per-draw limit. They stage their array, publish only complete success, and clear temporary storage on failure. No source rewind or partial permutation is exposed. Empty permutations are allowed. Outputs use U32 vertex values and all allocation products are checked.

`trifact/research.h` exposes an owning research stream and a borrowed exact-read source adapter. Only `toy`, `small`, and `medium` are accepted; their complete parameter descriptors remain a later registry delivery. Creation absorbs the seed synchronously, so caller seed storage need not survive the call. `trifact_uniform_u32` accepts a U64 bound to represent `2^32` exactly and returns a U32 residue. `trifact_random_permutation` takes a U32 vertex count. Both require a valid exact-read source and a positive `max_draws`; an empty permutation accepts a null output, and a one-element permutation consumes no random bytes. Sampling errors preserve outputs but can advance the source. Exhaustion does not poison a healthy source. No default source or hidden retry budget is selected.

## Validation and sources

Tests cover authoritative SHAKE256 known answers, independently framed vectors, substitutions, scripted entropy interruptions and failures, sampling boundaries, and cross-platform research replay. `trifact.rng-boundaries` checks the largest accepted U64, every rejected tail value for the selected bounds, retry exhaustion, source errors, bounds `1` and `2^32`, big-endian draws, and unchanged outputs. Its finite oracle counts residues on complete small accepted blocks and enumerates all 24 choice paths for a four-vertex permutation, comparing their image to independently enumerated permutations. This is a bounded deterministic check, not a statistical proof of a source's randomness.

`trifact.entropy-system-mock` substitutes Linux `getrandom` at compile time only in its test executable. It checks flags, 256-byte request limits, positive short reads, `EINTR`, other errors, zero progress, and impossible reported lengths. Windows CI exercises the real BCrypt adapter and the shared provider's scripted failures. `trifact.rng-allocation` fails every heap request in the primitive and randomness wrappers, including staged permutations backed by entropy. It verifies no leaked ownership, no consumption on pre-read allocation failure, reusable providers after allocation errors, and cleared bytes immediately before release. The production library has no fault-injection controls. Public test seeds and vectors carry no secret material. Passing these checks does not establish TRIFACT security.

- [NIST FIPS 202](https://doi.org/10.6028/NIST.FIPS.202)
- [NIST CAVP secure hashing vectors](https://csrc.nist.gov/Projects/cryptographic-algorithm-validation-program/Secure-Hashing)
- [Linux getrandom manual](https://man7.org/linux/man-pages/man2/getrandom.2.html)
- [Microsoft BCryptGenRandom contract](https://learn.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgenrandom)

The streaming API, built-in backend, framed hash API, and callback entropy provider are implemented. `trifact.shake` checks the lifecycle and `trifact.shake-vectors` retains the independent reference corpus. `trifact.hash` and `trifact.hash-substitutions` check framing and the registry. `trifact.entropy` checks short reads, interruption budgets, zero and excessive progress, staged output, permanent failure, and null-safe destruction. Operating-system adapters are implemented and `trifact.entropy-system` exercises the selected platform. Research streams and unbiased sampling are implemented; `trifact.research` and `trifact.sampling` check replay and basic sampling behavior.
