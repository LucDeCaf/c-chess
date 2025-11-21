#include "move.h"
#include <stdint.h>

Move new_move(int source, int target, uint8_t flags) {
    return (source) | (target << 6) | ((uint16_t)flags << 12);
}

int move_source(Move move) {
    return move & 63;
}

int move_target(Move move) {
    return (move >> 6) & 63;
}

uint8_t move_flags(Move move) {
    return move >> 12;
}
