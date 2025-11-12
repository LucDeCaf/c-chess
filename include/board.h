#ifndef BOARD_H
#define BOARD_H

#include "bitboard.h"
#include "color.h"
#include "move.h"
#include "piece.h"

typedef struct {
    U64 bitboards[12];
    Color current_turn;
    uint8_t flags;      // en passant, castling rights
    uint8_t halfmoves;  // 75 move rule
    uint16_t fullmoves; // Longest possible game = ~8800 fullmoves
} Board;

void board_init(Board *);
void board_make_move(Board *, Move);
Piece board_piece_at(Board *, Square);

#endif // BOARD_H
