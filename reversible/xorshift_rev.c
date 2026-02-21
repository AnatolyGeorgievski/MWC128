#include <stdint.h>
#include <stdbool.h>
#include <math.h>
/*!
A0 x ^= x << a; x ^= x >> b; x ^= x << c; X1
A1 x ^= x >> a; x ^= x << b; x ^= x >> c; X3
A2 x ^= x << c; x ^= x >> b; x ^= x << a; X2
A3 x ^= x >> c; x ^= x << b; x ^= x >> a; X4
A4 x ^= x << a; x ^= x << c; x ^= x >> b; X5
A5 x ^= x >> a; x ^= x >> c; x ^= x << b; X6
A6 x ^= x >> b; x ^= x << a; x ^= x << c; X7
A7 x ^= x << b; x ^= x >> a; x ^= x >> c; X8
 */
// A( 8, 29, 19) - 
// A(11, 31, 18) - хороший миксер A0,A2 const uint64_t M_PCG = 0x5851F42D4C957F2D;
// трансформации A_{2n}-A1_{2n+1} сопряженные по операции bit-reverse
// G₂(x) = bit_reverse(G₁(bit_reverse(x)))
uint64_t xorshiftA0(uint64_t x, int a, int b, int c) {
	x ^= x << a; x ^= x >> b; x ^= x << c;
	return x;
}
uint64_t xorshiftA1(uint64_t x, int a, int b, int c) {
	x ^= x >> a; x ^= x << b; x ^= x >> c;
	return x;
}
uint64_t xorshiftA2(uint64_t x, int a, int b, int c) {
	x ^= x << c; x ^= x >> b; x ^= x << a;
	return x;
}
uint64_t xorshiftA3(uint64_t x, int a, int b, int c) {
	x ^= x >> a; x ^= x << b; x ^= x >> c;
	return x;
}
static uint64_t bit_reverse(uint64_t x) {
	x = ((x >> 1) & 0x5555555555555555ULL) | ((x & 0x5555555555555555ULL) << 1);
    x = ((x >> 2) & 0x3333333333333333ULL) | ((x & 0x3333333333333333ULL) << 2);
    x = ((x >> 4) & 0x0F0F0F0F0F0F0F0FULL) | ((x & 0x0F0F0F0F0F0F0F0FULL) << 4);	
	return __builtin_bswap64(x);
}
// сдвиговые константы для генераторов xorshift
const uint8_t xsh[][3] = {
{ 1, 1,54}, { 1, 1,55}, { 1, 3,45}, { 1, 7, 9}, { 1, 7,44}, { 1, 7,46}, { 1, 9,50}, { 1,11,35}, { 1,11,50}, 
{ 1,13,45}, { 1,15, 4}, { 1,15,63}, { 1,19, 6}, { 1,19,16}, { 1,23,14}, { 1,23,29}, { 1,29,34}, { 1,35, 5}, 
{ 1,35,11}, { 1,35,34}, { 1,45,37}, { 1,51,13}, { 1,53, 3}, { 1,59,14}, { 2,13,23}, { 2,31,51}, { 2,31,53}, 
{ 2,43,27}, { 2,47,49}, { 3, 1,11}, { 3, 5,21}, { 3,13,59}, { 3,21,31}, { 3,25,20}, { 3,25,31}, { 3,25,56}, 
{ 3,29,40}, { 3,29,47}, { 3,29,49}, { 3,35,14}, { 3,37,17}, { 3,43, 4}, { 3,43, 6}, { 3,43,11}, { 3,51,16}, 
{ 3,53, 7}, { 3,61,17}, { 3,61,26}, { 4, 7,19}, { 4, 9,13}, { 4,15,51}, { 4,15,53}, { 4,29,45}, { 4,29,49}, 
{ 4,31,33}, { 4,35,15}, { 4,35,21}, { 4,37,11}, { 4,37,21}, { 4,41,19}, { 4,41,45}, { 4,43,21}, { 4,43,31}, 
{ 4,53, 7}, { 5, 9,23}, { 5,11,54}, { 5,15,27}, { 5,17,11}, { 5,23,36}, { 5,33,29}, { 5,41,20}, { 5,45,16}, 
{ 5,47,23}, { 5,53,20}, { 5,59,33}, { 5,59,35}, { 5,59,63}, { 6, 1,17}, { 6, 3,49}, { 6,17,47}, { 6,23,27}, 
{ 6,27, 7}, { 6,43,21}, { 6,49,29}, { 6,55,17}, { 7, 5,41}, { 7, 5,47}, { 7, 5,55}, { 7, 7,20}, { 7, 9,38}, 
{ 7,11,10}, { 7,11,35}, { 7,13,58}, { 7,19,17}, { 7,19,54}, { 7,23, 8}, { 7,25,58}, { 7,27,59}, { 7,33, 8}, 
{ 7,41,40}, { 7,43,28}, { 7,51,24}, { 7,57,12}, { 8, 5,59}, { 8, 9,25}, { 8,13,25}, { 8,13,61}, { 8,15,21}, 
{ 8,25,59}, { 8,29,19}, { 8,31,17}, { 8,37,21}, { 8,51,21}, { 9, 1,27}, { 9, 5,36}, { 9, 5,43}, { 9, 7,18}, 
{ 9,19,18}, { 9,21,11}, { 9,21,20}, { 9,21,40}, { 9,23,57}, { 9,27,10}, { 9,29,12}, { 9,29,37}, { 9,37,31}, 
{ 9,41,45}, {10, 7,33}, {10,27,59}, {10,53,13}, {11, 5,32}, {11, 5,34}, {11, 5,43}, {11, 5,45}, {11, 9,14}, 
{11, 9,34}, {11,13,40}, {11,15,37}, {11,23,42}, {11,23,56}, {11,25,48}, {11,27,26}, {11,29,14}, {11,31,18}, 
{11,53,23}, {12, 1,31}, {12, 3,13}, {12, 3,49}, {12, 7,13}, {12,11,47}, {12,25,27}, {12,39,49}, {12,43,19}, 
{13, 3,40}, {13, 3,53}, {13, 7,17}, {13, 9,15}, {13, 9,50}, {13,13,19}, {13,17,43}, {13,19,28}, {13,19,47}, 
{13,21,18}, {13,21,49}, {13,29,35}, {13,35,30}, {13,35,38}, {13,47,23}, {13,51,21}, {14,13,17}, {14,15,19}, 
{14,23,33}, {14,31,45}, {14,47,15}, {15, 1,19}, {15, 5,37}, {15,13,28}, {15,13,52}, {15,17,27}, {15,19,63}, 
{15,21,46}, {15,23,23}, {15,45,17}, {15,47,16}, {15,49,26}, {16, 5,17}, {16, 7,39}, {16,11,19}, {16,11,27}, 
{16,13,55}, {16,21,35}, {16,25,43}, {16,27,53}, {16,47,17}, {17,15,58}, {17,23,29}, {17,23,51}, {17,23,52}, 
{17,27,22}, {17,45,22}, {17,47,28}, {17,47,29}, {17,47,54}, {18, 1,25}, {18, 3,43}, {18,19,19}, {18,25,21}, 
{18,41,23}, {19, 7,36}, {19, 7,55}, {19,13,37}, {19,15,46}, {19,21,52}, {19,25,20}, {19,41,21}, {19,43,27}, 
{20, 1,31}, {20, 5,29}, {21, 1,27}, {21, 9,29}, {21,13,52}, {21,15,28}, {21,15,29}, {21,17,24}, {21,17,30}, 
{21,17,48}, {21,21,32}, {21,21,34}, {21,21,37}, {21,21,38}, {21,21,40}, {21,21,41}, {21,21,43}, {21,41,23}, 
{22, 3,39}, {23, 9,38}, {23, 9,48}, {23, 9,57}, {23,13,38}, {23,13,58}, {23,13,61}, {23,17,25}, {23,17,54}, 
{23,17,56}, {23,17,62}, {23,41,34}, {23,41,51}, {24, 9,35}, {24,11,29}, {24,25,25}, {24,31,35}, {25, 7,46}, 
{25, 7,49}, {25, 9,39}, {25,11,57}, {25,13,29}, {25,13,39}, {25,13,62}, {25,15,47}, {25,21,44}, {25,27,27}, 
{25,27,53}, {25,33,36}, {25,39,54}, {28, 9,55}, {28,11,53}, {29,27,37}, {31, 1,51}, {31,25,37}, {31,27,35}, 
{33,31,43}, {33,31,55}, {43,21,46}, {49,15,61}, {55, 9,56}
};
const static uint64_t gamma = 0x9e3779b97f4a7c15;
/*! 
Константы M₃₂, M₈ и M₂ взяты из работы L’Ecuyer (1999) и дополнены Richard Simard; 
они обладают высокими значениями figure of merit для размерностей t=32, t=8 и t=2 
соответственно (Vigna, 2016, Table IV).

Три множителя из Table IV Vigna (2016) — это примитивные элементы мультипликативной группы ℤ/2⁶⁴ℤ, 
отобранные по критерию figure of merit (метрика L’Ecuyer 1999 для оценки качества мультипликативных 
конгруэнтных генераторов в t-мерном пространстве). Они взяты из теории комбинированных множественных 
рекуррентных генераторов (CMRG/MCG).

Pierre L’Ecuyer, «Good Parameters and Implementations for Combined Multiple Recursive Random Number Generators», Operations Research, Vol. 47, No. 1 (1999), pp. 159–164.
https://pubsonline.informs.org/doi/10.1287/opre.47.1.159
(https://www.iro.umontreal.ca/~lecuyer/myftp/papers/goodcomb.pdf)
*/
const uint64_t GR  = 0x9e3779b97f4a7c15; // golden ratio
const uint64_t WY  = 0x60bee2bee120fc15; // wyrand increment
const uint64_t M32 = 2685821657736338717u; 
const uint64_t M8  = 1181783497276652981u;
const uint64_t M2  = 8372773778140471301u;
// 117E8BB3680E4B7C
const uint64_t M_PCG = 0x5851F42D4C957F2D;
const uint64_t M_LXM = 0xd1342543de82ef95;
const uint64_t M_LXM2= 0xd605bbb58c8abbfd;
const uint64_t M_LEH = 0xda942042e4dd58b5;// Lehmer64 multiplier
const uint64_t M_WYH = 0xa3b195354a39b70d;// WYhash multiplier
bool has_max_order(uint64_t x) {
    int count = 61; // 2^61
    do { 
		x = x * x; // x ← x² mod 2⁶⁴
		if (x <= 1) return false; 
	} while (--count);
    return true;
}
bool has_max_order32(uint32_t x) {
    int count = 29;
    do { 
		x = x * x; // x ← x² mod 2^{32}
		if (x <= 1) return false; 
	} while (--count);
    return true;
}
static uint64_t inverse_u64(uint64_t a) {
    uint64_t x = a;
    // 5 итераций — стандарт для 64 бит
    x *= 2ULL - a * x;
    x *= 2ULL - a * x;
    x *= 2ULL - a * x;
    x *= 2ULL - a * x;
    x *= 2ULL - a * x;
    return x;
}

