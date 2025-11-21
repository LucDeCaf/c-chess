#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include "board.h"
#include <inttypes.h>
#include <stdint.h>

int move_gen_init(void);
void move_gen_cleanup(void);
uint64_t move_gen_bishop_moves(int square, uint64_t blockers);
uint64_t move_gen_rook_moves(int square, uint64_t blockers);
int move_gen_generate_moves(Board *board, Move *moves);
int move_gen_generate_legal_moves(Board *board, Move *moves);
int move_gen_is_legal_move(Board *board, Move move);
int ctz_ll(uint64_t);

#endif // MOVE_GEN_H
