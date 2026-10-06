#include "trifact/trifact.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    trifact_edge_t edge;
    trifact_factor_label_t label;
} edge_label_t;

static const trifact_edge_t degree_two[] = {
    {{0U, 1U, 2U}}, {{0U, 1U, 3U}}, {{2U, 4U, 5U}}, {{3U, 4U, 5U}}};
static const trifact_factor_label_t two_labels[] = {0U, 1U, 1U, 0U};
static const trifact_edge_t degree_three[] = {{{0U, 1U, 2U}}, {{0U, 1U, 3U}}, {{0U, 1U, 4U}},
                                              {{2U, 3U, 5U}}, {{2U, 4U, 5U}}, {{3U, 4U, 5U}}};
static const trifact_factor_label_t three_labels[] = {0U, 1U, 2U, 2U, 1U, 0U};

static int compare_records(const void *left, const void *right) {
    const edge_label_t *first = left;
    const edge_label_t *second = right;
    size_t slot = 0U;

    for (slot = 0U; slot < 3U; ++slot) {
        if (first->edge.vertices[slot] < second->edge.vertices[slot]) {
            return -1;
        }
        if (first->edge.vertices[slot] > second->edge.vertices[slot]) {
            return 1;
        }
    }
    return 0;
}

static void relabel(const trifact_edge_t *edges, const trifact_factor_label_t *labels, size_t count,
                    const trifact_vertex_t *permutation, trifact_edge_t *out_edges,
                    trifact_factor_label_t *out_labels) {
    edge_label_t records[6] = {{{{0U}}, 0U}};
    size_t edge = 0U;

    for (edge = 0U; edge < count; ++edge) {
        size_t slot = 0U;

        records[edge].label = labels[edge];
        for (slot = 0U; slot < 3U; ++slot) {
            size_t previous = slot;
            const trifact_vertex_t value = permutation[edges[edge].vertices[slot]];

            while (previous > 0U && records[edge].edge.vertices[previous - 1U] > value) {
                records[edge].edge.vertices[previous] = records[edge].edge.vertices[previous - 1U];
                --previous;
            }
            records[edge].edge.vertices[previous] = value;
        }
    }
    qsort(records, count, sizeof(*records), compare_records);
    for (edge = 0U; edge < count; ++edge) {
        out_edges[edge] = records[edge].edge;
        out_labels[edge] = records[edge].label;
    }
}

static int audit_rows(trifact_vertex_t vertices, const trifact_edge_t *edges, size_t edge_count,
                      uint32_t factors, const size_t *rows, size_t row_count) {
    size_t edge_hits[6] = {0U};
    trifact_vertex_t vertex = 0U;
    size_t edge = 0U;

    if (edge_count > 6U || row_count != (size_t)vertices * (size_t)factors) {
        return 0;
    }
    for (vertex = 0U; vertex < vertices; ++vertex) {
        size_t slot = 0U;

        for (slot = 0U; slot < (size_t)factors; ++slot) {
            const size_t offset = (size_t)vertex * (size_t)factors + slot;
            const size_t entry = rows[offset];

            if (entry >= edge_count || (slot > 0U && entry <= rows[offset - 1U]) ||
                (edges[entry].vertices[0] != vertex && edges[entry].vertices[1] != vertex &&
                 edges[entry].vertices[2] != vertex)) {
                return 0;
            }
            ++edge_hits[entry];
        }
    }
    for (edge = 0U; edge < edge_count; ++edge) {
        if (edge_hits[edge] != 3U) {
            return 0;
        }
    }
    return 1;
}

static int check_incidence(const trifact_hypergraph_t *hypergraph, uint32_t factors) {
    trifact_incidence_index_t *index = NULL;
    const trifact_vertex_t vertices = trifact_hypergraph_vertex_count(hypergraph);
    const trifact_edge_t *edges = trifact_hypergraph_edges(hypergraph);
    const size_t edge_count = trifact_hypergraph_edge_count(hypergraph);
    size_t rows[27] = {0U};
    trifact_vertex_t vertex = 0U;
    int failed = 0;

    if ((size_t)vertices * (size_t)factors > 27U ||
        trifact_incidence_index_create(&index, hypergraph, factors) != TRIFACT_STATUS_OK) {
        return 1;
    }
    for (vertex = 0U; vertex < vertices; ++vertex) {
        const size_t *entries = NULL;
        size_t count = 0U;

        if (trifact_incidence_index_vertex_edges(index, vertex, &entries, &count) !=
                TRIFACT_STATUS_OK ||
            count != (size_t)factors || entries == NULL) {
            failed = 1;
            break;
        }
        (void)memcpy(&rows[(size_t)vertex * (size_t)factors], entries, count * sizeof(*entries));
    }
    if (failed == 0 && audit_rows(vertices, edges, edge_count, factors, rows,
                                  (size_t)vertices * (size_t)factors) == 0) {
        failed = 1;
    }
    trifact_incidence_index_destroy(index);
    return failed;
}

