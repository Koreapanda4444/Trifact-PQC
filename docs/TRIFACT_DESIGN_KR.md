# TRIFACT — Post-Quantum Digital Signature Design

[English](TRIFACT_DESIGN.md) | [한국어](TRIFACT_DESIGN_KR.md)

> **상태: 초안 / 실험적 / 미검증**
>
> 이 문서는 연구 제안을 설명하며, 양자내성 안전성 또는 실제 환경에서의 사용 가능성을 주장하지 않는다.

## 1. 개요

**TRIFACT**는 3-uniform hypergraph의 숨겨진 1-factorization을 기반으로 설계하는 실험적 Post-Quantum 전자서명 알고리즘이다.

분류:

**Post-Quantum Digital Signature Algorithm**

핵심 기반 문제:

**Random 3-Uniform Hypergraph Factorization Recovery Problem (R3HFR)**

TRIFACT의 핵심 아이디어는 공개된 hypergraph 안에 존재하는 여러 개의 perfect matching 분해를 비밀키로 사용하는 것이다.

공격자는 원래 생성된 비밀키와 동일한 분해를 찾을 필요가 없다.

**공개 hypergraph의 유효한 1-factorization을 하나라도 찾으면 공격 성공**으로 정의한다.

---

# 2. 수학적 객체

정점 집합을

```math
V=\{0,\ldots,n-1\}
```

이라 한다.

`n`은 3의 배수이다.

TRIFACT에서 사용하는 hypergraph는 모든 hyperedge가 정확히 3개의 정점을 가지는 **3-uniform hypergraph**이다.

```math
H=(V,E)
```

각 edge는

```math
e=\{v_a,v_b,v_c\}
```

형태다.

---

# 3. 1-Factor

3-uniform hypergraph의 **1-factor** `F`를 다음과 같이 정의한다.

```math
F\subseteq E
```

이고,

- `F`의 모든 hyperedge가 서로 vertex-disjoint
- 모든 vertex가 정확히 하나의 edge에 포함

되어야 한다.

따라서

```math
|F|=\frac n3
```

이다.

이러한 hypergraph의 1-factor 및 1-factorization 자체는 기존 조합론에서 연구되는 개념이다.

---

# 4. 1-Factorization

공개 hypergraph가 `d`-regular라고 하자.

TRIFACT에서는 edge 집합 전체가

```math
E=F_1\;\dot\cup\;F_2\;\dot\cup\;\cdots\;\dot\cup\;F_d
```

로 분해된다.

각 `F_i`는 1-factor다.

따라서 모든 vertex는

```math
F_1,\ldots,F_d
```

각각에 정확히 한 번씩 등장한다.

전체 edge 수는

```math
m=\frac{dn}{3}.
```

---

# 5. 비밀 표현

각 hyperedge에 factor 번호를 부여하는 함수

```math
c:E\rightarrow\{0,\ldots,d-1\}
```

를 정의한다.

`c(e)=j`라면

```math
e\in F_j
```

라는 뜻이다.

유효한 witness는 모든 vertex `v`에 대해

```math
\{c(e):v\in e\} = \{0,\ldots,d-1\}
```

를 만족해야 한다.

즉 `v`에 연결된 `d`개의 hyperedge가 모두 서로 다른 factor에 속해야 한다.

이 조건 하나로 전체 edge 집합이 `d`개의 perfect matching으로 분해된다.

---

# 6. 새로운 기반 문제

## Random 3-Uniform Hypergraph Factorization Recovery

약칭:

**R3HFR**

### Instance

TRIFACT KeyGen 분포에서 생성된

```math
d\text{-regular 3-uniform hypergraph }H.
```

factor label은 제거되어 있다.

### Goal

다음을 만족하는 임의의 함수

```math
c':E\rightarrow\{0,\ldots,d-1\}
```

를 찾아라.

모든 `v\in V`에 대해

```math
\{c'(e):v\in e\} = \{0,\ldots,d-1\}.
```

즉:

> 공개 hypergraph를 `d`개의 1-factor로 분해하라.

원래 KeyGen에서 사용한 factorization과 같을 필요는 없다.

---

# 7. 왜 Recovery 문제로 정의하는가

보안성을

> 원래 secret을 복구하기 어렵다

에 두지 않는다.

공격자가 다른 factorization

```math
c'\neq c
```

을 발견해도 서명용 witness로 사용할 수 있기 때문이다.

따라서 보안 가정 자체가 처음부터

```math
\boxed{\text{Find ANY valid factorization}}
```

이다.

