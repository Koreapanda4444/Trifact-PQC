#include "trifact/rng.h"

#include <stdlib.h>
#include <string.h>

int main(void) {
    trifact_entropy_provider_t *provider = NULL;
    trifact_status_t status = TRIFACT_STATUS_OK;
    unsigned char output[515] = {0U};
    int failed = 0;

    if (trifact_entropy_provider_create_system(NULL, 4U) != TRIFACT_STATUS_NULL_ARGUMENT) {
        return EXIT_FAILURE;
    }
    status = trifact_entropy_provider_create_system(&provider, 4U);
#if defined(_WIN32) || defined(__linux__)
    if (status != TRIFACT_STATUS_OK) {
        return EXIT_FAILURE;
    }
    memset(output, 0xa5, sizeof(output));
    if (trifact_entropy_read_exact(provider, NULL, 0U) != TRIFACT_STATUS_OK ||
        trifact_entropy_read_exact(provider, output + 1U, 513U) != TRIFACT_STATUS_OK ||
        output[0] != 0xa5U || output[514] != 0xa5U ||
        trifact_entropy_read_exact(provider, output + 1U, 32U) != TRIFACT_STATUS_OK ||
        output[0] != 0xa5U || output[514] != 0xa5U) {
        failed = 1;
    }
#else
    (void)output;
    if (status != TRIFACT_STATUS_PLATFORM_UNAVAILABLE || provider != NULL) {
        failed = 1;
    }
#endif
    trifact_entropy_provider_destroy(provider);
    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
