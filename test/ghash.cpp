/*!
	\see [RFC7714] AES-GCM Authenticated Encryption in the Secure Real-time Transport Protocol (SRTP)
 GHASH
    https://pdfs.semanticscholar.org/114a/4222c53f1a6879f1a77f1bae2fc0f8f55348.pdf
    https://www.intel.com/content/dam/www/public/us/en/documents/software-support/enabling-high-performance-gcm.pdf
    https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/communications-ia-galois-counter-mode-paper.pdf
	https://software.intel.com/sites/default/files/managed/72/cc/clmul-wp-rev-2.02-2014-04-20.pdf

// полином 2^128 + 2^7 + 2^2 + 2^1 + 1 (0x87)
// reflected (A)*reflected (B) =reflected (A*B) >>1
// reflected (A)*reflected (H<<1 mod g(x)) = reflected (A*H) mod g(x)
//
GHASH64
// полином 2^64  + 2^4 + 2^3 + 2^1 + 1 (0x1b)

  Aggr2 
  Yi = [(Xi • H) + (Xi-1+Yi-2) •H2] mod P
  Aggr4 
  Yi = [(Xi • H) + (Xi-1 • H2) + (Xi-2 • H3) + (Xi-3+Yi-4) •H4] mod P


linear folding 

gcc -I. -I:/msys64/mingw64/include/glib-2.0 -O3 -march=native -o ghash ghash.c aes.c cipher.c -lglib-2.0

$ gcc -I. -I:/msys64/mingw64/include/glib-2.0 -O3 -march=icelake-server -o ghash ghash.c aes.c cipher.c -lglib-2.0
Тестирование на симуляторе
$ /sde/sde.exe -icx -- ./ghash.exe
Оптимизация
$ llvm-mca --mcpu=skx -timeline ghash.s

*/
#include <stdint.h>
#include <stdio.h>
#include <x86intrin.h>
// использование карацубы дает выигрыш буквально в один такт.
#define Karatsuba 1

typedef  uint32_t v4si __attribute__((__vector_size__(16)));
typedef  int64_t v2di __attribute__((__vector_size__(16)));
typedef  uint8_t v16qi __attribute__((__vector_size__(16)));
typedef  uint8_t v32qi __attribute__((__vector_size__(32)));
typedef  uint8_t v64qi __attribute__((__vector_size__(64)));
//typedef v4si (*CipherEncrypt128)(void *ctx, v4si src);

static inline
v2di CL_MUL128(v2di x, v2di y, const int c) __attribute__ ((__target__("pclmul")));
/*! Значение с =0x00 (a0*b0) 0x11 (a1*b1)
 */
static inline
v2di CL_MUL128(v2di x, v2di y, const int c)
{
    return (v2di)_mm_clmulepi64_si128 ((__m128i)x,(__m128i)y,c);
}
static inline
v2di SHUFFLE(v2di x)
{
    return __builtin_shuffle (x,(v2di){1,0});
}
static inline
v16qi REVERSE(v16qi x)
{
    return __builtin_shuffle ((v16qi)x,(v16qi){15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0});
}
static inline
__m256i REV128x2(__m256i x)
{// _mm256_shuffle_epi8 AVX2
	const v32qi SWAP =
		{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
		 31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16};
    return (__m256i)__builtin_shuffle ((v32qi)x,SWAP);
}
static inline
v4si LOADU128(const void* src)
{
// defined(__SSE2__) 
	return (v4si)_mm_loadu_si128((const __m128i_u*)src);
}
static inline
void STOREU128(void* dst, v4si x)
{
// defined(__SSE2__) 
	_mm_storeu_si128((__m128i_u*)dst, (__m128i)x);
}

