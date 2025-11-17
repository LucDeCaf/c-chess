#include "move_gen.h"
#include "gen/magics.h"
#include "move.h"
#include "piece.h"
#include <inttypes.h>
#include <stdlib.h>

const uint64_t KNIGHT_TARGETS[64] = {
    132096ULL,
    329728ULL,
    659712ULL,
    1319424ULL,
    2638848ULL,
    5277696ULL,
    10489856ULL,
    4202496ULL,
    33816580ULL,
    84410376ULL,
    168886289ULL,
    337772578ULL,
    675545156ULL,
    1351090312ULL,
    2685403152ULL,
    1075839008ULL,
    8657044482ULL,
    21609056261ULL,
    43234889994ULL,
    86469779988ULL,
    172939559976ULL,
    345879119952ULL,
    687463207072ULL,
    275414786112ULL,
    2216203387392ULL,
    5531918402816ULL,
    11068131838464ULL,
    22136263676928ULL,
    44272527353856ULL,
    88545054707712ULL,
    175990581010432ULL,
    70506185244672ULL,
    567348067172352ULL,
    1416171111120896ULL,
    2833441750646784ULL,
    5666883501293568ULL,
    11333767002587136ULL,
    22667534005174272ULL,
    45053588738670592ULL,
    18049583422636032ULL,
    145241105196122112ULL,
    362539804446949376ULL,
    725361088165576704ULL,
    1450722176331153408ULL,
    2901444352662306816ULL,
    5802888705324613632ULL,
    11533718717099671552ULL,
    4620693356194824192ULL,
    288234782788157440ULL,
    576469569871282176ULL,
    1224997833292120064ULL,
    2449995666584240128ULL,
    4899991333168480256ULL,
    9799982666336960512ULL,
    1152939783987658752ULL,
    2305878468463689728ULL,
    1128098930098176ULL,
    2257297371824128ULL,
    4796069720358912ULL,
    9592139440717824ULL,
    19184278881435648ULL,
    38368557762871296ULL,
    4679521487814656ULL,
    9077567998918656ULL,
};

int magic_index(const MagicEntry *entry, uint64_t blockers) {
    return ((blockers & entry->mask) * entry->magic) >> entry->shift;
}

uint64_t magic_rook_moves(int square, uint64_t blockers) {
    MagicEntry magic = ROOK_MAGICS[square];
    uint64_t *moves = magic.att;
    return moves[magic_index(&magic, blockers)];
}

uint64_t magic_bishop_moves(int square, uint64_t blockers) {
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
        if (blockers & (1ULL << (r * 8 + file)))
            break;
    }
    for (r = rank - 1; r >= 0; r--) {
        result |= 1ULL << (r * 8 + file);
        if (blockers & (1ULL << (r * 8 + file)))
            break;
    }
    for (f = file + 1; f < 8; f++) {
        result |= 1ULL << (rank * 8 + f);
        if (blockers & (1ULL << (rank * 8 + f)))
            break;
    }
    for (f = file - 1; f >= 0; f--) {
        result |= 1ULL << (rank * 8 + f);
        if (blockers & (1ULL << (rank * 8 + f)))
            break;
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
        if (blockers & (1ULL << (r * 8 + f)))
            break;
    }
    for (r = rank - 1, f = file + 1; r >= 0 && f < 8; r--, f++) {
        result |= 1ULL << (r * 8 + f);
        if (blockers & (1ULL << (r * 8 + f)))
            break;
    }
    for (r = rank + 1, f = file - 1; r < 8 && f >= 0; r++, f--) {
        result |= 1ULL << (r * 8 + f);
        if (blockers & (1ULL << (r * 8 + f)))
            break;
    }
    for (r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
        result |= 1ULL << (r * 8 + f);
        if (blockers & (1ULL << (r * 8 + f)))
            break;
    }

    return result;
}