이렇게 하면 equivalent-secret 문제를 보안 정의 안으로 포함시킨다.

---

# 8. KeyGen

보안 파라미터에 따라

```math
(n,d)
```

를 선택한다.

정점:

```math
V=\{0,\ldots,n-1\}.
```

각

```math
j\in\{0,\ldots,d-1\}
```

에 대해 독립적인 random permutation

```math
\pi_j\in S_n
```

을 생성한다.

이를 연속된 3개씩 묶는다.

```math
F_j= \{ \{\pi_j(0),\pi_j(1),\pi_j(2)\}, \ldots, \{\pi_j(n-3),\pi_j(n-2),\pi_j(n-1)\} \}.
```

따라서 `F_j`는 자동으로 1-factor다.

모든 factor를 합친다.

```math
E=\bigcup_{j=0}^{d-1}F_j.
```

동일한 hyperedge가 서로 다른 factor에 중복되면 해당 KeyGen instance를 폐기하고 다시 생성한다.

마지막으로 전체 vertex에 독립적인 random permutation

```math
\rho
```

를 적용한다.

이를 통해 factor 생성 순서와 vertex numbering 사이의 직접적인 상관관계를 제거한다.

### 공개키

```math
pk=H=(V,E)
```

### 비밀키

```math
sk=c
```

즉 각 edge가 어느 `F_j`에 속하는지 나타내는 factorization이다.

---

# 9. KeyGen의 중요한 특징

공개키를 보면 다음 사실들은 알 수 있다.

- 3-uniform
- `d`-regular
- edge 수 `dn/3`
- 적어도 하나의 완전한 1-factorization 존재

숨겨지는 것은

```math
E=F_1\dot\cup\cdots\dot\cup F_d
```

라는 **partition 자체**다.

따라서 security-through-obscurity를 사용하지 않는다.

공격자는 KeyGen 알고리즘과 공개키 생성 분포까지 전부 알고 있다고 가정한다.

---

# 10. 난제의 배경

일반적인 `r`-uniform hypergraph에서 perfect matching 문제는 `r\ge3`일 때 계산적으로 어려운 문제군에 속한다. 3-dimensional matching도 NP-complete이며, hypergraph perfect matching의 복잡도에 대한 여러 NP-completeness 결과가 알려져 있다.

하지만 이것은 **TRIFACT의 보안 증명이 아니다.**

TRIFACT에 필요한 주장은 훨씬 강하다.

> KeyGen이 생성하는 특별한 random `d`-regular hypergraph 분포에서 R3HFR가 평균적으로 어렵다.

이것은 별도로 분석해야 한다.

---

# 11. 서명의 기본 전략

비밀 factorization을 서명에 직접 노출하지 않는다.

대신 signer는

```math
C_H(c)=1
```

이라는 NP relation의 witness `c`를 알고 있음을 zero-knowledge proof of knowledge 형태로 증명한다.

여기서 `C_H`는 공개 hypergraph `H`와 edge labeling `c`를 받아 factorization이 올바른지 검사하는 검증 관계다.

---

# 12. Factorization Relation

검증 관계:

```math
R(H,c)=1
```

iff 다음 조건이 모두 성립한다.

### Label Range

모든 edge `e`에 대해

```math
0\le c(e)<d.
```

### Local Uniqueness

모든 vertex `v`와 서로 다른 두 incident edge

```math
e_i,e_j\ni v
```

에 대해

```math
c(e_i)\neq c(e_j).
```

`H`는 `d`-regular이고 사용 가능한 label도 정확히 `d`개이므로, 이 조건이 성립하면 각 vertex에는 모든 label이 정확히 한 번씩 나타난다.

따라서 각 label class가 perfect matching이 된다.

---

# 13. Proof Layer

현재 TRIFACT 설계에서는 새로운 ZK 프로토콜까지 동시에 발명하지 않는다.

R3HFR witness relation을 **MPC-in-the-Head 계열 proof-of-knowledge**로 증명하는 구조를 사용한다.

개념적으로 signer는 factorization verification을 여러 가상의 MPC participant가 공동 실행하는 것처럼 시뮬레이션한다.

비밀 `c`는 participant들 사이에 secret-share된다.

각 participant는 전체 witness를 알지 못한다.

검증 circuit은

```math
R(H,c)
```

를 계산한다.

각 participant의 execution view를 commitment한다.

---

# 14. 한 라운드

한 proof repetition은 개념적으로 다음과 같다.

### 1. Share

비밀 witness

```math
c
```

를 여러 share로 분해한다.

### 2. Simulate

가상의 MPC parties가

