# TRIFACT 원시 함수와 난수 공급자

[English](CRYPTO_PROVIDERS.md) | [한국어](CRYPTO_PROVIDERS_KR.md)

## Backend와 확보 방법

연구용 기본 구현은 FIPS 202의 Keccak-f[1600] 치환을 구현하는 내장 portable C17 SHAKE256 backend를 선택한다. Round 24개, rate 136바이트, capacity 512비트, SHAKE byte suffix `0x1f`를 사용한다. Lane과 byte 사이 변환에는 native byte order나 비정렬 cast 대신 명시적 shift를 사용한다. 이는 참조 구현이며 검증 인증을 받은 암호 모듈이 아니다.

외부 C library·package 다운로드·실행 시 backend 탐색·fallback은 필요 없다. 일반 source checkout과 기존 CMake toolchain으로 전체 backend를 확보한다. 다른 공급자를 사용하려면 명시적으로 검토한 변경, 동일한 상태·오류 계약, 유지된 참조 corpus 검사가 필요하다. 외부 공급자를 통합하기 전에 dependency version과 확보 방법을 문서화한다.

## SHAKE256 생명주기

공개 wrapper는 opaque context를 소유한다. 생성은 absorbing context를 반환하거나 실패 시 출력 slot을 비운다. 유일한 정상 전이는 명시적 finalize를 통한 `absorbing -> squeezing`이다. Absorb는 finalize 전에만, squeeze는 그 뒤에만 허용한다. 중복 finalize·finalize 후 absorb·finalize 전 squeeze는 context와 호출자 byte를 유지하며 `INVALID_STATE`를 반환한다.

올바른 phase에서 길이 0의 absorb·squeeze는 `NULL` byte 포인터를 허용한다. 길이가 양수면 해당 길이의 저장소를 가리켜야 한다. 빈 message와 binary message는 text 변환 없이 처리한다. Squeeze는 rate 경계에서도 동일 stream을 이어가며 raw SHAKE256에는 출력 길이를 입력으로 넣지 않는다. One-shot wrapper는 create·absorb·finalize·squeeze·destroy와 같다. 인자·할당 오류는 one-shot 출력을 유지한다.

Context는 독립적으로 소유하고 전역 가변 상태를 사용하지 않으며 동시에 공유하지 않는다. 호출자 buffer는 context 저장소와 겹치면 안 된다. 해제는 `NULL`을 허용하며 반환 전에 volatile byte 쓰기로 상태를 비운다. 반복 해제는 호출자의 소유 포인터를 먼저 `NULL`로 바꿔야 한다. Clear routine은 compiler가 만든 복사본이나 외부 복사본의 삭제를 보장하지 않는다.

## Framed hash 경계

기존 encoding 명세가 기준이다. 정확한 `TRIFACT-HASH`·`TRIFACT-XOF` prefix, `Blob(domain)`, `Sequence(fields)`, 고정 XOF의 마지막 `U32(output_length)`를 유지한다. 등록 domain 9개의 이름과 H32·고정 XOF·stream mode는 정확히 일치해야 한다. 미등록 domain과 primitive mode 불일치는 hashing 전에 실패한다.

Field는 해당 codec 또는 protocol 계층이 제공한 canonical byte다. Primitive wrapper는 포인터·U32 개수와 길이·framing 전체 산술을 확인한다. 이후 key·proof codec을 구현하거나 임의 field list가 올바른 protocol object라고 판정하지 않는다. Hash32 출력은 정확히 32바이트다. 고정 XOF 길이는 `UINT32_MAX` 이하이며 stream read에는 요청 길이를 덧붙이지 않는다. 상위 계층은 primitive를 호출하기 전에 승인된 resource limit을 적용한다.

## Entropy callback

소유 provider는 callback과 호출자가 소유한 context를 참조한다. Callback은 쓰기 가능한 임시 저장소·남은 용량·받은 개수 slot을 입력받는다. `OK`는 `1 <= received <= requested`여야 하며 짧은 정상 read는 누적한다. `INTERRUPTED`는 받은 byte 0개여야 하고 exact-read 호출에 대해 호출자가 지정한 중단 재시도 예산 안에서만 반복한다. 그 외 status·진행 없음·과도한 개수는 provider를 영구 실패 상태로 만든다.

