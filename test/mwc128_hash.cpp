/*
 * MWC128 and similar hashes
 * Copyright (C) 2026  Anatoly M. Georgievskii
 */
#include "Platform.h"
#include "Hashlib.h"
#include "Mathmult.h"
// Doug Lea's mixing function, fastmix дважды

#define MC UINT64_C(0xdaba0b6eb09322e3);
#define MC2 UINT64_C(0xad4d2974b2f97955);
static inline uint64_t mix_lea(uint64_t h) {
  h = (h ^ h >> 32)*MC;
  h = (h ^ h >> 32)*MC;
  return (h ^ h >> 32);
}
static inline uint64_t unmix_lea(uint64_t h) {
  h = (h ^ h >> 32)*MC2;
  h = (h ^ h >> 32)*MC2;
  return (h ^ h >> 32);
}
//typedef unsigned int __attribute__((mode(TI)))   uint128_t;
#define MWC_A1 		UINT64_C(0xffebb71d94fcdaf9)
#define MWC_NA1     UINT64_C(0x001448E26B032507)
#define MUM_C       UINT64_C(0xa3b195354a39b70d)
#define MUM_S       UINT64_C(0x82d2e9550235efc5)
#define IV 	        UINT64_C(0x9e3779b97f4a7c15) // хорошая аддитивная константа golden ratio
#define STATE_SZ 2
#define unmix unmix_lea
#define mix mix_lea
static inline uint64_t _mum( uint64_t A, uint64_t B) {
    uint64_t rlo, rhi;
    MathMult::mult64_128(rlo, rhi, B, A);
    return rlo ^ rhi;
}
// -log2(p-value) summary: Если операция XOR
//           0     1     2     3     4     5     6     7     8     9    10    11    12
//         ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
//          6044  1318   612   345   166    70    42    15     8     3     2     1     1
// -log2(p-value) summary: Если операция ADD
//           0     1     2     3     4     5     6     7     8     9    10    11    12  
//         ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
//          6069  1326   589   333   141    82    48    23     7     4     4     1     0