static inline
__m256i LOADU256(const void* src)
{
// defined(__AVX__) 
	return _mm256_loadu_si256((const __m256i_u*)src);
}
#ifdef __AVX512F__
static inline
__m512i LOADU512(const void* src)
{
// defined(__AVX__) 
	return _mm512_loadu_si512(src);
}
static inline
__m512i REV128x4(__m512i x)
{//  AVX512BW
	const v64qi REV_SHUF = 
		{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
		 31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
		 47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
		 63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48};
	return _mm512_shuffle_epi8 (x, (__m512i)REV_SHUF);
}
#endif
static inline
v16qi LOADZU(const uint8_t* src, int len)
{
#if defined(__AVX512VL__) && defined(__AVX512BW__)// AVX512VL + AVX512BW
	__mmask16 mm = ~0;
	return (v16qi)_mm_maskz_loadu_epi8 (mm>>(-len & 0xF), src);
#else
	v16qi x;
	x^=x;
	__builtin_memcpy((uint8_t*)&x, src, len&0xF);
	return x;
#endif
}
static inline
void STOREU(uint8_t* dst, v16qi x, int len)
{
#if defined(__AVX512VL__) && defined(__AVX512BW__)// AVX512VL + AVX512BW
	__mmask16 mm = ~0;
	_mm_mask_storeu_epi8 (dst, mm>>(-len & 0xF),(__m128i) x);
#else
	__builtin_memcpy(dst, (uint8_t*)&x, len&0xF);
#endif
}
/*! перед использованием один из аргументов следует сдвинуть влево SLM 
	Редуцирование можно вынести из цикла.
 */
typedef uint64_t poly64x2_t __attribute__((__vector_size__(16)));
/*! \brief этот алгоритм короче чем в референсном варианте */
static
v4si gmul128(v4si a, const v4si b)
{
	const __m128i  poly = _mm_setr_epi32(0x1,0,0,0xc2000000);//{1,0xc2ULL<<56};
    __m128i  M,L,H;
#if (Karatsuba==1) // карацуба
	L = _mm_clmulepi64_si128((__m128i)a, (__m128i)b, 0x00);
	H = _mm_clmulepi64_si128((__m128i)a, (__m128i)b, 0x11);
	v4si t;
	t = (v4si){a[0],a[1],b[0],b[1]} ^ (v4si){a[2],a[3], b[2],b[3]};
	M = _mm_clmulepi64_si128((__m128i)t, (__m128i)t, 0x01) ^ L ^ H;
#else
    L = _mm_clmulepi64_si128((__m128i)a, (__m128i)b, 0x00);
    M = _mm_clmulepi64_si128((__m128i)a, (__m128i)b, 0x01);
    H = _mm_clmulepi64_si128((__m128i)a, (__m128i)b, 0x11);
    M^= _mm_clmulepi64_si128((__m128i)a, (__m128i)b, 0x10);
#endif
// редуцирование по модулю, работает!
	M^= _mm_shuffle_epi32(L, (_MM_PERM_ENUM)78);//(poly64x2_t){L[1],L[0]};//SHUFFLE(L);
    M^= _mm_clmulepi64_si128(L, poly, 0x10);
// редуцирование по модулю, работает! это можно использовать как отдельную функцию
	H^= _mm_shuffle_epi32(M, (_MM_PERM_ENUM)78);//(poly64x2_t){M[1],M[0]};//SHUFFLE(M);
    H^= _mm_clmulepi64_si128(M, poly, 0x10);
    return (v4si)H;
}
static 
v4si gmul128x4(__m512i a,  const __m512i b) __attribute__((__target__("avx512f","vpclmulqdq")));
static 
v4si gmul128x4(__m512i a,  const __m512i b)
{
	const __m512i  poly = _mm512_setr_epi32(0x1,0,0,0xc2000000, 0x1,0,0,0xc2000000,0x1, 0,0,0xc2000000, 0x1,0,0,0xc2000000);
    __m512i  M,L,H;
//__asm volatile("# LLVM-MCA-BEGIN gmul128x4");
#if (Karatsuba==1) // карацуба быстрее
	L = _mm512_clmulepi64_epi128(a, b, 0x00);
	H = _mm512_clmulepi64_epi128(a, b, 0x11);
	__m512i t;// не проверял -- работает медленно
	t = (__m512i)_mm512_shuffle_ps((__m512)a, (__m512)b, 68) 
	  ^ (__m512i)_mm512_shuffle_ps((__m512)a, (__m512)b, 238);
	M = _mm512_clmulepi64_epi128((__m512i)t, (__m512i)t, 0x01);
	M^=  L ^ H;// _mm512_ternarylogic_epi64(M, H, L, 0x96);//
#else
    L = _mm512_clmulepi64_epi128(a, b, 0x00);
    M = _mm512_clmulepi64_epi128(a, b, 0x01);
    H = _mm512_clmulepi64_epi128(a, b, 0x11);
    M^= _mm512_clmulepi64_epi128(a, b, 0x10);
#endif
// редуцирование по модулю, работает!
	__m512i m1 = _mm512_shuffle_epi32(L, (_MM_PERM_ENUM)78);//(poly64x2_t){L[1],L[0]};//SHUFFLE(L);
    __m512i m2 = _mm512_clmulepi64_epi128(L, poly, 0x10);
	M = _mm512_ternarylogic_epi64(M, m1, m2, 0x96);// M^m1^m2
//	M^= m1 ^ m2;
// редуцирование по модулю, работает!
	m1 = _mm512_shuffle_epi32(M, (_MM_PERM_ENUM)78);//(poly64x2_t){M[1],M[0]};//SHUFFLE(M);
    m2 = _mm512_clmulepi64_epi128(M, poly, 0x10);
	H = _mm512_ternarylogic_epi64(H, m1, m2, 0x96);// H^m1^m2
//__asm volatile("# LLVM-MCA-END gmul128x4");
	__m256i h = _mm512_castsi512_si256(H) ^ _mm512_extracti32x8_epi32(H, 1);
	return (v4si)(_mm256_castsi256_si128(h) ^ _mm256_extracti32x4_epi32(h, 1));
}

