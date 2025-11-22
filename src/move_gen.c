#include "move_gen.h"
#include "board.h"
#include "color.h"
#include "gen/magics.h"
#include "masks.h"
#include "move.h"
#include "piece.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

int magic_index(const MagicEntry *entry, uint64_t blockers) {
    blockers &= entry->mask;
    blockers *= entry->magic;
    blockers >>= entry->shift;
    return blockers;
}

uint64_t move_gen_rook_moves(int square, uint64_t blockers) {
    MagicEntry magic = ROOK_MAGICS[square];
    uint64_t *moves = magic.att;
    return moves[magic_index(&magic, blockers)];
}

uint64_t move_gen_bishop_moves(int square, uint64_t blockers) {
    MagicEntry magic = BISHOP_MAGICS[square];
    uint64_t *moves = magic.att;
    return moves[magic_index(&magic, blockers)];
}

uint64_t rook_moves(int square, uint64_t blockers) {
    uint64_t result = 0ULL;
    int r, f;
    int rank = square / 8;
    int file = square % 8;

    for (r = rank + 1; r < 8; r++) {
        result |= 1ULL << (r * 8 + file);
        if (blockers & (1ULL << (r * 8 + file))) break;
    }
    for (r = rank - 1; r >= 0; r--) {
        result |= 1ULL << (r * 8 + file);
        if (blockers & (1ULL << (r * 8 + file))) break;
    }
    for (f = file + 1; f < 8; f++) {
        result |= 1ULL << (rank * 8 + f);
        if (blockers & (1ULL << (rank * 8 + f))) break;
    }
    for (f = file - 1; f >= 0; f--) {
        result |= 1ULL << (rank * 8 + f);
        if (blockers & (1ULL << (rank * 8 + f))) break;
    }

    return result;
}

uint64_t bishop_moves(int square, uint64_t blockers) {
    uint64_t result = 0ULL;
    int r, f;
    int rank = square / 8;
    int file = square % 8;

    for (r = rank + 1, f = file + 1; r < 8 && f < 8; r++, f++) {
        result |= 1ULL << (r * 8 + f);
        if (blockers & (1ULL << (r * 8 + f))) break;
    }
    for (r = rank - 1, f = file + 1; r >= 0 && f < 8; r--, f++) {
        result |= 1ULL << (r * 8 + f);
        if (blockers & (1ULL << (r * 8 + f))) break;
    }
    for (r = rank + 1, f = file - 1; r < 8 && f >= 0; r++, f--) {
        result |= 1ULL << (r * 8 + f);
        if (blockers & (1ULL << (r * 8 + f))) break;
    }
    for (r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
        result |= 1ULL << (r * 8 + f);
        if (blockers & (1ULL << (r * 8 + f))) break;
    }

    return result;
}

int fill_table(int square, MagicEntry *magic, int bishop) {
    int table_len = 1 << (64 - magic->shift);
    uint64_t *table = malloc(sizeof(uint64_t) * table_len);
    if (table == NULL) return -1;

    for (int i = 0; i < table_len; i++) {
        table[i] = 0ULL;
    }

    int i;
    uint64_t moves;
    uint64_t mask = magic->mask;
    uint64_t blockers = 0ULL;
    do {
        moves = bishop ? bishop_moves(square, blockers)
                       : rook_moves(square, blockers);
        i = magic_index(magic, blockers);
        table[i] = moves;

        blockers = (blockers - mask) & mask;
    } while (blockers);

    magic->att = table;

    return 0;
}

int magic_init(void) {
    int i, error;
    for (i = 0; i < 64; i++) {
        error = fill_table(i, &ROOK_MAGICS[i], 0);
        if (error) return error;
        error = fill_table(i, &BISHOP_MAGICS[i], 1);
        if (error) return error;
    }
    return 0;
}

