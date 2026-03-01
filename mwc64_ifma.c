#include <stdint.h>
#include <x86intrin.h>
#define MWC64_A1 0xFFFEB81Bu
// AVX
static inline __m256i MWC64x4 (__m256i v, __m256i d, const __m256i A1) {
    const __m256i m32 = _mm256_set1_epi64x(0xFFFFFFFF);
    v = _mm256_add_epi64(v, _mm256_and_si256(d,m32));
    v = _mm256_add_epi64(_mm256_mul_epu32(v, A1), _mm256_srli_epi64(v, 32));
    v = _mm256_add_epi64(v, _mm256_srli_epi64(d, 32));
    v = _mm256_add_epi64(_mm256_mul_epu32(v, A1), _mm256_srli_epi64(v, 32));
    return v;
}
// AVX512
static inline __m512i MWC64x8 (__m512i v, __m512i d, const __m512i A1) {
    const __m512i m32 = _mm512_set1_epi64(0xFFFFFFFF);
    v = _mm512_xor_epi64(v, d);
//    v = _mm512_add_epi64(v, _mm512_and_si512(d,m32));
    v = _mm512_add_epi64(_mm512_mul_epu32(v, A1), _mm512_srli_epi64(v, 32));
//    v = _mm512_add_epi64(v, _mm512_srli_epi64(d, 32));
//    v = _mm512_add_epi64(_mm512_mul_epu32(v, A1), _mm512_srli_epi64(v, 32));
    return v;
}

// znver4
// IPC:               4.64
// Block RThroughput: 7.8 128/7.8 = 16.4 bytes/cycle 82 @ 5GHz 
__m256i mwc64_round_0(__m256i h1, const uint8_t * data, int count) {
    const __m256i A1 = _mm256_set1_epi64x(MWC64_A1);


    __m256i h2 = _mm256_setzero_si256();
    __m256i h3 = _mm256_setzero_si256();
    __m256i h4 = _mm256_setzero_si256();
//__asm volatile ("# LLVM-MCA-BEGIN mwc_mix");
    do{
        __m256i d1 = _mm256_loadu_si256((const void*)data); data+=32;
        __m256i d2 = _mm256_loadu_si256((const void*)data); data+=32;
        __m256i d3 = _mm256_loadu_si256((const void*)data); data+=32;
        __m256i d4 = _mm256_loadu_si256((const void*)data); data+=32;
        h1 = MWC64x4(h1, d1, A1);
        h2 = MWC64x4(h2, d2, A1);
        h3 = MWC64x4(h3, d3, A1);
        h4 = MWC64x4(h4, d4, A1);
    }while (--count);
//__asm volatile ("# LLVM-MCA-END mwc_mix");
    return (h1 ^ h3) ^ (h2 ^ h4);
}

// IPC:               2.74
// Block RThroughput: 10.0 256/10 
// IPC:               2.74
// Block RThroughput: 8.0 256/8 = 32*5 = 150G/s

__m512i mwc64_round_(__m512i h1, const uint8_t * data, int count) {
    const __m512i A1 = _mm512_set1_epi64(MWC64_A1);


    __m512i h2 = _mm512_setzero_si512();
    __m512i h3 = _mm512_setzero_si512();
    __m512i h4 = _mm512_setzero_si512();
__asm volatile ("# LLVM-MCA-BEGIN mwc_mix512");
    do{
        __m512i d1 = _mm512_loadu_si512((const void*)data); data+=64;
        __m512i d2 = _mm512_loadu_si512((const void*)data); data+=64;
        __m512i d3 = _mm512_loadu_si512((const void*)data); data+=64;
        __m512i d4 = _mm512_loadu_si512((const void*)data); data+=64;
        h1 = MWC64x8(h1, d1, A1);
        h2 = MWC64x8(h2, d2, A1);
        h3 = MWC64x8(h3, d3, A1);
        h4 = MWC64x8(h4, d4, A1);
    }while (--count);
__asm volatile ("# LLVM-MCA-END mwc_mix512");
    return (h1 ^ h3) ^ (h2 ^ h4);
}

typedef uint64_t  uint64x4_t __attribute__((__vector_size__(32)));
typedef uint64_t  uint64x8_t __attribute__((__vector_size__(64)));

//static
uint64x8_t mwc64_round(uint8_t* data, uint64_t count, uint64_t* out){
    uint64x8_t d, h={0};
    do {
        d = *(uint64x8_t*) data; data+=32;
        h+= d & 0xFFFFFFFFu;
        h = (h>>32) + (h & 0xFFFFFFFFu) * MWC64_A1;
        h+= (d>>32);
        h = (h>>32) + (h & 0xFFFFFFFFu) * MWC64_A1;

    } while (--count>0);
    *(uint64x8_t*)out = h;
}