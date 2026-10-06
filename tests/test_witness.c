#include "trifact/witness.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int check_normalization(const trifact_factor_label_t *data,
                               const trifact_factor_label_t *expected, size_t count) {
    trifact_label_vector_t *original = NULL;
    trifact_label_vector_t *normalized = NULL;
    trifact_label_vector_t *repeated = NULL;
    int equivalent = 0;
    int failed = 0;

    if (trifact_label_vector_create(&original, data, count) != TRIFACT_STATUS_OK ||
        trifact_label_vector_normalize(&normalized, original) != TRIFACT_STATUS_OK ||
        trifact_label_vector_normalize(&repeated, normalized) != TRIFACT_STATUS_OK) {
        failed = 1;
    } else if (trifact_label_vector_count(normalized) != count ||
               trifact_label_vector_count(repeated) != count ||
               (count > 0U &&
                (memcmp(trifact_label_vector_data(original), data, count * sizeof(*data)) != 0 ||
                 memcmp(trifact_label_vector_data(normalized), expected,
                        count * sizeof(*expected)) != 0 ||
                 memcmp(trifact_label_vector_data(repeated), expected, count * sizeof(*expected)) !=
                     0)) ||
               trifact_label_vectors_equivalent(original, normalized, &equivalent) !=
                   TRIFACT_STATUS_OK ||
               equivalent != 1) {
        failed = 1;
    }
    trifact_label_vector_destroy(repeated);
    trifact_label_vector_destroy(normalized);
    trifact_label_vector_destroy(original);
    return failed;
}

static int check_equivalence(const trifact_factor_label_t *left, size_t left_count,
                             const trifact_factor_label_t *right, size_t right_count,
                             int expected) {
    trifact_label_vector_t *left_vector = NULL;
    trifact_label_vector_t *right_vector = NULL;
    int actual = -1;
    int failed = 0;

    if (trifact_label_vector_create(&left_vector, left, left_count) != TRIFACT_STATUS_OK ||
        trifact_label_vector_create(&right_vector, right, right_count) != TRIFACT_STATUS_OK) {
        failed = 1;
    } else {
        if (trifact_label_vectors_equivalent(left_vector, right_vector, &actual) !=
                TRIFACT_STATUS_OK ||
            actual != expected ||
            trifact_label_vectors_equivalent(right_vector, left_vector, &actual) !=
                TRIFACT_STATUS_OK ||
            actual != expected) {
            failed = 1;
        }
        actual = -1;
        if (trifact_label_vectors_equivalent(left_vector, NULL, &actual) !=
                TRIFACT_STATUS_NULL_ARGUMENT ||
            actual != -1 ||
            trifact_label_vectors_equivalent(NULL, right_vector, &actual) !=
                TRIFACT_STATUS_NULL_ARGUMENT ||
            actual != -1 ||
            trifact_label_vectors_equivalent(left_vector, right_vector, NULL) !=
                TRIFACT_STATUS_NULL_ARGUMENT) {
            failed = 1;
        }
    }
    trifact_label_vector_destroy(right_vector);
    trifact_label_vector_destroy(left_vector);
    return failed;
}

int main(void) {
    const trifact_factor_label_t example[] = {4U, 4U, 2U, 4U, 7U, 2U, 7U};
    const trifact_factor_label_t canonical[] = {0U, 0U, 1U, 0U, 2U, 1U, 2U};
    const trifact_factor_label_t large[] = {UINT32_MAX, 19U, UINT32_MAX, 0U};
    const trifact_factor_label_t large_expected[] = {0U, 1U, 0U, 2U};
    const trifact_factor_label_t distinct[] = {9U, 7U, 4U, 2U};
    const trifact_factor_label_t distinct_expected[] = {0U, 1U, 2U, 3U};
    const trifact_factor_label_t same[] = {UINT32_MAX, UINT32_MAX, UINT32_MAX};
    const trifact_factor_label_t same_expected[] = {0U, 0U, 0U};
    const trifact_factor_label_t split[] = {0U, 1U, 2U, 3U};
    const trifact_factor_label_t merged[] = {0U, 0U, 0U, 1U};
    const trifact_factor_label_t reordered[] = {0U, 0U, 1U, 2U};
    trifact_label_vector_t *out_labels = NULL;
    int failures = 0;

    failures += check_normalization(example, canonical, 7U);
    failures += check_normalization(large, large_expected, 4U);
    failures += check_normalization(distinct, distinct_expected, 4U);
    failures += check_normalization(same, same_expected, 3U);
    failures += check_normalization(NULL, NULL, 0U);
    failures += check_equivalence(example, 7U, canonical, 7U, 1);
    failures += check_equivalence(large, 4U, large_expected, 4U, 1);
    failures += check_equivalence(large, 4U, split, 4U, 0);
    failures += check_equivalence(large, 4U, merged, 4U, 0);
    failures += check_equivalence(large, 4U, reordered, 4U, 0);
    failures += check_equivalence(large, 4U, large, 3U, 0);
    failures += check_equivalence(NULL, 0U, NULL, 0U, 1);
    if (trifact_label_vector_normalize(NULL, NULL) != TRIFACT_STATUS_NULL_ARGUMENT ||
        trifact_label_vector_normalize(&out_labels, NULL) != TRIFACT_STATUS_NULL_ARGUMENT ||
        out_labels != NULL) {
        ++failures;
    }
    if (failures != 0) {
        (void)fprintf(stderr, "witness checks failed: %d\n", failures);
    }
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
