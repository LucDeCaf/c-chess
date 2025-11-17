#ifndef MAGIC_H
#define MAGIC_H

#include <inttypes.h>

typedef struct {
    uint64_t *att;
    uint64_t mask;
    uint64_t magic;
    uint8_t shift;
} MagicEntry;

#endif // MAGIC_H
