#define next mwc128_next

/*!

https://en.wikipedia.org/wiki/Multiply-with-carry_pseudorandom_number_generator

	$ gcc -DTEST_MWC128 -O3 -march=native -o test mwc128.c
	
	
 [2] Blackman, David; Vigna, Sebastiano (2018). "Scrambled Linear Pseudorandom Generators". arXiv:1805.01407 [cs.DS].
 */
#include <stdint.h>

static uint64_t s[2] = { 123, -1ULL };

#define MWC_A1 		0xffebb71d94fcdaf9ull // MWC128  число A1, B1 = (1<<64)
typedef unsigned int __attribute__((mode(TI)))   uint128_t;
#if 0
#define MWC_A1 		0xff3a275c007b8ee6 // MWC128 - другой вариант, см википедию
#endif
/* Модуль M1 = A1*B1-1 
	выполняется тождество A1 = B1^{-1} mod M1 B1 - обратное число для A1
 */
static const uint64_t MWC_M1[2] = { 0xffffffffffffffff, MWC_A1 - 1 };
static const uint64_t MWC128_jump64[2] = { 0xa72f9a3547208003, 0x2f65fed2e8400983 };
static const uint64_t MWC128_jump96[2] = { 0xe6f7814467f3fcdd, 0x394649cfd6769c91 };
/* Marsaglia multiply-with-carry Random Number Generator */
/* uint64_t x, c -- состояние 

Выражение x = (x+c*b)*A mod A*b-1 == x*A + c
Доказательство:
(x+c*b)*A  = x*A + c*(A*b-1) + c
Средняя часть обращается в ноль, потому что кратно модулю. 

*/
static inline uint64_t mwc128_next() {
	const uint64_t result = s[0];
	const uint128_t t = (uint128_t)MWC_A1 * s[0] + s[1];
	s[0] = t;
	s[1] = t >> 64;
	return result;
}
/*! \brief Генерация псевдо-случайного числа. Один шаг алгоритма */
#define MWC64_A0 0xFFFEB81Bu
static inline uint64_t mwc64x_next()
{
	uint64_t x = *s;
	*s = MWC64_A0*(uint32_t)(x) + (x>>32);
    return ((x>>32) ^ (x&0xFFFFFFFFU));
}


static inline uint32_t rotl32(const uint32_t x, int k) {
	return (x << k) | (x >> (32 - k));
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

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}


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
