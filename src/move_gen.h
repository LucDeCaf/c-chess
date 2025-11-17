#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include "board.h"

int move_gen_init();
void move_gen_cleanup();
int generate_moves(Board *board, Move *moves);

#endif // MOVE_GEN_H
