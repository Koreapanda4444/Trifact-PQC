#include "trifact/core.h"
#include "memory.h"

#include <stdint.h>
#include <string.h>

struct trifact_hypergraph {
    trifact_vertex_t vertex_count;
    size_t edge_count;
    trifact_edge_t *edges;
};

struct trifact_label_vector {
    size_t count;
    trifact_factor_label_t *labels;
};

static trifact_status_t validate_edge(const trifact_edge_t *edge, trifact_vertex_t vertex_count) {
    if (edge->vertices[0] >= vertex_count || edge->vertices[1] >= vertex_count ||
        edge->vertices[2] >= vertex_count) {
        return TRIFACT_STATUS_VERTEX_OUT_OF_RANGE;
    }

    if (edge->vertices[0] >= edge->vertices[1] || edge->vertices[1] >= edge->vertices[2]) {
        return TRIFACT_STATUS_EDGE_NOT_CANONICAL;
    }

    return TRIFACT_STATUS_OK;
}

static int compare_edges(const trifact_edge_t *left, const trifact_edge_t *right) {
    size_t index = 0U;

    for (index = 0U; index < 3U; ++index) {
        if (left->vertices[index] < right->vertices[index]) {
            return -1;
        }
        if (left->vertices[index] > right->vertices[index]) {
            return 1;
        }
    }

    return 0;
}

trifact_status_t trifact_edge_init(trifact_edge_t *out_edge, trifact_vertex_t vertex_count,
                                   trifact_vertex_t first, trifact_vertex_t second,
                                   trifact_vertex_t third) {
    trifact_edge_t candidate = {{first, second, third}};
    trifact_status_t status = TRIFACT_STATUS_OK;

    if (out_edge == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    if (vertex_count < 3U) {
        return TRIFACT_STATUS_INVALID_VERTEX_COUNT;
    }

    status = validate_edge(&candidate, vertex_count);
    if (status != TRIFACT_STATUS_OK) {
        return status;
    }

    *out_edge = candidate;
    return TRIFACT_STATUS_OK;
}

trifact_status_t trifact_hypergraph_create(trifact_hypergraph_t **out_hypergraph,
                                           trifact_vertex_t vertex_count,
                                           const trifact_edge_t *edges, size_t edge_count) {
    trifact_hypergraph_t *hypergraph = NULL;
    trifact_edge_t *edge_copy = NULL;
    size_t index = 0U;

    if (out_hypergraph == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    *out_hypergraph = NULL;

    if (vertex_count < 3U) {
        return TRIFACT_STATUS_INVALID_VERTEX_COUNT;
    }
    if (edge_count > 0U && edges == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    if (edge_count > (size_t)UINT32_MAX) {
        return TRIFACT_STATUS_COUNT_OUT_OF_RANGE;
    }

    for (index = 0U; index < edge_count; ++index) {
        const trifact_status_t status = validate_edge(&edges[index], vertex_count);

        if (status != TRIFACT_STATUS_OK) {
            return status;
        }
        if (index > 0U) {
            const int comparison = compare_edges(&edges[index - 1U], &edges[index]);

            if (comparison == 0) {
                return TRIFACT_STATUS_DUPLICATE_EDGE;
            }
            if (comparison > 0) {
                return TRIFACT_STATUS_EDGE_LIST_NOT_SORTED;
            }
        }
    }

    if (edge_count > SIZE_MAX / sizeof(*edge_copy)) {
        return TRIFACT_STATUS_SIZE_OVERFLOW;
    }

    hypergraph = trifact_memory_allocate(sizeof(*hypergraph));
    if (hypergraph == NULL) {
        return TRIFACT_STATUS_ALLOCATION_FAILURE;
    }

    if (edge_count > 0U) {
        const size_t byte_count = edge_count * sizeof(*edge_copy);

        edge_copy = trifact_memory_allocate(byte_count);
        if (edge_copy == NULL) {
            trifact_memory_free(hypergraph);
            return TRIFACT_STATUS_ALLOCATION_FAILURE;
        }
        (void)memcpy(edge_copy, edges, byte_count);
    }

    hypergraph->vertex_count = vertex_count;
    hypergraph->edge_count = edge_count;
    hypergraph->edges = edge_copy;
    *out_hypergraph = hypergraph;
    return TRIFACT_STATUS_OK;
}

void trifact_hypergraph_destroy(trifact_hypergraph_t *hypergraph) {
    if (hypergraph != NULL) {
        trifact_memory_free(hypergraph->edges);
        trifact_memory_free(hypergraph);
    }
}

trifact_vertex_t trifact_hypergraph_vertex_count(const trifact_hypergraph_t *hypergraph) {
    return hypergraph == NULL ? 0U : hypergraph->vertex_count;
}

size_t trifact_hypergraph_edge_count(const trifact_hypergraph_t *hypergraph) {
    return hypergraph == NULL ? 0U : hypergraph->edge_count;
}

const trifact_edge_t *trifact_hypergraph_edges(const trifact_hypergraph_t *hypergraph) {
    return hypergraph == NULL ? NULL : hypergraph->edges;
}

trifact_status_t trifact_label_vector_create(trifact_label_vector_t **out_labels,
                                             const trifact_factor_label_t *labels,
                                             size_t label_count) {
    trifact_label_vector_t *vector = NULL;
    trifact_factor_label_t *label_copy = NULL;

    if (out_labels == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    *out_labels = NULL;

    if (label_count > 0U && labels == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    if (label_count > (size_t)UINT32_MAX) {
        return TRIFACT_STATUS_COUNT_OUT_OF_RANGE;
    }
    if (label_count > SIZE_MAX / sizeof(*label_copy)) {
        return TRIFACT_STATUS_SIZE_OVERFLOW;
    }

    vector = trifact_memory_allocate(sizeof(*vector));
    if (vector == NULL) {
        return TRIFACT_STATUS_ALLOCATION_FAILURE;
    }

    if (label_count > 0U) {
        const size_t byte_count = label_count * sizeof(*label_copy);

        label_copy = trifact_memory_allocate(byte_count);
        if (label_copy == NULL) {
            trifact_memory_free(vector);
            return TRIFACT_STATUS_ALLOCATION_FAILURE;
        }
        (void)memcpy(label_copy, labels, byte_count);
    }

    vector->count = label_count;
    vector->labels = label_copy;
    *out_labels = vector;
    return TRIFACT_STATUS_OK;
}

void trifact_label_vector_destroy(trifact_label_vector_t *labels) {
    if (labels != NULL) {
        trifact_memory_free(labels->labels);
        trifact_memory_free(labels);
    }
}

size_t trifact_label_vector_count(const trifact_label_vector_t *labels) {
    return labels == NULL ? 0U : labels->count;
}

const trifact_factor_label_t *trifact_label_vector_data(const trifact_label_vector_t *labels) {
    return labels == NULL ? NULL : labels->labels;
}
