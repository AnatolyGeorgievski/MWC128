/*! 
Сборка
	$ gcc -march=native -O3 -o mwc64x_clz mwc64x_clz.c 
    $ gcc -DTEST_CLZ -march=native -O3 -o mwc_clz mwc_clz.c

Результаты
----------------------------------423
mwc64,A=FFFEF0E2 30.3 (44.3) 76.3
mwc64,A=FFFECC43 43.6 (44.4) 77.0
mwc64,A=FFFE7369 36.6 (44.7) 74.7
mwc64,A=FFFE60AF 39.1 (43.7) 76.0
mwc64,A=FFFE5BD5 32.6 (44.8) 77.7
mwc64,A=FFFE5A0A 43.2 (45.1) 83.8
mwc64,A=FFFE53E3 49.0 (43.9) 69.0
MWC128           44.9 (43.8)
xoroshiro128++   34.9 (44.1)

 */
#include <stdint.h>
#include <stdio.h>
static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) ^ (x >> (64 - k));
}
static inline uint64_t rotr(const uint64_t x, int k) {
	return (x << (64 - k)) ^ (x >> k);
}
// Scramblers
static uint64_t count_next(){
	static uint64_t x = 0;
	++x;
	return x;
}
static uint64_t gray_next(){
	static uint64_t x = ~0;
	++x;
	return x^rotl(x,63);
}
static uint64_t scrambler_ss_next(){
	static uint64_t x = ~0;
	++x;
	return rotl(x * 5, 7) * 9;
}
static uint64_t scrambler_1_next(){
	static uint64_t x = ~0;
	++x;
	return x^(rotl(~x,1)&rotl(x,2));
}
static uint64_t scrambler_1b_next(){
	static uint64_t x = ~0;
	++x;
	return x^(rotl(~x,8)&rotl(x,16));
}

static uint64_t scrambler_2_next(){
	static uint64_t x = ~0;
	++x;
	return x^(rotl(x,1)^rotl(x,2));
}
static uint64_t scrambler_2b_next(){
	static uint64_t x = ~0;
	++x;
	return x^(rotl(x,8)^rotl(x,16));
}
static uint64_t scrambler_3_next(){
	static uint64_t x = ~0;
	++x;
	return x^(rotl(~x,1)&rotl(x,2)&rotl(x,3));
}
static uint64_t scrambler_3b_next(){
	static uint64_t x = ~0;
	++x;
	return x^(rotl(~x,8)&rotl(x,16)&rotl(x,24));
}
static uint64_t scrambler_sigma0_next(){
	static uint64_t x = ~0;
	++x;
	return rotr(x,1)^rotr(x,8)^(x>>7);
}
static uint64_t scrambler_sigma1_next(){
	static uint64_t x = ~0;
	++x;
	return rotr(x,19)^rotr(x,61)^(x>>6);
}
static uint64_t scrambler_sum0_next(){
	static uint64_t x = ~0;
	++x;
	return rotr(x,28)^rotr(x,34)^rotr(x, 39);
}
static uint64_t scrambler_sum1_next(){
	static uint64_t x = 0;
	++x;
	return rotr(x,14)^rotr(x,18)^rotr(x, 41);
}
static uint64_t scrambler_p_next(){
	static uint64_t x = 0;
	++x;
	return x + rotl(x,32);
}
#if 0
extern void sha256d(uint8_t *hash, const uint8_t *data, unsigned int len);
uint8_t hash[32];
uint8_t midstate[32] = {1};
double  diff_sum = 0;
void digest_register(){}
static uint64_t sha256d_next(){
	*(uint32_t*)(midstate+28) = *(uint32_t*)(midstate+28)+1;
	sha256d(hash, (uint8_t *)midstate, 32);
	return *(uint64_t *)(hash+18);
}
#endif
/*! \brief Генерация псевдо-случайного числа. Один шаг алгоритма */
#define MWC_A0
uint64_t a0 = 0xfffeb81bULL;
uint32_t aa[] = {
    0xFFFEFD4E,0xFFFEF0E2,0xFFFECC43,0xFFFEA405,0xFFFE7369,
    0xFFFE60AF,0xFFFE5BD5,0xFFFE5A0A,0xFFFE53E3,0xFFFE03C4,
};

