# TRIFACT 키 생성 및 서명 인터페이스

[English](KEY_AND_SIGNING_INTERFACES.md) | [한국어](KEY_AND_SIGNING_INTERFACES_KR.md)

> **상태: 규범적 인터페이스 명세**
>
> 이 문서는 키 생성, 서명, 검증 사이의 논리 객체와 정보 흐름을 고정한다. proof system을 구체화하거나 바이트 인코딩을 정의하지는 않는다.

## 1. 목적

초기 설계안은 비밀키를 factor-label 벡터 `c`만으로 표현하면서 서명 인터페이스를 `Sign(sk, M)`으로 제시했다. 그러나 서명자는 공개 hypergraph, canonical edge index, parameter identifier, 새로운 randomness도 알아야 하므로 이 인터페이스는 불완전했다.

이 명세는 다음을 정의해 그 공백을 해결한다.

- 공개키와 비밀키의 논리적 구성;
- parameter 조회 규칙;
- 명시적인 randomness 입력;
- 제한된 KeyGen의 실패 동작;
- 생성된 factor label과 canonical edge index 사이의 대응;
- 서명자가 사용할 수 있는 정보;
- `KeyGen`, `Sign`, `Verify`의 성공 및 실패 동작.

## 2. 범위 경계

이 문서는 논리 값과 인터페이스를 정의한다. 다음 항목은 의도적으로 정의하지 않는다.

- 바이트 인코딩과 필드 너비;
- hash 또는 XOF 선택과 domain string;
- 구체적인 연구 parameter 값;
- proof system, transcript, challenge 또는 signature field;
- 실사용 강화와 외부 API binding.

이 항목들은 이후 명세에서 다룬다. 서명 구현은 이 문서에 없는 내용을 임의로 정해서는 안 된다.

## 3. 공통 타입

해석된 parameter identifier에 대해 다음과 같이 둔다.

```math
V=[n]=\{0,\ldots,n-1\},
\qquad
m=\frac{dn}{3}.
```

논리 타입은 다음과 같다.

```text
ParameterId   등록된 불투명 identifier
Vertex        [0, n) 범위의 정수
Edge          a < b < c를 만족하는 순서 있는 삼중항 (a, b, c)
EdgeList      사전식으로 엄격히 증가하는 m개의 중복 없는 edge 배열
FactorLabel   [0, d) 범위의 정수
LabelVector   m개의 factor label 배열
Message       유한한 바이트열
Signature     proof 명세에서 정의할 불투명 protocol 객체
```

공개키와 비밀키 객체는 다음과 같다.

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

정점 집합은 parameter descriptor에서 암묵적으로 결정되며 별도 목록으로 중복 저장하지 않는다.

## 4. Parameter 해석

모든 연산은 하나의 공통 registry를 통해 `parameter_id`를 해석하는 것으로 시작한다.

```text
ResolveParameters(parameter_id) -> Result<ParameterDescriptor, UnknownParameter>
```

descriptor는 각 연산에 필요한 모든 제한을 제공하며, 최소한 다음 내용을 포함한다.

```text
ParameterDescriptor {
    n
    d
    keygen_attempt_limit
    message_size_limit
    protocol_configuration   // proof layer가 고정될 때까지는 사용할 수 없음
}
```

연구 parameter 명세에서 identifier와 구체적인 제한을 정한다. identifier는 설정 선택자이며 암호학적 강도에 대한 주장이 아니다.

규칙은 다음과 같다.

- `KeyGen`은 parameter identifier를 직접 입력받는다.
- `Sign`은 `sk.public_key`에서 identifier를 얻는다.
- `Verify`는 `pk`에서 identifier를 얻는다.
- 알 수 없거나 비활성화됐거나 불완전한 descriptor는 실패로 처리한다.
- 구현은 기본 descriptor로 임의 대체해서는 안 된다.

## 5. Randomness 인터페이스

키 생성과 서명은 randomness를 명시적으로 입력받는다.

```text
RandomSource.read(length) -> Result<byte[length], RandomnessFailure>
```

source는 새로운 바이트를 반환하거나 명시적인 오류를 반환해야 한다. 짧은 값을 반환하거나 이전 buffer를 조용히 반복하거나 예측 가능한 source로 대체해서는 안 된다.

두 randomness 역할은 논리적으로 분리한다.

```text
keygen_rng   factor 및 정점 순열 sampling
sign_rng     salt 및 모든 proof randomness 생성
```

정확한 결정론적 확장과 domain separation은 인코딩 및 hash domain 명세에서 정의한다. 그전까지 구현은 두 역할 사이에서 raw byte를 재사용할 수 있다고 가정해서는 안 된다.

