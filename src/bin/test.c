#include "../board.h"
#include "../fen.h"
#include "../move_gen.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handle_print(Board *);
void handle_dump(Board *);
void handle_moves(Board *);
void handle_move(Board *, char *);
void handle_set(Board *, char *);
void move_to_string(Move, char[6]);
Move move_from_string(char *);

int main(void) {
    Board board;
    board_init(&board);
    move_gen_init();

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

        else if (strncmp(buf, "clear", 5) == 0) {
            printf("\033[1;1H\033[2J");
        }

        else if (strncmp(buf, "reset", 5) == 0) {
            board_init(&board);
            printf("Reset board to starting position.\n");
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
            int error = load_fen(&board, fen, FEN_ERRORS_SAFE);
            if (error) {
                printf("Error loading FEN: Error code %d\n", error);
            }
        }

        else if (strncmp(buf, "empty", 5) == 0) {
            for (int i = 0; i < 12; i++) {
                board.bitboards[i] = 0;
            }
        }

        else if (strncmp(buf, "move", 4) == 0) {
            if (strlen(buf) < 5) {
                printf("Expected move.\n");
                continue;
            }
            handle_move(&board, buf + 5);
        }

        else if (strncmp(buf, "set", 3) == 0) {
            handle_set(&board, buf);
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
    uint8_t flags = board->flags;
    printf("- Castling rights: ");
    if (flags & FLAG_WHITE_KINGSIDE) printf("K");
    if (flags & FLAG_WHITE_QUEENSIDE) printf("Q");
    if (flags & FLAG_BLACK_KINGSIDE) printf("k");
    if (flags & FLAG_BLACK_QUEENSIDE) printf("q");
    if (!(flags & 0xf)) printf("-");
    printf("\n");
    printf("- En passant: ");
    if (flags & FLAG_CAN_EP)
        printf("%c%c", 'a' + (flags >> 5), board->current_turn ? '6' : '3');
    else
        printf("-");
    printf("\n");
    printf("- Pieces Array:\n");
    for (int i = 7; i >= 0; i--) {
        for (int j = 0; j < 8; j++) {
            int square = i * 8 + j;
            Piece piece = board->pieces[square] & 0xf;
            Color color = board->pieces[square] >> 4;
            if (piece == PieceNone)
                printf("- ");
            else if ((0 <= piece && piece <= 6) && (color == 0 || color == 1))
                printf("%c ", piece_char(piece, color));
            else
                printf("! ");
        }
        printf("\n");
    }
}

void handle_moves(Board *board) {
    Move moves[256];
    int movecount = move_gen_generate_legal_moves(board, moves);
    printf("Moves:\n");
    char buf[6];
    for (int i = 0; i < movecount; i++) {
        move_to_string(moves[i], buf);
        printf("%s\n", buf);
    }
    printf("Total moves: %d\n", movecount);
}

void handle_move(Board *board, char *buf) {
    Move moves[300];
    int movecount = move_gen_generate_legal_moves(board, moves);
    Move move = move_from_string(buf);

    for (int i = 0; i < movecount; i++) {
        // Check if moves have same source and target
        if ((moves[i] & 0xfff) == (move & 0xfff)) {
            // Replace move with genned move to get flags
            move = moves[i];
            break;
        }
    }

    board_make_move(board, move);
}

void handle_set(Board *board, char *buf) {
    char chars[2];
    int square, piece, color;
    if (sscanf(buf, "set %c%c %d %d", &chars[0], &chars[1], &piece, &color) !=
        4) {
        printf("Expected 'set <square> <piece> <color>'.\n");
        return;
    }

    square = (buf[4] - 'a') + (buf[5] - '1') * 8;
    if (square < 0 || square > 63) {
        printf("Invalid square (%d)\n", square);
        return;
    }

    printf("square: %d\npiece: %d\ncolor: %d\n", square, piece, color);

    if (board->pieces[square] != PieceNone) board_clear_piece(board, square);
    board_add_piece(board, square, piece, color);
}

Move move_from_string(char *buf) {
    int len = strlen(buf);
    if (len != 4 && len != 5) return 0;

    int source = (buf[0] - 'a') + (buf[1] - '1') * 8;
    if (source < 0 || source > 63) return 0;

    int target = (buf[2] - 'a') + (buf[3] - '1') * 8;
    if (target < 0 || target > 63) return 0;

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

    uint8_t flags = (promotion == PieceNone) ? 0 : (MOVE_PROMOTION | promotion);
    return new_move(source, target, flags);
}

void move_to_string(Move move, char out[6]) {
    // Check promotion bit
    char promotion_chars[4] = "nbrq";
    int s = move_source(move), t = move_target(move);
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
