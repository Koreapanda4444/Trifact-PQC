#include "secure.h"

void trifact_secure_clear(void *pointer, size_t length) {
    volatile unsigned char *bytes = pointer;
    size_t index = 0U;

    for (index = 0U; index < length; ++index) {
        bytes[index] = 0U;
    }
}
