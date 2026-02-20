/*! Тестирование скрамблеров на распределение сложности по числу ведущих нулей

Сборка
	$ gcc -DTEST_SCRAMBLER -march=native -O3 -o mwc_scrambler mwc_scrambler.c 
 */
#include <stdint.h>
#include <stdio.h>
static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) ^ (x >> (64 - k));
}
static inline uint32_t rotl32(const uint32_t x, int k) {
	return (x << k) ^ (x >> (32 - k));
}
static inline uint64_t rotr(const uint64_t x, int k) {
	return (x << (64 - k)) ^ (x >> k);
}
static inline uint32_t rotr32(const uint32_t x, int k) {
	return (x << (32 - k)) ^ (x >> k);
}
// Hamming weight for 64-bit integers
static inline int hamming_weight64(uint64_t x) {
    return __builtin_popcountll(x);
}
// Hamming weight for 32-bit integers
static inline int hamming_weight32(uint32_t x) {
    return __builtin_popcount(x);
}
const static uint64_t gamma32 = 0x9e3779b9;
const static uint64_t gamma = 0x9e3779b97f4a7c15;// ~0, 1,3,5,7,11,13, .... (~0<<32) + 1, 
// 0xc45a11730cc8ffe3
// 0x2b13b77d0b289bbd
// 0x40ead42ca1cd0131
// 0xBB67AE8584CAA73B
// Сдвиги 27–33–27, 32–29–32, 33–28–31 — самые частые и удачные.
// Scramblers
static uint64_t count_next(){
	static uint64_t x = 0;
	return x+=gamma;
}
static uint64_t gray_next(){
	static uint64_t x = 0;
	x+=gamma;
	return x^rotl(x,63);
}
static uint64_t gray32R_next(){
	static uint32_t x = 0;
	x+=gamma32;
	return x^rotr32(x,1);
}
static uint64_t gray32L_next(){
	static uint32_t x = 0;
	x+=gamma32;
	return x^rotl32(x,1);
}
static uint64_t scrambler_s32_next(){
	static uint64_t x = 0;
	x+=gamma;
	return x * 0x9E3779BBu;
}
static uint64_t scrambler_s_next(){
	static uint64_t x = 0;
	x+=gamma;
	return x * 0x9e3779b97f4a7c13ull;
}
static uint64_t scrambler_ss_next(){
	static uint64_t x = 0;
	x+=gamma;
	return rotl(x * 5, 7) * 9;
}
static uint64_t scrambler_1_next(){
	static uint64_t x = 0;
	x+=gamma;
	return x^(rotl(x,1)^rotl(x,2));
}
static uint64_t scrambler_1_32_next(){
	static uint32_t x = 0;
	x+=gamma32;
	return x^(rotl32(x,1)^rotl32(x,2));
}
static uint64_t scrambler_1b_next(){
	static uint64_t x = 0;
	x+=gamma;
	return x^(rotl(x,8)^rotl(x,16));
}
static uint64_t scrambler_1b32_next(){
	static uint32_t x = 0;
	x+=gamma32;
	return x^(rotl32(x,8)^rotl32(x,16));
}
static uint64_t scrambler_2_next(){
	static uint64_t x = 0;
	x+=gamma;
	return x^(rotl(~x,1)&rotl(x,2));
}
static uint64_t scrambler_2_32_next(){
	static uint32_t x = 0;
	x+=gamma32;
	return x^(rotl32(~x,1)&rotl32(x,2));
}
static uint64_t scrambler_2b_next(){
	static uint64_t x = 0;
	x+=gamma;
	return x^(rotl(~x,8)&rotl(x,16));
}
static uint64_t scrambler_2b32_next(){
	static uint32_t x = 0;
	x+=gamma32;
	return x^(rotl32(~x,8)&rotl32(x,16));
}
static uint64_t scrambler_3_next(){
	static uint64_t x = ~0;
	x+=gamma;
	return x^(rotl(~x,1)&rotl(x,2)&rotl(x,3));
}
static uint64_t scrambler_3_32_next(){
	static uint32_t x = ~0;
	x+=gamma32;
	return x^(rotl32(~x,1)&rotl32(x,2)&rotl32(x,3));
}
static uint64_t scrambler_3b_next(){
	static uint64_t x = ~0;
	x+=gamma;
	return x^(rotl(~x,8)&rotl(x,16)&rotl(x,24));
}
static uint64_t scrambler_sigma0_next(){
	static uint64_t x = ~0;
	x+=gamma;
	return rotr(x,1)^rotr(x,8)^(x>>7);
}
static uint64_t scrambler_sigma1_next(){
	static uint64_t x = ~0;
	x+=gamma;
	return rotr(x,19)^rotr(x,61)^(x>>6);
}
static uint64_t scrambler_sigma032_next(){
	static uint32_t x = ~0;
	x+=gamma32;
	return rotr32(x,7)^rotr32(x,18)^(x>>3);
}
static uint64_t scrambler_sigma132_next(){
	static uint32_t x = ~0;
	x+=gamma;
	return rotr32(x,17)^rotr32(x,19)^(x>>10);
}
static uint64_t scrambler_sum0_next(){
	static uint64_t x = ~0;
	x+=gamma;
	return rotr(x,28)^rotr(x,34)^rotr(x, 39);
}
static uint64_t scrambler_sum1_next(){
	static uint64_t x = ~0;
	x+=gamma;
	return rotr(x,14)^rotr(x,18)^rotr(x, 41);
}
static uint64_t scrambler_sum032_next(){
	static uint32_t x = ~0;
	x+=gamma32;
	return rotr32(x,2)^rotr32(x,13)^rotr32(x, 22);
}
static uint64_t scrambler_sum132_next(){
	static uint32_t x = ~0;
	x+=gamma32;
	return rotr32(x,6)^rotr32(x,11)^rotr32(x, 25);
}
static uint64_t scrambler_p_next(){
	static uint64_t x = 0;
	x+=gamma;
	return x + rotl(x,32);
}
/*! \see https://vigna.di.unimi.it/ftp/papers/xorshift.pdf */
static uint64_t xorshift64s_next() {
	static uint64_t x = 0;
	x+=gamma;
	x ^= x >> 12; // a
	x ^= x << 25; // b
	x ^= x >> 27; // c
	return x * UINT64_C(0x2545F4914F6CDD1D);
}
static uint64_t xorshift64s_A1_next() {
	static uint64_t x = 0;
	x+=gamma;
	x ^= x >> 11; // a
	x ^= x << 31; // b
	x ^= x >> 18; // c
	return x * UINT64_C(0x5851F42D4C957F2D);
}
static uint64_t xorshift64s_A3_next() {
	static uint64_t x = 0;
	x+=gamma;
	x ^= x >> 18; // c
	x ^= x << 31; // b
	x ^= x >> 11; // a
	return x * UINT64_C(0xd1342543de82ef95);
}
static uint64_t xorshift64s1_A3_next() {
	static uint64_t x = 0;
	x+=gamma;
	x ^= x >> 19; // c
	x ^= x << 29; // b
	x ^= x >>  8; // a
	return x * UINT64_C(0xd605bbb58c8abbfd);
}
// A( 8, 29, 19) - 
// A(11, 31, 18) - хороший миксер A0,A2 const uint64_t M_PCG = 0x5851F42D4C957F2D;

