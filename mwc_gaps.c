#include <stdint.h>
static inline uint32_t rotl32(const uint32_t x, int k) {
	return (x << k) ^ (x >> (32 - k));
}
static inline uint32_t rotr32(const uint32_t x, int k) {
	return (x << (32 - k)) ^ (x >> k);
}
/* Идея составить для фильтра блума такую хеш функцию чтобы 
она эффективно считалась в векторном виде
*/
uint64_t mwc64x( uint64_t* state, const uint64_t A)
{
	uint64_t x = *state;
	*state = A*(uint32_t)(x) + (x>>32);
    return x;//((x>>32) ^ (x&0xFFFFFFFFU));
}
uint32_t mwc32x2( uint32_t* state, const uint32_t A1, const uint32_t A2)
{
	uint32_t x = state[0];
	state[0] = A1*(uint16_t)(x) + (x>>16);
	uint32_t c = state[1];
	state[1] = A2*(uint16_t)(c) + (c>>16);
    return x^c;
}
uint32_t mwc32x( uint32_t* state, const uint32_t A)
{
	uint32_t x = *state;
	*state = A*(uint16_t)(x) + (x>>16);
    return x;//((x>>16) ^ (x&0xFFFFU));
}
uint16_t mwc16x( uint16_t* state, const uint16_t A)
{
	uint16_t x = *state;
	*state = A*(uint8_t)(x) + (x>>8);
    return x;//((x>>8) ^ (x & 0xFFFFU));
}
/*
Критерием выбора A является период повтора 

тут используется число (~A)<<32-1, A = 

https://calculat.io/ru/number/prime/65000--128000

83857	83869	83873	83891	83903
83911	83921	83933	83939	83969
83983	83987	84011	84017	84047 

257	263
269	271	277	281	283
293	307	311	313	317
331	337	347	349	353
359	367	373	379	383
389	397	401	409	419
421	431	433	439	443
449	457	461	463	467
479	487	491	499	503
509
(Источник: https://calculat.io/ru/number/prime/128--512)
*/
uint32_t mwc32_check(uint32_t A){
	const uint32_t seed = 0xFF;
	uint32_t s = seed;
	mwc32x(&s, A);
	const uint32_t B = 0xFFFFFFFF;
	uint32_t i;
	for (i=0; i!= B; i++){
		if (mwc32x(&s, A)==seed) break;
	}
	return i;
}

/*! 
Последовательность MWC — это последовательность пар $x_{n},c_{n}$, определяемых
$$x_{n}=(ax_{n-1}+c_{n-1})\,{\bmod {\,}}b,\ c_{n}=\left\lfloor {\frac {ax_{n-1}+c_{n-1}}{b}}\right\rfloor$$

Последовательность с запаздыванием $r$ является обобщением последовательности с запаздыванием на 1, 
позволяющим использовать более длинные периоды [2]. Последовательность MWC с запаздыванием $r$ — это последовательность пар $x_{n},c_{n}$ (для $n>r$), определяемых
$$x_{n}=(ax_{n-r}+c_{n-1})\,{\bmod {\,}}b,\ c_{n}=\left\lfloor {\frac {ax_{n-r}+c_{n-1}}{b}}\right\rfloor$$

[2]: Marsaglia, George¨ (May 2003). "Random number generators". Journal of Modern Applied Statistical Methods. 2 (1): 2–13. doi:10.22237/jmasm/1051747320.

Если $p = ab^r − 1$ является простым числом, то малая теорема Ферма гарантирует, что порядок любого элемента должен делиться на $p − 1 = ab^r − 2$, поэтому один из способов обеспечить большой порядок — выбрать $a$ так, 
чтобы $p$ было «безопасным простым числом», то есть p и $(p − 1)/2 = ab^r/2 − 1$ были простыми числами. В таком случае для $b = 2^{32}$ и $r = 1$ период будет равен $ab^r/2 − 1$, что приближается к $2^{63}$, что на практике может быть приемлемо большим подмножеством числа возможных 32-битных пар $(x, c)$. -- перевод с Wiki
 */