static inline void mwc128_next(uint64_t* state, uint64_t d, int r) {
	uint128_t  t =*(uint128_t*)state + d;
	t = (uint128_t)MWC_A1 * ((uint64_t)t<<(64-r)) + (t>>r);
    *(uint128_t*)state = t;
}
template <int tlen, bool bswap>
void mwc128_hash(const void *in, size_t len, uint64_t seed, void* out) {
    const uint8_t* data = (const uint8_t*)in;
	uint64_t s[2];
    s[0] = _mum(seed += IV, MUM_S);
    s[1] = IV;
    while (len >= 8) {
        uint64_t d = GET_U64<false>(data, 0);
        mwc128_next(s, d, 64);// ROUND MIX
        len -= 8; data+=8;
    }
	if (len) {
        if (len&4) {
            uint64_t d = GET_U32<false>(data, 0); data+=4;
            mwc128_next(s, d, 32);
        }
        if (len&2) {
            uint64_t d = *(uint16_t*)data; data+=2;
            mwc128_next(s, d, 16);
        }
        if (len&1) {
            uint64_t d = *(uint8_t*)data; data++;
            mwc128_next(s, d, 8);
        }
    }
    uint64_t d = _mum(s[0]^s[1], MUM_C);// WY multiplier
    PUT_U64<bswap>(d, (uint8_t *)out,  0);
    if (tlen==128){
        mwc128_next(s+0, seed+=IV, 64);
        uint64_t d1 = _mum(s[0]^s[1], MUM_C);
        PUT_U64<bswap>(d1, (uint8_t *)out,  8);
    }
}
#define MWC_A2 0xffa04e67b3c95d86u // MWC192, B2 = 1<<128
static inline void mwc192_next(uint64_t* state, uint64_t d, int r) {
	uint128_t  t =((uint128_t)state[0]<<64|state[1])+d;
	t = (uint128_t)MWC_A2 * ((uint64_t)t<<(64-r)) + (t>>r);
	state[0] = state[2];
	state[1] = t;
	state[2] = t >> 64;
}
template <int tlen, bool bswap>
void mwc192_hash(const void *in, size_t len, seed_t seed, void* out) {
    const uint8_t* data = (const uint8_t*)in;
	uint64_t s[3];
    s[0] = _mum(seed += IV, UINT64_C(0x82d2e9550235efc5));
    s[1] = _mum(seed += IV, UINT64_C(0x82d2e9550235efc5));
    s[2] = IV;//_mum(seed += IV, UINT64_C(0x82d2e9550235efc5));
	for (unsigned int i=0; i<len/8; i++, data+=8){
        uint64_t d = GET_U64<bswap>(data, 0);
		mwc192_next(s, d, 64);// ROUND MIX
	}
	int r = len%8;
	if (r) {
        uint64_t d = GET_U64<bswap>(data, 0);
        d &= ~0uLL>>(64-r*8);
        mwc192_next(s, d, r*8);
    }
    mwc192_next(s+0, seed+=IV, 64);
    uint64_t d = _mum(s[0]^s[1], UINT64_C(0xa3b195354a39b70d));
    PUT_U64<bswap>(d, (uint8_t *)out,  0);
    if (tlen==128){
        mwc192_next(s+0, seed+=IV, 64);
        mwc192_next(s+0, seed+=IV, 64);
        uint64_t d1 = _mum(s[0]^s[1], UINT64_C(0xa3b195354a39b70d));
        PUT_U64<bswap>(d1, (uint8_t *)out,  8);
    }
}
#define MWC_A  0x7ff8c871
static inline int64_t next(int64_t x, int r){
    return (x>>r) - ((uint32_t)x<<(32-r))*(int64_t)MWC_A;
}
template <bool bswap>
void mwc64s_hash(const void* in, size_t len, uint64_t seed, void* out){
    const uint8_t* data = (const uint8_t*)in;
    int64_t hash = (int64_t)unmix(seed+IV);
    for (size_t i=0; i<len>>2; i++){
        hash += *(uint32_t*) data; data+=4;
        hash  = next(hash, 32);
    }
    if (len&2){
        hash += *(uint16_t*) data; data+=2;
        hash  = next(hash, 16);
    }
    if (len&1){
        hash += *(uint8_t*) data; data+=1;
        hash  = next(hash, 8);
    }
    uint32_t d = mix_lea((uint64_t)hash)-IV;
    PUT_U32<bswap>(d, (uint8_t *)out,  0);
}
#define MWC_A0  0xFFFF1EBDuLL//0xfffe59a7uLL//fffeb81bULL
#define MWC_NA0 0x1A659
#define MWC_INV UINT64_C(0x0001A65BB8CE0887)
#define MWC_J64 UINT64_C(0xFFFCB350B8C98AF1)
#define MWC_J65 UINT64_C(0xFFFD7037A3FAD2D9)
#define MWC_J66 UINT64_C(0xFFFC3A198F1F7E79)
#define MWC_PRIME ((MWC_A0<<32) -1)
static inline uint64_t _next(uint64_t x, int r){
    return ((uint32_t)x<<(32-r))*MWC_A0 + (x>>r);
}
static inline uint64_t mwc_mod(uint128_t ac, uint64_t M, uint64_t M_INV) {	
	ac-= (((ac>>64)*M_INV + ac)>>64)*M;
	if (ac>>64) ac -= M;
	return ac;
}
static const uint64_t j64  = (MWC_A0*MWC_A0);//UINT64_C(0xFFFCB350B8C98AF1);
static const uint64_t j96  = UINT64_C(0xB8C85A1586E21F87);
static const uint64_t j_64 = UINT64_C(0x00034CAF4736750F);
static const uint64_t j128 = UINT64_C(0x86E141001432DA26);
static const uint64_t j192 = UINT64_C(0xAD97A763E40AF999);
static const uint64_t j256 = UINT64_C(0x4E5429F8166590FE);

static inline uint128_t mwc_foldm(uint128_t h, uint128_t d, const uint64_t j1, const uint64_t j2, const uint64_t M);

