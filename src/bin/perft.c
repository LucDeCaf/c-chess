#include "../board.h"
#include "../fen.h"
#include "../move_gen.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT(C)                                                                \
    EXITCODE = C;                                                              \
    goto exit;

typedef struct {
    int count;
    int captures;
    int en_passants;
    int castles;
    int promotions;
    int checks;
    int checkmates;
    // int discovered_checks;
    // int double_checks;
} PerftResult;

int EXITCODE = 0;

void scan_moves(Move *moves, size_t moves_len, PerftResult *results) {
    for (size_t i = 0; i < moves_len; i++) {
        int flags = move_flags(moves[i]);
        if (flags & MOVE_CAPTURE) results->captures++;
        if (flags == MOVE_EN_PASSANT) results->en_passants++;
        if (flags == MOVE_KINGSIDE || flags == MOVE_QUEENSIDE)
            results->castles++;
        if (flags & MOVE_PROMOTION) results->promotions++;
    }
    results->count += moves_len;
}

uint64_t perft(Board *board, int depth, PerftResult *results) {
    if (depth == 0) return 1;

    Move moves[300];
    int movecount = move_gen_generate_legal_moves(board, moves);

    // Read most of the data in
    if (depth == 1) scan_moves(moves, movecount, results);

    Board new_board;
    uint64_t count = 0;
    for (int i = 0; i < movecount; i++) {
        new_board = *board;
        board_make_move(&new_board, moves[i]);
        int moves_found = perft(&new_board, depth - 1, results);
        if (moves_found == 0) results->checkmates++;
        if (depth == 1 && board_in_check(&new_board, new_board.current_turn))
            results->checks++;
        count += moves_found;
    }

    return count;
}

void print_usage(const char *program) {
    printf("Usage: %s [fen] depth\n", program);
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

    if (argc == 2) {
        depth = atoi(argv[1]);
    } else if (argc == 3) {
        fen = argv[1];
        depth = atoi(argv[2]);
    } else {
        print_usage(argv[0]);
        EXIT(1);
    }

    if (depth <= 0) {
        printf("Invalid value for depth '%s'\n", argv[1]);
        EXIT(1);
    }

    // -5 == no halfmoves/fullmoves, which is irrelevant for perft
    int error = load_fen(&board, fen, FEN_ERRORS_IGNORE);
    if (error && error != -5) {
        printf("Invalid fen: ");
        switch (error) {
        case -1:
            printf("Missing or invalid board string.\n");
            break;
        case -2:
            printf("Missing or invalid color.\n");
            break;
        case -3:
            printf("Missing or invalid castling rights.\n");
            break;
        case -4:
            printf("Missing or invalid en passant.\n");
            break;
        default:
            printf("Unknown error.\n");
        }
        EXIT(1);
    }

    PerftResult results = {0};
    perft(&board, depth, &results);
    printf("Nodes:      %d\n", results.count);
    printf("Captures:   %d\n", results.captures);
    printf("E.p.:       %d\n", results.en_passants);
    printf("Castles:    %d\n", results.castles);
    printf("Promotions: %d\n", results.promotions);
    printf("Checks:     %d\n", results.checks);
    printf("Checkmates: %d\n", results.checkmates);

exit:
    move_gen_cleanup();
    return EXITCODE;
}
