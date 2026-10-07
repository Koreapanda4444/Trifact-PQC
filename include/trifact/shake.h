#ifndef TRIFACT_SHAKE_H
#define TRIFACT_SHAKE_H

#include "trifact/core.h"

typedef struct trifact_shake256 trifact_shake256_t;

trifact_status_t trifact_shake256_create(trifact_shake256_t **out_context);
trifact_status_t trifact_shake256_absorb(trifact_shake256_t *context, const unsigned char *input,
                                         size_t length);
trifact_status_t trifact_shake256_finalize(trifact_shake256_t *context);
trifact_status_t trifact_shake256_squeeze(trifact_shake256_t *context, unsigned char *output,
                                          size_t length);
void trifact_shake256_destroy(trifact_shake256_t *context);
trifact_status_t trifact_shake256(const unsigned char *input, size_t input_length,
                                  unsigned char *output, size_t output_length);

#endif
