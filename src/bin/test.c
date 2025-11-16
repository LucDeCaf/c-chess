#include "../board.h"
#include "../fen.h"
#include "../move_gen.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handle_print(Board *);
void handle_dump(Board *);
void handle_moves(Board *);
void handle_move(Board *, char *);
void move_to_string(Move, char[6]);
Move move_from_string(char *);

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

        else if (strncmp(buf, "dump", 4) == 0) {
            handle_dump(&board);
        }

        else if (strncmp(buf, "moves", 5) == 0) {
            handle_moves(&board);
        }

        else if (strncmp(buf, "fen", 3) == 0) {
            char *fen = to_fen(&board);
            printf("%s\n", fen);
            free(fen);
        }

        else if (strncmp(buf, "load", 4) == 0) {
            if (buf[4] != ' ' || buf[5] == '\0') {
                printf("Expected FEN string.\n");
                continue;
            }
            char *fen = buf + 5;
            load_fen(&board, fen);
        }

        else if (strncmp(buf, "empty", 5) == 0) {
            for (int i = 0; i < 12; i++) {
                board.bitboards[i] = 0;
            }
        }

        else if (strncmp(buf, "move", 4) == 0) {
            if (strlen(buf) < 5) {
                printf("Expected move.");
                continue;
            }
            handle_move(&board, buf + 5);
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

void dump_bb(const char *piece, uint64_t bb) {
    printf("- %s:\n", piece);
    for (int i = 7; i >= 0; i--) {
        int pieces = bb >> (i * 8);
        // clang-format off
        printf("  %d %d %d %d %d %d %d %d\n",
            (pieces >> 0) & 1,
            (pieces >> 1) & 1,
            (pieces >> 2) & 1,
            (pieces >> 3) & 1,
            (pieces >> 4) & 1,
            (pieces >> 5) & 1,
            (pieces >> 6) & 1,
            (pieces >> 7) & 1
        );
        // clang-format on
    }
}

void handle_dump(Board *board) {
    printf("=== BOARD INFO ===\n");
    printf("** Metadata\n");
    printf("- Current turn: %s\n", board->current_turn ? "White" : "Black");
    printf("- Halfmoves: %d\n", board->halfmoves);
    printf("- Fullmoves: %d\n", board->fullmoves);
    printf("** Bitboards\n");
    dump_bb("Black Knights", board->bitboards[0]);
    dump_bb("Black Bishops", board->bitboards[1]);
    dump_bb("Black Rooks", board->bitboards[2]);
    dump_bb("Black Queens", board->bitboards[3]);
    dump_bb("Black King", board->bitboards[4]);
    dump_bb("Black Pawns", board->bitboards[5]);
    dump_bb("White Knights", board->bitboards[6]);
    dump_bb("White Bishops", board->bitboards[7]);
    dump_bb("White Rooks", board->bitboards[8]);
    dump_bb("White Queens", board->bitboards[9]);
    dump_bb("White King", board->bitboards[10]);
    dump_bb("White Pawns", board->bitboards[11]);
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
    printf("Total moves: %d\n", movecount);
}

void handle_move(Board *board, char *buf) {
    Move move = move_from_string(buf);
    char temp_buf[6];
    temp_buf[5] = '\0';
    move_to_string(move, temp_buf);
    board_make_move(board, move);
}

Move move_from_string(char *buf) {
    int len = strlen(buf);
    if (len != 4 && len != 5)
        return 0;

    Square source = (buf[0] - 'a') + (buf[1] - '1') * 8;
    if (source < 0 || source > 63)
        return 0;

    Square target = (buf[2] - 'a') + (buf[3] - '1') * 8;
    if (target < 0 || target > 63)
        return 0;

    Piece promotion = PieceNone;
    if (len == 5) {
        switch (buf[4]) {
        case 'n':
            promotion = PieceKnight;
            break;
        case 'b':
            promotion = PieceBishop;
            break;
        case 'r':
            promotion = PieceRook;
            break;
        case 'q':
            promotion = PieceQueen;
            break;
        }
    }

    uint16_t flags = (promotion == PieceNone) ? 0b0000 : (0b1000 | promotion);
    return new_move(source, target, flags);
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
