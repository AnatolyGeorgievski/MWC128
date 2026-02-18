/*!
    $ gcc -std=c99 -Wall -O3 -o test-mwc64r2 test-mwc64r2.c -Iinclude -Llib -ltestu01 -lprobdist -lmylib -lm
    $ ./test-mwc64r2


 */
#include "TestU01.h"
static inline uint64_t fastmix(uint64_t x) {
	x = x ^ (x>>32);
//	x *= 0xdaba0b6eb09322e3ull;
	x *= 0xda942042e4dd58b5ull;
    return x ^ (x>>32);
}
static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}
#define mix(x) (x)
uint32_t A2 = 0xffe118ab;
static uint64_t mwc64r2_next() {
//    const uint32_t A2 = 0xffe118ab;//fffe71fb;
        static uint32_t state[4] = { ~1, 12, 1456, 1};
//    uint32_t r = state[2]^state[3];
    uint64_t t = (uint64_t)A2*state[0] + state[3];
        state[0] = state[1];
        state[1] = state[2];//^state[3];
        state[2] = t;
        state[3] = t>>32;
    return mix(t);
}

#include <unif01.h>
#include <smarsa.h>
#include <scomp.h>
#include <bbattery.h>
#include <swrite.h>

#define next mwc64r2_next
static unsigned long u32bits(void *a, void *b) {
	return (uint32_t)next();
}
static double u01(void *a, void *b) {
	return ((uint32_t)next()) * 0x1.0p-32;
}

void write_state(void *unused) {}


extern int    bbattery_NTests;      // сколько p-value всего
extern double bbattery_pVal[];      // p-value[j-1] — j-й тест
extern char  *bbattery_TestNames[]; // имя j-го теста
#include <string.h>
int main(int argc, char *argv[]) {

// ------------------ массив кандидатов ------------------
    const uint32_t candidates[] = {
0xfffcd08f,
0xffe118ab, 
// 0 — конец
    };
	int sz = sizeof(candidates)/sizeof(uint32_t);
	double 	candidates_min_p[sz];
	double 	candidates_max_p[sz];
	double  candidates_maxof[sz];
	double  candidates_maxad[sz];
	double  candidates_birth[sz];
	int 	candidates_min_t[sz];
	char name[64]; 
	unif01_Gen gen;
	gen.name = name;
	gen.Write = write_state;
	gen.GetU01 = u01;
	gen.GetBits = u32bits;

	swrite_Basic = FALSE;
/* Цикл выбора параметра A0 по результатам теста SmallCrush
особое внимание тестам smarsa_BirthdaySpacings и sknuth_MaxOft 
 */
int count = 5;
while(count-->0) {
for (int i=0; i<sz; i++){
	A2 = candidates[i];
	sprintf(name, "MWC64r2,A=%08x", A2);
	bbattery_SmallCrush(&gen);
	double min_p = 1;
	double max_p = 0;
	candidates_min_t[i] = 0;
	for (int j = 0; j < bbattery_NTests; j++) {
		double p = bbattery_pVal[j];
		if (strcmp("MaxOft", bbattery_TestNames[j])==0)
			candidates_maxof[i] = p;
		else
		if (strcmp("MaxOft AD", bbattery_TestNames[j])==0)
			candidates_maxad[i] = p;
		else
		if (strcmp("BirthdaySpacings", bbattery_TestNames[j])==0)
			candidates_birth[i] = p;
		if (p < min_p) {
			min_p = p;
			candidates_min_t[i] = j;
		}
		if (p > max_p) {
			max_p = p;
			candidates_min_t[i] = j;
		}
	}
	candidates_min_p[i] = min_p;
	candidates_max_p[i] = max_p;
}
/* Вывести сравнительную таблицу по ряду параметров */
double eps = 0.01;
printf("A0       | Result | BirthSp| MaxOft | Max.AD | min p - max p | Weak Test\n");
for (int i=0; i<sz; i++){
	int result = (candidates_min_p[i]>eps && candidates_max_p[i]<1-eps);
	printf("%08x | %6s | %-6.3f | %-6.3f | %-6.3f | %-5.3f - %-5.3f | %s\n", candidates[i], 
		result? "PASS":"FAIL",
		candidates_birth[i], candidates_maxof[i], candidates_maxad[i],
		candidates_min_p[i], candidates_max_p[i],  bbattery_TestNames[candidates_min_t[i]]);
}
}
/* Запустить полный тест для выбранных значений */
    A2 = 0xffe118ab;
	bbattery_Crush(&gen);
for (int i=0; i<bbattery_NTests; i++) {
	printf("%-16s: %8.3g\n", bbattery_TestNames[i], bbattery_pVal[i]);
}
	return 0;
	/* The following TestU01 tests give an idea of how the linear degree
	   increases from low to high bits using the + scrambler. Linearity tests
	   are failed on the lowest bits, but are passed as we go up. Estimations of
	   the linear degree can be found in "Scrambled Linear Pseudorandom Number
	   Generators".

	   The failures are due exclusively to the bits of low degree, as there are
	   no linear dependencies between the bits. */

	// 600x600 Binary-rank test on the lower 4 bits (fail)
	smarsa_MatrixRank(&gen, NULL, 1, 100, 28,  4, 600, 600);

	// 5000x5000 binary-rank test on the lower 10 bits (fail)
	smarsa_MatrixRank(&gen, NULL, 1, 100, 22,  10, 2000, 2000);

	// 10000x10000 binary-rank test on bits [5..15) (pass)
//	smarsa_MatrixRank(&gen, NULL, 1, 100, 17,  10, 10000, 10000);
for (int i=32; i-->0;){
	// Linear-complexity test on bit 0 (fail)
	scomp_LinearComp(&gen, NULL, 1, 400000, i, 1);
}
	// Linear-complexity test with larger bound on bit 2 (fail)
//	scomp_LinearComp(&gen, NULL, 1, 1000000, 29, 1);
    return 0;
}

