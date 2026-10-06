#ifndef TRIFACT_MEMORY_H
#define TRIFACT_MEMORY_H

#include <stddef.h>

void *trifact_memory_allocate(size_t size);
void *trifact_memory_allocate_zero(size_t count, size_t size);
void trifact_memory_free(void *pointer);

#endif
