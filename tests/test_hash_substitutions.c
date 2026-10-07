#include "hash_vectors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const names[] = {
    "TRIFACT/pk-digest",       "TRIFACT/message-digest",  "TRIFACT/signing-context",
    "TRIFACT/keygen-attempt",  "TRIFACT/sign-salt",       "TRIFACT/proof-expand",
    "TRIFACT/view-commitment", "TRIFACT/commitment-root", "TRIFACT/challenge"};

static void write_u32(unsigned char *bytes, uint32_t value) {
    bytes[0] = (unsigned char)(value >> 24U);
    bytes[1] = (unsigned char)(value >> 16U);
    bytes[2] = (unsigned char)(value >> 8U);
    bytes[3] = (unsigned char)value;
}

static int reference(trifact_domain_t domain, int mode, const trifact_hash_field_t *fields,
                     size_t count, unsigned char *output, size_t length) {
    unsigned char frame[1024] = {0U};
    const char *prefix = mode == 0 ? "TRIFACT-HASH" : "TRIFACT-XOF";
    size_t position = strlen(prefix);
    size_t name_length = strlen(names[domain]);
    size_t index = 0U;

    memcpy(frame, prefix, position);
    write_u32(frame + position, (uint32_t)name_length);
    position += 4U;
    memcpy(frame + position, names[domain], name_length);
    position += name_length;
    write_u32(frame + position, (uint32_t)count);
    position += 4U;
    for (index = 0U; index < count; ++index) {
        if (position > sizeof(frame) - 4U || fields[index].length > sizeof(frame) - position - 4U) {
            return 0;
        }
        write_u32(frame + position, (uint32_t)fields[index].length);
        position += 4U;
        if (fields[index].length != 0U) {
            memcpy(frame + position, fields[index].data, fields[index].length);
        }
        position += fields[index].length;
    }
    if (mode == 1) {
        if (position > sizeof(frame) - 4U) {
            return 0;
        }
        write_u32(frame + position, (uint32_t)length);
        position += 4U;
    }
    return trifact_shake256(frame, position, output, length) == TRIFACT_STATUS_OK;
}