static int check_transform(trifact_vertex_t vertices, const trifact_edge_t *edges,
                           const trifact_factor_label_t *labels, size_t edge_count,
                           uint32_t factors, const trifact_vertex_t *permutation,
                           trifact_status_t expected, size_t *unpaired_failures,
                           size_t *different_classes) {
    trifact_edge_t transformed_edges[6] = {{{0U}}};
    trifact_factor_label_t transformed_labels[6] = {0U};
    trifact_hypergraph_t *hypergraph = NULL;
    trifact_incidence_index_t *index = NULL;
    trifact_label_vector_t *vector = NULL;
    trifact_label_vector_t *original = NULL;
    trifact_label_vector_t *normalized = NULL;
    int equivalent = 0;
    int failed = 0;

    relabel(edges, labels, edge_count, permutation, transformed_edges, transformed_labels);
    if (trifact_hypergraph_create(&hypergraph, vertices, transformed_edges, edge_count) !=
            TRIFACT_STATUS_OK ||
        trifact_label_vector_create(&vector, transformed_labels, edge_count) != TRIFACT_STATUS_OK ||
        trifact_relation_validate(hypergraph, factors, vector, NULL) != expected) {
        failed = 1;
    } else if (expected == TRIFACT_STATUS_VERTEX_DEGREE_MISMATCH) {
        if (trifact_incidence_index_create(&index, hypergraph, factors) != expected ||
            index != NULL) {
            failed = 1;
        }
    } else {
        failed = check_incidence(hypergraph, factors);
        if (trifact_label_vector_normalize(&normalized, vector) != TRIFACT_STATUS_OK ||
            trifact_relation_validate(hypergraph, factors, normalized, NULL) != expected) {
            failed = 1;
        }
        if (expected == TRIFACT_STATUS_OK) {
            if (trifact_label_vector_create(&original, labels, edge_count) != TRIFACT_STATUS_OK ||
                trifact_label_vectors_equivalent(original, vector, &equivalent) !=
                    TRIFACT_STATUS_OK) {
                failed = 1;
            } else {
                if (equivalent == 0) {
                    ++*different_classes;
                }
                if (trifact_relation_validate(hypergraph, factors, original, NULL) ==
                    TRIFACT_STATUS_INCIDENT_LABEL_COLLISION) {
                    ++*unpaired_failures;
                }
            }
        }
    }
    trifact_incidence_index_destroy(index);
    trifact_label_vector_destroy(normalized);
    trifact_label_vector_destroy(original);
    trifact_label_vector_destroy(vector);
    trifact_hypergraph_destroy(hypergraph);
    return failed;
}

static int enumerate_vertex_permutations(trifact_vertex_t *permutation, size_t position,
                                         size_t *cases, size_t *unpaired_failures,
                                         size_t *different_classes) {
    const trifact_factor_label_t collision[] = {0U, 1U, 0U, 0U};
    const trifact_edge_t irregular[] = {
        {{0U, 1U, 2U}}, {{0U, 1U, 3U}}, {{0U, 1U, 4U}}, {{3U, 4U, 5U}}};
    trifact_vertex_t candidate = 0U;

    if (position == 6U) {
        ++*cases;
        return check_transform(6U, degree_two, two_labels, 4U, 2U, permutation, TRIFACT_STATUS_OK,
                               unpaired_failures, different_classes) +
               check_transform(6U, degree_three, three_labels, 6U, 3U, permutation,
                               TRIFACT_STATUS_OK, unpaired_failures, different_classes) +
               check_transform(6U, degree_two, collision, 4U, 2U, permutation,
                               TRIFACT_STATUS_INCIDENT_LABEL_COLLISION, unpaired_failures,
                               different_classes) +
               check_transform(6U, irregular, two_labels, 4U, 2U, permutation,
                               TRIFACT_STATUS_VERTEX_DEGREE_MISMATCH, unpaired_failures,
                               different_classes);
    }
    for (candidate = 0U; candidate < 6U; ++candidate) {
        size_t previous = 0U;

        for (previous = 0U; previous < position; ++previous) {
            if (permutation[previous] == candidate) {
                break;
            }
        }
        if (previous == position) {
            permutation[position] = candidate;
            if (enumerate_vertex_permutations(permutation, position + 1U, cases, unpaired_failures,
                                              different_classes) != 0) {
                return 1;
            }
        }
    }
    return 0;
}