uint16_t mwc16_period(uint16_t a){
	return (a<<7)-2;
}
uint32_t mwc32_period(uint32_t a){
	return (a<<15)-2;
}
/*

A=00F9 i=31870 (6)
A=00F3 i=31102 (12)
A=00E4 i=29182 (27)
A=00E3 i=29054 (28)
A=00DA i=27902 (37)
A=00D2 i=26878 (45)
A=00CC i=26110 (51)
A=00C3 i=24958 (60)
A=00BD i=24190 (66)
A=00B6 i=23294 (73)
A=00AE i=22270 (81)
A=00A7 i=21374 (88)
A=00A5 i=21118 (90)
A=00A4 i=20990 (91)
A=009B i=19838 (100)
A=008F i=18302 (112)
A=008C i=17918 (115)
A=008A i=17662 (117)
A=0084 i=16894 (123)
A=0081 i=16510 (126)

*/
uint16_t mwc16_check(uint16_t A){
	const uint16_t seed = 1;
	uint16_t s = seed;
	uint16_t h0 = mwc16x(&s, A);
	const uint16_t B = 0xFFFF;
	uint16_t i;
	for (i=0; i!= B; i++){
		if (mwc16x(&s, A)==h0) break;
	}
	return i;
}
#include <stdio.h>
uint32_t xoroshiro64s_next(uint32_t s[2]) {
	const uint32_t s0 = s[0];
	uint32_t s1 = s[1];
	const uint32_t result = s0 * 0x9E3779BB;

	s1 ^= s0;
	s[0] = rotl32(s0, 26) ^ s1 ^ (s1 << 9); // a, b
	s[1] = rotl32(s1, 13); // c

	return result;
}

// тест на зазор между степенями двойки
uint16_t mwc16_ones(uint16_t A, uint16_t seed) {
	seed %= (A<<8) -1;
	uint16_t s = seed;
	const uint16_t B = 0xFFFF;
	mwc16x(&s, A);
	uint16_t i, prev =0;
	for (i=0; i!= B; i++){
		if (mwc16x(&s, A)==seed) break;
		if ((s>>__builtin_ctz(s))==1) {
			printf(" %2d", i - prev, __builtin_ctz(s));
			prev = i;
		}
	}
	printf(" =%d\n", i);
	return i;
}
static inline uint32_t _scrambler(uint32_t x) {return x;}
// static inline uint32_t _scrambler(uint32_t x) {return __builtin_bswap32(x);}
// static inline uint32_t _scrambler(uint32_t x) {return x^rotl32(x,16);}
// static inline uint32_t _scrambler(uint32_t x) {return x^rotl32(x,8);}
// static inline uint32_t _scrambler(uint32_t x) {return x^rotl32(x,1);}
// static inline uint32_t _scrambler(uint32_t x) {return x^rotl32(x,8)^rotl32(x,16);}
// static inline uint32_t _scrambler(uint32_t x) {return x^rotl32(x,1)^rotl32(x,2);}
//static inline uint32_t _scrambler(uint32_t x) {return x^(rotl32(~x,1)&rotl32(x,2));}
uint32_t mwc32_ones(uint32_t A, uint32_t pattern) {
	uint32_t s = 1;// (A<<15)-1u;// -- чтобы попасть на вторую орбиту 2^{-1}
	const uint32_t B = 0x7FFFFFFF;
	mwc32x(&s, A);
	uint32_t i, prev = 0, r;
	for (i=1; i!= B; i++){
		mwc32x(&s, A);
		//if (r==pattern) break;
        uint32_t r = _scrambler(s);
		if (rotr32(r, __builtin_ctz(r))==pattern && i-prev>1) {
			printf(" %2d", i - prev,__builtin_ctz(r));
            prev = i;
		}
	}
	printf(" =%x\n", i);
	return i;
}
uint32_t xoro_ones(uint32_t* s, uint32_t pattern) {
	const uint32_t B = 0x7FFFFFFF;
	xoroshiro64s_next(s);
	uint32_t i, prev = 0, r;
	for (i=1; i!= B; i++){
		xoroshiro64s_next(s);
		//if (r==pattern) break;
        uint32_t r = _scrambler(s[0]);
		if (rotr32(r, __builtin_ctz(r))==pattern && i-prev>1) {
			printf(" %2d", i - prev,__builtin_ctz(r));
            prev = i;
		}
	}
	printf(" =%x\n", i);
	return i;
}
uint32_t mwc32x2_ones(uint32_t A1, uint32_t A2, uint32_t pattern) {
	uint32_t s[2] = {1,1};
	const uint32_t B = 0x7FFFFFFF;
	mwc32x2(s, A1,A2);
	uint32_t i, prev = 0;
	for (i=1; i!= B; i++){
        uint32_t r = mwc32x2(s, A1,A2);
		if (s[0]==s[1]) break;
		if (rotr32(r, __builtin_ctz(r))==pattern && i-prev>1) {
			printf(" %2d", i - prev,__builtin_ctz(r));
            prev = i;
		}
	}
	printf(" =%x\n", i);
	return i;
}

