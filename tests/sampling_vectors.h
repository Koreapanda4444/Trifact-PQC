#ifndef TRIFACT_TEST_SAMPLING_VECTORS_H
#define TRIFACT_TEST_SAMPLING_VECTORS_H

#include "trifact/core.h"

typedef struct {
    const char *parameter_id;
    trifact_vertex_t vertex_count;
    trifact_vertex_t permutation[60];
    uint32_t uniform_results[7];
} sampling_kat_t;

static const sampling_kat_t sampling_kats[] = {
    {"toy",
     12U,
     {10U, 0U, 4U, 5U, 11U, 7U, 9U, 3U, 1U, 2U, 8U, 6U},
     {UINT32_C(0), UINT32_C(0), UINT32_C(2), UINT32_C(4), UINT32_C(3), UINT32_C(228636973),
      UINT32_C(4254371635)}},
    {"small",
     24U,
     {8U,  9U,  14U, 4U,  0U,  5U,  12U, 2U,  1U,  23U, 6U, 22U,
      19U, 15U, 11U, 21U, 20U, 10U, 13U, 17U, 16U, 7U,  3U, 18U},
     {UINT32_C(0), UINT32_C(1), UINT32_C(1), UINT32_C(3), UINT32_C(6), UINT32_C(2108020589),
      UINT32_C(3443618921)}},
    {"medium",
     60U,
     {4U,  13U, 23U, 46U, 27U, 32U, 55U, 49U, 18U, 57U, 5U,  36U, 6U,  15U, 51U,
      8U,  12U, 42U, 56U, 54U, 10U, 26U, 59U, 33U, 53U, 22U, 19U, 41U, 39U, 40U,
      45U, 29U, 9U,  58U, 7U,  52U, 50U, 20U, 35U, 16U, 17U, 38U, 21U, 47U, 48U,
      28U, 31U, 34U, 14U, 2U,  24U, 25U, 11U, 43U, 30U, 0U,  44U, 3U,  37U, 1U},
     {UINT32_C(0), UINT32_C(0), UINT32_C(2), UINT32_C(4), UINT32_C(6), UINT32_C(3096302774),
      UINT32_C(141497372)}}};

#endif
