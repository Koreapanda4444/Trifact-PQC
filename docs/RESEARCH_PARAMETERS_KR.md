# TRIFACT 연구 Parameter 및 Resource Limit

[English](RESEARCH_PARAMETERS.md) | [한국어](RESEARCH_PARAMETERS_KR.md)

> **상태: 규범적 연구 profile 명세**
>
> 이 profile은 correctness check와 공격 실험을 위해 존재한다. 암호학적 보안 수준을 나타내지 않으며 실제 데이터 보호에 사용해서는 안 된다.

## 1. 목적

R3HFR 분포, 키 인터페이스, canonical encoding을 구현하기 전에 구체적인 제한이 필요하다. 제한 없는 sampling, parsing, solver 실행은 결과를 재현할 수 없게 만들고 malformed input이 통제되지 않은 resource consumption으로 이어지게 할 수 있다.

이 명세는 다음을 정의한다.

- 등록된 연구 parameter identifier;
- 구체적인 `(n,d,m)` 값;
- 각 profile의 목적;
- KeyGen attempt limit;
- key 및 message size limit;
- parser 전체에 적용되는 safety limit;
- recovery solver time 및 memory limit;
- 재현 가능한 benchmark seed set;
- failure, timeout, censored result 기록 규칙.

## 2. 보안 수준이 아닌 이름

등록된 parameter identifier는 다음 세 개뿐이다.

```text
toy
small
medium
```

이 이름은 실험 규모만 설명한다. bit security, NIST category, classical work factor, quantum work factor, deployment recommendation과 대응하지 않는다.

구현과 보고서는 이 profile을 `secure`, `recommended`, `128`, `192`, `256` 같은 이름으로 바꾸면 안 된다.

모든 profile이 쉽게 풀린다면 현재 설계가 viability gate에서 실패했다는 것이 올바른 결과다. solver가 우연히 timeout될 때까지 profile을 반복해서 키워서는 안 된다.

## 3. 공통 수학 요구사항

등록된 모든 profile은 다음을 만족한다.

```math
n\ge6,
\qquad
n\equiv0\pmod3,
\qquad
2\le d\le\binom{n-1}{2},
\qquad
m=\frac{dn}{3}.
```

모든 vertex와 label은 `U32`에 들어간다. 성공한 모든 공개키는 `V=[n]` 위의 simple `d`-regular 3-uniform hypergraph이며 정확히 `m`개의 canonical edge를 갖는다.

이 문법적 조건은 선택된 분포의 factorization이 어렵다는 뜻이 아니다.

## 4. Profile registry

| Parameter ID | `n` | `d` | `m = dn/3` | 주 용도 |
| --- | ---: | ---: | ---: | --- |
| `toy` | 12 | 3 | 12 | Invariant, mutation test, 완전한 witness-class enumeration |
| `small` | 24 | 4 | 32 | Solver 개발, cross-check, statistical smoke test |
| `medium` | 60 | 6 | 120 | 가장 큰 초기 recovery 및 scaling 실험 |

registry는 정확히 고정된다. parameter identifier는 한 행으로 해석되며 runtime에 개별 field를 override할 수 없다.

격리된 exploratory tool은 custom `(n,d)` 값을 허용할 수 있지만, 이는 등록 parameter가 아니며 project key로 encoding할 수 없고 공식 benchmark summary에 섞을 수 없다.

## 5. `toy`의 용도

`toy` profile은 가장 작은 공통 correctness target이다.

필수 용도는 다음과 같다.

- KeyGen 및 codec golden test;
- relation-validator positive 및 negative case;
- canonical edge-label remapping test;
- factor-label permutation test;
- SAT 및 exact-cover solver bring-up;
- 가능할 경우 labeled witness의 완전한 enumeration;
- global label permutation을 quotient한 witness count;
- local-swap 및 automorphism test fixture.

`toy`의 빠른 recovery는 예상된 결과이며 보안 결론을 주지 않는다. `toy` instance를 recover하지 못하는 solver는 더 큰 profile 측정에 사용할 준비가 되지 않은 것이다.

## 6. `small`의 용도

`small` profile은 독립적인 공격 구현의 integration target이다.

