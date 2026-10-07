# TRIFACT 암호 테스트 벡터

[English](CRYPTO_VECTORS.md) | [한국어](CRYPTO_VECTORS_KR.md)

## SHAKE256 출처

`tests/shake_vectors.h`는 NIST의 [0비트 message 예제](https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/SHAKE256_Msg0.pdf)와 [1600비트 message 예제](https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/SHAKE256_Msg1600.pdf)의 완전한 512바이트 출력을 유지한다. 후자의 입력은 `a3` byte를 200번 반복한 것이다. 각 예제의 마지막 출력에서 expected byte를 옮긴 후 보관 전에 Python 3.12의 `hashlib.shake_256`으로 독립 확인했다. Test는 TRIFACT 구현으로 기대값을 다시 생성하지 않는다.

추가 vector 13개는 `input[i] = (17*i + 3) mod 256`, 길이 `0,1,7,8,31,32,135,136,137,271,272,273,4096`을 사용한다. 출력 513바이트는 독립 구현으로 계산했다. Rate 경계·padding 중첩·복수 absorb block·복수 squeeze block을 확인한다. C test는 출력 prefix 길이 `0,1,31,32,135,136,137,271,272,273,512,513`과 입력·출력 chunk `1,7,135,136,137`바이트를 비교한다.

`trifact.shake-vectors`는 network나 Python runtime 없이 commit한 C data를 읽는다. 유지된 모든 message는 공개 테스트 자료다. NIST 예제 검사는 비공식 구현 확인이며 CAVP 인증에 해당하지 않는다.

## Framed hash 출처

`tests/hash_vectors.h`는 등록 domain 9개 모두에 대해 독립적으로 계산한 정답을 보관한다. 각 fixture는 세 field를 사용한다. 첫 field는 `03 74 6f 79` (`Name("toy")`), 두 번째는 `00 61 ff 62`, 세 번째는 빈 field다. 이 저수준 framing test는 해당 목록이 향후 protocol 객체의 field schema를 만족한다고 주장하지 않는다. H32 domain은 32바이트, fixed-XOF·stream domain은 64바이트를 출력한다. 별도로 구성한 byte string을 Python 3.12 `hashlib.shake_256`으로 계산했다. 순서는 literal mode prefix·big-endian domain 길이·ASCII domain·big-endian field 수·각 big-endian field 길이와 정확한 field byte다. Fixed XOF만 big-endian `U32(64)`를 덧붙인다.

`trifact.hash-substitutions`는 commit된 정답과 별도 C framing builder를 모두 비교한다. 정확한 registry 이름·미등록 이름·금지된 mode 대체·parameter 변경·field 순서 변경과 제거·내부 0바이트·빈 field와 field 없음·동일한 연결 byte의 다른 field 경계·fixed-XOF 출력 길이 변경을 검사한다. U32보다 큰 수와 길이는 field 데이터를 읽기 전에 거부하며 오류 시 호출자 출력을 유지하는지 검사한다. Stream fixture는 두 번 나누어 squeeze해 이어지는 출력을 검사한다. 외부 runtime이나 production 코드로 생성한 정답을 사용하지 않는다.

## 연구 stream 출처

`tests/research_vectors.h`는 Python 3.12 `hashlib.shake_256`으로 독립 계산한 273바이트 출력 9개를 보관한다. 공개 seed는 `00`부터 `1f`까지의 byte다. Parameter 이름 `toy`·`small`·`medium` 각각에 attempt `0`·`1`·`4294967295`를 사용한다. 각 입력은 `[Name(parameter_id), seed, U32(attempt)]` field를 사용하는 규정된 `TRIFACT/keygen-attempt` stream frame이며 길이·attempt 정수는 모두 big-endian이다. 출력 길이를 덧붙이지 않는다. Fixture는 squeeze 경계 두 개와 전체 U32 attempt encoding을 검사한다. `trifact.research`는 전체 read와 불규칙 chunk read를 비교하고 호출자 저장소 변경 전에 seed를 absorb했는지 확인한다. `trifact.sampling`은 결정론적 순열 재현·범위·중복 없음을 검사한다.

`tests/sampling_vectors.h`는 같은 공개 seed와 attempt 1에서 `toy`·`small`·`medium`의 크기 12·24·60 순열 및 각 stream의 뒤이은 bound `1,2,3,7,12,4294967295,4294967296` draw를 추가로 보관한다. 기대값은 `hashlib` stream byte, Python 임의 정밀도 구간 한계 `(2**64 // q) * q`, `x >= limit` 거부, descending Fisher-Yates를 사용해 독립 계산했다. `trifact.sampling-vectors`는 전체 순열과 이후 draw를 각각 비교하므로 stream 소비와 U64 byte 순서도 platform 간 재현에 포함된다. C test build와 실행에 Python이 필요하지 않다.
