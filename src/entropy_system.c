#include "trifact/rng.h"

#if defined(_WIN32)
#include <windows.h>

#include <bcrypt.h>
#elif defined(__linux__)
#include <errno.h>
#include <sys/random.h>
#endif

#if defined(_WIN32) || defined(__linux__)
static trifact_status_t system_read(void *context, unsigned char *buffer, size_t requested,
                                    size_t *received) {
    const size_t chunk = requested < 256U ? requested : 256U;

    (void)context;
    *received = 0U;
#if defined(_WIN32)
    if (BCryptGenRandom(NULL, buffer, (ULONG)chunk, BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) {
        return TRIFACT_STATUS_RANDOMNESS_FAILURE;
    }
    *received = chunk;
#else
    {
        const ssize_t result = getrandom(buffer, chunk, 0U);

        if (result < 0) {
            return errno == EINTR ? TRIFACT_STATUS_INTERRUPTED : TRIFACT_STATUS_RANDOMNESS_FAILURE;
        }
        *received = (size_t)result;
    }
#endif
    return TRIFACT_STATUS_OK;
}
#endif

trifact_status_t trifact_entropy_provider_create_system(trifact_entropy_provider_t **out_provider,
                                                        uint32_t interruption_limit) {
    if (out_provider == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    *out_provider = NULL;
#if defined(_WIN32) || defined(__linux__)
    return trifact_entropy_provider_create(out_provider, system_read, NULL, interruption_limit);
#else
    (void)interruption_limit;
    return TRIFACT_STATUS_PLATFORM_UNAVAILABLE;
#endif
}