연구 도구는 재현 가능한 테스트를 위해 결정론적 source를 주입할 수 있다. 결정론적 seed는 비밀키를 재현할 수 있으므로 민감한 정보다. seed는 `pk`나 `sk`의 구성 요소가 아니며, 비밀로 유지할 키와 함께 공개해서는 안 된다.

## 6. 공개키 객체

논리적 공개키는 다음과 같다.

```math
pk=(\mathsf{parameter\_id},E_{\mathrm{can}}),
```

여기서 `E_can`은 R3HFR 명세의 canonical edge list다.

구조적으로 유효한 공개키는 다음 조건을 모두 만족해야 한다.

1. parameter identifier 해석에 성공한다.
2. 해석된 `n`이 3으로 나누어지고 `(n,d)`가 허용 범위에 있다.
3. 모든 edge가 `[n]` 범위의 서로 다른 정점 세 개를 포함한다.
4. 모든 edge 내부가 정렬되어 있다.
5. edge 배열이 사전식으로 엄격히 증가한다.
6. 배열에 정확히 `m = dn/3`개의 edge가 있다.
7. 모든 정점의 차수가 정확히 `d`다.

공개키 검증은 factorization을 찾으려 하지 않는다. 구조적으로 유효한 임의의 키에 witness가 존재하는지 판단하는 것 자체가 recovery 문제이기 때문이다. 정직한 `KeyGen` 출력에는 witness가 있음을 KeyGen이 보장한다.

## 7. 비밀키 객체

논리적 비밀키는 단순한 `c`가 아니라 다음과 같다.

```math
sk=(pk,c).
```

내장된 공개키는 서명자에게 다음 정보를 제공한다.

- 정확한 parameter identifier;
- canonical 공개 hypergraph;
- edge 수와 canonical edge index;
- proof transcript에 binding해야 하는 공개 입력.

label 벡터는 다음을 만족한다.

```math
|c|=|E_{\mathrm{can}}|=m
```

그리고 `c_i`는 canonical index `i`에 있는 공개 edge의 label이다.

유효한 비밀키는 다음을 만족해야 한다.

```math
R_{\mathrm{R3HFR}}(pk.edges,c)=1.
```

`pk`를 포함하면 저장된 키 material에 공개 정보가 중복되지만, 모호한 외부 의존성을 제거하고 `Sign(sk, M, rng)`을 자체 완결적으로 만든다.

## 8. Key pair 일관성

성공한 모든 KeyGen 결과 `(pk, sk)`에 대해 다음은 완전한 논리적 동등 관계다.

```text
sk.public_key == pk
```

또한 다음을 만족한다.

```math
R_{\mathrm{R3HFR}}(pk.edges,sk.labels)=1.
```

공개키와 비밀키를 별도로 import하는 API는 함께 사용하기 전에 공개키 객체 전체를 비교해야 한다. `n`, `d`, edge 개수 또는 외부 파일 이름만 비교하는 것은 충분하지 않다.

다음 helper는 내장 공개키를 재생성하거나 변경하지 않고 그대로 반환한다.

```text
PublicKeyFromSecretKey(sk) -> pk
```

## 9. Canonical edge-label remapping

KeyGen은 공개 edge list를 정렬하기 전에 다음과 같은 label이 붙은 pair를 구성한다.

```text
(generated_edge, factor_label)
```

factor `j`에서 생성된 모든 edge에는 label `j`를 짝지어 둔다. 정점 순열을 적용한 뒤 각 pair를 다음과 같이 변환한다.

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

edge와 그 label은 하나의 pair로 함께 이동한다. edge만 정렬하고 label 배열을 생성 순서에 그대로 두는 것은 잘못이다.

정점 순열은 전단사이므로 duplicate-edge rejection을 통과한 candidate에는 변환 후 중복 edge가 생길 수 없다. 그런데도 이 단계에서 중복이 탐지되면 candidate를 조용히 고치지 않고 내부 invariant failure를 보고한다.

이 remapping 중 factor label 자체의 이름은 바꾸지 않는다. `CanonLabel(c)` 같은 global label normalization은 분석 연산이며 KeyGen의 일부가 아니다.

## 10. KeyGen 인터페이스

인터페이스는 다음과 같다.

```text
KeyGen(parameter_id, keygen_rng) -> Result<KeyPair, KeyGenError>

KeyPair {
    public_key: PublicKey,
    secret_key: SecretKey
}
```

논리적 절차는 다음과 같다.

