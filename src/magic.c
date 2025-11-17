#include "magic.h"
#include "gen/magics.h"
#include "square.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int magic_index(const MagicEntry *entry, uint64_t blockers) {
    return ((blockers & entry->mask) * entry->magic) >> entry->shift;
}

uint64_t magic_rook_moves(Square square, uint64_t blockers) {
    MagicEntry magic = ROOK_MAGICS[square];
    uint64_t *moves = magic.att;
    return moves[magic_index(&magic, blockers)];
}

uint64_t magic_bishop_moves(Square square, uint64_t blockers) {
    MagicEntry magic = BISHOP_MAGICS[square];
    uint64_t *moves = magic.att;
    return moves[magic_index(&magic, blockers)];
}

uint64_t rook_moves(int square, uint64_t blockers) {
    uint64_t result = 0ULL;
    int r, f;
    int rank = square / 8;
    int file = square % 8;

    for (r = rank + 1; r < 8; r++) {
        result |= 1ULL << (r * 8 + file);
        if (blockers & (1ULL << (r * 8 + file)))
            break;
    }
    for (r = rank - 1; r >= 0; r--) {
        result |= 1ULL << (r * 8 + file);
        if (blockers & (1ULL << (r * 8 + file)))
            break;
    }
    for (f = file + 1; f < 8; f++) {
        result |= 1ULL << (rank * 8 + f);
        if (blockers & (1ULL << (rank * 8 + f)))
            break;
    }
    for (f = file - 1; f >= 0; f--) {
        result |= 1ULL << (rank * 8 + f);
        if (blockers & (1ULL << (rank * 8 + f)))
            break;
    }

    return result;
}

uint64_t bishop_moves(int square, uint64_t blockers) {
    uint64_t result = 0ULL;
    int r, f;
    int rank = square / 8;
    int file = square % 8;

    for (r = rank + 1, f = file + 1; r < 8 && f < 8; r++, f++) {
        result |= 1ULL << (r * 8 + f);
        if (blockers & (1ULL << (r * 8 + f)))
            break;
    }
    for (r = rank - 1, f = file + 1; r >= 0 && f < 8; r--, f++) {
        result |= 1ULL << (r * 8 + f);
        if (blockers & (1ULL << (r * 8 + f)))
            break;
    }
    for (r = rank + 1, f = file - 1; r < 8 && f >= 0; r++, f--) {
        result |= 1ULL << (r * 8 + f);
        if (blockers & (1ULL << (r * 8 + f)))
            break;
    }
    for (r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
        result |= 1ULL << (r * 8 + f);
        if (blockers & (1ULL << (r * 8 + f)))
            break;
    }

    return result;
}

int fill_table(int square, MagicEntry *magic, int bishop) {
    int table_len = 1 << (64 - magic->shift);
    uint64_t *table = malloc(sizeof(uint64_t) * table_len);
    if (table == NULL)
        return -1;

    for (int i = 0; i < table_len; i++) {
        table[i] = 0ULL;
    }

    int i;
    uint64_t moves;
    uint64_t mask = magic->mask;
    uint64_t blockers = 0ULL;
    do {
        moves = bishop ? bishop_moves(square, blockers)
                       : rook_moves(square, blockers);
        i = magic_index(magic, blockers);
        table[i] = moves;

        blockers = (blockers - mask) & mask;
    } while (blockers);

    magic->att = table;

    return 0;
}

int magic_init(void) {
    int i, error;
    for (i = 0; i < 64; i++) {
        error = fill_table(i, &ROOK_MAGICS[i], 0);
        if (error)
            return error;
        error = fill_table(i, &BISHOP_MAGICS[i], 1);
        if (error)
            return error;
    }
    return 0;
}

void magic_cleanup(void) {
    for (int i = 0; i < 64; i++) {
        if (ROOK_MAGICS[i].att != NULL) {
            free(ROOK_MAGICS[i].att);
            ROOK_MAGICS[i].att = NULL;
        }
        if (BISHOP_MAGICS[i].att != NULL) {
            free(BISHOP_MAGICS[i].att);
            BISHOP_MAGICS[i].att = NULL;
        }
    }
}
