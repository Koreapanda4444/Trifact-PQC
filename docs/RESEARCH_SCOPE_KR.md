# TRIFACT 연구 범위 및 의사결정 단계

[English](RESEARCH_SCOPE.md) | [한국어](RESEARCH_SCOPE_KR.md)

> **상태: 현재 적용되는 연구 정책**
>
> 각 단계를 통과하면 다음 연구 단계로 진행할 수 있다. 이는 암호학적 안전성이 증명되었다는 의미가 아니다.

## 1. 목적

TRIFACT는 Random 3-Uniform Hypergraph Factorization Recovery 문제(R3HFR)가 실험적인 양자내성 전자서명의 기반으로 사용될 수 있는지 연구한다.

완전한 증명 시스템이나 서명 구현에 투자하기 전에 다음 기반 문제를 먼저 판단해야 한다.

> TRIFACT KeyGen이 생성한 hypergraph에서 임의의 유효한 1-factorization을 복구하는 것이 평균적으로 어려운가?

프로젝트는 다음 두 가지 중 하나의 유효한 결과로 종료된다.

1. 현재 R3HFR 구조를 폐기해야 하는 이유를 설명하는 재현 가능한 실패 결과
2. 명확한 정의, 기본 공격 분석, 테스트, 측정 결과와 남은 보안 공백을 갖춘 연구용 프로토타입

## 2. 포함 범위

프로젝트 범위에는 다음이 포함된다.

- R3HFR 인스턴스 분포의 정확한 정의
- 정확한 witness relation과 factor label 동치 규칙
- hypergraph, key, transcript의 canonical representation
- 결정론적인 참조 relation validator
- 연구용 인스턴스를 위한 재현 가능한 KeyGen 구현
- SAT, exact-cover, peeling, local swap, automorphism 및 통계적 공격 baseline
- 복구 비용과 witness 밀도에 대한 실험적 측정
- R3HFR가 타당성 검증 단계를 통과한 경우에만 진행하는 구체적인 MPC-in-the-Head proof 구성
- proof protocol이 완전히 명세된 이후에만 진행하는 참조 `KeyGen`, `Sign`, `Verify` 인터페이스
- 정확성, 실패 사례, mutation 및 parser 테스트
- 재현 가능한 크기 및 실행시간 측정
- 영문 기준 문서와 대응하는 한국어 번역본

## 3. 제외 범위

현재 프로젝트는 다음을 목표로 하지 않는다.

- 실제 환경 배포 또는 실제 데이터 보호
- NIST를 포함한 표준화 제출
- 구체적인 cryptanalysis 이전의 Category 1, 3, 5 보안 주장
- worst-case NP-hardness가 average-case security를 증명한다는 주장
- 기존 proof 계열을 구체화할 수 있는데도 별도의 새로운 zero-knowledge protocol을 동시에 발명하는 작업
- SIMD, GPU, embedded 또는 hardware 최적화 구현
- constant-time 또는 side-channel 방어가 적용된 실사용 코드
- 안정적인 상호운용 형식이나 하위 호환성 보장
- 공식 인증, 외부 감사 또는 구현 적합성 주장
- 완전한 선행연구 조사 없이 TRIFACT가 최초라고 주장하는 행위
- 현재 문서화 단계에서의 라이선스 선정

제외된 항목은 기반 문제와 proof 설계가 계속 유효한 경우에만 나중에 검토할 수 있다.

## 4. 필수 진행 순서

작업은 다음 순서로 진행한다.

1. 문제, witness 동치, 인터페이스, 인코딩 및 연구용 parameter를 정의한다.
2. 수학적 참조 코어를 구현하고 테스트한다.
3. 서로 독립적인 여러 복구 공격을 구현한다.
4. 공격 비용 증가 추세를 측정하고 타당성 결정을 기록한다.
5. `GO` 결정 이후에만 proof protocol을 구체화한다.
6. proof 명세와 circuit이 일치한 이후에만 signature API를 제공한다.
7. 명확한 한계를 포함한 연구 평가 결과를 공개한다.

proof 또는 signature 구현을 이용해 해결되지 않은 R3HFR 결과를 회피하거나 미루거나 숨겨서는 안 된다.

## 5. Gate A — 명세 준비

### 요구사항

수학적 코어를 구현하기 전에 문서에서 다음을 정의해야 한다.

- 정확한 KeyGen sampling distribution
- 모든 유효성 조건과 rejection 조건
- labeled witness 및 전체 factor-label permutation에 대한 동치 관계
- 공격자의 성공 조건
- public key와 secret key의 구성
- `Sign`이 사용할 수 있는 정보
- canonical ordering 및 byte encoding 규칙
- 연구 전용 parameter profile과 resource limit

