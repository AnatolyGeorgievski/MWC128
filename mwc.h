#ifndef _MWC_H
#define _MWC_H
#include <stdint.h>

typedef struct _PRNG PRNG_t;

struct _PRNG {
    double   (*u01 )(uint64_t *state);
    uint64_t (*next)(uint64_t *state);
    uint64_t (*prev)(uint64_t *state);
    uint64_t (*jump)(uint64_t *state, int distance);
    const char *name;
    uint64_t  state[0];
};
// набор тестов
PRNG_t * mwc_gen_new(int bits, uint64_t seed);
double clz_test(const char* name, uint64_t (*next)(uint64_t *state));
double dif_test(const char* name, uint64_t (*next)(uint64_t *state));

#endif //_MWC_H