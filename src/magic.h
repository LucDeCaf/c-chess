#ifndef MAGIC_H
#define MAGIC_H

#include "square.h"
#include <stdint.h>

typedef struct {
    uint64_t *att;
    uint64_t mask;
    uint64_t magic;
    uint8_t shift;
} MagicEntry;

int magic_init(void);
void magic_cleanup(void);
uint64_t magic_rook_moves(Square square, uint64_t blockers);
uint64_t magic_bishop_moves(Square square, uint64_t blockers);

#endif // MAGIC_H