template <bool bswap>
void mwc64_hash_xor(const void* in, size_t len, uint64_t seed, void* out){
    const uint8_t* data = (const uint8_t*)in;
    uint64_t hash = unmix(seed ^ IV);
#ifdef __SIZEOF_INT128__
    if (len>=32) {
        uint128_t h = (uint128_t)hash;
        while (len>=8) {
            uint64_t d0 = (*(uint64_t*) data); data+=8; 
            h^= d0;
            h = (h>>64) + (uint64_t)h * (uint128_t)j64;// round mix 128
            len -= 8;
        }
        len &= 7;
        hash = mwc_mod(h, MWC_PRIME, MWC_INV);
    }
#endif
    while (len>=4){
        hash^= (*(uint32_t*) data); data+=4;
        hash = _next(hash, 32);
        len -= 4;
    }
    if (len&2){
        hash^= *(uint16_t*) data; data+=2;
        hash = _next(hash, 16);
    }
    if (len&1){
        hash^= *(uint8_t*) data; data+=1;
        hash = _next(hash, 8);
    }
    uint64_t d = mix(hash)^IV;
    PUT_U64<bswap>(d, (uint8_t *)out,  0);
}

template <bool bswap>
void mwc64_hash(const void* in, size_t len, uint64_t seed, void* out){
    const unsigned F = 4;//2:128 256, 
    const uint8_t* data = (const uint8_t*)in;
    uint64_t hash = unmix(seed + IV);
//    uint64_t hash = _mum(seed + IV, UINT64_C(0x82d2e9550235efc5));
#ifdef __SIZEOF_INT128__
    if (len>=32) {
        uint128_t h[F];
        h[0] = (uint128_t)hash;
        if (0 && len>=8*F) {
            for (unsigned int i=1;i<F;i++) h[i] = 0;
            while (len>=8*F) {
                for (unsigned int i=0;i<F;i++)
                    h[i]+= *(uint64_t*)(data+i*8);
                for (unsigned int i=0;i<F;i++) 
                    h[i] = (h[i]>>64) + (uint64_t)h[i] * (uint128_t)j256;
                data+=8*F; len -= 8*F;
            }
            h[0] = (h[0]>>64)*j64 + (uint64_t)h[0] * (uint128_t)j128;
            h[1] = (h[1]>>64)*j64 + (uint64_t)h[1] * (uint128_t)j128;
            if (__builtin_add_overflow(h[0],h[2],&h[0]))
                h[0] -= (uint128_t)MWC_PRIME <<64;
            if (__builtin_add_overflow(h[1],h[3],&h[1]))
                h[1] -= (uint128_t)MWC_PRIME <<64;
            h[0] = (h[0]>>64) + (uint64_t)h[0] * (uint128_t)j64;
            if (__builtin_add_overflow(h[0],h[1],&h[0]))
                h[0] -= (uint128_t)MWC_PRIME <<64;
        }

        while (len>=8) {
            uint64_t d0 = (*(uint64_t*) data); data+=8; 
            h[0]+= d0;
            h[0] = (h[0]>>64) + (uint64_t)h[0] * (uint128_t)j64;// round mix 128
            len -= 8;
        }
        len &= 7;
        // if (r) { // рабочий но медленный
        //     uint64_t d0 = (*(uint64_t*) data); data+=8;
        //     d0 &= ~0uLL>>(64-r*8);
        //     h+=d0;
        //     h = (h>>(r*8)) + ((uint64_t)h<<(64-r*8)) * (uint128_t)j64;
        // }
        hash = mwc_mod(h[0], MWC_PRIME, MWC_INV);
    }
#endif
    unsigned int blocks = (len>>2);
    for (unsigned int i=0; i<blocks; i++)
    {
        hash+= *(uint32_t*) data; data+=4;
        hash = _next(hash, 32);
    }
    if (len&2){
        hash+= *(uint16_t*) data; data+=2;
        hash = _next(hash, 16);
    }
    if (len&1){
        hash+= *(uint8_t*) data; data+=1;
        hash = _next(hash, 8);
    }
    uint64_t d = mix(hash)-IV;
//    uint64_t d = _mum(hash^hash>>32, UINT64_C(0xa3b195354a39b70d))-IV;
    PUT_U64<bswap>(d, (uint8_t *)out,  0);
}

