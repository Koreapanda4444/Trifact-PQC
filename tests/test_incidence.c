#include "trifact/incidence.h"

#include <stdio.h>
#include <stdlib.h>

static const trifact_edge_t valid_edges[] = {
    {{0U, 1U, 2U}}, {{0U, 1U, 3U}}, {{2U, 4U, 5U}}, {{3U, 4U, 5U}}};

static int test_valid_index(void) {
    const size_t expected[6][2] = {{0U, 1U}, {0U, 1U}, {0U, 2U}, {1U, 3U}, {2U, 3U}, {2U, 3U}};
    trifact_hypergraph_t *hypergraph = NULL;
    trifact_incidence_index_t *index = NULL;
    const size_t *entries = NULL;
    size_t count = 0U;
    size_t total = 0U;
    trifact_vertex_t vertex = 0U;
    int failures = 0;

    if (trifact_hypergraph_create(&hypergraph, 6U, valid_edges, 4U) != TRIFACT_STATUS_OK ||
        trifact_incidence_index_create(&index, hypergraph, 2U) != TRIFACT_STATUS_OK) {
        trifact_hypergraph_destroy(hypergraph);
        return 1;
    }
    trifact_hypergraph_destroy(hypergraph);
    hypergraph = NULL;
    if (trifact_incidence_index_vertex_count(index) != 6U ||
        trifact_incidence_index_edge_count(index) != 4U ||
        trifact_incidence_index_factor_count(index) != 2U) {
        ++failures;
    }
    for (vertex = 0U; vertex < 6U; ++vertex) {
        if (trifact_incidence_index_vertex_edges(index, vertex, &entries, &count) !=
                TRIFACT_STATUS_OK ||
            count != 2U || entries == NULL) {
            ++failures;
            continue;
        }
        if (entries[0] != expected[vertex][0] || entries[1] != expected[vertex][1]) {
            ++failures;
        }
        total += count;
    }
    if (total != 12U ||
        trifact_incidence_index_vertex_edges(index, 6U, &entries, &count) !=
            TRIFACT_STATUS_VERTEX_OUT_OF_RANGE ||
        entries != NULL || count != 0U) {
        ++failures;
    }
    count = 2U;
    if (trifact_incidence_index_vertex_edges(index, 0U, NULL, &count) !=
            TRIFACT_STATUS_NULL_ARGUMENT ||
        count != 0U ||
        trifact_incidence_index_vertex_edges(index, 0U, &entries, NULL) !=
            TRIFACT_STATUS_NULL_ARGUMENT ||
        entries != NULL) {
        ++failures;
    }
    trifact_incidence_index_destroy(index);
    index = NULL;
    trifact_incidence_index_destroy(index);
    if (trifact_incidence_index_vertex_count(NULL) != 0U ||
        trifact_incidence_index_edge_count(NULL) != 0U ||
        trifact_incidence_index_factor_count(NULL) != 0U ||
        trifact_incidence_index_vertex_edges(NULL, 0U, &entries, &count) !=
            TRIFACT_STATUS_NULL_ARGUMENT) {
        ++failures;
    }
    return failures;
}

static int check_invalid(trifact_vertex_t vertices, const trifact_edge_t *edges, size_t edge_count,
                         uint32_t factors, trifact_status_t expected) {
    trifact_hypergraph_t *hypergraph = NULL;
    trifact_incidence_index_t *index = NULL;
    int failed = 0;

    if (trifact_hypergraph_create(&hypergraph, vertices, edges, edge_count) != TRIFACT_STATUS_OK) {
        return 1;
    }
    if (trifact_incidence_index_create(&index, hypergraph, factors) != expected || index != NULL) {
        failed = 1;
    }
    trifact_incidence_index_destroy(index);
    trifact_hypergraph_destroy(hypergraph);
    return failed;
}

int main(void) {
    const trifact_edge_t irregular[] = {
        {{0U, 1U, 2U}}, {{0U, 1U, 3U}}, {{0U, 1U, 4U}}, {{3U, 4U, 5U}}};
    trifact_incidence_index_t *index = NULL;
    int failures = test_valid_index();

    failures += check_invalid(7U, valid_edges, 4U, 2U, TRIFACT_STATUS_INVALID_VERTEX_COUNT);
    failures += check_invalid(6U, valid_edges, 4U, 1U, TRIFACT_STATUS_INVALID_FACTOR_COUNT);
    failures += check_invalid(6U, valid_edges, 4U, 11U, TRIFACT_STATUS_INVALID_FACTOR_COUNT);
    failures += check_invalid(6U, valid_edges, 3U, 2U, TRIFACT_STATUS_EDGE_COUNT_MISMATCH);
    failures += check_invalid(6U, irregular, 4U, 2U, TRIFACT_STATUS_VERTEX_DEGREE_MISMATCH);
    if (trifact_incidence_index_create(NULL, NULL, 2U) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_incidence_index_create(&index, NULL, 2U) != TRIFACT_STATUS_NULL_ARGUMENT ||
        index != NULL) {
        ++failures;
    }
    if (failures != 0) {
        (void)fprintf(stderr, "incidence checks failed: %d\n", failures);
    }
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
