# R3HFR Distribution and Witness Specification

[English](R3HFR_SPECIFICATION.md) | [한국어](R3HFR_SPECIFICATION_KR.md)

> **Status: Research specification**
>
> This document defines the instance distribution, witness relation, and witness equivalence used by TRIFACT. It does not assert that R3HFR is hard.

## 1. Purpose

R3HFR stands for **Random 3-Uniform Hypergraph Factorization Recovery**.

The problem asks an attacker to recover any valid 1-factorization of a public hypergraph sampled by TRIFACT KeyGen. The recovered factorization may differ from the planted factorization used during sampling.

This document fixes the mathematical distribution and success condition shared by KeyGen, relation validators, recovery solvers, and benchmarks.

## 2. Notation

Let

```math
V=\{0,\ldots,n-1\}
```

be the vertex set, and let

```math
\binom{V}{3}=\{e\subseteq V:|e|=3\}
```

be the set of all unordered triples of distinct vertices.

The following notation is used:

- `n`: number of vertices;
- `d`: number of planted factors and degree of every vertex;
- `m`: number of public edges;
- `S_n`: permutation group on `V`;
- `[d]`: label set `\{0,\ldots,d-1\}`;
- `H=(V,E)`: public 3-uniform hypergraph;
- `F_j`: the planted factor with label `j`;
- `c`: an edge-label vector representing a factorization.

For a valid instance,

```math
m=|E|=\frac{dn}{3}.
```

## 3. Parameter Domain

A syntactically valid parameter pair `(n,d)` satisfies:

```math
n\ge6,
```

```math
n\equiv0\pmod3,
```

and

```math
2\le d\le\binom{n-1}{2}.
```

The upper bound is necessary because a vertex can occur in at most `\binom{n-1}{2}` distinct 3-edges in a simple hypergraph. It does not guarantee that KeyGen has an acceptable rejection probability.

Concrete research profiles, attempt limits, and resource budgets are defined separately. Parameters outside an approved research profile must not be assigned a security level.

## 4. Uniform 1-Factor Sampler

For one factor label `j`, sample a uniformly random permutation

```math
\pi_j\leftarrow S_n.
```

Partition it into consecutive blocks of three and convert every block into an unordered edge:

```math
F_j=
\left\{
\{\pi_j(3k),\pi_j(3k+1),\pi_j(3k+2)\}
:0\le k<\frac n3
\right\}.
```

Every `F_j` is a 1-factor because its edges are pairwise disjoint and cover every vertex exactly once.

Every 1-factor is represented by exactly

```math
\left(\frac n3\right)!(3!)^{n/3}
```

permutations. Sampling `\pi_j` uniformly therefore samples 1-factors uniformly.

## 5. Planted Factorization Distribution

Define the ordered planted-factor tuple

```math
\mathbf F=(F_0,\ldots,F_{d-1}).
```

One candidate tuple is sampled as follows:

1. independently sample `F_j` with the uniform 1-factor sampler for every `j\in[d]`;
2. reject the complete tuple if any edge appears in two different factors;
3. otherwise accept the tuple.

Equivalently, the accepted tuple is distributed as `d` independent uniform 1-factors conditioned on pairwise edge-disjointness:

```math
F_i\cap F_j=\varnothing
\quad\text{for all }i\ne j.
```

Whole-tuple rejection is required. Replacing only the colliding factor would define a different distribution.

## 6. Vertex Relabeling

After accepting `\mathbf F`, sample an independent uniform permutation

```math
\rho\leftarrow S_n
```

and map every edge by

```math
\rho(\{a,b,c\})=\{\rho(a),\rho(b),\rho(c)\}.
```

The relabeled factor tuple is

```math
\rho(\mathbf F)=(\rho(F_0),\ldots,\rho(F_{d-1})).
```

Vertex labels have no semantic meaning. The relabeling step is retained as an explicit part of KeyGen, but security must not rely on it. Because the factor sampler and rejection condition are invariant under vertex permutations, this final step may be distributionally redundant.

## 7. Public Instance Distribution