```math
R(H,c)
```

를 계산한다.

### 3. Commit

각 party의

- randomness
- input share
- communication
- execution state

에 commitment를 만든다.

### 4. Challenge

commitment와 메시지를 hash하여 verifier challenge를 만든다.

### 5. Open

challenge가 요구하는 일부 MPC view만 공개한다.

### 6. Verify

verifier는 공개된 view의 consistency와

```math
R(H,c)=1
```

계산이 정상적으로 수행됐는지 검사한다.

---

# 15. Non-Interactive Signature

전자서명에서는 verifier가 실제 challenge를 전송하지 않는다.

Fiat-Shamir 계열 변환을 사용한다.

메시지를 `M`이라 하면 challenge는 개념적으로

```math
ch= H( \text{domain} \parallel pk \parallel M \parallel salt \parallel commitments )
```

로 만든다.

따라서 서명은 대략

```math
\sigma= ( salt, commitments, opened\ views, auxiliary\ data )
```

형태다.

---

# 16. 알고리즘 인터페이스

## KeyGen

```math
\operatorname{KeyGen}(params) \rightarrow(pk,sk)
```

출력:

```math
pk=H
```

```math
sk=c.
```

---

## Sign

```math
\operatorname{Sign}(sk,M) \rightarrow\sigma
```

1. `pk`에 대응하는 factorization `c` 사용
2. proof repetitions 생성
3. transcript commitment 생성
4. 메시지 `M`와 transcript로 challenge 생성
5. 요구되는 view 공개
6. signature encoding

---

## Verify

```math
\operatorname{Verify}(pk,M,\sigma) \rightarrow\{0,1\}
```

1. encoding 검사
2. transcript commitment 검사
3. challenge 재계산
4. opened MPC view consistency 검사
5. 공개된 proof execution 검사
6. 모두 통과하면 accept

---

# 17. Correctness

정상 KeyGen이 생성한 비밀키 `c`는 construction상

```math
R(H,c)=1
```

이다.

따라서 정상 signer는 항상 유효한 proof transcript를 생성할 수 있다.

목표:

```math
\Pr[ Verify(pk,M,Sign(sk,M))=1 ]=1.
```

즉 cryptographic correctness failure가 없는 구조를 목표로 한다.

---

# 18. 보안 가정

TRIFACT는 크게 세 층의 가정을 갖는다.

## Assumption A — R3HFR

KeyGen 분포에서 생성한 공개키 `H`만으로 임의의 유효한 factorization `c'`를 찾는 것이 어렵다.

핵심 신규 가정이다.

---

## Assumption B — Proof of Knowledge

유효한 proof를 지속적으로 생성할 수 있는 공격자로부터

```math
R(H,c')=1
```

인 witness를 추출할 수 있어야 한다.

---

## Assumption C — Symmetric Primitives

commitment와 Fiat-Shamir transcript에 사용하는 hash/XOF가 quantum attacker에 대해서도 필요한 보안 수준을 제공해야 한다.

---

# 19. 최종 보안 목표

전자서명으로서 목표하는 보안은

```math
EUF\text{-}CMA
```

이다.

공격자는 선택한 메시지들에 대한 정상 서명을 얻은 후에도 새로운 메시지

```math
M^*
```

에 대해

```math
Verify(pk,M^*,\sigma^*)=1
```

인 서명을 만들 수 없어야 한다.

---

# 20. 예상 보안 논리

최종적으로 만들고 싶은 reduction의 형태는:

```math
\text{TRIFACT Forgery}
```

```math
\Downarrow
```

```math
\text{PoK Extraction}
```

```math
\Downarrow
```

```math
\text{Valid Factorization Recovery}
```

```math
\Downarrow
```

```math
\text{R3HFR Break}.
```

즉 성공적인 signature forgery에서 새로운 factorization witness를 추출할 수 있다면 R3HFR 공격으로 연결한다.

**현재는 아직 정식 security proof가 존재한다고 주장하지 않는다.**

---

# 21. 주요 공격 모델

R3HFR에는 최소 다음 공격을 반드시 고려한다.

### Perfect-Matching Peeling

perfect matching 하나를 찾고 제거하고,

```math
H\rightarrow H-F_1
```

다시 matching을 찾는 방식.

---

### Exact Cover

한 factor를 exact-cover 문제로 변환.

---

### SAT / SMT / ILP

각 edge의 factor label을 변수로 두고

```math
c(e_i)\neq c(e_j)
```

constraint를 직접 푼다.

이 공격은 특히 중요하다.

---

### Global Edge Coloring

