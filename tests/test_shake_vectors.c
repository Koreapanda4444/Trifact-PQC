#include "shake_vectors.h"
#include "trifact/shake.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char nibble(char value) {
    if (value >= '0' && value <= '9') {
        return (unsigned char)(value - '0');
    }
    if (value >= 'a' && value <= 'f') {
        return (unsigned char)(value - 'a' + 10);
    }
    return (unsigned char)(value - 'A' + 10);
}

static void decode_hex(const char *text, unsigned char *bytes, size_t count) {
    size_t index = 0U;

    for (index = 0U; index < count; ++index) {
        bytes[index] = (unsigned char)((unsigned int)nibble(text[2U * index]) * 16U +
                                       (unsigned int)nibble(text[2U * index + 1U]));
    }
}

static int check_stream(const unsigned char *input, size_t input_length,
                        const unsigned char *expected, size_t output_length, size_t chunk) {
    trifact_shake256_t *context = NULL;
    unsigned char output[513] = {0U};
    size_t offset = 0U;
    int failed = 0;

    if (trifact_shake256_create(&context) != TRIFACT_STATUS_OK) {
        return 1;
    }
    while (offset < input_length) {
        const size_t length = input_length - offset < chunk ? input_length - offset : chunk;

        if (trifact_shake256_absorb(context, input + offset, length) != TRIFACT_STATUS_OK) {
            failed = 1;
        }
        offset += length;
    }
    if (trifact_shake256_finalize(context) != TRIFACT_STATUS_OK) {
        failed = 1;
    }
    offset = 0U;
    while (offset < output_length) {
        const size_t length = output_length - offset < chunk ? output_length - offset : chunk;

        if (trifact_shake256_squeeze(context, output + offset, length) != TRIFACT_STATUS_OK) {
            failed = 1;
        }
        offset += length;
    }
    if (memcmp(output, expected, output_length) != 0) {
        failed = 1;
    }
    trifact_shake256_destroy(context);
    return failed;
}

int main(void) {
    const size_t chunks[] = {1U, 7U, 135U, 136U, 137U};
    const size_t lengths[] = {0U, 1U, 31U, 32U, 135U, 136U, 137U, 271U, 272U, 273U, 512U, 513U};
    unsigned char input[4096] = {0U};
    unsigned char expected[513] = {0U};
    unsigned char output[513] = {0U};
    size_t vector = 0U;
    int failures = 0;

    for (vector = 0U; vector < sizeof(nist_shake_kats) / sizeof(nist_shake_kats[0]); ++vector) {
        const shake_kat_t *kat = &nist_shake_kats[vector];
        size_t chunk = 0U;

        if (kat->input_length > sizeof(input) || kat->output_length > sizeof(output) ||
            strlen(kat->message_hex) != 2U * kat->input_length ||
            strlen(kat->output_hex) != 2U * kat->output_length) {
            return EXIT_FAILURE;
        }
        decode_hex(kat->message_hex, input, kat->input_length);
        decode_hex(kat->output_hex, expected, kat->output_length);
        if (trifact_shake256(input, kat->input_length, output, kat->output_length) !=
                TRIFACT_STATUS_OK ||
            memcmp(output, expected, kat->output_length) != 0) {
            ++failures;
        }
        for (chunk = 0U; chunk < sizeof(chunks) / sizeof(chunks[0]); ++chunk) {
            failures +=
                check_stream(input, kat->input_length, expected, kat->output_length, chunks[chunk]);
        }
    }
    for (vector = 0U; vector < sizeof(boundary_shake_kats) / sizeof(boundary_shake_kats[0]);
         ++vector) {
        size_t index = 0U;
        size_t boundary = 0U;
        const shake_boundary_t *kat = &boundary_shake_kats[vector];

        for (index = 0U; index < kat->input_length; ++index) {
            input[index] = (unsigned char)((index * 17U + 3U) % 256U);
        }
        decode_hex(kat->output_hex, expected, sizeof(expected));
        for (boundary = 0U; boundary < sizeof(lengths) / sizeof(lengths[0]); ++boundary) {
            const size_t length = lengths[boundary];

            if (trifact_shake256(input, kat->input_length, output, length) != TRIFACT_STATUS_OK ||
                memcmp(output, expected, length) != 0) {
                ++failures;
            }
            failures += check_stream(input, kat->input_length, expected, length, 137U);
        }
    }
    if (failures != 0) {
        (void)fprintf(stderr, "SHAKE256 reference mismatches: %d\n", failures);
    }
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