필수 용도는 다음과 같다.

- SAT, exact-cover, peeling output 비교;
- recover한 모든 witness가 native relation을 통과하는지 검증;
- 구현 사이의 seed reproducibility;
- malformed-input 및 parser resource test;
- preliminary factor-swap 및 automorphism measurement;
- 가장 큰 초기 실행 전 statistical feature collection.

완전한 enumeration은 선택 사항이며 별도 enumeration limit 안에서만 시도할 수 있다. timeout은 기록하며 추정 witness count로 변환하지 않는다.

## 7. `medium`의 용도

`medium` profile은 R3HFR viability decision 전 사용하는 가장 큰 초기 profile이다.

필수 용도는 다음과 같다.

- `toy` 및 `small`과 recovery cost 비교;
- wall time 및 peak memory 측정;
- 고정 seed set에 대한 전체 baseline solver 실행;
- planted-distribution statistical test;
- flat하거나 너무 느리게 증가하는 scaling 탐지.

`medium`은 deployment 후보 parameter가 아니다. `medium`의 timeout은 censored measurement이며 hardness의 증거가 아니다.

## 8. Parameter descriptor

등록 identifier를 해석하면 다음을 반환한다.

```text
ParameterDescriptor {
    parameter_id
    n
    d
    m
    keygen_attempt_limit
    message_size_limit
    public_key_size
    secret_key_size
    signature_input_limit
    recovery_timeout_seconds
    recovery_memory_limit_mib
    enumeration_timeout_seconds
    enumeration_memory_limit_mib
    protocol_configuration
}
```

descriptor는 immutable하다. `m`은 편의를 위해 저장하지만 registry load 시 checked arithmetic으로 다시 계산해 `dn/3`와 비교해야 한다.

현재 모든 profile에서 다음과 같다.

```text
protocol_configuration = unavailable
```

따라서 `Sign`은 `IncompleteProtocolConfiguration`을 반환하며 `Verify`는 protocol body를 accept할 수 없다. viability gate가 proof-layer 작업을 허용하고 proof 명세가 완전한 설정을 제공할 때까지 이 제한을 유지한다.

## 9. KeyGen attempt limit

현재 모든 profile은 다음을 사용한다.

```text
keygen_attempt_limit = 256
```

attempt는 `0`부터 `255`까지 index한다. 각 attempt는 encoding 명세에서 정의한 독립적인 `TRIFACT/keygen-attempt` stream을 사용한다.

완전한 factor tuple에 duplicate unordered edge가 포함된 경우에만 attempt를 reject한다. 내부 invariant failure는 일반 rejection이 아니라 오류다.

attempt `255`도 reject되면 KeyGen은 `SamplingExhausted`를 반환한다. attempt `256`을 시작하거나 profile을 변경하거나 `d`를 줄이거나 partial tuple을 유지해서는 안 된다.

구현은 연구 진단을 위해 사용된 attempt 수를 기록한다. 이 count는 `pk` 또는 `sk`의 일부가 아니다.

## 10. Expected rejection 진단값

`n`개 정점에서 독립적으로 sampling한 uniform 1-factor 두 개의 공통 edge 기대값은 다음과 같다.

```math
\frac{2n}{3(n-1)(n-2)}.
```

독립적으로 sampling한 `d`개 factor 전체에서 conditioning 전 pairwise edge collision 기대값은 다음과 같다.

```math
\binom d2\frac{2n}{3(n-1)(n-2)}.
```

여러 collision event가 서로 의존하므로 이 값은 진단용 기대값이며 정확한 rejection probability가 아니다.

구현은 성공한 key마다 사용한 attempt 수를 보고해야 한다. 동일한 seed를 사용한 구현 사이에 결과가 크게 다르면 sampling 또는 canonicalization bug가 있다는 뜻이며 이후 benchmark를 차단한다.

## 11. 정확한 key size

canonical encoding 명세에 따른 현재 profile의 정확한 key size는 다음과 같다.

| Parameter ID | Encoded public key | Encoded secret key |
| --- | ---: | ---: |
| `toy` | 185 bytes | 270 bytes |
| `small` | 427 bytes | 592 bytes |
| `medium` | 1,484 bytes | 2,001 bytes |