void magic_cleanup(void) {
    for (int i = 0; i < 64; i++) {
        if (ROOK_MAGICS[i].att != NULL) {
            free(ROOK_MAGICS[i].att);
            ROOK_MAGICS[i].att = NULL;
        }
        if (BISHOP_MAGICS[i].att != NULL) {
            free(BISHOP_MAGICS[i].att);
            BISHOP_MAGICS[i].att = NULL;
        }
    }
}

int move_gen_init(void) {
    return magic_init();
}

void move_gen_cleanup(void) {
    magic_cleanup();
}

// Using compiler builtins (eg. __builtin_ctzll) is faster than generic case
// but I want this to be mostly compiler agnostic (I use GCC on one machine
// and Clang on another).
int ctz_ll(uint64_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return x == 0 ? 64 : __builtin_ctzll(x);
#elif defined(_MSC_VER)
    uint64_t i;
    if (_BitScanForward64(&i, x)) return i;
    return 64;
#else
    if (x == 0) return 64;
    static const int lookup[64] = {
        0,  1,  2,  7,  3,  13, 8,  19, 4,  25, 14, 28, 9,  34, 20, 40,
        5,  17, 26, 38, 15, 31, 29, 45, 10, 21, 35, 47, 41, 54, 48, 63,
        6,  11, 18, 23, 27, 33, 39, 44, 16, 22, 32, 37, 30, 43, 46, 53,
        50, 49, 57, 56, 55, 62, 52, 61, 42, 51, 58, 59, 60, 24, 36, 60};
    return lookup[((uint64_t)((x & -x) * 0x0218A392CD3D5E45ULL)) >> 58];
#endif
}

static inline uint64_t rrot(uint64_t n, int d) {
    return (n >> d) | (n << (64 - d));
}

