#include <stdint.h>
#include <stdlib.h>

uint64_t random_u64() {
    uint64_t a, b, c, d;
    a = (uint64_t)rand() & 0xFFFF;
    b = (uint64_t)rand() & 0xFFFF;
    c = (uint64_t)rand() & 0xFFFF;
    d = (uint64_t)rand() & 0xFFFF;

    return a | (b << 16) | (c << 32) | (d << 48);
}

int popcount(uint64_t mask) {
    int n = 0;
    while (mask) {
        n++;
        mask &= mask - 1;
    }
    return n;
}

uint64_t rook_mask(int square) {
    uint64_t result = 0;
    int rank = square / 8;
    int file = square % 8;

    int r, f;
    for (r = rank + 1; r <= 6; r++)
        result |= (1ull << (file + r * 8));
    for (r = rank - 1; r >= 1; r--)
        result |= (1ull << (file + r * 8));
    for (f = file + 1; f <= 6; f++)
        result |= (1ull << (f + rank * 8));
    for (f = file - 1; f >= 1; f--)
        result |= (1ull << (f + rank * 8));
    return result;
}

uint64_t bishop_mask(int square) {
    uint64_t result = 0;
    int rank = square / 8;
    int file = square % 8;

    int r, f;
    for (r = rank + 1, f = file + 1; r <= 6 && f <= 6; r++, f++)
        result |= (1ull << (f + r * 8));
    for (r = rank + 1, f = file - 1; r <= 6 && f >= 1; r++, f--)
        result |= (1ull << (f + r * 8));
    for (r = rank - 1, f = file + 1; r >= 1 && f <= 6; r--, f++)
        result |= (1ull << (f + r * 8));
    for (r = rank - 1, f = file - 1; r >= 1 && f >= 1; r--, f--)
        result |= (1ull << (f + r * 8));
    return result;
}

uint64_t rook_attacks(int square, uint64_t block) {
    uint64_t result = 0;
    int rank = square / 8;
    int file = square % 8;

    int r, f;
    for (r = rank + 1; r <= 7; r++) {
        result |= (1ull << (file + r * 8));
        if (block & (1ull << (file + r * 8)))
            break;
    }
    for (r = rank - 1; r >= 0; r--) {
        result |= (1ull << (file + r * 8));
        if (block & (1ull << (file + r * 8)))
            break;
    }
    for (f = file + 1; f <= 7; f++) {
        result |= (1ull << (f + rank * 8));
        if (block & (1ull << (f + rank * 8)))
            break;
    }
    for (f = file - 1; f >= 0; f--) {
        result |= (1ull << (f + rank * 8));
        if (block & (1ull << (f + rank * 8)))
            break;
    }
    return result;
}

uint64_t bishop_attacks(int square, uint64_t block) {
    uint64_t result = 0;
    int rank = square / 8;
    int file = square % 8;

    int r, f;

    for (r = rank + 1, f = file + 1; r <= 7 && f <= 7; r++, f++) {
        result |= (1ull << (f + r * 8));
        if (block & (1ull << (f + r * 8)))
            break;
    }
    for (r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f--) {
        result |= (1ull << (f + r * 8));
        if (block & (1ull << (f + r * 8)))
            break;
    }
    for (r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++) {
        result |= (1ull << (f + r * 8));
        if (block & (1ull << (f + r * 8)))
            break;
    }
    for (r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
        result |= (1ull << (f + r * 8));
        if (block & (1ull << (f + r * 8)))
            break;
    }

    return result;
}

int magic_index(int square, uint64_t blockers) {
}
