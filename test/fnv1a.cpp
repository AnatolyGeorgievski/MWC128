/*
 * FNV and similar hashes
 * Copyright (C) 2026  Anatoly Georgivskii
 *
 * Переписал FNV-1a с другими коэффициентами, чтобы функция проходила тесты Avalanche,BIC,Sparse
 * Версия подготовлена для теста SMHasher3.
 */
#include "Platform.h"
#include "Hashlib.h"

#include "Mathmult.h"

#define C_ 	        UINT64_C(0x100000001b3)// -- было
#define C1 	        UINT64_C(0xcbf29ce484222325)
#define C_2	        UINT64_C(0xa3b195354a39b70d)
#define MC 	        UINT64_C(0xa3b195354a39b70d)
static FORCE_INLINE uint64_t nomix( uint64_t A, uint64_t B) {
    (void)B;
    return A;
}
static FORCE_INLINE uint64_t _mum( uint64_t A, uint64_t B) {
    uint64_t rlo, rhi;
    MathMult::mult64_128(rlo, rhi, B, A);
    return rlo ^ rhi;
}
static FORCE_INLINE uint64_t _mix( uint64_t A, uint64_t B) {
    uint64_t rlo, rhi;
    MathMult::mult64_128(rlo, rhi, B, A);
    return rlo + rhi;
}
static FORCE_INLINE uint64_t mumix( uint64_t A, uint64_t B) {
    return _mum(A^A>>32, B);
}

typedef uint64_t (*mix_t)(uint64_t, uint64_t);
template <bool bswap, mix_t unmix, mix_t mix, uint64_t C2>
static void FNV1a( const void * in, size_t len, uint64_t seed, void * out ) {
    const uint8_t * data = (const uint8_t *)in;
    uint64_t h = unmix(C1^seed, MC);// добавил миксер на SEED
    while (len-->0)
        h = (h^*data++) * C2;
    h = mix(h, MC);// добавил миксер на выход функции
    PUT_U64<bswap>(h, (uint8_t *)out, 0);
}
#undef C2
#if 0
// -log2(p-value) summary:
//   0     1     2     3     4     5     6     7     8     9    10    11    12
// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
//  4443  1295   589   260   146    82    29    18     8     5     4     2     0

//  13    14    15    16    17    18    19    20    21    22    23    24    25+
// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
//   2     0     0     0     0     0     0     0     0     0     0     0     0

#define C2      UINT64_C(0x8b4e7465b45a2765) // 62 32
#define C4      UINT64_C(0x317184d7c113edd9) // 61 32
#define C8      UINT64_C(0x79f258ac5d3181f1) // 60 32
#define C16     UINT64_C(0xe36d4dd41a36c4e1) // 59 32
#define C32     UINT64_C(0x0d6e9a5d5a554dc1) // 58 32
#define C64     UINT64_C(0xcf42c9526dc7ab81) // 57 32
#define C128    UINT64_C(0x1ad2146bef739701) // 56 32
#define C256    UINT64_C(0x0bd56afae1f82e01) // 55 32
#define C512    UINT64_C(0x49f2df6aec345c01) // 54 32
#elif 1
// -log2(p-value) summary:
//   0     1     2     3     4     5     6     7     8     9    10    11    12
// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
//  4362  1297   664   284   141    71    34    12    10     4     2     2     0
#define C2      UINT64_C(0x99e4d4b2e459691d) // 62 32
#define C4      UINT64_C(0x53c596c9e952cd49) // 61 32
#define C8      UINT64_C(0x8999bd190961fed1) // 60 32
#define C16     UINT64_C(0xcf8e683fca0566a1) // 59 32
#define C32     UINT64_C(0xaa13d9513f6eb141) // 58 32
#define C64     UINT64_C(0x7fea083ccc96f281) // 57 32
#define C128    UINT64_C(0x4b3ea7f6ace42501) // 56 32
#define C256    UINT64_C(0xe5fdc8f747214a01) // 55 32
#define C512    UINT64_C(0x699bd94eb7a69401) // 54 32
#endif
template <bool bswap>
static void FNV1a_fast( const void * in, size_t len, uint64_t seed, void * out ) {
    const uint8_t * data = (const uint8_t *)in;
    uint64_t h = _mum(C1^seed, C2);// добавил миксер на SEED
    if (unlikely(len>=32)) {
        uint64_t h1 = 0;
        uint64_t h2 = 0;
        uint64_t h3 = 0;
        while (len>= 32) {
            uint64_t d0 = GET_U64<bswap>(data, 0);
            uint64_t d1 = GET_U64<bswap>(data, 8);
            uint64_t d2 = GET_U64<bswap>(data,16);
            uint64_t d3 = GET_U64<bswap>(data,24);
            h  = _mum(h ^d0,C64);
            h1 = _mum(h1^d1,C64);
            h2 = _mum(h2^d2,C64);
            h3 = _mum(h3^d3,C64);
            len-=32; data+=32;
        }
        h  =h1^_mum(h, C16); // merge
        h2 =h3^_mum(h2,C16);
        h  =h2^_mum(h, C32);
    }
    while (len>=8) {
        uint64_t d0 = GET_U64<bswap>(data, 0);
        h  = _mum(h ^d0,C16);
        len-=8; data+=8;
    }
    while (len-->0)
        h = _mum(h^*data++,C2);
    h = _mum(h^h>>32, C2);// добавил миксер на выход функции
    PUT_U64<bswap>(h, (uint8_t *)out, 0);
}
#undef  C2
#define C2 	        UINT64_C(0xa3b195354a39b70d)
// В этой версии использован миксер rhi + rlo из проекта MUM-hash
template <bool bswap>
static void FNV1a_mix( const void * in, size_t len, uint64_t seed, void * out ) {
    const uint8_t * data = (const uint8_t *)in;
    uint64_t h = _mum(C1^seed, C2);// добавил миксер на SEED
    while (len-->0)
        h = _mix(h^*data++, C2);// миксер как в проекте MUM-hash
    h = _mum(h^h>>32, C2);// добавил миксер на выход функции
    PUT_U64<bswap>(h, (uint8_t *)out, 0);
}
template <bool bswap>
static void FNV1a_mum( const void * in, size_t len, uint64_t seed, void * out ) {
    const uint8_t * data = (const uint8_t *)in;
    uint64_t h = _mum(C1^seed, C2);// добавил миксер на SEED
    while (len-->0)
        h = _mum(h^*data++, C2);// миксер как в проекте MUM-hash
    h = _mum(h, C2);// добавил миксер на выход функции
    PUT_U64<bswap>(h, (uint8_t *)out, 0);
}