The mathematical public edge set is

```math
E=\bigcup_{j=0}^{d-1}\rho(F_j).
```

The factor labels are erased, and the public instance is

```math
H=(V,E).
```

The resulting hypergraph is:

- 3-uniform;
- simple, because duplicate edges were rejected;
- `d`-regular, because every vertex occurs once in every factor;
- guaranteed to have at least one 1-factorization;
- composed of exactly `dn/3` edges.

The public distribution is the distribution induced by sampling an ordered planted factorization and then forgetting its labels. It is **not** the uniform distribution over all simple, factorable, `d`-regular 3-uniform hypergraphs.

In particular, a public hypergraph that admits more ordered factorizations can receive more probability mass. This sampling bias is part of the R3HFR assumption and must be measured during cryptanalysis.

## 8. Canonical Public Representation

Although `E` is mathematically a set, implementations represent it as a canonical indexed sequence

```math
(e_0,\ldots,e_{m-1}).
```

Canonicalization applies the following rules:

1. sort the three vertices inside every edge in increasing order;
2. sort the complete edge sequence lexicographically;
3. reject duplicate edges rather than silently removing them;
4. assign edge indices only after the final vertex relabeling and sorting.

The exact byte encoding is defined in the encoding specification. This document defines only the mathematical ordering.

## 9. Planted Witness Construction

For every canonical edge index `i`, let `c_i` be the label of the planted factor containing `e_i`:

```math
c=(c_0,\ldots,c_{m-1})\in[d]^m.
```

Sorting the public edges requires applying the same index permutation to the planted labels. Labels must never be attached to pre-sort positions after canonicalization.

The vector `c` is a planted valid witness. It is not necessarily the only valid witness and is not necessarily the witness an attacker will recover.

This document defines the mathematical witness. The serialized secret-key structure and the information supplied to `Sign` are defined separately.

## 10. Witness Relation

For a canonical public instance `H=(V,E)` and a candidate vector `c'`, define

```math
R_{\mathrm{R3HFR}}(H,c')=1
```

if and only if all of the following conditions hold.

### 10.1 Structural Validity

- `H` is a simple 3-uniform hypergraph on `V`;
- every vertex has degree exactly `d`;
- `|E|=dn/3`;
- `c'` has exactly `m` entries.

### 10.2 Label Range

For every edge index `i`,

```math
c'_i\in[d].
```

### 10.3 Local Uniqueness

For every vertex `v` and all distinct incident edge indices `i` and `k`,

```math
v\in e_i\cap e_k
\quad\Longrightarrow\quad
c'_i\ne c'_k.
```

Equivalently, for every vertex `v`,

```math
\{c'_i:v\in e_i\}=[d].
```

## 11. Why the Relation Defines a 1-Factorization

Fix a label `\ell\in[d]` and define

```math
F'_{\ell}=\{e_i:c'_i=\ell\}.
```

Local uniqueness ensures that no vertex occurs in two edges of `F'_{\ell}`. Because every vertex has degree `d` and sees `d` distinct labels drawn from `[d]`, every vertex occurs in exactly one edge of `F'_{\ell}`.

Thus every `F'_{\ell}` is a 1-factor, and the label classes partition the complete edge set:

```math
E=F'_0\dot\cup\cdots\dot\cup F'_{d-1}.
```

## 12. Labeled Witness Equivalence

Factor labels are names, not mathematical secrets. Let `\sigma\in S_d` be a permutation of the label set. Define its action on a witness by

```math
(\sigma\cdot c)_i=\sigma(c_i).
```

Two valid labeled witnesses are equivalent when

```math
c\sim c'
```

if and only if there exists `\sigma\in S_d` such that

```math
c'=\sigma\cdot c.
```

An equivalence class

```math
[c]=\{\sigma\cdot c:\sigma\in S_d\}
```

represents one unlabeled 1-factorization.

Every valid factorization uses all `d` labels, so the group action is free. Each equivalence class therefore contains exactly

```math
d!
```

labeled witnesses.

