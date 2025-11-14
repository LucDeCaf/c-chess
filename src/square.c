#include "square.h"

uint64_t square_mask(Square square) {
    return 1 << square;
}

uint8_t square_rank(Square square) {
    return square / 8;
}

uint8_t square_file(Square square) {
    return square % 8;
}