template <bool bswap>
static void FNV1a_128( const void * in, size_t len, const seed_t seed, void * out ) {
    const uint8_t * data = (const uint8_t *)in;
    const uint64_t  C1lo = UINT64_C(0x62b821756295c58d);
    const uint64_t  C1hi = UINT64_C(0x6c62272e07bb0142);
    const uint64_t  C2mx = UINT64_C(0xa3b195354a39b70d);

    register uint128_t h = (((uint128_t) C1hi << 64) | C1lo);
    h^= seed;
    h = (h>>64) + (uint64_t)h * (uint128_t)C64;// MWC?
    if (len>=32) {
        while (len>=16) {
            h^= *(uint128_t*)data; data+=16;
            h = (h>>64) + (uint64_t)h * (uint128_t)C128;// MWC?
            len -= 16;
        }
    }
    while (len>=8) {
        h^= *(uint64_t*)data; data+=8;
        h = (h>>64) + (uint64_t)h * (uint128_t)C64;// MWC?
        len -= 8;
    }
    while (len-->0) {
        h^= *data++;
        int r = 1;
        h = (h>>(r*8)) + ((uint64_t)h<<(64-r*8)) * (uint128_t)C2mx;// MWC?
    }
    uint64_t hash[2];
    h = (h>>64) + (uint64_t)h * (uint128_t)C64;// MWC?
    hash[0] = h>>64;
    hash[1] = h;
    PUT_U64<bswap>(hash[0], (uint8_t *)out, 0);
    PUT_U64<bswap>(hash[1], (uint8_t *)out, 8);
}


REGISTER_FAMILY(fnv1a,
   $.src_url    = "https://github.com/AnatolyGeorgievski/MWC128/",
   $.src_status = HashFamilyInfo::SRC_ACTIVE
 );
REGISTER_HASH(FNV1a_64,
   $.desc       = "64-bit bytewise FNV-1a original",
   $.hash_flags =
         0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_64 |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN    |
         FLAG_IMPL_VERY_SLOW,
   $.bits = 64,
   $.verification_LE = 0x103455FC,
   $.verification_BE = 0x4B032B63,
   $.hashfn_native   = FNV1a<false, nomix, nomix, C_>,
   $.hashfn_bswap    = FNV1a<true,  nomix, nomix, C_>
 );
REGISTER_HASH(FNV1a_64__b,
   $.desc       = "64-bit bytewise FNV-1a with output avalanche bit mixer",
   $.hash_flags =
         0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_64 |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN    |
         FLAG_IMPL_VERY_SLOW,
   $.bits = 64,
   $.verification_LE = 0x3B88917F,
   $.verification_BE = 0xF5A57F48,
   $.hashfn_native   = FNV1a<false, _mum, mumix, C_2>,
   $.hashfn_bswap    = FNV1a<true, _mum, mumix, C_2>
 );
REGISTER_HASH(FNV1a_64__fast,
   $.desc       = "64-bit FNV-1a Fast with Seed and wymum-mixer",
   $.hash_flags =
         0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_64 |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN,
   $.bits = 64,
   $.verification_LE = 0x8027FA7E,
   $.verification_BE = 0xA8D080D9,
   $.hashfn_native   = FNV1a_fast<false>,
   $.hashfn_bswap    = FNV1a_fast<true>
 );
REGISTER_HASH(FNV1a_64__mix,
   $.desc       = "64-bit bytewise FNV-1a with Seed and MUM-mixer",
   $.hash_flags =
         0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_64 |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN    |
         FLAG_IMPL_VERY_SLOW,
   $.bits = 64,
   $.verification_LE = 0x82902BDE,
   $.verification_BE = 0x57F87794,
   $.hashfn_native   = FNV1a_mix<false>,
   $.hashfn_bswap    = FNV1a_mix<true>
 );
REGISTER_HASH(FNV1a_64__mum,
   $.desc       = "64-bit bytewise FNV-1a with Seed and wymum-mixer",
   $.hash_flags =
         0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_64 |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN    |
         FLAG_IMPL_VERY_SLOW,
   $.bits = 64,
   $.verification_LE = 0x8E17A335,
   $.verification_BE = 0xF8C4ED4B,
   $.hashfn_native   = FNV1a_mum<false>,
   $.hashfn_bswap    = FNV1a_mum<true>
 );
REGISTER_HASH(FNV1a_128,
   $.desc       = "128-bit bytewise FNV-1a (Fowler-Noll-Vo)",
   $.hash_flags =
         FLAG_HASH_ENDIAN_INDEPENDENT |
         FLAG_HASH_NO_SEED,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_128    |
         FLAG_IMPL_LICENSE_BSD        
         ,
   $.bits = 128,
   $.verification_LE = 0x0269D36F,
   $.verification_BE = 0x0269D36F,
   $.hashfn_native   = FNV1a_128<false>,
   $.hashfn_bswap    = FNV1a_128<true>
 );
