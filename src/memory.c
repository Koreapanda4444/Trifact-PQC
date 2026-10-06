#include "memory.h"

#include <stdlib.h>

void *trifact_memory_allocate(size_t size) { return malloc(size); }

void *trifact_memory_allocate_zero(size_t count, size_t size) { return calloc(count, size); }

void trifact_memory_free(void *pointer) { free(pointer); }
