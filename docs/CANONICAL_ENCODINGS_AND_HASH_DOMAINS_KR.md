# TRIFACT Canonical Encoding 및 Hash Domain

[English](CANONICAL_ENCODINGS_AND_HASH_DOMAINS.md) | [한국어](CANONICAL_ENCODINGS_AND_HASH_DOMAINS_KR.md)

> **상태: 규범적 인코딩 명세**
>
> 이 문서는 canonical binary framing, 엄격한 decoding, randomness expansion, domain-separated hash 입력을 고정한다. signature의 내부 proof body는 정의하지 않는다.

## 1. 목적

TRIFACT의 모든 구현은 같은 논리 객체에 대해 같은 바이트를 생성해야 한다. 키, message, commitment, challenge가 최종적으로 hash되므로 구조적 동등성만으로는 충분하지 않다.

이 명세는 다음을 정의한다.

- unsigned integer encoding;
- length-prefixed byte string과 sequence;
- parameter identifier encoding;
- canonical record framing;
- edge, edge list, label vector, 공개키, 비밀키 encoding;
- message와 signature 외부 framing;
- malformed 또는 non-canonical 입력의 엄격한 rejection;
- 결정론적 randomness expansion;
- 공통 hash 및 XOF primitive;
- 현재 필요한 모든 domain string과 순서가 고정된 입력;
- 필수 parameter, 공개키, message binding.

## 2. Byte 및 정수 규칙

`byte`는 `[0,255]` 범위의 정수다. 이어 붙이기는 `||`로 표시한다. 모든 unsigned integer는 고정 너비 big-endian encoding을 사용한다.

```text
U8(x)   1바이트
U16(x)  2바이트, 최상위 바이트부터
U32(x)  4바이트, 최상위 바이트부터
U64(x)  8바이트, 최상위 바이트부터
```

encoder는 선택한 너비를 벗어난 값을 거부한다. decoder는 정확히 필요한 바이트 수만 accept하며 signed, variable-width, decimal text, hexadecimal text, little-endian 대체 표현을 허용하지 않는다.

예시:

```text
U8(5)          = 05
U16(258)       = 0102
U32(3)         = 00000003
U64(256)       = 0000000000000100
```

## 3. Length-prefixed byte

바이트열 `X`에 대해 다음을 정의한다.

```text
Blob(X) = U32(len(X)) || X
```

표현 가능한 payload 최대 길이는 `2^32 - 1`이지만, 모든 parser는 메모리를 할당하기 전에 선택된 parameter descriptor의 더 작은 resource limit를 적용한다.

예시:

```text
Blob(empty) = 00000000
Blob("abc") = 00000003 616263
```

예시에서 따옴표로 표시한 text는 ASCII로 인코딩한다. field가 달리 명시하지 않는 한 protocol 객체는 byte이며 문자 변환이나 정규화를 거치지 않는다.

## 4. Canonical sequence

순서가 있는 바이트열 sequence `X[0], ..., X[k-1]`에 대해 다음을 정의한다.

```text
Sequence(X) = U32(k) || Blob(X[0]) || ... || Blob(X[k-1])
```

sequence 순서는 유의미하다. schema는 element의 정렬 방식을 명시해야 한다. decoder는 메모리를 할당하기 전에 element count와 payload length가 resource limit를 넘는지 검사한다.

set은 iteration order로 인코딩하지 않는다. element를 지정된 canonical order로 변환한 뒤 sequence로 인코딩해야 한다.

## 5. 제한된 이름

Parameter identifier, record name, proof component name에는 ASCII만 사용한다.

제한된 이름은 다음과 일치해야 한다.

```text
[a-z][a-z0-9-]{0,31}
```

encoding은 다음과 같다.

```text
Name(s) = U8(len(s)) || ASCII(s)
```

이름은 대소문자를 구분한다. uppercase text, underscore, whitespace, Unicode, 빈 이름, 32바이트보다 긴 이름은 거부한다.

구체적인 parameter identifier는 연구 parameter 명세에서 등록한다. decode됐지만 등록되지 않은 identifier는 유효하지 않다.

## 6. Canonical record

record는 제한된 이름 하나와 번호가 붙은 field의 ordered list로 구성한다.

```text
Field(id, payload) = U16(id) || Blob(payload)

Record(name, fields) =
    Name(name)
    || U16(field_count)
    || Field(id_0, payload_0)
    || ...
    || Field(id_t, payload_t)
```

