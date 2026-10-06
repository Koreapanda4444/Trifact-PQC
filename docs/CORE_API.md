# TRIFACT Core API

[English](CORE_API.md) | [한국어](CORE_API_KR.md)

## Ownership and construction

`trifact_edge_init` accepts only `first < second < third < vertex_count`; it does not sort input and leaves the destination unchanged on failure.

`trifact_hypergraph_create` copies an already canonical, unique, lexicographically sorted edge list. The generic container accepts vertex counts of at least three and may hold an empty graph. Construction alone does not establish the R3HFR relation.

`trifact_label_vector_create` copies labels without assigning them a factor count or validating a relation. A zero-length vector is permitted. The returned graph and vector own their copied arrays; accessors return borrowed read-only data valid until destruction. Do not modify borrowed storage.

Object constructors clear their output slot to `NULL` on failure. Use an empty output slot and destroy an existing object before replacing it. Destructors accept `NULL`; set an owned pointer to `NULL` after destruction before calling the destructor again.

## Factorization relation

`trifact_relation_validate(hypergraph, factor_count, labels, error)` returns `TRIFACT_STATUS_OK` precisely when the constructed public graph and witness satisfy the R3HFR specification. It changes neither object.

The relation enforces `n >= 6`, `n % 3 == 0`, `2 <= d <= (n-1)(n-2)/2`, `m = dn/3`, witness length `m`, labels in `[0,d)`, vertex degree `d`, and local label uniqueness. Canonical edges and simplicity are guaranteed by the graph constructor.

`error` is optional. When supplied, its status equals the returned status. Unavailable vertex and edge indexes are `SIZE_MAX`, including on success. Labels outside the range report their first edge index. Degree failures report the first vertex index. A local collision reports the vertex and later incident edge that repeat a label.

| Status | Meaning |
|---|---|
| `NULL_ARGUMENT` | Required graph or label object is absent. |
| `INVALID_VERTEX_COUNT` | The relation's vertex-count domain is violated. |
| `INVALID_FACTOR_COUNT` | The relation's degree domain is violated. |
| `SIZE_OVERFLOW` | A count or allocation product cannot fit `size_t`. |
| `EDGE_COUNT_MISMATCH` | Public edge count differs from `dn/3`. |
| `WITNESS_LENGTH_MISMATCH` | Witness length differs from the edge count. |
| `LABEL_OUT_OF_RANGE` | A candidate label is outside `[0,d)`. |
| `VERTEX_DEGREE_MISMATCH` | A public vertex does not have degree `d`. |
| `INCIDENT_LABEL_COLLISION` | A vertex occurs twice under the same label. |
| `ALLOCATION_FAILURE` | Temporary validation storage could not be obtained. |

The validator first checks arguments, parameter domains, expected edge count, witness length, and label range. It then checks degree counts before local uniqueness. Workspace uses `O(n + nd)` memory and execution uses `O(n + nd + m)` work. Allocation and arithmetic failures are errors, not a claim that an instance has no factorization.

## Validated incidence indexes

`trifact_incidence_index_create(out_index, hypergraph, factor_count)` checks the relation's parameter domain, edge count, and degree of every vertex before returning an owning index. It checks all storage products and rejects irregular graphs. It does not accept or validate a witness. Each vertex has exactly `d` entries; the total is `nd = 3m`.

`trifact_incidence_index_vertex_edges(index, vertex, out_edges, out_count)` returns a borrowed, read-only row of canonical edge indexes in increasing order. Both output arguments are required. On failure it clears every supplied edge-pointer slot to `NULL` and count slot to zero; out-of-range vertices return `VERTEX_OUT_OF_RANGE`. The vertex, edge, and factor-count accessors return zero for a `NULL` index.

The index owns its rows independently of the source graph and survives that graph's destruction. Its edge numbers refer to the original canonical edge order. Construction uses `O(n + m)` work and `O(n + nd)` temporary and retained storage; querying a vertex uses constant work. Constructor and destructor ownership rules match the core containers. `trifact.incidence` checks exact rows, totals, ordering, source lifetime independence, invalid graphs, and accessor failures.

## Witness label symmetry

`trifact_label_vector_normalize(out_labels, labels)` returns a new owning vector whose labels are numbered `0,1,...` in first-occurrence order, without reordering edges. For example, `(4,4,2,4,7,2,7)` becomes `(0,0,1,0,2,1,2)`. Empty vectors and arbitrary `uint32_t` label names, including `UINT32_MAX`, are accepted. The source is unchanged, output failures clear the slot, and normalization is idempotent. It uses `O(m^2)` comparisons and `O(m)` workspace, without allocating according to the largest label name.

`trifact_label_vectors_equivalent(left, right, out_equivalent)` returns whether two equal-length vectors have the same equality pattern at the same edge positions. The result is 0 or 1; differing lengths return 0. Missing arguments return `NULL_ARGUMENT` and leave any supplied result unchanged. This query performs `O(m^2)` work without allocating.

These helpers do not validate factorization. For witnesses on the same fixed canonical graph, equivalence expresses a single global label permutation. Vertex permutations, edge reordering, and graph isomorphisms are separate operations and are not collapsed. `trifact.witness` checks examples, arbitrary label names, idempotence, source preservation, equivalence in both directions, split and merged classes, differing positions and lengths, and empty inputs.

## Tests

The `trifact.relation` CTest target checks valid factorization and each failure category, including diagnostic indexes and the optional-diagnostic path. All core source is C17 and contains no code comments. Key generation and proof implementation remain later work.

`trifact.relation-oracle` independently checks whether each factor is a disjoint edge cover of every vertex. It does not use the validator's degree counters or incident-label table. It exhausts all simple six-vertex graphs with zero through four edges and all labels in `{0,1,2}` for `d = 2`: 424,996 candidates, including wrong-size and irregular public graphs and out-of-range labels. Exactly 90 candidates satisfy the relation. Additional exhaustive labelings cover a six-vertex degree-three graph, a nine-vertex graph, and invalid vertex-count domains. These bounded tests establish implementation agreement, not cryptographic hardness.

`trifact.invariants` applies all 720 six-vertex permutations to degree-two and degree-three factorizations, an invalid witness, and an irregular graph. It canonicalizes each transformed triple and sorts edge-label pairs together, checks relation verdicts, and independently audits index memberships, ordering, and three incidences per edge. It also covers a nine-vertex permutation and all six degree-three label permutations. Cases where leaving labels behind breaks the relation and where vertex permutation changes the witness equality pattern ensure that these operations remain distinct. Deliberately duplicated, missing, out-of-range, misordered, and nonincident entries are checked in copies of index rows; borrowed library storage is never modified.

`trifact.allocation` links the same core sources against a private test allocator, in place of the production heap wrapper. No allocator control or mutable test state enters the public API or production library. Each owning constructor, normalization, and relation validation is first measured on success; every allocation is then failed individually, followed by an uninjected recovery call. The test tracks all live pointers, checks cleared outputs, unchanged inputs, diagnostic errors, partial-construction cleanup, early and late rejection cleanup, and zero live allocations at exit. Destruction is repeated only after setting the owned slot to `NULL`. It also checks that equivalence and argument rejection allocate nothing.
