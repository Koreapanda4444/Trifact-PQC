#include "failing_allocator.h"
#include "memory.h"

#include <stdlib.h>

static void *live_pointers[64] = {NULL};
static size_t allocation_calls = 0U;
static size_t failure_index = 0U;
static size_t live_count = 0U;
static size_t errors = 0U;

static int should_fail(void) {
    ++allocation_calls;
    return failure_index != 0U && allocation_calls == failure_index;
}

static void *record_allocation(void *pointer) {
    size_t slot = 0U;

    if (pointer == NULL) {
        return NULL;
    }
    for (slot = 0U; slot < 64U; ++slot) {
        if (live_pointers[slot] == NULL) {
            live_pointers[slot] = pointer;
            ++live_count;
            return pointer;
        }
    }
    ++errors;
    free(pointer);
    return NULL;
}

void *trifact_memory_allocate(size_t size) {
    return should_fail() != 0 ? NULL : record_allocation(malloc(size));
}

void *trifact_memory_allocate_zero(size_t count, size_t size) {
    return should_fail() != 0 ? NULL : record_allocation(calloc(count, size));
}

void trifact_memory_free(void *pointer) {
    size_t slot = 0U;

    if (pointer == NULL) {
        return;
    }
    for (slot = 0U; slot < 64U; ++slot) {
        if (live_pointers[slot] == pointer) {
            live_pointers[slot] = NULL;
            --live_count;
            free(pointer);
            return;
        }
    }
    ++errors;
}

void trifact_test_allocator_reset(size_t failure_call) {
    allocation_calls = 0U;
    failure_index = failure_call;
}

size_t trifact_test_allocator_calls(void) { return allocation_calls; }

size_t trifact_test_allocator_live(void) { return live_count; }

size_t trifact_test_allocator_errors(void) { return errors; }
