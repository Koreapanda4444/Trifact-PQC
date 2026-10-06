#ifndef TRIFACT_FAILING_ALLOCATOR_H
#define TRIFACT_FAILING_ALLOCATOR_H

#include <stddef.h>

void trifact_test_allocator_reset(size_t failure_call);
size_t trifact_test_allocator_calls(void);
size_t trifact_test_allocator_live(void);
size_t trifact_test_allocator_errors(void);

#endif
