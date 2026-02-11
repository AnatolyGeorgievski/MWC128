#include <stdint.h>
#define MP_SIZE 4
#define MWC_A3 		0xfff62cf2ccc0cdaf // MWC256
#define MWC_P_INVL 	0x0009d36dbbdff328

#include "mp.c" // шаблон класса для заданной размерности MP_SIZE

//#define MWC_A3 0xff377e26f82da74a // MWC256 см википедия
static const uint64_t MWC_M3[4] = { 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, MWC_A3 - 1 };
static const uint64_t MWC256_jump128[4] = { 0x28c3ff11313847eb, 0xfe88c291203b2254, 0xf6f8c3fd02ec98fb, 0x4b89aa2cd51c37b9 };
static const uint64_t MWC256_jump192[4] = { 0x64c6e39cf92f77a4, 0xf95382f758ac9877, 0x6c40ce860e0d702, 0xaf5ca22408cdc83 };
/* Marsaglia multiply-with-carry Random Number Generator */
/* uint64_t x, c -- состояние 

Выражение x = (x+c*B)*A mod A*B-1 == x*A + c
Доказательство:
(x+c*B)*A  = x*A + c*(A*B-1) + c
Средняя часть обращается в ноль, потому что кратно модулю. 

Константа B выбрана =1<<192 - сдвичг разрядов числа
State*A3 -- так можно выразить эту операцию

 \see https://prng.di.unimi.it/MWC256.c

*/
uint64_t mwc256_next(uint64_t* state) {
	const uint64_t r = state[2];
	const unsigned __int128 t = (unsigned __int128)MWC_A3 * state[0] + state[3];
	state[0] = state[1];
	state[1] = r;
	state[2] = t;
	state[3] = t >> 64;
	return r;
}

void mwc256_jump(uint64_t* state) {
	mp_mulm(state, state, MWC256_jump192, MWC_M3);
}
/*! \brief 
	\param distance дистанция за вычетом 1 
	
	Результат - S = S*A3^{distance};
 */
void mwc256_skip(uint64_t* state, uint64_t distance) {
	uint64_t x[MP_SIZE] = {MWC_A3};
	mp_powm_ui(x, x, distance, MWC_M3);
	mp_mulm(state, state, x, MWC_M3);
}
#include <stdio.h>
int main(){

	const uint64_t P[4] = { 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, MWC_A3 - 1 };
uint64_t state[MP_SIZE] = {12345,1};
	uint64_t MAX_COUNT = (1U<<20);
	uint64_t r [MP_SIZE];
	uint64_t b0[MP_SIZE];
	uint64_t b1[MP_SIZE];
	uint64_t q [MP_SIZE];
	uint64_t s [MP_SIZE] = {12345,1};
	int i;
	for (i=0;i<MAX_COUNT; i++)
	{
		uint64_t a = i<<6;
		mp_mov(b0, s);
		mp_mov(b1, s);
		mp_mulm_u_(r, b0, a, P);
		mp_mulm_ui(q, b1, a, P);
//		if (mp_cmp(r,state)!=0) break;
		if (mp_cmp(r,q)!=0) break;
//		mwc128_next(state);
	}
	if (i<MAX_COUNT) {
		printf("fail (%d)\n", i);
		for (i=0; i<MP_SIZE; i++){
			printf(" %016llx", r[i]);
		}
		printf("\n");
		for (i=0; i<MP_SIZE; i++){
			printf(" %016llx", q[i]);
		}
		printf("\n");
	}
	uint64_t inv = INVL(MWC_A3);
	printf("INVL = 0x%016llx\n", inv);
	// 3, 9,  (1<<(2+1)) - 3
	state[0] = 1, state[1]=0, state[2]=0, state[3]=0;
	for (i=0; i<((1<<5)-0); i++) {
		mwc256_next(state);
		mwc256_next(state);
		mwc256_next(state);
	}
// с коэффициентом x3
	printf("stat(31x3)= {0x%016llx, 0x%016llx, 0x%016llx, 0x%016llx};\n", state[0], state[1], state[2], state[3]);
	state[0] = MWC_A3, state[1]=0, state[2]=0, state[3]=0;
	mwc256_skip(state, (1<<5)-1);
	printf("skip(31)  = {0x%016llx, 0x%016llx, 0x%016llx, 0x%016llx};\n", state[0], state[1], state[2], state[3]);

	
	printf("\n");
	s[0] = MWC_A3, s[1]=0, s[2]=0, s[3]=0;
// расчет сдвиговой константы A^{2^32}
	i=0;
	if (1)for (; i<31; i++){
		mp_mulm(s, s, s, P);
	printf("jump(%d) = {0x%016llx, 0x%016llx, 0x%016llx, 0x%016llx};\n", i+1, s[0], s[1], s[2], s[3]);
	}
	printf("\n");
// расчет сдвиговой константы A3^{2^32}
	for (; i<32; i++)
		mp_mulm(s, s, s, P);
	printf("jump(32) = {0x%016llx, 0x%016llx, 0x%016llx, 0x%016llx};\n", s[0], s[1], s[2], s[3]);
	state[0] = MWC_A3, state[1]=0, state[2]=0, state[3]=0;
	mwc256_skip(state, (1uLL<<32)-1);
	printf("skip(32) = {0x%016llx, 0x%016llx, 0x%016llx, 0x%016llx};\n", state[0], state[1], state[2], state[3]);

// расчет сдвиговой константы A3^{2^64}
	for (; i<64; i++)
		mp_mulm(s, s, s, P);
	printf("jump(64) = {0x%016llx, 0x%016llx, 0x%016llx, 0x%016llx};\n", s[0], s[1], s[2], s[3]);
	state[0] = MWC_A3, state[1]=0, state[2]=0, state[3]=0;
	mwc256_skip(state, ~0ULL);
	printf("skip(64) = {0x%016llx, 0x%016llx, 0x%016llx, 0x%016llx};\n", state[0], state[1], state[2], state[3]);
// расчет сдвиговой константы A3^{2^128}
	for (; i<128; i++)
		mp_mulm(s, s, s, P);
	printf("jump(128)= {0x%016llx, 0x%016llx, 0x%016llx, 0x%016llx};\n", s[0], s[1], s[2], s[3]);
// расчет сдвиговой константы A3^{2^192}
	for (; i<192; i++)
		mp_mulm(s, s, s, P);
	printf("jump(192)= {0x%016llx, 0x%016llx, 0x%016llx, 0x%016llx};\n", s[0], s[1], s[2], s[3]);
	
	s[0] =  0, s[1] =  0, s[2] =  0, s[3] = 1;
	q[0] = -3, q[1] = -1, q[2] = -1, q[3] = MWC_A3-1;
	mp_powm(s,s,q, P);
	printf("jump(32) = {0x%016llx, 0x%016llx, 0x%016llx, 0x%016llx};\n", s[0], s[1], s[2], s[3]);
	
	printf("err count = %d\n", err_count);
	return 0;
}