static 
v4si gmul128x2(__m256i a,  const v4si h1, const v4si h2) __attribute__((__target__("avx512vl","vpclmulqdq")));
static 
v4si gmul128x2(__m256i a,  const v4si h1, const v4si h2)
{
	__m256i b = _mm256_setr_m128i((__m128i)h2,(__m128i)h1);
	const __m256i  poly = _mm256_setr_epi32(0x1,0,0,0xc2000000,0x1,0,0,0xc2000000);
    __m256i  M,L,H;
//__asm volatile("# LLVM-MCA-BEGIN gmul128x2");

    L = _mm256_clmulepi64_epi128(a, b, 0x00);
    M = _mm256_clmulepi64_epi128(a, b, 0x01);
    H = _mm256_clmulepi64_epi128(a, b, 0x11);
    M^= _mm256_clmulepi64_epi128(a, b, 0x10);
// редуцирование по модулю, работает!
	M^= _mm256_shuffle_epi32(L, (_MM_PERM_ENUM)78);//(poly64x2_t){L[1],L[0]};//SHUFFLE(L);
    M^= _mm256_clmulepi64_epi128(L, poly, 0x10);
// редуцирование по модулю, работает!
	H^= _mm256_shuffle_epi32(M,(_MM_PERM_ENUM) 78);//(poly64x2_t){M[1],M[0]};//SHUFFLE(M);
    H^= _mm256_clmulepi64_epi128(M, poly, 0x10);
//__asm volatile("# LLVM-MCA-END gmul128x2");
	return (v4si)(_mm256_castsi256_si128(H) ^ _mm256_extractf128_si256(H, 1));
}
static
v4si gmul128_aggr2(v4si x0, v4si x1, const v4si h, const v4si h2)
{
	const __m128i  poly = _mm_setr_epi32(0x1,0,0,0xc2000000);//{1,0xc2ULL<<56};
    __m128i  L,M,H;
//__asm volatile("# LLVM-MCA-BEGIN gmul128_aggr2");
    L = _mm_clmulepi64_si128((__m128i)x1, (__m128i)h, 0x00);
    M = _mm_clmulepi64_si128((__m128i)x1, (__m128i)h, 0x01);
    H = _mm_clmulepi64_si128((__m128i)x1, (__m128i)h, 0x11);
    M^= _mm_clmulepi64_si128((__m128i)x1, (__m128i)h, 0x10);

    L^= _mm_clmulepi64_si128((__m128i)x0, (__m128i)h2, 0x00);
    M^= _mm_clmulepi64_si128((__m128i)x0, (__m128i)h2, 0x01);
    H^= _mm_clmulepi64_si128((__m128i)x0, (__m128i)h2, 0x11);
    M^= _mm_clmulepi64_si128((__m128i)x0, (__m128i)h2, 0x10);

// редуцирование по модулю, работает!
	M^= _mm_shuffle_epi32(L, (_MM_PERM_ENUM)78);//(poly64x2_t){L[1],L[0]};//SHUFFLE(L);
    M^= _mm_clmulepi64_si128(L, poly, 0x10);
// редуцирование по модулю, работает! это можно использовать как отдельную функцию
	H^= _mm_shuffle_epi32(M, (_MM_PERM_ENUM)78);//(poly64x2_t){M[1],M[0]};//SHUFFLE(M);
    H^= _mm_clmulepi64_si128(M, poly, 0x10);
//__asm volatile("# LLVM-MCA-END gmul128_aggr2");
    return (v4si)H;
}
static 
v4si gmul128_aggr4(v4si x0, v4si x1, v4si x2, v4si x3, const v4si h, const v4si h2, const v4si h3, const v4si h4)
{
	const __m128i  poly = _mm_setr_epi32(0x1,0,0,0xc2000000);//{1,0xc2ULL<<56};
    __m128i  L,M,H;
//__asm volatile("# LLVM-MCA-BEGIN gmul128_aggr4");
    L = _mm_clmulepi64_si128((__m128i)x3, (__m128i)h, 0x00);
    M = _mm_clmulepi64_si128((__m128i)x3, (__m128i)h, 0x01);
    H = _mm_clmulepi64_si128((__m128i)x3, (__m128i)h, 0x11);
    M^= _mm_clmulepi64_si128((__m128i)x3, (__m128i)h, 0x10);

    L^= _mm_clmulepi64_si128((__m128i)x2, (__m128i)h2, 0x00);
    M^= _mm_clmulepi64_si128((__m128i)x2, (__m128i)h2, 0x01);
    H^= _mm_clmulepi64_si128((__m128i)x2, (__m128i)h2, 0x11);
    M^= _mm_clmulepi64_si128((__m128i)x2, (__m128i)h2, 0x10);
	
    L^= _mm_clmulepi64_si128((__m128i)x1, (__m128i)h3, 0x00);
    M^= _mm_clmulepi64_si128((__m128i)x1, (__m128i)h3, 0x01);
    H^= _mm_clmulepi64_si128((__m128i)x1, (__m128i)h3, 0x11);
    M^= _mm_clmulepi64_si128((__m128i)x1, (__m128i)h3, 0x10);
	
    L^= _mm_clmulepi64_si128((__m128i)x0, (__m128i)h4, 0x00);
    M^= _mm_clmulepi64_si128((__m128i)x0, (__m128i)h4, 0x01);
    H^= _mm_clmulepi64_si128((__m128i)x0, (__m128i)h4, 0x11);
    M^= _mm_clmulepi64_si128((__m128i)x0, (__m128i)h4, 0x10);

// редуцирование по модулю, работает!
	M^= _mm_shuffle_epi32(L, (_MM_PERM_ENUM)78);//(poly64x2_t){L[1],L[0]};//SHUFFLE(L);
    M^= _mm_clmulepi64_si128(L, poly, 0x10);
// редуцирование по модулю, работает! это можно использовать как отдельную функцию
	H^= _mm_shuffle_epi32(M, (_MM_PERM_ENUM)78);//(poly64x2_t){M[1],M[0]};//SHUFFLE(M);
    H^= _mm_clmulepi64_si128(M, poly, 0x10);
//__asm volatile("# LLVM-MCA-END gmul128_aggr4");
    return (v4si)H;
}
/*! \brief возведение в квадрат 
	Сложность 4М (четыре операции умножения)
 */
