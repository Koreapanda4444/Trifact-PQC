# TRIFACT Cryptographic Test Vectors

[English](CRYPTO_VECTORS.md) | [한국어](CRYPTO_VECTORS_KR.md)

## SHAKE256 provenance

`tests/shake_vectors.h` retains the complete 512-byte outputs from NIST's [zero-bit message example](https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/SHAKE256_Msg0.pdf) and [1600-bit message example](https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/SHAKE256_Msg1600.pdf). The latter input is 200 repetitions of byte `a3`. Expected bytes were transcribed from each example's final output and independently checked with Python 3.12's `hashlib.shake_256` before retention. Tests never regenerate expectations from the TRIFACT implementation.

Thirteen additional vectors use byte `input[i] = (17*i + 3) mod 256`, lengths `0,1,7,8,31,32,135,136,137,271,272,273,4096`, and 513 output bytes computed using that independent implementation. They exercise the rate boundary, padding overlap, multiple absorb blocks, and multiple squeeze blocks. The C test compares output prefixes of lengths `0,1,31,32,135,136,137,271,272,273,512,513` and split input/output chunks of `1,7,135,136,137` bytes.

`trifact.shake-vectors` reads committed C data without network access or a Python runtime. Every retained message is public test data. The NIST example checks are informal implementation checks and do not constitute CAVP validation.

## Framed hash provenance

`tests/hash_vectors.h` contains independently computed answers for all nine registered domains. Every fixture uses three fields: bytes `03 74 6f 79` (`Name("toy")`), bytes `00 61 ff 62`, and an empty field. The low-level framing tests deliberately do not claim these lists satisfy a future protocol object's field schema. H32 domains produce 32 bytes; fixed-XOF and stream domains produce 64 bytes. Expected bytes were computed with Python 3.12 `hashlib.shake_256` from a separately constructed byte string: the literal mode prefix, big-endian domain length, ASCII domain, big-endian field count, and each big-endian field length followed by its exact bytes. Only fixed XOF appends big-endian `U32(64)`.

`trifact.hash-substitutions` compares both the committed answers and a separate C framing builder. It covers exact registry names, unknown names, forbidden mode substitutions, parameter changes, field reordering and removal, embedded zero bytes, empty fields versus no fields, ambiguous concatenations split at different boundaries, and fixed-XOF output-length changes. It rejects counts and lengths above U32 before reading field data and checks that errors preserve caller output. Stream fixtures squeeze in two chunks to check continuation. Tests use no external runtime or generated production expectations.

## Research stream provenance

`tests/research_vectors.h` retains nine 273-byte outputs independently computed with Python 3.12 `hashlib.shake_256`. The public seed is bytes `00` through `1f`; parameter names are `toy`, `small`, and `medium`, each at attempts `0`, `1`, and `4294967295`. Each input is precisely the specified stream frame for `TRIFACT/keygen-attempt` with `[Name(parameter_id), seed, U32(attempt)]`, all length and attempt integers in big-endian order. No output length is appended. These fixtures exercise two squeeze boundaries and the full U32 attempt encoding. `trifact.research` compares whole and irregularly chunked reads, then verifies the seed was absorbed before caller storage changed. `trifact.sampling` checks deterministic permutation replay and range/uniqueness properties.
