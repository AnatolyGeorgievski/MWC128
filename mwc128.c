/*!

https://en.wikipedia.org/wiki/Multiply-with-carry_pseudorandom_number_generator

	$ gcc -DTEST_MWC128 -O3 -march=native -o test mwc128.c
 */
#include <stdint.h>
#define MP_SIZE 2
#define MWC_A1 		0xffebb71d94fcdaf9ull // MWC128  число A1, B1 = (1<<64)
#define MWC_P_INVL  0x00144a7e03c11fcd // замена деления на умножение и сдвиг

#include "mp.c" // шаблон класса для заданной размерности MP_SIZE

//#define MWC_A1 0xff3a275c007b8ee6 // MWC128 - другой вариант, см википедию

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
uint64_t mwc128_next(uint64_t* state) {
	const uint64_t result = state[0];
	const uint128_t t = (uint128_t)MWC_A1 * state[0] + state[1];
	state[0] = t;
	state[1] = t >> 64;
	return result;
}

/* This is the jump function for the generator. It is equivalent
   to 2^64 calls to next(); it can be used to generate 2^64
   non-overlapping subsequences for parallel computations.

   Equivalent C++ Boost multiprecision code:
   cpp_int b = cpp_int(1) << 64;
   cpp_int m = MWC_A1 * b - 1;
   cpp_int r = cpp_int("0x2f65fed2e8400983a72f9a3547208003");
   cpp_int s = ((x + c * b) * r) % m;
   x = uint64_t(s);
   c = uint64_t(s >> 64);
*/
void mwc128_jump(uint64_t* state) {
	mp_mulm(state, state, MWC128_jump64, MWC_M1);
}
void mwc128_seed(uint64_t* state, uint64_t seed) {
	state[0] = seed % MWC_A1;
	state[0] = 1;
}
void mwc128_skip(uint64_t* state, uint64_t distance) {
	uint64_t x[MP_SIZE] = {MWC_A1};
__asm volatile("# LLVM-MCA-BEGIN mwc128_skip");
	mp_powm_ui(x, x, distance, MWC_M1);
	mp_mulm(state, state, x, MWC_M1);
__asm volatile("# LLVM-MCA-END mwc128_skip");
}
void mwc128_streams(uint64_t* state, unsigned n_streams, uint64_t distance) {
	uint64_t x[MP_SIZE] = {MWC_A1};
	mp_powm_ui(x, x, distance, MWC_M1);
	for (int i=0; i<n_streams-1; i++)
		mp_mulm(&state[2*i+2], &state[2*i], x, MWC_M1);
}



#ifdef TEST_MWC128
/*! \brief однократное схлопывание, расчет констант
	\param n =1 для сдвига на 3 слова 192 бита
 */
#include <stdio.h>

