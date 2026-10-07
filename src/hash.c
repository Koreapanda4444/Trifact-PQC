#include "trifact/hash.h"

#include <string.h>

typedef enum { HASH_MODE, XOF_MODE, STREAM_MODE } primitive_mode_t;

typedef struct {
    const char *name;
    primitive_mode_t mode;
} domain_entry_t;

static const domain_entry_t domains[TRIFACT_DOMAIN_COUNT] = {
    {"TRIFACT/pk-digest", HASH_MODE},       {"TRIFACT/message-digest", HASH_MODE},
    {"TRIFACT/signing-context", HASH_MODE}, {"TRIFACT/keygen-attempt", STREAM_MODE},
    {"TRIFACT/sign-salt", XOF_MODE},        {"TRIFACT/proof-expand", XOF_MODE},
    {"TRIFACT/view-commitment", HASH_MODE}, {"TRIFACT/commitment-root", HASH_MODE},
    {"TRIFACT/challenge", STREAM_MODE}};

const char *trifact_domain_name(trifact_domain_t domain) {
    if ((unsigned int)domain >= (unsigned int)TRIFACT_DOMAIN_COUNT) {
        return NULL;
    }
    return domains[domain].name;
}

trifact_status_t trifact_domain_from_name(const char *name, trifact_domain_t *out_domain) {
    unsigned int index = 0U;

    if (name == NULL || out_domain == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    for (index = 0U; index < (unsigned int)TRIFACT_DOMAIN_COUNT; ++index) {
        if (strcmp(name, domains[index].name) == 0) {
            *out_domain = (trifact_domain_t)index;
            return TRIFACT_STATUS_OK;
        }
    }
    return TRIFACT_STATUS_INVALID_DOMAIN;
}

static int add_size(size_t *total, size_t amount) {
    if (amount > SIZE_MAX - *total) {
        return 0;
    }
    *total += amount;
    return 1;
}

static trifact_status_t validate_frame(trifact_domain_t domain, primitive_mode_t mode,
                                       const trifact_hash_field_t *fields, size_t field_count) {
    size_t total = 0U;
    size_t index = 0U;
    const char *name = trifact_domain_name(domain);

    if (name == NULL) {
        return TRIFACT_STATUS_INVALID_DOMAIN;
    }
    if (domains[domain].mode != mode) {
        return TRIFACT_STATUS_INVALID_PRIMITIVE_MODE;
    }
    if (fields == NULL && field_count != 0U) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    if (field_count > UINT32_MAX) {
        return TRIFACT_STATUS_COUNT_OUT_OF_RANGE;
    }
    if (field_count > SIZE_MAX / sizeof(*fields)) {
        return TRIFACT_STATUS_SIZE_OVERFLOW;
    }
    if (!add_size(&total, mode == HASH_MODE ? 12U : 11U) || !add_size(&total, 4U) ||
        !add_size(&total, strlen(name)) || !add_size(&total, 4U) ||
        (mode == XOF_MODE && !add_size(&total, 4U))) {
        return TRIFACT_STATUS_SIZE_OVERFLOW;
    }
    for (index = 0U; index < field_count; ++index) {
        if (fields[index].data == NULL && fields[index].length != 0U) {
            return TRIFACT_STATUS_NULL_ARGUMENT;
        }
        if (fields[index].length > UINT32_MAX) {
            return TRIFACT_STATUS_COUNT_OUT_OF_RANGE;
        }
        if (!add_size(&total, 4U) || !add_size(&total, fields[index].length)) {
            return TRIFACT_STATUS_SIZE_OVERFLOW;
        }
    }
    return TRIFACT_STATUS_OK;
}

static trifact_status_t absorb_u32(trifact_shake256_t *context, uint32_t value) {
    const unsigned char bytes[4] = {(unsigned char)(value >> 24U), (unsigned char)(value >> 16U),
                                    (unsigned char)(value >> 8U), (unsigned char)value};

    return trifact_shake256_absorb(context, bytes, sizeof(bytes));
}

static trifact_status_t absorb_blob(trifact_shake256_t *context, const unsigned char *bytes,
                                    size_t length) {
    trifact_status_t status = absorb_u32(context, (uint32_t)length);

    if (status == TRIFACT_STATUS_OK) {
        status = trifact_shake256_absorb(context, bytes, length);
    }
    return status;
}

static trifact_status_t create_frame(trifact_shake256_t **out_context, trifact_domain_t domain,
                                     primitive_mode_t mode, const trifact_hash_field_t *fields,
                                     size_t field_count) {
    trifact_shake256_t *context = NULL;
    const char *prefix = mode == HASH_MODE ? "TRIFACT-HASH" : "TRIFACT-XOF";
    trifact_status_t status = validate_frame(domain, mode, fields, field_count);
    size_t index = 0U;

    if (status != TRIFACT_STATUS_OK) {
        return status;
    }
    status = trifact_shake256_create(&context);
    if (status == TRIFACT_STATUS_OK) {
        status = trifact_shake256_absorb(context, (const unsigned char *)prefix, strlen(prefix));
    }
    if (status == TRIFACT_STATUS_OK) {
        status = absorb_blob(context, (const unsigned char *)domains[domain].name,
                             strlen(domains[domain].name));
    }
    if (status == TRIFACT_STATUS_OK) {
        status = absorb_u32(context, (uint32_t)field_count);
    }
    for (index = 0U; index < field_count && status == TRIFACT_STATUS_OK; ++index) {
        status = absorb_blob(context, fields[index].data, fields[index].length);
    }
    if (status != TRIFACT_STATUS_OK) {
        trifact_shake256_destroy(context);
        return status;
    }
    *out_context = context;
    return TRIFACT_STATUS_OK;
}

trifact_status_t trifact_hash32(trifact_domain_t domain, const trifact_hash_field_t *fields,
                                size_t field_count, unsigned char output[32]) {
    trifact_shake256_t *context = NULL;
    trifact_status_t status = TRIFACT_STATUS_OK;

    if (output == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    status = create_frame(&context, domain, HASH_MODE, fields, field_count);
    if (status == TRIFACT_STATUS_OK) {
        status = trifact_shake256_finalize(context);
    }
    if (status == TRIFACT_STATUS_OK) {
        status = trifact_shake256_squeeze(context, output, 32U);
    }
    trifact_shake256_destroy(context);
    return status;
}

trifact_status_t trifact_xof(trifact_domain_t domain, const trifact_hash_field_t *fields,
                             size_t field_count, unsigned char *output, size_t output_length) {
    trifact_shake256_t *context = NULL;
    trifact_status_t status = TRIFACT_STATUS_OK;

    if (output == NULL && output_length != 0U) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    if (output_length > UINT32_MAX) {
        return TRIFACT_STATUS_COUNT_OUT_OF_RANGE;
    }
    status = create_frame(&context, domain, XOF_MODE, fields, field_count);
    if (status == TRIFACT_STATUS_OK) {
        status = absorb_u32(context, (uint32_t)output_length);
    }
    if (status == TRIFACT_STATUS_OK) {
        status = trifact_shake256_finalize(context);
    }
    if (status == TRIFACT_STATUS_OK) {
        status = trifact_shake256_squeeze(context, output, output_length);
    }
    trifact_shake256_destroy(context);
    return status;
}

trifact_status_t trifact_xof_stream_create(trifact_shake256_t **out_context,
                                           trifact_domain_t domain,
                                           const trifact_hash_field_t *fields, size_t field_count) {
    trifact_shake256_t *context = NULL;
    trifact_status_t status = TRIFACT_STATUS_OK;

    if (out_context == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    *out_context = NULL;
    status = create_frame(&context, domain, STREAM_MODE, fields, field_count);
    if (status == TRIFACT_STATUS_OK) {
        status = trifact_shake256_finalize(context);
    }
    if (status != TRIFACT_STATUS_OK) {
        trifact_shake256_destroy(context);
        return status;
    }
    *out_context = context;
    return TRIFACT_STATUS_OK;
}