//const static uint64_t MC = 0xdaba0b6eb09322e3u;// Avalanche mixer multiplier
const static uint64_t MC = 0xffebb71d94fcdaf9ull;// Avalanche mixer multiplier
static uint64_t fastmix_next(){
	static uint64_t x = 0;
	uint64_t y = (x+=gamma);
	y = (y^y>>32) * MC;
	return (y^y>>32);
}
static uint64_t lea_next(){
	static uint64_t x = 0;
	uint64_t y = (x+=gamma);
	y = (y ^ y>>32) * MC;
	y = (y ^ y>>32) * MC;
	return (y ^ y>>32);
}
static uint64_t splitmix64() {
	static uint64_t seed = 0;
    uint64_t z = (seed += 0x9e3779b97f4a7c15);
    z = (z ^ z>>30) * 0xbf58476d1ce4e5b9;
    z = (z ^ z>>27) * 0x94d049bb133111eb;
    return z ^ z>>31;
}
static uint64_t scrambler_mwc64() {
	const  uint64_t A0 = 0xfffebaebULL;
	static uint64_t state = 0;
	uint64_t z = (state += 0x9e3779b97f4a7c15);
	return A0*(uint32_t)(state) + (state>>32);
}
// https://github.com/MersenneTwister-Lab/XSadd/blob/master/xsadd.h
static uint32_t xsadd_next()
{
	static uint32_t state[4];
	static const int sh1 = 15;
	static const int sh2 = 18;
	static const int sh3 = 11;
	uint32_t t;
	t  = state[0];
	t ^= t << sh1;
	t ^= t >> sh2;
	t ^= state[3] << sh3;
	state[0] = state[1];
	state[1] = state[2];
	state[2] = state[3];
	state[3] = t;
	return state[2] + state[3]; // `p` скрамблер
}
static uint64_t xorshift128p_next() {
	static uint64_t s[4] = {1,~1};
	uint64_t s1 = s[0];
	const uint64_t s0 = s[1];
	const uint64_t result = s0 + s1;
	s[0] = s0;
	s1 ^= s1 << 23; // a
	s[1] = s1 ^ (s1 >> 18) ^ s0 ^ (s0 >> 5); // b, c
	return result;
}

