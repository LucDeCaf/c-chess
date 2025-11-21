#ifndef FEN_H
#define FEN_H

#include "board.h"

// clang-format off
#define FEN_E_BOARD_STR  -1
#define FEN_E_COLOR      -2
#define FEN_E_CASTLING   -3
#define FEN_E_EN_PASSANT -4
#define FEN_E_MOVES      -5
#define FEN_ERRORS_SAFE   0
#define FEN_ERRORS_IGNORE 1
// clang-format on

int load_fen(Board *, char *, int);
char *to_fen(Board *);

#endif // FEN_H
