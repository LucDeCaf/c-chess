#include "../board.h"
#include "../move_gen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT(C)                                                                \
    EXITCODE = C;                                                              \
    goto exit;

int EXITCODE = 0;

int perft(Board *board, int depth) {
    if (depth == 0) return 1;

    Move moves[300];
    int movecount = generate_moves(board, moves);
    if (depth == 1) return movecount;

    Board new_board;
    int count = 0;
    for (int i = 0; i < movecount; i++) {
        new_board = *board;
        board_make_move(&new_board, moves[i]);
        count += perft(&new_board, depth - 1);
    }

    return count;
}

void print_usage(const char *program) {
    printf("Usage: %s depth\n", program);
}

int main(int argc, char *argv[]) {
    Board board;
    int depth;
    char *fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    board_init(&board);
    move_gen_init();

    if (argc < 2) {
        print_usage(argv[0]);
        EXIT(1);
    }

    if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0 ||
        strcmp(argv[1], "-h") == 0) {
        print_usage(argv[0]);
        EXIT(0);
    }

    depth = atoi(argv[1]);
    if (depth <= 0) {
        printf("Invalid value for depth '%s'\n", argv[1]);
        EXIT(1);
    }

    int count = perft(&board, depth);
    printf("Perft result: %d\n", count);

exit:
    move_gen_cleanup();
    return EXITCODE;
}
