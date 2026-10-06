#include "trifact/relation.h"

#include <stdlib.h>

static trifact_status_t relation_result(trifact_relation_error_t *error, trifact_status_t status,
                                        size_t vertex_index, size_t edge_index) {
    if (error != NULL) {
        error->status = status;
        error->vertex_index = vertex_index;
        error->edge_index = edge_index;
    }
    return status;
}

trifact_status_t trifact_relation_validate(const trifact_hypergraph_t *hypergraph,
                                           uint32_t factor_count,
                                           const trifact_label_vector_t *labels,
                                           trifact_relation_error_t *error) {
    size_t *degrees = NULL;
    unsigned char *seen = NULL;
    const trifact_edge_t *edges = NULL;
    const trifact_factor_label_t *data = NULL;
    size_t vertex_count = 0U;
    size_t edge_count = 0U;
    size_t index = 0U;
    size_t slot = 0U;
    size_t seen_count = 0U;
    uint64_t maximum_degree = 0U;

    if (hypergraph == NULL || labels == NULL) {
        return relation_result(error, TRIFACT_STATUS_NULL_ARGUMENT, SIZE_MAX, SIZE_MAX);
    }
    vertex_count = (size_t)trifact_hypergraph_vertex_count(hypergraph);
    edge_count = trifact_hypergraph_edge_count(hypergraph);
    edges = trifact_hypergraph_edges(hypergraph);
    data = trifact_label_vector_data(labels);

    if (vertex_count < 6U || vertex_count % 3U != 0U) {
        return relation_result(error, TRIFACT_STATUS_INVALID_VERTEX_COUNT, SIZE_MAX, SIZE_MAX);
    }
    maximum_degree = (uint64_t)(vertex_count - 1U) * (uint64_t)(vertex_count - 2U) / 2U;
    if (factor_count < 2U || (uint64_t)factor_count > maximum_degree) {
        return relation_result(error, TRIFACT_STATUS_INVALID_FACTOR_COUNT, SIZE_MAX, SIZE_MAX);
    }
    if (vertex_count / 3U > SIZE_MAX / (size_t)factor_count) {
        return relation_result(error, TRIFACT_STATUS_SIZE_OVERFLOW, SIZE_MAX, SIZE_MAX);
    }
    if (edge_count != (vertex_count / 3U) * (size_t)factor_count) {
        return relation_result(error, TRIFACT_STATUS_EDGE_COUNT_MISMATCH, SIZE_MAX, SIZE_MAX);
    }
    if (trifact_label_vector_count(labels) != edge_count) {
        return relation_result(error, TRIFACT_STATUS_WITNESS_LENGTH_MISMATCH, SIZE_MAX, SIZE_MAX);
    }
    for (index = 0U; index < edge_count; ++index) {
        if (data[index] >= factor_count) {
            return relation_result(error, TRIFACT_STATUS_LABEL_OUT_OF_RANGE, SIZE_MAX, index);
        }
    }
    if (vertex_count > SIZE_MAX / sizeof(*degrees) ||
        vertex_count > SIZE_MAX / (size_t)factor_count) {
        return relation_result(error, TRIFACT_STATUS_SIZE_OVERFLOW, SIZE_MAX, SIZE_MAX);
    }
    seen_count = vertex_count * (size_t)factor_count;
    degrees = calloc(vertex_count, sizeof(*degrees));
    if (degrees == NULL) {
        return relation_result(error, TRIFACT_STATUS_ALLOCATION_FAILURE, SIZE_MAX, SIZE_MAX);
    }
    for (index = 0U; index < edge_count; ++index) {
        for (slot = 0U; slot < 3U; ++slot) {
            ++degrees[edges[index].vertices[slot]];
        }
    }
    for (index = 0U; index < vertex_count; ++index) {
        if (degrees[index] != (size_t)factor_count) {
            free(degrees);
            return relation_result(error, TRIFACT_STATUS_VERTEX_DEGREE_MISMATCH, index, SIZE_MAX);
        }
    }
    free(degrees);
    seen = calloc(seen_count, sizeof(*seen));
    if (seen == NULL) {
        return relation_result(error, TRIFACT_STATUS_ALLOCATION_FAILURE, SIZE_MAX, SIZE_MAX);
    }
    for (index = 0U; index < edge_count; ++index) {
        for (slot = 0U; slot < 3U; ++slot) {
            const size_t vertex = (size_t)edges[index].vertices[slot];
            const size_t offset = vertex * (size_t)factor_count + (size_t)data[index];

            if (seen[offset] != 0U) {
                free(seen);
                return relation_result(error, TRIFACT_STATUS_INCIDENT_LABEL_COLLISION, vertex,
                                       index);
            }
            seen[offset] = 1U;
        }
    }
    free(seen);
    return relation_result(error, TRIFACT_STATUS_OK, SIZE_MAX, SIZE_MAX);
}
