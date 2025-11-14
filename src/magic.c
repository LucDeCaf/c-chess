#include "magic.h"
#include "gen/magics.c"
#include "square.h"
#include <stdint.h>
#include <sys/types.h>

uint64_t ROOK_MOVES[64][4096];
uint64_t BISHOP_MOVES[64][512];

int magic_index(MagicEntry *entry, uint64_t blockers) {
    return ((blockers & entry->mask) * entry->magic) >> entry->shift;
}

uint64_t magic_rook_moves(Square square, uint64_t blockers) {
    MagicEntry magic = ROOK_MAGICS[square];
    uint64_t *moves = ROOK_MOVES[square];
    return moves[magic_index(&magic, blockers)];
}

uint64_t magic_bishop_moves(Square square, uint64_t blockers) {
    MagicEntry magic = BISHOP_MAGICS[square];
    uint64_t *moves = BISHOP_MOVES[square];
    return moves[magic_index(&magic, blockers)];
}

uint64_t rook_attacks(Square square, uint64_t blockers) {
    uint64_t result = 0ULL;
    int rk = square / 8, fl = square % 8, r, f;
    for (r = rk + 1; r <= 7; r++) {
        result |= (1ULL << (fl + r * 8));
        if (blockers & (1ULL << (fl + r * 8)))
            break;
    }
    for (r = rk - 1; r >= 0; r--) {
        result |= (1ULL << (fl + r * 8));
        if (blockers & (1ULL << (fl + r * 8)))
            break;
    }
    for (f = fl + 1; f <= 7; f++) {
        result |= (1ULL << (f + rk * 8));
        if (blockers & (1ULL << (f + rk * 8)))
            break;
    }
    for (f = fl - 1; f >= 0; f--) {
        result |= (1ULL << (f + rk * 8));
        if (blockers & (1ULL << (f + rk * 8)))
            break;
    }
    return result;
}

uint64_t bishop_attacks(Square square, uint64_t blockers) {
    uint64_t result = 0ULL;
    int rk = square / 8, fl = square % 8, r, f;
    for (r = rk + 1, f = fl + 1; r <= 7 && f <= 7; r++, f++) {
        result |= (1ULL << (f + r * 8));
        if (blockers & (1ULL << (f + r * 8)))
            break;
    }
    for (r = rk + 1, f = fl - 1; r <= 7 && f >= 0; r++, f--) {
        result |= (1ULL << (f + r * 8));
        if (blockers & (1ULL << (f + r * 8)))
            break;
    }
    for (r = rk - 1, f = fl + 1; r >= 0 && f <= 7; r--, f++) {
        result |= (1ULL << (f + r * 8));
        if (blockers & (1ULL << (f + r * 8)))
            break;
    }
    for (r = rk - 1, f = fl - 1; r >= 0 && f >= 0; r--, f--) {
        result |= (1ULL << (f + r * 8));
        if (blockers & (1ULL << (f + r * 8)))
            break;
    }
    return result;
}

void fill_table(uint64_t *piece_tables[64], MagicEntry magics[64],
                uint64_t (*get_moves)(Square, uint64_t)) {
    for (Square square = 0; square < 64; square++) {
        MagicEntry *entry = &magics[square];
        uint64_t mask = entry->mask;
        uint64_t *table = piece_tables[square];

        uint64_t blockers = 0;
        do {
            uint64_t moves = get_moves(square, blockers);
            int i = magic_index(entry, blockers);
            table[i] = moves;

            blockers = (blockers - mask) & mask;
        } while (blockers);
    }
}

void magic_init() {
    fill_table(ROOK_MOVES, ROOK_MAGICS, rook_attacks);
    fill_table(BISHOP_MOVES, BISHOP_MAGICS, bishop_attacks);
}