### 통과 조건

문서에 따라 작성된 두 개의 독립 구현이 추가적인 protocol 규칙을 임의로 만들지 않고 호환되는 객체를 생성할 수 있어야 한다.

### 실패 시 조치

필수 규칙에 모호함이나 모순이 남아 있으면 구현을 중단하고 명세를 먼저 수정한다.

## 6. Gate B — 참조 코어 준비

### 요구사항

참조 코어는 다음을 입증해야 한다.

- 성공한 모든 KeyGen 출력이 3-uniform, simple, `d`-regular 조건을 만족한다.
- edge 수가 정확히 `dn/3`이다.
- 생성된 witness가 native relation validator를 통과한다.
- factor-label permutation이 witness 유효성을 보존한다.
- canonical sorting 이후에도 edge와 label의 대응이 유지된다.
- 동일한 seed로 동일한 연구용 인스턴스가 재현된다.
- malformed 및 non-canonical encoding이 crash 없이 거부된다.

### 통과 조건

모든 연구 profile에서 invariant, boundary, negative 및 round-trip 테스트를 통과한다.

### 실패 시 조치

잘못된 generator나 validator는 이후 결과 전체를 무효화하므로 참조 코어를 수정하기 전까지 공격 측정을 진행하지 않는다.

## 7. Gate C — R3HFR 타당성

이 단계는 핵심 Go/No-Go 판단 지점이다.

### 필수 근거

평가에는 다음이 포함되어야 한다.

- 보고하는 각 비용 증가 추세당 최소 3개의 증가하는 parameter point
- 완전탐색 결과가 존재하는 경우를 제외하고 parameter point당 최소 30개의 독립 seed 인스턴스
- 비교 대상 solver 전체에서 동일하게 사용하는 저장된 instance set
- SAT와 구조적으로 다른 복구 방법 최소 1개
- 계산 가능한 작은 인스턴스에서 전체 label permutation을 quotient한 witness class 열거
- local factor swap, automorphism 및 sampler 종속 통계 검사
- wall time, peak memory, timeout, 성공 여부 및 복구 witness 유효성 기록
- 재현에 필요한 raw result, 환경 정보, solver 설정, 명령어 및 seed

복구된 모든 witness는 native relation validator로 검사해야 한다. Solver가 반환했어도 relation을 통과하지 못한 assignment는 성공으로 계산하지 않는다.

Timeout은 hardness의 증명이 아니라 관측이 중단된 결과로 기록한다.

### 결정 결과

#### `GO`

다음 조건을 모두 만족할 때만 `GO`를 사용한다.

- 직접적인 polynomial-time 복구 방법을 발견하지 못했다.
- 서로 독립적인 공격 계열에서 시험한 parameter grid에 따라 resource cost가 증가한다.
- 큰 연구용 인스턴스가 계속 쉽게 풀리지 않고 사전에 정의한 resource limit에 도달한다.
- 시험한 통계 신호나 local operation이 반복적으로 저비용 전체 복구를 제공하지 않는다.
- 대체 witness class가 복구 문제를 사실상 단순하게 만들지 않는다.
- 저장된 입력과 명령으로 모든 실험을 재현할 수 있다.

`GO`는 현재 구조가 구현한 baseline 공격에 즉시 붕괴하지 않았다는 뜻일 뿐이다. 특정 보안 수준의 근거가 아니다.

#### `REDESIGN`

핵심 relation에는 가능성이 남아 있지만 수정 가능한 설계 선택 때문에 실패한 경우 `REDESIGN`을 사용한다. 예시는 다음과 같다.

- 특정 sampling rule에서 발생한 정보 노출
- parameter 선택으로 인한 과도한 witness 밀도
- 피할 수 있는 canonicalization 또는 representation 정보 노출
- 핵심 가정을 바꾸지 않고 줄일 가능성이 있는 과도한 key, circuit 또는 proof 크기 추정치

변경된 설계는 영향을 받은 가장 앞 단계로 돌아가야 한다. 분포를 수정한 뒤에는 이전 측정값을 근거로 재사용하지 않고 다시 실행한다.

#### `NO-GO`

다음 중 하나가 발견되면 `NO-GO`를 사용한다.

- KeyGen 분포에 대한 직접적인 polynomial-time factorization algorithm
- parameter 증가에도 비용이 낮게 유지되는 재현 가능한 구조적 공격
- 실험적 비용 증가가 불충분한 practical SAT, exact-cover, peeling 또는 결합 공격
- 전체 복구를 저비용으로 끝낼 만큼 충분한 factor 정보를 안정적으로 노출하는 통계적 방법
- 의도한 탐색 비용을 붕괴시키는 작은 local swap 또는 과도한 equivalent witness
- 공격 비용보다 public key 또는 proof 비용이 더 빠르게 증가하는 parameter 구조
- 안전성을 위해 KeyGen, instance distribution 또는 다른 공개 설계 정보를 숨겨야 하는 상황

