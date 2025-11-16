#ifndef FEN_H
#define FEN_H

#include "board.h"

int load_fen(Board *, char *);
char *to_fen(Board *);

#endif // FEN_H
