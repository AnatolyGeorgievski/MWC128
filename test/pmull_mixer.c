/*! PMULL и RRX-mixer
 * Copyright (C) 2026  Anatoly M. Georgievskii

## RRX - миксер (Rotation + XOR)

Множественные операции вращения ROR и XOR можно представить, как умножение на матрицу 
аффинного преобразования в поле GF(2).  
Операция ROR порождает матрицу-циркулянт, в которой все строки являются циклическими сдвигами первой строки.  
Матрица полностью определяется первой строкой или соответствующим бинарным полиномом p(x).

Произведение матрицы на вектор эквивалентно полиномиальному умножению с редуцированием:
    y = x · p(x)  mod (x⁶⁴ + 1)
в кольце GF(2)[x] / (x⁶⁴ + 1).  

Это делает класс RRX-миксеров похожим на MUM, но вместо обычного умножения используется 
умножение без переносов (carry-less multiplication, PMULL/CLMUL).

**Обратимость.**  
Обратная матрица также циркулянтная и определяется полиномом q(x) таким, что:
    p(x) · q(x) ≡ 1  (mod x⁶⁴ + 1)
Чтобы найти q(x), достаточно возвести p(x) в степень, пока не будет найден обратный элемент 
(обычно достаточно малого числа итераций).

**Изоморфизм конечных полей**

Все конечные поля одной и той же степени изоморфны.  
В частности, любые два представления поля GF(2ⁿ), построенные по разным неприводимым полиномам степени n, 
изоморфны как поля.  
Это означает, что линейные преобразования (в том числе миксеры), построенные в одном представлении, 
можно «перенести» в другое представление через подходящий изоморфизм.

В контексте циркулянтных преобразований особенно полезна операция _сопряжения_:
    p_new(x) = r(x) · p(x) · r⁻¹(x)  mod (x⁶⁴ + 1)
где r(x) — любой обратимый полином (т.е. gcd(r(x), x⁶⁴ + 1) = 1).  

(https://en.wikipedia.org/wiki/Conjugacy_class)

Такое преобразование сохраняет многие важные свойства миксера (лавинный эффект, диффузию, независимость битов), 
но меняет вес Хэмминга как прямого, так и обратного полинома.

Аналогия: эта формула напоминает сопряжение в группе кватернионов (или матриц вращения) — 
p_new является «тем же самым» преобразованием, но записанным в другом базисе.

Благодаря этому можно:
- начинать с разреженного полинома (дешёвое прямое вычисление),
- подобрать короткий r(x), чтобы сделать вес обратного полинома приемлемым,
- или наоборот — сделать прямое преобразование плотнее, если это улучшает смешивание.

Таким образом, пространство хороших RRX-миксеров значительно шире, чем кажется на первый взгляд: 
один удачный полином порождает целое семейство эквивалентных (в смысле изоморфизма) миксеров.


 */
#include <stdint.h>
#define ROTR64(v, i) 	(((v)<<(64-i)) ^ ((v)>>(i)))
#define ROTL64(v, i) 	(((v)<<(i)) ^ ((v)>>(64-i)))

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
/*! \brief Обратный полином методом возведения в квадрат
	\return обратный полином для a. Если обратный полином не найден возвращает `0`
*/
static uint64_t p_inverse64(uint64_t a){
    uint64_t sqr=a, acc=a;
    for (int i=0; i<6; i++) {
        sqr=p_mul64(sqr,sqr);
        if (sqr==1) return acc;
        acc=p_mul64(acc,sqr);
    }
    return 0;
}
#if 0
void m_transpose_8x8_avx2(uint8_t dst[64], const uint8_t src[64])
{
    __m256i r0 = _mm256_loadu_si256((const __m256i*)(src +  0));
    __m256i r1 = _mm256_loadu_si256((const __m256i*)(src + 32));

    __m256i t0, t1, t2, t3, t4, t5, t6, t7;

    t0 = _mm256_unpacklo_epi8(r0, r1);
    t1 = _mm256_unpackhi_epi8(r0, r1);
    t2 = _mm256_unpacklo_epi16(t0, t1);
    t3 = _mm256_unpackhi_epi16(t0, t1);
    t4 = _mm256_unpacklo_epi32(t2, t3);
    t5 = _mm256_unpackhi_epi32(t2, t3);

    t6 = _mm256_permute2x128_si256(t4, t5, 0x20);
    t7 = _mm256_permute2x128_si256(t4, t5, 0x31);

    t0 = _mm256_unpacklo_epi64(t6, t7);
    t1 = _mm256_unpackhi_epi64(t6, t7);

    _mm256_storeu_si256((__m256i*)(dst +  0), t0);
    _mm256_storeu_si256((__m256i*)(dst + 32), t1);
}
#endif
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

#include <stdio.h>
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

const uint64_t m[] = {
	0x9e3779b97f4a7c15, // golden ratio
	0x60bee2bee120fc15, // wyrand increment
	2685821657736338717u, 
	1181783497276652981u,
	8372773778140471301u,
	0x5851F42D4C957F2D,
	0xd1342543de82ef95,
	0xd605bbb58c8abbfd,
	0xda942042e4dd58b5,// Lehmer64 multiplier
	0xa3b195354a39b70d,// WYhash multiplier
	0xb492b66fbe98f273,//CityHash
	0xC2B2AE3D27D4EB4F
};
	int sz = sizeof(m)/sizeof(uint64_t);
	for (int i=0;i<sz;i++){
		q = m[i];
		int hweight = __builtin_popcountll(q);
		if ((hweight & 1) == 0) continue;
		int count = 0;
		//for (ex = 2; ex<64; ex <<= 1) {
		for (ex = 1; ex<64; ex=((ex+1)<<1)-1) {
			count++;
			q_inv = p_pow64(q, ex);
			e = p_mul64(q, q_inv);
			if (e==1) break;
		}
		int qweight = __builtin_popcountll(q_inv);
		printf("| 0x%016llx | 0x%016llx | %x | %d (%d) | %d ..%s\n", q, q_inv, ex, 
			hweight, qweight, count, e==1? "ok":"fail");
	}


	return 0;
}