Exact read는 모든 byte를 비공개 임시 저장소에 모은 뒤 완전히 성공했을 때만 출력한다. Callback 실패는 임시 저장소를 비우고 호출자 출력을 유지한다. 할당·인자 실패는 entropy를 소비하지 않고 사용 가능한 provider를 실패 상태로 만들지 않는다. 실패한 provider는 길이 0을 포함한 모든 이후 read에서 `RANDOMNESS_FAILURE`를 반환한다. 정상 상태의 길이 0 read는 buffer·할당·callback 호출이 필요 없다. 시간·process ID·`rand`·연구 seed로 fallback하지 않는다.

Callback은 제한된 시간 안에 반환하고 제공된 용량을 지켜야 한다. Wrapper는 보고된 개수를 확인할 수 있지만 안전하지 않은 callback 자체를 안전하게 만들 수는 없다. Provider는 개별 소유하며 순차 사용한다. Callback context는 provider보다 오래 살아 있어야 한다. 별도 exact-read source 인터페이스를 통해 sampler가 entropy provider 또는 명시적으로 선택한 연구 stream을 소비한다.

## 운영체제 adapter

Linux는 blocking `getrandom(..., 0)`을 사용하고 callback 요청당 최대 256바이트를 처리한다. `EINTR`는 `INTERRUPTED`로 바꾸며 제한된 재시도와 짧은 read 조립은 공통 provider가 처리한다. 다른 실패는 provider를 닫는다. Windows는 `BCryptGenRandom(NULL, ..., BCRYPT_USE_SYSTEM_PREFERRED_RNG)`을 사용하고 시스템 `bcrypt` library를 연결한다. 두 adapter 모두 device file fallback을 추가하지 않는다. 지원하지 않는 platform은 `PLATFORM_UNAVAILABLE`를 반환한다.

## 연구 stream과 sampling

연구 mode는 명시적이며 결정론적이다. 32바이트 seed·등록 연구 parameter 이름·U32 attempt index를 받아 정확한 `TRIFACT/keygen-attempt`와 `[Name(parameter_id), seed, U32(attempt)]`를 사용한다. 시스템 entropy를 얻지 않는다. 재현 가능한 공개 실험용이며 비밀 운영 key를 위한 entropy source가 아니다. Seed는 absorb하며 별도의 호출자 공개 key component로 보관하지 않는다.

균등 draw는 stream의 다음 8바이트를 big-endian으로 읽는다. `1 <= q <= 2^32`에 대해 크기가 `q`로 나누어지는 최대 U64 초기 구간을 허용한 뒤 나머지를 반환한다. 거부된 tail 값은 새 byte를 소비한다. 호출자가 양수 draw limit을 명시적으로 제공하며 소진하면 결과 없이 `SAMPLING_EXHAUSTED`를 반환한다. 이 값은 구현 종료 예산이며 승인된 KeyGen attempt 256개를 대체하지 않는다.

순열은 동일한 uniform 규칙과 명시적 draw별 예산으로 `[0,n)`에 descending Fisher-Yates를 적용한다. 임시 배열에서 처리하고 완전히 성공했을 때만 출력하며 실패 시 임시 저장소를 비운다. Source rewind나 부분 순열은 노출하지 않는다. 빈 순열을 허용한다. 출력 정점은 U32이며 모든 할당 곱셈을 검사한다.

## 검증과 출처

기본 생명주기 test는 provider 구현과 함께 추가한다. 이후 번호별 작업에서 공식 SHAKE256 known answer, 독립 framing vector, substitution, scripted entropy 중단·실패, sampling 경계, platform 간 연구 재현을 추가한다. 공개 test seed와 vector는 실제 비밀 자료가 아니다. 검사 통과가 TRIFACT의 안전성을 입증하지는 않는다.

- [NIST FIPS 202](https://doi.org/10.6028/NIST.FIPS.202)
- [NIST CAVP secure hashing vector](https://csrc.nist.gov/Projects/cryptographic-algorithm-validation-program/Secure-Hashing)
- [Linux getrandom manual](https://man7.org/linux/man-pages/man2/getrandom.2.html)
- [Microsoft BCryptGenRandom 계약](https://learn.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgenrandom)

`trifact/shake.h`의 streaming API와 내장 backend를 구현했다. `trifact.shake`는 빈 입력 출력·rate 경계를 넘는 binary 입력·1바이트 streaming·잘못된 전이·인자 실패·NULL 안전 해제를 검사한다. Framed hash와 난수는 다음 작업이다.
