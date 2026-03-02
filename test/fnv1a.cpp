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
static FORCE_INLINE uint64_t _mum_no_carry( uint64_t A, uint64_t B) {
    uint64_t rlo, rhi;
    MathMult::mult64_128_nocarry(rlo, rhi, B, A);
    return rlo ^ rhi;
}
static FORCE_INLINE uint64_t _mxm( uint64_t A, uint64_t B) {
    uint64_t rlo, rhi;
    MathMult::mult64_128(rlo, rhi, B, A);
    return rlo ^ rhi ^ (rhi<<32);
}
static FORCE_INLINE uint64_t _msm( uint64_t A, uint64_t B) {
    uint64_t rlo, rhi;
    MathMult::mult64_128(rlo, rhi, B, A);
    return rlo ^ rhi ^ (rhi>>32);
}
static FORCE_INLINE uint64_t _mxr( uint64_t A, uint64_t B) {
    uint64_t rlo, rhi;
    MathMult::mult64_128(rlo, rhi, B, A);
    return rlo ^ ROTR64(rhi,25);// подобрать оптимальный ROTL 7 9 13 18
}
static FORCE_INLINE uint64_t _arx( uint64_t A, uint64_t B) {
    uint64_t rlo, rhi;
    MathMult::mult64_128(rlo, rhi, B, A);
//   v0 += v3;
//   v3 = v0 ^ ROTL64(v3,21); 
    return (rlo + rhi) ^ ROTL64(rhi,21);// подобрать оптимальный 13 17 21 
}
static FORCE_INLINE uint64_t _axr( uint64_t A, uint64_t B) {
    uint64_t rlo, rhi;
    MathMult::mult64_128(rlo, rhi, B, A);
    return rlo ^ ROTL64(rlo + rhi,18);// подобрать оптимальный 7 9 13 18
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

#define M1      UINT64_C(0x99e4d4b2e459691d) // 62 32
#define M2      UINT64_C(0x53c596c9e952cd49) // 61 32
#define M4      UINT64_C(0x53c596c9e952cd49) // 61 32
#define M8      UINT64_C(0x8999bd190961fed1) // 60 32
#define M16     UINT64_C(0xcf8e683fca0566a1) // 59 32
#define M24     (M16*M8) // 58 32
#define M32     UINT64_C(0xaa13d9513f6eb141) // 58 32
// -log2(p-value) summary:
//   0     1     2     3     4     5     6     7     8     9    10    11    12
// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
//  4434  1273   599   291   141    78    32    23     5     6     5     2     0

template <bool bswap, mix_t unmix, mix_t mix, uint64_t C2>
static void FNV1( const void * in, size_t len, uint64_t seed, void * out ) {
    const uint8_t * data = (const uint8_t *)in;
    uint128_t h = unmix(C1^seed, MC);// добавил миксер на SEED
    if (0 && len>=32) {
        uint128_t h1 = 0;
        while (len>=32) {// 12 GB/s
            h ^= *(uint128_t*)data; data+=16;
            h1^= *(uint128_t*)data; data+=16;
            h = (h>>64)*(uint128_t)M24 + (uint64_t)h*(uint128_t)M32;
            h1 = (h1>>64)*(uint128_t)M24 + (uint64_t)h1*(uint128_t)M32;
            len-=32;
        }
        h = (h>>64)*(uint128_t)M4 + (uint64_t)h*(uint128_t)M8;
        h^= h1;
    }
    while (0 && len>=16) {// 12 GB/s
        h^= *(uint128_t*)data; data+=16;
        h = (h>>64)*(uint128_t)M8 + (uint64_t)h*(uint128_t)M16;
        len-=16;
    }
    while (0 && len>=8) {
        h+= *(uint64_t*)data; data+=8;
        h = (h>>64)*(uint128_t) + (uint64_t)h*(uint128_t)M8;
        len-=8;
    }
    while (len-->0) {
        h^= *data++;
        h = (h>>64)*(uint128_t)M1 + (uint64_t)h*(uint128_t)M2;
    }
    h = mix(h^h>>64, MC);// добавил миксер на выход функции
    PUT_U64<bswap>(h, (uint8_t *)out, 0);
}
#undef C2
// -log2(p-value) summary:
//   0     1     2     3     4     5     6     7     8     9    10    11    12
// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
//  4362  1297   664   284   141    71    34    12    10     4     2     2     0

/*! 
 Принцип выбора чисел такой что их можно представить в виде Сn = (M_n<<n) +1
 Принцип выбора чисел в оригинальном алгоритме С0 = M_0*256 + K - простое число
 Мы выбираем так чтобы С0%4 == 3
 */
#define C2      UINT64_C(0x99e4d4b2e459691d) // 62 32
#define C4      UINT64_C(0x53c596c9e952cd49) // 61 32
#define C8      UINT64_C(0x8999bd190961fed1) // 60 32
#define C16     UINT64_C(0xcf8e683fca0566a1) // 59 32
#define C32     UINT64_C(0xaa13d9513f6eb141) // 58 32
#define C64     UINT64_C(0x7fea083ccc96f281) // 57 32
#define C128    UINT64_C(0x1ad2146bef739701) // 56 32
template <bool bswap, mix_t mix>
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
            h  = mix(h ^d0,C64);
            h1 = mix(h1^d1,C64);
            h2 = mix(h2^d2,C64);
            h3 = mix(h3^d3,C64);
            len-=32; data+=32;
        }
        h  =h2^mix(h, C32); // merge
        h1 =h3^mix(h1,C32);
        h  =h1^mix(h, C16);
    }
    while (len>=8) {
        uint64_t d0 = GET_U64<bswap>(data, 0);
        h  = mix(h ^d0,C16);
        len-=8; data+=8;
    }
    while (len-->0)
        h = mix(h^*data++,C2);
    h = _mum(h^h>>32, C2);// добавил миксер на выход функции
    PUT_U64<bswap>(h, (uint8_t *)out, 0);
}
#undef  C2
#define C2 	        UINT64_C(0xa3b195354a39b70d)
template <bool bswap, mix_t mix>
static void FNV1a_mum( const void * in, size_t len, uint64_t seed, void * out ) {
    const uint8_t * data = (const uint8_t *)in;
    uint64_t h = _mum(C1^seed, C2);// добавил миксер на SEED
    while (len-->0)
        h = mix(h^*data++, C2);
    h = _mum(h, C2);// добавил миксер на выход функции
    PUT_U64<bswap>(h, (uint8_t *)out, 0);
}
template <bool bswap, mix_t mix>
static void FNV1a_mwc( const void * in, size_t len, uint64_t seed, void * out ) {
    const uint8_t * data = (const uint8_t *)in;
    uint64_t h = _mum(C1^seed, C2);// добавил миксер на SEED
    while (len-->0) {
        h =(h^*data++)* 0x100000001b3u;
        h = ROTR64(h, 17) ^ h;
    }
    h = _mum(h, C2);// добавил миксер на выход функции
    PUT_U64<bswap>(h, (uint8_t *)out, 0);
}

