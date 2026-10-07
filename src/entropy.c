#include "memory.h"
#include "secure.h"
#include "trifact/rng.h"

#include <string.h>

struct trifact_entropy_provider {
    trifact_entropy_callback_t callback;
    void *context;
    uint32_t interruption_limit;
    int failed;
};

trifact_status_t trifact_entropy_provider_create(trifact_entropy_provider_t **out_provider,
                                                 trifact_entropy_callback_t callback, void *context,
                                                 uint32_t interruption_limit) {
    trifact_entropy_provider_t *provider = NULL;

    if (out_provider == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    *out_provider = NULL;
    if (callback == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    provider = trifact_memory_allocate_zero(1U, sizeof(*provider));
    if (provider == NULL) {
        return TRIFACT_STATUS_ALLOCATION_FAILURE;
    }
    provider->callback = callback;
    provider->context = context;
    provider->interruption_limit = interruption_limit;
    *out_provider = provider;
    return TRIFACT_STATUS_OK;
}

trifact_status_t trifact_entropy_read_exact(trifact_entropy_provider_t *provider,
                                            unsigned char *output, size_t length) {
    unsigned char *temporary = NULL;
    size_t position = 0U;
    uint32_t interruptions = 0U;
    trifact_status_t status = TRIFACT_STATUS_OK;

    if (provider == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    if (provider->failed != 0) {
        return TRIFACT_STATUS_RANDOMNESS_FAILURE;
    }
    if (output == NULL && length != 0U) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    if (length == 0U) {
        return TRIFACT_STATUS_OK;
    }
    temporary = trifact_memory_allocate(length);
    if (temporary == NULL) {
        return TRIFACT_STATUS_ALLOCATION_FAILURE;
    }
    while (position < length) {
        size_t received = 0U;
        const size_t remaining = length - position;

        status = provider->callback(provider->context, temporary + position, remaining, &received);
        if (status == TRIFACT_STATUS_INTERRUPTED && received == 0U &&
            interruptions < provider->interruption_limit) {
            ++interruptions;
            continue;
        }
        if (status != TRIFACT_STATUS_OK || received == 0U || received > remaining) {
            provider->failed = 1;
            status = TRIFACT_STATUS_RANDOMNESS_FAILURE;
            break;
        }
        position += received;
    }
    if (status == TRIFACT_STATUS_OK) {
        memcpy(output, temporary, length);
    }
    trifact_secure_clear(temporary, length);
    trifact_memory_free(temporary);
    return status;
}

static trifact_status_t entropy_read(void *context, unsigned char *output, size_t length) {
    return trifact_entropy_read_exact(context, output, length);
}

trifact_random_source_t trifact_entropy_source(trifact_entropy_provider_t *provider) {
    const trifact_random_source_t source = {provider == NULL ? NULL : entropy_read, provider};

    return source;
}

void trifact_entropy_provider_destroy(trifact_entropy_provider_t *provider) {
    if (provider != NULL) {
        trifact_secure_clear(provider, sizeof(*provider));
        trifact_memory_free(provider);
    }
}
