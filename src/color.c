#include "color.h"

Color color_inverse(Color color) {
    return color ^ 1;
}

// -1 for white, 1 for black
// Useful for pawn calculations
int color_direction(Color color) {
    return 1 - (color * 2);
}