R3HFR를 특수한 hypergraph edge-coloring 문제로 직접 해결한다.

---

### Meet-in-the-Middle

vertex 집합 또는 edge 집합을 나누어 partial factorization을 만든 후 연결한다.

---

### Factor Swapping

작은 alternating structure를 찾아 여러 factor 사이의 edge를 교환하면서 새로운 valid factorization을 생성한다.

이는 equivalent witness를 빠르게 만드는 공격으로 이어질 수 있다.

---

### Statistical Recovery

KeyGen이 factor별 random permutation으로 생성한다는 사실을 이용해 동일 factor였던 edge 사이에 통계적인 상관성이 남는지 분석한다.

---

### Automorphism Attack

공개 hypergraph의 automorphism을 이용하여 factor space를 축소한다.

---

### Quantum Search

Grover/amplitude amplification뿐 아니라 quantum walk 또는 quantum backtracking이 factorization search에 제공하는 가속을 고려한다.

---

# 22. 가장 위험한 부분

TRIFACT의 가장 큰 미해결 문제는 명확하다.

> **random union-of-perfect-matchings 분포의 평균적인 factorization recovery가 정말 어려운가?**

일반 hypergraph matching의 worst-case NP-hardness만으로는 부족하다.

KeyGen이 생성하는 hypergraph가 오히려 일반 instance보다 훨씬 쉽게 factorization될 가능성이 있다.

이게 확인되면 TRIFACT는 폐기한다.

---

# 23. 두 번째 위험

factorization의 수가 지나치게 많을 수도 있다.

하나의 공개키가

```math
N_H
```

개의 factorization을 가진다면 공격자가 특정 secret을 찾을 필요가 없으므로 탐색 난이도가 크게 떨어질 수 있다.

따라서 중요한 security quantity 중 하나를

```math
N_H= |\{c:R(H,c)=1\}|
```

로 둔다.

단순히 `N_H=1`을 목표로 하지는 않는다.

대신

```math
\text{valid witness density}
```

와 공격 비용 사이의 관계를 파라미터 선정에 반드시 반영한다.

---

# 24. 세 번째 위험

MPC-in-the-Head를 사용하면 signature가 커질 수 있다.

따라서 TRIFACT의 실용성을 결정하는 것은

- factorization witness 크기
- verification circuit 크기
- proof repetition 수

가 된다.

기반 난제가 강해도 signature가 수십\~수백 KB까지 커지면 표준 경쟁력은 크게 떨어진다.

---

# 25. Parameter Structure

아직 보안 수치가 검증되지 않았으므로 `128/192/256` 같은 이름을 붙이지 않는다.

parameter set은 우선

```math
P=(n,d,\tau,R)
```

로 정의한다.

- `n`: vertex 수
- `d`: factor 수 / vertex degree
- `\tau`: 구조 생성 rejection 조건
- `R`: PoK repetition 수

향후 실제 cryptanalysis 결과에 따라 Category 1/3/5 parameter를 별도로 선정한다.

---

# 26. 초기 설계 방향

기반 문제는 **3-uniform**으로 고정한다.

2-uniform graph로 내려가면 graph perfect matching이 효율적으로 해결되기 때문에 TRIFACT의 핵심 비대칭성이 사라진다.

반대로 uniformity를 지나치게 높이면 public key와 relation circuit이 커진다.

따라서

```math
r=3
```

을 기본값으로 선택한다.

---

# 27. Public Key 표현

canonical representation은 sorted triples의 목록이다.

각 edge

```math
\{a,b,c\}
```

에서

```math
a<b<c
```

로 정렬한다.

전체 edge도 lexicographic order로 정렬한다.

따라서 동일한 hypergraph는 항상 동일한 byte representation을 갖는다.

factor 번호는 공개키에 포함되지 않는다.

---

# 28. Secret Key 표현

secret은 공개키의 각 edge index에 대한 factor label 배열로 정의한다.

논리적으로

```math
sk=(c_0,\ldots,c_{m-1})
```

이다.

factor 이름 자체에는 의미가 없으므로

```math
0,\ldots,d-1
```

의 전체 permutation은 equivalent secret이다.

보안 분석에서 이 symmetry를 반드시 고려한다.

---

# 29. Domain Separation

모든 cryptographic hash context는 분리한다.

논리적 domain:

```text
TRIFACT/KEYGEN
TRIFACT/COMMIT
TRIFACT/MPC
TRIFACT/CHALLENGE
TRIFACT/SIGN
TRIFACT/EXPAND

```

