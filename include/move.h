#ifndef MOVE_H
#define MOVE_H

#include "square.h"
#include <stdint.h>

typedef uint16_t Move;

Move new_move(Square source, Square target, uint8_t flags);
Square move_source(Move);
Square move_target(Move);
uint8_t move_flags(Move);

#endif // MOVE_H
