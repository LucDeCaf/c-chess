/**
 * The pawn masks didn't include the first and last ranks and I
 * need them to for square_attacked_by.
 */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

void print_pawns(int color) {
    int square = 0;

    if (!color) {
        for (; square < 8; square++) {
            printf("0x0\n");
        }
    }

    for (; square < (color ? 56 : 64); square++) {
        uint64_t mask = 0ULL;
        int file = square % 8;
        if (color) {
            if (file != 0) mask |= (1ULL << (square + 7));
            if (file != 7) mask |= (1ULL << (square + 9));
        } else {
            if (file != 0) mask |= (1ULL << (square - 9));
            if (file != 7) mask |= (1ULL << (square - 7));
        }
        printf("0x%" PRIx64 "\n", mask);
    }

    if (color) {
        for (; square < 64; square++) {
            printf("0x0\n");
        }
    }
}

int main(void) {
    printf("WHITE\n");
    print_pawns(1);
    printf("BLACK\n");
    print_pawns(0);

    return 0;
}