static int equal_hex(const unsigned char *bytes, size_t length, const char *hex) {
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

static trifact_status_t evaluate(const hash_kat_t *kat, const trifact_hash_field_t *fields,
                                 size_t count, unsigned char *output) {
    trifact_shake256_t *context = NULL;
    trifact_status_t status = TRIFACT_STATUS_OK;

    if (kat->mode == 0) {
        return trifact_hash32(kat->domain, fields, count, output);
    }
    if (kat->mode == 1) {
        return trifact_xof(kat->domain, fields, count, output, kat->output_length);
    }
    status = trifact_xof_stream_create(&context, kat->domain, fields, count);
    if (status == TRIFACT_STATUS_OK) {
        status = trifact_shake256_squeeze(context, output, 17U);
    }
    if (status == TRIFACT_STATUS_OK) {
        status = trifact_shake256_squeeze(context, output + 17U, kat->output_length - 17U);
    }
    trifact_shake256_destroy(context);
    return status;
}

static int check_vectors(void) {
    const unsigned char parameter[] = {3U, 't', 'o', 'y'};
    const unsigned char other_parameter[] = {5U, 's', 'm', 'a', 'l', 'l'};
    const unsigned char binary[] = {0U, 'a', 0xffU, 'b'};
    const trifact_hash_field_t fields[] = {
        {parameter, sizeof(parameter)}, {binary, sizeof(binary)}, {NULL, 0U}};
    trifact_hash_field_t changed[3] = {{0}};
    unsigned char output[64] = {0U};
    unsigned char expected[64] = {0U};
    unsigned char alternate[64] = {0U};
    size_t index = 0U;

    for (index = 0U; index < sizeof(hash_kats) / sizeof(hash_kats[0]); ++index) {
        const hash_kat_t *kat = &hash_kats[index];
        trifact_domain_t found = TRIFACT_DOMAIN_COUNT;

        if (trifact_domain_from_name(names[index], &found) != TRIFACT_STATUS_OK ||
            found != kat->domain || strcmp(trifact_domain_name(found), names[index]) != 0 ||
            evaluate(kat, fields, 3U, output) != TRIFACT_STATUS_OK ||
            !equal_hex(output, kat->output_length, kat->output_hex) ||
            !reference(kat->domain, kat->mode, fields, 3U, expected, kat->output_length) ||
            memcmp(expected, output, kat->output_length) != 0) {
            return 0;
        }
        memcpy(changed, fields, sizeof(changed));
        changed[0].data = other_parameter;
        changed[0].length = sizeof(other_parameter);
        if (evaluate(kat, changed, 3U, alternate) != TRIFACT_STATUS_OK ||
            !reference(kat->domain, kat->mode, changed, 3U, expected, kat->output_length) ||
            memcmp(expected, alternate, kat->output_length) != 0 ||
            memcmp(output, alternate, kat->output_length) == 0) {
            return 0;
        }
        changed[0] = fields[1];
        changed[1] = fields[0];
        if (evaluate(kat, changed, 3U, alternate) != TRIFACT_STATUS_OK ||
            memcmp(output, alternate, kat->output_length) == 0 ||
            evaluate(kat, fields, 2U, alternate) != TRIFACT_STATUS_OK ||
            memcmp(output, alternate, kat->output_length) == 0) {
            return 0;
        }
    }
    return 1;
}

static int check_boundaries(void) {
    const unsigned char abc[] = {'a', 'b', 'c'};
    const trifact_hash_field_t first[] = {{abc, 2U}, {abc + 2U, 1U}};
    const trifact_hash_field_t second[] = {{abc, 1U}, {abc + 1U, 2U}};
    const trifact_hash_field_t empty = {NULL, 0U};
    const trifact_hash_field_t invalid = {NULL, 1U};
    unsigned char left[64] = {0U};
    unsigned char right[64] = {0U};
    unsigned char sentinel[64] = {0U};
    trifact_shake256_t *context = NULL;
    trifact_domain_t domain = TRIFACT_DOMAIN_CHALLENGE;
    size_t index = 0U;

    if (trifact_hash32(TRIFACT_DOMAIN_PK_DIGEST, first, 2U, left) != TRIFACT_STATUS_OK ||
        trifact_hash32(TRIFACT_DOMAIN_PK_DIGEST, second, 2U, right) != TRIFACT_STATUS_OK ||
        memcmp(left, right, 32U) == 0 ||
        trifact_hash32(TRIFACT_DOMAIN_MESSAGE_DIGEST, first, 2U, right) != TRIFACT_STATUS_OK ||
        memcmp(left, right, 32U) == 0 ||
        trifact_hash32(TRIFACT_DOMAIN_PK_DIGEST, NULL, 0U, left) != TRIFACT_STATUS_OK ||
        trifact_hash32(TRIFACT_DOMAIN_PK_DIGEST, &empty, 1U, right) != TRIFACT_STATUS_OK ||
        memcmp(left, right, 32U) == 0 ||
        trifact_xof(TRIFACT_DOMAIN_SIGN_SALT, first, 2U, left, 32U) != TRIFACT_STATUS_OK ||
        trifact_xof(TRIFACT_DOMAIN_SIGN_SALT, first, 2U, right, 64U) != TRIFACT_STATUS_OK ||
        memcmp(left, right, 32U) == 0) {
        return 0;
    }
    memset(left, 0x5a, sizeof(left));
    memcpy(sentinel, left, sizeof(left));
    if (trifact_hash32(TRIFACT_DOMAIN_PK_DIGEST, &invalid, 1U, left) !=
            TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_hash32((trifact_domain_t)-1, first, 2U, left) != TRIFACT_STATUS_INVALID_DOMAIN ||
        trifact_hash32(TRIFACT_DOMAIN_PK_DIGEST, NULL, 0U, NULL) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_xof(TRIFACT_DOMAIN_SIGN_SALT, first, 2U, NULL, 1U) !=
            TRIFACT_STATUS_NULL_ARGUMENT ||
        memcmp(left, sentinel, sizeof(left)) != 0) {
        return 0;
    }
#if SIZE_MAX > UINT32_MAX
    {
        const trifact_hash_field_t oversized = {abc, (size_t)UINT32_MAX + 1U};

        if (trifact_hash32(TRIFACT_DOMAIN_PK_DIGEST, &oversized, 1U, left) !=
                TRIFACT_STATUS_COUNT_OUT_OF_RANGE ||
            trifact_hash32(TRIFACT_DOMAIN_PK_DIGEST, first, (size_t)UINT32_MAX + 1U, left) !=
                TRIFACT_STATUS_COUNT_OUT_OF_RANGE ||
            trifact_xof(TRIFACT_DOMAIN_SIGN_SALT, first, 2U, left, (size_t)UINT32_MAX + 1U) !=
                TRIFACT_STATUS_COUNT_OUT_OF_RANGE ||
            memcmp(left, sentinel, sizeof(left)) != 0) {
            return 0;
        }
    }
#endif
    for (index = 0U; index < sizeof(hash_kats) / sizeof(hash_kats[0]); ++index) {
        const hash_kat_t *kat = &hash_kats[index];

        if (kat->mode != 0 &&
            trifact_hash32(kat->domain, first, 2U, left) != TRIFACT_STATUS_INVALID_PRIMITIVE_MODE) {
            return 0;
        }
        if (kat->mode != 1 && trifact_xof(kat->domain, first, 2U, left, 32U) !=
                                  TRIFACT_STATUS_INVALID_PRIMITIVE_MODE) {
            return 0;
        }
        if (kat->mode != 2 && (trifact_xof_stream_create(&context, kat->domain, first, 2U) !=
                                   TRIFACT_STATUS_INVALID_PRIMITIVE_MODE ||
                               context != NULL)) {
            return 0;
        }
    }
    if (memcmp(left, sentinel, sizeof(left)) != 0 ||
        trifact_domain_from_name("TRIFACT/challenge ", &domain) != TRIFACT_STATUS_INVALID_DOMAIN ||
        trifact_domain_from_name("TRIFACT/unknown", &domain) != TRIFACT_STATUS_INVALID_DOMAIN ||
        trifact_domain_from_name("TRIFACT/Challenge", &domain) != TRIFACT_STATUS_INVALID_DOMAIN ||
        domain != TRIFACT_DOMAIN_CHALLENGE ||
        trifact_xof_stream_create(&context, TRIFACT_DOMAIN_CHALLENGE, &invalid, 1U) !=
            TRIFACT_STATUS_NULL_ARGUMENT ||
        context != NULL) {
        return 0;
    }
    return 1;
}

int main(void) {
    if (!check_vectors() || !check_boundaries()) {
        fputs("Domain or framing substitution check failed\n", stderr);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
