#include "board.h"
#include "color.h"
#include "masks.h"
#include "move.h"
#include "move_gen.h"
#include "piece.h"
#include "square.h"
#include <inttypes.h>
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

    board->flags = 15; // 0b0000_1111
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

int board_in_check(Board *board, Color checked_color) {
    int king_square = ctz_ll(board_bitboard(board, PieceKing, checked_color));
    return board_square_attacked_by(board, king_square,
                                    color_inverse(checked_color));
}

void board_make_move(Board *board, Move move) {
    int source = move_source(move);
    int target = move_target(move);
    uint8_t flags = move_flags(move);
    Color color = board->current_turn;
    Piece moved_piece = board_piece_at(board, source);
    Piece captured_piece = board_piece_at(board, target);

    uint64_t *moved_bb = board_bitboard_p(board, moved_piece, color);

    // Move moved piece, unless promotion
    if (flags & MOVE_PROMOTION) {
        Piece prom_piece = flags & MOVE_SPECIAL;
        uint64_t *prom_bb = board_bitboard_p(board, prom_piece, color);
        *moved_bb ^= 1ULL << source;
        *prom_bb |= 1ULL << target;
    } else {
        *moved_bb ^= (1ULL << source) | (1ULL << target);
    }

    // Handle double moves and EP flags
    if (flags == MOVE_DOUBLE_PUSH) {
        // If double move, allow EP
        board->flags |= FLAG_CAN_EP | (square_file(source) << 5);
    } else {
        // Else, clear en passant bits
        board->flags &= 0x0f;
    }

    // If EP, capture pawn
    if (flags == MOVE_EN_PASSANT) {
        captured_piece = PiecePawn;
        target += (8 * color_direction(color));
    }

    // If castling, move rook and unset castling
    else if (flags == MOVE_KINGSIDE) {
        const uint64_t ROOK_MASK = 0xa000000000000000;

        int shift = color * 56;
        uint64_t *rook_bb = board_bitboard_p(board, PieceRook, color);
        *rook_bb ^= ROOK_MASK >> shift;
    } else if (flags == MOVE_QUEENSIDE) {
        const uint64_t ROOK_MASK = 0x900000000000000;

        int shift = color * 56;
        uint64_t *rook_bb = board_bitboard_p(board, PieceRook, color);
        *rook_bb ^= ROOK_MASK >> shift;
    }

    // If capture, remove captured piece
    if (captured_piece != PieceNone) {
        uint64_t *captured_bb =
            board_bitboard_p(board, captured_piece, color_inverse(color));
        *captured_bb ^= square_mask(target);
    }

    // Unset castling flags if necessary
    if (source == A1 || target == A1) board->flags &= ~FLAG_WHITE_QUEENSIDE;
    if (source == H1 || target == H1) board->flags &= ~FLAG_WHITE_KINGSIDE;
    if (source == A8 || target == A8) board->flags &= ~FLAG_BLACK_QUEENSIDE;
    if (source == H8 || target == H8) board->flags &= ~FLAG_BLACK_KINGSIDE;
    if (moved_piece == PieceKing)
        board->flags &= ~(FLAG_WHITE_CASTLE << (color * 2));

    // Update metadata
    board->current_turn ^= 1;
    board->fullmoves += board->current_turn;
    // TODO: Other ways in which halfmove counter is reset
    if (moved_piece == PiecePawn || captured_piece != PieceNone) {
        board->halfmoves = 0;
    } else {
        board->halfmoves++;
    }
}

uint64_t board_all_pieces(Board *board) {
    return board->bitboards[0] | board->bitboards[1] | board->bitboards[2] |
           board->bitboards[3] | board->bitboards[4] | board->bitboards[5] |
           board->bitboards[6] | board->bitboards[7] | board->bitboards[8] |
           board->bitboards[9] | board->bitboards[10] | board->bitboards[11];
}

Piece board_piece_at(Board *board, int square) {
    uint64_t mask = 1ULL << square;
    if (!(board_all_pieces(board) & mask)) return PieceNone;

    for (int i = 0; i < 12; i++) {
        if (board->bitboards[i] & mask) {
            Piece piece = i > 5 ? i - 6 : i;
            return piece;
        }
    }

    return PieceNone;
}

int board_square_attacked_by(Board *board, int square, Color attacking_color) {
    // Pawn attacks
    uint64_t pawns = board_bitboard(board, PiecePawn, attacking_color);
    uint64_t pawn_targets = attacking_color ? BLACK_PAWN_TARGETS[square]
                                            : WHITE_PAWN_TARGETS[square];
    if (pawns & pawn_targets) return 1;

    // King attacks
    uint64_t kings = board_bitboard(board, PieceKing, attacking_color);
    if (kings & KING_TARGETS[square]) return 1;

    // Knight attacks
    uint64_t knights = board_bitboard(board, PieceKnight, attacking_color);
    if (knights & KNIGHT_TARGETS[square]) return 1;

    // Rooks & queens
    uint64_t blockers = board_pieces(board, White) | board_pieces(board, Black);
    uint64_t rooks_queens = board_bitboard(board, PieceRook, attacking_color) |
                            board_bitboard(board, PieceQueen, attacking_color);
    if (move_gen_rook_moves(square, blockers) & rooks_queens) return 1;

    // Bishops & queens
    uint64_t bishops_queens =
        board_bitboard(board, PieceBishop, attacking_color) |
        board_bitboard(board, PieceQueen, attacking_color);
    if (move_gen_bishop_moves(square, blockers) & bishops_queens) return 1;

    return 0;
}
