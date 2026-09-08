# R3HFR 분포 및 Witness 명세

[English](R3HFR_SPECIFICATION.md) | [한국어](R3HFR_SPECIFICATION_KR.md)

> **상태: 연구 명세**
>
> 이 문서는 TRIFACT가 사용하는 인스턴스 분포, witness relation 및 witness 동치를 정의한다. R3HFR가 어렵다고 주장하지 않는다.

## 1. 목적

R3HFR는 **Random 3-Uniform Hypergraph Factorization Recovery**의 약자다.

이 문제에서 공격자는 TRIFACT KeyGen이 생성한 공개 hypergraph의 유효한 1-factorization을 하나라도 복구해야 한다. 복구한 factorization은 sampling 과정에서 사용한 planted factorization과 달라도 된다.

이 문서는 KeyGen, relation validator, recovery solver 및 benchmark가 공유하는 수학적 분포와 성공 조건을 고정한다.

## 2. 표기법

정점 집합을

```math
V=\{0,\ldots,n-1\}
```

로 정의하고, 서로 다른 정점 세 개로 이루어진 모든 unordered triple의 집합을

```math
\binom{V}{3}=\{e\subseteq V:|e|=3\}
```

로 정의한다.

다음 표기법을 사용한다.

- `n`: 정점 수
- `d`: planted factor 수이자 각 정점의 degree
- `m`: 공개 edge 수
- `S_n`: `V` 위의 permutation group
- `[d]`: label 집합 `\{0,\ldots,d-1\}`
- `H=(V,E)`: 공개 3-uniform hypergraph
- `F_j`: label `j`를 가진 planted factor
- `c`: factorization을 나타내는 edge-label vector

유효한 인스턴스에서는

```math
m=|E|=\frac{dn}{3}
```

이다.

## 3. Parameter 범위

문법적으로 유효한 parameter pair `(n,d)`는 다음을 만족한다.

```math
n\ge6,
```

```math
n\equiv0\pmod3,
```

```math
2\le d\le\binom{n-1}{2}.
```

simple hypergraph에서 하나의 정점은 최대 `\binom{n-1}{2}`개의 서로 다른 3-edge에 포함될 수 있으므로 마지막 상한은 필요한 조건이다. 하지만 이 조건만으로 KeyGen의 rejection probability가 허용 가능한 수준이라는 사실은 보장되지 않는다.

구체적인 연구 profile, attempt limit 및 resource budget은 별도 문서에서 정의한다. 승인된 연구 profile에 속하지 않는 parameter에 보안 수준을 부여해서는 안 된다.

## 4. Uniform 1-Factor Sampler

factor label `j` 하나에 대해 uniform random permutation을 생성한다.

```math
\pi_j\leftarrow S_n.
```

이를 연속된 세 개씩 나누고 각 block을 unordered edge로 변환한다.

```math
F_j=
\left\{
\{\pi_j(3k),\pi_j(3k+1),\pi_j(3k+2)\}
:0\le k<\frac n3
\right\}.
```

`F_j`의 edge는 서로 겹치지 않으며 모든 정점을 정확히 한 번 덮으므로 항상 1-factor다.

각 1-factor를 생성하는 permutation 수는 정확히

```math
\left(\frac n3\right)!(3!)^{n/3}
```

으로 같다. 따라서 `\pi_j`를 균등하게 생성하면 1-factor도 균등하게 생성된다.

## 5. Planted Factorization 분포

순서가 있는 planted factor tuple을

```math
\mathbf F=(F_0,\ldots,F_{d-1})
```

로 정의한다.

하나의 candidate tuple은 다음과 같이 생성한다.

1. 모든 `j\in[d]`에 대해 uniform 1-factor sampler로 `F_j`를 독립적으로 생성한다.
2. 서로 다른 factor에 같은 edge가 하나라도 존재하면 tuple 전체를 폐기한다.
3. 충돌이 없으면 tuple을 채택한다.

