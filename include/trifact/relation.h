#ifndef TRIFACT_RELATION_H
#define TRIFACT_RELATION_H

#include "trifact/core.h"

typedef struct {
    trifact_status_t status;
    size_t vertex_index;
    size_t edge_index;
} trifact_relation_error_t;

trifact_status_t trifact_relation_validate(const trifact_hypergraph_t *hypergraph,
                                           uint32_t factor_count,
                                           const trifact_label_vector_t *labels,
                                           trifact_relation_error_t *error);

#endif
