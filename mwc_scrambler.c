/*! 
Сборка
	$ gcc -DTEST_SCRAMBLER -march=native -O3 -o mwc_scrambler mwc_scrambler.c 
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
	return x^(rotl(x,1)^rotl(x,2));
}
static uint64_t scrambler_1b_next(){
	static uint64_t x = ~0;
	++x;
	return x^(rotl(x,8)^rotl(x,16));
}
static uint64_t scrambler_2_next(){
	static uint64_t x = ~0;
	++x;
	return x^(rotl(~x,1)&rotl(x,2));
}
static uint64_t scrambler_2b_next(){
	static uint64_t x = ~0;
	++x;
	return x^(rotl(~x,8)&rotl(x,16));
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

/*! \brief Генерация псевдо-случайного числа. Один шаг алгоритма */
#define MWC_A0
uint64_t a0 = 0xfffeb81bULL;
uint32_t aa[] = {
    0xFFFEFD4E, 0xFFFEF0E2, 0xFFFECC43, 0xFFFEA405,
    0xFFFE7369, 0xFFFE60AF, 0xFFFE5BD5, 0xFFFE5A0A,
    0xFFFE53E3, 0xFFFE03C4,
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
static inline uint64_t xoroshiro64s_next() {
	const uint32_t s0 = s[0];
	uint32_t s1 = s[1];
	const uint32_t result = s0 * 0x9E3779BB;

	s1 ^= s0;
	s[0] = rotl32(s0, 26) ^ s1 ^ (s1 << 9); // a, b
	s[1] = rotl32(s1, 13); // c

	return result;
}
static inline uint64_t xoroshiro64ss_next() {
 	const uint32_t s0 = s[0];
	uint32_t s1 = s[1];
	const uint32_t result = rotl32(s0 * 0x9E3779BB, 5) * 5;

    s1 ^= s0;
	s[0] = rotl32(s0, 26) ^ s1 ^ (s1 << 9); // a, b
	s[1] = rotl32(s1, 13); // c
	return result;
}
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
#include <math.h>
static inline double difficulty(uint64_t x, double A) {
	return 1/fmax((double)x,0.5);
}
double dif_test(const char* name, uint64_t (*next)()) {
	_Float64x diffi = 0;
    uint64_t count = 1uLL<<32;
	double A = count;
    do {
		uint64_t x = next();
		//if ((x>>(64-0))==0) 
			diffi += difficulty(rotl(x,32)>>(32-8),A);
	} while(--count);
	return diffi;
}
#if defined(TEST_SCRAMBLER)
int main(){

    struct {
        const char* name;
        uint64_t (*next)();
    } gen[] = {
        {"Counter", count_next},
        {"Gray code", gray_next},
        {"Scrambler 1", scrambler_1_next},
        {"Scrambler 2", scrambler_2_next},
        {"Scrambler 3", scrambler_3_next},
        {"Scrambler 1b", scrambler_1b_next},
        {"Scrambler 2b", scrambler_2b_next},
        {"Scrambler 3b", scrambler_3b_next},
        {"Scrambler Sigma0", scrambler_sigma0_next},
        {"Scrambler Sigma1", scrambler_sigma1_next},
        {"Scrambler Sum0", scrambler_sum0_next},
        {"Scrambler Sum1", scrambler_sum1_next},

        {"Scrambler **", scrambler_ss_next},
        {"Scrambler +", scrambler_p_next},
        {"MWC64x", mwc64x_next},
        {"MWC128", mwc128_next},
        {"MWC128x1b", mwc128x1b_next},
        {"Xoroshiro128++", xoroshiro128pp_next},

    };
  if (0) // 
	for (int i=0; i<32; i++) {
		double x = difficulty(1uLL<<i, 0x1p+31);
		printf("%2d:diff = %.2f\n", i, x);
	}
  if (1) {// difficulty
	char* name = "MWC128";
	double x;
    int n_tests = sizeof(gen)/sizeof(gen[0]);
	for (int n=0;n<512;n++) {
		printf ("----------------------------------%d\n", n);
        for (int k=0;k<n_tests;k++) {
            x = dif_test(gen[k].name, gen[k].next);
            printf("|%-16s| %-9.4g\n", gen[k].name, x);
        }
	}
  }
   	return 0;
}
#endif