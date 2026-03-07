/*!
	\author __Анатолий М. Георгиевский, ИТМО__, 2026
	
[2302.02778](https://arxiv.org/pdf/2302.02778) Reversible random number generation for
adjoint Monte Carlo simulation of the heat equation

Идея обратимого (reversible) псевдослучайного генератора — полезная концепция, в нескольких областях:

* adjoint Monte Carlo / обратные симуляции (физика, молекулярная динамика, теплообмен)
* рандомизация с undo/redo (игры, симуляции)
* рандомизация в обратимых (квантовых) вычислениях
* дифференциальный вероятностный криптоанализ (когда нужно восстановить предыдущее состояние)
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
uint64_t xoroshiro128pp_prev(uint64_t* s)
{
	uint64_t s0, s1;
	s1 = rotr(s[1], 28); // c
	s0 = s[0] ^ s1 ^ (s1 << 21); // a, b
	s0 = rotr(s0, 49);
    s1 ^= s0;
	s[0] = s0;
	s[1] = s1;
    const uint64_t r = rotl(s0 + s1, 17) + s0;
	return r;
}
// обратная операция для x ^= x<<17
uint64_t shift17_rev(uint64_t x) {
	x ^= x<<17;
	x ^= x<<17;
	x ^= x<<17;
	return x;
}
uint64_t xoshiro256ss_next(uint64_t* s) {
	const uint64_t result = rotl(s[1] * 5, 7) * 9;
	const uint64_t t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;

	s[3] = rotl(s[3], 45);

	return result;
}
uint64_t xoshiro256p_next(uint64_t* s) {
	const uint64_t result = s[0] + s[3];

	const uint64_t t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;

	s[3] = rotl(s[3], 45);

	return result;
}
uint64_t xoshiro256pp_next(uint64_t* s) {
	const uint64_t result = rotl(s[0] + s[3], 23) + s[0];

	const uint64_t t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;

	s[3] = rotl(s[3], 45);

	return result;
}
uint64_t xoshiro256ss_prev(uint64_t* s) {
	const uint64_t t = s[1];
	s[3] = rotr(s[3], 45);
	s[0] ^= s[3];
	s[1]  = shift17_rev(s[1] ^ s[2]);
	s[2]  = t ^ s[0] ^ s[1];
	s[3] ^= s[1];
	const uint64_t result = rotl(s[1] * 5, 7) * 9;
	return result;
}
uint64_t xoshiro256p_prev(uint64_t* s) {
	const uint64_t t = s[1];
	s[3] = rotr(s[3], 45);
	s[0] ^= s[3];
	s[1]  = shift17_rev(s[1] ^ s[2]);
	s[2]  = t ^ s[0] ^ s[1];
	s[3] ^= s[1];
	const uint64_t result = s[0] + s[3];
	return result;
}
uint64_t xoshiro256pp_prev(uint64_t* s) {
	const uint64_t t = s[1];
	s[3] = rotr(s[3], 45);
	s[0] ^= s[3];
	s[1]  = shift17_rev(s[1] ^ s[2]);
	s[2]  = t ^ s[0] ^ s[1];
	s[3] ^= s[1];
	const uint64_t result = rotl(s[0] + s[3], 23) + s[0];
	return result;
}
#ifdef TEST_REV// тестирование
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
	uint64_t s2[2] = {1,-1};
    printf("xoroshiro128pp\n");
    for (i = 0; i< nr; i++){
        uint64_t x = xoroshiro128pp_next(s2);
        printf ("%016llx\n", x);
    }
    for (; i-->0;){
        uint64_t x = xoroshiro128pp_prev(s2);
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
    printf("xoshiro256ss\n");
	uint64_t s4[4] = {1,-1, 1,-1};
    for (i = 0; i< nr; i++){
        uint64_t x = xoshiro256ss_next(s4);
    }
    for (; i-->0;){
        uint64_t x = xoshiro256ss_prev(s4);
    }
	if (s4[0] ==1 && s4[1]==-1 && s4[2]==1 && s4[3]==-1) printf("..ok\n");
    printf("xoshiro256p\n");
    for (i = 0; i< nr; i++){
        uint64_t x = xoshiro256p_next(s4);
    }
    for (; i-->0;){
        uint64_t x = xoshiro256p_prev(s4);
    }
	if (s4[0] ==1 && s4[1]==-1 && s4[2]==1 && s4[3]==-1) printf("..ok\n");
    printf("xoshiro256pp\n");
    for (i = 0; i< 255; i++){
        uint64_t x = xoshiro256pp_next(s4);
    }
    for (; i-->0;){
        uint64_t x = xoshiro256pp_prev(s4);
    }
	if (s4[0] ==1 && s4[1]==-1 && s4[2]==1 && s4[3]==-1) printf("..ok\n");
    return 0;
}
#endif