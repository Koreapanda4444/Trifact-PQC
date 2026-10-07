#include "research_vectors.h"
#include "trifact/research.h"

#include <stdlib.h>
#include <string.h>

static int equals_hex(const unsigned char *bytes, size_t length, const char *hex) {
    const char *digits = "0123456789abcdef";
    size_t index = 0U;

    if (strlen(hex) != 2U * length) {
        return 0;
    }
    for (index = 0U; index < length; ++index) {
        if (hex[2U * index] != digits[bytes[index] >> 4U] ||
            hex[2U * index + 1U] != digits[bytes[index] & 15U]) {
            return 0;
        }
    }
    return 1;
}

static int replay(const research_kat_t *kat) {
    unsigned char seed[32] = {0U};
    unsigned char first[273] = {0U};
    unsigned char second[273] = {0U};
    trifact_research_stream_t *left = NULL;
    trifact_research_stream_t *right = NULL;
    trifact_random_source_t source = {NULL, NULL};
    size_t position = 0U;
    size_t index = 0U;
    int valid = 1;

    for (index = 0U; index < sizeof(seed); ++index) {
        seed[index] = (unsigned char)index;
    }
    if (trifact_research_stream_create(&left, kat->parameter_id, seed, kat->attempt) !=
            TRIFACT_STATUS_OK ||
        trifact_research_stream_create(&right, kat->parameter_id, seed, kat->attempt) !=
            TRIFACT_STATUS_OK) {
        trifact_research_stream_destroy(left);
        trifact_research_stream_destroy(right);
        return 0;
    }
    memset(seed, 0xff, sizeof(seed));
    source = trifact_research_source(left);
    if (trifact_research_stream_read(left, NULL, 0U) != TRIFACT_STATUS_OK ||
        trifact_research_stream_read(left, NULL, 1U) != TRIFACT_STATUS_NULL_ARGUMENT) {
        valid = 0;
    }
    while (position < sizeof(first)) {
        size_t length = position % 17U + 1U;

        if (length > sizeof(first) - position) {
            length = sizeof(first) - position;
        }
        if (source.read(source.context, first + position, length) != TRIFACT_STATUS_OK) {
            valid = 0;
            break;
        }
        position += length;
    }
    if (trifact_research_stream_read(right, second, sizeof(second)) != TRIFACT_STATUS_OK ||
        memcmp(first, second, sizeof(first)) != 0 ||
        !equals_hex(first, sizeof(first), kat->output_hex)) {
        valid = 0;
    }
    trifact_research_stream_destroy(left);
    trifact_research_stream_destroy(right);
    return valid;
}

int main(void) {
    trifact_research_stream_t *stream = NULL;
    trifact_random_source_t empty = trifact_research_source(NULL);
    unsigned char seed[32] = {0U};
    size_t index = 0U;

    for (index = 0U; index < sizeof(research_kats) / sizeof(research_kats[0]); ++index) {
        if (!replay(&research_kats[index])) {
            return EXIT_FAILURE;
        }
    }
    if (trifact_research_stream_create(NULL, "toy", seed, 0U) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_research_stream_create(&stream, NULL, seed, 0U) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_research_stream_create(&stream, "toy", NULL, 0U) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_research_stream_create(&stream, "TOY", seed, 0U) !=
            TRIFACT_STATUS_INVALID_PARAMETER_ID ||
        trifact_research_stream_create(&stream, "toy ", seed, 0U) !=
            TRIFACT_STATUS_INVALID_PARAMETER_ID ||
        trifact_research_stream_create(&stream, "unknown", seed, 0U) !=
            TRIFACT_STATUS_INVALID_PARAMETER_ID ||
        stream != NULL ||
        trifact_research_stream_read(NULL, seed, 1U) != TRIFACT_STATUS_NULL_ARGUMENT ||
        empty.read != NULL || empty.context != NULL) {
        return EXIT_FAILURE;
    }
    trifact_research_stream_destroy(NULL);
    return EXIT_SUCCESS;
}
