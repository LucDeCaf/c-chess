#include "../../../include/move.h"
#include "../../../include/square.h"

Move new_move(Square source, Square target, uint8_t flags) {
    return (source) | (target << 6) | (flags << 12);
}

Square move_source(Move move) {
    return move & 0b111111;
}

Square move_target(Move move) {
    return (move >> 6) & 0b111111;
}

uint8_t move_flags(Move move) {
    return move >> 12;
}
