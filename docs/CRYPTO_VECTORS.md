# TRIFACT Cryptographic Test Vectors

[English](CRYPTO_VECTORS.md) | [한국어](CRYPTO_VECTORS_KR.md)

## SHAKE256 provenance

`tests/shake_vectors.h` retains the complete 512-byte outputs from NIST's [zero-bit message example](https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/SHAKE256_Msg0.pdf) and [1600-bit message example](https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/SHAKE256_Msg1600.pdf). The latter input is 200 repetitions of byte `a3`. Expected bytes were transcribed from each example's final output and independently checked with Python 3.12's `hashlib.shake_256` before retention. Tests never regenerate expectations from the TRIFACT implementation.

Thirteen additional vectors use byte `input[i] = (17*i + 3) mod 256`, lengths `0,1,7,8,31,32,135,136,137,271,272,273,4096`, and 513 output bytes computed using that independent implementation. They exercise the rate boundary, padding overlap, multiple absorb blocks, and multiple squeeze blocks. The C test compares output prefixes of lengths `0,1,31,32,135,136,137,271,272,273,512,513` and split input/output chunks of `1,7,135,136,137` bytes.

`trifact.shake-vectors` reads committed C data without network access or a Python runtime. Every retained message is public test data. The NIST example checks are informal implementation checks and do not constitute CAVP validation.
