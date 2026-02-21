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
// Weyl sequence: d+=362437; period 2^{32}
const static uint64_t gamma32 = 362437;
const static uint64_t gamma = ~0;// ~0, 1,3,5,7,11,13, .... (~0<<32) + 1, 
const static uint64_t gamma_ms = 0xb5ad4eceda1ce2a9;
// 0x9e3779b97f4a7c15
// 0xc45a11730cc8ffe3
// 0x2b13b77d0b289bbd
// 0xb5ad4eceda1ce2a9 -- https://arxiv.org/pdf/1704.00358 Middle Square
// 0x278c5a4d8419fe6b --
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
static uint64_t scrambler_xor(){
	static uint64_t x = 0;
	x+=gamma;
	return x^x>>32;
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
	x ^= x >>  8; // a
	x ^= x << 29; // b
	x ^= x >> 19; // c
//	x *= UINT64_C(0xd605bbb58c8abbfd);
	return x;
}
// A( 8, 29, 19) - 
// A(11, 31, 18) - хороший миксер A0,A2 const uint64_t M_PCG = 0x5851F42D4C957F2D;

//const static uint64_t MC = 0xdaba0b6eb09322e3u;// Avalanche mixer multiplier
const static uint64_t MC = 0xda942042e4dd58b5u;// Avalanche mixer Lehmer multiplier
//const static uint64_t MC = 0xffebb71d94fcdaf9ull;// Avalanche mixer multiplier
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
static uint64_t mwc64s_mix(){
	const  uint64_t A0 = 0x7ff8c871;
	static uint64_t x = 0;
	uint64_t y = (x += gamma);
	y = (y>>32) - (uint32_t)y*A0;
	y = (y>>32) - (uint32_t)y*A0;
	return y;
}
static uint64_t mwc64_mix() {
	const  uint64_t A0 = 0xfffebaeb;
	static uint64_t x = 0;
	uint64_t y = (x += gamma);
	y = (y>>32) + (uint32_t)y*A0;
	y = (y>>32) + (uint32_t)y*A0;
	return y;
}
// da942042e4dd58b5 ..yes 8b838d0354ead59d
static inline uint64_t mix_1584() {
 	static uint64_t x = 0;
	uint64_t h = (x+=gamma);
  h ^= h >> 32;
  h *= 0xda942042e4dd58b5;
  h ^= h >> 32;
  h *= 0xda942042e4dd58b5;
  h ^= h >> 32;
  return h;
}
static uint64_t splitmix64() {
	static uint64_t seed = 0;
    uint64_t z = (seed += gamma);
    z = (z ^ z>>30) * 0xbf58476d1ce4e5b9;
    z = (z ^ z>>27) * 0x94d049bb133111eb;
    return z ^ z>>31;
}
static inline uint64_t mix_stafford13() {
	static uint64_t seed = 0;
    uint64_t h = (seed += gamma);
   h ^= h >> 30;
  h *= 0xbf58476d1ce4e5b9ull;
  h ^= h >> 27;
  h *= 0x94d049bb133111ebull;
  h ^= h >> 31;
  return h;
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
static uint64_t msws64_next() {
	static uint64_t x = 0, w1 = 0, s1 = 0xb5ad4eceda1ce2a9;
	static uint64_t y = 0, w2 = 0, s2 = 0x278c5a4d8419fe6b;
	x = rotl(x,32); x = x*x + (w1 += s1); 
	y = rotl(y,32); y = y*y + (w2 += s2); 
	return x^y;
}
static uint64_t ms64_mix() {
	static uint64_t x = 0, w = 0;
	x = x*x + (w += gamma); x = rotl(x,32); 
	return x;
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

uint64_t lehmer64_next() {
	static __uint128_t state=0;
    state *= 0xda942042e4dd58b5ULL;   // фиксированный множитель (хороший 64-бит → 128-бит)
    return (uint64_t)(state >> 64);   // берём старшие 64 бита как результат
}
uint64_t lehmer64_mix() {
	static uint64_t s = 0;
	static __uint128_t x = 0;  // 128-битное состояние (обычно глобальная переменная)
    x = x * 0xda942042e4dd58b5uLL + (gamma_ms);   // фиксированный множитель (хороший 64-бит → 128-бит)
    return (uint64_t)x;   // берём старшие 64 бита как результат
}
// https://github.com/wangyi-fudan/wyhash
uint64_t wyrand(uint64_t *seed) {
    *seed += 0x60bee2bee120fc15ULL;
    __uint128_t tmp = (__uint128_t)(*seed) * 0xa3b195354a39b70dULL;
    return (uint64_t)(tmp >> 64) ^ (uint64_t)tmp;
}
// wyhash (автор Wang Yi, ~2019 год, последняя стабильная версия — final3/final4)
static uint64_t wyrand_mix() {
	static uint64_t s = 0;
    s += gamma;//0x60bee2bee120fc15ULL;
    __uint128_t y = s * (__uint128_t)0xa3b195354a39b70dULL;
    return y;//^(y >> 64);
}
// https://github.com/vnmakarov/mum-hash
static uint64_t _mum_primes[] = {
  0x9ebdcae10d981691, 0x32b9b9b97a27ac7d, 0x29b5584d83d35bbd, 0x4b04e0e61401255f,
  0x25e8f7b1f1c9d027, 0x80d4c8c000f3e881, 0xbd1255431904b9dd, 0x8a3bd4485eee6d81,
  0x3bc721b2aad05197, 0x71b1a19b907d6e33, 0x525e6c1084a8534b, 0x9e4c2cd340c1299f,
  0xde3add92e94caa37, 0x7e14eadb1f65311d, 0x3f5aa40f89812853, 0x33b15a3b587d15c9,
};

#include <math.h>
static inline double difficulty(uint64_t x) {
	return 1/((double)x+0.5);
}
#define M 32
double dif_test(const char* name, uint64_t (*next)(), uint64_t *sum, int Nr) {
	#define DIM 3
	const int m = 0;	  // группировка значений по разрядам 0:1, 1:1.5=3/2, 2:1.875= 15/8; 3:255/128, m: (2^2^m-1)/2^{2^m-1}
	double r = 1;
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
			sum[i] += v[i];
	}
	if (1)  { // вывод подробного отчета по категориям
		printf ("##:  difficulty | frequency | hashrate | avg.hrate |\n");
		for(int i=0;i<M>>m; i++)
			if (v[i]!=0) {
				//double P =(double)1.0/(1uLL<<(31 -(i<<m)));
				int ex = -(31 -(i<<m));
				double hr, ar = 0;
				hr = __builtin_ldexp(  v[i],ex);
				double r1 = (M>>m)-1 == i?2:r;
				int ok;
				if (sum) {
					ar = __builtin_ldexp(sum[i],ex)/(Nr+1);
					// KS statistics max(D+,D-)\sqrt(n) <= eps
					ok = fabs(ar - r1)* sqrt(__builtin_ldexp((Nr+1),-ex)) <= r1;
				} else
					ok = fabs(hr - r1)<= r1* sqrt(__builtin_ldexp(  1,ex));
				printf ("%2d: %12.3f| %-10u| %8.6f | %9.7f |%s\n", i, hist[i], v[i], hr, ar, ok?"":" fail");
			}
	}
	return diffi; // суммарная сложность
}
#if defined(TEST_SCRAMBLER)
int main(){
    // 64-bit
    long double pow64 = ldexp(1.0, 64);          // 2^64 в double
    uint64_t gamma64 = (uint64_t) floorl( pow64 * (sqrtl(5.0L) - 1.0L) / 2.0L );
    printf("64-bit: 0x%016llX\n", gamma64);        // → 0x9E3779B97F4A7C15
    struct {
        const char* name;
        uint64_t (*next)();
		double diff;
		uint64_t sum[M];
    } gen[] = {
//        {"Counter", count_next},
//        {"Gray-64L code", gray_next},
//        {"XOR mix", scrambler_xor},
//        {"Gray-32R code", gray32R_next},
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
//		{"Doug Lea's", lea_next},
//		{"Stafford 13", mix_stafford13},
		{"MWC64 mix",  mwc64_mix},
		{"mwc64s-mix", mwc64s_mix},
//		{"MidSquare", msws64_next},
//		{"MidSquare mix", ms64_mix},
		//{"XorShift64 mix", xorshift64s_next},
		// {"XS-PCG-mixer", xorshift64s_A1_next},
		// {"XS-LCG-mixer", xorshift64s_A3_next},
//		{"XorShift mix", xorshift64s1_A3_next},
		{"Lehmer64 mix", lehmer64_mix},
		{"WYrand mix", wyrand_mix},
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
            gen[k].diff += dif_test(gen[k].name, gen[k].next, gen[k].sum, n);
            printf("%-16s| %-9.4g\n", gen[k].name, gen[k].diff);
        }
	}
  }
   	return 0;
}
#endif