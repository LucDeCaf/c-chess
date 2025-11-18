#include "../board.h"
#include <stdio.h>

int main(void) {
    Board board;
    board_init(&board);
    printf("bitboards[0]: %" PRIu64 "\n", board.bitboards[0]);
    return 0;
}
