#ifndef TRIFACT_INCIDENCE_H
#define TRIFACT_INCIDENCE_H

#include "trifact/core.h"

typedef struct trifact_incidence_index trifact_incidence_index_t;

trifact_status_t trifact_incidence_index_create(trifact_incidence_index_t **out_index,
                                                const trifact_hypergraph_t *hypergraph,
                                                uint32_t factor_count);

void trifact_incidence_index_destroy(trifact_incidence_index_t *index);
trifact_vertex_t trifact_incidence_index_vertex_count(const trifact_incidence_index_t *index);
size_t trifact_incidence_index_edge_count(const trifact_incidence_index_t *index);
uint32_t trifact_incidence_index_factor_count(const trifact_incidence_index_t *index);

trifact_status_t trifact_incidence_index_vertex_edges(const trifact_incidence_index_t *index,
                                                      trifact_vertex_t vertex,
                                                      const size_t **out_edges, size_t *out_count);

#endif
