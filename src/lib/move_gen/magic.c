#include "../../../include/chess/internal/square.h"

typedef struct {
    uint64_t mask;
    uint64_t magic;
    uint8_t shift;
} MagicEntry;

MagicEntry ROOK_MAGICS[64];
MagicEntry BISHOP_MAGICS[64];
uint64_t *ROOK_MOVES[64];
uint64_t *BISHOP_MOVES[64];

int magic_index(MagicEntry *entry, uint64_t blockers) {
    return ((blockers & entry->mask) * entry->magic) >> entry->shift;
}

uint64_t rook_moves(Square square, uint64_t blockers) {
    MagicEntry magic = ROOK_MAGICS[square];
    uint64_t *moves = ROOK_MOVES[square];
    return moves[magic_index(&magic, blockers)];
}

uint64_t bishop_moves(Square square, uint64_t blockers) {
    MagicEntry magic = BISHOP_MAGICS[square];
    uint64_t *moves = BISHOP_MOVES[square];
    return moves[magic_index(&magic, blockers)];
}
