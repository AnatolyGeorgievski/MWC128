
#include "TestU01.h"
#include <stdint.h>

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}
int64_t A0 = 0x7ff804cf;
static inline uint64_t mwc64s_next() {
    static int64_t h = { 1};
	uint64_t x = h;
    h = (h>>32) - (int64_t)A0*(uint32_t)h;
    return x^(x>>32);
}
static inline uint64_t mixer_next() {
    static int64_t h = { 1};
	uint64_t x = h;
    h = A0*(uint32_t)h;
    return x^(x>>32);
}

#include <unif01.h>
#include <smarsa.h>
#include <scomp.h>
#include <bbattery.h>
#include <swrite.h>
typedef struct _Cond Cond_t;
struct _Cond {
	double birth;
	double maxof;
	double maxad;
	double min_p;
	double max_p;
	int    min_t;
	int    max_t;
};
void collect(Cond_t * candidate);
#define next mwc64s_next
static unsigned long u32bits(void *a, void *b) {
	uint64_t x = next();
	return (uint32_t)x;
}
static double u01(void *a, void *b) {
	return (next()&((1uLL<<63) - 1)) * 0x1.0p-63;
}
void write_state(void *unused) {}
extern int    bbattery_NTests;      // сколько p-value всего
extern double bbattery_pVal[];      // p-value[j-1] — j-й тест
extern char  *bbattery_TestNames[]; // имя j-го теста
#include <string.h>
int main(int argc, char *argv[]) {
	char name[32];
	unif01_Gen gen;
	gen.name = name;
	gen.Write = write_state;
	gen.GetU01 = u01;
	gen.GetBits = u32bits;
const uint64_t cand[] = {
0x7ff9b49300000001, 0x7ff9b38500000001, 0x7ff9aabb00000001, 0x7ff99f8d00000001, 0x7ff99db900000001, 0x7ff99d3b00000001, 0x7ff99b9100000001,
0x7ff9981300000001, 0x7ff9948900000001, 0x7ff991d700000001, 0x7ff98abd00000001, 0x7ff989e500000001, 0x7ff9862500000001, 0x7ff983a900000001,
0x7ff9820b00000001, 0x7ff97c1700000001, 0x7ff97c1100000001, 0x7ff979ef00000001, 0x7ff9770700000001, 0x7ff9729900000001, 0x7ff96f2d00000001,
0x7ff96c0300000001, 0x7ff9689700000001, 0x7ff9673b00000001, 0x7ff9669900000001, 0x7ff964c500000001, 0x7ff9631500000001, 0x7ff9623100000001,
0x7ff95c1300000001, 0x7ff95b7700000001, 0x7ff959d300000001, 0x7ff9576300000001, 0x7ff9523b00000001, 0x7ff9503700000001, 0x7ff94f1100000001,
0x7ff94ef900000001, 0x7ff94cef00000001, 0x7ff94b3900000001, 0x7ff9493500000001, 0x7ff9444f00000001, 0x7ff9444900000001, 0x7ff943d700000001,
0x7ff9439500000001, 0x7ff9426f00000001, 0x7ff93c8700000001, 0x7ff93b6100000001, 0x7ff9398700000001, 0x7ff9397b00000001, 0x7ff9380100000001,
0x7ff9329d00000001, 0x7ff9327300000001, 0x7ff9321f00000001, 0x7ff92f2b00000001, 0x7ff92ea100000001, 0x7ff92cd900000001, 0x7ff92a9900000001,
0x7ff9286500000001, 0x7ff9226500000001, 0x7ff9208b00000001, 0x7ff91d8500000001, 0x7ff91ca700000001, 0x7ff9170700000001, 0x7ff9168f00000001,
0x7ff9165900000001, 0x7ff9122700000001, 0x7ff90ed300000001, 0x7ff90c5d00000001, 0x7ff90b9700000001, 0x7ff903b100000001, 0x7ff902af00000001,

0x7ff8fcd300000001, 0x7ff8f7c900000001, 0x7ff8f78d00000001, 0x7ff8f49300000001, 
0x7ff8f1c900000001, 0x7ff8f0bb00000001, 0x7ff8e68300000001, 0x7ff8e0e300000001,
0x7ff8d83d00000001, 0x7ff8d50d00000001, 0x7ff8d1cb00000001, 0x7ff8d08100000001, 0x7ff8d06900000001, 
0x7ff8c8a100000001, 0x7ff8c87100000001,  
};
	swrite_Basic = FALSE;
	int sz = sizeof(cand)/sizeof(uint64_t);
	Cond_t candidate[sz];
	int count = 25;
	double eps=0.003;
  for(int n =0; n<count; n++){
	eps = 0.002;
	for (int i=0; i<sz; i++) {
		if (n>0 && (candidate[i].min_p<eps || candidate[i].max_p>1-eps)) continue;
		A0= cand[i]>>32;
		sprintf(name, "MWC64s ,A=%08x",(uint32_t) A0);
		bbattery_SmallCrush(&gen);
		collect(candidate+i);
	}
	/* Вывести сравнительную таблицу по ряду параметров */
//	eps = 0.01;
	printf("A0       | Result | BirthSp| MaxOft | Max.AD | min p - max p | Weak Test\n");
	for (int i=0; i<sz; i++){
		int result = (candidate[i].min_p>eps && candidate[i].max_p<1-eps);
		printf("%08lx | %6s | %-6.3f | %-6.3f | %-6.3f | %-5.3f - %-5.3f | %s\n", cand[i], 
			result? "\e[32mPASS\e[m":"\e[31mFAIL\e[m",
			candidate[i].birth, candidate[i].maxof, candidate[i].maxad,
			candidate[i].min_p, candidate[i].max_p,  bbattery_TestNames[candidate[i].min_t]);
	}
  }
	bbattery_Crush(&gen);
    return 0;
}
typedef struct _Cond Cond_t;
void collect(Cond_t * candidate){
	double min_p = 1;
	double max_p = 0;
	candidate->min_t = 0;
	candidate->max_t = 0;
	for (int j = 0; j < bbattery_NTests; j++) {
		double p = bbattery_pVal[j];
		if (strcmp("MaxOft", bbattery_TestNames[j])==0)
			candidate->maxof = p;
		else
		if (strcmp("MaxOft AD", bbattery_TestNames[j])==0)
			candidate->maxad = p;
		else
		if (strcmp("BirthdaySpacings", bbattery_TestNames[j])==0)
			candidate->birth = p;
		if (p < min_p) {
			min_p = p;
			candidate->min_t = j;
		}
		if (p > max_p) {
			max_p = p;
			candidate->max_t = j;
		}
	}
	candidate->min_p = min_p;
	candidate->max_p = max_p;
}