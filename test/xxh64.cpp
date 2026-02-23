/*! xxh64-hash -- быстрая _portable_ реализация
 * Copyright (C) 2026  Anatoly M. Georgievskii

    Оригинальный алгоритм Yann Collet

 * xxHash - Extremely Fast Hash algorithm
 * Copyright (C) 2012-2023 Yann Collet

 */
#include <stdint.h>

#include "Platform.h"
#include "Hashlib.h"
#include "Mathmult.h"

static const uint64_t Prime1 = UINT64_C(0x9E3779B185EBCA87);
static const uint64_t Prime2 = UINT64_C(0xC2B2AE3D27D4EB4F);
static const uint64_t Prime3 = UINT64_C(0x165667B19E3779F9);
static const uint64_t Prime4 = UINT64_C(0x85EBCA77C2B2AE63);
static const uint64_t Prime5 = UINT64_C(0x27D4EB2F165667C5);

static FORCE_INLINE uint64_t ROUND64(uint64_t acc, uint64_t x){
    return ROTL64(acc + x*Prime2,31)*Prime1;
}
static FORCE_INLINE uint64_t MERGE64(uint64_t hash, uint64_t x){
    return (hash ^ ROUND64(0,x))*Prime1 + Prime4;
}
// Миксер avalanche от xxHash64
static FORCE_INLINE uint64_t avalanche(uint64_t hash){
    hash ^= hash >> 33;
    hash *= Prime2;
    hash ^= hash >> 29;
    hash *= Prime3;
    hash ^= hash >> 32;
    return hash;
}
template <bool bswap>
void xxh64_hash(const void* in, uint64_t data_len, uint64_t seed0, void * out)
{
    const uint8_t* data = (const uint8_t*)in;
	uint64_t hash = seed0; 
    if (data_len>=32){
        uint64_t state[4] = {Prime1 + Prime2, Prime2, 0, -Prime1};
        state[0] +=hash;
        state[1] +=hash;
        state[2] +=hash;
        state[3] +=hash;
        uint64_t blocks = data_len>>5;
        uint64_t i;
		for (i=0; i<blocks; i++, data+=32) {// вектор 256 бит
            state[0] = ROUND64(state[0], GET_U64<bswap>(data,  0));
            state[1] = ROUND64(state[1], GET_U64<bswap>(data,  8));
            state[2] = ROUND64(state[2], GET_U64<bswap>(data, 16));
            state[3] = ROUND64(state[3], GET_U64<bswap>(data, 24));
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
    uint64_t i;
    for (i=0; i < data_len>>3; i++, data+=8){
        hash = ROTL64(hash ^ ROUND64(0,GET_U64<bswap>(data, 0)), 27) * Prime1 + Prime4;
    }
    for (i*=2; i < data_len>>2; i++, data+=4) {
        hash = ROTL64(hash ^ GET_U32<bswap>(data, 0) * Prime1, 23) * Prime2 + Prime3;
    }
    for (i*=4; i < data_len; i++, data++) {
        hash = ROTL64(hash ^ *data * Prime5, 11) * Prime1;
    }
	hash = avalanche(hash);
    PUT_U64<bswap>(hash, (uint8_t *)out,  0);
}
REGISTER_FAMILY(xxh64,
   $.src_url    = "https://github.com/AnatolyGeorgievski/MWC128/",
   $.src_status = HashFamilyInfo::SRC_ACTIVE
 );

REGISTER_HASH(xxh64_64,
   $.desc       = "64-bit xxHash-64 portable",
   $.hash_flags =
        0,
   $.impl_flags =
        FLAG_IMPL_CANONICAL_LE        |
        FLAG_IMPL_MULTIPLY_64_64      |
        FLAG_IMPL_ROTATE              |
        FLAG_IMPL_LICENSE_PUBLIC_DOMAIN,
   $.bits = 64,
   $.verification_LE = 0x024B7CF4,
   $.verification_BE = 0xB96ABE81,
   $.hashfn_native   = xxh64_hash<false>,
   $.hashfn_bswap    = xxh64_hash<true>
 );
