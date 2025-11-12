#include "../../include/board.h"
#include <stdio.h>

int main() {
    Board board;
    board_init(&board);
    printf("bitboards[0]: %ld\n", board.bitboards[0]);
    return 0;
}
