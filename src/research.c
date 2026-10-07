#include "trifact/research.h"
#include "memory.h"
#include "secure.h"
#include "trifact/hash.h"

#include <string.h>

struct trifact_research_stream {
    trifact_shake256_t *context;
};

trifact_status_t trifact_research_stream_create(trifact_research_stream_t **out_stream,
                                                const char *parameter_id,
                                                const unsigned char seed[32], uint32_t attempt) {
    trifact_research_stream_t *stream = NULL;
    unsigned char parameter[7] = {0U};
    const unsigned char attempt_bytes[4] = {(unsigned char)(attempt >> 24U),
                                            (unsigned char)(attempt >> 16U),
                                            (unsigned char)(attempt >> 8U), (unsigned char)attempt};
    trifact_hash_field_t fields[3] = {{0}};
    size_t name_length = 0U;
    trifact_status_t status = TRIFACT_STATUS_OK;

    if (out_stream == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    *out_stream = NULL;
    if (parameter_id == NULL || seed == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    if (strcmp(parameter_id, "toy") != 0 && strcmp(parameter_id, "small") != 0 &&
        strcmp(parameter_id, "medium") != 0) {
        return TRIFACT_STATUS_INVALID_PARAMETER_ID;
    }
    name_length = strlen(parameter_id);
    parameter[0] = (unsigned char)name_length;
    memcpy(parameter + 1U, parameter_id, name_length);
    fields[0].data = parameter;
    fields[0].length = name_length + 1U;
    fields[1].data = seed;
    fields[1].length = 32U;
    fields[2].data = attempt_bytes;
    fields[2].length = sizeof(attempt_bytes);
    stream = trifact_memory_allocate_zero(1U, sizeof(*stream));
    if (stream == NULL) {
        return TRIFACT_STATUS_ALLOCATION_FAILURE;
    }
    status = trifact_xof_stream_create(&stream->context, TRIFACT_DOMAIN_KEYGEN_ATTEMPT, fields, 3U);
    if (status != TRIFACT_STATUS_OK) {
        trifact_secure_clear(stream, sizeof(*stream));
        trifact_memory_free(stream);
        return status;
    }
    *out_stream = stream;
    return TRIFACT_STATUS_OK;
}

trifact_status_t trifact_research_stream_read(trifact_research_stream_t *stream,
                                              unsigned char *output, size_t length) {
    if (stream == NULL) {
        return TRIFACT_STATUS_NULL_ARGUMENT;
    }
    return trifact_shake256_squeeze(stream->context, output, length);
}

static trifact_status_t research_read(void *context, unsigned char *output, size_t length) {
    return trifact_research_stream_read(context, output, length);
}

trifact_random_source_t trifact_research_source(trifact_research_stream_t *stream) {
    const trifact_random_source_t source = {stream == NULL ? NULL : research_read, stream};

    return source;
}

void trifact_research_stream_destroy(trifact_research_stream_t *stream) {
    if (stream != NULL) {
        trifact_shake256_destroy(stream->context);
        trifact_secure_clear(stream, sizeof(*stream));
        trifact_memory_free(stream);
    }
}
