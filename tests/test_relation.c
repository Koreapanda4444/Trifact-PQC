#include "trifact/relation.h"

#include <stdio.h>
#include <stdlib.h>

static const trifact_edge_t valid_edges[] = {
    {{0U, 1U, 2U}}, {{0U, 1U, 3U}}, {{2U, 4U, 5U}}, {{3U, 4U, 5U}}};
static const trifact_factor_label_t valid_labels[] = {0U, 1U, 1U, 0U};

static int check_relation(const char *name, trifact_vertex_t vertex_count,
                          const trifact_edge_t *edges, size_t edge_count, uint32_t factor_count,
                          const trifact_factor_label_t *data, size_t label_count,
                          trifact_status_t expected, size_t vertex_index, size_t edge_index) {
    trifact_hypergraph_t *hypergraph = NULL;
    trifact_label_vector_t *labels = NULL;
    trifact_relation_error_t error = {TRIFACT_STATUS_OK, 0U, 0U};
    trifact_status_t status =
        trifact_hypergraph_create(&hypergraph, vertex_count, edges, edge_count);
    int failed = 0;

    if (status != TRIFACT_STATUS_OK) {
        (void)fprintf(stderr, "%s: fixture graph failed\n", name);
        return 1;
    }
    status = trifact_label_vector_create(&labels, data, label_count);
    if (status != TRIFACT_STATUS_OK) {
        trifact_hypergraph_destroy(hypergraph);
        (void)fprintf(stderr, "%s: fixture labels failed\n", name);
        return 1;
    }
    status = trifact_relation_validate(hypergraph, factor_count, labels, &error);
    if (status != expected || error.status != expected || error.vertex_index != vertex_index ||
        error.edge_index != edge_index) {
        (void)fprintf(stderr, "%s: relation status or diagnostic mismatch\n", name);
        failed = 1;
    }
    if (trifact_relation_validate(hypergraph, factor_count, labels, NULL) != expected) {
        (void)fprintf(stderr, "%s: optional diagnostic mismatch\n", name);
        failed = 1;
    }
    trifact_label_vector_destroy(labels);
    trifact_hypergraph_destroy(hypergraph);
    return failed;
}

int main(void) {
    const trifact_edge_t irregular_edges[] = {
        {{0U, 1U, 2U}}, {{0U, 1U, 3U}}, {{0U, 1U, 4U}}, {{3U, 4U, 5U}}};
    const trifact_factor_label_t out_of_range[] = {0U, 1U, 2U, 0U};
    const trifact_factor_label_t collision[] = {0U, 1U, 0U, 0U};
    trifact_relation_error_t error = {TRIFACT_STATUS_OK, 0U, 0U};
    int failures = 0;

    failures += check_relation("valid", 6U, valid_edges, 4U, 2U, valid_labels, 4U,
                               TRIFACT_STATUS_OK, SIZE_MAX, SIZE_MAX);
    failures += check_relation("vertex divisibility", 7U, valid_edges, 4U, 2U, valid_labels, 4U,
                               TRIFACT_STATUS_INVALID_VERTEX_COUNT, SIZE_MAX, SIZE_MAX);
    failures += check_relation("factor lower bound", 6U, valid_edges, 4U, 1U, valid_labels, 4U,
                               TRIFACT_STATUS_INVALID_FACTOR_COUNT, SIZE_MAX, SIZE_MAX);
    failures += check_relation("factor upper bound", 6U, valid_edges, 4U, 11U, valid_labels, 4U,
                               TRIFACT_STATUS_INVALID_FACTOR_COUNT, SIZE_MAX, SIZE_MAX);
    failures += check_relation("edge count", 6U, valid_edges, 3U, 2U, valid_labels, 3U,
                               TRIFACT_STATUS_EDGE_COUNT_MISMATCH, SIZE_MAX, SIZE_MAX);
    failures += check_relation("witness length", 6U, valid_edges, 4U, 2U, valid_labels, 3U,
                               TRIFACT_STATUS_WITNESS_LENGTH_MISMATCH, SIZE_MAX, SIZE_MAX);
    failures += check_relation("label range", 6U, valid_edges, 4U, 2U, out_of_range, 4U,
                               TRIFACT_STATUS_LABEL_OUT_OF_RANGE, SIZE_MAX, 2U);
    failures += check_relation("vertex degree", 6U, irregular_edges, 4U, 2U, valid_labels, 4U,
                               TRIFACT_STATUS_VERTEX_DEGREE_MISMATCH, 0U, SIZE_MAX);
    failures += check_relation("incident collision", 6U, valid_edges, 4U, 2U, collision, 4U,
                               TRIFACT_STATUS_INCIDENT_LABEL_COLLISION, 2U, 2U);
    failures += check_relation("empty graph", 6U, NULL, 0U, 2U, NULL, 0U,
                               TRIFACT_STATUS_EDGE_COUNT_MISMATCH, SIZE_MAX, SIZE_MAX);
    if (trifact_relation_validate(NULL, 2U, NULL, &error) != TRIFACT_STATUS_NULL_ARGUMENT ||
        error.status != TRIFACT_STATUS_NULL_ARGUMENT || error.vertex_index != SIZE_MAX ||
        error.edge_index != SIZE_MAX) {
        ++failures;
    }
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
