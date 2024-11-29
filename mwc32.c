#include <stdint.h>
/* Идея составить для фильтра блума такую хеш функцию чтобы 
она эффективно считалась в векторном виде
*/
uint64_t mwc64x( uint64_t* state, const uint64_t A)
{
	uint64_t x = *state;
	*state = A*(uint32_t)(x) + (x>>32);
    return x;//((x>>32) ^ (x&0xFFFFFFFFU));
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
#include <math.h>
int main (){

if(0){// однородность распределения
	uint32_t distr[256];
	__builtin_bzero(distr, 256*4);
	uint16_t s = 1;
	uint16_t A1 = 0xF3;
	
	mwc16x(&s, A1);
	for (uint32_t i=0; i!=0xFFFFF; i++){ 
		uint32_t v = mwc16x(&s, A1);
		distr[v&255]++;
	}
	for (int i=0; i<256; i++) 
		printf("%04x\n", distr[i]);
	return 0;
}
if(0){// однородность распределения
	uint32_t distr[256];
	__builtin_bzero(distr, 256*4);
	uint32_t s = 1;
	uint32_t A1 = 0xFF9B;//0xFE4E;
	
	mwc32x(&s, A1);
	for (uint32_t i=0; i!=0xFFFFFFF; i++){ 
		uint32_t v = mwc32x(&s, A1);
		distr[(v>>8)&255]++;
	}
	for (int i=0; i<256; i++) 
		printf("%06x %1.3f\n", distr[i], fabs((int32_t)distr[i]-0x100000)/0x100000);
	return 0;
}



	const uint64_t A = (uint32_t)(~84047)-1;//0xfffeb81bULL;
	const uint64_t B = 0xFFFFFFFF;
	const uint64_t seed = 1;
/*	
	uint64_t s = seed;
	mwc64x(&s, A);
	uint64_t i;
	for (i=0; i!= B; i++){
		if (mwc64x(&s, A)==seed) break;
	}
	*/



	// Вариант генерации - от списка простых чисел 
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
//(Источник: https://calculat.io/ru/number/prime/128--1024)
	for (int k=0; a[k]!=0; k++){ 
		uint32_t A1 = (uint16_t)(~a[k]);
		uint32_t i = mwc32_check(A1);
		if (i>0x3f000000uL) printf("A=%04X i=%08x (%d)\n", A1, i,a[k]);
	}

	for (int k=1; k!=0xFF; k++){ 
		uint16_t A1 = (uint8_t)(~k);
		uint16_t i = mwc16_check(A1);
		if (i>0x4000uL) printf("A=%02X i=%04x (%d)\n", A1, i,(uint8_t)(~A1));
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

	for (int k=1; k!=0x3FF; k++){ 
		uint32_t A1 = (uint16_t)(0 - k);
		uint32_t i = mwc32_check(A1);
		if (i>0x7f000000uL) printf("A=%04X i=%08x (%u)\n", A1, i,(uint16_t)(~A1));
	}


	return 0;
}