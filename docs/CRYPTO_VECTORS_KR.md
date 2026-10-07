# TRIFACT 암호 테스트 벡터

[English](CRYPTO_VECTORS.md) | [한국어](CRYPTO_VECTORS_KR.md)

## SHAKE256 출처

`tests/shake_vectors.h`는 NIST의 [0비트 message 예제](https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/SHAKE256_Msg0.pdf)와 [1600비트 message 예제](https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/SHAKE256_Msg1600.pdf)의 완전한 512바이트 출력을 유지한다. 후자의 입력은 `a3` byte를 200번 반복한 것이다. 각 예제의 마지막 출력에서 expected byte를 옮긴 후 보관 전에 Python 3.12의 `hashlib.shake_256`으로 독립 확인했다. Test는 TRIFACT 구현으로 기대값을 다시 생성하지 않는다.

추가 vector 13개는 `input[i] = (17*i + 3) mod 256`, 길이 `0,1,7,8,31,32,135,136,137,271,272,273,4096`을 사용한다. 출력 513바이트는 독립 구현으로 계산했다. Rate 경계·padding 중첩·복수 absorb block·복수 squeeze block을 확인한다. C test는 출력 prefix 길이 `0,1,31,32,135,136,137,271,272,273,512,513`과 입력·출력 chunk `1,7,135,136,137`바이트를 비교한다.

`trifact.shake-vectors`는 network나 Python runtime 없이 commit한 C data를 읽는다. 유지된 모든 message는 공개 테스트 자료다. NIST 예제 검사는 비공식 구현 확인이며 CAVP 인증에 해당하지 않는다.

## Framed hash 출처

`tests/hash_vectors.h`는 등록 domain 9개 모두에 대해 독립적으로 계산한 정답을 보관한다. 각 fixture는 세 field를 사용한다. 첫 field는 `03 74 6f 79` (`Name("toy")`), 두 번째는 `00 61 ff 62`, 세 번째는 빈 field다. 이 저수준 framing test는 해당 목록이 향후 protocol 객체의 field schema를 만족한다고 주장하지 않는다. H32 domain은 32바이트, fixed-XOF·stream domain은 64바이트를 출력한다. 별도로 구성한 byte string을 Python 3.12 `hashlib.shake_256`으로 계산했다. 순서는 literal mode prefix·big-endian domain 길이·ASCII domain·big-endian field 수·각 big-endian field 길이와 정확한 field byte다. Fixed XOF만 big-endian `U32(64)`를 덧붙인다.

`trifact.hash-substitutions`는 commit된 정답과 별도 C framing builder를 모두 비교한다. 정확한 registry 이름·미등록 이름·금지된 mode 대체·parameter 변경·field 순서 변경과 제거·내부 0바이트·빈 field와 field 없음·동일한 연결 byte의 다른 field 경계·fixed-XOF 출력 길이 변경을 검사한다. U32보다 큰 수와 길이는 field 데이터를 읽기 전에 거부하며 오류 시 호출자 출력을 유지하는지 검사한다. Stream fixture는 두 번 나누어 squeeze해 이어지는 출력을 검사한다. 외부 runtime이나 production 코드로 생성한 정답을 사용하지 않는다.
