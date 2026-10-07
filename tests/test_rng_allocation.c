#include "failing_allocator.h"
#include "trifact/trifact.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    SHAKE_CREATE,
    SHAKE_ONESHOT,
    HASH32,
    XOF,
    XOF_STREAM,
    ENTROPY_CREATE,
    ENTROPY_READ,
    SYSTEM_CREATE,
    RESEARCH_CREATE,
    RESEARCH_READ,
    PERMUTATION,
    ENTROPY_PERMUTATION,
    EMPTY_ENTROPY_READ,
    EMPTY_PERMUTATION,
    UNIFORM,
    OPERATION_COUNT
} operation_t;

static trifact_status_t entropy_bytes(void *context, unsigned char *buffer, size_t requested,
                                      size_t *received) {
    size_t *calls = context;

    ++*calls;
    memset(buffer, 0xa5, requested);
    *received = requested;
    return TRIFACT_STATUS_OK;
}

static trifact_status_t zero_bytes(void *context, unsigned char *output, size_t length) {
    size_t *calls = context;

    ++*calls;
    memset(output, 0, length);
    return TRIFACT_STATUS_OK;
}

static int check_operation(operation_t operation, size_t failure_call, size_t allocation_count,
                           size_t *observed_calls) {
    unsigned char seed[32] = {0U};
    unsigned char bytes[32] = {0U};
    unsigned char sentinel[32] = {0U};
    trifact_vertex_t permutation[4] = {99U, 98U, 97U, 96U};
    const trifact_vertex_t saved_permutation[4] = {99U, 98U, 97U, 96U};
    const trifact_hash_field_t field = {seed, sizeof(seed)};
    trifact_shake256_t *shake = NULL;
    trifact_entropy_provider_t *created_provider = NULL;
    trifact_entropy_provider_t *provider = NULL;
    trifact_research_stream_t *created_stream = NULL;
    trifact_research_stream_t *stream = NULL;
    size_t source_calls = 0U;
    trifact_random_source_t source = {zero_bytes, &source_calls};
    uint32_t value = 99U;
    size_t baseline = 0U;
    const int failing = failure_call != 0U && failure_call <= allocation_count;
    const trifact_status_t expected =
        failing ? TRIFACT_STATUS_ALLOCATION_FAILURE : TRIFACT_STATUS_OK;
    trifact_status_t status = TRIFACT_STATUS_INVALID_STATE;
    int failed = 0;

    memset(bytes, 0xa5, sizeof(bytes));
    memcpy(sentinel, bytes, sizeof(bytes));
    trifact_test_allocator_reset(0U);
    if (trifact_entropy_provider_create(&provider, entropy_bytes, &source_calls, 1U) !=
            TRIFACT_STATUS_OK ||
        trifact_research_stream_create(&stream, "toy", seed, 0U) != TRIFACT_STATUS_OK) {
        trifact_entropy_provider_destroy(provider);
        trifact_research_stream_destroy(stream);
        return 1;
    }
    baseline = trifact_test_allocator_live();
    trifact_test_allocator_reset(failure_call);
    switch (operation) {
    case SHAKE_CREATE:
        status = trifact_shake256_create(&shake);
        break;
    case SHAKE_ONESHOT:
        status = trifact_shake256(seed, sizeof(seed), bytes, sizeof(bytes));
        break;
    case HASH32:
        status = trifact_hash32(TRIFACT_DOMAIN_PK_DIGEST, &field, 1U, bytes);
        break;
    case XOF:
        status = trifact_xof(TRIFACT_DOMAIN_SIGN_SALT, &field, 1U, bytes, sizeof(bytes));
        break;
    case XOF_STREAM:
        status = trifact_xof_stream_create(&shake, TRIFACT_DOMAIN_KEYGEN_ATTEMPT, &field, 1U);
        break;
    case ENTROPY_CREATE:
        status =
            trifact_entropy_provider_create(&created_provider, entropy_bytes, &source_calls, 1U);
        break;
    case ENTROPY_READ:
        status = trifact_entropy_read_exact(provider, bytes, sizeof(bytes));
        break;
    case SYSTEM_CREATE:
        status = trifact_entropy_provider_create_system(&created_provider, 1U);
#if !defined(_WIN32) && !defined(__linux__)
        if (status == TRIFACT_STATUS_PLATFORM_UNAVAILABLE) {
            status = TRIFACT_STATUS_OK;
        }
#endif
        break;
    case RESEARCH_CREATE:
        status = trifact_research_stream_create(&created_stream, "medium", seed, UINT32_MAX);
        break;
    case RESEARCH_READ:
        status = trifact_research_stream_read(stream, bytes, sizeof(bytes));
        break;
    case PERMUTATION:
        status = trifact_random_permutation(&source, 4U, 1U, permutation);
        break;
    case ENTROPY_PERMUTATION:
        source = trifact_entropy_source(provider);
        status = trifact_random_permutation(&source, 4U, 2U, permutation);
        break;
    case EMPTY_ENTROPY_READ:
        status = trifact_entropy_read_exact(provider, NULL, 0U);
        break;
    case EMPTY_PERMUTATION:
        status = trifact_random_permutation(&source, 0U, 1U, NULL);
        break;
    case UNIFORM:
        status = trifact_uniform_u32(&source, 7U, 1U, &value);
        break;
    case OPERATION_COUNT:
        break;
    }
    *observed_calls = trifact_test_allocator_calls();
    if (status != expected || (failing && *observed_calls != failure_call) ||
        (failure_call > allocation_count && *observed_calls != allocation_count) ||
        (failing && (shake != NULL || created_provider != NULL || created_stream != NULL ||
                     memcmp(bytes, sentinel, sizeof(bytes)) != 0 || value != 99U ||
                     memcmp(permutation, saved_permutation, sizeof(permutation)) != 0))) {
        failed = 1;
    }
    trifact_shake256_destroy(shake);
    trifact_entropy_provider_destroy(created_provider);
    trifact_research_stream_destroy(created_stream);
    if (trifact_test_allocator_live() != baseline) {
        failed = 1;
    }
    if (failing && (operation == ENTROPY_READ || operation == ENTROPY_PERMUTATION)) {
        if ((failure_call == 1U || (operation == ENTROPY_PERMUTATION && failure_call == 2U)) &&
            source_calls != 0U) {
            failed = 1;
        }
        trifact_test_allocator_reset(0U);
        if (trifact_entropy_read_exact(provider, bytes, 1U) != TRIFACT_STATUS_OK) {
            failed = 1;
        }
    }
    trifact_entropy_provider_destroy(provider);
    trifact_research_stream_destroy(stream);
    if (trifact_test_allocator_live() != 0U || trifact_test_allocator_errors() != 0U) {
        failed = 1;
    }
    if (failed != 0) {
        (void)fprintf(stderr, "crypto allocation failure: operation=%d call=%zu status=%d\n",
                      (int)operation, failure_call, (int)status);
    }
    return failed;
}

int main(void) {
    int operation = 0;
    int failures = 0;

    trifact_test_allocator_require_clear(1);
    for (operation = 0; operation < (int)OPERATION_COUNT; ++operation) {
        size_t calls = 0U;
        size_t observed = 0U;
        size_t failure = 0U;

        failures += check_operation((operation_t)operation, 0U, 0U, &calls);
        for (failure = 1U; failure <= calls + 1U; ++failure) {
            failures += check_operation((operation_t)operation, failure, calls, &observed);
        }
    }
    trifact_test_allocator_require_clear(0);
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