static void mp_folding(uint64_t* r, int n, const uint64_t* P)
{
	r[0] = 1, r[MP_SIZE-1] = -MWC_A1; // -P , ~P = (-P-1), -P = ~P+1
	for(int i=0; i<n;i++) {
		mp_shlm(r, P);
		printf("[%d]= {%016llx, %016llx},\n", i, r[0], r[1]);
	}
	printf("\n");
}
static void mp_reduction(uint64_t* r, int size)
{
	uint64_t rc=MWC_A1+1; // число должно быть меньше MWC_A1
	uint64_t f[2] = {1, -MWC_A1};// folding
	for (int i=0; i<size; i++) {
		rc = mp_mac_ui(r, f, rc);
		printf("0x%016llx, ", rc);
	}
	printf("\n");
	mp_mod_ui(r, rc, MWC_M1);
/*
	for (int i=MP_SIZE; i<size; i--){
		mp_macm_ui(r, f, r[i]);
		mp_shlm(f, MWC_M1);// расчет констант
	} */
}
int main(){

	const uint64_t P[2] = { 0xffffffffffffffff, MWC_A1 - 1 };
uint64_t state[2] = {12345,1};
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
	uint64_t inv = INVL(MWC_A1);
	printf("INVL = 0x%016llx\n", inv);

	printf("Folding[]=\n");
	mp_folding(r, 3, P);
// расчет сдвиговой константы A^{2^16}
	state[0] = MWC_A1, state[1]=0;
	for (i=0; i<(1<<16)-1; i++) mwc128_next(state);
	printf("stat(16) = {0x%016llx, 0x%016llx};\n", state[0], state[1]);

	s[0] = MWC_A1, s[1]=0;
// расчет сдвиговой константы A^{2^16}
	for (i=0; i<16; i++)
		mp_mulm(s, s, s, P);
	printf("jump(%d) = {0x%016llx, 0x%016llx};\n", i, s[0], s[1]);
// расчет сдвиговой константы A^{2^16}
	state[0] = MWC_A1, state[1]=0;
	mwc128_skip(state, (1<<16)-1);
	printf("skip(16) = {0x%016llx, 0x%016llx};\n", state[0], state[1]);

// расчет сдвиговой константы A^{2^32}
	for (; i<32; i++)
		mp_mulm(s, s, s, P);
	printf("jump(%d) = {0x%016llx, 0x%016llx};\n", i, s[0], s[1]);

// расчет сдвиговой константы A^{2^32}
	state[0] = MWC_A1, state[1]=0;
	mwc128_skip(state, (1uLL<<32)-1);
	printf("skip(32) = {0x%016llx, 0x%016llx};\n", state[0], state[1]);

// расчет сдвиговой константы A^{2^64}
	for (; i<64; i++)
		mp_mulm(s, s, s, P);
	printf("jump(64) = {0x%016llx, 0x%016llx};\n", s[0], s[1]);

	state[0] = MWC_A1, state[1]=0;
	mwc128_skip(state, ~0ULL);
	printf("skip(64) = {0x%016llx, 0x%016llx};\n", state[0], state[1]);

// расчет сдвиговой константы A^{2^96}
	for (; i<96; i++)
		mp_mulm(s, s, s, P);
	printf("jump(96) = {0x%016llx, 0x%016llx};\n", s[0], s[1]);
	
	s[0] = MWC_A1, s[1]=0;
	q[0] = -3, q[1] = MWC_A1-1;// обратное число расчитвается, как a^{p-2} = a^{-1}, a^{p-1} = 1
	mp_powm(s,s,q, P);
	printf("inv(P-2) = {0x%016llx, 0x%016llx}; -- %s\n", s[0], s[1], (s[0]==0 && s[1]==1)?"ok":"fail"); // должно давать число B=(1<<64)

	s[0] = MWC_A1, s[1]=-35;
	mp_reduction(s, 0);
	printf("fold(1) = {0x%016llx, 0x%016llx};\n", s[0], s[1]);
	s[0] = MWC_A1, s[1]=-35;
	mp_reduction(s, 8);
	printf("fold(3) = {0x%016llx, 0x%016llx};\n", s[0], s[1]);
	printf("err count = %d\n", err_count);

if (1){// тестирование операции сдвига влево-вправо
	uint64_t B[2] = {0xb3c95d86b3c95d86, 0xffa04e670000};
	uint64_t B2[2]; B2[0]=B[0]; B2[1]=B[1];
	mp_shrm(B, P);
	mp_shrm(B, P);
	mp_shrm(B, P);
	printf("R = 0x%016llX %016llX\n",B[0], B[1]);
	mp_shlm(B, P);
	mp_shlm(B, P);
	mp_shlm(B, P);
	// Операция сдвига влево и вправо должна быть обратимая
	printf("R = 0x%016llX %016llX ..%s\n",B[0], B[1], (B[0]==B2[0] && B[1]==B2[1])?"ok":"fail");
	uint64_t B1[2] = {0xb3c95d86, 0xffa04e67};
	const int sh = 239;
	for (int i=0;i<sh; i++)
		mp_hlvm(B1, P);
	printf("R = 0x%016llX %016llX\n",B1[0], B1[1]);
	for (int i=0;i<sh; i++)
		mp_dubm(B1, P);
	// Операция удвоения и уполовинивания должна быть обратимая
	printf("R = 0x%016llX %016llX ..%s\n",B1[0], B1[1], (B1[0]==0xb3c95d86 && B1[1]==0xffa04e67)?"ok":"fail");
}

	return 0;
	
}
#endif