static int test_label_permutations(void) {
    trifact_hypergraph_t *hypergraph = NULL;
    trifact_label_vector_t *reference = NULL;
    trifact_label_vector_t *original = NULL;
    trifact_factor_label_t first = 0U;
    trifact_factor_label_t second = 0U;
    size_t cases = 0U;
    int failed = 0;

    if (trifact_hypergraph_create(&hypergraph, 6U, degree_three, 6U) != TRIFACT_STATUS_OK ||
        trifact_label_vector_create(&original, three_labels, 6U) != TRIFACT_STATUS_OK ||
        trifact_label_vector_normalize(&reference, original) != TRIFACT_STATUS_OK) {
        failed = 1;
    } else {
        for (first = 0U; first < 3U; ++first) {
            for (second = 0U; second < 3U; ++second) {
                if (first != second) {
                    const trifact_factor_label_t mapping[] = {first, second, 3U - first - second};
                    trifact_factor_label_t renamed[6] = {0U};
                    trifact_label_vector_t *vector = NULL;
                    trifact_label_vector_t *normalized = NULL;
                    size_t edge = 0U;
                    int equivalent = 0;

                    for (edge = 0U; edge < 6U; ++edge) {
                        renamed[edge] = mapping[three_labels[edge]];
                    }
                    if (trifact_label_vector_create(&vector, renamed, 6U) != TRIFACT_STATUS_OK ||
                        trifact_relation_validate(hypergraph, 3U, vector, NULL) !=
                            TRIFACT_STATUS_OK ||
                        trifact_label_vector_normalize(&normalized, vector) != TRIFACT_STATUS_OK ||
                        memcmp(trifact_label_vector_data(normalized),
                               trifact_label_vector_data(reference), sizeof(renamed)) != 0 ||
                        trifact_label_vectors_equivalent(original, vector, &equivalent) !=
                            TRIFACT_STATUS_OK ||
                        equivalent != 1) {
                        failed = 1;
                    }
                    ++cases;
                    trifact_label_vector_destroy(normalized);
                    trifact_label_vector_destroy(vector);
                }
            }
        }
    }
    trifact_label_vector_destroy(reference);
    trifact_label_vector_destroy(original);
    trifact_hypergraph_destroy(hypergraph);
    return failed != 0 || cases != 6U;
}

static int test_corrupted_rows(void) {
    const size_t original[] = {0U, 1U, 0U, 1U, 0U, 2U, 1U, 3U, 2U, 3U, 2U, 3U};
    size_t rows[12] = {0U};
    int failed = 0;

    if (audit_rows(6U, degree_two, 4U, 2U, original, 12U) == 0 ||
        audit_rows(6U, degree_two, 4U, 2U, original, 11U) != 0) {
        failed = 1;
    }
    (void)memcpy(rows, original, sizeof(rows));
    rows[1] = rows[0];
    failed |= audit_rows(6U, degree_two, 4U, 2U, rows, 12U);
    (void)memcpy(rows, original, sizeof(rows));
    rows[11] = 4U;
    failed |= audit_rows(6U, degree_two, 4U, 2U, rows, 12U);
    (void)memcpy(rows, original, sizeof(rows));
    rows[0] = 2U;
    rows[1] = 3U;
    failed |= audit_rows(6U, degree_two, 4U, 2U, rows, 12U);
    (void)memcpy(rows, original, sizeof(rows));
    rows[0] = 1U;
    rows[1] = 0U;
    failed |= audit_rows(6U, degree_two, 4U, 2U, rows, 12U);
    return failed;
}

int main(void) {
    const trifact_edge_t nine_edges[] = {{{0U, 1U, 2U}}, {{0U, 3U, 6U}}, {{1U, 4U, 7U}},
                                         {{2U, 5U, 8U}}, {{3U, 4U, 5U}}, {{6U, 7U, 8U}}};
    const trifact_factor_label_t nine_labels[] = {0U, 1U, 1U, 1U, 0U, 0U};
    const trifact_vertex_t exchange[] = {3U, 1U, 2U, 0U, 4U, 5U, 6U, 7U, 8U};
    trifact_vertex_t permutation[6] = {0U};
    size_t cases = 0U;
    size_t unpaired_failures = 0U;
    size_t different_classes = 0U;
    int failures = enumerate_vertex_permutations(permutation, 0U, &cases, &unpaired_failures,
                                                 &different_classes);

    if (cases != 720U) {
        ++failures;
    }
    failures += check_transform(9U, nine_edges, nine_labels, 6U, 2U, exchange, TRIFACT_STATUS_OK,
                                &unpaired_failures, &different_classes);
    if (unpaired_failures == 0U || different_classes == 0U) {
        ++failures;
    }
    failures += test_label_permutations();
    failures += test_corrupted_rows();
    if (failures != 0) {
        (void)fprintf(stderr, "invariant checks failed: %d\n", failures);
    }
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
