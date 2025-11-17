#include "board.h"
#include "color.h"
#include "move.h"
#include "piece.h"
#include "square.h"
#include <stdint.h>
#include <stdio.h>

enum {
    BB_KNIGHT_BLACK = 0,
    BB_BISHOP_BLACK,
    BB_ROOK_BLACK,
    BB_QUEEN_BLACK,
    BB_KING_BLACK,
    BB_PAWN_BLACK,
    BB_KNIGHT_WHITE,
    BB_BISHOP_WHITE,
    BB_ROOK_WHITE,
    BB_QUEEN_WHITE,
    BB_KING_WHITE,
    BB_PAWN_WHITE,
};

// clang-format off
static const uint8_t FL_WHITE_KINGSIDE  = 0b00000001;
static const uint8_t FL_WHITE_QUEENSIDE = 0b00000010;
static const uint8_t FL_BLACK_KINGSIDE  = 0b00000100;
static const uint8_t FL_BLACK_QUEENSIDE = 0b00001000;
static const uint8_t FL_EP_CHECK_FLAG   = 0b00010000; // 1 if can ep, 0 if cannot
static const uint8_t FL_EP_FILE         = 0b11100010; // 0-7, corresponds to each rank
// clang-format on

void board_init(Board *board) {
    // Starting position
    board->bitboards[BB_KNIGHT_BLACK] = 0x4200000000000000;
    board->bitboards[BB_BISHOP_BLACK] = 0x2400000000000000;
    board->bitboards[BB_ROOK_BLACK] = 0x8100000000000000;
    board->bitboards[BB_QUEEN_BLACK] = 0x800000000000000;
    board->bitboards[BB_KING_BLACK] = 0x1000000000000000;
    board->bitboards[BB_PAWN_BLACK] = 0xff000000000000;
    board->bitboards[BB_KNIGHT_WHITE] = 0x42;
    board->bitboards[BB_BISHOP_WHITE] = 0x24;
    board->bitboards[BB_ROOK_WHITE] = 0x81;
    board->bitboards[BB_QUEEN_WHITE] = 0x8;
    board->bitboards[BB_KING_WHITE] = 0x10;
    board->bitboards[BB_PAWN_WHITE] = 0xff00;

    board->flags = 0b00001111;
    board->current_turn = White;
    board->halfmoves = 0;
    board->fullmoves = 1;
}

uint64_t board_bitboard(Board *board, Piece piece, Color color) {
    return board->bitboards[piece + (color * 6)];
}

uint64_t *board_bitboard_p(Board *board, Piece piece, Color color) {
    return &board->bitboards[piece + (color * 6)];
}

uint64_t board_pieces(Board *board, Color color) {
    int m = color * 6;
    uint64_t result = 0ULL;
    for (int i = m; i < m + 6; i++)
        result |= board->bitboards[i];
    return result;
}

void board_make_move(Board *board, Move move) {
    Square source = move_source(move);
    Square target = move_target(move);
    uint8_t flags = move_flags(move);
    Color moved_color = board->current_turn;
    Piece moved_piece = board_piece_at(board, source);
    Piece captured_piece = board_piece_at(board, target);

    // Move moved piece
    uint64_t *moved_bb = board_bitboard_p(board, moved_piece, moved_color);
    *moved_bb ^= square_mask(source) | square_mask(target);

    // If capture, remove captured piece
    if (captured_piece != PieceNone) {
        uint64_t *captured_bb =
            board_bitboard_p(board, captured_piece, color_inverse(moved_color));
        *captured_bb ^= square_mask(target);
    }

    // Update metadata
    board->current_turn ^= 1;
    board->fullmoves += board->current_turn;
    // TODO: Other ways in which halfmove counter is reset
    if (moved_piece == PiecePawn) {
        board->halfmoves = 0;
    } else {
        board->halfmoves++;
    }

    return;
}

Piece board_piece_at(Board *board, Square square) {
    uint64_t mask = square_mask(square);

    for (int i = 0; i < 12; i++) {
        if (board->bitboards[i] & mask) {
            Piece piece = i > 5 ? i - 6 : i;
            return piece;
        }
    }

    return PieceNone;
}
