/*!  \brief xxh64 - Extremely Fast Hash algorithm - vector implementation
    Copyright (C) 2021-2025 Anatoly М. Georgievskii 

    \see https://github.com/AnatolyGeorgievski/LZJB


    gcc -O3 -march=native -S -o test.s xxh64.c
    kernel optimized to AVX2 256bit vector instructions:
```asm
    vmovdqu8      (%rax), %ymm0
    vpmullq %ymm0, %ymm3, %ymm0
    vpaddq  %ymm1, %ymm0, %ymm0
    vprolq  $31,   %ymm0, %ymm0
    vpmullq %ymm2, %ymm0, %ymm1
```

    \see  xxHash - Extremely Fast Hash algorithm
 *   - xxHash homepage  : https://www.xxhash.com
 *   - xxHash repository: https://github.com/Cyan4973/xxHash
 *   - Copyright (C) 2012-2023 Yann Collet

    \sa https://datatracker.ietf.org/doc/html/rfc4418
*/
#include <stdint.h>
#include <stddef.h>
static const uint64_t Prime1 = 0x9E3779B185EBCA87ULL;
static const uint64_t Prime2 = 0xC2B2AE3D27D4EB4FULL;
static const uint64_t Prime3 = 0x165667B19E3779F9ULL;
static const uint64_t Prime4 = 0x85EBCA77C2B2AE63ULL;
static const uint64_t Prime5 = 0x27D4EB2F165667C5ULL;

typedef uint64_t uint64x4_t __attribute__((__vector_size__(32)));

typedef struct _HashCtx HashCtx_t;
struct _HashCtx {
    uint64_t state[4];
    uint8_t  block[32];
    int offset;
};

#define ROTL64(x, n) ((x)<<n | (x)>>(64-n))
static inline uint64_t ROUND64(uint64_t x){
    return ROTL64(x*Prime2,31)*Prime1;
}
static inline uint64_t MERGE64(uint64_t hash, uint64_t x){
    return (hash ^ ROUND64(x))*Prime1 + Prime4;
}
uint64_t xxh64(uint64_t hash, uint8_t* data, size_t data_len)
{
    if (data_len>=32){
        uint64x4_t state = (uint64x4_t){Prime1 + Prime2, Prime2, 0, -Prime1};
        state+=hash;
        int blocks = data_len>>5;
        int  i;
        for (i=0; i<blocks; i++) {// вектор 256 бит
            uint64x4_t block;
            __builtin_memcpy(&block, data, 32); data+=32;
            state = ROTL64(state + block*Prime2, 31) * Prime1;
        }
        hash  = ROTL64(state[0],  1) +
                ROTL64(state[1],  7) +
                ROTL64(state[2], 12) +
                ROTL64(state[3], 18);
        hash  = MERGE64(hash, state[0]);
        hash  = MERGE64(hash, state[1]);
        hash  = MERGE64(hash, state[2]);
        hash  = MERGE64(hash, state[3]);
    } else {
        hash = hash + Prime5;
    }
    hash += data_len;
	// finalize()
    data_len &= 0x1F;
    int i;
    for (i=0; i < data_len>>3; i++, data+=8)
        hash = ROTL64(hash ^ ROUND64(*(uint64_t*)data), 27) * Prime1 + Prime4;
    for (i*=2; i < data_len>>2; i++, data+=4)
        hash = ROTL64(hash ^ *(uint32_t*)data * Prime1, 23) * Prime2 + Prime3;
    for (i*=4; i < data_len; i++, data++)
        hash = ROTL64(hash ^ *data * Prime5, 11) * Prime1;
	// avalanche(hash)
    hash ^= hash >> 33;
    hash *= Prime2;
    hash ^= hash >> 29;
    hash *= Prime3;
    hash ^= hash >> 32;
    return hash;
}