static 
v4si gsqr128(const v4si b)
{
	const __m128i poly = _mm_setr_epi32(0x1,0,0,0xc2000000);
    __m128i L,H;
    L = _mm_clmulepi64_si128((__m128i)b, (__m128i)b, 0x00);
    H = _mm_clmulepi64_si128((__m128i)b, (__m128i)b, 0x11);//(a*x + b)^2= a^2*x^2 + b^2
	L = _mm_shuffle_epi32(L, (_MM_PERM_ENUM)78)
      ^ _mm_clmulepi64_si128(L, poly, 0x10);
	L = _mm_shuffle_epi32(L, (_MM_PERM_ENUM)78)
      ^ _mm_clmulepi64_si128(L, poly, 0x10);
    return (v4si)(H^L);
}
/* сдвиг влево на один разряд */
static 
v4si SLM128(v4si d)
{
#if 1
	__m128i r;// = _mm_alignr_epi8 ((__m128i)d,(__m128i)d, 12);
	r = _mm_slli_epi32((__m128i)d, 1);
	r^= _mm_srli_epi32(_mm_alignr_epi8 ((__m128i)d,(__m128i)d, 12), 31);
	r^= _mm_srai_epi32((__m128i)d, 31) & _mm_setr_epi32(0,0,0,0xc2000000);// сдвиг арифметический
    return  (v4si)(r);
#else
    v4si r = (v4si){d[3],d[0],d[1],d[2]};
	r >>=31;
    if (r[0]!=0) r[3] ^= 0xc2000000;
	return (d<<1) ^ r;
#endif
}

