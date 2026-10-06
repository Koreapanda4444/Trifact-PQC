#include "trifact/witness.h"
#include "memory.h"

trifact_status_t trifact_label_vector_normalize(trifact_label_vector_t **out_labels,
                                                const trifact_label_vector_t *labels) {
    trifact_factor_label_t *normalized = NULL;
    const trifact_factor_label_t *data = NULL;
    trifact_factor_label_t next_label = 0U;
    size_t count = 0U;
    size_t index = 0U;
    trifact_status_t status = TRIFACT_STATUS_OK;

    if (out_labels == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    *out_labels = NULL;
    if (labels == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    count = trifact_label_vector_count(labels);
    data = trifact_label_vector_data(labels);
    if (count > SIZE_MAX / sizeof(*normalized)) {
        return TRIFACT_STATUS_SIZE_OVERFLOW;
    }
    if (count == 0U) {
        return trifact_label_vector_create(out_labels, NULL, 0U);
    }
    normalized = trifact_memory_allocate(count * sizeof(*normalized));
    if (normalized == NULL) {
        return TRIFACT_STATUS_ALLOCATION_FAILURE;
    }
    for (index = 0U; index < count; ++index) {
        size_t previous = 0U;

        for (previous = 0U; previous < index; ++previous) {
            if (data[previous] == data[index]) {
                break;
            }
        }
        if (previous < index) {
            normalized[index] = normalized[previous];
        } else {
            normalized[index] = next_label;
            ++next_label;
        }
    }
    status = trifact_label_vector_create(out_labels, normalized, count);
    trifact_memory_free(normalized);
    return status;
}

trifact_status_t trifact_label_vectors_equivalent(const trifact_label_vector_t *left,
                                                  const trifact_label_vector_t *right,
                                                  int *out_equivalent) {
    const trifact_factor_label_t *left_data = NULL;
    const trifact_factor_label_t *right_data = NULL;
    size_t count = 0U;
    size_t index = 0U;

    if (left == NULL || right == NULL || out_equivalent == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    count = trifact_label_vector_count(left);
    if (count != trifact_label_vector_count(right)) {
        *out_equivalent = 0;
        return TRIFACT_STATUS_OK;
    }
    left_data = trifact_label_vector_data(left);
    right_data = trifact_label_vector_data(right);
    for (index = 0U; index < count; ++index) {
        size_t previous = 0U;

        for (previous = 0U; previous < index; ++previous) {
            if ((left_data[index] == left_data[previous]) !=
                (right_data[index] == right_data[previous])) {
                *out_equivalent = 0;
                return TRIFACT_STATUS_OK;
            }
        }
    }
    *out_equivalent = 1;
    return TRIFACT_STATUS_OK;
}