canonical record 규칙은 다음과 같다.

- field identifier는 `[1,65535]` 범위다.
- identifier는 엄격히 증가한다.
- 같은 identifier가 두 번 나타날 수 없다.
- schema는 모든 필수 field와 정확한 type을 고정한다.
- 누락, 반복, 순서 오류, 알 수 없는 field는 거부한다.
- field를 조용히 무시하지 않는다.
- record parser는 입력 전체를 소비해야 한다.

record name은 한 객체 type이 다른 객체 type으로 decode되는 것을 막는다. field number는 모호하지 않은 framing을 제공하지만 명세되지 않은 데이터를 accept하기 위한 확장 장치가 아니다.

## 7. Parameter identifier encoding

`ParameterId`의 canonical encoding은 다음과 같다.

```text
EncodeParameterId(parameter_id) = Name(parameter_id)
```

decoder는 이름 문법 검증과 registry lookup을 모두 수행한다. fallback parameter와 prefix matching은 없다.

## 8. Edge encoding

edge `e = (a,b,c)`에 대해 다음을 정의한다.

```text
EncodeEdge(e) = U32(a) || U32(b) || U32(c)
```

encoding 전과 decoding 후 edge는 다음을 만족해야 한다.

```math
0\le a<b<c<n.
```

encoder는 잘못된 입력을 caller 대신 정렬하지 않는다. 순서가 다른 triple은 serialization 중 정규화하지 않고 거부한다.

## 9. Canonical edge-list encoding

canonical edge list를 다음과 같이 둔다.

```math
E_{\mathrm{can}}=(e_0,\ldots,e_{m-1}).
```

encoding은 다음과 같다.

```text
EncodeEdgeList(E_can) =
    U32(m)
    || EncodeEdge(e_0)
    || ...
    || EncodeEdge(e_(m-1))
```

decoder는 list를 accept하기 전에 parameter identifier에서 `(n,d)`를 해석하며 다음을 요구한다.

- `m = dn/3`가 정확히 성립한다.
- 모든 edge가 유효하고 내부적으로 정렬되어 있다.
- 사전식 순서로 `e_i < e_(i+1)`을 만족한다.
- duplicate edge가 없다.
- 모든 정점의 차수가 정확히 `d`다.
- `n`, `d`, `m`이 지정된 제한에 맞는다.

edge list에는 다른 sparse, text, set, adjacency, compressed canonical form이 없다.

## 10. Label-vector encoding

label vector `c = (c_0, ..., c_(m-1))`에 대해 다음을 정의한다.

```text
EncodeLabelVector(c) =
    U32(m)
    || U32(c_0)
    || ...
    || U32(c_(m-1))
```

모든 label은 `[0,d)` 범위여야 한다. `i`번째 label은 내장된 canonical 공개키의 `i`번째 edge에 대응한다.

encoder는 주어진 factor name을 보존하며 `CanonLabel`을 적용하지 않는다. 따라서 global permutation된 유효 witness는 같은 witness equivalence class에 속하더라도 서로 다른 비밀키 encoding을 갖는다.

## 11. 공개키 encoding

canonical 공개키 바이트는 다음과 같다.

```text
EncodePublicKey(pk) = Record(
    "trifact-public-key",
    [
        (1, EncodeParameterId(pk.parameter_id)),
        (2, EncodeEdgeList(pk.edges))
    ]
)
```

encoding에는 `ValidatePublicKey(pk)` 성공이 필요하다. decoding 후에도 같은 구조 검증을 적용한다.

parameter identifier는 공개키의 일부다. 서로 다른 identifier에서 edge list가 같더라도 서로 다른 공개키이며 encoding도 다르다.

## 12. 비밀키 encoding

canonical 비밀키 바이트는 다음과 같다.

```text
EncodeSecretKey(sk) = Record(
    "trifact-secret-key",
    [
        (1, EncodePublicKey(sk.public_key)),
        (2, EncodeLabelVector(sk.labels))
    ]
)
```

encoding에는 `ValidateSecretKey(sk)` 성공이 필요하다. decoding은 내장 공개키, label-vector 길이와 범위, 완전한 R3HFR relation을 검증한다.

비밀키 decoder는 자체 완결적인 `SecretKey` 하나를 반환한다. 외부 공개키를 내장 객체의 대체물로 accept하지 않는다.

## 13. Message encoding

`Sign`과 `Verify`는 정확한 message 바이트열 `M`을 입력받는다. transcript에서의 canonical 표현은 다음과 같다.

