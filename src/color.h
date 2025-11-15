#ifndef COLOR_H
#define COLOR_H

typedef enum {
    Black = 0,
    White,
} Color;

Color color_inverse(Color);
int color_direction(Color);

#endif // COLOR_H
