#include "trifact/rng.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const uint64_t *values;
    size_t count;
    size_t position;
    size_t failure_call;
} draw_script_t;

static trifact_status_t scripted_draw(void *context, unsigned char *output, size_t length) {
    draw_script_t *script = context;
    size_t index = 0U;
    uint64_t value = 0U;

    if (length != 8U) {
        return TRIFACT_STATUS_COUNT_OUT_OF_RANGE;
    }
    if (script->position >= script->count) {
        return TRIFACT_STATUS_RANDOMNESS_FAILURE;
    }
    ++script->position;
    if (script->position == script->failure_call) {
        return TRIFACT_STATUS_RANDOMNESS_FAILURE;
    }
    value = script->values[script->position - 1U];
    for (index = 0U; index < 8U; ++index) {
        output[index] = (unsigned char)(value >> (unsigned int)(56U - 8U * index));
    }
    return TRIFACT_STATUS_OK;
}

static uint64_t interval_remainder(uint64_t bound) {
    uint64_t remainder = UINT64_C(1) % bound;
    unsigned int bit = 0U;

    for (bit = 0U; bit < 64U; ++bit) {
        remainder = (2U * remainder) % bound;
    }
    return remainder;
}

static int rejection_boundaries(void) {
    const uint64_t bounds[] = {1U, 2U, 3U, 5U, 6U, 7U, 12U, UINT32_MAX, UINT64_C(1) << 32U};
    size_t index = 0U;

    for (index = 0U; index < sizeof(bounds) / sizeof(bounds[0]); ++index) {
        const uint64_t bound = bounds[index];
        const uint64_t remainder = interval_remainder(bound);
        const uint64_t boundary = UINT64_MAX - remainder;
        uint64_t values[] = {boundary, UINT64_MAX, UINT64_MAX, bound - 1U, 0U};
        draw_script_t script = {values, 5U, 0U, 0U};
        const trifact_random_source_t source = {scripted_draw, &script};
        uint32_t result = 0xa5a5a5a5U;

        if (trifact_uniform_u32(&source, bound, 1U, &result) != TRIFACT_STATUS_OK ||
            (uint64_t)result != boundary % bound || script.position != 1U) {
            return 0;
        }
        if (remainder != 0U) {
            if (trifact_uniform_u32(&source, bound, 3U, &result) != TRIFACT_STATUS_OK ||
                (uint64_t)result != bound - 1U || script.position != 4U) {
                return 0;
            }
            script.position = 1U;
            result = 0xa5a5a5a5U;
            if (trifact_uniform_u32(&source, bound, 2U, &result) !=
                    TRIFACT_STATUS_SAMPLING_EXHAUSTED ||
                result != 0xa5a5a5a5U || script.position != 3U ||
                trifact_uniform_u32(&source, bound, 1U, &result) != TRIFACT_STATUS_OK ||
                (uint64_t)result != bound - 1U || script.position != 4U) {
                return 0;
            }
            script.position = 1U;
            script.failure_call = 3U;
            result = 0xa5a5a5a5U;
            if (trifact_uniform_u32(&source, bound, 3U, &result) !=
                    TRIFACT_STATUS_RANDOMNESS_FAILURE ||
                result != 0xa5a5a5a5U || script.position != 3U) {
                return 0;
            }
            script.failure_call = 0U;
        } else if (trifact_uniform_u32(&source, bound, 1U, &result) != TRIFACT_STATUS_OK ||
                   (uint64_t)result != UINT64_MAX % bound || script.position != 2U) {
            return 0;
        }
        if (remainder > 0U) {
            uint64_t rejected = 0U;

            for (rejected = 0U; rejected < remainder; ++rejected) {
                values[0] = UINT64_MAX - rejected;
                script.position = 0U;
                result = 0xa5a5a5a5U;
                if (trifact_uniform_u32(&source, bound, 1U, &result) !=
                        TRIFACT_STATUS_SAMPLING_EXHAUSTED ||
                    result != 0xa5a5a5a5U || script.position != 1U) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

static int uniform_oracle(void) {
    uint64_t bound = 0U;

    for (bound = 1U; bound <= 16U; ++bound) {
        unsigned int counts[16] = {0U};
        uint64_t candidate = 0U;
        draw_script_t script = {&candidate, 1U, 0U, 0U};
        const trifact_random_source_t source = {scripted_draw, &script};
        uint32_t result = 0U;
        size_t index = 0U;

        for (candidate = 0U; candidate < 4U * bound; ++candidate) {
            script.position = 0U;
            if (trifact_uniform_u32(&source, bound, 1U, &result) != TRIFACT_STATUS_OK ||
                (uint64_t)result != candidate % bound || (uint64_t)result >= bound) {
                return 0;
            }
            ++counts[result];
        }
        for (index = 0U; index < (size_t)bound; ++index) {
            if (counts[index] != 4U) {
                return 0;
            }
        }
    }
    return 1;
}

static int permutation_oracle(void) {
    unsigned int seen[256] = {0U};
    uint64_t choices[3] = {0U};
    uint32_t a = 0U;
    uint32_t b = 0U;
    uint32_t c = 0U;
    uint32_t d = 0U;

    for (choices[0] = 0U; choices[0] < 4U; ++choices[0]) {
        for (choices[1] = 0U; choices[1] < 3U; ++choices[1]) {
            for (choices[2] = 0U; choices[2] < 2U; ++choices[2]) {
                draw_script_t script = {choices, 3U, 0U, 0U};
                const trifact_random_source_t source = {scripted_draw, &script};
                trifact_vertex_t permutation[4] = {0U};
                unsigned int mask = 0U;
                unsigned int key = 0U;
                size_t index = 0U;

                if (trifact_random_permutation(&source, 4U, 1U, permutation) != TRIFACT_STATUS_OK ||
                    script.position != 3U) {
                    return 0;
                }
                for (index = 0U; index < 4U; ++index) {
                    if (permutation[index] >= 4U) {
                        return 0;
                    }
                    mask |= 1U << permutation[index];
                    key = 4U * key + permutation[index];
                }
                if (mask != 15U || ++seen[key] != 1U) {
                    return 0;
                }
            }
        }
    }
    for (a = 0U; a < 4U; ++a) {
        for (b = 0U; b < 4U; ++b) {
            for (c = 0U; c < 4U; ++c) {
                for (d = 0U; d < 4U; ++d) {
                    const unsigned int key = 64U * a + 16U * b + 4U * c + d;
                    const unsigned int expected =
                        a != b && a != c && a != d && b != c && b != d && c != d ? 1U : 0U;

                    if (seen[key] != expected) {
                        return 0;
                    }
                }
            }
        }
    }
    return 1;
}

static int error_boundaries(void) {
    const uint64_t values[] = {UINT64_C(0x0102030405060708), UINT64_MAX, UINT64_MAX, 7U};
    draw_script_t script = {values, 4U, 0U, 0U};
    const trifact_random_source_t source = {scripted_draw, &script};
    const trifact_random_source_t empty = {NULL, NULL};
    uint32_t value = 0xa5a5a5a5U;
    trifact_vertex_t permutation[] = {99U, 98U, 97U, 96U};
    const trifact_vertex_t sentinel[] = {99U, 98U, 97U, 96U};

    if (trifact_uniform_u32(&source, 0U, 1U, &value) != TRIFACT_STATUS_INVALID_BOUND ||
        trifact_uniform_u32(&source, (UINT64_C(1) << 32U) + 1U, 1U, &value) !=
            TRIFACT_STATUS_INVALID_BOUND ||
        trifact_uniform_u32(&source, 3U, 0U, &value) != TRIFACT_STATUS_COUNT_OUT_OF_RANGE ||
        trifact_uniform_u32(NULL, 3U, 1U, &value) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_uniform_u32(&empty, 3U, 1U, &value) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_uniform_u32(&source, 3U, 1U, NULL) != TRIFACT_STATUS_NULL_ARGUMENT ||
        value != 0xa5a5a5a5U || script.position != 0U ||
        trifact_uniform_u32(&source, UINT64_C(1) << 32U, 1U, &value) != TRIFACT_STATUS_OK ||
        value != UINT32_C(0x05060708)) {
        return 0;
    }
    script.position = 0U;
    if (trifact_random_permutation(NULL, 4U, 1U, permutation) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_random_permutation(&empty, 4U, 1U, permutation) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_random_permutation(&source, 4U, 0U, permutation) !=
            TRIFACT_STATUS_COUNT_OUT_OF_RANGE ||
        trifact_random_permutation(&source, 4U, 1U, NULL) != TRIFACT_STATUS_NULL_ARGUMENT ||
        script.position != 0U || memcmp(permutation, sentinel, sizeof(permutation)) != 0 ||
        trifact_random_permutation(&source, 4U, 2U, permutation) !=
            TRIFACT_STATUS_SAMPLING_EXHAUSTED ||
        script.position != 3U || memcmp(permutation, sentinel, sizeof(permutation)) != 0 ||
        trifact_uniform_u32(&source, 2U, 1U, &value) != TRIFACT_STATUS_OK || value != 1U ||
        script.position != 4U) {
        return 0;
    }
    script.position = 0U;
    script.failure_call = 2U;
    if (trifact_random_permutation(&source, 4U, 2U, permutation) !=
            TRIFACT_STATUS_RANDOMNESS_FAILURE ||
        script.position != 2U || memcmp(permutation, sentinel, sizeof(permutation)) != 0) {
        return 0;
    }
#if SIZE_MAX <= UINT32_MAX
    if (trifact_random_permutation(&source, UINT32_MAX, 1U, permutation) !=
        TRIFACT_STATUS_SIZE_OVERFLOW) {
        return 0;
    }
#endif
    return 1;
}

typedef struct {
    const int *events;
    size_t count;
    size_t position;
    unsigned char next;
} entropy_script_t;

static trifact_status_t entropy_event(void *context, unsigned char *buffer, size_t requested,
                                      size_t *received) {
    entropy_script_t *script = context;
    int event = 0;

    *received = 0U;
    if (requested == 0U || script->position == script->count) {
        return TRIFACT_STATUS_RANDOMNESS_FAILURE;
    }
    event = script->events[script->position++];
    if (event < 0) {
        if (event == -2) {
            buffer[0] = script->next;
            *received = 1U;
        }
        return TRIFACT_STATUS_INTERRUPTED;
    }
    if (event == 0) {
        return TRIFACT_STATUS_OK;
    }
    buffer[0] = script->next++;
    *received = 1U;
    return TRIFACT_STATUS_OK;
}

static int entropy_boundaries(void) {
    const int events[] = {-1, 1, -1, 1, -1, 1, -1, 1};
    entropy_script_t script = {events, 8U, 0U, 0U};
    trifact_entropy_provider_t *provider = NULL;
    unsigned char bytes[] = {0xa5U, 0xa5U, 0xa5U};
    int valid = 1;

    if (trifact_entropy_provider_create(&provider, entropy_event, &script, 2U) !=
        TRIFACT_STATUS_OK) {
        return 0;
    }
    if (trifact_entropy_read_exact(provider, bytes, 2U) != TRIFACT_STATUS_OK || bytes[0] != 0U ||
        bytes[1] != 1U || script.position != 4U ||
        trifact_entropy_read_exact(provider, bytes, 2U) != TRIFACT_STATUS_OK || bytes[0] != 2U ||
        bytes[1] != 3U || script.position != 8U) {
        valid = 0;
    }
    trifact_entropy_provider_destroy(provider);
    provider = NULL;
    script.position = 0U;
    script.next = 0U;
    memset(bytes, 0xa5, sizeof(bytes));
    if (trifact_entropy_provider_create(&provider, entropy_event, &script, 2U) !=
        TRIFACT_STATUS_OK) {
        return 0;
    }
    if (trifact_entropy_read_exact(provider, bytes, 3U) != TRIFACT_STATUS_RANDOMNESS_FAILURE ||
        script.position != 5U || bytes[0] != 0xa5U || bytes[1] != 0xa5U || bytes[2] != 0xa5U ||
        trifact_entropy_read_exact(provider, bytes, 1U) != TRIFACT_STATUS_RANDOMNESS_FAILURE ||
        script.position != 5U) {
        valid = 0;
    }
    trifact_entropy_provider_destroy(provider);
    provider = NULL;
    {
        const int invalid_events[] = {-2};

        script = (entropy_script_t){invalid_events, 1U, 0U, 0U};
        if (trifact_entropy_provider_create(&provider, entropy_event, &script, 2U) !=
            TRIFACT_STATUS_OK) {
            return 0;
        }
        if (trifact_entropy_read_exact(provider, bytes, 1U) != TRIFACT_STATUS_RANDOMNESS_FAILURE ||
            bytes[0] != 0xa5U || script.position != 1U) {
            valid = 0;
        }
        trifact_entropy_provider_destroy(provider);
        provider = NULL;
    }
    script = (entropy_script_t){events, 8U, 0U, 0U};
    if (trifact_entropy_provider_create(&provider, entropy_event, &script, 0U) !=
        TRIFACT_STATUS_OK) {
        return 0;
    }
    if (trifact_entropy_read_exact(provider, bytes, 1U) != TRIFACT_STATUS_RANDOMNESS_FAILURE ||
        bytes[0] != 0xa5U || script.position != 1U) {
        valid = 0;
    }
    trifact_entropy_provider_destroy(provider);
    return valid;
}

int main(void) {
    if (!rejection_boundaries() || !uniform_oracle() || !permutation_oracle() ||
        !error_boundaries() || !entropy_boundaries()) {
        fputs("Entropy or sampling boundary check failed\n", stderr);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