#if 0
//static inline 
v2di GMUL128(v2di a, const v2di b)
{
	const v2di Px = {0xc2ULL<<56};
    v2di M,L,H;
    L = CL_MUL128(a, b, 0x00);
    M = CL_MUL128(a, b, 0x01);
    M^= CL_MUL128(a, b, 0x10);
    H = CL_MUL128(a, b, 0x11);
// редуцирование по модулю, работает!
	M^= SHUFFLE(L);
    M^= CL_MUL128(L, Px, 0x00);
// редуцирование по модулю, работает! это можно использовать как отдельную функцию
	H^= SHUFFLE(M);
    H^= CL_MUL128(M, Px, 0x00);
    return H;// ^ SHUFFLE(M ^ L);
}

/*! An alternative technique trades-off one multiplication for additional XOR operations. It
can be viewed as “one iteration carry-less Karatsuba” multiplication [7, 9].
Algorithm 2
Step 1: multiply carry-less the following operands: A1 with B1, A0 with B0, and A0 + A1
with B0 + B1. Let the results of the above three multiplications be: [C1:C0], [D1:D0] and
[E1:E0], respectively.
Step 2: construct the 256-bit output of the multiplication [A1:A0] * [B1:B0] as follows:
[A1 : A0]*[B1 : B0] * [C1 : C0+C1 + D1 + E1 : D1 + C0 + D0 + E0 : D0] (6)
*/

// умножение и редуцирование по полиному, только порядок бит вывернут
// x^128 + x^7 + x^2 + x^1 + 1
v4si SRM128(v4si d)
{
    v4si r = (v4si){d[1],d[2],d[3],d[0]};
    r <<=31;
    r = (r!=0) & (v4si){1<<31,1<<31,1<<31,0xe1000000};
    return  (d>>1) ^ r;
}
v4si SLM128(v4si d)
{
    v4si r = (v4si){d[3],d[0],d[1],d[2]};
    r >>=31;
    if (r[0]!=0) r[3] ^= 0xc2000000;
    //r = (r!=0) & (v4si){1,1,1,0xc2000000};
    return  (d<<1) ^ r;
}

