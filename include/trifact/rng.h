#ifndef TRIFACT_RNG_H
#define TRIFACT_RNG_H

#include "trifact/core.h"

typedef struct trifact_entropy_provider trifact_entropy_provider_t;

typedef trifact_status_t (*trifact_entropy_callback_t)(void *context, unsigned char *buffer,
                                                       size_t requested, size_t *received);
typedef trifact_status_t (*trifact_random_read_t)(void *context, unsigned char *output,
                                                  size_t length);

typedef struct {
    trifact_random_read_t read;
    void *context;
} trifact_random_source_t;

trifact_status_t trifact_entropy_provider_create(trifact_entropy_provider_t **out_provider,
                                                 trifact_entropy_callback_t callback, void *context,
                                                 uint32_t interruption_limit);
trifact_status_t trifact_entropy_read_exact(trifact_entropy_provider_t *provider,
                                            unsigned char *output, size_t length);
trifact_status_t trifact_entropy_provider_create_system(trifact_entropy_provider_t **out_provider,
                                                        uint32_t interruption_limit);
trifact_random_source_t trifact_entropy_source(trifact_entropy_provider_t *provider);
void trifact_entropy_provider_destroy(trifact_entropy_provider_t *provider);

trifact_status_t trifact_uniform_u32(const trifact_random_source_t *source, uint64_t bound,
                                     uint32_t max_draws, uint32_t *out_value);
trifact_status_t trifact_random_permutation(const trifact_random_source_t *source,
                                            trifact_vertex_t vertex_count, uint32_t max_draws,
                                            trifact_vertex_t *output);

#endif
