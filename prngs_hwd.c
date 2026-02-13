
/*!	
	\see https://github.com/AnatolyGeorgievski/MWC128
	\see https://en.wikipedia.org/wiki/Multiply-with-carry_pseudorandom_number_generator


	[2] Blackman, David; Vigna, Sebastiano (2018). "Scrambled Linear Pseudorandom Generators". arXiv:1805.01407 [cs.DS].

Сборка
	$ gcc -O3 -march=native -DHWD_BITS=32  -o hwd hwd.c
 */
#if (HWD_BITS==32)
//#define next mwc64r0_next
#define next mwc64r2_next
//#define next mwc32x2_next
#else
#define next mwc128_next
#endif
static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}
static inline uint32_t rotl32(const uint32_t x, int k) {
	return (x << k) | (x >> (32 - k));
}

/*!
	
 */
#include <stdint.h>

static uint64_t s[2] = { 123, -1ULL };

#define MWC_A1 		0xffebb71d94fcdaf9ull // MWC128  число A1, B1 = (1<<64)
typedef unsigned int __attribute__((mode(TI)))   uint128_t;
#if 0
#define MWC_A1 		0xff3a275c007b8ee6 // MWC128 - другой вариант, см википедию
#endif

/* Marsaglia multiply-with-carry Random Number Generator */
/* uint64_t x, c -- состояние 

Выражение x = (x+c*b)*A mod A*b-1 == x*A + c
Доказательство:
(x+c*b)*A  = x*A + c*(A*b-1) + c
Средняя часть обращается в ноль, потому что кратно модулю. 

*/
static inline uint64_t mwc128_next() {
	static uint64_t s[2] = { 123, -1ULL };
	const uint64_t result = s[0];
	const uint128_t t = (uint128_t)MWC_A1 * s[0] + s[1];
	s[0] = t;
	s[1] = t >> 64;
	return result;
}
static inline uint64_t mwc128x2_next() {
	static uint64_t s[2] = { 123, -1ULL };
	const uint64_t x = s[0];
	const uint128_t t = (uint128_t)MWC_A1 * s[0] + s[1];
	s[0] = t;
	s[1] = t >> 64;
	return x^(rotl(~x,1)&rotl(x,2));
}
/*! \brief Генерация псевдо-случайного числа. Один шаг алгоритма */
#define MWC64_A0 0xFFFEB81BuLL
#define MWC64_A1 0xfffebaebuLL
static inline uint64_t mwc64x_next()
{
	static uint64_t s[2] = { 123, -1ULL };
	uint64_t x = *s;
	x  = MWC64_A0*(uint32_t)(x) + (x>>32);
	*s = MWC64_A0*(uint32_t)(x) + (x>>32);
    return x^(x>>32);// ^ (x&0xFFFFFFFFU));
}
static inline uint64_t mwc64x2_next()
{
	static uint64_t s[2] = { 1, -1ULL };
	uint64_t x = s[0];
	uint64_t c = s[1];
	x = MWC64_A0*(uint32_t)(x) + (x>>32);
	c = MWC64_A1*(uint32_t)(c) + (c>>32);
	s[0] = MWC64_A0*(uint32_t)(x) + (x>>32);
	s[1] = MWC64_A1*(uint32_t)(c) + (c>>32);
    return x^c;
}
static inline uint32_t mwc32x2_next()
{
	const uint32_t A1 = 0xFFEA; 
	const uint32_t A2 = 0xFF94;
	static uint32_t s[2] = { 1, ~1 };
	uint32_t x = s[0];
	uint32_t c = s[1];
	x = A1*(uint16_t)(x) + (x>>16);
	c = A2*(uint16_t)(c) + (c>>16);
	s[0] = A1*(uint16_t)(x) + (x>>16);
	s[1] = A2*(uint16_t)(c) + (c>>16);
    return x^c;
	
}

static inline uint32_t mwc64r0_next() {
	const uint32_t A0 = 0xfffebaebuLL;
	static uint32_t state[2] = { 1, ~1};
    uint32_t r = state[0];// ^state[1];
    uint64_t t = (uint64_t)A0*state[0] + state[1];
	state[0] = t;
	state[1] = t>>32;
    return r;
}
static inline uint32_t mwc64r2_next() {
	const uint32_t A2 = 0xffe118ab;
	static uint32_t state[4] = { ~1, ~1, ~1, 1};
    uint32_t r = state[2];// ^state[3];
    uint64_t t = (uint64_t)A2*state[0] + state[3];
	state[0] = state[1];
	state[1] = state[2];
	state[2] = t;
	state[3] = t>>32;
    return r;
}
static inline uint32_t xoroshiro64s_next() {
	const uint32_t s0 = s[0];
	uint32_t s1 = s[1];
	const uint32_t result = s0 * 0x9E3779BB;

	s1 ^= s0;
	s[0] = rotl32(s0, 26) ^ s1 ^ (s1 << 9); // a, b
	s[1] = rotl32(s1, 13); // c

	return result;
}
static inline uint32_t xoroshiro64ss_next() {
 	const uint32_t s0 = s[0];
	uint32_t s1 = s[1];
	const uint32_t result = rotl32(s0 * 0x9E3779BB, 5) * 5;

    s1 ^= s0;
	s[0] = rotl32(s0, 26) ^ s1 ^ (s1 << 9); // a, b
	s[1] = rotl32(s1, 13); // c
	return result;
}
/* Sample file for hwd.c (xoroshiro128+). */



static inline uint64_t xoroshiro128p_next() {
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];

	const uint64_t result_plus = s0 + s1;

	s1 ^= s0;
	s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16);
	s[1] = rotl(s1, 37);

	return result_plus;
}

static inline uint64_t xoroshiro128pp_next()
{
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = rotl(s0 + s1, 17) + s0;
	s1 ^= s0;
	s[0] = rotl(s0, 49) ^ s1 ^ (s1 << 21); // a, b
	s[1] = rotl(s1, 28); // c
	return r;
}
static inline uint64_t xoroshiro128ss_next()
{
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = rotl(s0 * 5, 7) * 9;

	s1 ^= s0;
	s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
	s[1] = rotl(s1, 37); // c
	return r;
}
