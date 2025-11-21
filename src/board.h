#ifndef BOARD_H
#define BOARD_H

#include "color.h"
#include "move.h"
#include "piece.h"

typedef struct {
    uint64_t bitboards[12];
    Color current_turn;
    uint8_t flags;      // en passant, castling rights
    uint8_t halfmoves;  // 75 move rule
    uint16_t fullmoves; // Longest possible game = ~8800 fullmoves
    Piece pieces[64];   // Saves on expensive for-loops for finding pieces
} Board;

// clang-format off
#define FLAG_BLACK_KINGSIDE  1      // 0b0000_0001
#define FLAG_BLACK_QUEENSIDE 2      // 0b0000_0010
#define FLAG_WHITE_KINGSIDE  4      // 0b0000_0100
#define FLAG_WHITE_QUEENSIDE 8      // 0b0000_1000
#define FLAG_BLACK_CASTLE    3      // 0b0000_0011
#define FLAG_WHITE_CASTLE    12     // 0b0000_1100
#define FLAG_CAN_EP          16     // 0b0001_0000
#define FLAG_EP_FILE         7 << 5 // 0b1110_0000
// clang-format on

void board_init(Board *);
void board_make_move(Board *, Move);
Piece board_piece_at(Board *, int);
void board_add_piece(Board *, int, Piece, Color);
void board_clear_piece(Board *, int);
uint64_t board_bitboard(Board *, Piece, Color);
uint64_t *board_bitboard_p(Board *, Piece, Color);
uint64_t board_pieces(Board *, Color);
int board_in_check(Board *, Color);
int board_square_attacked_by(Board *, int, Color);

#endif // BOARD_H
