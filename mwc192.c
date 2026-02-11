#include <stdint.h>
#define MP_SIZE 3
#define MWC_A2 0xffa04e67b3c95d86 // MWC192, B2 = 1<<128
#define MWC_P_INVL 0x005fd56afa7bccee
#include "mp.c" // шаблон класса для заданной размерности MP_SIZE


static const uint64_t MWC_M2[3] = { 0xffffffffffffffff, 0xffffffffffffffff, MWC_A2 - 1 };
static const uint64_t MWC192_jump96 [3] = { 0xd94fb8d87c7c6437, 0xafc217e3b9edf985, 0xdc2be36e4bd21a2 };
static const uint64_t MWC192_jump144[3] = { 0xd0e7cedd16a0758e, 0xec956c3909137b2d, 0x3c6528aaead6bbdd };
/* Marsaglia multiply-with-carry Random Number Generator */
/* uint64_t x, c -- состояние 

Выражение x = (x+c*b)*A mod A*b-1 == x*A + c
Доказательство:
(x+c*b)*A  = x*A + c*(A*b-1) + c == x*A + c
Средняя часть обращается в ноль, потому что кратно модулю. 

*/
uint64_t mwc192_next(uint64_t* state) {
	const uint64_t r = state[1];
	const unsigned __int128 t = (unsigned __int128)MWC_A2 * state[0] + state[2];
	state[0] = r;
	state[1] = t;
	state[2] = t >> 64;
	return r;
}

void mwc192_jump(uint64_t* state) {
	mp_mulm(state, state, MWC192_jump96, MWC_M2);
}
/*! \brief 
	\param distance дистанция за вычетом 1 
 */
void mwc192_skip(uint64_t* state, uint64_t distance) {
	uint64_t x[MP_SIZE] = {MWC_A2};
	mp_powm_ui(x, x, distance, MWC_M2);
	mp_mulm(state, state, x, MWC_M2);
}
#include <stdio.h>
int main(){

	const uint64_t P[3] = { 0xffffffffffffffff, 0xffffffffffffffff, MWC_A2 - 1 };
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
	uint64_t inv = INVL(MWC_A2);
	printf("INVL = 0x%016llx\n", inv);
	// 1, 5, 13, 29
	state[0] = MWC_A2, state[1]=0, state[2]=0;
	for (i=0; i<((1<<16)-1); i++) {// удвоенное число вызовов
		mwc192_next(state);
		mwc192_next(state);
	}
	printf("stat(16) = {0x%016llx, 0x%016llx, 0x%016llx};\n", state[0], state[1], state[2]);
	state[0] = MWC_A2, state[1]=0, state[2]=0;
	mwc192_skip(state, (1<<16)-1);
	printf("skip(16) = {0x%016llx, 0x%016llx, 0x%016llx};\n", state[0], state[1], state[2]);
	
	s[0] = MWC_A2, s[1]=0, s[2]=0;
// расчет сдвиговой константы A^{2^32}
	for (i=0; i<16; i++){
		mp_mulm(s, s, s, P);
	}
	printf("jump(%d) = {0x%016llx, 0x%016llx, 0x%016llx};\n", i, s[0], s[1], s[2]);

	state[0] = MWC_A2, state[1]=0, state[2]=0;
	for (i=0; i<((1<<24)-1); i++) {// удвоенное число вызовов
		mwc192_next(state);
		mwc192_next(state);
	}
	printf("stat(24) = {0x%016llx, 0x%016llx, 0x%016llx};\n", state[0], state[1], state[2]);
	state[0] = MWC_A2, state[1]=0, state[2]=0;
	mwc192_skip(state, (1ull<<24)-1);
	printf("skip(24) = {0x%016llx, 0x%016llx, 0x%016llx};\n", state[0], state[1], state[2]);

	state[0] = MWC_A2, state[1]=0, state[2]=0;
	mwc192_skip(state, (1ull<<32)-1);
	printf("skip(32) = {0x%016llx, 0x%016llx, 0x%016llx};\n", state[0], state[1], state[2]);

// расчет сдвиговой константы A^{2^32}
	for (; i<32; i++)
		mp_mulm(s, s, s, P);
	printf("jump(32) = {0x%016llx, 0x%016llx, 0x%016llx};\n", s[0], s[1], s[2]);

	state[0] = MWC_A2, state[1]=0, state[2]=0;
	mwc192_skip(state, (1ull<<48)-1);
	printf("skip(48) = {0x%016llx, 0x%016llx, 0x%016llx};\n", state[0], state[1], state[2]);

	for (; i<48; i++)
		mp_mulm(s, s, s, P);
	printf("jump(48) = {0x%016llx, 0x%016llx, 0x%016llx};\n", s[0], s[1], s[2]);


	state[0] = MWC_A2, state[1]=0, state[2]=0;
	mwc192_skip(state, ~0ull);
	printf("skip(64) = {0x%016llx, 0x%016llx, 0x%016llx};\n", state[0], state[1], state[2]);
// расчет сдвиговой константы A^{2^64}
	for (; i<64; i++)
		mp_mulm(s, s, s, P);
	printf("jump(64) = {0x%016llx, 0x%016llx, 0x%016llx};\n", s[0], s[1], s[2]);
// расчет сдвиговой константы A^{2^96}
	for (; i<96; i++)
		mp_mulm(s, s, s, P);
	printf("jump(96) = {0x%016llx, 0x%016llx, 0x%016llx};\n", s[0], s[1], s[2]);

	state[0] = MWC_A2, state[1]=0, state[2]=0;
	mwc192_jump(state);
	printf("jump(96) = {0x%016llx, 0x%016llx, 0x%016llx};\n", state[0], state[1], state[2]);
// расчет сдвиговой константы A^{2^96}
	for (; i<128; i++)
		mp_mulm(s, s, s, P);
	printf("jump(128)= {0x%016llx, 0x%016llx, 0x%016llx};\n", s[0], s[1], s[2]);
// расчет сдвиговой константы A^{2^96}
	for (; i<144; i++)
		mp_mulm(s, s, s, P);
	printf("jump(144)= {0x%016llx, 0x%016llx, 0x%016llx};\n", s[0], s[1], s[2]);
	
	s[0] =  0, s[1] =  0, s[2] = 1;
	q[0] = -3, q[1] = -1, q[2] = MWC_A2-1;
	mp_powm(s,s,q, P);
	printf("inv(P-2) = {0x%016llx, 0x%016llx, 0x%016llx};\n", s[0], s[1], s[2]);
	printf("err count = %d\n", err_count);

	return 0;
}