static inline uint128_t mwc_foldm(uint128_t h, uint128_t d, const uint64_t j1, const uint64_t j2, const uint64_t M){
    if (__builtin_add_overflow(h, d, &h))
        h -= (uint128_t)M<<64;
    uint128_t t;
    t = (uint64_t)h * (uint128_t)j1;// k
    h = (h>>64) * (uint128_t)j2; // k-64
    if (__builtin_add_overflow(h, t, &h))
        h -= (uint128_t)M<<64;
    return h;
}


REGISTER_FAMILY(mwc128,
   $.src_url    = "https://github.com/AnatolyGeorgievski/MWC128/",
   $.src_status = HashFamilyInfo::SRC_ACTIVE
 );

REGISTER_HASH(MWC64_64,
   $.desc       = "64-bit MWC-64",
   $.hash_flags =
        0,
   $.impl_flags =
        FLAG_IMPL_MULTIPLY   |
        FLAG_IMPL_LICENSE_PUBLIC_DOMAIN,
   $.bits = 64,
   $.verification_LE = 0x3EBAAF43,
   $.verification_BE = 0x4B032B63,
   $.hashfn_native   = mwc64_hash<false>,
   $.hashfn_bswap    = mwc64_hash<true>
 );
REGISTER_HASH(MWC64_64__xor,
   $.desc       = "64-bit MWC-64",
   $.hash_flags =
        0,
   $.impl_flags =
        FLAG_IMPL_MULTIPLY   |
        FLAG_IMPL_LICENSE_PUBLIC_DOMAIN,
   $.bits = 64,
   $.verification_LE = 0x3EBAAF43,
   $.verification_BE = 0x4B032B63,
   $.hashfn_native   = mwc64_hash_xor<false>,
   $.hashfn_bswap    = mwc64_hash_xor<true>
 );
REGISTER_HASH(MWC64s_64,
   $.desc       = "64-bit MWC-64 signed",
   $.hash_flags =
        0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_64  |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN,
   $.bits = 32,
   $.verification_LE = 0x305E8D1D,
   $.verification_BE = 0x4B032B63,
   $.hashfn_native   = mwc64s_hash<false>,
   $.hashfn_bswap    = mwc64s_hash<true>
 );
REGISTER_HASH(MWC128_64,
   $.desc       = "64-bit MWC-128",
   $.hash_flags =
        0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_128  |
         FLAG_IMPL_ROTATE           |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN,
   $.bits = 64,
   $.verification_LE = 0x3C5C2C5C,
   $.verification_BE = 0x168D4939,
   $.hashfn_native   = mwc128_hash<64,false>,
   $.hashfn_bswap    = mwc128_hash<64,true>
 );
REGISTER_HASH(MWC128_128,
   $.desc       = "128-bit MWC-128",
   $.hash_flags =
        0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_128  |
         FLAG_IMPL_ROTATE           |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN,
   $.bits = 128,
   $.verification_LE = 0x21B4F453,
   $.verification_BE = 0xE6A1525F,
   $.hashfn_native   = mwc128_hash<128,false>,
   $.hashfn_bswap    = mwc128_hash<128,true>
 );
REGISTER_HASH(MWC192_128,
   $.desc       = "128-bit MWC-128",
   $.hash_flags =
        0,
   $.impl_flags =
         FLAG_IMPL_MULTIPLY_64_128  |
         FLAG_IMPL_ROTATE           |
         FLAG_IMPL_LICENSE_PUBLIC_DOMAIN,
   $.bits = 128,
   $.verification_LE = 0x305E8D1D,
   $.verification_BE = 0x4B032B63,
   $.hashfn_native   = mwc192_hash<128,false>,
   $.hashfn_bswap    = mwc192_hash<128,true>
 );
