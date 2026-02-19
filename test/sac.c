#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define N      (1ULL << 22)     // 1M проб — хороший баланс скорость/точность
#define BITS   64

// Пример миксера (замени на свой)
static uint64_t splitmix64(uint64_t *state) {
    uint64_t z = (*state += 0x9e3779b97f4a7c15ull);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    return z ^ (z >> 31);
}
// Doug Lea's mixing function, fastmix дважды
static inline uint64_t mix_lea(uint64_t h) {
    const uint64_t C = 0xdaba0b6eb09322e3;
//    const uint64_t C = 0x9fb21c651e98df25L;
  h ^= h >> 32;
  h *= C;
  h ^= h >> 28;
  h *= C;
  h ^= h >> 33;
  return h;
}
static inline uint64_t unmix_lea(uint64_t h) {
    const uint64_t C = 0xa6f8e26927e132cb;
  h ^= h >> 32;
  h *= C;
  h ^= h >> 32;
  h *= C;
  h ^= h >> 32;
  return h;
}

static inline uint64_t mix_stafford13(uint64_t h) {
  h ^= h >> 30;
  h *= 0xbf58476d1ce4e5b9ull;
  h ^= h >> 27;
  h *= 0x94d049bb133111ebull;
  h ^= h >> 31;
  return h;
}


#define A0 0xffebb71d94fcdaf9ull
//#define A0 0xff3a275c007b8ee6ull
uint64_t mwc128(uint64_t* s){
    uint64_t r = s[0]^s[1];
    unsigned __int128 t = (unsigned __int128)A0 * s[0] + s[1];
    s[0] = t;
    s[1] = t>>64;
    return r;
}

/*!  \brief измерение среднеквадратичного эффекта 

uint64_t next(uint64_t *state, uint64_t gamma) {
  uint64_t s = *state;
  *state += gamma;
  return mix(s);
}
// This is fed to TestU01 by
static void runTestsInt(unsigned int (*prng)(void), char name[]) {
  unif01_Gen* gen = unif01_CreateExternGenBits(name, prng);
  bbattery_BigCrush(gen);

  unif01_DeleteExternGenBits(gen);
}
 */
double sac(uint64_t (*mixer) (uint64_t), double * mean_bias)
{
    uint64_t counts[BITS][BITS] = {0};
    uint64_t seed[2] = {0x517cc1b727220a94ull,1};  // любой хороший seed

    for (uint64_t trial = 0; trial < N; trial++) {
        uint64_t x  = mwc128(seed);   // или mwc128...
        uint64_t hx = mixer(x);

        for (int i = 0; i < BITS; i++) {
            uint64_t xi  = x ^ (1ULL << i);
            uint64_t hxi = mixer(xi);
            uint64_t diff = hx ^ hxi;

            for (int j = 0; j < BITS; j++) {
                if (diff & (1ULL << j))
                    counts[i][j]++;
            }
        }
    }

    double max_bias  = 0.0;
    double sum_bias  = 0.0;
    double sum_sqr   = 0.0;

    int total = 0;

    for (int i = 0; i < BITS; i++) {
        for (int j = 0; j < BITS; j++) {
            double p    = (double)counts[i][j] / N;
            double bias = (p - 0.5);
            sum_bias += fabs(bias);
            sum_sqr  += bias*bias;
            if (bias > max_bias) max_bias = bias;
            total++;
        }
    }

    *mean_bias = sum_bias / total;
//    return max_bias;
    return sqrt(sum_sqr / total);
}

//#define mixer mix_stafford13
int main(void) 
{
	struct _op {
		const char* name;
		int a;
		uint64_t prime1;
		int b;
		uint64_t prime2;
		int c;
	} op[] = {
{"Custom 1",    32, 0xbf58476d1ce4e5b9, 28, 0x94d049bb133111eb, 33},
{"Custom2",     32, 0xdaba0b6eb09322e3, 33, 0xa6f8e26927e132cb, 32},
{"SplitMix64",  30, 0xbf58476d1ce4e5b9, 27, 0x94d049bb133111eb, 31},
//{"MurmurHash3", 33, 0xff51afd7ed558ccd, 33, 0xc4ceb9fe1a85ec53, 33},
{"Custom2",     32, 0xda942042e4dd58b5, 32, 0xda942042e4dd58b5, 32},
{"Doug Lea's",  32, 0xdaba0b6eb09322e3, 32, 0xdaba0b6eb09322e3, 32},
{"unmix Lea's", 32, 0xa6f8e26927e132cb, 32, 0xa6f8e26927e132cb, 32},
{"Avalanche",   33, 0xC2B2AE3D27D4EB4F, 29, 0x165667B19E3779F9, 32},
{"Mix3",        32, 0xbea225f9eb34556d, 29, 0xbea225f9eb34556d, 32},

	{"Mix01",31 ,0x7fb5d329728ea185 ,27 ,0x81dadef4bc2dd44d ,33},
	{"Mix02",33	,0x64dd81482cbd31d7	,31	,0xe36aa5c613612997	,31},
	{"Mix03",31	,0x99bcf6822b23ca35	,30	,0x14020a57acced8b7	,33},
	{"Mix04",33	,0x62a9d9ed799705f5	,28	,0xcb24d0a5c88c35b3	,32},
	{"Mix05",31	,0x79c135c1674b9add	,29	,0x54c77c86f6913e45	,30},
	{"Mix06",31	,0x69b0bc90bd9a8c49	,27	,0x3d5e661a2a77868d	,30},
	{"Mix07",30	,0x16a6ac37883af045	,26	,0xcc9c31a4274686a5	,32},
	{"Mix08",30	,0x294aa62849912f0b	,28	,0x0a9ba9c8a5b15117	,31},
	{"Mix09",32	,0x4cd6944c5cc20b6d	,29	,0xfc12c5b19d3259e9	,32},
	{"Mix10",30	,0xe4c7e495f4c683f5	,32	,0xfda871baea35a293	,33},
	{"Mix11",27	,0x97d461a8b11570d9	,28	,0x02271eb7c6c4cd6b	,32},
	{"Mix12",29	,0x3cd0eb9d47532dfb	,26	,0x63660277528772bb	,33},
	{"Mix13",30	,0xbf58476d1ce4e5b9	,27	,0x94d049bb133111eb	,31},
	{"Mix14",30	,0x4be98134a5976fd3	,29	,0x3bc0993a5ad19a13	,31},
	};
    struct _op *m = NULL;
    uint64_t mixer(uint64_t x){
        x ^= x>>m->a; x*=m->prime1;
        x ^= x>>m->b; x*=m->prime2;
        x ^= x>>m->c;
        return x;
    }

    printf("SAC  (N=2^%.0f):\n", log2(N));
    double mean_bias, max_bias;
    int sz = sizeof(op)/sizeof(struct _op);
    printf("|%-12s| %9s | %9s |\n", "Mixer", "Max bias","Mean bias");
    for (int i=0; i<sz;i++){
        m = op+i;
        max_bias = sac(mixer, &mean_bias);
        printf("|%-12s| %9.6f | %9.6f |\n", m->name, max_bias, mean_bias);
    }
    return 0;
}