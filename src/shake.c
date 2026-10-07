#include "trifact/shake.h"
#include "keccak.h"
#include "memory.h"
#include "secure.h"

struct trifact_shake256 {
    uint64_t lanes[25];
    size_t position;
    int squeezing;
};

static void absorb_byte(trifact_shake256_t *context, size_t position, unsigned char byte) {
    context->lanes[position / 8U] ^= (uint64_t)byte << (unsigned int)(8U * (position % 8U));
}

trifact_status_t trifact_shake256_create(trifact_shake256_t **out_context) {
    trifact_shake256_t *context = NULL;

    if (out_context == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    *out_context = NULL;
    context = trifact_memory_allocate_zero(1U, sizeof(*context));
    if (context == NULL) {
        return TRIFACT_STATUS_ALLOCATION_FAILURE;
    }
    *out_context = context;
    return TRIFACT_STATUS_OK;
}

trifact_status_t trifact_shake256_absorb(trifact_shake256_t *context, const unsigned char *input,
                                         size_t length) {
    size_t index = 0U;

    if (context == NULL || (input == NULL && length > 0U)) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    if (context->squeezing != 0) {
        return TRIFACT_STATUS_INVALID_STATE;
    }
    for (index = 0U; index < length; ++index) {
        absorb_byte(context, context->position, input[index]);
        ++context->position;
        if (context->position == 136U) {
            trifact_keccak_permute(context->lanes);
            context->position = 0U;
        }
    }
    return TRIFACT_STATUS_OK;
}

trifact_status_t trifact_shake256_finalize(trifact_shake256_t *context) {
    if (context == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    if (context->squeezing != 0) {
        return TRIFACT_STATUS_INVALID_STATE;
    }
    absorb_byte(context, context->position, 0x1fU);
    absorb_byte(context, 135U, 0x80U);
    trifact_keccak_permute(context->lanes);
    context->position = 0U;
    context->squeezing = 1;
    return TRIFACT_STATUS_OK;
}

trifact_status_t trifact_shake256_squeeze(trifact_shake256_t *context, unsigned char *output,
                                          size_t length) {
    size_t index = 0U;

    if (context == NULL || (output == NULL && length > 0U)) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    if (context->squeezing == 0) {
        return TRIFACT_STATUS_INVALID_STATE;
    }
    for (index = 0U; index < length; ++index) {
        if (context->position == 136U) {
            trifact_keccak_permute(context->lanes);
            context->position = 0U;
        }
        output[index] = (unsigned char)(context->lanes[context->position / 8U] >>
                                        (unsigned int)(8U * (context->position % 8U)));
        ++context->position;
    }
    return TRIFACT_STATUS_OK;
}

void trifact_shake256_destroy(trifact_shake256_t *context) {
    if (context != NULL) {
        trifact_secure_clear(context, sizeof(*context));
        trifact_memory_free(context);
    }
}

trifact_status_t trifact_shake256(const unsigned char *input, size_t input_length,
                                  unsigned char *output, size_t output_length) {
    trifact_shake256_t *context = NULL;
    trifact_status_t status = TRIFACT_STATUS_OK;

    if ((input == NULL && input_length > 0U) || (output == NULL && output_length > 0U)) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    status = trifact_shake256_create(&context);
    if (status == TRIFACT_STATUS_OK) {
        status = trifact_shake256_absorb(context, input, input_length);
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