즉 채택된 tuple은 `d`개의 독립적인 uniform 1-factor에 pairwise edge-disjoint 조건을 부여한 조건부 분포를 따른다.

```math
F_i\cap F_j=\varnothing
\quad\text{for all }i\ne j.
```

반드시 tuple 전체를 폐기하고 다시 생성해야 한다. 충돌한 factor만 교체하면 다른 분포가 된다.

## 6. 정점 재배열

`\mathbf F`를 채택한 뒤 독립적인 uniform permutation을 생성한다.

```math
\rho\leftarrow S_n.
```

모든 edge를 다음과 같이 변환한다.

```math
\rho(\{a,b,c\})=\{\rho(a),\rho(b),\rho(c)\}.
```

재배열한 factor tuple은

```math
\rho(\mathbf F)=(\rho(F_0),\ldots,\rho(F_{d-1}))
```

이다.

정점 번호 자체에는 의미가 없다. 이 단계는 KeyGen의 명시적인 과정으로 유지하지만 안전성을 이 과정에 의존해서는 안 된다. Factor sampler와 rejection 조건은 vertex permutation에 대해 불변이므로 마지막 재배열은 분포상 중복되는 단계일 수 있다.

## 7. 공개 인스턴스 분포

수학적인 공개 edge set은

```math
E=\bigcup_{j=0}^{d-1}\rho(F_j)
```

이다.

factor label을 제거하고 공개 인스턴스를

```math
H=(V,E)
```

로 정의한다.

생성된 hypergraph는 다음 조건을 만족한다.

- 3-uniform
- duplicate edge를 폐기했으므로 simple
- 모든 정점이 각 factor에 한 번씩 등장하므로 `d`-regular
- 적어도 하나의 1-factorization이 존재
- edge 수가 정확히 `dn/3`

공개 분포는 순서가 있는 planted factorization을 생성한 뒤 label을 제거하여 유도되는 분포다. 모든 simple하고 factorable한 `d`-regular 3-uniform hypergraph 위의 균등분포가 **아니다**.

특히 더 많은 ordered factorization을 가진 공개 hypergraph는 더 큰 확률 질량을 받을 수 있다. 이 sampling bias도 R3HFR 가정의 일부이며 cryptanalysis에서 반드시 측정해야 한다.

## 8. Canonical 공개 표현

`E`는 수학적으로 set이지만 구현에서는 다음 canonical indexed sequence로 표현한다.

```math
(e_0,\ldots,e_{m-1}).
```

Canonicalization은 다음 규칙을 적용한다.

1. 각 edge 내부의 세 정점을 오름차순으로 정렬한다.
2. 전체 edge sequence를 lexicographic order로 정렬한다.
3. duplicate edge를 조용히 제거하지 않고 거부한다.
4. 최종 vertex relabeling과 sorting이 끝난 뒤에만 edge index를 부여한다.

정확한 byte encoding은 encoding 명세에서 정의한다. 이 문서에서는 수학적인 ordering만 정의한다.

## 9. Planted Witness 생성

각 canonical edge index `i`에 대해 `c_i`를 `e_i`가 속한 planted factor의 label로 정의한다.

```math
c=(c_0,\ldots,c_{m-1})\in[d]^m.
```

공개 edge를 정렬할 때 planted label에도 같은 index permutation을 적용해야 한다. Canonicalization 이후 pre-sort 위치에 label을 연결해서는 안 된다.

vector `c`는 planted valid witness다. 유일한 valid witness일 필요가 없으며 공격자가 복구할 witness와 같을 필요도 없다.

이 문서는 수학적 witness만 정의한다. serialized secret-key 구조와 `Sign`에 제공할 정보는 별도 문서에서 정의한다.

## 10. Witness Relation

canonical public instance `H=(V,E)`와 candidate vector `c'`에 대해

```math
R_{\mathrm{R3HFR}}(H,c')=1
```

을 다음 조건을 모두 만족하는 경우로 정의한다.