```text
EncodeMessage(M) = Blob(M)
```

인터페이스는 text decoding, Unicode normalization, newline conversion, JSON parsing, file metadata inclusion, prehash selection을 수행하지 않는다.

빈 message도 유효한 바이트열이다. 단 이후 protocol rule에서 명시적으로 제외할 수 있다. 설정된 message-size limit는 전체 입력을 할당하거나 hash하기 전에 검사한다.

## 14. Signature 외부 encoding

proof 명세에서 `protocol_body`를 정의한다. signature 외부 framing은 지금 고정한다.

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

salt는 정확히 32바이트다. signature parameter identifier는 `pk.parameter_id`와 같아야 하며, `Verify`는 mismatch를 거부한다.

`EncodeProtocolBody` 자체도 proof 명세에서 schema가 고정된 하나의 canonical record여야 한다. 그 schema가 존재하기 전에는 완전한 signature encoding이 존재하지 않으며 서명은 계속 차단된다.

## 15. Transcript component encoding

구조화된 모든 transcript component는 `Record`를 사용한다. 반복되는 모든 collection은 proof protocol에서 정한 순서에 따라 `Sequence`를 사용한다.

다음 정렬 규칙은 이미 적용된다.

- repetition index는 `0`부터 증가한다.
- 각 repetition 안에서 party index는 `0`부터 증가한다.
- commitment는 `(repetition_index, party_index)` 순서로 인코딩한다.
- opened view는 challenge 순서를 따르고 동률이면 party index 순서로 정렬한다.
- index를 포함하는 component는 자체 record 안에 해당 index를 포함한다.
- map, hash table, filesystem 순서, thread completion 순서는 serialization 순서가 될 수 없다.

proof 명세는 모든 commitment, view, opening, challenge, auxiliary value에 record name과 field identifier를 할당해야 한다. 이 framing을 구현별 serializer로 바꿀 수 없다.

## 16. 엄격한 decoding

decoder는 parse, bound, validate, consume-all 규칙을 따른다.

1. 다음 bounded field를 판단하는 데 필요한 바이트만 읽는다.
2. 선언된 length와 count를 parameter 및 global limit와 비교한다.
3. limit를 넘으면 allocation 전에 거부한다.
4. 요구된 순서로 field를 parse한다.
5. canonical form과 수학적 범위를 검증한다.
6. 객체가 끝난 뒤 바이트가 하나라도 남으면 거부한다.

최소한 다음 입력은 decoding에서 거부한다.

- 잘린 integer, name, blob, record, sequence;
- 잘못된 ASCII name;
- 등록되지 않은 parameter identifier;
- 0, 중복, 감소, 누락 또는 알 수 없는 record field identifier;
- 일치하지 않는 count 또는 length;
- `m = dn/3` 계산의 integer overflow 및 arithmetic overflow;
- 잘못됐거나 중복됐거나 정렬되지 않은 edge;
- 범위를 벗어난 label 또는 잘못된 label count;
- 유효하지 않은 secret witness;
- signature parameter mismatch;
- 큰 memory allocation 전의 oversized input;
- trailing byte.

malformed input은 일반적인 decoding failure를 발생시킨다. panic, partial object, implicit repair, fallback parser를 발생시켜서는 안 된다.

## 17. Decode-and-reencode 규칙

지원하는 각 객체 type과 accept된 모든 바이트열 `B`에 대해 다음이 성립해야 한다.

```text
Encode(Decode(B)) == B
```

반대로 모든 유효한 논리 객체 `X`에 대해 다음이 성립해야 한다.

```text
Decode(Encode(X)) == X
```

parser가 하나의 논리 객체에 두 바이트열을 accept할 수 있다면, 둘 중 최소 하나는 non-canonical이며 반드시 거부해야 한다.

## 18. Hash 및 XOF primitive

TRIFACT는 공통 연구 hash 및 XOF 인터페이스에 `SHAKE256`을 사용한다.

이미 canonical한 field byte의 ordered list `F`에 대해 다음을 정의한다.

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

그리고 다음을 정의한다.

```text
H32(domain, F) = SHAKE256(HashFrame(domain, F), 32 bytes)

XOF(domain, F, length) =
    SHAKE256(XofFrame(domain, F) || U32(length), length bytes)

XOFStream(domain, F) =
    SHAKE256(XofFrame(domain, F))가 내보내는 byte stream
```