void SRM128_(uint32_t* d) {
    const uint32_t r0 = d[0];
    const uint32_t r1 = d[1];
    const uint32_t r2 = d[2];
    const uint32_t r3 = d[3];
    d[0] = (r0>>1) | (r1<<31);
    d[1] = (r1>>1) | (r2<<31);
    d[2] = (r2>>1) | (r3<<31);
	if (r0&1) d[3] = (r3>>1) ^ 0xe1000000;
	else d[3] = (r3>>1);
}
uint64_t SRM64_(uint64_t d) {
	if (d&1) d = (d>>1) ^ 0xd800000000000000ULL;
	else d = (d>>1);
	return d;
}
/*! сдвиг вправо */
// GHASH 1110 0001 || 0^120
v4si MUL128(v4si y, uint32_t *H)
{
    v4si z = {0};
    int i;
    for (i=0; i<4; i++)
    {
        uint32_t xi = H[3-i];
        int j;
        for (j=0; j<32; j++)
        {
            if (xi>>31) z ^= y;
			//SRM128_((void*)&y); // сдвиг и редуцирование по модулю
			y = SRM128(y); // сдвиг и редуцирование по модулю
			xi<<=1;
        }// while (xi<<=1);
    }
    return z;
}
#endif
/*! \brief Функция хеши основана на умножении в поле GF(2^128)
 */
void GHASH_(const void *in, uint64_t len, uint64_t seed, void *out)
{
	const uint8_t *data = (const uint8_t *)in;
    int i;
	v4si y = {(uint32_t)seed, (uint32_t)(seed>>32)};
	v4si h = {0};
//    h = SLM128(h);
	int blocks;
#if defined(__VPCLMULQDQ__)
	v4si h2 = gsqr128(h);// 4М -- эту операцию можно выполнить снаружи
	blocks = len>>6;
	if (blocks){
		// todo умножение {h3, h4} = {h, h2} * {h2,h2} можно выполнить параллельно на векторе 256 бит
		v4si h3 = gmul128(h2, h);
		v4si h4 = gsqr128(h2);
		__m512i b = _mm512_zextsi128_si512((__m128i)h4);
		b = _mm512_inserti32x4(b, (__m128i)h, 3);
		b = _mm512_inserti32x4(b, (__m128i)h2,2);
		b = _mm512_inserti32x4(b, (__m128i)h3,1);
//__asm volatile("# LLVM-MCA-BEGIN GHASH_");
		for (i=0; i<blocks; i++){
			__m512i x1, y1;
			y1 = _mm512_zextsi128_si512((__m128i)y);
			x1 = LOADU512(data); data+=64;
			x1 = REV128x4(x1);
			y = gmul128x4((y1^x1), b);
		}
//__asm volatile("# LLVM-MCA-END GHASH_");
	}
	if(len&32){
		__m256i x1, y1;
		y1 = _mm256_zextsi128_si256((__m128i)y);
        x1 = LOADU256(data); data+=32;
		x1 = REV128x2(x1);
		y = gmul128x2((y1^x1), h, h2);
    }
	if(len&16){
		v4si x;
        x = LOADU128(data); data+=16;
		x = (v4si)REVERSE((v16qi)x);
        y = gmul128((y^x), h);
    }
#else
	blocks = len>>4;
//__asm volatile("# LLVM-MCA-BEGIN GHASH_");
	for (i=0; i<blocks; i++){
		v4si x;
		x = LOADU128(data); data+=16;
		x = (v4si)REVERSE((v16qi)x);
		y = gmul128((y^x), h);
//			y = gmul128_aggr2((y^x0), x1, h, h2);
	}
//__asm volatile("# LLVM-MCA-END GHASH_");
#endif
	if (len &0xF){
		v4si x;
		x = (v4si)LOADZU(data, len);
		x = (v4si)REVERSE((v16qi)x);
		y = gmul128((y^x), h);
	}
	__builtin_memcpy(out, &y, 16);
}

#include "Platform.h"
#include "Hashlib.h"
#include "Mathmult.h"

REGISTER_FAMILY(GHASH,
   $.src_url    = "https://github.com/AnatolyGeorgievski/MWC128/",
   $.src_status = HashFamilyInfo::SRC_ACTIVE
 );

REGISTER_HASH(GHASH_128,
   $.desc       = "128-bit GHash",
   $.hash_flags =
        0,
   $.impl_flags =
        FLAG_IMPL_MULTIPLY   |
        FLAG_IMPL_LICENSE_PUBLIC_DOMAIN,
   $.bits = 128,
   $.verification_LE = 0x3EBAAF43,
   $.verification_BE = 0x4B032B63,
   $.hashfn_native   = GHASH_,
   $.hashfn_bswap    = GHASH_
 );
