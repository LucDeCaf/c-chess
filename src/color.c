#include "color.h"

Color color_inverse(Color c) {
    return (Color)(c ^ 1);
}

int color_direction(Color c) {
    return 1 - (2 * c);
}
