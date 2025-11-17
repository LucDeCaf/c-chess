#include "square.h"

uint64_t square_mask(int square) {
    return 1ULL << square;
}

uint8_t square_rank(int square) {
    return square / 8;
}

uint8_t square_file(int square) {
    return square % 8;
}
