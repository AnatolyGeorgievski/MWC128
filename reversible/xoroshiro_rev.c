/*!
	\author __Анатолий М. Георгиевский, ИТМО__, 2026
	
[2302.02778](https://arxiv.org/pdf/2302.02778) Reversible random number generation for
adjoint Monte Carlo simulation of the heat equation

Идея обратимого (reversible) псевдослучайного генератора — полезная концепция, в нескольких областях:

* adjoint Monte Carlo / обратные симуляции (физика, молекулярная динамика, теплообмен)
* рандомизация с undo/redo (игры, симуляции)
* рандомизация в обратимых (квантовых) вычислениях
* выявление слабых шифров и криптоанализ (когда нужно восстановить предыдущее состояние)
* тестирование и отладка RNG-последовательностей

    \sa https://github.com/adam4130/reversible-random-number-generators

Сборка
    $ gcc -o test xoroshiro_rev.c
    $ ./test.exe
 */

#include <stdint.h>
#include <stdio.h>


static inline uint32_t rotl32(const uint32_t x, int k) {
	return (x << k) | (x >> (32 - k));
}
static inline uint32_t rotr32(const uint32_t x, int k) {
	return (x << (32 - k)) | (x >> k);
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
uint32_t xoroshiro64s_prev(uint32_t s[2]) {
	uint32_t s0 = s[0];
	uint32_t s1 = s[1];
	s1 = rotr32(s1, 13); // c
	s0 = s0 ^ s1 ^ (s1 << 9); // a, b
    s0 = rotr32(s0, 26);
	s1 ^= s0;
    s[0] = s0;
    s[1] = s1;
	const uint32_t result = s0 * 0x9E3779BB;
	return result;
}

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) ^ (x >> (64 - k));
}
static inline uint64_t rotr(const uint64_t x, int k) {
	return (x << (64 - k)) ^ (x >> k);
}

static uint64_t s[2] = {-1, 1};
uint64_t xoroshiro128p_next()
{
	uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = s0 + s1;
	s1 ^= s0;
    s0 = rotl(s0, 24);
	s[0] = s0 ^ s1 ^ (s1 << 16); // a, b
	s[1] = rotl(s1, 37); // c
	return r;	
}
uint64_t xoroshiro128p_prev()
{
	uint64_t s0, s1;
	s1 = rotr(s[1], 37); // c
	s0 = s[0] ^ s1 ^ (s1 << 16); // a, b
	s0 = rotr(s0, 24);
    s1 ^= s0;
	s[0] = s0;
	s[1] = s1;
    const uint64_t r = s0 + s1;
	return r;
}

int main (){
    int const nr = 8;
    int i;
    printf("xoroshiro128p*\n");
    for (i = 0; i< nr; i++){
        uint64_t x = xoroshiro128p_next();
        printf ("%016llx\n", x);
    }
    for (; i-->0;){
        uint64_t x = xoroshiro128p_prev();
        printf ("%016llx\n", x);
    }
    printf("xoroshiro64*\n");
    uint32_t st[2] = {1,-1};
    for (i = 0; i< nr; i++){
        uint64_t x = xoroshiro64s_next(st);
        printf ("%016llx\n", x);
    }
    for (; i-->0;){
        uint64_t x = xoroshiro64s_prev(st);
        printf ("%016llx\n", x);
    }
    return 0;
}