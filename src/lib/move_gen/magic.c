#include "../../../include/chess/internal/magic.h"
#include "../../../include/chess/internal/square.h"
#include "../gen/magics.c"
#include <stdint.h>
#include <sys/types.h>

uint64_t ROOK_MOVES[64][4096];
uint64_t BISHOP_MOVES[64][512];

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

// TODO: Copy over rook and bishop move functions from: src/bin/magic_gen.c
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
