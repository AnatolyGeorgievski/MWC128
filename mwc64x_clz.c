#include <stdint.h>
#include <stdio.h>
/*! \brief Генерация псевдо-случайного числа. Один шаг алгоритма */
#define MWC_A0 0xfffeb81bULL
uint64_t state[2] = {-1, 1};
uint64_t mwc64x_next()
{
	uint64_t x = *state;
	*state = MWC_A0*(uint32_t)(x) + (x>>32);
    return ((x>>32) ^ (x));
}
static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

/* 2019 by David Blackman and Sebastiano Vigna */
uint64_t s[2] = {1, -1};
uint64_t xoroshiro128p_next()
{
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = s0 + s1;
	s1 ^= s0;
	s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
	s[1] = rotl(s1, 37); // c
	return r;	
}
uint64_t xoroshiro128pp_next()
{
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = rotl(s0 + s1, 17) + s0;
	s1 ^= s0;
	s[0] = rotl(s0, 49) ^ s1 ^ (s1 << 21); // a, b
	s[1] = rotl(s1, 28); // c
	return r;
}
uint64_t xoroshiro128ss_next()
{
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = rotl(s0 * 5, 7) * 9;

	s1 ^= s0;
	s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
	s[1] = rotl(s1, 37); // c
	return r;
}
int main(){

    uint64_t hist[64]={0};
    uint64_t count = 1uLL<<18;
    do {
        uint64_t x = xoroshiro128ss_next();
        int i = __builtin_clzll(x);
        hist[i]++;
    } while (--count);
    int N = 64; 
    while (hist[N-1]==0) N--;
    uint64_t n =0;
    for (int i=0; i<N; i++)
        n += hist[i];
    double P=0.5, chi2;
    for (int i=0; i<N && n*P>=8; i++, P/=2) {
        double v = (hist[i] - n*P);
        v = v*v/(n*P);
        printf("%2d, %10llu , %10.0f , %5.2f\n", i, hist[i], (n*P), v);
        chi2 += v;
    }
    printf("chi2 = %.1f\n", chi2);
    return 0;
}