#include "board.h"
#include "color.h"
#include "masks.h"
#include "move.h"
#include "move_gen.h"
#include "piece.h"
#include "square.h"
#include <inttypes.h>
#include <stdint.h>

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
    for (int i = 0; i < 12; i++)
        board->bitboards[i] = 0ULL;
    for (int i = 0; i < 64; i++)
        board->pieces[i] = PieceNone;

    // White
    board_add_piece(board, A1, PieceRook, White);
    board_add_piece(board, B1, PieceKnight, White);
    board_add_piece(board, C1, PieceBishop, White);
    board_add_piece(board, D1, PieceQueen, White);
    board_add_piece(board, E1, PieceKing, White);
    board_add_piece(board, F1, PieceBishop, White);
    board_add_piece(board, G1, PieceKnight, White);
    board_add_piece(board, H1, PieceRook, White);
    // White pawns
    board_add_piece(board, A2, PiecePawn, White);
    board_add_piece(board, B2, PiecePawn, White);
    board_add_piece(board, C2, PiecePawn, White);
    board_add_piece(board, D2, PiecePawn, White);
    board_add_piece(board, E2, PiecePawn, White);
    board_add_piece(board, F2, PiecePawn, White);
    board_add_piece(board, G2, PiecePawn, White);
    board_add_piece(board, H2, PiecePawn, White);

    // Black
    board_add_piece(board, A8, PieceRook, Black);
    board_add_piece(board, B8, PieceKnight, Black);
    board_add_piece(board, C8, PieceBishop, Black);
    board_add_piece(board, D8, PieceQueen, Black);
    board_add_piece(board, E8, PieceKing, Black);
    board_add_piece(board, F8, PieceBishop, Black);
    board_add_piece(board, G8, PieceKnight, Black);
    board_add_piece(board, H8, PieceRook, Black);
    // Black pawns
    board_add_piece(board, A7, PiecePawn, Black);
    board_add_piece(board, B7, PiecePawn, Black);
    board_add_piece(board, C7, PiecePawn, Black);
    board_add_piece(board, D7, PiecePawn, Black);
    board_add_piece(board, E7, PiecePawn, Black);
    board_add_piece(board, F7, PiecePawn, Black);
    board_add_piece(board, G7, PiecePawn, Black);
    board_add_piece(board, H7, PiecePawn, Black);

    // Metadata
    board->flags = 0x0f; // 0b0000_1111
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

// NB: Error to call this with PieceNone
void board_add_piece(Board *board, int square, Piece piece, Color color) {
    *board_bitboard_p(board, piece, color) |= 1ULL << square;
    // Store color of piece with piece
    board->pieces[square] = piece | (color << 4);
}

// NB: Error to call this on PieceNone
void board_clear_piece(Board *board, int square) {
    Color color = board->pieces[square] >> 4;
    Piece piece = board->pieces[square] & 0xf;
    *board_bitboard_p(board, piece, color) &= ~(1ULL << square);
    board->pieces[square] = PieceNone;
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

    // Remove moved piece from source
    board_clear_piece(board, source);

    // Handle double moves and EP flags
    board->flags &= 0x0f;
    board->flags |= (flags == MOVE_DOUBLE_PUSH) *
                    (FLAG_CAN_EP | (square_file(source) << 5));

    // If EP, capture pawn
    if (flags == MOVE_EN_PASSANT) {
        captured_piece = PiecePawn;
        board_clear_piece(board, target + 8 * color_direction(color));
    }

    // If castling, move rook and unset castling
    else if (flags == MOVE_KINGSIDE) {
        int rook_source = color ? H1 : H8;
        int rook_target = color ? F1 : F8;
        board_clear_piece(board, rook_source);
        board_add_piece(board, rook_target, PieceRook, color);
    } else if (flags == MOVE_QUEENSIDE) {
        int rook_source = color ? A1 : A8;
        int rook_target = color ? D1 : D8;
        board_clear_piece(board, rook_source);
        board_add_piece(board, rook_target, PieceRook, color);
    }

    // If capture, remove captured piece
    if (captured_piece != PieceNone) {
        board_clear_piece(board, target);
    }

    // Move moved piece
    Piece added_piece =
        (flags & MOVE_PROMOTION) ? (flags & MOVE_SPECIAL) : moved_piece;
    board_add_piece(board, target, added_piece, color);

    // Unset castling flags if necessary
    uint8_t clear_mask =
        (((source == A1) | (target == A1)) * FLAG_WHITE_QUEENSIDE) |
        (((source == H1) | (target == H1)) * FLAG_WHITE_KINGSIDE) |
        (((source == A8) | (target == A8)) * FLAG_BLACK_QUEENSIDE) |
        (((source == H8) | (target == H8)) * FLAG_BLACK_KINGSIDE) |
        ((moved_piece == PieceKing) * (FLAG_BLACK_CASTLE << (color * 2)));
    uint8_t castling_bits = (board->flags & 0x0f) & ~clear_mask;
    board->flags = (board->flags & 0xf0) | castling_bits;

    // Update metadata
    board->current_turn ^= 1;
    board->fullmoves += board->current_turn;
    board->halfmoves +=
        (moved_piece == PiecePawn || captured_piece != PieceNone);
}

uint64_t board_all_pieces(Board *board) {
    return board->bitboards[0] | board->bitboards[1] | board->bitboards[2] |
           board->bitboards[3] | board->bitboards[4] | board->bitboards[5] |
           board->bitboards[6] | board->bitboards[7] | board->bitboards[8] |
           board->bitboards[9] | board->bitboards[10] | board->bitboards[11];
}

// TODO check if storing `Piece pieces[64]` in board is significantly faster
// than this
Piece board_piece_at(Board *board, int square) {
    return board->pieces[square] & 0xf;
}

int board_square_attacked_by(Board *board, int square, Color attacking_color) {

    // Knight attacks
    uint64_t knights = board_bitboard(board, PieceKnight, attacking_color);
    if (knights & KNIGHT_TARGETS[square]) return 1;

    // King attacks
    uint64_t kings = board_bitboard(board, PieceKing, attacking_color);
    if (kings & KING_TARGETS[square]) return 1;

    // Pawn attacks
    uint64_t pawns = board_bitboard(board, PiecePawn, attacking_color);
    uint64_t pawn_targets =
        PAWN_TARGETS[color_inverse(attacking_color)][square];
    if (pawns & pawn_targets) return 1;

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
