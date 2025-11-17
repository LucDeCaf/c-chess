#include "fen.h"
#include "board.h"
#include "color.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char PIECE_CHARS[12] = "nbrqkpNBRQKP";

int load_fen(Board *board, char *fen) {
    Board new_board;

    int len = strlen(fen);
    int rank = 7, file = 0;
    int i;

    for (i = 0; i < 12; i++) {
        new_board.bitboards[i] = 0;
    }

    for (i = 0; i < len; i++) {
        int square = rank * 8 + file;
        if (square < 0 || square > 63) {
            return -1;
        }

        if (fen[i] == ' ')
            return -1;

        int skip;
        switch (fen[i]) {
        case 'n':
            *board_bitboard_p(&new_board, PieceKnight, Black) |= 1ULL << square;
            file++;
            break;
        case 'b':
            *board_bitboard_p(&new_board, PieceBishop, Black) |= 1ULL << square;
            file++;
            break;
        case 'r':
            *board_bitboard_p(&new_board, PieceRook, Black) |= 1ULL << square;
            file++;
            break;
        case 'q':
            *board_bitboard_p(&new_board, PieceQueen, Black) |= 1ULL << square;
            file++;
            break;
        case 'k':
            *board_bitboard_p(&new_board, PieceKing, Black) |= 1ULL << square;
            file++;
            break;
        case 'p':
            *board_bitboard_p(&new_board, PiecePawn, Black) |= 1ULL << square;
            file++;
            break;
        case 'N':
            *board_bitboard_p(&new_board, PieceKnight, White) |= 1ULL << square;
            file++;
            break;
        case 'B':
            *board_bitboard_p(&new_board, PieceBishop, White) |= 1ULL << square;
            file++;
            break;
        case 'R':
            *board_bitboard_p(&new_board, PieceRook, White) |= 1ULL << square;
            file++;
            break;
        case 'Q':
            *board_bitboard_p(&new_board, PieceQueen, White) |= 1ULL << square;
            file++;
            break;
        case 'K':
            *board_bitboard_p(&new_board, PieceKing, White) |= 1ULL << square;
            file++;
            break;
        case 'P':
            *board_bitboard_p(&new_board, PiecePawn, White) |= 1ULL << square;
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
            return -1;
        }

        if (file > 7) {
            file = 0;
            rank--;
        }

        if (rank < 0)
            break;
    }

    // Skip piece and following space
    i += 2;
    if (i >= len)
        return -2;

    switch (fen[i++]) {
    case 'w':
        new_board.current_turn = White;
        break;
    case 'b':
        new_board.current_turn = Black;
        break;
    default:
        return -2;
    }

    // TODO castling rights (unimplemented in board)
    i++;
    while (fen[i] != ' ' && i < len)
        i++;

    // TODO en passant (unimplemented in board)
    i++;
    while (fen[i] != ' ' && i < len)
        i++;

    // Halfmoves
    int halfmoves, fullmoves;
    if (sscanf(fen + i, " %d %d", &halfmoves, &fullmoves) != 2) {
        return -5;
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
