#ifndef COLOR_H
#define COLOR_H

typedef enum {
    Black = 0,
    White,
} Color;

Color color_inverse(Color c);
int color_direction(Color c);

#endif // COLOR_H
