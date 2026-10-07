#include "trifact/rng.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    size_t calls;
    unsigned int next;
    int mode;
} callback_state_t;

static trifact_status_t scripted(void *context, unsigned char *buffer, size_t requested,
                                 size_t *received) {
    callback_state_t *state = context;
    size_t index = 0U;
    size_t count = requested < 3U ? requested : 3U;

    ++state->calls;
    *received = 0U;
    if (state->mode == 1 && state->calls <= 2U) {
        return TRIFACT_STATUS_INTERRUPTED;
    }
    if (state->mode == 2 && state->calls > 1U) {
        return TRIFACT_STATUS_INVALID_STATE;
    }
    if (state->mode == 3) {
        return TRIFACT_STATUS_OK;
    }
    if (state->mode == 4) {
        *received = requested + 1U;
        return TRIFACT_STATUS_OK;
    }
    if (state->mode == 5) {
        return TRIFACT_STATUS_INTERRUPTED;
    }
    for (index = 0U; index < count; ++index) {
        buffer[index] = (unsigned char)state->next;
        ++state->next;
    }
    *received = count;
    return TRIFACT_STATUS_OK;
}

static int success_case(int mode) {
    callback_state_t state = {0U, 0U, mode};
    trifact_entropy_provider_t *provider = NULL;
    trifact_random_source_t source = {NULL, NULL};
    unsigned char bytes[9] = {0U};
    size_t index = 0U;
    int valid = 1;

    if (trifact_entropy_provider_create(&provider, scripted, &state, 2U) != TRIFACT_STATUS_OK) {
        return 0;
    }
    source = trifact_entropy_source(provider);
    if (trifact_entropy_read_exact(provider, NULL, 0U) != TRIFACT_STATUS_OK || state.calls != 0U ||
        trifact_entropy_read_exact(provider, NULL, 1U) != TRIFACT_STATUS_NULL_ARGUMENT ||
        state.calls != 0U ||
        source.read(source.context, bytes, sizeof(bytes)) != TRIFACT_STATUS_OK ||
        state.calls != (mode == 1 ? 5U : 3U)) {
        valid = 0;
    }
    for (index = 0U; index < sizeof(bytes); ++index) {
        if (bytes[index] != (unsigned char)index) {
            valid = 0;
        }
    }
    if (trifact_entropy_read_exact(provider, bytes, 1U) != TRIFACT_STATUS_OK || bytes[0] != 9U) {
        valid = 0;
    }
    trifact_entropy_provider_destroy(provider);
    return valid;
}

static int failure_case(int mode) {
    callback_state_t state = {0U, 0U, mode};
    trifact_entropy_provider_t *provider = NULL;
    unsigned char bytes[9] = {0U};
    unsigned char sentinel[9] = {0U};
    size_t calls = 0U;
    int valid = 1;

    memset(bytes, 0xa5, sizeof(bytes));
    memcpy(sentinel, bytes, sizeof(bytes));
    if (trifact_entropy_provider_create(&provider, scripted, &state, 2U) != TRIFACT_STATUS_OK) {
        return 0;
    }
    if (trifact_entropy_read_exact(provider, bytes, sizeof(bytes)) !=
            TRIFACT_STATUS_RANDOMNESS_FAILURE ||
        memcmp(bytes, sentinel, sizeof(bytes)) != 0 || (mode == 5 && state.calls != 3U)) {
        valid = 0;
    }
    calls = state.calls;
    if (trifact_entropy_read_exact(provider, bytes, sizeof(bytes)) !=
            TRIFACT_STATUS_RANDOMNESS_FAILURE ||
        trifact_entropy_read_exact(provider, NULL, 0U) != TRIFACT_STATUS_RANDOMNESS_FAILURE ||
        trifact_entropy_read_exact(provider, NULL, 1U) != TRIFACT_STATUS_RANDOMNESS_FAILURE ||
        state.calls != calls || memcmp(bytes, sentinel, sizeof(bytes)) != 0) {
        valid = 0;
    }
    trifact_entropy_provider_destroy(provider);
    return valid;
}

int main(void) {
    trifact_entropy_provider_t *provider = NULL;
    trifact_random_source_t source = trifact_entropy_source(NULL);
    int mode = 0;

    if (!success_case(0) || !success_case(1) || source.read != NULL || source.context != NULL ||
        trifact_entropy_provider_create(NULL, scripted, NULL, 0U) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_entropy_provider_create(&provider, NULL, NULL, 0U) !=
            TRIFACT_STATUS_NULL_ARGUMENT ||
        provider != NULL ||
        trifact_entropy_read_exact(NULL, NULL, 0U) != TRIFACT_STATUS_NULL_ARGUMENT) {
        return EXIT_FAILURE;
    }
    for (mode = 2; mode <= 5; ++mode) {
        if (!failure_case(mode)) {
            return EXIT_FAILURE;
        }
    }
    trifact_entropy_provider_destroy(NULL);
    return EXIT_SUCCESS;
}