이 값에는 record name, field framing, 비밀키에 내장된 공개키, edge count, label count가 포함된다.

다른 길이의 encoded key는 전체 객체를 구성하기 전에 거부한다. 정확한 길이가 구조 검증을 대체하지는 않는다.

## 12. Key-size 계산

`m`개 edge의 edge-list payload 길이는 다음과 같다.

```math
4+12m.
```

`m`개 label의 label-vector payload 길이는 다음과 같다.

```math
4+4m.
```

canonical record framing을 사용한 공개키 크기는 다음과 같다.

```text
toy:     41 + 12m = 185
small:   43 + 12m = 427
medium:  44 + 12m = 1,484
```

parameter identifier 길이가 다르므로 상수가 다르다.

비밀키 크기는 다음과 같다.

```math
37+|EncodePublicKey(pk)|+4m.
```

이 식은 codec test에서 반드시 assert해야 한다.

## 13. Message 및 signature input limit

현재 모든 profile은 다음을 사용한다.

```text
message_size_limit  = 1,048,576 bytes
signature_input_limit = 67,108,864 bytes
```

message limit는 API 및 parser bound이며 application에 권장하는 message 크기가 아니다.

signature limit는 이후 proof 실험을 위해 남겨 둔 hard outer allocation cap일 뿐이다. 예상 signature 크기가 아니며 proof configuration이 나오기 전에 signature 구현을 허용하지 않는다.

두 limit 중 하나를 넘는 입력은 완전한 객체를 hash하거나 선언된 크기의 buffer를 할당하기 전에 거부한다.

## 14. 일반 parser limit

모든 key, signature, transcript parser는 profile별 정확한 크기와 함께 다음 제한을 적용한다.

| Resource | Limit |
| --- | ---: |
| 단일 `Blob` payload | 67,108,864 bytes |
| 하나의 `Sequence` element | 1,048,576 |
| 하나의 `Record` field | 64 |
| Nested record depth | 8 |
| Restricted-name length | 32 bytes |
| Total signature input | 67,108,864 bytes |
| Instance당 저장하는 diagnostic solver output | 16,777,216 bytes |

적용 가능한 제한 중 더 작은 값이 항상 우선한다. 예를 들어 공개키 parser는 일반 `Blob` limit 대신 profile의 정확한 key size를 사용한다.

length 및 count 연산에는 checked operation을 사용한다. parser는 allocation 전에 overflow, multiplication overflow, 불가능한 nesting, over-limit declaration을 거부한다.

## 15. Recovery solver limit

표준 single-instance recovery limit는 다음과 같다.

| Parameter ID | Wall-clock timeout | Peak memory limit | Reference thread count |
| --- | ---: | ---: | ---: |
| `toy` | 10 seconds | 512 MiB | 1 |
| `small` | 60 seconds | 1,024 MiB | 1 |
| `medium` | 300 seconds | 2,048 MiB | 1 |

timeout은 solver가 공개 instance를 받기 직전에 시작하며 완전한 candidate witness 또는 terminal status를 반환할 때 끝난다.

instance loading, native witness validation, result serialization은 별도로 측정한다. 보고서는 solver time과 end-to-end time을 모두 포함한다.

추가 thread를 사용하는 solver는 별도 configuration으로 보고해야 한다. 서로 다른 thread count의 결과를 하나의 scaling curve에 합치지 않는다.

## 16. Enumeration limit

Witness-class enumeration은 별도 제한을 사용한다.

| Parameter ID | Enumeration 상태 | Wall-clock timeout | Peak memory limit |
| --- | --- | ---: | ---: |
| `toy` | 가능한 경우 필수 | 300 seconds | 2,048 MiB |
| `small` | 선택적인 exploratory run | 900 seconds | 4,096 MiB |
| `medium` | 초기 grid에서 비활성화 | — | — |

enumerator는 다음 상태를 구분해야 한다.

- complete enumeration;
- timeout;
- memory limit;
- solver failure;
- invalid emitted witness;
- externally interrupted run.

complete enumeration만 정확한 witness-class count를 제공한다. partial count는 lower bound이며 그렇게 표시해야 한다.

