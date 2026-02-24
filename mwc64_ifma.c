#include <stdint.h>
#include <x86intrin.h>
#define MWC64_A1 0xFFFEB81Bu
// AVX
static inline __m256i MWC64x4 (__m256i v, __m256i d, const __m256i A1) {
    const __m256i m32 = _mm256_set1_epi64x(0xFFFFFFFF);
    v = _mm256_add_epi64(v, _mm256_and_si256(d,m32));
    v = _mm256_mul_epu32(v, A1) + _mm256_srli_epi64(v, 32);
    v = _mm256_add_epi64(v, _mm256_srli_epi64(d, 32));
    v = _mm256_mul_epu32(v, A1) + _mm256_srli_epi64(v, 32);
    return v;
}

__m256i mwc_round(__m256i h1, const uint8_t * data, int count) {
// znver4
// IPC:               4.64
// Block RThroughput: 7.8 128/7.8 = 16.4 bytes/cycle 82 @ 5GHz 
    const __m256i A1 = _mm256_set1_epi64x(MWC64_A1);


    __m256i h2 = _mm256_setzero_si256();
    __m256i h3 = _mm256_setzero_si256();
    __m256i h4 = _mm256_setzero_si256();
__asm volatile ("# LLVM-MCA-BEGIN mwc_mix");
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
__asm volatile ("# LLVM-MCA-END mwc_mix");
    return (h1 ^ h2) ^ (h3 ^ h4);
}