#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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
static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) ^ (x >> (64 - k));
}
uint64_t xoroshiro128_next(uint64_t *s)
{
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = s0 + s1;
	s1 ^= s0;
	s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
	s[1] = rotl(s1, 37); // c
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
double sac(uint64_t (*mixer) (uint64_t), double * mean_bias, int expNr)
{
    uint64_t Nr = 1uLL<<expNr;
    uint64_t counts[BITS][BITS] = {0};
    uint64_t seed[2] = {0x517cc1b727220a94ull,1};  // любой хороший seed

    for (uint64_t trial = 0; trial < Nr; trial++) {
        uint64_t x  = xoroshiro128_next(seed);   // или mwc128...
//        uint64_t x  = mwc128(seed);   // или mwc128...
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
            double p    = (double)counts[i][j] / Nr;
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

static uint64_t xorshift64s1_A3_mix(uint64_t x) {
	x ^= x >> 19; // c
	x ^= x << 29; // b
	x ^= x >>  8; // a
	return x *
     UINT64_C(0xd605bbb58c8abbfd);
}
bool has_max_order(uint64_t x) {
    int count = 61; // 2^61
    do { 
		x = x * x; // x ← x² mod 2⁶⁴
		if (x <= 1) return false; 
	} while (--count);
    return true;
}

static uint64_t inverse_u64(uint64_t a) {
    uint64_t x = a;
    // 5 итераций — стандарт для 64 бит
    x *= 2ULL - a * x;
    x *= 2ULL - a * x;
    x *= 2ULL - a * x;
    x *= 2ULL - a * x;
    x *= 2ULL - a * x;
    return x;
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
{"M^588", 32,0xe30eea38586e792du,29, 0xc6f5a347bf1f3ea5u,32},
{"M^1584", 32,0xea35f4b8342332bdu,29, 0x86b34afd4cc4d895u,32},  // |  0.000476 |  0.000377 | >
{"M^2488", 32,0xc04ea3044f187bddu,29, 0x9db95f65be18b475u,32},  // |  0.000472 |  0.000376 | min

{"Custom 1",    32, 0xbf58476d1ce4e5b9, 28, 0x94d049bb133111eb, 33},// mix13
{"Custom LXM2", 32, 0xd605bbb58c8abbfd, 29, 0xad4d2974b2f97955, 32},//
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

    printf("SAC  (N=2^%.0f):\n", 22);
    double mean_bias, max_bias;
	printf("Prime1 Prime2\n");

    struct _op xs= {.a=32, .b=29, .c= 32};
    char name[16]="";
    xs.name = name;
    m = &xs;
    double min_bias = 1;
	uint64_t g = 0xd605bbb58c8abbfd;
	uint64_t x=1;
	if (1) for (int i=0; i<63; i++){
		x = x*x;
        if (i<0) continue;
		uint64_t p1 = inverse_u64(x);
		if ((x>>62) ==3 && (int64_t)p1 < 0 && has_max_order(x) && has_max_order(p1)){
            printf("{\"M^%d\", 32,0x%016llxu,29, 0x%016llxu,32},\t", i,x, p1);
            sprintf(name, "M^%d", i);
            xs.prime1 = x, xs.prime2 = p1;
            max_bias = sac(mixer, &mean_bias, 20);
            int ok = mean_bias< min_bias;
            if (ok) min_bias = mean_bias;
            printf("// | %9.6f | %9.6f | %s\n", max_bias, mean_bias, ok? "min":">");
		}
	}
	printf ("..done\n");
    
    int sz = sizeof(op)/sizeof(struct _op);
    printf("|%-12s| %9s | %9s |\n", "Mixer", "Max bias","Mean bias");
    min_bias = 1;
    for (int i=0; i<sz;i++){
        m = op+i;
        max_bias = sac(mixer, &mean_bias, 22);
        int ok = mean_bias< min_bias;
        if (ok) min_bias = mean_bias;
        printf("|%-12s| %9.6f | %9.6f | %s\n", m->name, max_bias, mean_bias, ok? "min":">");
    }
    return 0;
}