int fill_table(int square, MagicEntry *magic, int bishop) {
    int table_len = 1 << (64 - magic->shift);
    uint64_t *table = malloc(sizeof(uint64_t) * table_len);
    if (table == NULL)
        return -1;

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
        if (error)
            return error;
        error = fill_table(i, &BISHOP_MAGICS[i], 1);
        if (error)
            return error;
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

int move_gen_init() {
    return magic_init();
}

void move_gen_cleanup() {
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
    if (_BitScanForward64(&i, x))
        return i;
    return 64;
#else
    if (x == 0)
        return 64;
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

// NB: Caller needs to guarantee there is enough space in 'moves'
//     Normally, 218 should be enough (max moves in 1 pos), but
//     for variants this may change (unsure if I will add variant
//     support).
int generate_moves(Board *board, Move *moves) {
    int moves_i = 0;
    int source, target;

    Color color = board->current_turn;
    uint64_t pawns = board_bitboard(board, PiecePawn, color);
    // uint64_t knights = board_bitboard(board, PieceKnight, color);
    uint64_t bishops = board_bitboard(board, PieceBishop, color);
    // uint64_t rooks = board_bitboard(board, PieceRook, color);
    // uint64_t queens = board_bitboard(board, PieceQueen, color);
    // int king = ctz_ll(board_bitboard(board, PieceKing, color));
    uint64_t friends = board_pieces(board, color);
    uint64_t enemies = board_pieces(board, color_inverse(color));
    uint64_t blockers = friends | enemies;

    // Pawns
    // Forward moves
    uint64_t pawn_home_single_move_rank = 0x0000ff0000000000 >> color * 24;

    uint64_t pawn_targets = rrot(pawns, 8 + color * 48) & ~blockers;
    uint64_t pawn_double_targets =
        rrot(pawn_targets & pawn_home_single_move_rank, 8 + color * 48) &
        ~blockers;

    // Add to moves
    for (target = 0; pawn_targets; pawn_targets >>= 1, target++) {
        if (!(pawn_targets & 1))
            continue;

        Move move =
            new_move(target + (8 * color_direction(color)), target, MOVE_QUIET);
        moves[moves_i++] = move;
    }
    for (pawn_double_targets >>= 16, target = 16; pawn_double_targets;
         pawn_double_targets >>= 1, target++) {
        if (!(pawn_double_targets & 1))
            continue;

        Move move = new_move(target + (16 * color_direction(color)), target,
                             MOVE_DOUBLE_PUSH);
        moves[moves_i++] = move;
    }

    // Pawn captures
    // uint64_t left_captures = color == White ? pawns << 7 : pawns >> 7;
    uint64_t left_captures =
        (color ? pawns << 7 : pawns >> 9) & blockers & ~0x8080808080808080;
    uint64_t right_captures =
        (color ? pawns << 9 : pawns >> 7) & blockers & ~0x101010101010101;

    // Add to moves
    for (target = 0; left_captures; left_captures >>= 1, target++) {
        if (!(left_captures & 1))
            continue;

        Move move = new_move(target - (9 - color * 2), target, MOVE_CAPTURE);
        moves[moves_i++] = move;
    }
    for (target = 0; right_captures; right_captures >>= 1, target++) {
        if (!(right_captures & 1))
            continue;

        Move move = new_move(target - (7 + color * 2), target, MOVE_CAPTURE);
        moves[moves_i++] = move;
    }

    // TODO en passant

    // Bishops
    for (source = 0; bishops; bishops >>= 1, source++) {
        if (!(bishops & 1ULL))
            continue;

        uint64_t targets = magic_bishop_moves(source, blockers) & ~friends;
        for (target = 0; targets; targets >>= 1, target++) {
            if (!(targets & 1ULL))
                continue;

            uint64_t mask = 1ULL << target;
            uint8_t flags = (mask & enemies) ? MOVE_CAPTURE : MOVE_QUIET;
            Move move = new_move(source, target, flags);
            moves[moves_i++] = move;
        }
    }

    // TODO remaining pieces

    return moves_i;
}
