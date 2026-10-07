#ifndef TRIFACT_HASH_H
#define TRIFACT_HASH_H

#include "trifact/shake.h"

typedef enum {
    TRIFACT_DOMAIN_PK_DIGEST = 0,
    TRIFACT_DOMAIN_MESSAGE_DIGEST,
    TRIFACT_DOMAIN_SIGNING_CONTEXT,
    TRIFACT_DOMAIN_KEYGEN_ATTEMPT,
    TRIFACT_DOMAIN_SIGN_SALT,
    TRIFACT_DOMAIN_PROOF_EXPAND,
    TRIFACT_DOMAIN_VIEW_COMMITMENT,
    TRIFACT_DOMAIN_COMMITMENT_ROOT,
    TRIFACT_DOMAIN_CHALLENGE,
    TRIFACT_DOMAIN_COUNT
} trifact_domain_t;

typedef struct {
    const unsigned char *data;
    size_t length;
} trifact_hash_field_t;

const char *trifact_domain_name(trifact_domain_t domain);
trifact_status_t trifact_domain_from_name(const char *name, trifact_domain_t *out_domain);
trifact_status_t trifact_hash32(trifact_domain_t domain, const trifact_hash_field_t *fields,
                                size_t field_count, unsigned char output[32]);
trifact_status_t trifact_xof(trifact_domain_t domain, const trifact_hash_field_t *fields,
                             size_t field_count, unsigned char *output, size_t output_length);
trifact_status_t trifact_xof_stream_create(trifact_shake256_t **out_context,
                                           trifact_domain_t domain,
                                           const trifact_hash_field_t *fields, size_t field_count);

#endif
