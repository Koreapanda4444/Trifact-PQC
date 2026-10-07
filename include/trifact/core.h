#ifndef TRIFACT_CORE_H
#define TRIFACT_CORE_H

#include <stddef.h>
#include <stdint.h>

typedef uint32_t trifact_vertex_t;
typedef uint32_t trifact_factor_label_t;

typedef struct {
    trifact_vertex_t vertices[3];
} trifact_edge_t;

typedef struct trifact_hypergraph trifact_hypergraph_t;
typedef struct trifact_label_vector trifact_label_vector_t;

typedef enum {
    TRIFACT_STATUS_OK = 0,
    TRIFACT_STATUS_NULL_ARGUMENT,
    TRIFACT_STATUS_INVALID_VERTEX_COUNT,
    TRIFACT_STATUS_VERTEX_OUT_OF_RANGE,
    TRIFACT_STATUS_EDGE_NOT_CANONICAL,
    TRIFACT_STATUS_EDGE_LIST_NOT_SORTED,
    TRIFACT_STATUS_DUPLICATE_EDGE,
    TRIFACT_STATUS_COUNT_OUT_OF_RANGE,
    TRIFACT_STATUS_SIZE_OVERFLOW,
    TRIFACT_STATUS_ALLOCATION_FAILURE,
    TRIFACT_STATUS_INVALID_FACTOR_COUNT,
    TRIFACT_STATUS_EDGE_COUNT_MISMATCH,
    TRIFACT_STATUS_WITNESS_LENGTH_MISMATCH,
    TRIFACT_STATUS_VERTEX_DEGREE_MISMATCH,
    TRIFACT_STATUS_LABEL_OUT_OF_RANGE,
    TRIFACT_STATUS_INCIDENT_LABEL_COLLISION,
    TRIFACT_STATUS_INVALID_STATE,
    TRIFACT_STATUS_INVALID_DOMAIN,
    TRIFACT_STATUS_INVALID_PRIMITIVE_MODE,
    TRIFACT_STATUS_INTERRUPTED,
    TRIFACT_STATUS_RANDOMNESS_FAILURE,
    TRIFACT_STATUS_PLATFORM_UNAVAILABLE
} trifact_status_t;

trifact_status_t trifact_edge_init(trifact_edge_t *out_edge, trifact_vertex_t vertex_count,
                                   trifact_vertex_t first, trifact_vertex_t second,
                                   trifact_vertex_t third);

trifact_status_t trifact_hypergraph_create(trifact_hypergraph_t **out_hypergraph,
                                           trifact_vertex_t vertex_count,
                                           const trifact_edge_t *edges, size_t edge_count);

void trifact_hypergraph_destroy(trifact_hypergraph_t *hypergraph);
trifact_vertex_t trifact_hypergraph_vertex_count(const trifact_hypergraph_t *hypergraph);
size_t trifact_hypergraph_edge_count(const trifact_hypergraph_t *hypergraph);
const trifact_edge_t *trifact_hypergraph_edges(const trifact_hypergraph_t *hypergraph);

trifact_status_t trifact_label_vector_create(trifact_label_vector_t **out_labels,
                                             const trifact_factor_label_t *labels,
                                             size_t label_count);

void trifact_label_vector_destroy(trifact_label_vector_t *labels);
size_t trifact_label_vector_count(const trifact_label_vector_t *labels);
const trifact_factor_label_t *trifact_label_vector_data(const trifact_label_vector_t *labels);

#endif
