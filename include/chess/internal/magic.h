#ifndef MAGIC_H
#define MAGIC_H

#include <stdint.h>

typedef struct {
    uint64_t mask;
    uint64_t magic;
    int shift;
    int offset;
} MagicEntry;

#endif // MAGIC_H
