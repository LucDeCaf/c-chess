#ifndef PCG_BASIC_H
#define PCG_BASIC_H

#include <inttypes.h>

struct pcg_state_setseq_64 {
    uint64_t state;
    uint64_t inc;
};
typedef struct pcg_state_setseq_64 pcg32_random_t;

void pcg32_srandom_r(pcg32_random_t *rng, uint64_t initstate, uint64_t initseq);
uint32_t pcg32_random_r(pcg32_random_t *rng);

#endif // PCG_BASIC_H
