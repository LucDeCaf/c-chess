#include "../board.h"
#include "../move_gen.h"

#include <stdio.h>
#include <string.h>

void handle_print(Board *);
void handle_moves(Board *);
void move_to_string(Move, char[6]);

int main() {
    Board board;
    board_init(&board);
    init_move_generation();

    char buf[200];
    while (1) {
        // Get command
        printf("> ");
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            break;
        }
        // Strip newline
        int len = strlen(buf) - 1;
        buf[len] = '\0';

        // -- COMMANDS
        if (strncmp(buf, "exit", 4) == 0) {
            break;
        }

        else if (strncmp(buf, "clear", 4) == 0) {
            printf("\e[1;1H\e[2J");
        }

        else if (strncmp(buf, "print", 5) == 0) {
            handle_print(&board);
        }

        else if (strncmp(buf, "moves", 5) == 0) {
            handle_moves(&board);
        }

        else if (strncmp(buf, "swapturn", 8) == 0) {
            board.current_turn = color_inverse(board.current_turn);
        }

        else {
            printf("Invalid command: '%s'\n", buf);
        }
    }

    printf("End of input\n");

    return 0;
}

char piece_char(Piece piece, Color color) {
    static const char PIECE_CHARS[12] = {
        'n', 'b', 'r', 'q', 'k', 'p', 'N', 'B', 'R', 'Q', 'K', 'P',
    };

    return PIECE_CHARS[piece + (color * 6)];
}

void handle_print(Board *board) {
    char chars[8][8] = {
        {'-', '-', '-', '-', '-', '-', '-', '-'},
        {'-', '-', '-', '-', '-', '-', '-', '-'},
        {'-', '-', '-', '-', '-', '-', '-', '-'},
        {'-', '-', '-', '-', '-', '-', '-', '-'},
        {'-', '-', '-', '-', '-', '-', '-', '-'},
        {'-', '-', '-', '-', '-', '-', '-', '-'},
        {'-', '-', '-', '-', '-', '-', '-', '-'},
        {'-', '-', '-', '-', '-', '-', '-', '-'},
    };

    for (int i = 0; i < 12; i++) {
        Piece piece = i > 5 ? i - 6 : i;
        Color color = i > 5;
        uint64_t bb = board->bitboards[i];

        int sq = 0;
        while (bb) {
            if (bb & 1) {
                // Piece found
                int rank = square_rank(sq);
                int file = square_file(sq);
                chars[rank][file] = piece_char(piece, color);
            }

            bb >>= 1;
            sq++;
        }
    }

    for (int i = 7; i >= 0; i--) {
        // clang-format off
        printf(
            "%d %c %c %c %c %c %c %c %c\n",
            i + 1,
            chars[i][0],
            chars[i][1],
            chars[i][2],
            chars[i][3],
            chars[i][4],
            chars[i][5],
            chars[i][6],
            chars[i][7]
        );
        // clang-format on
    }
    printf("  A B C D E F G H\n");
}

void handle_moves(Board *board) {
    Move moves[256];
    int movecount = generate_moves(board, moves);
    printf("Moves:\n");
    char buf[6];
    for (int i = 0; i < movecount; i++) {
        move_to_string(moves[i], buf);
        printf("%s\n", buf);
    }
}

void move_to_string(Move move, char out[6]) {
    // Check promotion bit
    char promotion_chars[4] = "nbrq";
    Square s = move_source(move), t = move_target(move);
    int f = move_flags(move);
    int sr, sf, tr, tf;
    sr = square_rank(s);
    sf = square_file(s);
    tr = square_rank(t);
    tf = square_file(t);
    int is_prom = f >> 3;
    out[0] = 'a' + sf;
    out[1] = '1' + sr;
    out[2] = 'a' + tf;
    out[3] = '1' + tr;
    out[4] = (promotion_chars[f & MOVE_SPECIAL] * is_prom);
    out[5] = 0;
}