`XOF`는 더 작은 resource limit 범위 안에서 `[0, 2^32 - 1]` 길이를 허용한다. 요청 길이를 입력에 포함하므로 서로 다른 fixed-length 요청은 구별된다. `XOFStream`은 sequential consumer가 명시된 곳에서만 사용한다.

domain string은 아래 registry의 대소문자를 구분하는 정확한 ASCII constant다. protocol code는 등록되지 않은 domain을 accept하지 않는다.

## 19. Digest 및 context 정의

다음을 정의한다.

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

parameter identifier는 공개키 안과 명시적인 context field에 모두 나타난다. 이 중복은 의도된 것이다. 모든 서명과 검증 transcript를 하나의 등록된 parameter 설정에 명확히 한정한다.

어떤 signature도 message만으로, 공개키 파일 이름으로, 또는 축약된 non-canonical 표현으로 challenge를 생성할 수 없다.

## 20. KeyGen randomness expansion

`KeyGen`은 정확히 32바이트를 한 번 얻는다.

```text
keygen_seed = keygen_rng.read(32)
```

0부터 시작하는 attempt index `a`에 대해 독립적인 attempt stream을 정의한다.

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

attempt limit는 `U32` 범위에 들어야 한다. factor permutation은 factor index 증가 순서로 이 stream에서 sampling한 뒤 정점 순열을 sampling한다. reject된 attempt는 남은 stream을 폐기한다. 다음 attempt는 `a + 1`을 사용하며 reject된 attempt가 소비한 바이트 수에 영향을 받지 않는다.

`1 <= q <= 2^32`에서 uniform integer를 뽑으려면 stream의 다음 8바이트를 `x = U64(bytes)`로 해석하고 다음과 같이 둔다.

```math
L=2^{64}-(2^{64}\bmod q).
```

`x < L`이면 accept하고 `x mod q`를 반환하며, 아니면 다시 8바이트를 뽑는다. permutation은 descending Fisher-Yates 순서를 사용하고 `i = n-1, ..., 1`에 대해 `q = i + 1`로 이 규칙을 호출한다.

이 규칙은 modulo bias를 제거하고 주어진 `keygen_seed`가 구현 사이에서 같은 결과를 만들도록 한다.

## 21. Signing randomness expansion

`Sign`은 비밀키와 message limit를 검증한 뒤 정확히 32개의 새로운 바이트를 한 번 얻는다.

```text
sign_seed = sign_rng.read(32)
```

공개 salt를 다음과 같이 생성한다.

```text
salt = XOF(
    "TRIFACT/sign-salt",
    [context_digest, sign_seed],
    32
)
```

proof component는 다음 함수로 독립적인 바이트를 생성한다.

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

proof 명세는 모든 `component_name`과 필요한 length를 등록한다. 두 의미에 같은 component name을 재사용해서는 안 된다.

seed는 signature에 포함하지 않는다. `sign_rng.read(32)`가 실패하거나 길이가 다르면 signature 없이 서명에 실패한다.

## 22. Commitment 및 challenge domain

canonical encoding된 party view와 그 commitment randomizer에 대해 다음을 정의한다.

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

`ordered_commitments`에 모든 32바이트 view commitment를 `(repetition_index, party_index)` 순서로 넣는다. 다음을 정의한다.

```text
commitment_root = H32(
    "TRIFACT/commitment-root",
    [
        context_digest,
        Sequence(ordered_commitments)
    ]
)
```

challenge byte stream은 다음과 같다.

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

`EncodeView`와 `EncodeChallengeAux`는 proof 명세에서 정의하는 canonical record다. auxiliary value가 필요하지 않다면 field를 생략하는 대신 하나의 명시적인 empty record를 정의해야 한다.

proof 명세는 `challenge_stream`에서 unbiased challenge를 sampling하는 방식을 정의한다. context의 다른 일부만 hash하도록 변경할 수 없다.

## 23. Domain registry

등록된 정확한 domain은 다음과 같다.

