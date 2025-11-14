#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include "board.h"

void init_move_generation();
int generate_moves(Board *board, Move *moves);

#endif // MOVE_GEN_H
