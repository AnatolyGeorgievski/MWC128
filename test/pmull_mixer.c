/*! PMULL и RRX-mixer
 * Copyright (C) 2026  Anatoly M. Georgievskii
 */
#include <stdint.h>
/*! \brief Операция умножения без переносов с последующем редуцированием по полиному (x^{64}+1) */
static inline uint64_t p_mul64(uint64_t a, uint64_t p);
/*! \brief Функция скрамблера эквивалентна множеству операций ROTR-XOR 
	\param p - образующий полином задает множество вращений
 */
static uint64_t scrambler64(uint64_t x, uint64_t p) {
    return p_mul64(x, p);
}
/*! \brief Модульная операция возведения в степень */
static uint64_t p_pow64(uint64_t a, uint64_t e){
	uint64_t sqr=a, acc=1;
	while(e!=0){
		if(e&1)
			acc=p_mul64(acc,sqr);
		sqr=p_mul64(sqr,sqr);
		e>>=1;
	}
	return acc;
}

/*! Умножение полиномов, можно предложить реализацию через PMULL и CLMUL - умножение без переносов 
    с редуцированием по полиному $x^n+1$
 */
#if defined(__aarch64__) && defined(__ARM_FEATURE_CRYPTO)
#include <arm_neon.h>
static inline poly64_t p_mul(poly64_t a, poly64_t b, int n)
{
    poly128_t r  =  vmull_p64 ( a,  b);
    poly64_t r0   =  vgetq_lane_p64((poly64x2_t)r, 0);
	return (r0^r0>>n) & (~0uLL>>(64-n));
}
static inline poly64_t p_mul64(poly64_t a, poly64_t b)
{
    poly128_t r  =  vmull_p64 ( a,  b);
    poly64_t r0   =  vgetq_lane_p64((poly64x2_t)r, 0);
    poly64_t r1   =  vgetq_lane_p64((poly64x2_t)r, 1);
	return (r0^r1);
}
#elif defined(__PCLMUL__)
#include <x86intrin.h>
typedef  int64_t v2di __attribute__((__vector_size__(16)));
static inline uint64_t p_mul(uint64_t a, uint64_t b, int n) {
    v2di r = (v2di)_mm_clmulepi64_si128((v2di){a},(v2di){b},0);
    return (r[0]^r[0]>>n) & (~0uLL>>(64-n));
}
static inline uint64_t p_mul64(uint64_t a, uint64_t b) {
    v2di r = (v2di)_mm_clmulepi64_si128((v2di){a},(v2di){b},0);
    return r[0]^r[1];
}
#else
static inline uint32_t p_mul(uint32_t a, uint32_t b, int n) {
    uint64_t r = 0;
    while (b){
        int sh = __builtin_ctz(b);
        r ^= ((uint64_t)a<<sh);
        b ^= (1u<<sh);
    }
    return (r^r>>n) & (~0u>>(32-n));
}
static inline uint64_t p_mul64(uint64_t a, uint64_t b) {
    unsigned __int128 r = 0;
    while (b){
        int sh = __builtin_ctzll(b);
        r ^= ((unsigned __int128)a<<sh);
        b ^= (1uLL<<sh);
    }
    return (r^r>>64);
}
#endif
int main(){
	uint64_t x = 1, e;
	uint64_t q, q_;
	// polynomial 0x7 inv 0xdb6db6db6db6db6d
	q = x ^ ROTL64(x,1)^ROTL64(x,2); 
	// polynomial 0x10000008001 inv 0x11a351cd01b945d1
	q = x ^ ROTR64(x,49)^ROTR64(x,24); // обратная операция для rrx
	// polynomial 0x1042000000 inv 0x5ddcc8bbe8de6245
	q = ROTR64(x,28) ^ ROTR64(x,34)^ROTR64(x,39); // Sum0 SHA-512
	// polynomial 0x4400000800000 inv 0x7095fb5ba83ce527
	q = ROTR64(x,14) ^ ROTR64(x,18)^ROTR64(x,41); // Sum1 SHA-512
	uint64_t q_inv = 0;
	int ex;
	for (ex = 1; ex<64; ex+=2) {
		q_inv = p_pow64(q, ex);
		e = p_mul64(q, q_inv);
		if (e==1) break;
	}
	printf("polynomial 0x%llx inv 0x%llx %x..%s\n", q, q_inv, ex, e==1? "ok":"fail");
	uint64_t x_max = 0xFFFFFu;
	for ( x=0; x<x_max; x++) {
		uint64_t y = scrambler64(x, q);
		uint64_t z = scrambler64(y, q_inv);
		if (z!=x) {
			printf("op fail\n");
			break;
		}
	}
	if(x==x_max) printf("scrambler64 op ..ok\n");
	for (ex = 63; ex>0; ex = (ex>>1)+1) {
		q_ = p_pow64(q_inv, ex);
		e = p_mul64(q_, q_inv);
		if (e==1) break;
	}
	printf("polynomial 0x%llx inv 0x%llx %x..%s\n", q_, q_inv, ex, e==1? "ok":"fail");
	return 0;
}