1. parameter identifier를 해석한다.
2. descriptor가 KeyGen에 필요한 내용을 모두 갖췄는지 검증한다.
3. `keygen_attempt_limit`까지 각 attempt에서 다음을 수행한다.
   1. `keygen_rng`를 사용해 `d`개 factor의 완전한 ordered tuple을 sampling한다.
   2. 동일한 unordered edge가 두 번 이상 나타나면 tuple 전체를 reject한다.
   3. `keygen_rng`를 사용해 정점 순열을 sampling한다.
   4. canonical edge-label remapping을 적용한다.
   5. `pk`와 `sk = (pk, c)`를 구성한다.
   6. 공개키 구조와 witness relation을 검증한다.
   7. key pair를 반환한다.
4. 어떤 attempt도 성공하지 못하면 `SamplingExhausted`를 반환한다.

reject된 attempt도 randomness를 소비한다. 새로운 attempt에서는 새로운 factor tuple 전체와 새로운 정점 순열을 sampling하며 reject된 tuple의 일부를 재사용해서는 안 된다.

## 11. KeyGen 오류

`KeyGenError`는 최소한 다음 오류를 구분한다.

```text
UnknownParameter
InvalidParameterDescriptor
RandomnessFailure
SamplingExhausted
InternalInvariantFailure
```

공개 반환값에는 부분적인 키 material이 포함되지 않는다.

구현은 실패에 대응해 다음 행동을 해서는 안 된다.

- parameter identifier 변경;
- `d` 감소;
- 호출자 모르게 attempt limit 연장;
- 충돌하지 않은 factor만 유지;
- 충돌한 edge만 개별 교체;
- 검증되지 않은 key pair 반환.

진단 도구는 rejection 횟수와 내부 원인을 기록할 수 있지만, 일반적인 실패 출력은 비밀 중간 candidate를 노출해서는 안 된다.

## 12. Sign 인터페이스

인터페이스는 다음과 같다.

```text
Sign(secret_key, message, sign_rng) -> Result<Signature, SignError>
```

`message`는 정확한 바이트열이다. 텍스트 변환, 파일 읽기, 문자 정규화는 이 인터페이스 밖에서 수행한다.

proof 생성 전에 서명자는 다음을 수행한다.

1. `secret_key.public_key.parameter_id`를 해석한다.
2. 공개키 구조를 검증한다.
3. label vector 길이와 범위를 확인한다.
4. 전체 R3HFR witness relation을 검증한다.
5. message 크기와 protocol resource limit를 적용한다.
6. 내장 공개키와 message로 signing context를 구성한다.
7. `sign_rng`에서 새로운 salt와 proof randomness를 얻는다.
8. proof-layer signing procedure가 명세된 후 그 절차를 호출한다.

signature 객체는 이 문서에서 불투명한 상태로 둔다. proof와 transcript 명세가 나오기 전에는 필드를 고정할 수 없다.

## 13. Signing context

향후 proof layer에 전달되는 논리적 context는 다음과 같다.

```text
SigningContext {
    parameter_id
    public_key
    message
}
```

proof layer에 전달되는 witness는 다음과 같다.

```text
Witness {
    labels   // public_key.edges를 index로 사용
}
```

모든 index `i`에 대해 proof circuit은 `labels[i]`를 `public_key.edges[i]`의 label로 해석한다. 이 시점 이후 어떤 module도 edge를 독립적으로 다시 정렬할 수 없다.

완전한 parameter identifier, 공개키, message는 non-interactive transcript에 binding되어야 한다. 정확한 바이트 구성은 canonical encoding 및 hash domain 명세에서 정의한다.

## 14. Sign 오류와 randomness 소비

`SignError`는 최소한 다음 오류를 구분한다.

```text
UnknownParameter
IncompleteProtocolConfiguration
InvalidSecretKey
MessageTooLarge
RandomnessFailure
ProofGenerationFailure
InternalInvariantFailure
```

서명은 하나의 완전한 signature 또는 오류만 반환하며 부분 transcript를 반환하지 않는다.

randomness를 소비한 뒤 서명이 실패했다면 소비된 바이트는 폐기한다. 재시도에는 새로운 randomness가 필요하다. 구현은 부분적으로 생성된 proof transcript를 이어 쓰거나 재사용하거나 노출해서는 안 된다.

서명은 이전 signature에 대해 stateless하다. 이 요구사항이 deterministic signing을 의미하지는 않는다.

## 15. Verify 인터페이스

외부 인터페이스는 모든 입력에 대해 결과를 반환한다.

```text
Verify(public_key, message, signature) -> boolean
```

유효하지 않은 모든 입력에 `false`를 반환하며 caller에게 실패 분류를 요구하지 않는다.

검증은 다음 순서로 수행한다.