template <bool bswap>
static void FNV1a_128( const void * in, size_t len, const seed_t seed, void * out ) {
    const uint8_t * data = (const uint8_t *)in;
    const uint64_t  C1lo = UINT64_C(0x62b821756295c58d);
    const uint64_t  C1hi = UINT64_C(0x6c62272e07bb0142);
// 128-bit FNV_Prime = 2^{88} + 2^8 + 0x3B
    const uint64_t  C2mx = UINT64_C(0xa3b195354a39b70d);
#define MWC_A1 		(uint128_t)0xffebb71d94fcdaf9ull
    register uint128_t h = (((uint128_t) C1hi << 64) | C1lo);
    h = _mum(h^seed, C2mx);
    while (len>=8) {
        h+= *(uint64_t*)data; data+=8;
        h = (h>>64) + (uint64_t)h * (uint128_t)MWC_A1;
        len -= 8;
    }
    while (len-->0) {
        h+= *data++;
        int r = 1;
        h = (h>>(r*8)) + ((uint64_t)h<<(64-r*8)) * (uint128_t)MWC_A1;
    }
    PUT_U64<bswap>(_mum(h^h>>64, C2mx), (uint8_t *)out, 0);
    h = (h>>64) + (uint64_t)h * (uint128_t)MWC_A1;
    PUT_U64<bswap>(_mum(h^h>>64, C2mx), (uint8_t *)out, 8);
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
REGISTER_HASH(FNV1_64,
   $.desc       = "64-bit bytewise FNV-1 original",
   $.hash_flags =
         0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_64 |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN    |
         FLAG_IMPL_VERY_SLOW,
   $.bits = 64,
   $.verification_LE = 0x103455FC,
   $.verification_BE = 0x4B032B63,
   $.hashfn_native   = FNV1<false, nomix, nomix, C_>,
   $.hashfn_bswap    = FNV1<true,  nomix, nomix, C_>
 );
REGISTER_HASH(FNV1a_64__b,
   $.desc       = "64-bit bytewise FNV-1a with avalanche bit mixer",
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
REGISTER_HASH(FNV1_64__b,
   $.desc       = "64-bit bytewise FNV-1 with avalanche bit mixer",
   $.hash_flags =
         0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_64 |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN    |
         FLAG_IMPL_VERY_SLOW,
   $.bits = 64,
   $.verification_LE = 0x3B88917F,
   $.verification_BE = 0xF5A57F48,
   $.hashfn_native   = FNV1<false, _mum, mumix, C_2>,
   $.hashfn_bswap    = FNV1<true, _mum, mumix, C_2>
 );
REGISTER_HASH(FNV1a_64__fast,
   $.desc       = "64-bit FNV-1a Fast with Seed and wymum-mixer",
   $.hash_flags =
         0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_64 |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN,
   $.bits = 64,
   $.verification_LE = 0x9476D68F,
   $.verification_BE = 0x61F152DA,
   $.hashfn_native   = FNV1a_fast<false, _mum>,
   $.hashfn_bswap    = FNV1a_fast<true, _mum>
 );
REGISTER_HASH(FNV1a_64__noc,
   $.desc       = "64-bit FNV-1a Fast with Seed and mum-mixer w/o carry",
   $.hash_flags =
         0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_64 |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN,
   $.bits = 64,
   $.verification_LE = 0x9476D68F,
   $.verification_BE = 0x61F152DA,
   $.hashfn_native   = FNV1a_fast<false, _mum_no_carry>,
   $.hashfn_bswap    = FNV1a_fast<true, _mum_no_carry>
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
   $.hashfn_native   = FNV1a_mum<false, _mum>,
   $.hashfn_bswap    = FNV1a_mum<true, _mum>
 );
REGISTER_HASH(FNV1a_64__mxm,
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
   $.hashfn_native   = FNV1a_mum<false, _mxm>,
   $.hashfn_bswap    = FNV1a_mum<true, _mxm>
 );
REGISTER_HASH(FNV1a_64__msm,
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
   $.hashfn_native   = FNV1a_mum<false, _msm>,
   $.hashfn_bswap    = FNV1a_mum<true, _msm>
 );
REGISTER_HASH(FNV1a_64__mxr,
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
   $.hashfn_native   = FNV1a_mum<false, _mxr>,
   $.hashfn_bswap    = FNV1a_mum<true, _mxr>
 );
REGISTER_HASH(FNV1a_64__arx,
   $.desc       = "64-bit bytewise FNV-1a with Seed and ARX-mixer",
   $.hash_flags =
         0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_64 |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN    |
         FLAG_IMPL_VERY_SLOW,
   $.bits = 64,
   $.verification_LE = 0x8E17A335,
   $.verification_BE = 0xF8C4ED4B,
   $.hashfn_native   = FNV1a_mum<false, _arx>,
   $.hashfn_bswap    = FNV1a_mum<true, _arx>
 );
REGISTER_HASH(FNV1a_64__axr,
   $.desc       = "64-bit bytewise FNV-1a with Seed and ARX-mixer",
   $.hash_flags =
         0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_64 |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN    |
         FLAG_IMPL_VERY_SLOW,
   $.bits = 64,
   $.verification_LE = 0x8E17A335,
   $.verification_BE = 0xF8C4ED4B,
   $.hashfn_native   = FNV1a_mum<false, _axr>,
   $.hashfn_bswap    = FNV1a_mum<true, _axr>
 );
REGISTER_HASH(FNV1a_64__mumx,
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
   $.hashfn_native   = FNV1a_mum<false, mumix>,
   $.hashfn_bswap    = FNV1a_mum<true, mumix>
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
