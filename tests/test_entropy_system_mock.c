#include "trifact/rng.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>

static size_t calls = 0U;
static size_t requested_sizes[16] = {0U};
static int mode = 0;
static int invalid_flags = 0;

ssize_t trifact_test_getrandom(void *buffer, size_t requested, unsigned int flags) {
    size_t count = requested;

    if (calls >= 16U || requested > 256U || requested == 0U || flags != 0U) {
        invalid_flags = 1;
        errno = EINVAL;
        return -1;
    }
    requested_sizes[calls++] = requested;
    if (mode == 1 && calls <= 2U) {
        errno = EINTR;
        return -1;
    }
    if (mode == 2 && calls == 2U) {
        errno = EIO;
        return -1;
    }
    if (mode == 3) {
        return 0;
    }
    if (mode == 4) {
        return (ssize_t)(requested + 1U);
    }
    if (mode == 5) {
        errno = EINTR;
        return -1;
    }
    if (mode == 6 && requested > 100U) {
        count = 100U;
    }
    memset(buffer, (int)calls, count);
    return (ssize_t)count;
}

static int check_mode(int selected) {
    trifact_entropy_provider_t *provider = NULL;
    unsigned char bytes[513] = {0U};
    unsigned char sentinel[513] = {0U};
    const int succeeds = selected == 0 || selected == 1 || selected == 6;
    trifact_status_t status = TRIFACT_STATUS_OK;
    size_t index = 0U;
    int valid = 1;

    calls = 0U;
    mode = selected;
    memset(bytes, 0xa5, sizeof(bytes));
    memcpy(sentinel, bytes, sizeof(bytes));
    if (trifact_entropy_provider_create_system(&provider, 2U) != TRIFACT_STATUS_OK ||
        trifact_entropy_read_exact(provider, NULL, 0U) != TRIFACT_STATUS_OK || calls != 0U) {
        trifact_entropy_provider_destroy(provider);
        return 0;
    }
    status = trifact_entropy_read_exact(provider, bytes, sizeof(bytes));
    if (succeeds) {
        size_t position = 0U;

        if (status != TRIFACT_STATUS_OK || calls != (selected == 1   ? 5U
                                                     : selected == 6 ? 6U
                                                                     : 3U)) {
            valid = 0;
        }
        for (index = selected == 1 ? 2U : 0U; index < calls; ++index) {
            size_t count = requested_sizes[index];
            size_t offset = 0U;

            if (selected == 6 && count > 100U) {
                count = 100U;
            }
            for (offset = 0U; offset < count && position < sizeof(bytes); ++offset) {
                if (bytes[position++] != (unsigned char)(index + 1U)) {
                    valid = 0;
                }
            }
        }
        if (position != sizeof(bytes)) {
            valid = 0;
        }
    } else {
        const size_t before = calls;

        if (status != TRIFACT_STATUS_RANDOMNESS_FAILURE ||
            memcmp(bytes, sentinel, sizeof(bytes)) != 0 || (selected == 5 && calls != 3U) ||
            trifact_entropy_read_exact(provider, bytes, 1U) != TRIFACT_STATUS_RANDOMNESS_FAILURE ||
            calls != before) {
            valid = 0;
        }
    }
    trifact_entropy_provider_destroy(provider);
    return valid;
}

int main(void) {
    int selected = 0;

    for (selected = 0; selected <= 6; ++selected) {
        if (!check_mode(selected)) {
            return EXIT_FAILURE;
        }
    }
    return invalid_flags == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
