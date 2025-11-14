#include "../../include/chess/internal/board.h"

#include <stdio.h>
#include <string.h>

void handle_print(Board *);
void handle_move(Board *, const char *);

int main() {
    Board board;
    board_init(&board);

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

        if (strncmp(buf, "clear", 4) == 0) {
            printf("\e[1;1H\e[2J");
            continue;
        }

        if (strncmp(buf, "print", 5) == 0) {
            handle_print(&board);
            continue;
        }

        if (strncmp(buf, "move", 4) == 0) {
            handle_move(&board, buf);
            continue;
        }

        printf("Invalid command: '%s'\n", buf);
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

void handle_move(Board *board, const char *buf) {
    printf("TODO: Get move list, if can't find move in move list then get "
           "move flags from user\n");
}
