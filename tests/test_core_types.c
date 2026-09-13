#include "trifact/core.h"

#include <stdio.h>
#include <stdlib.h>

static int expect_status(const char *name, trifact_status_t actual, trifact_status_t expected) {
    if (actual == expected) {
        return 0;
    }

    (void)fprintf(stderr, "%s: unexpected status\n", name);
    return 1;
}

static int expect_true(const char *name, int condition) {
    if (condition != 0) {
        return 0;
    }

    (void)fprintf(stderr, "%s: condition failed\n", name);
    return 1;
}

static int test_edge_construction(void) {
    trifact_edge_t edge = {{0U, 0U, 0U}};
    trifact_status_t status = TRIFACT_STATUS_OK;
    int failures = 0;

    status = trifact_edge_init(&edge, 6U, 0U, 1U, 5U);
    failures += expect_status("canonical edge", status, TRIFACT_STATUS_OK);
    failures += expect_true("first vertex", edge.vertices[0] == 0U);
    failures += expect_true("second vertex", edge.vertices[1] == 1U);
    failures += expect_true("third vertex", edge.vertices[2] == 5U);

    status = trifact_edge_init(NULL, 6U, 0U, 1U, 2U);
    failures += expect_status("null edge output", status, TRIFACT_STATUS_NULL_ARGUMENT);

    status = trifact_edge_init(&edge, 2U, 0U, 1U, 2U);
    failures += expect_status("small vertex set", status, TRIFACT_STATUS_INVALID_VERTEX_COUNT);

    status = trifact_edge_init(&edge, 6U, 0U, 1U, 6U);
    failures += expect_status("vertex range", status, TRIFACT_STATUS_VERTEX_OUT_OF_RANGE);

    status = trifact_edge_init(&edge, 6U, 0U, 2U, 1U);
    failures += expect_status("edge order", status, TRIFACT_STATUS_EDGE_NOT_CANONICAL);

    status = trifact_edge_init(&edge, 6U, 0U, 1U, 1U);
    failures += expect_status("repeated vertex", status, TRIFACT_STATUS_EDGE_NOT_CANONICAL);

    return failures;
}

