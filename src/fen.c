#include "fen.h"
#include "board.h"
#include "color.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char PIECE_CHARS[12] = {'n', 'b', 'r', 'q', 'k', 'p',
                              'N', 'B', 'R', 'Q', 'K', 'P'};

int load_fen(Board *board, char *fen, int write_on_error) {
    Board new_board;

    int len = strlen(fen);
    int rank = 7, file = 0;
    int i;

    // Clear new board
    for (i = 0; i < 12; i++) {
        new_board.bitboards[i] = 0;
    }
    new_board.flags = 0;
    new_board.current_turn = White;
    new_board.halfmoves = 0;
    new_board.fullmoves = 1;

    for (i = 0; i < len; i++) {
        int square = rank * 8 + file;
        if (square < 0 || square > 63) {
            if (write_on_error) *board = new_board;
            return FEN_E_BOARD_STR;
        }

        if (fen[i] == ' ') {
            if (write_on_error) *board = new_board;
            return FEN_E_BOARD_STR;
        }

        int skip;
        switch (fen[i]) {
        case 'n':
            board_add_piece(&new_board, square, PieceKnight, Black);
            file++;
            break;
        case 'b':
            board_add_piece(&new_board, square, PieceBishop, Black);
            file++;
            break;
        case 'r':
            board_add_piece(&new_board, square, PieceRook, Black);
            file++;
            break;
        case 'q':
            board_add_piece(&new_board, square, PieceQueen, Black);
            file++;
            break;
        case 'k':
            board_add_piece(&new_board, square, PieceKing, Black);
            file++;
            break;
        case 'p':
            board_add_piece(&new_board, square, PiecePawn, Black);
            file++;
            break;
        case 'N':
            board_add_piece(&new_board, square, PieceKnight, White);
            file++;
            break;
        case 'B':
            board_add_piece(&new_board, square, PieceBishop, White);
            file++;
            break;
        case 'R':
            board_add_piece(&new_board, square, PieceRook, White);
            file++;
            break;
        case 'Q':
            board_add_piece(&new_board, square, PieceQueen, White);
            file++;
            break;
        case 'K':
            board_add_piece(&new_board, square, PieceKing, White);
            file++;
            break;
        case 'P':
            board_add_piece(&new_board, square, PiecePawn, White);
            file++;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
            skip = fen[i] - '0';
            file += skip;
            break;
        case '/':
            break;
        default:
            if (write_on_error) *board = new_board;
            return FEN_E_BOARD_STR;
        }

        if (file > 7) {
            file = 0;
            rank--;
        }

        if (rank < 0) break;
    }

    // Skip piece and following space
    i += 2;
    if (i >= len) {
        if (write_on_error) *board = new_board;
        return FEN_E_COLOR;
    }

    switch (fen[i++]) {
    case 'w':
        new_board.current_turn = White;
        break;
    case 'b':
        new_board.current_turn = Black;
        break;
    default:
        if (write_on_error) *board = new_board;
        return FEN_E_COLOR;
    }

    // Castling rights
    new_board.flags = 0;
    i++;
    while (fen[i] != ' ' && i < len)
        switch (fen[i++]) {
        case 'K':
            new_board.flags |= FLAG_WHITE_KINGSIDE;
            break;
        case 'Q':
            new_board.flags |= FLAG_WHITE_QUEENSIDE;
            break;
        case 'k':
            new_board.flags |= FLAG_BLACK_KINGSIDE;
            break;
        case 'q':
            new_board.flags |= FLAG_BLACK_QUEENSIDE;
            break;
        case '-':
            break;
        default:
            if (write_on_error) *board = new_board;
            return FEN_E_CASTLING;
        }

    // Only EP file and current turn are needed to get EP square
    i++;
    if (i >= len) {
        if (write_on_error) *board = new_board;
        return FEN_E_EN_PASSANT;
    }
    switch (fen[i]) {
    case 'a':
        new_board.flags |= FLAG_CAN_EP;
        break;
    case 'b':
        new_board.flags |= FLAG_CAN_EP | (1 << 5);
        break;
    case 'c':
        new_board.flags |= FLAG_CAN_EP | (2 << 5);
        break;
    case 'd':
        new_board.flags |= FLAG_CAN_EP | (3 << 5);
        break;
    case 'e':
        new_board.flags |= FLAG_CAN_EP | (4 << 5);
        break;
    case 'f':
        new_board.flags |= FLAG_CAN_EP | (5 << 5);
        break;
    case 'g':
        new_board.flags |= FLAG_CAN_EP | (6 << 5);
        break;
    case 'h':
        new_board.flags |= FLAG_CAN_EP | (7 << 5);
        break;
    case '-':
        break;
    default:
        if (write_on_error) *board = new_board;
        return FEN_E_EN_PASSANT;
    }
    // Skip rank + space
    i += 2;

    // Halfmoves
    int halfmoves, fullmoves;
    if (sscanf(fen + i, " %d %d", &halfmoves, &fullmoves) != 2) {
        if (write_on_error) *board = new_board;
        return FEN_E_MOVES;
    }

    new_board.halfmoves = halfmoves;
    new_board.fullmoves = fullmoves;

    *board = new_board;

    return 0;
}

char *to_fen(Board *board) {
    // I think max FEN length is around 90, but allocating 100 to be safe
    char *fen = malloc(sizeof(char) * 100);

    int i = 0;
    for (int rank = 7; rank >= 0; rank--) {
        int since_last = 0;
        for (int file = 0; file < 8; file++) {
            int no_piece = 1;
            int square = rank * 8 + file;
            printf("square: %d\n", square);
            uint64_t mask = 1ULL << square;

            for (int bb = 0; bb < 12; bb++) {
                uint64_t bitboard = board->bitboards[bb];
                if (bitboard & mask) {
                    if (since_last) {
                        fen[i++] = '0' + since_last;
                        since_last = 0;
                    }
                    fen[i++] = PIECE_CHARS[bb];
                    no_piece = 0;
                    break;
                }
            }

            since_last += no_piece;

            if (file == 7 && rank != 0) {
                if (since_last) {
                    fen[i++] = '0' + since_last;
                    since_last = 0;
                }
                fen[i++] = '/';
            }
        }
    }

    fen[i] = '\0';
    return fen;
}
