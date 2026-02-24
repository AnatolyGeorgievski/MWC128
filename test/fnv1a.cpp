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

//#define C2 	        UINT64_C(0x100000001b3) -- было
#define C1 	        UINT64_C(0xcbf29ce484222325)
#define C2 	        UINT64_C(0xa3b195354a39b70d)
#define C4  UINT64_C(0xae7a58388aad96a9)
#define C8  UINT64_C(0xa3b9b6132d147b91)
#define C16 UINT64_C(0x163eb89a844ca821)
#define C32 UINT64_C(0xbe44dc0a4a035441)
#define C64 UINT64_C(0xd50039d4a940b881)

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
template <bool bswap>
static void FNV1a( const void * in, size_t len, uint64_t seed, void * out ) {
    const uint8_t * data = (const uint8_t *)in;
    uint64_t h = _mum(C1^seed, C2);// добавил миксер на SEED
    while (len-->0)
        h = (h^*data++) * C2;
    h = _mum(h^h>>32, C2);// добавил миксер на выход функции
    PUT_U64<bswap>(h, (uint8_t *)out, 0);
}
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
        h = (h^*data++) * C2;
    h = _mum(h^h>>32, C2);// добавил миксер на выход функции
    PUT_U64<bswap>(h, (uint8_t *)out, 0);
}
// В этой версии использован миксер rhi + rlo из проекта MUM-hash
template <bool bswap>
static void FNV1a_mum( const void * in, size_t len, uint64_t seed, void * out ) {
    const uint8_t * data = (const uint8_t *)in;
    uint64_t h = _mum(C1^seed, C2);// добавил миксер на SEED
    for (size_t i = 0; i < len; i++)
        h = _mix(h^data[i], C2);// миксер как в проекте MUM-hash
    h = _mum(h^h>>32, C2);// добавил миксер на выход функции
    PUT_U64<bswap>(h, (uint8_t *)out, 0);
}
REGISTER_FAMILY(fnv1a,
   $.src_url    = "https://github.com/AnatolyGeorgievski/MWC128/",
   $.src_status = HashFamilyInfo::SRC_ACTIVE
 );
REGISTER_HASH(FNV1a_64,
   $.desc       = "64-bit bytewise FNV-1a with Seed and wymum-mixer",
   $.hash_flags =
         0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_64 |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN    |
         FLAG_IMPL_VERY_SLOW,
   $.bits = 64,
   $.verification_LE = 0x3B88917F,
   $.verification_BE = 0xF5A57F48,
   $.hashfn_native   = FNV1a<false>,
   $.hashfn_bswap    = FNV1a<true>
 );
REGISTER_HASH(FNV1a_64__fast,
   $.desc       = "64-bit FNV-1a with Seed and wymum-mixer",
   $.hash_flags =
         0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_64 |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN,
   $.bits = 64,
   $.verification_LE = 0x46720F0C,
   $.verification_BE = 0xA66619D4,
   $.hashfn_native   = FNV1a_fast<false>,
   $.hashfn_bswap    = FNV1a_fast<true>
 );
REGISTER_HASH(FNV1a_64__mum,
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
   $.hashfn_native   = FNV1a_mum<false>,
   $.hashfn_bswap    = FNV1a_mum<true>
 );