#include <math.h>
int main (){
	// Вариант генерации - от списка простых чисел 
	uint32_t AA[] = {
		0xFFEA, 0xFFD7, 0xFFBD, 
		0xFF9B, 0xFF81, 0xFF80, 0xFF7B, 
		0xFF75, 0xFF48, 0xFF3F, 0xFF3C,
		0xFF2C, 0xFF09, 0xFF03, 
		0xFF00, 0xFEE4, 0xFEA8, 0xFEA5,
		0xFEA0, 0xFE94, 0xFE8B, 0xFE72, 
		0xFE4E, 0xFE30, 0xFE22, 0xFE15, 
		0xFE04,
		0};
// A=FFEA i=7ff4fffe ( 21), P mod 24 =23, A mod 3 =0
// A=FFD7 i=7feb7ffe ( 40), P mod 24 = 7, A mod 3 =2
// A=FFBD i=7fde7ffe ( 66), P mod 24 =23, A mod 3 =0
// A=FFA8 i=7fd3fffe ( 87), P mod 24 =23, A mod 3 =0
// A=FF9B i=7fcd7ffe (100), P mod 24 = 7, A mod 3 =2
// A=FF81 i=7fc07ffe (126), P mod 24 =23, A mod 3 =0
// A=FF80 i=7fbffffe (127), P mod 24 = 7, A mod 3 =2
// A=FF7B i=7fbd7ffe (132), P mod 24 =23, A mod 3 =0
// A=FF75 i=7fba7ffe (138), P mod 24 =23, A mod 3 =0
// A=FF48 i=7fa3fffe (183), P mod 24 =23, A mod 3 =0
// A=FF3F i=7f9f7ffe (192), P mod 24 =23, A mod 3 =0
// A=FF3C i=7f9dfffe (195), P mod 24 =23, A mod 3 =0
// A=FF2C i=7f95fffe (211), P mod 24 = 7, A mod 3 =2
// A=FF09 i=7f847ffe (246), P mod 24 =23, A mod 3 =0
// A=FF03 i=7f817ffe (252), P mod 24 =23, A mod 3 =0
// A=FF00 i=7f7ffffe (255), P mod 24 =23, A mod 3 =0
// A=FEEB i=7f757ffe (276), P mod 24 =23, A mod 3 =0
// A=FEE4 i=7f71fffe (283), P mod 24 = 7, A mod 3 =2
// A=FEA8 i=7f53fffe (343), P mod 24 = 7, A mod 3 =2
// A=FEA5 i=7f527ffe (346), P mod 24 = 7, A mod 3 =2
// A=FEA0 i=7f4ffffe (351), P mod 24 =23, A mod 3 =0
// A=FE94 i=7f49fffe (363), P mod 24 =23, A mod 3 =0
// A=FE8B i=7f457ffe (372), P mod 24 =23, A mod 3 =0
// A=FE72 i=7f38fffe (397), P mod 24 = 7, A mod 3 =2
// A=FE4E i=7f26fffe (433), P mod 24 = 7, A mod 3 =2
// A=FE30 i=7f17fffe (463), P mod 24 = 7, A mod 3 =2
// A=FE22 i=7f10fffe (477), P mod 24 =23, A mod 3 =0
// A=FE15 i=7f0a7ffe (490), P mod 24 = 7, A mod 3 =2
// A=FE04 i=7f01fffe (507), P mod 24 =23, A mod 3 =0
	uint16_t a[] = {255, 257, 263,
269,	271,	277,	281,	283,
293,	307,	311,	313,	317,
331,	337,	347,	349,	353,
359,	367,	373,	379,	383,
389,	397,	401,	409,	419,
421,	431,	433,	439,	443,
449,	457,	461,	463,	467,
479,	487,	491,	499,	503,
509, 
521,523,541,547,557,563,569,571,577,587,593,599,
601,607,613,617,619,631,641,643,647,653,659,661,
0};
    if (1) {
		printf("Xoroshiro64s\n");
		uint32_t s[2] = {1,-1};

        uint32_t r;
			r = xoro_ones( s, 0b01);
			r = xoro_ones( s, (1<<2) -1);
			r = xoro_ones( s, (1<<4) -1);
			r = xoro_ones( s, (1<<8) -1);
			r = xoro_ones( s, (1<<16) -1);
			r = xoro_ones( s, (1u<<2) |1);
			r = xoro_ones( s, (1u<<4) |1);
			r = xoro_ones( s, (1u<<8) |1);
			r = xoro_ones( s, (1u<<16) |1);
    }
    if (1) {
    // A=FEA0 i=7f4ffffe (351), P mod 24 =23, A mod 3 =0
    // A=FE94 i=7f49fffe (363), P mod 24 =23, A mod 3 =0
        uint32_t A1 = 0xFF3C;
        uint32_t A2 = 0xFE94;
		printf("mwc32x2 A1=%4x, A2=%4x\n", A1, A2);
        uint32_t r;
			r = mwc32x2_ones( A1,A2, 0b01);
			r = mwc32x2_ones( A1,A2, 0b11);
			r = mwc32x2_ones( A1,A2, (1<<4) -1);
			r = mwc32x2_ones( A1,A2, (1<<8) -1);
			r = mwc32x2_ones( A1,A2, (1<<16) -1);
//			r = mwc32x2_ones( A1,A2, 0b111);
			r = mwc32x2_ones( A1,A2, (1u<<1) |1);
			r = mwc32x2_ones( A1,A2, (1u<<2) |1);
			r = mwc32x2_ones( A1,A2, (1u<<4) |1);
			r = mwc32x2_ones( A1,A2, (1u<<8) |1);
			r = mwc32x2_ones( A1,A2, (1u<<16) |1);
    }
    if (1) {
    // A=FEA0 i=7f4ffffe (351), P mod 24 =23, A mod 3 =0
    // A=FE94 i=7f49fffe (363), P mod 24 =23, A mod 3 =0
        uint32_t A2 = 0xFEA0;
        uint32_t A1 = 0xFE94;
        uint32_t r;
		for (int i=0; AA[i]!=0; i++) {
			A1 = AA[i];
			printf("A=%04X, %d\n", A1, (A1<<(16-5)) -1);
			r = mwc32_ones( A1, 0b01);
			r = mwc32_ones( A1, (1<<2) -1);
			r = mwc32_ones( A1, (1<<4) -1);
			r = mwc32_ones( A1, (1<<8) -1);
			r = mwc32_ones( A1, (1<<16) -1);

			r = mwc32_ones( A1, (1u<<2) |1);
			r = mwc32_ones( A1, (1u<<4) |1);
			r = mwc32_ones( A1, (1u<<8) |1);
			r = mwc32_ones( A1, (1u<<16) |1);
		}
    }
    uint32_t r;
	for (int k=0; a[k]!=0; k++){ 
		uint32_t A1 = (uint16_t)(~a[k]);
		uint32_t i = mwc32_check(A1);
		if (i==mwc32_period(A1)) {
			printf("A=%04X i=%08x (%d), %d\n", A1, i,a[k], (A1<<(16-5)) -1);
			r = mwc32_ones( A1, 0b01);
			r = mwc32_ones( A1, (1<<2) -1);
			r = mwc32_ones( A1, (1<<4) -1);
			r = mwc32_ones( A1, (1<<8) -1);
			r = mwc32_ones( A1, (1<<16) -1);

			r = mwc32_ones( A1, (1u<<2) |1);
			r = mwc32_ones( A1, (1u<<4) |1);
			r = mwc32_ones( A1, (1u<<8) |1);
			r = mwc32_ones( A1, (1u<<16)|1);
		}
	}
    if (0)
	for (int k=0; k!=0xFF; k++){ 
		uint16_t A1 = (uint8_t)(~k);
		uint16_t i = mwc16_check(A1);
		if (i==mwc16_period(A1)) {
			printf("A=%02X i=%04x (%d)\n", A1, i,(uint8_t)(~A1));
			for (int n=1;n <8;n++)
			{
				uint32_t r = mwc16_ones( A1, (1<<n)-2);
				// printf(" %x", r);
			}
			printf("\n");
		}
	}
/*
A=FFEA i=7ff4fffe (21)
A=FFD7 i=7feb7ffe (40)
A=FFBD i=7fde7ffe (66)
A=FFA8 i=7fd3fffe (87)
A=FF9B i=7fcd7ffe (100)
A=FF81 i=7fc07ffe (126)
A=FF80 i=7fbffffe (127)
A=FF7B i=7fbd7ffe (132)
A=FF75 i=7fba7ffe (138)
A=FF48 i=7fa3fffe (183)
A=FF3F i=7f9f7ffe (192)
A=FF3C i=7f9dfffe (195)
A=FF2C i=7f95fffe (211)
A=FF09 i=7f847ffe (246)
A=FF03 i=7f817ffe (252)

A=FF00 i=7f7ffffe (255)
A=FEEB i=7f757ffe (276)
A=FEE4 i=7f71fffe (283)
A=FEA8 i=7f53fffe (343)
A=FEA5 i=7f527ffe (346)
A=FEA0 i=7f4ffffe (351)
A=FE94 i=7f49fffe (363)
A=FE8B i=7f457ffe (372)
A=FE72 i=7f38fffe (397)
A=FE4E i=7f26fffe (433)
A=FE30 i=7f17fffe (463)
A=FE22 i=7f10fffe (477)
A=FE15 i=7f0a7ffe (490)
A=FE04 i=7f01fffe (507)



*/
    if (0)
	for (int k=1; k!=0x1FF; k++){ 
		uint32_t A1 = (uint32_t)((1<<16) - k);
		uint32_t i = mwc32_check(A1);
		if (i>=mwc32_period(A1)) printf("A=%04X i=%08x (%u)\n", A1, i,(uint16_t)(~A1));
	}


	return 0;
}