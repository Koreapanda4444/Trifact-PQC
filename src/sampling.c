#include "memory.h"
#include "secure.h"
#include "trifact/rng.h"

#include <string.h>

static int permutation_size(size_t count, size_t *out_size) {
    if (count > SIZE_MAX / sizeof(trifact_vertex_t)) {
        return 0;
    }
    *out_size = count * sizeof(trifact_vertex_t);
    return 1;
}

trifact_status_t trifact_uniform_u32(const trifact_random_source_t *source, uint64_t bound,
                                     uint32_t max_draws, uint32_t *out_value) {
    unsigned char bytes[8] = {0U};
    uint32_t draw = 0U;
    uint64_t tail = 0U;
    trifact_status_t status = TRIFACT_STATUS_SAMPLING_EXHAUSTED;

    if (source == NULL || source->read == NULL || out_value == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    if (bound == 0U || bound > (UINT64_C(1) << 32U)) {
        return TRIFACT_STATUS_INVALID_BOUND;
    }
    if (max_draws == 0U) {
        return TRIFACT_STATUS_COUNT_OUT_OF_RANGE;
    }
    tail = (UINT64_MAX % bound + UINT64_C(1)) % bound;
    for (draw = 0U; draw < max_draws; ++draw) {
        uint64_t value = 0U;
        size_t index = 0U;

        status = source->read(source->context, bytes, sizeof(bytes));
        if (status != TRIFACT_STATUS_OK) {
            trifact_secure_clear(bytes, sizeof(bytes));
            return status;
        }
        for (index = 0U; index < sizeof(bytes); ++index) {
            value = (value << 8U) | (uint64_t)bytes[index];
        }
        if (value <= UINT64_MAX - tail) {
            *out_value = (uint32_t)(value % bound);
            trifact_secure_clear(bytes, sizeof(bytes));
            return TRIFACT_STATUS_OK;
        }
    }
    trifact_secure_clear(bytes, sizeof(bytes));
    return TRIFACT_STATUS_SAMPLING_EXHAUSTED;
}

trifact_status_t trifact_random_permutation(const trifact_random_source_t *source,
                                            trifact_vertex_t vertex_count, uint32_t max_draws,
                                            trifact_vertex_t *output) {
    trifact_vertex_t *temporary = NULL;
    trifact_vertex_t index = 0U;
    size_t byte_length = 0U;
    trifact_status_t status = TRIFACT_STATUS_OK;

    if (source == NULL || source->read == NULL || (output == NULL && vertex_count != 0U)) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    if (max_draws == 0U) {
        return TRIFACT_STATUS_COUNT_OUT_OF_RANGE;
    }
    if (!permutation_size((size_t)vertex_count, &byte_length)) {
        return TRIFACT_STATUS_SIZE_OVERFLOW;
    }
    if (vertex_count == 0U) {
        return TRIFACT_STATUS_OK;
    }
    temporary = trifact_memory_allocate(byte_length);
    if (temporary == NULL) {
        return TRIFACT_STATUS_ALLOCATION_FAILURE;
    }
    for (index = 0U; index < vertex_count; ++index) {
        temporary[index] = index;
    }
    for (index = vertex_count; index > 1U; --index) {
        uint32_t selected = 0U;
        trifact_vertex_t saved = 0U;

        status = trifact_uniform_u32(source, (uint64_t)index, max_draws, &selected);
        if (status != TRIFACT_STATUS_OK) {
            break;
        }
        saved = temporary[index - 1U];
        temporary[index - 1U] = temporary[selected];
        temporary[selected] = saved;
    }
    if (status == TRIFACT_STATUS_OK) {
        memcpy(output, temporary, byte_length);
    }
    trifact_secure_clear(temporary, byte_length);
    trifact_memory_free(temporary);
    return status;
}
