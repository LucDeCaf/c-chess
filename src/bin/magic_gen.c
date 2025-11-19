#include "../pcg_basic.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    uint64_t *att;
    uint64_t magic;
    uint64_t mask;
    uint8_t shift;
} MagicEntry;

pcg32_random_t rng;

uint64_t random_u64(void) {
    uint64_t a, b;
    a = pcg32_random_r(&rng);
    b = pcg32_random_r(&rng);
    return (a << 32) | b;
}

uint64_t random_magic(void) {
    return random_u64() & random_u64() & random_u64();
}

uint64_t rook_mask(int square) {
    uint64_t result = 0ULL;
    int r, f;
    int rank = square / 8;
    int file = square % 8;

    for (r = rank + 1; r < 7; r++)
        result |= 1ULL << (r * 8 + file);
    for (r = rank - 1; r > 0; r--)
        result |= 1ULL << (r * 8 + file);
    for (f = file + 1; f < 7; f++)
        result |= 1ULL << (rank * 8 + f);
    for (f = file - 1; f > 0; f--)
        result |= 1ULL << (rank * 8 + f);

    return result;
}

uint64_t rook_moves_iter(int square, uint64_t blockers) {
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

uint64_t bishop_mask(int square) {
    uint64_t result = 0ULL;
    int r, f;
    int rank = square / 8;
    int file = square % 8;

    for (r = rank + 1, f = file + 1; r < 7 && f < 7; r++, f++)
        result |= 1ULL << (r * 8 + f);
    for (r = rank - 1, f = file + 1; r > 0 && f < 7; r--, f++)
        result |= 1ULL << (r * 8 + f);
    for (r = rank + 1, f = file - 1; r < 7 && f > 0; r++, f--)
        result |= 1ULL << (r * 8 + f);
    for (r = rank - 1, f = file - 1; r > 0 && f > 0; r--, f--)
        result |= 1ULL << (r * 8 + f);

    return result;
}

uint64_t bishop_moves_iter(int square, uint64_t blockers) {
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

int idx(uint64_t blockers, MagicEntry *entry) {
    blockers &= entry->mask;
    blockers *= entry->magic;
    blockers >>= entry->shift;
    return blockers;
}

MagicEntry *find_magic(int square, int index_bits, int bishop) {
    uint64_t mask = bishop ? bishop_mask(square) : rook_mask(square);
    int shift = 64 - index_bits;
    uint64_t *table = malloc(sizeof(uint64_t) * (1 << index_bits));
    MagicEntry *entry = malloc(sizeof(MagicEntry));
    entry->att = table;
    entry->mask = mask;
    entry->shift = shift;

    while (1) {
        entry->magic = random_magic();
        int fail = 0;

        for (int i = 0; i < 1 << index_bits; i++) {
            table[i] = 0;
        }

        uint64_t blockers = 0ULL;
        do {
            uint64_t moves = bishop ? bishop_moves_iter(square, blockers)
                                    : rook_moves_iter(square, blockers);
            int magic_i = idx(blockers, entry);
            if (table[magic_i] == 0ULL)
                table[magic_i] = moves;
            else if (table[magic_i] != moves) {
                // printf("destructive collision\n");
                fail = 1;
                break;
            }

            blockers = (blockers - mask) & mask;
        } while (blockers);

        if (!fail) return entry;
    }
}

void print_bitboard(uint64_t bb) {
    for (int i = 7; i >= 0; i--) {
        int pieces = bb >> (i * 8);
        // clang-format off
        printf("%d %d %d %d %d %d %d %d\n",
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
    printf("\n");
}

void free_magic(MagicEntry *magic) {
    free(magic->att);
    free(magic);
}

// clang-format off
const int ROOK_INDEX_BITS[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12,
};
const int BISHOP_INDEX_BITS[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6,
};
// clang-format on

void write_magic(FILE *restrict fp, MagicEntry *magics[64],
                 const char *array_name) {
    fprintf(fp, "static MagicEntry %s[64] = {\n", array_name);
    for (int i = 0; i < 64; i++) {
        fprintf(fp,
                "    { .att = 0, .mask = 0x%" PRIx64 ", .magic = 0x%" PRIx64
                ", .shift = "
                "%d },\n",
                magics[i]->mask, magics[i]->magic, magics[i]->shift);
    }
    fprintf(fp, "};\n");
}

void write_magics(FILE *restrict fp, MagicEntry *rook_magics[64],
                  MagicEntry *bishop_magics[64]) {
    fprintf(fp, "#ifndef MAGICS_H\n"
                "#define MAGICS_H\n\n"
                "#include <inttypes.h>\n\n"
                "typedef struct {\n"
                "    uint64_t *att;\n"
                "    uint64_t mask;\n"
                "    uint64_t magic;\n"
                "    uint8_t shift;\n"
                "} MagicEntry;\n\n");
    write_magic(fp, rook_magics, "ROOK_MAGICS");
    fprintf(fp, "\n");
    write_magic(fp, bishop_magics, "BISHOP_MAGICS");
    fprintf(fp, "\n#endif // MAGICS_H\n");
}

int main(void) {
    pcg32_srandom_r(&rng, time(NULL), (intptr_t)&rng);

    MagicEntry *rook_magics[64];
    MagicEntry *bishop_magics[64];

    for (int square = 0; square < 64; square++) {
        rook_magics[square] = find_magic(square, ROOK_INDEX_BITS[square], 0);
        printf("%d: 0x%" PRIx64 "\n", square, rook_magics[square]->magic);
    }
    for (int square = 0; square < 64; square++) {
        bishop_magics[square] =
            find_magic(square, BISHOP_INDEX_BITS[square], 1);
        printf("%d: 0x%" PRIx64 "\n", square, bishop_magics[square]->magic);
    }

    FILE *restrict fp = fopen("./src/gen/magics.h", "w");

    if (fp == NULL) {
        printf("Failed to open file 'gen/magics.h'.");
        return 1;
    }

    write_magics(fp, rook_magics, bishop_magics);

    fclose(fp);

    for (int i = 0; i < 64; i++) {
        free_magic(rook_magics[i]);
        free_magic(bishop_magics[i]);
    }

    return 0;
}