### 10.1 구조적 유효성

- `H`는 `V` 위의 simple 3-uniform hypergraph다.
- 모든 정점의 degree가 정확히 `d`다.
- `|E|=dn/3`이다.
- `c'`의 entry 수가 정확히 `m`이다.

### 10.2 Label 범위

모든 edge index `i`에 대해

```math
c'_i\in[d]
```

이다.

### 10.3 Local Uniqueness

모든 정점 `v`와 서로 다른 incident edge index `i`, `k`에 대해

```math
v\in e_i\cap e_k
\quad\Longrightarrow\quad
c'_i\ne c'_k
```

를 만족해야 한다.

동일하게 모든 정점 `v`에 대해

```math
\{c'_i:v\in e_i\}=[d]
```

를 만족해야 한다.

## 11. Relation이 1-Factorization을 정의하는 이유

label `\ell\in[d]` 하나를 고정하고

```math
F'_{\ell}=\{e_i:c'_i=\ell\}
```

로 정의한다.

Local uniqueness에 따라 하나의 정점은 `F'_{\ell}`의 두 edge에 동시에 포함될 수 없다. 모든 정점의 degree가 `d`이고 `[d]`의 서로 다른 label `d`개를 가지므로 각 정점은 `F'_{\ell}`의 edge 하나에 정확히 한 번 포함된다.

따라서 모든 `F'_{\ell}`은 1-factor이며 각 label class가 전체 edge set을 분할한다.

```math
E=F'_0\dot\cup\cdots\dot\cup F'_{d-1}.
```

## 12. Labeled Witness 동치

factor label은 수학적 비밀이 아니라 이름이다. `\sigma\in S_d`를 label set의 permutation이라 하고 witness에 대한 작용을 다음과 같이 정의한다.

```math
(\sigma\cdot c)_i=\sigma(c_i).
```

두 valid labeled witness `c`, `c'`에 대해 어떤 `\sigma\in S_d`가 존재하여

```math
c'=\sigma\cdot c
```

를 만족할 때만

```math
c\sim c'
```

로 정의한다.

동치류

```math
[c]=\{\sigma\cdot c:\sigma\in S_d\}
```

는 하나의 unlabeled 1-factorization을 나타낸다.

모든 valid factorization은 label `d`개를 모두 사용하므로 group action은 free하다. 따라서 각 equivalence class에는 정확히

```math
d!
```

개의 labeled witness가 존재한다.

`N_{\mathrm{lab}}(H)`를 valid labeled witness 수, `N_{\mathrm{cls}}(H)`를 equivalence class 수라고 하면

```math
N_{\mathrm{lab}}(H)=d!\,N_{\mathrm{cls}}(H)
```

이다.

보안 분석에서는 필연적으로 존재하는 `d!` label symmetry를 서로 독립적인 추가 factorization으로 계산해서는 안 된다.

## 13. Canonical Equivalence-Class Representative

분석 도구는 각 equivalence class에 대해 하나의 고유한 representative를 사용한다.

canonical edge order에 따라 witness entry를 순회한다. 처음 등장하는 label을 `0`, 그다음 처음 등장하는 label을 `1`로 매핑하며 first-occurrence order에 따라 계속한다.

예시는 다음과 같다.

```text
(4,4,2,4,7,2,7,...) -> (0,0,1,0,2,1,2,...)
```

이 결정론적 정규화를 `CanonLabel(c)`로 정의한다. 그러면

```math
c\sim c'
\quad\Longleftrightarrow\quad
\operatorname{CanonLabel}(c)=\operatorname{CanonLabel}(c')
```

가 성립한다.

이 표현은 비교, 열거 및 결과 보고에만 사용한다. Verifier가 허용하는 relation을 변경하지 않는다.

## 14. Recovery Experiment

공격 algorithm `\mathcal A`에 대해 recovery experiment를 다음과 같이 정의한다.

