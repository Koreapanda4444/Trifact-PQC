#include "failing_allocator.h"
#include "trifact/trifact.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    trifact_hypergraph_t *graph;
    trifact_label_vector_t *labels;
    trifact_label_vector_t *empty;
    trifact_incidence_index_t *index;
} fixture_t;

typedef enum {
    CREATE_GRAPH,
    CREATE_EMPTY_GRAPH,
    CREATE_LABELS,
    CREATE_EMPTY_LABELS,
    CREATE_INDEX,
    NORMALIZE_LABELS,
    NORMALIZE_EMPTY_LABELS,
    VALIDATE_RELATION,
    VALIDATE_WITHOUT_DIAGNOSTIC,
    OPERATION_COUNT
} operation_t;

static const trifact_edge_t edges[] = {
    {{0U, 1U, 2U}}, {{0U, 1U, 3U}}, {{2U, 4U, 5U}}, {{3U, 4U, 5U}}};
static const trifact_factor_label_t data[] = {0U, 1U, 1U, 0U};

static int fixture_unchanged(const fixture_t *fixture) {
    const size_t *row = NULL;
    size_t count = 0U;

    return trifact_hypergraph_vertex_count(fixture->graph) == 6U &&
           trifact_hypergraph_edge_count(fixture->graph) == 4U &&
           memcmp(trifact_hypergraph_edges(fixture->graph), edges, sizeof(edges)) == 0 &&
           trifact_label_vector_count(fixture->labels) == 4U &&
           memcmp(trifact_label_vector_data(fixture->labels), data, sizeof(data)) == 0 &&
           trifact_label_vector_count(fixture->empty) == 0U &&
           trifact_incidence_index_vertex_edges(fixture->index, 2U, &row, &count) ==
               TRIFACT_STATUS_OK &&
           count == 2U && row != NULL && row[0] == 0U && row[1] == 2U;
}