static int test_hypergraph_construction(void) {
    trifact_edge_t edges[] = {
        {{0U, 1U, 2U}},
        {{0U, 3U, 4U}},
        {{1U, 3U, 5U}},
    };
    const trifact_edge_t duplicate_edges[] = {
        {{0U, 1U, 2U}},
        {{0U, 1U, 2U}},
    };
    const trifact_edge_t unsorted_edges[] = {
        {{0U, 3U, 4U}},
        {{0U, 1U, 2U}},
    };
    const trifact_edge_t noncanonical_edge[] = {
        {{0U, 2U, 1U}},
    };
    const trifact_edge_t out_of_range_edge[] = {
        {{0U, 1U, 6U}},
    };
    trifact_hypergraph_t *hypergraph = NULL;
    const trifact_edge_t *stored_edges = NULL;
    trifact_status_t status = TRIFACT_STATUS_OK;
    int failures = 0;

    status = trifact_hypergraph_create(&hypergraph, 6U, edges, 3U);
    failures += expect_status("canonical hypergraph", status, TRIFACT_STATUS_OK);
    failures += expect_true("hypergraph created", hypergraph != NULL);
    failures += expect_true("vertex count", trifact_hypergraph_vertex_count(hypergraph) == 6U);
    failures += expect_true("edge count", trifact_hypergraph_edge_count(hypergraph) == 3U);

    stored_edges = trifact_hypergraph_edges(hypergraph);
    failures += expect_true("edge storage", stored_edges != NULL);
    failures += expect_true("owned edge copy", stored_edges != edges);
    edges[0].vertices[0] = 5U;
    failures += expect_true("copy independence", stored_edges[0].vertices[0] == 0U);
    trifact_hypergraph_destroy(hypergraph);
    hypergraph = NULL;

    status = trifact_hypergraph_create(&hypergraph, 6U, duplicate_edges, 2U);
    failures += expect_status("duplicate edge", status, TRIFACT_STATUS_DUPLICATE_EDGE);
    failures += expect_true("duplicate output", hypergraph == NULL);

    status = trifact_hypergraph_create(&hypergraph, 6U, unsorted_edges, 2U);
    failures += expect_status("edge-list order", status, TRIFACT_STATUS_EDGE_LIST_NOT_SORTED);
    failures += expect_true("unsorted output", hypergraph == NULL);

    status = trifact_hypergraph_create(&hypergraph, 6U, noncanonical_edge, 1U);
    failures += expect_status("edge canonical form", status, TRIFACT_STATUS_EDGE_NOT_CANONICAL);
    failures += expect_true("noncanonical output", hypergraph == NULL);

    status = trifact_hypergraph_create(&hypergraph, 6U, out_of_range_edge, 1U);
    failures += expect_status("edge vertex range", status, TRIFACT_STATUS_VERTEX_OUT_OF_RANGE);
    failures += expect_true("range output", hypergraph == NULL);

    status = trifact_hypergraph_create(&hypergraph, 6U, NULL, 1U);
    failures += expect_status("missing edges", status, TRIFACT_STATUS_NULL_ARGUMENT);
    failures += expect_true("missing-edge output", hypergraph == NULL);

    status = trifact_hypergraph_create(&hypergraph, 6U, NULL, 0U);
    failures += expect_status("empty edge list", status, TRIFACT_STATUS_OK);
    failures += expect_true("empty hypergraph", hypergraph != NULL);
    failures += expect_true("empty edge count", trifact_hypergraph_edge_count(hypergraph) == 0U);
    failures += expect_true("empty edge data", trifact_hypergraph_edges(hypergraph) == NULL);
    trifact_hypergraph_destroy(hypergraph);

    status = trifact_hypergraph_create(NULL, 6U, duplicate_edges, 2U);
    failures += expect_status("null hypergraph output", status, TRIFACT_STATUS_NULL_ARGUMENT);
    trifact_hypergraph_destroy(NULL);

    return failures;
}

static int test_label_vector(void) {
    trifact_factor_label_t labels[] = {2U, 0U, 1U};
    trifact_label_vector_t *vector = NULL;
    const trifact_factor_label_t *stored_labels = NULL;
    trifact_status_t status = TRIFACT_STATUS_OK;
    int failures = 0;

    status = trifact_label_vector_create(&vector, labels, 3U);
    failures += expect_status("label vector", status, TRIFACT_STATUS_OK);
    failures += expect_true("label vector created", vector != NULL);
    failures += expect_true("label count", trifact_label_vector_count(vector) == 3U);

    stored_labels = trifact_label_vector_data(vector);
    failures += expect_true("label storage", stored_labels != NULL);
    failures += expect_true("owned label copy", stored_labels != labels);
    labels[0] = 0U;
    failures += expect_true("label copy independence", stored_labels[0] == 2U);
    trifact_label_vector_destroy(vector);
    vector = NULL;

    status = trifact_label_vector_create(&vector, NULL, 1U);
    failures += expect_status("missing labels", status, TRIFACT_STATUS_NULL_ARGUMENT);
    failures += expect_true("missing-label output", vector == NULL);

    status = trifact_label_vector_create(&vector, NULL, 0U);
    failures += expect_status("empty label vector", status, TRIFACT_STATUS_OK);
    failures += expect_true("empty label count", trifact_label_vector_count(vector) == 0U);
    failures += expect_true("empty label data", trifact_label_vector_data(vector) == NULL);
    trifact_label_vector_destroy(vector);

    status = trifact_label_vector_create(NULL, labels, 3U);
    failures += expect_status("null label output", status, TRIFACT_STATUS_NULL_ARGUMENT);
    trifact_label_vector_destroy(NULL);

    return failures;
}

int main(void) {
    int failures = 0;

    failures += test_edge_construction();
    failures += test_hypergraph_construction();
    failures += test_label_vector();

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