```math
\begin{aligned}
&(H,c)\leftarrow\operatorname{SampleR3HFR}(n,d),\\
&c'\leftarrow\mathcal A(H),\\
&\text{return }R_{\mathrm{R3HFR}}(H,c').
\end{aligned}
```

공격 성공 확률은

```math
\operatorname{Succ}^{\mathrm{R3HFR}}_{\mathcal A}(n,d)
=
\Pr[R_{\mathrm{R3HFR}}(H,\mathcal A(H))=1]
```

이다.

공격자는 **어떤** factorization class에 속하든 **임의의** valid labeled representative를 출력하면 성공한다. Planted witness와

```math
[c']=[c]
```

인지 여부는 중요하지 않다.

Planted witness와의 비교값은 분석 목적으로 보고할 수 있지만 공격 성공 조건에는 포함하지 않는다.

## 15. Rejection 및 Sampling Failure

수학적 분포는 factor가 pairwise edge-disjoint라는 조건부 분포로 정의한다. 실제 구현에서는 bounded rejection sampling을 사용한다.

다음 경우 candidate를 거부한다.

- 유효하지 않은 parameter pair가 입력된 경우
- 서로 다른 factor에 같은 unordered triple이 등장한 경우

`\rho`는 bijection이므로 vertex relabeling은 duplicate를 만들거나 제거할 수 없다.

설정된 attempt limit을 모두 소진하면 KeyGen은 명시적인 sampling failure를 반환한다. 다음 동작은 허용하지 않는다.

- `d`를 조용히 감소시키기
- 일부 factor만 유지하기
- 충돌한 edge만 교체하기
- factor sampler를 변경하기
- 명세를 만족하지 않는 인스턴스 반환하기

attempt limit과 허용할 rejection rate는 연구 parameter 명세에서 고정한다.

## 16. 필수 분포 검사

구현과 분석 도구는 최소한 다음 invariant를 검사해야 한다.

- 모든 edge가 서로 다른 정점 세 개를 포함한다.
- 모든 edge가 canonical하며 중복되지 않는다.
- 모든 정점의 degree가 `d`다.
- edge 수가 `dn/3`이다.
- planted witness가 relation을 만족한다.
- 모든 global label permutation이 witness 유효성을 보존한다.
- canonical class normalization이 global label permutation에 대해 불변이다.
- 고정된 seed가 같은 연구 인스턴스를 재현한다.
- whole-tuple rejection과 다른 resampling rule을 테스트에서 구분할 수 있다.
- 공개 분포의 bias를 연구할 수 있도록 sampler 통계를 기록한다.

## 17. 명시적으로 주장하지 않는 사항

이 명세는 다음을 주장하지 않는다.

- R3HFR가 worst case 또는 average case에서 어렵다.
- 유도된 공개 분포가 uniform random regular hypergraph에 가깝다.
- vertex relabeling이 planted structure를 숨긴다.
- raw witness 수가 많으면 안전하다.
- solver timeout이 hardness를 의미한다.
- 문법적인 parameter 범위를 만족하면 어떤 암호학적 보안 수준을 제공한다.

이 문제는 이후 cryptanalysis 및 parameter 연구를 통해 가능한 범위에서 판단한다.

## 18. 규범적 요약

R3HFR 인스턴스 분포는 다음과 같다.

```math
\boxed{
\begin{array}{c}
d\text{개의 독립적인 uniform 1-factor}\\
\text{pairwise edge-disjoint 조건을 부여한 조건부 분포}\\
\xrightarrow{\text{factor label 제거}}
\text{public simple }d\text{-regular 3-uniform hypergraph}
\end{array}}
```

witness relation은 모든 정점에서 label이 local uniqueness를 만족하는 edge labeling을 허용한다.

Recovery target은 임의의 valid witness다. 전체 factor label permutation 하나만 다른 witness는 같은 equivalence class에 속하며, 보안 분석에서는 이 class 수와 raw labeled witness 수를 구분한다.
