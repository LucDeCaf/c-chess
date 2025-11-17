#ifndef MOVE_H
#define MOVE_H

#include "square.h"
#include <inttypes.h>

// Move struct copied from https://www.chessprogramming.org/Encoding_Moves
typedef uint16_t Move;

// clang-format off
#define MOVE_PROMOTION   8 // 0b1000
#define MOVE_CAPTURE     4 // 0b0100
#define MOVE_SPECIAL     3 // 0b0011
#define MOVE_QUIET       0 // 0b0000
#define MOVE_DOUBLE_PUSH 1 // 0b0001
// clang-format on

Move new_move(int source, int target, uint8_t flags);
int move_source(Move);
int move_target(Move);
uint8_t move_flags(Move);

#endif // MOVE_H
