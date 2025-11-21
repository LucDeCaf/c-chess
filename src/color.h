#ifndef COLOR_H
#define COLOR_H

typedef enum {
    Black = 0,
    White,
} Color;

inline Color color_inverse(Color c) {
    return (Color)(c ^ 1);
}

inline int color_direction(Color c) {
    return 1 - (2 * c);
}

#endif // COLOR_H
