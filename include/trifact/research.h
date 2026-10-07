#ifndef TRIFACT_RESEARCH_H
#define TRIFACT_RESEARCH_H

#include "trifact/rng.h"

typedef struct trifact_research_stream trifact_research_stream_t;

trifact_status_t trifact_research_stream_create(trifact_research_stream_t **out_stream,
                                                const char *parameter_id,
                                                const unsigned char seed[32], uint32_t attempt);
trifact_status_t trifact_research_stream_read(trifact_research_stream_t *stream,
                                              unsigned char *output, size_t length);
trifact_random_source_t trifact_research_source(trifact_research_stream_t *stream);
void trifact_research_stream_destroy(trifact_research_stream_t *stream);

#endif