static int check_operation(operation_t operation, const fixture_t *fixture, size_t failure_call,
                           size_t allocation_count) {
    trifact_hypergraph_t *graph = fixture->graph;
    trifact_label_vector_t *labels = fixture->labels;
    trifact_incidence_index_t *index = fixture->index;
    trifact_relation_error_t error = {TRIFACT_STATUS_NULL_ARGUMENT, 0U, 0U};
    const size_t baseline = trifact_test_allocator_live();
    const int injects_failure = failure_call > 0U && failure_call <= allocation_count;
    const trifact_status_t expected =
        injects_failure != 0 ? TRIFACT_STATUS_ALLOCATION_FAILURE : TRIFACT_STATUS_OK;
    trifact_status_t status = TRIFACT_STATUS_NULL_ARGUMENT;
    int failed = 0;

    trifact_test_allocator_reset(failure_call);
    switch (operation) {
    case CREATE_GRAPH:
        status = trifact_hypergraph_create(&graph, 6U, edges, 4U);
        break;
    case CREATE_EMPTY_GRAPH:
        status = trifact_hypergraph_create(&graph, 6U, NULL, 0U);
        break;
    case CREATE_LABELS:
        status = trifact_label_vector_create(&labels, data, 4U);
        break;
    case CREATE_EMPTY_LABELS:
        status = trifact_label_vector_create(&labels, NULL, 0U);
        break;
    case CREATE_INDEX:
        status = trifact_incidence_index_create(&index, fixture->graph, 2U);
        break;
    case NORMALIZE_LABELS:
        status = trifact_label_vector_normalize(&labels, fixture->labels);
        break;
    case NORMALIZE_EMPTY_LABELS:
        status = trifact_label_vector_normalize(&labels, fixture->empty);
        break;
    case VALIDATE_RELATION:
        status = trifact_relation_validate(fixture->graph, 2U, fixture->labels, &error);
        if (error.status != status || error.vertex_index != SIZE_MAX ||
            error.edge_index != SIZE_MAX) {
            failed = 1;
        }
        break;
    case VALIDATE_WITHOUT_DIAGNOSTIC:
        status = trifact_relation_validate(fixture->graph, 2U, fixture->labels, NULL);
        break;
    case OPERATION_COUNT:
        return 1;
    }
    if (status != expected ||
        (injects_failure != 0 && trifact_test_allocator_calls() != failure_call) ||
        (failure_call > allocation_count && trifact_test_allocator_calls() != allocation_count)) {
        failed = 1;
    }
    switch (operation) {
    case CREATE_GRAPH:
    case CREATE_EMPTY_GRAPH:
        if (status == TRIFACT_STATUS_OK) {
            const size_t count = operation == CREATE_GRAPH ? 4U : 0U;

            if (graph == NULL || graph == fixture->graph ||
                trifact_hypergraph_edge_count(graph) != count) {
                failed = 1;
            }
            if (graph != fixture->graph) {
                trifact_hypergraph_destroy(graph);
            }
            graph = NULL;
            trifact_hypergraph_destroy(graph);
        } else if (graph != NULL) {
            failed = 1;
        }
        break;
    case CREATE_LABELS:
    case CREATE_EMPTY_LABELS:
    case NORMALIZE_LABELS:
    case NORMALIZE_EMPTY_LABELS:
        if (status == TRIFACT_STATUS_OK) {
            const size_t count =
                operation == CREATE_LABELS || operation == NORMALIZE_LABELS ? 4U : 0U;

            if (labels == NULL || labels == fixture->labels ||
                trifact_label_vector_count(labels) != count ||
                (count > 0U &&
                 memcmp(trifact_label_vector_data(labels), data, sizeof(data)) != 0)) {
                failed = 1;
            }
            if (labels != fixture->labels) {
                trifact_label_vector_destroy(labels);
            }
            labels = NULL;
            trifact_label_vector_destroy(labels);
        } else if (labels != NULL) {
            failed = 1;
        }
        break;
    case CREATE_INDEX:
        if (status == TRIFACT_STATUS_OK) {
            if (index == NULL || index == fixture->index ||
                trifact_incidence_index_vertex_count(index) != 6U ||
                trifact_incidence_index_edge_count(index) != 4U) {
                failed = 1;
            }
            if (index != fixture->index) {
                trifact_incidence_index_destroy(index);
            }
            index = NULL;
            trifact_incidence_index_destroy(index);
        } else if (index != NULL) {
            failed = 1;
        }
        break;
    case VALIDATE_RELATION:
    case VALIDATE_WITHOUT_DIAGNOSTIC:
    case OPERATION_COUNT:
        break;
    }
    if (trifact_test_allocator_live() != baseline || trifact_test_allocator_errors() != 0U ||
        fixture_unchanged(fixture) == 0) {
        failed = 1;
    }
    if (failed != 0) {
        (void)fprintf(stderr, "allocation check failed: operation=%d failure=%zu status=%d\n",
                      (int)operation, failure_call, (int)status);
    }
    return failed;
}