If `N_{\mathrm{lab}}(H)` is the number of valid labeled witnesses and `N_{\mathrm{cls}}(H)` is the number of equivalence classes, then

```math
N_{\mathrm{lab}}(H)=d!\,N_{\mathrm{cls}}(H).
```

Security analysis must not count the unavoidable `d!` label symmetry as additional independent factorizations.

## 13. Canonical Equivalence-Class Representative

Analysis tools use a unique representative for each equivalence class.

Traverse the witness entries in canonical edge order. Map the first previously unseen label to `0`, the next previously unseen label to `1`, and continue in first-occurrence order.

For example,

```text
(4,4,2,4,7,2,7,...) -> (0,0,1,0,2,1,2,...)
```

Let this deterministic normalization be `CanonLabel(c)`. Then

```math
c\sim c'
\quad\Longleftrightarrow\quad
\operatorname{CanonLabel}(c)=\operatorname{CanonLabel}(c').
```

This representation is for comparison, enumeration, and reporting. It does not change the relation accepted by the verifier.

## 14. Recovery Experiment

For an attack algorithm `\mathcal A`, define the recovery experiment:

```math
\begin{aligned}
&(H,c)\leftarrow\operatorname{SampleR3HFR}(n,d),\\
&c'\leftarrow\mathcal A(H),\\
&\text{return }R_{\mathrm{R3HFR}}(H,c').
\end{aligned}
```

The attack success probability is

```math
\operatorname{Succ}^{\mathrm{R3HFR}}_{\mathcal A}(n,d)
=
\Pr[R_{\mathrm{R3HFR}}(H,\mathcal A(H))=1].
```

The attack succeeds when it outputs **any** valid labeled representative of **any** factorization class. It is irrelevant whether

```math
[c']=[c]
```

for the planted witness.

Comparisons with the planted witness may be reported for analysis, but they are not part of the attack success condition.

## 15. Rejection and Sampling Failure

The mathematical distribution is defined by conditioning on pairwise edge-disjoint factors. A practical implementation uses bounded rejection sampling.

A candidate is rejected when:

- an invalid parameter pair is supplied; or
- the same unordered triple appears in more than one factor.

Vertex relabeling cannot introduce or remove a duplicate because `\rho` is a bijection.

If the configured attempt limit is exhausted, KeyGen returns an explicit sampling failure. It must not:

- silently lower `d`;
- keep only some factors;
- replace only the colliding edge;
- alter the factor sampler;
- return a nonconforming instance.

Attempt limits and acceptable rejection rates are fixed by the research parameter specification.

## 16. Required Distribution Checks

Implementations and analysis tools must verify at least the following invariants:

- every edge contains three distinct vertices;
- every edge is canonical and unique;
- every vertex has degree `d`;
- the edge count is `dn/3`;
- the planted witness satisfies the relation;
- every global label permutation preserves validity;
- canonical class normalization is invariant under global label permutation;
- fixed seeds reproduce the same research instance;
- whole-tuple rejection is distinguishable from other resampling rules in tests;
- sampler statistics are recorded so that public-distribution bias can be studied.

## 17. Explicit Non-Claims

This specification does not claim that:

- R3HFR is hard in the worst case or on average;
- the induced public distribution is close to a uniform random regular hypergraph;
- vertex relabeling hides the planted structure;
- a large raw witness count implies security;
- a solver timeout implies hardness;
- parameters satisfying the syntactic domain provide any cryptographic security level.

These questions are resolved, if possible, through later cryptanalysis and parameter studies.

## 18. Normative Summary

The R3HFR instance distribution is:

```math
\boxed{
\begin{array}{c}
d\text{ independent uniform 1-factors}\\
\text{conditioned on pairwise edge-disjointness}\\
\xrightarrow{\text{erase factor labels}}
\text{public simple }d\text{-regular 3-uniform hypergraph}
\end{array}}
```

The witness relation accepts an edge labeling whose labels are locally unique at every vertex.

The recovery target is any valid witness. Witnesses that differ only by one global permutation of factor labels belong to the same equivalence class, and security analysis counts these classes separately from raw labeled witnesses.