`NO-GO` 결정이 나오면 현재 R3HFR 구조에 대한 proof 및 signature 개발을 종료하고 실패 결과와 재현 절차를 기록한다.

## 8. Gate D — Proof Protocol 준비

이 단계는 Gate C가 `GO`일 때만 평가한다.

### 요구사항

proof 명세에서 다음을 확정해야 한다.

- 가상 MPC party 수
- sharing 방식 및 computation domain
- 정확한 relation circuit
- party randomness 및 view 구성
- commitment 입력과 hash/XOF 선택
- challenge space 및 생성 방식
- 공개할 view의 정확한 집합
- verifier consistency check
- repetition당 soundness error 및 전체 repetition 수
- salt 및 transcript binding
- Fiat-Shamir, QROM, proof-of-knowledge 및 EUF-CMA 논리의 적용 범위와 공백

### 통과 조건

생성하거나 변형한 모든 테스트 입력에서 native relation과 proof circuit의 결과가 일치해야 하며, 구현자가 임의로 결정해야 하는 protocol 항목이 문서에 남아 있지 않아야 한다.

### 실패 시 조치

proof protocol이 완전하고 내부적으로 일관될 때까지 signature API 구현을 진행하지 않는다.

## 9. Gate E — 연구용 공개 준비

### 요구사항

- `KeyGen`, `Sign`, `Verify`가 문서화된 format과 transcript 규칙을 사용한다.
- 결정론적 test hook으로 재현 가능한 known-answer vector를 생성한다.
- 잘못된 message, key, truncation, extension, component substitution 및 transcript mutation을 거부한다.
- parser가 malformed input에 대해 fail-closed로 동작하고 명시적인 크기 제한을 적용한다.
- fuzz 및 mutation test가 crash나 통제되지 않은 allocation 없이 끝난다.
- public key, secret key, signature 크기와 KeyGen, Sign, Verify 측정값을 공개한다.
- 보안 문서에서 시험한 특성, 가정, 미해결 문제 및 지원하지 않는 주장을 구분한다.
- 영문과 한국어 문서가 동일한 결정을 설명한다.

### 통과 조건

필수 테스트와 재현성 검사를 모두 통과하고, 결과물이 실제 데이터를 보호해서는 안 되는 실험적 연구 프로토타입임을 명확히 표시한다.

## 10. 측정 규칙

모든 cryptanalysis 측정은 다음 규칙을 따른다.

- parameter profile과 seed로 식별하는 변경 불가능한 input set을 사용한다.
- 비교 가능한 solver에는 같은 timeout과 resource accounting을 적용한다.
- solver failure, timeout 및 invalid recovered witness를 구분해서 기록한다.
- 평균만 남기지 않고 instance별 raw result를 보존한다.
- 가장 빠른 결과만이 아니라 median과 tail behavior를 함께 보고한다.
- recovered witness와 planted witness를 비교하기 전에 전체 factor label을 정렬한다.
- raw labeled witness와 factorization equivalence class를 별도로 계산한다.
- hardware, operating system, runtime, solver 및 dependency 정보를 기록한다.
- generator, relation, encoding 또는 solver model이 바뀌면 영향을 받은 실험을 다시 실행한다.

## 11. 변경 관리

- 영문 문서를 기준으로 하며 한국어 번역본은 `_KR` 접미사를 사용한다.
- 보안 관련 결정은 commit message나 대화에만 남기지 않고 저장소 문서에 기록한다.
- KeyGen distribution, witness relation, canonical encoding, transcript binding 또는 proof challenge가 변경되면 해당 규칙에 의존하는 결과는 무효가 된다.
- 새로운 근거가 이전 결론과 충돌하면 통과한 단계도 다시 열 수 있다.
- `GO` 결정 이후에도 필요한 경우 설계를 폐기할 수 있다.
- 라이선스는 구현과 기여 구조가 더 명확해질 때까지 의도적으로 보류한다.

## 12. 현재 프로젝트 상태

현재 프로젝트는 Gate A 이전 단계다.

다음 문서에서 아래 항목을 정의해야 한다.

1. R3HFR 분포와 witness 동치
2. key generation 및 signing interface
3. canonical encoding 및 hash domain
4. 연구용 parameter profile과 resource limit

현재 상태에서는 proof 구현, signature 구현 또는 보안 수준 주장을 진행하지 않는다.