| Domain | Primitive | 순서가 고정된 입력 | 목적 |
| --- | --- | --- | --- |
| `TRIFACT/pk-digest` | `H32` | encoded public key | 간결한 공개키 binding |
| `TRIFACT/message-digest` | `H32` | encoded message | 정확한 message binding |
| `TRIFACT/signing-context` | `H32` | parameter ID, public-key digest, message digest | 공통 signing context |
| `TRIFACT/keygen-attempt` | `XOFStream` | parameter ID, keygen seed, attempt index | 재현 가능한 KeyGen sampling |
| `TRIFACT/sign-salt` | `XOF` | context digest, signing seed | 공개 per-signature salt |
| `TRIFACT/proof-expand` | `XOF` | context digest, salt, signing seed, repetition index, component name | 독립적인 proof randomness |
| `TRIFACT/view-commitment` | `H32` | context digest, repetition index, party index, encoded view, randomizer | party-view commitment |
| `TRIFACT/commitment-root` | `H32` | context digest, ordered commitments | transcript commitment root |
| `TRIFACT/challenge` | `XOFStream` | context digest, salt, commitment root, encoded challenge auxiliary data | Fiat-Shamir challenge stream |

이 표는 규범적이다. spelling, capitalization, separator, primitive 선택, 입력 순서, framing이 정확히 일치해야 한다.

domain 추가에는 명세 변경이 필요하다. 기존 domain에 비공식 suffix를 이어 붙이는 것은 허용하지 않는다.

## 24. Binding 요구사항

모든 signature challenge는 다음 항목에 전이적으로 binding된다.

- 등록된 parameter identifier;
- 완전한 canonical 공개키;
- 정확한 message 길이와 바이트;
- 32바이트 signature salt;
- 순서가 고정된 모든 commitment;
- proof protocol에서 정의한 모든 challenge auxiliary data.

binding된 값이 하나라도 바뀌면 context, 해당되는 commitment, challenge를 다시 계산해야 한다.

verifier는 자체 `pk`와 `M`에서 `pk_digest`, `message_digest`, `context_digest`를 재구성한다. signature가 제공하는 digest를 신뢰하지 않는다.

## 25. 필수 encoding 테스트

구현은 최소한 다음 golden-byte 및 negative test를 포함해야 한다.

- 모든 fixed-width integer 경계;
- 빈 blob 및 sequence와 비어 있지 않은 blob 및 sequence;
- 유효하고 잘못된 restricted name;
- 공개키 및 비밀키 round trip;
- 서로 다른 edge 또는 field 순서;
- duplicate edge 및 record field;
- 누락되거나 알 수 없는 record field;
- 모든 바이트 위치에서 잘린 입력;
- accept된 모든 입력이 같은 바이트로 reencoding됨;
- trailing-byte rejection;
- allocation 전에 oversized declared length 거부;
- 유효하거나 범위를 벗어났거나 대응이 틀렸거나 길이가 잘못된 label vector;
- 서로 다른 parameter identifier가 서로 다른 public-key digest 생성;
- 공개키 또는 message의 1비트 변경이 signing context 변경;
- 동일한 KeyGen seed가 동일한 논리적 key pair 재현;
- 서로 다른 attempt index가 서로 다른 KeyGen stream 생성;
- 동일한 signing 입력과 signing seed가 동일한 salt 및 proof byte 재현;
- signing context 변경이 서로 다른 derived value 생성;
- 같은 field byte에서 등록된 각 domain의 출력이 다른 모든 domain과 구별됨.

codec이 완료된 것으로 판단하기 전에 이 명세에서 생성한 hash 및 XOF test vector를 구현 테스트와 함께 commit해야 한다.

## 26. 명시적 비주장

이 명세는 다음을 주장하지 않는다.

- canonical encoding이 R3HFR를 어렵게 만든다.
- `SHAKE256`만으로 signature 보안이 증명된다.
- 32바이트 digest가 TRIFACT 전체의 보안 수준을 확립한다.
- domain separation이 proof of knowledge 또는 Fiat-Shamir 분석의 공백을 메운다.
- 정의되지 않은 protocol body를 proof 명세 전에 안전하게 구현할 수 있다.

이 명세는 연구 설계에서 바이트 수준의 모호성과 cross-context reuse를 제거할 뿐이다.

## 27. 규범적 요약

TRIFACT는 fixed-width big-endian integer, length-prefixed byte, 엄격히 정렬된 record, canonical edge order, consume-all decoding을 사용한다.

key encoding은 다음과 같다.

```text
public key = Record("trifact-public-key", parameter ID, edge list)
secret key = Record("trifact-secret-key", complete public key, label vector)
```

공통 signing context는 다음과 같다.

```text
context_digest = H32(
    "TRIFACT/signing-context",
    parameter ID || public-key digest || message digest
)
```

모든 commitment와 challenge는 등록된 domain을 통해 해당 context를 포함한다. non-canonical input, unknown field, resource-limit violation, trailing byte는 거부한다.