// NB: Caller needs to guarantee there is enough space in 'moves'.
//     Normally, 218 should be enough (max moves in a single position),
//     but for variants this *might* change (unsure if I will add variant
//     support).
int move_gen_generate_moves(Board *board, Move *moves) {
    // Common variables
    int moves_i = 0;
    int source, target;
    uint16_t flags;
    int source_rank, target_rank, target_file;
    uint64_t targets;

    // Useful values
    Color friendly_color = board->current_turn;
    Color enemy_color = color_inverse(friendly_color);
    uint64_t pawns = board_bitboard(board, PiecePawn, friendly_color);
    uint64_t knights = board_bitboard(board, PieceKnight, friendly_color);
    uint64_t bishops = board_bitboard(board, PieceBishop, friendly_color);
    uint64_t rooks = board_bitboard(board, PieceRook, friendly_color);
    uint64_t queens = board_bitboard(board, PieceQueen, friendly_color);
    int king = ctz_ll(board_bitboard(board, PieceKing, friendly_color));
    uint64_t friends = board_pieces(board, friendly_color);
    uint64_t enemies = board_pieces(board, enemy_color);
    uint64_t blockers = friends | enemies;

    // Pawns
    // En passant
    if (board->flags & FLAG_CAN_EP) {
        target_file = board->flags >> 5;
        target_rank = 2 + (3 * friendly_color);
        target = target_rank * 8 + target_file;

        source_rank = target_rank + color_direction(friendly_color);
        int source_file_l = target_file - 1;
        int source_file_r = target_file + 1;

        source = source_rank * 8 + source_file_l;
        if (source_file_l >= 0 && (1ULL << source) & pawns) {
            moves[moves_i++] = new_move(source, target, MOVE_EN_PASSANT);
        }
        source = source_rank * 8 + source_file_r;
        if (source_file_r < 8 && (1ULL << source) & pawns) {
            moves[moves_i++] = new_move(source, target, MOVE_EN_PASSANT);
        }
    }

    // Forward moves
    uint64_t pawn_home_single_move_rank =
        0x0000ff0000000000 >> 24 * friendly_color;

    uint64_t single_targets = rrot(pawns, 8 + 48 * friendly_color) & ~blockers;
    uint64_t double_targets = rrot(single_targets & pawn_home_single_move_rank,
                                   8 + 48 * friendly_color) &
                              ~blockers;

    for (target = 0; single_targets; single_targets >>= 1, target++) {
        if (!(single_targets & 1)) continue;

        source = target + (8 * color_direction(friendly_color));

        if (target < 8 || target > 55) {
            moves[moves_i++] = new_move(source, target, MOVE_PROMOTION);
            moves[moves_i++] = new_move(source, target, MOVE_PROMOTION | 1);
            moves[moves_i++] = new_move(source, target, MOVE_PROMOTION | 2);
            moves[moves_i++] = new_move(source, target, MOVE_PROMOTION | 3);
            continue;
        }

        moves[moves_i++] = new_move(source, target, MOVE_QUIET);
    }

    for (double_targets >>= 16, target = 16; double_targets;
         double_targets >>= 1, target++) {
        if (!(double_targets & 1)) continue;

        source = target + (16 * color_direction(friendly_color));
        moves[moves_i++] = new_move(source, target, MOVE_DOUBLE_PUSH);
    }

    // Pawn captures
    // FIXME this is bad for branch prediction, use rrot or similar to
    // make branchless
    uint64_t left_captures = (friendly_color ? pawns << 7 : pawns >> 9) &
                             enemies & ~0x8080808080808080;
    uint64_t right_captures = (friendly_color ? pawns << 9 : pawns >> 7) &
                              enemies & ~0x0101010101010101;

    // Add to moves
    for (target = 0; left_captures; left_captures >>= 1, target++) {
        if (!(left_captures & 1)) continue;

        source = friendly_color ? target - 7 : target + 9;

        if (target < 8 || target > 55) {
            int base_flags = MOVE_PROMOTION | MOVE_CAPTURE;
            moves[moves_i++] = new_move(source, target, base_flags);
            moves[moves_i++] = new_move(source, target, base_flags | 1);
            moves[moves_i++] = new_move(source, target, base_flags | 2);
            moves[moves_i++] = new_move(source, target, base_flags | 3);
            continue;
        }
        moves[moves_i++] = new_move(source, target, MOVE_CAPTURE);
    }
    for (target = 0; right_captures; right_captures >>= 1, target++) {
        if (!(right_captures & 1)) continue;

        source = friendly_color ? target - 9 : target + 7;

        if (target < 8 || target > 55) {
            int base_flags = MOVE_PROMOTION | MOVE_CAPTURE;
            moves[moves_i++] = new_move(source, target, base_flags);
            moves[moves_i++] = new_move(source, target, base_flags | 1);
            moves[moves_i++] = new_move(source, target, base_flags | 2);
            moves[moves_i++] = new_move(source, target, base_flags | 3);
            continue;
        }
        moves[moves_i++] = new_move(source, target, MOVE_CAPTURE);
    }

    // Knights
    for (source = 0; knights; knights >>= 1, source++) {
        if (!(knights & 1ULL)) continue;

        targets = KNIGHT_TARGETS[source] & ~friends;
        for (target = 0; targets; targets >>= 1, target++) {
            if (!(targets & 1ULL)) continue;

            uint64_t mask = 1ULL << target;
            flags = (mask & enemies) ? MOVE_CAPTURE : MOVE_QUIET;
            moves[moves_i++] = new_move(source, target, flags);
        }
    }

    // Bishops
    for (source = 0; bishops; bishops >>= 1, source++) {
        if (!(bishops & 1ULL)) continue;

        targets = move_gen_bishop_moves(source, blockers) & ~friends;
        for (target = 0; targets; targets >>= 1, target++) {
            if (!(targets & 1ULL)) continue;

            uint64_t mask = 1ULL << target;
            flags = (mask & enemies) ? MOVE_CAPTURE : MOVE_QUIET;
            moves[moves_i++] = new_move(source, target, flags);
        }
    }

    // Rooks
    for (source = 0; rooks; rooks >>= 1, source++) {
        if (!(rooks & 1ULL)) continue;

        targets = move_gen_rook_moves(source, blockers) & ~friends;
        for (target = 0; targets; targets >>= 1, target++) {
            if (!(targets & 1ULL)) continue;

            uint64_t mask = 1ULL << target;
            flags = (mask & enemies) ? MOVE_CAPTURE : MOVE_QUIET;
            moves[moves_i++] = new_move(source, target, flags);
        }
    }

    // Queens
    for (source = 0; queens; queens >>= 1, source++) {
        if (!(queens & 1ULL)) continue;

        targets = (move_gen_rook_moves(source, blockers) |
                   move_gen_bishop_moves(source, blockers)) &
                  ~friends;
        for (target = 0; targets; targets >>= 1, target++) {
            if (!(targets & 1ULL)) continue;

            uint64_t mask = 1ULL << target;
            flags = (mask & enemies) ? MOVE_CAPTURE : MOVE_QUIET;
            moves[moves_i++] = new_move(source, target, flags);
        }
    }

    // King
    targets = KING_TARGETS[king] & ~friends;
    for (target = 0; targets; targets >>= 1, target++) {
        if (!(targets & 1ULL)) continue;

        uint64_t mask = 1ULL << target;
        flags = (mask & enemies) ? MOVE_CAPTURE : MOVE_QUIET;
        moves[moves_i++] = new_move(king, target, flags);
    }

    // Castling
    int can_castle;
    uint64_t castling_blockers;

    // Kingside
    can_castle = board->flags & (FLAG_BLACK_KINGSIDE << (2 * friendly_color));
    castling_blockers =
        blockers & (0x6000000000000000ULL >> (56 * friendly_color));
    if (can_castle && !castling_blockers) {
        // Check for checks
        // TODO See if maintaining incremental attack table is faster
        if (!board_square_attacked_by(board, king, enemy_color) &&
            !board_square_attacked_by(board, king + 1, enemy_color) &&
            !board_square_attacked_by(board, king + 2, enemy_color))
            moves[moves_i++] = new_move(king, king + 2, MOVE_KINGSIDE);
    }

    // Queenside
    can_castle = board->flags & (FLAG_BLACK_QUEENSIDE << (2 * friendly_color));
    castling_blockers =
        blockers & (0xe00000000000000ULL >> (56 * friendly_color));
    if (can_castle && !castling_blockers) {
        // Check for checks
        if (!board_square_attacked_by(board, king, enemy_color) &&
            !board_square_attacked_by(board, king - 1, enemy_color) &&
            !board_square_attacked_by(board, king - 2, enemy_color))
            moves[moves_i++] = new_move(king, king - 2, MOVE_QUEENSIDE);
    }

    return moves_i;
}

int move_gen_generate_legal_moves(Board *board, Move *moves) {
    int pseudo_count = move_gen_generate_moves(board, moves);

    int i = 0;
    int end = pseudo_count - 1;

    // Replace illegal moves with future moves and test those as well
    while (i < end) {
        if (!move_gen_is_legal_move(board, moves[i])) {
            moves[i] = moves[end--];
            pseudo_count--;
            continue;
        }
        i++;
    }

    // Check last move as well
    if (!move_gen_is_legal_move(board, moves[end])) pseudo_count--;

    if (pseudo_count < 0) return 0;
    return pseudo_count;
}

int move_gen_is_legal_move(Board *board, Move move) {
    Board new_board = *board;
    board_make_move(&new_board, move);
    uint64_t king_bb =
        board_bitboard(&new_board, PieceKing, board->current_turn);
    int king_square = ctz_ll(king_bb);

    return !board_square_attacked_by(&new_board, king_square,
                                     new_board.current_turn);
}