#include <stdio.h>
int main (){
    int const nr = 8;
    int i;
    printf("xorshift transform*\n");
	int sz = sizeof(xsh)/(3);
	uint64_t x = 0;
	for (int i=0; i<sz; i++) {
		int a = xsh[i][0], b = xsh[i][1], c = xsh[i][2];
		x+= gamma;
		uint64_t y = xorshiftA0(x, a, b, c);
		uint64_t z = bit_reverse(xorshiftA1(bit_reverse(x), a, b, c));
		printf("%016llx ..%s\n", x, y==z? "★ ok":"☆ fail");
	}

	bool ok =  has_max_order(M_PCG);
	printf("max order primes\n");
	printf("%016llx ..%s\n", M_PCG, ok?"yes":"no");
	printf("%016llx ..%-3s %016llx\n", M_LXM, has_max_order(M_LXM)?"yes":"no", inverse_u64(M_LXM));
	printf("%016llx ..%-3s %016llx\n", M_LXM2, has_max_order(M_LXM2)?"yes":"no", inverse_u64(M_LXM2));
	printf("%016llx ..%-3s %016llx\n", M_LEH, has_max_order(M_LEH)?"yes":"no", inverse_u64(M_LEH));
	printf("%016llx ..%-3s %016llx\n", M_WYH, has_max_order(M_WYH)?"yes":"no", inverse_u64(M_WYH));
	printf("%016llx ..%s\n", GR, has_max_order(GR)?"yes":"no");
	printf("%016llx ..%s\n", M2, has_max_order(M2)?"yes":"no");
	printf("%016llx ..%s\n", M8, has_max_order(M8)?"yes":"no");
	printf("%016llx ..%s\n", M32, has_max_order(M32)?"yes":"no");
	printf("%08llx ..%s\n", GR>>32, has_max_order32(GR>>32)?"yes":"no");


	struct _op {
		const char* name;
		int a;
		uint64_t prime1;
		int b;
		uint64_t prime2;
		int c;
	} op[] = {
{"Custom 1",    32, 0xbf58476d1ce4e5b9, 28, 0x94d049bb133111eb, 33},
{"Custom2",     32, 0xdaba0b6eb09322e3, 33, 0xa6f8e26927e132cb, 32},
{"SplitMix64",  30, 0xbf58476d1ce4e5b9, 27, 0x94d049bb133111eb, 31},
//{"MurmurHash3", 33, 0xff51afd7ed558ccd, 33, 0xc4ceb9fe1a85ec53, 33},
{"Custom2",     32, 0xda942042e4dd58b5, 32, 0xda942042e4dd58b5, 32},
{"Doug Lea's",  32, 0xdaba0b6eb09322e3, 32, 0xdaba0b6eb09322e3, 32},
{"unmix Lea's", 32, 0xa6f8e26927e132cb, 32, 0xa6f8e26927e132cb, 32},
{"Avalanche",   33, 0xC2B2AE3D27D4EB4F, 29, 0x165667B19E3779F9, 32},
{"Mix3",        32, 0xbea225f9eb34556d, 29, 0xbea225f9eb34556d, 32},

	{"Mix01",31 ,0x7fb5d329728ea185 ,27 ,0x81dadef4bc2dd44d ,33},
	{"Mix02",33	,0x64dd81482cbd31d7	,31	,0xe36aa5c613612997	,31},
	{"Mix03",31	,0x99bcf6822b23ca35	,30	,0x14020a57acced8b7	,33},
	{"Mix04",33	,0x62a9d9ed799705f5	,28	,0xcb24d0a5c88c35b3	,32},
	{"Mix05",31	,0x79c135c1674b9add	,29	,0x54c77c86f6913e45	,30},
	{"Mix06",31	,0x69b0bc90bd9a8c49	,27	,0x3d5e661a2a77868d	,30},
	{"Mix07",30	,0x16a6ac37883af045	,26	,0xcc9c31a4274686a5	,32},
	{"Mix08",30	,0x294aa62849912f0b	,28	,0x0a9ba9c8a5b15117	,31},
	{"Mix09",32	,0x4cd6944c5cc20b6d	,29	,0xfc12c5b19d3259e9	,32},
	{"Mix10",30	,0xe4c7e495f4c683f5	,32	,0xfda871baea35a293	,33},
	{"Mix11",27	,0x97d461a8b11570d9	,28	,0x02271eb7c6c4cd6b	,32},
	{"Mix12",29	,0x3cd0eb9d47532dfb	,26	,0x63660277528772bb	,33},
	{"Mix13",30	,0xbf58476d1ce4e5b9	,27	,0x94d049bb133111eb	,31},
	{"Mix14",30	,0x4be98134a5976fd3	,29	,0x3bc0993a5ad19a13	,31},
	};
    struct _op *m = NULL;
	uint64_t p1, p2;
    double mean_bias, max_bias;
    sz = sizeof(op)/sizeof(struct _op);
    printf("|%-12s| | %-16s | ord | %-16s | ord |\n", "Mixer", "prime1","prime2");

    for (int i=0; i<sz;i++){
        m = op+i;
		p1 = inverse_u64(m->prime1);
		p2 = inverse_u64(m->prime2);
		printf("|%-12s| | %016llx | %-3s ", m->name, m->prime1, has_max_order(m->prime1)?"yes":"no");
		printf(        "| %016llx | %-3s |", m->prime2, has_max_order(m->prime2)?"yes":"no");
		printf("%s\n", 
			m->prime1 == m->prime2?"symmetric":
			p1 == m->prime2?"conjugated": 
			has_max_order(p1) && has_max_order(p2)?"p_inv max order":"");
        // max_bias = sac(mixer, &mean_bias);
        // printf("|%-12s| %9.6f | %9.6f |\n", m->name, max_bias, mean_bias);
    }
	printf("Prime1 Prime2\n");
	uint64_t g = M_LXM2;
	x=1;
	if (0) for (int i=0; i<0xFFFF; i++){
		x = x*g;
		p1 = inverse_u64(x);
		if ((x>>62) ==3 && (int64_t)p1 < 0 && has_max_order(x) && has_max_order(p1)){
			printf("{\"M^%d\", 32,0x%016llxu,29, 0x%016llxu,32},\n", i,x, p1);
		}
	}
	printf ("..done\n");
	return 0;
}