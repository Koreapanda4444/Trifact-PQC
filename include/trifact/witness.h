#ifndef TRIFACT_WITNESS_H
#define TRIFACT_WITNESS_H

#include "trifact/core.h"

trifact_status_t trifact_label_vector_normalize(trifact_label_vector_t **out_labels,
                                                const trifact_label_vector_t *labels);

trifact_status_t trifact_label_vectors_equivalent(const trifact_label_vector_t *left,
                                                  const trifact_label_vector_t *right,
                                                  int *out_equivalent);

#endif
