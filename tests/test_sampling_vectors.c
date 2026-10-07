#include "sampling_vectors.h"
#include "trifact/research.h"

#include <stdlib.h>
#include <string.h>

static int check_vector(const sampling_kat_t *kat) {
    const uint64_t bounds[] = {1U, 2U, 3U, 7U, 12U, UINT32_MAX, UINT64_C(1) << 32U};
    unsigned char seed[32] = {0U};
    trifact_vertex_t permutation[60] = {0U};
    trifact_research_stream_t *stream = NULL;
    trifact_random_source_t source = {NULL, NULL};
    size_t index = 0U;
    int valid = 1;

    for (index = 0U; index < sizeof(seed); ++index) {
        seed[index] = (unsigned char)index;
    }
    if (trifact_research_stream_create(&stream, kat->parameter_id, seed, 1U) != TRIFACT_STATUS_OK) {
        return 0;
    }
    source = trifact_research_source(stream);
    if (trifact_random_permutation(&source, kat->vertex_count, 4U, permutation) !=
            TRIFACT_STATUS_OK ||
        memcmp(permutation, kat->permutation, (size_t)kat->vertex_count * sizeof(permutation[0])) !=
            0) {
        valid = 0;
    }
    for (index = 0U; index < sizeof(bounds) / sizeof(bounds[0]); ++index) {
        uint32_t value = 0U;

        if (trifact_uniform_u32(&source, bounds[index], 4U, &value) != TRIFACT_STATUS_OK ||
            value != kat->uniform_results[index]) {
            valid = 0;
        }
    }
    trifact_research_stream_destroy(stream);
    return valid;
}

int main(void) {
    size_t index = 0U;

    for (index = 0U; index < sizeof(sampling_kats) / sizeof(sampling_kats[0]); ++index) {
        if (!check_vector(&sampling_kats[index])) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
