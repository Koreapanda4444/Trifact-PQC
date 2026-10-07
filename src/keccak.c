#include "keccak.h"
#include "secure.h"

static uint64_t rotate_left(uint64_t value, unsigned int shift) {
    return shift == 0U ? value : (value << shift) | (value >> (64U - shift));
}

void trifact_keccak_permute(uint64_t state[25]) {
    static const uint64_t constants[24] = {
        UINT64_C(0x0000000000000001), UINT64_C(0x0000000000008082), UINT64_C(0x800000000000808a),
        UINT64_C(0x8000000080008000), UINT64_C(0x000000000000808b), UINT64_C(0x0000000080000001),
        UINT64_C(0x8000000080008081), UINT64_C(0x8000000000008009), UINT64_C(0x000000000000008a),
        UINT64_C(0x0000000000000088), UINT64_C(0x0000000080008009), UINT64_C(0x000000008000000a),
        UINT64_C(0x000000008000808b), UINT64_C(0x800000000000008b), UINT64_C(0x8000000000008089),
        UINT64_C(0x8000000000008003), UINT64_C(0x8000000000008002), UINT64_C(0x8000000000000080),
        UINT64_C(0x000000000000800a), UINT64_C(0x800000008000000a), UINT64_C(0x8000000080008081),
        UINT64_C(0x8000000000008080), UINT64_C(0x0000000080000001), UINT64_C(0x8000000080008008)};
    uint64_t columns[5] = {0U};
    uint64_t moved[25] = {0U};
    unsigned int round = 0U;
    unsigned int x = 0U;
    unsigned int y = 0U;

    for (round = 0U; round < 24U; ++round) {
        for (x = 0U; x < 5U; ++x) {
            columns[x] =
                state[x] ^ state[x + 5U] ^ state[x + 10U] ^ state[x + 15U] ^ state[x + 20U];
        }
        for (x = 0U; x < 5U; ++x) {
            const uint64_t delta = columns[(x + 4U) % 5U] ^ rotate_left(columns[(x + 1U) % 5U], 1U);

            for (y = 0U; y < 5U; ++y) {
                state[x + 5U * y] ^= delta;
            }
        }
        moved[0] = state[0];
        x = 1U;
        y = 0U;
        for (unsigned int step = 0U; step < 24U; ++step) {
            const unsigned int next_x = y;
            const unsigned int next_y = (2U * x + 3U * y) % 5U;
            const unsigned int rotation = ((step + 1U) * (step + 2U) / 2U) % 64U;

            moved[next_x + 5U * next_y] = rotate_left(state[x + 5U * y], rotation);
            x = next_x;
            y = next_y;
        }
        for (y = 0U; y < 5U; ++y) {
            for (x = 0U; x < 5U; ++x) {
                state[x + 5U * y] = moved[x + 5U * y] ^ (~moved[(x + 1U) % 5U + 5U * y] &
                                                         moved[(x + 2U) % 5U + 5U * y]);
            }
        }
        state[0] ^= constants[round];
    }
    trifact_secure_clear(columns, sizeof(columns));
    trifact_secure_clear(moved, sizeof(moved));
}