#define MWC_A0 0xfffeb81bULL
uint64_t mwc64_hash(uint64_t hash, uint8_t* data, size_t data_len){
    const uint64_t P = (MWC_A0<<32)-1;
    int i;
    for (i=0; i<data_len>>2; i++){
        hash+= *(uint32_t*) data; data+=4;
        //hash = ROTL64(hash, 32) - ((uint32_t)hash)*((1uLL<<32)-MWC_A0);
        hash = ((uint32_t)hash)*MWC_A0 + (hash>>32);
    }
    if (data_len&2){
        hash+= *(uint16_t*) data; data+=2;
        hash = ((uint16_t)hash)*(MWC_A0<<16) + (hash>>16);
    }
    if (data_len&1){
        hash+= *(uint8_t*) data; data+=1;
        hash = ((uint8_t)hash)*(MWC_A0<<24) + (hash>>8);
    }
    return hash;
}
#ifdef TEST_MWC64
uint64_t mwc64_hash_8(uint64_t hash, uint8_t* data, size_t data_len){
    for (int i=0; i<data_len; i++){
        hash+= *(uint8_t*) data; data+=1;
        hash = ((uint8_t)hash)*(MWC_A0<<24) + (hash>>8);
    }
    return hash;
}
typedef unsigned int __attribute__((mode(TI)))   uint128_t;
#include <stdio.h>
static inline uint64_t mwc64_next(uint64_t hash){
    return ((uint32_t)hash)*MWC_A0 + (hash>>32);
}
static inline float mwc64_to_float(uint64_t seed){
    union { float f; uint32_t i } rng;
    rng.i = 0x3f800000 | ((uint32_t)seed >> 9);
    return rng.f - 1.0f;
}
static inline uint64_t _shoup_const(uint64_t b, uint64_t P){
	return (((((unsigned __int128)b)<<32)+MWC_A0/2)<<32)/P ;
}
static inline uint64_t _mulm_shoup(uint64_t a, uint64_t b, uint64_t w, uint64_t P){
	union {
		unsigned __int128 t;
		uint64_t r[2];
	} v;
	uint64_t q = ((a*(unsigned __int128)w)>>64);
	v.t = a*(unsigned __int128)b - q*(unsigned __int128)P;
//	if (v.r[1]>0) printf ("O1\n");
//	if (v.r[1]) printf ("Ovf\n");
//	if (v.r[1]) v.r[0] -=P; -- если переполнение
//		printf ("Ovf\n");
//		if (v.r[0]>=P) printf ("Mod\n");
	return (v.r[1] || v.r[0]>=P)? v.r[0]-P: v.r[0];
}
static inline uint64_t _mulm(uint64_t a, uint64_t b, uint64_t P){
	return (unsigned __int128)a * b %P;
}
int main() {
    uint8_t str[] = "1234567890abcdef";
    uint64_t h1 = mwc64_hash(0, str, 11);
    uint64_t h8 = mwc64_hash_8(0, str, 11);
    if (h1 == h8) printf("MWC64 ..ok\n");

	const uint64_t P = (MWC_A0<<32)-1;
	uint64_t s = MWC_A0,s1,s2;
// расчет сдвиговой константы A^{2^k}
    uint64_t K[16+1];
    K[0] = MWC_A0;
	for (int i=0; i<16; i++) {
		s = _mulm(s, s, P);
        s1 = _mulm(s, MWC_A0, P);
        s2 = _mulm(s1, MWC_A0, P);
		printf("[%2d] = 0x%016llx, 0x%016llx, 0x%016llx, \n", i+1, s, s1, s2);
        K[i+1] = s;
	}
    printf("test\n"); // проверка сдвиговых констант
    
    uint64_t seed = 31;
    uint64_t h = seed;
    for (int i=0, k=0; i<256; i++) {
        h = mwc64_next(h);
        if (i+1 == (1u<<k)) {
			uint64_t w = _shoup_const(K[k], P);
            printf("[%2d] = 0x%016llx ..%s\n", k, h, (h == _mulm_shoup(seed,K[k],w,P))? "ok":"fail");
            k++;
        }
    }
	uint64_t w = _shoup_const(K[8], P);
	for (uint64_t x=P; x>1uLL<<32; --x){
		if (_mulm(x,K[8], P)!=_mulm_shoup(x, K[8], w, P)) {
			printf("%08llx ..fail\n", x);
		}
	}
	return 0;
}
#endif