/*! \brief Генерация псевдо-случайного числа. Один шаг алгоритма */
#define MWC_A0
uint64_t a0 = 0xfffeb81bULL;
uint32_t aa[] = {
    0xFFFEFD4E, 0xFFFEF0E2, 0xFFFECC43, 0xFFFEA405,
    0xFFFE7369, 0xFFFE60AF, 0xFFFE5BD5, 0xFFFE5A0A,
    0xFFFE53E3, 0xFFFE03C4,
};

//#define MWC_A0 0xfffebaebULL
uint64_t state[2] = {-1, 1};
uint64_t mwc64x_next()
{
	uint64_t x = *state;
	*state = a0*(uint32_t)(x) + (x>>32);
    return x;//((x>>32) ^ (x));
}
#define MWC_A1 		0xffebb71d94fcdaf9ull // MWC128  число A1, B1 = (1<<64)
//#define MWC_A1 		0xff3a275c007b8ee6 // MWC128 - другой вариант, см википедию
typedef unsigned int __attribute__((mode(TI)))   uint128_t;
static inline uint64_t mwc128_next() {
	static uint64_t s[2] = {-1, 1};
	const uint64_t result = s[0];
	const uint128_t t = (uint128_t)MWC_A1 * s[0] + s[1];
	s[0] = t;
	s[1] = t >> 64;
	return result;
}
static inline uint64_t mwc128x1b_next() {
	static uint64_t s[2] = {-1, 1};
	const uint64_t x = s[0];
	const uint128_t t = (uint128_t)MWC_A1 * s[0] + s[1];
	s[0] = t;
	s[1] = t >> 64;
	return x^(rotl(~x,8)&rotl(x,16));
}
/* 2019 by David Blackman and Sebastiano Vigna */
static inline uint64_t xoroshiro64s_next() {
	static uint64_t s[2] = {-1, 1};
	const uint32_t s0 = s[0];
	uint32_t s1 = s[1];
	const uint32_t result = s0 * 0x9E3779BB;

	s1 ^= s0;
	s[0] = rotl32(s0, 26) ^ s1 ^ (s1 << 9); // a, b
	s[1] = rotl32(s1, 13); // c

	return result;
}
static inline uint64_t xoroshiro64ss_next() {
	static uint64_t s[2] = {-1, 1};
 	const uint32_t s0 = s[0];
	uint32_t s1 = s[1];
	const uint32_t result = rotl32(s0 * 0x9E3779BB, 5) * 5;

    s1 ^= s0;
	s[0] = rotl32(s0, 26) ^ s1 ^ (s1 << 9); // a, b
	s[1] = rotl32(s1, 13); // c
	return result;
}
uint64_t xoroshiro128p_next()
{
	static uint64_t s[2] = {-1, 1};
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = s0 + s1;
	s1 ^= s0;
	s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
	s[1] = rotl(s1, 37); // c
	return r;	
}
uint64_t xoroshiro128pp_next()
{
	static uint64_t s[2] = {-1, 1};
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = rotl(s0 + s1, 17) + s0;
	s1 ^= s0;
	s[0] = rotl(s0, 49) ^ s1 ^ (s1 << 21); // a, b
	s[1] = rotl(s1, 28); // c
	return r;
}
uint64_t xoroshiro128ss_next()
{
	static uint64_t s[2] = {-1, 1};
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = rotl(s0 * 5, 7) * 9;

	s1 ^= s0;
	s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
	s[1] = rotl(s1, 37); // c
	return r;
}
#include <math.h>
static inline double difficulty(uint64_t x) {
	return 1/((double)x+0.5);
}
double dif_test(const char* name, uint64_t (*next)(), double *sum) {
	#define M 32
	#define DIM 3
	const int m = 1;	  // группировка значений по разрядам 0:1, 1:1.5=3/2, 2:1.875= 15/8; 3:255/128, m: (2^2^m-1)/2^{2^m-1}
	long double diffi = 0; 		// суммарная сложность
    uint64_t count = 1uLL<<32; 	// число отсчетов в тесте
	double hist[M] = {0}; // распределение сложности по категориям
	uint32_t v [M] = {0}; // частоты попадания в каждую категорию
    for (uint64_t k = 0; k< count; k++) {
		uint64_t x = next();
		double d = difficulty(x);
		diffi += d;
		uint32_t x0 = x;
		//if (k%DIM == 1) // фильтр 1/3
		{// распределение по числу нулевых бит
			int i = x0? __builtin_clz(x0): 31;
			hist[(i>>m)] += d; // 
			v   [(i>>m)] ++; // частоты по битовой сложности
		}
	}
	if (sum) {// суммирование по категориям
		for(int i=0;i<M; i++)
			sum[i] += hist[i];
	}
	if (1)  { // вывод подробного отчета по категориям
		printf ("##:  difficulty | frequency | hashrate \n");
		for(int i=0;i<M>>m; i++)
			if (v[i]!=0) {
				double hr = v[i]/(double)(1uLL<<(31 -(i<<m)));
				printf ("%2d: %12.3f| %-10u| %f\n", i, hist[i], v[i], hr);
			}
	}
	return diffi; // суммарная сложность
}
#if defined(TEST_SCRAMBLER)
int main(){
// 32-bit
    uint64_t pow32 = UINT64_C(1) << 32;
    uint32_t gamma32 = (uint32_t) floor( pow32 * (sqrt(5.0) - 1.0) / 2.0 );
    printf("32-bit: 0x%08X\n", gamma32);           // → 0x9E3779B9

    // 64-bit
    long double pow64 = ldexpl(1.0L, 64);          // 2^64 в long double
    uint64_t gamma64 = (uint64_t) floorl( pow64 * (sqrtl(5.0L) - 1.0L) / 2.0L );
    printf("64-bit: 0x%016llX\n", gamma64);        // → 0x9E3779B97F4A7C15
    struct {
        const char* name;
        uint64_t (*next)();
    } gen[] = {
        {"Counter", count_next},
//        {"Gray-64L code", gray_next},
        {"Gray-32R code", gray32R_next},
//        {"Gray-32L code", gray32L_next},
#if 0		
        {"Scrambler 1-32", scrambler_1_32_next},
        {"Scrambler 2-32", scrambler_2_32_next},
        {"Scrambler 3-32", scrambler_3_32_next},
        {"Scrambler 1b32", scrambler_1b32_next},
        {"Scrambler 2b32", scrambler_2b32_next},
        {"Scrambler Sigma0-32", scrambler_sigma032_next},
        {"Scrambler Sigma1-32", scrambler_sigma132_next},
        {"Scrambler Sum0-32", scrambler_sum032_next},
        {"Scrambler Sum1-32", scrambler_sum132_next},
#endif
#if 0
		{"Scrambler 1-64", scrambler_1_next},
        {"Scrambler 2-64", scrambler_2_next},
        {"Scrambler 3-64", scrambler_3_next},
        {"Scrambler 1b", scrambler_1b_next},
        {"Scrambler 2b", scrambler_2b_next},
        {"Scrambler 3b", scrambler_3b_next},
        {"Scrambler Sigma0", scrambler_sigma0_next},
        {"Scrambler Sigma1", scrambler_sigma1_next},
        {"Scrambler Sum0", 	scrambler_sum0_next},
        {"Scrambler Sum1", 	scrambler_sum1_next},
        {"Scrambler *(32)", scrambler_s32_next},
        {"Scrambler *", 	scrambler_s_next},
        {"Scrambler **", 	scrambler_ss_next},
        {"Scrambler +", scrambler_p_next},
#endif
//		{"SplitMix64", splitmix64},
		{"Doug Lea's", lea_next},
		{"MWC64 mix", scrambler_mwc64},
		{"XorShift64 mix", xorshift64s_next},
		{"XS-PCG-mixer", xorshift64s_A1_next},
		{"XS-LCG-mixer", xorshift64s_A3_next},
		{"XS-LCG2-mixer", xorshift64s1_A3_next},
//        {"MWC64x", mwc64x_next},
//        {"MWC128", mwc128_next},
//        {"MWC128x1b", mwc128x1b_next},
//        {"Xoroshiro128++", xoroshiro128pp_next},

    };
  if (0) // 
	for (int i=0; i<32; i++) {
		double x = difficulty(1uLL<<i);
		printf("%2d:diff = %.2f\n", i, x);
	}
  if (1) {// difficulty
	char* name = "MWC128";
	double x;
    int n_tests = sizeof(gen)/sizeof(gen[0]);
	for (int n=0;n<512;n++) {
		printf ("----------------------------------%d\n", n);
        for (int k=0;k<n_tests;k++) {
            x = dif_test(gen[k].name, gen[k].next, NULL);
            printf("|%-16s| %-9.4g\n", gen[k].name, x);
        }
	}
  }
   	return 0;
}
#endif