## 17. 고정 benchmark instance set

모든 공식 초기 benchmark는 profile마다 정확히 30개의 생성 instance를 사용한다.

`[0,29]` 범위의 index `i`에 대해 32바이트 benchmark seed는 다음과 같다.

```text
BenchmarkSeed(i) = 28 zero bytes || U32(i)
```

instance identifier는 다음과 같다.

```text
toy-000     through toy-029
small-000   through small-029
medium-000  through medium-029
```

각 profile 및 index에서 fixed-seed `RandomSource`는 KeyGen의 단 한 번의 32바이트 요청에 `BenchmarkSeed(i)`를 반환한다. 이후 모든 attempt와 permutation은 canonical KeyGen expansion으로 결정된다.

이 공개 결정론적 seed는 재현 가능한 공격 연구만을 위해 존재한다. 비밀로 유지할 secret key 생성에는 사용해서는 안 된다.

## 18. Generator와 solver 입력 분리

benchmark generator는 다음을 보관할 수 있다.

- profile identifier;
- benchmark index 및 seed;
- 공개키;
- planted label vector;
- attempt count 및 rejection diagnostic.

recovery solver가 받는 것은 다음뿐이다.

- `pk` 안에 이미 포함된 profile identifier;
- canonical 공개키 byte;
- 선언된 resource configuration.

solver에 seed, planted label vector, attempt stream, generation order, rejection trace를 전달해서는 안 된다. 재현을 위해 이 값을 공개하는 것과 solver의 유효 입력으로 사용하는 것은 다르다.

recover된 output은 native relation validator가 accept할 때만 성공이다. planted witness와의 동등성은 선택적인 진단 정보이며 성공 조건이 아니다.

## 19. Benchmark 실행 규칙

비교 가능한 solver run은 다음 순서를 따른다.

1. 고정된 30-instance set을 생성하거나 load한다.
2. 공격 시간을 측정하기 전에 모든 공개키와 planted witness를 검증한다.
3. 비교할 모든 solver를 같은 공개 instance에서 실행한다.
4. profile별 timeout, memory, thread limit를 적용한다.
5. 성공했다고 주장한 모든 recovery를 native relation validator로 검증한다.
6. aggregation 전 raw per-instance result를 보존한다.
7. environment, command, solver setting, source commit을 보고한다.
8. 어려운 instance를 삭제하거나 exhausted KeyGen seed를 조용히 교체하지 않는다.

system load가 결과에 영향을 줄 수 있으면 solver 순서를 instance seed와 독립적으로 rotate하거나 randomize한다. 선택한 schedule을 기록한다.

## 20. 필수 per-instance result

모든 recovery result는 최소한 다음을 기록한다.

```text
instance_id
parameter_id
n
d
m
solver_name
solver_configuration
thread_count
wall_time
end_to_end_time
peak_memory
status
candidate_witness_present
candidate_witness_valid
recovered_class_id_if_available
environment_id
```

`status`는 정확히 다음 중 하나다.

```text
success
unsolved
timeout
memory-limit
solver-error
invalid-witness
interrupted
```

invalid witness를 `unsolved` 또는 `success`로 바꿀 수 없다. 누락된 측정값은 0 대신 명시적으로 표시한다.

## 21. Timeout 및 censoring 규칙

timeout은 설정된 run이 wall-clock limit 안에 끝나지 않았다는 뜻일 뿐이다.

보고서는 다음 규칙을 따른다.

- timeout을 solver error와 별도로 계산한다.
- timeout runtime을 right-censored observation으로 취급한다.
- timeout 값을 실제 solve time으로 보고하지 않는다.
- runtime summary와 success rate를 함께 표시한다.
- 성공한 run만 평균했다면 그 selection을 밝힌다.
- 비교 가능한 instance 전체에서 설정된 limit를 그대로 유지한다.

어려운 seed 하나를 확인한 뒤 timeout을 늘리면 새로운 solver configuration이 되며 별도로 보고해야 한다.

## 22. Dataset의 KeyGen failure 규칙

고정 benchmark seed가 `SamplingExhausted`에 도달하면 generator는 해당 instance identifier의 dataset construction failure를 기록한다.

