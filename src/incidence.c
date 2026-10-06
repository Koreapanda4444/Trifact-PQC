#include "trifact/incidence.h"
#include "memory.h"

struct trifact_incidence_index {
    trifact_vertex_t vertex_count;
    uint32_t factor_count;
    size_t edge_count;
    size_t *entries;
};

trifact_status_t trifact_incidence_index_create(trifact_incidence_index_t **out_index,
                                                const trifact_hypergraph_t *hypergraph,
                                                uint32_t factor_count) {
    trifact_incidence_index_t *index = NULL;
    const trifact_edge_t *edges = NULL;
    size_t *degrees = NULL;
    size_t vertex_count = 0U;
    size_t edge_count = 0U;
    size_t entry_count = 0U;
    size_t edge = 0U;
    size_t vertex = 0U;
    size_t slot = 0U;
    uint64_t maximum_degree = 0U;

    if (out_index == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    *out_index = NULL;
    if (hypergraph == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    vertex_count = (size_t)trifact_hypergraph_vertex_count(hypergraph);
    edge_count = trifact_hypergraph_edge_count(hypergraph);
    edges = trifact_hypergraph_edges(hypergraph);
    if (vertex_count < 6U || vertex_count % 3U != 0U) {
        return TRIFACT_STATUS_INVALID_VERTEX_COUNT;
    }
    maximum_degree = (uint64_t)(vertex_count - 1U) * (uint64_t)(vertex_count - 2U) / 2U;
    if (factor_count < 2U || (uint64_t)factor_count > maximum_degree) {
        return TRIFACT_STATUS_INVALID_FACTOR_COUNT;
    }
    if (vertex_count / 3U > SIZE_MAX / (size_t)factor_count) {
        return TRIFACT_STATUS_SIZE_OVERFLOW;
    }
    if (edge_count != (vertex_count / 3U) * (size_t)factor_count) {
        return TRIFACT_STATUS_EDGE_COUNT_MISMATCH;
    }
    if (vertex_count > SIZE_MAX / (size_t)factor_count ||
        vertex_count > SIZE_MAX / sizeof(*degrees)) {
        return TRIFACT_STATUS_SIZE_OVERFLOW;
    }
    entry_count = vertex_count * (size_t)factor_count;
    if (entry_count > SIZE_MAX / sizeof(*index->entries)) {
        return TRIFACT_STATUS_SIZE_OVERFLOW;
    }
    degrees = trifact_memory_allocate_zero(vertex_count, sizeof(*degrees));
    if (degrees == NULL) {
        return TRIFACT_STATUS_ALLOCATION_FAILURE;
    }
    for (edge = 0U; edge < edge_count; ++edge) {
        for (slot = 0U; slot < 3U; ++slot) {
            ++degrees[edges[edge].vertices[slot]];
        }
    }
    for (vertex = 0U; vertex < vertex_count; ++vertex) {
        if (degrees[vertex] != (size_t)factor_count) {
            trifact_memory_free(degrees);
            return TRIFACT_STATUS_VERTEX_DEGREE_MISMATCH;
        }
        degrees[vertex] = 0U;
    }
    index = trifact_memory_allocate(sizeof(*index));
    if (index == NULL) {
        trifact_memory_free(degrees);
        return TRIFACT_STATUS_ALLOCATION_FAILURE;
    }
    index->entries = trifact_memory_allocate(entry_count * sizeof(*index->entries));
    if (index->entries == NULL) {
        trifact_memory_free(index);
        trifact_memory_free(degrees);
        return TRIFACT_STATUS_ALLOCATION_FAILURE;
    }
    for (edge = 0U; edge < edge_count; ++edge) {
        for (slot = 0U; slot < 3U; ++slot) {
            vertex = (size_t)edges[edge].vertices[slot];
            index->entries[vertex * (size_t)factor_count + degrees[vertex]] = edge;
            ++degrees[vertex];
        }
    }
    trifact_memory_free(degrees);
    index->vertex_count = (trifact_vertex_t)vertex_count;
    index->factor_count = factor_count;
    index->edge_count = edge_count;
    *out_index = index;
    return TRIFACT_STATUS_OK;
}

void trifact_incidence_index_destroy(trifact_incidence_index_t *index) {
    if (index != NULL) {
        trifact_memory_free(index->entries);
        trifact_memory_free(index);
    }
}

trifact_vertex_t trifact_incidence_index_vertex_count(const trifact_incidence_index_t *index) {
    return index == NULL ? 0U : index->vertex_count;
}

size_t trifact_incidence_index_edge_count(const trifact_incidence_index_t *index) {
    return index == NULL ? 0U : index->edge_count;
}

uint32_t trifact_incidence_index_factor_count(const trifact_incidence_index_t *index) {
    return index == NULL ? 0U : index->factor_count;
}

trifact_status_t trifact_incidence_index_vertex_edges(const trifact_incidence_index_t *index,
                                                      trifact_vertex_t vertex,
                                                      const size_t **out_edges, size_t *out_count) {
    if (out_edges != NULL) {
        *out_edges = NULL;
    }
    if (out_count != NULL) {
        *out_count = 0U;
    }
    if (index == NULL || out_edges == NULL || out_count == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    if (vertex >= index->vertex_count) {
        return TRIFACT_STATUS_VERTEX_OUT_OF_RANGE;
    }
    *out_edges = &index->entries[(size_t)vertex * (size_t)index->factor_count];
    *out_count = (size_t)index->factor_count;
    return TRIFACT_STATUS_OK;
}
