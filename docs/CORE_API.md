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

## Tests

The `trifact.relation` CTest target checks valid factorization and each failure category, including diagnostic indexes and the optional-diagnostic path. All core source is C17 and contains no code comments. Key generation and proof implementation remain later work.

`trifact.relation-oracle` independently checks whether each factor is a disjoint edge cover of every vertex. It does not use the validator's degree counters or incident-label table. It exhausts all simple six-vertex graphs with zero through four edges and all labels in `{0,1,2}` for `d = 2`: 424,996 candidates, including wrong-size and irregular public graphs and out-of-range labels. Exactly 90 candidates satisfy the relation. Additional exhaustive labelings cover a six-vertex degree-three graph, a nine-vertex graph, and invalid vertex-count domains. These bounded tests establish implementation agreement, not cryptographic hardness.
