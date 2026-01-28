/*  Written in 2016 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#include <stdint.h>

/* This is xoroshiro64* 1.0, our best and fastest 32-bit small-state
   generator for 32-bit floating-point numbers. We suggest to use its
   upper bits for floating-point generation, as it is slightly faster than
   xoroshiro64**. It passes all tests we are aware of except for linearity
   tests, as the lowest six bits have low linear complexity, so if low
   linear complexity is not considered an issue (as it is usually the
   case) it can be used to generate 32-bit outputs, too.

   We suggest to use a sign test to extract a random Boolean value, and
   right shifts to extract subsets of bits.

   The state must be seeded so that it is not everywhere zero. */

/* This is xoroshiro64** 1.0, our 32-bit all-purpose, rock-solid,
   small-state generator. It is extremely fast and it passes all tests we
   are aware of, but its state space is not large enough for any parallel
   application.

   For generating just single-precision (i.e., 32-bit) floating-point
   numbers, xoroshiro64* is even faster.

   The state must be seeded so that it is not everywhere zero. */


static inline uint32_t rotl32(const uint32_t x, int k) {
	return (x << k) | (x >> (32 - k));
}
uint32_t xoroshiro64s_next(uint32_t s[2]) {
	const uint32_t s0 = s[0];
	uint32_t s1 = s[1];
	const uint32_t result = s0 * 0x9E3779BB;

	s1 ^= s0;
	s[0] = rotl32(s0, 26) ^ s1 ^ (s1 << 9); // a, b
	s[1] = rotl32(s1, 13); // c

	return result;
}
uint32_t xoroshiro64ss_next(uint32_t s[2]) {
 	const uint32_t s0 = s[0];
	uint32_t s1 = s[1];
	const uint32_t result = rotl32(s0 * 0x9E3779BB, 5) * 5;

    s1 ^= s0;
	s[0] = rotl32(s0, 26) ^ s1 ^ (s1 << 9); // a, b
	s[1] = rotl32(s1, 13); // c
	return result;
}

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

/* 2019 by David Blackman and Sebastiano Vigna */
uint64_t xoroshiro128p_next(uint64_t* s)
{
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = s0 + s1;
	s1 ^= s0;
	s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
	s[1] = rotl(s1, 37); // c
	return r;	
}
uint64_t xoroshiro128pp_next(uint64_t* s)
{
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = rotl(s0 + s1, 17) + s0;
	s1 ^= s0;
	s[0] = rotl(s0, 49) ^ s1 ^ (s1 << 21); // a, b
	s[1] = rotl(s1, 28); // c
	return r;
}
uint64_t xoroshiro128ss_next(uint64_t* s)
{
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = rotl(s0 * 5, 7) * 9;

	s1 ^= s0;
	s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
	s[1] = rotl(s1, 37); // c
	return r;
}
uint64_t xoshiro256p_next(uint64_t* s)
{
	const uint64_t r = s[0] + s[3];
	const uint64_t t = s[1] << 17;
	s[2] ^= s[0];
	s[3] ^= s[1];

	s[1] ^= s[2];
	s[0] ^= s[3];
	s[2] ^= t;
	s[3] = rotl(s[3], 45);

	return r;
}
uint64_t xoshiro256pp_next(uint64_t* s)
{
	const uint64_t r = rotl(s[0] + s[3], 23) + s[0];
	const uint64_t t = s[1] << 17;
	s[2] ^= s[0];
	s[3] ^= s[1];

	s[1] ^= s[2];
	s[0] ^= s[3];
	s[2] ^= t;
	s[3] = rotl(s[3], 45);

	return r;
}
