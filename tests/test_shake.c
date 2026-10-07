#include "trifact/shake.h"

#include <stdlib.h>
#include <string.h>

int main(void) {
    const unsigned char empty_expected[] = {0x46U, 0xb9U, 0xddU, 0x2bU, 0x0bU, 0xa8U, 0x8dU, 0x13U};
    unsigned char input[300] = {0U};
    unsigned char first[300] = {0U};
    unsigned char second[300] = {0U};
    trifact_shake256_t *context = NULL;
    size_t index = 0U;
    int failed = 0;

    for (index = 0U; index < sizeof(input); ++index) {
        input[index] = (unsigned char)index;
    }
    if (trifact_shake256(NULL, 0U, first, sizeof(empty_expected)) != TRIFACT_STATUS_OK ||
        memcmp(first, empty_expected, sizeof(empty_expected)) != 0 ||
        trifact_shake256_create(&context) != TRIFACT_STATUS_OK) {
        return EXIT_FAILURE;
    }
    first[0] = 0xaaU;
    if (trifact_shake256_squeeze(context, first, 1U) != TRIFACT_STATUS_INVALID_STATE ||
        first[0] != 0xaaU || trifact_shake256_absorb(context, NULL, 0U) != TRIFACT_STATUS_OK ||
        trifact_shake256_absorb(context, NULL, 1U) != TRIFACT_STATUS_NULL_ARGUMENT) {
        failed = 1;
    }
    for (index = 0U; index < sizeof(input); ++index) {
        if (trifact_shake256_absorb(context, &input[index], 1U) != TRIFACT_STATUS_OK) {
            failed = 1;
        }
    }
    if (trifact_shake256_finalize(context) != TRIFACT_STATUS_OK ||
        trifact_shake256_finalize(context) != TRIFACT_STATUS_INVALID_STATE ||
        trifact_shake256_absorb(context, NULL, 0U) != TRIFACT_STATUS_INVALID_STATE ||
        trifact_shake256_squeeze(context, NULL, 0U) != TRIFACT_STATUS_OK ||
        trifact_shake256_squeeze(context, NULL, 1U) != TRIFACT_STATUS_NULL_ARGUMENT) {
        failed = 1;
    }
    for (index = 0U; index < sizeof(second); ++index) {
        if (trifact_shake256_squeeze(context, &second[index], 1U) != TRIFACT_STATUS_OK) {
            failed = 1;
        }
    }
    if (trifact_shake256(input, sizeof(input), first, sizeof(first)) != TRIFACT_STATUS_OK ||
        memcmp(first, second, sizeof(first)) != 0 ||
        trifact_shake256(NULL, 1U, first, 1U) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_shake256(input, 1U, NULL, 1U) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_shake256(NULL, 0U, NULL, 0U) != TRIFACT_STATUS_OK ||
        trifact_shake256_create(NULL) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_shake256_finalize(NULL) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_shake256_squeeze(NULL, NULL, 0U) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_shake256_absorb(NULL, NULL, 0U) != TRIFACT_STATUS_NULL_ARGUMENT) {
        failed = 1;
    }
    trifact_shake256_destroy(context);
    context = NULL;
    trifact_shake256_destroy(context);
    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