1. 공개키의 parameter identifier를 해석한다.
2. canonical 공개키 구조를 검증한다.
3. message, signature, protocol resource limit를 적용한다.
4. signature 객체가 정의된 뒤 그 canonical form을 검증한다.
5. 제공된 공개키와 정확한 message 바이트로 verification context를 재구성한다.
6. transcript에 binding된 모든 값을 다시 계산한다.
7. 이후 proof 명세에 따라 proof를 검증한다.
8. 모든 검사가 성공한 경우에만 `true`를 반환한다.

내부 연구 도구는 진단 결과를 노출할 수 있지만 boolean 인터페이스와 완전히 같은 입력 집합을 accept해야 한다.

검증은 factorization 복구를 시도해서는 안 되며, signature 내부에 복사된 parameter 또는 공개키 데이터가 제공된 `pk`와 충돌할 경우 이를 신뢰해서는 안 된다.

## 16. 검증 helper

구현은 다음 reference check를 외부에 제공하거나 내부에서 공유한다.

```text
ValidatePublicKey(pk) -> Result<(), PublicKeyError>
ValidateSecretKey(sk) -> Result<(), SecretKeyError>
ValidateRelation(pk.edges, sk.labels) -> Result<(), RelationError>
PublicKeyFromSecretKey(sk) -> pk
```

`ValidateSecretKey`는 `ValidatePublicKey`와 `ValidateRelation`을 모두 호출한다. `KeyGen`과 `Sign`은 테스트에서 사용하는 동일한 reference validator를 사용하며, 더 약한 별도 정의를 유지하지 않는다.

이후 proof circuit은 `ValidateRelation`과 정확히 같은 witness relation을 accept해야 한다.

## 17. 동등한 비밀키

임의의 global factor-label permutation `sigma`에 대해 두 비밀키 객체

```math
(pk,c)
\quad\text{와}\quad
(pk,\sigma\cdot c)
```

는 서로 다른 논리 객체지만 같은 공개키에 대한 동등한 witness다.

둘 다 비밀키 검증을 통과해야 하며 둘 다 서명에 사용할 수 있다. 인터페이스는 심어진 factor 이름의 복구 또는 보존을 요구하지 않는다.

first-occurrence representative인 `CanonLabel(c)`는 실험에서 equivalence class를 비교할 때 사용한다. 서명 중 자동 대체하지 않으며 추가 유효성 조건도 아니다.

## 18. 필수 인터페이스 테스트

향후 구현은 최소한 다음을 테스트해야 한다.

- `sk.public_key`가 KeyGen이 반환한 `pk`와 정확히 같다.
- 정점 순열과 edge 정렬 후에도 label vector가 올바르게 정렬된다.
- 적절한 test fixture에서 label을 함께 옮기지 않고 edge만 정렬하면 relation 검증에 실패한다.
- 성공한 모든 KeyGen 결과가 공개키 및 비밀키 검증을 통과한다.
- 결정론적 test source가 같은 논리적 key pair를 재현한다.
- randomness 오류가 key 또는 signature를 반환하지 않는다.
- attempt exhaustion이 parameter fallback 없이 `SamplingExhausted`를 반환한다.
- 내장 공개키를 교체하면 비밀키가 무효가 된다.
- 공개 edge 하나, label 하나 또는 parameter identifier를 변경하면 탐지된다.
- 모든 factor label의 global permutation이 비밀키 유효성을 보존한다.
- `Sign`이 proof 생성 전에 잘못된 비밀키를 거부한다.
- `Verify`가 알 수 없는 parameter와 모든 context mismatch를 거부한다.
- boolean 검증 경로와 진단 검증 경로가 같은 입력을 accept한다.

## 19. 미확정 항목

다음 항목은 의도적으로 아직 정하지 않는다.

- 구체적인 parameter identifier와 수치 제한;
- 공개키, 비밀키, message, signature의 canonical byte;
- seed로부터의 결정론적 확장;
- hash 및 XOF domain;
- proof protocol과 그 설정;
- 구체적인 `Signature` 객체;
- transcript challenge 생성과 soundness parameter.

이 항목들이 고정되기 전까지 인터페이스는 명세로만 존재한다. 특히 불투명한 `Signature` 타입이 구현 중 임의의 proof system을 선택해도 된다는 뜻은 아니다.

## 20. 규범적 요약

수정된 인터페이스는 다음과 같다.

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

서명자는 비밀키에서 완전한 공개 context를 얻는다. canonical edge sorting은 항상 각 edge를 factor label과 함께 이동시킨다. parameter와 randomness는 명시적인 입력이며 모든 실패는 조용한 fallback 없이 보고한다.
