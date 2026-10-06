#include "trifact/relation.h"

#include <stdio.h>
#include <stdlib.h>

static int cover_oracle(trifact_vertex_t vertex_count, const trifact_edge_t *edges,
                        size_t edge_count, uint32_t factor_count,
                        const trifact_factor_label_t *labels, size_t label_count) {
    uint32_t previous_code = 0U;
    uint32_t factor = 0U;
    size_t index = 0U;

    if (vertex_count < 6U || vertex_count > 31U || vertex_count % 3U != 0U || factor_count < 2U ||
        (uint64_t)factor_count >
            (uint64_t)(vertex_count - 1U) * (uint64_t)(vertex_count - 2U) / 2U ||
        (uint64_t)edge_count != (uint64_t)vertex_count * (uint64_t)factor_count / 3U ||
        label_count != edge_count) {
        return 0;
    }
    for (index = 0U; index < edge_count; ++index) {
        const uint32_t first = edges[index].vertices[0];
        const uint32_t second = edges[index].vertices[1];
        const uint32_t third = edges[index].vertices[2];
        const uint32_t code = (first * vertex_count + second) * vertex_count + third;

        if (first >= second || second >= third || third >= vertex_count ||
            (index > 0U && code <= previous_code) || labels[index] >= factor_count) {
            return 0;
        }
        previous_code = code;
    }
    for (factor = 0U; factor < factor_count; ++factor) {
        uint32_t covered = 0U;
        size_t factor_edges = 0U;

        for (index = 0U; index < edge_count; ++index) {
            if (labels[index] == factor) {
                const uint32_t mask = (UINT32_C(1) << edges[index].vertices[0]) |
                                      (UINT32_C(1) << edges[index].vertices[1]) |
                                      (UINT32_C(1) << edges[index].vertices[2]);

                if ((covered & mask) != 0U) {
                    return 0;
                }
                covered |= mask;
                ++factor_edges;
            }
        }
        if (covered != (UINT32_C(1) << vertex_count) - 1U ||
            factor_edges != (size_t)vertex_count / 3U) {
            return 0;
        }
    }
    return 1;
}

static int enumerate_labels(trifact_vertex_t vertex_count, const trifact_edge_t *edges,
                            size_t edge_count, uint32_t factor_count, size_t *cases,
                            size_t *accepted) {
    trifact_hypergraph_t *hypergraph = NULL;
    trifact_factor_label_t labels[6] = {0U};
    size_t combinations = 1U;
    size_t assignment = 0U;
    size_t index = 0U;

    if (edge_count > 6U || trifact_hypergraph_create(&hypergraph, vertex_count, edges,
                                                     edge_count) != TRIFACT_STATUS_OK) {
        return 1;
    }
    for (index = 0U; index < edge_count; ++index) {
        combinations *= (size_t)factor_count + 1U;
    }
    for (assignment = 0U; assignment < combinations; ++assignment) {
        trifact_label_vector_t *vector = NULL;
        size_t digits = assignment;
        const uint32_t base = factor_count + 1U;
        int expected = 0;
        trifact_status_t status = TRIFACT_STATUS_OK;

        for (index = 0U; index < edge_count; ++index) {
            labels[index] = (uint32_t)(digits % (size_t)base);
            digits /= (size_t)base;
        }
        expected = cover_oracle(vertex_count, edges, edge_count, factor_count, labels, edge_count);
        if (trifact_label_vector_create(&vector, labels, edge_count) != TRIFACT_STATUS_OK) {
            trifact_hypergraph_destroy(hypergraph);
            return 1;
        }
        status = trifact_relation_validate(hypergraph, factor_count, vector, NULL);
        trifact_label_vector_destroy(vector);
        if ((status == TRIFACT_STATUS_OK) != expected ||
            status == TRIFACT_STATUS_ALLOCATION_FAILURE || status == TRIFACT_STATUS_SIZE_OVERFLOW) {
            (void)fprintf(stderr, "oracle mismatch: n=%u d=%u m=%zu assignment=%zu status=%d\n",
                          (unsigned int)vertex_count, (unsigned int)factor_count, edge_count,
                          assignment, (int)status);
            trifact_hypergraph_destroy(hypergraph);
            return 1;
        }
        ++*cases;
        if (expected != 0) {
            ++*accepted;
        }
    }
    trifact_hypergraph_destroy(hypergraph);
    return 0;
}

static int enumerate_graphs(const trifact_edge_t *universe, size_t begin, trifact_edge_t *selected,
                            size_t count, size_t *cases, size_t *accepted) {
    size_t index = 0U;

    if (enumerate_labels(6U, selected, count, 2U, cases, accepted) != 0) {
        return 1;
    }
    if (count == 4U) {
        return 0;
    }
    for (index = begin; index < 20U; ++index) {
        selected[count] = universe[index];
        if (enumerate_graphs(universe, index + 1U, selected, count + 1U, cases, accepted) != 0) {
            return 1;
        }
    }
    return 0;
}

int main(void) {
    const trifact_edge_t degree_three[] = {{{0U, 1U, 2U}}, {{0U, 1U, 3U}}, {{0U, 1U, 4U}},
                                           {{2U, 3U, 5U}}, {{2U, 4U, 5U}}, {{3U, 4U, 5U}}};
    const trifact_edge_t nine_vertices[] = {{{0U, 1U, 2U}}, {{0U, 3U, 6U}}, {{1U, 4U, 7U}},
                                            {{2U, 5U, 8U}}, {{3U, 4U, 5U}}, {{6U, 7U, 8U}}};
    const trifact_edge_t one_edge[] = {{{0U, 1U, 2U}}};
    trifact_edge_t universe[20] = {{{0U}}};
    trifact_edge_t selected[4] = {{{0U}}};
    trifact_vertex_t first = 0U;
    trifact_vertex_t second = 0U;
    trifact_vertex_t third = 0U;
    size_t count = 0U;
    size_t cases = 0U;
    size_t accepted = 0U;

    for (first = 0U; first < 6U; ++first) {
        for (second = first + 1U; second < 6U; ++second) {
            for (third = second + 1U; third < 6U; ++third) {
                universe[count].vertices[0] = first;
                universe[count].vertices[1] = second;
                universe[count].vertices[2] = third;
                ++count;
            }
        }
    }
    if (count != 20U || enumerate_graphs(universe, 0U, selected, 0U, &cases, &accepted) != 0 ||
        cases != 424996U || accepted != 90U) {
        (void)fprintf(stderr, "six-vertex enumeration failed: cases=%zu accepted=%zu\n", cases,
                      accepted);
        return EXIT_FAILURE;
    }
    cases = 0U;
    accepted = 0U;
    if (enumerate_labels(6U, degree_three, 6U, 3U, &cases, &accepted) != 0 || cases != 4096U ||
        accepted != 6U) {
        return EXIT_FAILURE;
    }
    cases = 0U;
    accepted = 0U;
    if (enumerate_labels(9U, nine_vertices, 6U, 2U, &cases, &accepted) != 0 || cases != 729U ||
        accepted != 2U || enumerate_labels(3U, one_edge, 1U, 2U, &cases, &accepted) != 0 ||
        enumerate_labels(7U, degree_three, 4U, 2U, &cases, &accepted) != 0 || accepted != 2U) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