static int check_rejected_inputs(const fixture_t *fixture) {
    const trifact_edge_t irregular[] = {
        {{0U, 1U, 2U}}, {{0U, 1U, 3U}}, {{0U, 1U, 4U}}, {{3U, 4U, 5U}}};
    const trifact_factor_label_t collision[] = {0U, 1U, 0U, 0U};
    trifact_hypergraph_t *graph = NULL;
    trifact_label_vector_t *labels = NULL;
    trifact_incidence_index_t *index = fixture->index;
    trifact_edge_t edge = {{0U, 1U, 5U}};
    const trifact_edge_t saved_edge = edge;
    const size_t baseline = trifact_test_allocator_live();
    int equivalent = -1;
    int failed = 0;

    trifact_test_allocator_reset(0U);
    if (trifact_hypergraph_create(&graph, 6U, irregular, 4U) != TRIFACT_STATUS_OK ||
        trifact_label_vector_create(&labels, collision, 4U) != TRIFACT_STATUS_OK) {
        failed = 1;
    } else {
        const size_t with_inputs = trifact_test_allocator_live();

        trifact_test_allocator_reset(0U);
        if (trifact_relation_validate(graph, 2U, fixture->labels, NULL) !=
                TRIFACT_STATUS_VERTEX_DEGREE_MISMATCH ||
            trifact_test_allocator_live() != with_inputs || trifact_test_allocator_calls() != 1U) {
            failed = 1;
        }
        trifact_test_allocator_reset(0U);
        if (trifact_incidence_index_create(&index, graph, 2U) !=
                TRIFACT_STATUS_VERTEX_DEGREE_MISMATCH ||
            index != NULL || trifact_test_allocator_live() != with_inputs ||
            trifact_test_allocator_calls() != 1U) {
            failed = 1;
        }
        trifact_test_allocator_reset(0U);
        if (trifact_relation_validate(fixture->graph, 2U, labels, NULL) !=
                TRIFACT_STATUS_INCIDENT_LABEL_COLLISION ||
            trifact_test_allocator_live() != with_inputs || trifact_test_allocator_calls() != 2U) {
            failed = 1;
        }
    }
    trifact_label_vector_destroy(labels);
    trifact_hypergraph_destroy(graph);
    labels = fixture->labels;
    graph = fixture->graph;
    index = fixture->index;
    trifact_test_allocator_reset(1U);
    if (trifact_label_vector_normalize(&labels, NULL) != TRIFACT_STATUS_NULL_ARGUMENT ||
        labels != NULL ||
        trifact_hypergraph_create(&graph, 2U, NULL, 0U) != TRIFACT_STATUS_INVALID_VERTEX_COUNT ||
        graph != NULL ||
        trifact_incidence_index_create(&index, NULL, 2U) != TRIFACT_STATUS_NULL_ARGUMENT ||
        index != NULL ||
        trifact_edge_init(&edge, 6U, 0U, 1U, 6U) != TRIFACT_STATUS_VERTEX_OUT_OF_RANGE ||
        memcmp(&edge, &saved_edge, sizeof(edge)) != 0 ||
        trifact_label_vectors_equivalent(fixture->labels, fixture->labels, &equivalent) !=
            TRIFACT_STATUS_OK ||
        equivalent != 1 ||
        trifact_label_vectors_equivalent(NULL, fixture->labels, &equivalent) !=
            TRIFACT_STATUS_NULL_ARGUMENT ||
        equivalent != 1 || trifact_test_allocator_calls() != 0U ||
        trifact_test_allocator_live() != baseline || trifact_test_allocator_errors() != 0U ||
        fixture_unchanged(fixture) == 0) {
        failed = 1;
    }
    return failed;
}

int main(void) {
    fixture_t fixture = {NULL, NULL, NULL, NULL};
    int operation = 0;
    int failures = 0;

    trifact_test_allocator_reset(0U);
    if (trifact_hypergraph_create(&fixture.graph, 6U, edges, 4U) != TRIFACT_STATUS_OK ||
        trifact_label_vector_create(&fixture.labels, data, 4U) != TRIFACT_STATUS_OK ||
        trifact_label_vector_create(&fixture.empty, NULL, 0U) != TRIFACT_STATUS_OK ||
        trifact_incidence_index_create(&fixture.index, fixture.graph, 2U) != TRIFACT_STATUS_OK) {
        failures = 1;
    } else {
        for (operation = 0; operation < (int)OPERATION_COUNT; ++operation) {
            size_t allocation_count = 0U;
            size_t failure_call = 0U;

            failures += check_operation((operation_t)operation, &fixture, 0U, 0U);
            allocation_count = trifact_test_allocator_calls();
            if (allocation_count == 0U) {
                ++failures;
                continue;
            }
            for (failure_call = 1U; failure_call <= allocation_count + 1U; ++failure_call) {
                failures += check_operation((operation_t)operation, &fixture, failure_call,
                                            allocation_count);
            }
        }
        failures += check_rejected_inputs(&fixture);
    }
    trifact_incidence_index_destroy(fixture.index);
    trifact_label_vector_destroy(fixture.empty);
    trifact_label_vector_destroy(fixture.labels);
    trifact_hypergraph_destroy(fixture.graph);
    fixture = (fixture_t){NULL, NULL, NULL, NULL};
    trifact_incidence_index_destroy(fixture.index);
    trifact_label_vector_destroy(fixture.empty);
    trifact_label_vector_destroy(fixture.labels);
    trifact_hypergraph_destroy(fixture.graph);
    if (trifact_test_allocator_live() != 0U || trifact_test_allocator_errors() != 0U) {
        ++failures;
    }
    if (failures != 0) {
        (void)fprintf(stderr, "allocation and cleanup checks failed: %d\n", failures);
    }
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
