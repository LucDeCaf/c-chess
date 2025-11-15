#ifndef MOVE_H
#define MOVE_H

#include "square.h"
#include <stdint.h>

// Move struct copied from https://www.chessprogramming.org/Encoding_Moves
typedef uint16_t Move;

// clang-format off
#define MOVE_PROMOTION 0b1000
#define MOVE_CAPTURE   0b0100
#define MOVE_SPECIAL   0b0011
// clang-format on

Move new_move(Square source, Square target, uint8_t flags);
Square move_source(Move);
Square move_target(Move);
uint8_t move_flags(Move);

#endif // MOVE_H
