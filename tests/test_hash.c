#include "trifact/hash.h"

#include <stdlib.h>
#include <string.h>

int main(void) {
    const unsigned char frame[] = {'T', 'R', 'I', 'F', 'A', 'C', 'T',   '-', 'H', 'A', 'S', 'H',
                                   0U,  0U,  0U,  17U, 'T', 'R', 'I',   'F', 'A', 'C', 'T', '/',
                                   'p', 'k', '-', 'd', 'i', 'g', 'e',   's', 't', 0U,  0U,  0U,
                                   2U,  0U,  0U,  0U,  3U,  0U,  0xffU, 1U,  0U,  0U,  0U,  0U};
    const unsigned char input[] = {0U, 0xffU, 1U};
    const trifact_hash_field_t fields[] = {{input, sizeof(input)}, {NULL, 0U}};
    unsigned char expected[32] = {0U};
    unsigned char output[32] = {0U};
    trifact_domain_t domain = TRIFACT_DOMAIN_COUNT;
    trifact_shake256_t *context = NULL;
    int failed = 0;

    if (trifact_domain_from_name("TRIFACT/pk-digest", &domain) != TRIFACT_STATUS_OK ||
        domain != TRIFACT_DOMAIN_PK_DIGEST ||
        strcmp(trifact_domain_name(domain), "TRIFACT/pk-digest") != 0 ||
        trifact_shake256(frame, sizeof(frame), expected, sizeof(expected)) != TRIFACT_STATUS_OK ||
        trifact_hash32(domain, fields, 2U, output) != TRIFACT_STATUS_OK ||
        memcmp(output, expected, sizeof(output)) != 0) {
        return EXIT_FAILURE;
    }
    memset(output, 0x5a, sizeof(output));
    if (trifact_hash32(TRIFACT_DOMAIN_COUNT, fields, 2U, output) != TRIFACT_STATUS_INVALID_DOMAIN ||
        trifact_hash32(TRIFACT_DOMAIN_SIGN_SALT, fields, 2U, output) !=
            TRIFACT_STATUS_INVALID_PRIMITIVE_MODE ||
        trifact_hash32(domain, NULL, 1U, output) != TRIFACT_STATUS_NULL_ARGUMENT ||
        output[0] != 0x5aU ||
        trifact_xof(TRIFACT_DOMAIN_SIGN_SALT, fields, 2U, output, sizeof(output)) !=
            TRIFACT_STATUS_OK ||
        trifact_xof(TRIFACT_DOMAIN_SIGN_SALT, NULL, 0U, NULL, 0U) != TRIFACT_STATUS_OK ||
        trifact_xof_stream_create(&context, TRIFACT_DOMAIN_KEYGEN_ATTEMPT, fields, 2U) !=
            TRIFACT_STATUS_OK ||
        trifact_shake256_absorb(context, NULL, 0U) != TRIFACT_STATUS_INVALID_STATE ||
        trifact_shake256_squeeze(context, output, sizeof(output)) != TRIFACT_STATUS_OK) {
        failed = 1;
    }
    trifact_shake256_destroy(context);
    context = NULL;
    if (trifact_domain_from_name("trifact/pk-digest", &domain) != TRIFACT_STATUS_INVALID_DOMAIN ||
        domain != TRIFACT_DOMAIN_PK_DIGEST || trifact_domain_name(TRIFACT_DOMAIN_COUNT) != NULL ||
        trifact_domain_from_name(NULL, &domain) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_domain_from_name("TRIFACT/pk-digest", NULL) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_xof_stream_create(NULL, TRIFACT_DOMAIN_KEYGEN_ATTEMPT, NULL, 0U) !=
            TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_xof_stream_create(&context, TRIFACT_DOMAIN_PK_DIGEST, NULL, 0U) !=
            TRIFACT_STATUS_INVALID_PRIMITIVE_MODE ||
        context != NULL) {
        failed = 1;
    }
    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
