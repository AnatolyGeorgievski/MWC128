#include <math.h>
#include <stdint.h>
#include "mwc.h"

#define M 32
#include <x86intrin.h>
// посчитать число бит в каждой позиции
static inline uint8_t* HistogramBits( uint64_t x, int N_bits, uint8_t * v ) {
// использовать инструкции AVX2 и AVX512 - с маской __mmask32
#if defined(__AVX2__)
    const __m256i ONE  = _mm256_set1_epi8(1);
    const __m256i MASK = _mm256_set1_epi64x(0x8040201008040201u);
	for (int i=0; i<2; i++){
        uint32_t word = x>>(i*32);
        __m256i base  = _mm256_set1_epi32(word);
		__m256i cnt = _mm256_loadu_si256((const __m256i *)v);
        __m256i inc = _mm256_min_epu8(_mm256_and_si256(base, MASK), ONE);
		        cnt = _mm256_add_epi8(cnt, inc);
		_mm256_storeu_si256((__m256i_u *)v, cnt); v += 32;
    }
#else
	for (int i=0; i<64; i++){
		*v++ += (x>>i) & 1;
	}
#endif
	return v;
}
/*! \brief Bit Independent Criteria Test 

 */
double bic_test(uint64_t (*next)(void*), uint64_t* state, uint64_t *sum, int expN) {
    uint64_t count = 1uLL<<expN; 	// число отсчетов в тесте
	uint32_t  bin [M]; // bit independence
	uint8_t   bit8[M]; // аккумулятор, не более 255 циклов
	__builtin_bzero(bin, M*sizeof(uint32_t));
	__builtin_bzero(bit8, M);
	for (uint64_t k = 0; k< count; k++) {
		uint64_t x = next(state);
		HistogramBits(x, M, bit8);
		if((k&127) == 127) {
			for(int i=0; i<M; i++)	bin[i] += bit8[i]; 
			__builtin_bzero(bit8, M);
		} 
	}
	if (sum)
		for (int i=0; i<M; i++)	sum[i] += bin[i];
	double ks = 0;// ks statistics
	for (int i=0; i<M; i++)	{
		double d = __builtin_ldexp((uint64_t)bin[i] - (count/2), -expN/2);
		ks = fmax(fabs(d),ks);
	}
	return ks;
}

#include <stdio.h>
double dif_test(uint64_t (*next)(void*), uint64_t* state, uint64_t *sum, int Nr) {
	#define DIM 3
	const int m = 0;	  // группировка значений по разрядам 0:1, 1:1.5=3/2, 2:1.875= 15/8; 3:255/128, m: (2^2^m-1)/2^{2^m-1}
	double r = 1;
	long double diffi = 0; 		// суммарная сложность
	const int expN = 32;
    uint64_t count = 1uLL<<expN; 	// число отсчетов в тесте
	double hist[M] = {0}; 		// распределение сложности по категориям
	uint32_t  v    [M] = {0}; // clz частоты попадания в каждую категорию
	uint32_t  v1   [M] = {0}; // ctz
	uint32_t  bit  [64]= {0}; // bit independence
	uint8_t bit8[64] = {0};
    for (uint64_t k = 0; k< count; k++) {
		uint64_t x = next(state);
		double d = difficulty(x);
		diffi += d;
		uint32_t x0 = x;
		uint32_t x1 = x;
		//if (k%DIM == 1) // фильтр 1/3 1/2 - опция
		if (1) {// распределение по числу нулевых бит
			int i = x0? __builtin_clz(x0): 31;
			hist[(i>>m)] += d; // 
			v   [(i>>m)] ++; // частоты по битовой сложности
		}
		if (1) {// распределение по числу нулевых бит
			int i = x1? __builtin_ctz(x1): 31;
			v1   [(i>>m)] ++;   // частоты по битовой сложности
		}
		if (1) {
			HistogramBits(x, 64, bit8);
			if((k&127) == 127) {
				for(int i=0; i<64; i++)	bit[i] += bit8[i]; 
				__builtin_bzero(bit8, 64);
			} 
		}
	}
	if (sum) {// суммирование по категориям
		for(int i=0;i<M; i++) {
			sum[i  ] += v [i]; 	// clz
			sum[i+M] += v1[i];	// ctz
		}
	}
	if (1)  { // вывод подробного отчета по категориям
		printf ("##:  difficulty | freq. clz | freq. ctz | freq.bicL | freq.bicH | hashrate | hrate.tz | avg.hrate |\n");
		for(int i=0;i<M>>m; i++)
			if (v[i]!=0) {
				//double P =(double)1.0/(1uLL<<(31 -(i<<m)));
				int ex = -(31 -(i<<m));
				double hr, hrt, ar = 0, ar1 = 0;
				hr = __builtin_ldexp(  v[i],ex);
				hrt = __builtin_ldexp( v1[i],ex);
				double r1 = (M>>m)-1 == i?2:r;
				int ok;
				if (sum) {
					ar = __builtin_ldexp(sum[i],ex)/(Nr+1);
					ar1= __builtin_ldexp(sum[i+M],ex)/(Nr+1);
					// KS statistics max(D+,D-)\sqrt(n) <= eps
					ok = fabs(ar - r1)* sqrt(__builtin_ldexp((Nr+1),-ex)) <= r1;
					ok&= fabs(ar1 - r1)* sqrt(__builtin_ldexp((Nr+1),-ex)) <= r1;
				} else
					ok = fabs(hr - r1)<= r1* sqrt(__builtin_ldexp(  1,ex));
				// bit independence criteria KS для 2^{32}
				double bic = __builtin_ldexp((double)bit[i] - (count/2), -expN/2);//(count/2);
				double bic1 = __builtin_ldexp((double)bit[M+i] - (count/2), -expN/2);//(count/2);
				printf ("%2d: %12.3f| %-10u| %-10u| %+9.6f | %+9.6f | %8.6f | %8.6f | %9.7f |%s\n", i, 
					hist[i], v[i], v1[i], bic, bic1, hr, hrt, ar, ok?"":" fail");
			}
	}
	return diffi; // суммарная сложность
}