다음 행동은 금지한다.

- KeyGen이 성공할 때까지 seed 증가;
- random seed로 대체;
- `d` 또는 attempt limit 변경;
- denominator에서 해당 instance 제외;
- 다른 profile의 instance 재사용.

원인을 파악하고 명세를 의도적으로 변경하기 전까지 dataset generation을 차단한다.

## 23. Statistical experiment limit

planted distribution에서 수집하는 statistic은 연구가 사전에 별도 sample set을 선언하지 않는 한 recovery experiment와 같은 30개 공개 instance를 사용한다.

모든 statistic은 다음을 기록한다.

- 정확한 profile 및 instance identifier;
- public data만 사용하는지 여부;
- comparison distribution과 그 sampler;
- 분석 자체에 사용한 random seed;
- 해당할 경우 여러 statistic test에 대한 correction;
- summary 또는 plotting 전 raw value.

planted label을 사용하는 statistic은 설명용 diagnostic이며 public-key-only distinguisher가 아니다.

## 24. 확장 정책

초기 grid는 현재 세 profile의 결과가 완전하거나 blocker가 문서화된 뒤에만 확장할 수 있다.

새 profile에는 다음이 필요하다.

- 새로운 등록 lowercase identifier;
- 정확한 `n`, `d`, derived `m`;
- 명시된 실험 목적;
- KeyGen rejection measurement;
- 정확한 encoded key size;
- parser, timeout, memory, enumeration limit;
- 독립적인 30-instance seed namespace;
- 두 언어 문서와 모든 registry test update.

새 profile은 이전 benchmark result를 소급해 변경하지 않는다. Custom exploratory parameter는 공식 grid 밖에 남는다.

## 25. Gate 해석

초기 profile grid는 즉각적인 붕괴, 구현 오류, flat scaling, 뚜렷한 statistical leakage, trivial witness multiplicity를 탐지하기에 충분하다.

암호학적 보안을 확립하기에는 충분하지 않다. 특히 다음이 적용된다.

- 세 profile을 모두 저비용으로 풀면 `NO-GO` 또는 `REDESIGN`의 강한 근거다.
- solver timeout 하나만으로 `GO`의 근거가 되지 않는다.
- `GO` 결정에는 독립적인 attack family와 앞서 정의한 모든 gate evidence의 합의가 필요하다.
- 세 지점의 trend가 불분명하면 더 큰 실험이 필요할 수 있다.
- viability decision이 허용하기 전에는 proof 또는 signature 구현을 시작하지 않는다.

## 26. 필수 parameter test

구현은 최소한 다음을 테스트해야 한다.

- registry lookup이 정확히 `toy`, `small`, `medium`만 accept한다.
- 알 수 없거나 대소문자가 바뀐 identifier를 거부한다.
- 모든 row가 수학 요구사항을 만족한다.
- `m`의 checked recomputation이 저장된 값과 같다.
- 정확한 encoded public-key 및 secret-key size가 표와 일치한다.
- attempt index가 `255`에서 끝난다.
- attempt exhaustion이 `SamplingExhausted`를 반환한다.
- 같은 benchmark seed가 같은 key pair와 attempt count를 재현한다.
- 서로 다른 benchmark index가 서로 다른 attempt stream을 생성한다.
- public solver에 seed 또는 planted witness field를 전달하지 않는다.
- allocation 전에 parser limit를 적용한다.
- timeout, memory-limit, solver-error, invalid-witness status를 서로 구분한다.
- unavailable protocol configuration이 모든 profile의 signing을 차단한다.

## 27. 규범적 요약

초기 연구 grid는 다음과 같다.

```text
toy     n=12  d=3  m=12
small   n=24  d=4  m=32
medium  n=60  d=6  m=120
```

모든 profile은 최대 256번의 KeyGen attempt, 최대 1 MiB의 message, 고정된 30-instance benchmark seed set을 사용한다. Recovery run은 profile별 time 및 memory limit를 사용하고 timeout을 censored result로 취급한다.

모든 proof configuration은 unavailable 상태다. 이 profile은 R3HFR 연구만을 지원하며 보안을 주장하지 않는다.
