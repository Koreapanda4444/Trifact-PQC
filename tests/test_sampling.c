#include "trifact/research.h"

#include <stdlib.h>
#include <string.h>

static trifact_status_t sequential(void *context, unsigned char *output, size_t length) {
    uint64_t *next = context;
    size_t index = 0U;

    if (length != 8U) {
        return TRIFACT_STATUS_COUNT_OUT_OF_RANGE;
    }
    for (index = 0U; index < length; ++index) {
        output[index] = (unsigned char)(*next >> (unsigned int)(56U - 8U * index));
    }
    ++*next;
    return TRIFACT_STATUS_OK;
}

static int permutation_replay(void) {
    unsigned char seed[32] = {0U};
    trifact_vertex_t first[12] = {0U};
    trifact_vertex_t second[12] = {0U};
    unsigned int seen[12] = {0U};
    trifact_research_stream_t *left = NULL;
    trifact_research_stream_t *right = NULL;
    trifact_random_source_t left_source = {NULL, NULL};
    trifact_random_source_t right_source = {NULL, NULL};
    size_t index = 0U;
    int valid = 1;

    for (index = 0U; index < sizeof(seed); ++index) {
        seed[index] = (unsigned char)index;
    }
    if (trifact_research_stream_create(&left, "toy", seed, 0U) != TRIFACT_STATUS_OK ||
        trifact_research_stream_create(&right, "toy", seed, 0U) != TRIFACT_STATUS_OK) {
        trifact_research_stream_destroy(left);
        trifact_research_stream_destroy(right);
        return 0;
    }
    left_source = trifact_research_source(left);
    right_source = trifact_research_source(right);
    if (trifact_random_permutation(&left_source, 12U, 4U, first) != TRIFACT_STATUS_OK ||
        trifact_random_permutation(&right_source, 12U, 4U, second) != TRIFACT_STATUS_OK ||
        memcmp(first, second, sizeof(first)) != 0) {
        valid = 0;
    }
    for (index = 0U; index < 12U; ++index) {
        if (first[index] >= 12U || seen[first[index]] != 0U) {
            valid = 0;
            break;
        }
        seen[first[index]] = 1U;
    }
    trifact_research_stream_destroy(left);
    trifact_research_stream_destroy(right);
    return valid;
}

int main(void) {
    uint64_t next = 0U;
    const trifact_random_source_t source = {sequential, &next};
    unsigned int counts[12] = {0U};
    uint32_t value = 0U;
    unsigned int index = 0U;

    for (index = 0U; index < 120U; ++index) {
        if (trifact_uniform_u32(&source, 12U, 1U, &value) != TRIFACT_STATUS_OK || value >= 12U) {
            return EXIT_FAILURE;
        }
        ++counts[value];
    }
    for (index = 0U; index < 12U; ++index) {
        if (counts[index] != 10U) {
            return EXIT_FAILURE;
        }
    }
    if (next != 120U || trifact_random_permutation(&source, 0U, 1U, NULL) != TRIFACT_STATUS_OK ||
        trifact_random_permutation(&source, 1U, 1U, &value) != TRIFACT_STATUS_OK || value != 0U ||
        next != 120U || !permutation_replay()) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