서로 다른 프로토콜 단계가 동일 hash domain을 공유하지 않는다.

---

# 30. Stateless Signing

TRIFACT는 state를 유지하지 않는 서명을 목표로 한다.

각 signature에 독립적인

```math
salt
```

와 proof randomness를 사용한다.

randomness failure에 대비한 hedged generation 구조는 후속 security design에서 다룬다.

---

# 31. Kill Criteria

다음 중 하나가 발견되면 현재 R3HFR 기반 설계를 폐기한다.

### 즉시 폐기

- KeyGen 분포에서 polynomial-time factorization
- practical SAT/ILP attack의 매우 낮은 scaling
- factorization을 드러내는 통계적 distinguisher
- 작은 local swap만으로 factorization을 쉽게 생성
- 공격 비용이 parameter 증가에 따라 충분히 증가하지 않음
- 효과적인 quantum structural algorithm 발견

### 강한 재설계 필요

- Category 1에서 public key가 지나치게 큼
- signature가 현실적으로 감당하기 어려운 크기
- verification circuit이 과도하게 큼
- proof repetition 비용 과다

---

# 32. 기존 PQC와의 차이

TRIFACT의 기반은

- lattice
- LWE/SIS
- error-correcting code decoding
- multivariate polynomial solving
- hash-tree signature
- isogeny

가 아니다.

핵심 secret은

```math
\boxed{\text{hypergraph 1-factorization}}
```

이며 핵심 공격 문제는

```math
\boxed{\text{R3HFR}}
```

이다.

현재 추가 PQ 서명 연구에서도 Matrix Subcode Equivalence처럼 새로운 문제를 직접 제안하고 이를 MPC-in-the-Head/VOLE-in-the-Head 서명으로 연결하는 연구가 존재하므로, **“새 search problem + generic ZK/PoK layer”라는 연구 형태 자체는 정상적인 PQC 연구 방향**이다.

---

# 33. Novelty 상태

현재 확인한 문헌에서는

- hypergraph 1-factorization 자체의 조합론 연구
- hypergraph isomorphism 기반 인증 아이디어
- subgraph isomorphism 기반 PQ signature

등은 존재한다.

하지만 이번 1차 검색에서는

> **random 3-uniform regular hypergraph의 hidden 1-factorization recovery를 핵심 average-case assumption으로 두고 이를 PQ 전자서명으로 구성하는 정확히 동일한 제안**

은 확인하지 못했다.

따라서 현재 표기는:

**Potentially novel — formal prior-art review required**

로 둔다.

세계 최초라고 주장하지 않는다.

---

# 34. 알고리즘 이름

설계가 hypergraph의 **3-uniform factorization**을 중심으로 굳었으므로 알고리즘 이름은

# TRIFACT

로 정한다.

의미:

**TRI**
→ 3-uniform hypergraph

**FACT**
→ factor / factorization

정식 표기:

> **TRIFACT — A Post-Quantum Digital Signature Scheme Based on Random 3-Uniform Hypergraph Factorization Recovery**

기반 난제:

> **R3HFR — Random 3-Uniform Hypergraph Factorization Recovery Problem**

---

# 35. 현재 설계 상태

**Primitive**

Post-Quantum Digital Signature

**Algorithm**

TRIFACT

**Underlying problem**

R3HFR

**Public key**

Randomized `d`-regular 3-uniform hypergraph

**Secret key**

Valid 1-factorization

**Signing principle**

Zero-knowledge proof of knowledge of a valid factorization

**Transformation**

Fiat-Shamir-style non-interactive transformation

**Target**

EUF-CMA security in a post-quantum setting

**Current security status**

Experimental / Unproven

---

# 36. 결론

TRIFACT의 설계 핵심은 단순하다.

```math
\boxed{ \text{random perfect matchings} \rightarrow \text{factor labels 제거} \rightarrow \text{public hypergraph} }
```

비밀키는

```math
\boxed{ \text{hidden 1-factorization} }
```

이고 공격 문제는

```math
\boxed{ \text{public hypergraph에서 아무 valid factorization이나 복구} }
```

이다.

서명자는 factorization 자체를 공개하는 대신 해당 factorization을 알고 있다는 proof-of-knowledge를 메시지에 결합한다.

따라서 TRIFACT의 연구 성공 여부는 결국 하나의 질문으로 결정된다.

> **R3HFR가 KeyGen의 평균적인 random instance에서 고전 및 양자 공격 모두에 충분히 어려운가?**

여기서 YES가 나오면 TRIFACT를 계속 발전시키고, NO가 나오면 기반 문제를 폐기한다.