//#define MWC_A0 0xfffebaebULL
uint64_t state[2] = {-1, 1};
uint64_t mwc64x_next()
{
	uint64_t x = *state;
	*state = a0*(uint32_t)(x) + (x>>32);
    return x;//((x>>32) ^ (x));
}
#define MWC_A1 		0xffebb71d94fcdaf9ull // MWC128  число A1, B1 = (1<<64)
//#define MWC_A1 		0xff3a275c007b8ee6 // MWC128 - другой вариант, см википедию
typedef unsigned int __attribute__((mode(TI)))   uint128_t;
static inline uint64_t mwc128_next() {
	static uint64_t s[2] = {-1, 1};
	const uint64_t result = s[0];
	const uint128_t t = (uint128_t)MWC_A1 * s[0] + s[1];
	s[0] = t;
	s[1] = t >> 64;
	return result;
}
static inline uint64_t mwc128x1b_next() {
	static uint64_t s[2] = {-1, 1};
	const uint64_t x = s[0];
	const uint128_t t = (uint128_t)MWC_A1 * s[0] + s[1];
	s[0] = t;
	s[1] = t >> 64;
	return x^(rotl(~x,8)&rotl(x,16));
}
/* 2019 by David Blackman and Sebastiano Vigna */
uint64_t s[2] = {-1, 1};
static inline uint32_t rotl32(const uint32_t x, int k) {
	return (x << k) | (x >> (32 - k));
}
static uint64_t xoroshiro64s_next() {
    static uint32_t s[2] = {1,-1};
	const uint32_t s0 = s[0];
	uint32_t s1 = s[1];
	const uint32_t result = s0 * 0x9E3779BB;

	s1 ^= s0;
	s[0] = rotl32(s0, 26) ^ s1 ^ (s1 << 9); // a, b
	s[1] = rotl32(s1, 13); // c

	return result;
}
static uint64_t xoroshiro64ss_next() {
    static uint32_t s[2] = {1,-1};
 	const uint32_t s0 = s[0];
	uint32_t s1 = s[1];
	const uint32_t result = rotl32(s0 * 0x9E3779BB, 5) * 5;

    s1 ^= s0;
	s[0] = rotl32(s0, 26) ^ s1 ^ (s1 << 9); // a, b
	s[1] = rotl32(s1, 13); // c
	return result;
}
static uint64_t xoroshiro128p_next()
{
    static uint64_t s[2] = {1,-1};
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = s0 + s1;
	s1 ^= s0;
	s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
	s[1] = rotl(s1, 37); // c
	return r;	
}
static uint64_t xoroshiro128pp_next()
{
    static uint64_t s[2] = {1,-1};
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = rotl(s0 + s1, 17) + s0;
	s1 ^= s0;
	s[0] = rotl(s0, 49) ^ s1 ^ (s1 << 21); // a, b
	s[1] = rotl(s1, 28); // c
	return r;
}
static uint64_t xoroshiro128ss_next()
{
    static uint64_t s[2] = {1,-1};
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = rotl(s0 * 5, 7) * 9;

	s1 ^= s0;
	s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
	s[1] = rotl(s1, 37); // c
	return r;
}
#include <math.h>
/* Алгоритм состоит из двух частей, можно запускать только одну */ 
double clz_test(const char* name, uint64_t (*next)()) {
    uint64_t hist0[64]={0};
    uint64_t count = 1uLL<<32;
    do {
        uint64_t x = next();
        int i = __builtin_clzll(x);
        hist0[i]++;
    } while (--count);
    int N = 32; 
    while (hist0[N-1]==0) N--;
    uint64_t n =0;
    for (int i=0; i<N; i++)
        n += hist0[i];
    double P=0.5, chi2=0;
    for (int i=0; i<N && n*P>=112; i++, P/=2) {
        double v = (hist0[i] - n*P);
        v = v*v/((n*P));
        chi2 += v;
    }
	return chi2;
}
#if 1//defined(TEST_CLZ)
int main(){
    struct {
        const char* name;
        uint64_t (*next)();
    } gen[] = {
//        {"xoroshiro64*",   xoroshiro64s_next},
//        {"xoroshiro64**",  xoroshiro64ss_next},
        {"xoroshiro128**", xoroshiro128ss_next},
        {"xoroshiro128+",  xoroshiro128p_next},
        {"xoroshiro128++", xoroshiro128pp_next},
        {"MWC128", mwc128_next},
    };

    char name[64];
	uint32_t mask[8] = {0};
	double avg[128] = {0};
	double max[128] = {0};
	double avgx=0,avgm=0,avgs=0;
	double maxs=0;
	int count = 512;
	int n=0;
	int sz = sizeof(aa)/sizeof(uint32_t);
    int n_gen = sizeof(gen)/sizeof(gen[0]);
for(int k=16;k<count; k++){
	++n;
	printf ("----------------------------------%d\n", k);
	if (1) for (int i=0;i<sz; i++) {
		if (mask[i>>5]&(1u<<(i&31))) continue;
		a0 = aa[i];
		state[0] %= ((a0<<32) - 1);
		double chi2 = clz_test(name, mwc64x_next);
		avg[i]+=chi2;
		if (chi2>max[i])  max[i] = chi2;
		sprintf(name, "mwc64,A=%08X %.1f (%.1f) %.1f", a0, chi2, avg[i]/n, max[i]);
		puts(name);
		if (n>3 && avg[i]>27*n) mask[i>>5]|=(1u<<(i&31)) ;
	}
    for(int i=0; i<n_gen; i++) {
        double chi2 = clz_test(gen[i].name, gen[i].next);
	    avg[i+sz]+=chi2;
        if (chi2>max[i+sz]) max[i+sz] = chi2;
	    printf("%-16s %.1f (%.1f) %.1f\n", gen[i].name, chi2, avg[i+sz]/n, max[i+sz]);
    }
#if 0
		*(uint32_t*)midstate = 1u<<(k&31);
		diff_sum = 0;
		chi2 = clz_test("sha256d", sha256d_next);
		avgs+=chi2;
		if (chi2>maxs)  maxs = chi2;
		sprintf(name, "sha256d          %.1f (%.1f) %.1f diff=%.1f", chi2, avgs/n, maxs, diff_sum);
		puts(name);
#endif
}
   	return 0;
}
#endif