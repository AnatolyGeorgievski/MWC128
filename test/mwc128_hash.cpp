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
static inline uint64_t lehmer64_mix(uint64_t *s) {
    uint128_t x = *(uint128_t*)s;
    x = x * UINT64_C(0xda942042e4dd58b5) - UINT64_C(0x9e3779b97f4a7c15);   // фиксированный множитель (хороший 64-бит → 128-бит)
    return x^(x>>64);   // берём старшие 64 бита как результат
}
//typedef unsigned int __attribute__((mode(TI)))   uint128_t;
#define MWC_A1 		(uint128_t)0xffebb71d94fcdaf9ull
#define IV 	0x9e3779b97f4a7c15u
#define PAD 0x0102030405060708u
#define STATE_SZ 2
#define unmix unmix_lea
#define mix mix_lea
static inline uint64_t _mum( uint64_t A, uint64_t B) {
    uint64_t rlo, rhi;
    MathMult::mult64_128(rlo, rhi, A, B);
    return rlo ^ rhi;
}

static inline void mwc128_next(uint64_t* state, uint64_t d, int r) {
	uint128_t  t =*(uint128_t*)state + d;
	t = (uint128_t)MWC_A1 * ((uint64_t)t<<(64-r)) + (t>>r);
    *(uint128_t*)state = t;
}
template <bool bswap>
void mwc128_hash(const void *in, size_t len, uint64_t seed, void* out) {
    const uint8_t* data = (const uint8_t*)in;
	uint64_t s[STATE_SZ] = {IV, 1};
    s[0] = _mum(seed += IV, UINT64_C(0x82d2e9550235efc5));
    s[1] = _mum(seed += IV, UINT64_C(0x82d2e9550235efc5));
    unsigned int blocks = (len>>3);
	for (unsigned int i=0; i<blocks; i++){
		uint64_t d = GET_U64<false>(data, 0); data+=8;
		mwc128_next(s, d, 64);
	}
	int r = len&7;
	if (r) {
        uint64_t d = GET_U64<false>(data, 0);
        d &= ~0uLL>>(64-r*8);
        mwc128_next(s, d, r*8);
    } 
    uint64_t d = _mum(s[0]^s[1], UINT64_C(0xa3b195354a39b70d))-IV;
    PUT_U64<bswap>(d, (uint8_t *)out,  0);
}
#define MWC_A2 0xffa04e67b3c95d86u // MWC192, B2 = 1<<128
static inline void mwc192_next(uint64_t* state) {
	const uint128_t t = (uint128_t)MWC_A2 * state[0] + state[1];
	state[0] = state[2];
	state[1] = t >> 64;
	state[2] = t;
}
template <bool bswap>
void mwc192_hash(const void *in, size_t len, seed_t seed, void* out) {
    const uint8_t* data = (const uint8_t*)in;
	uint64_t s[3];
	uint64_t d;
	for (int i=0; i<3; i++)
		s[i] = unmix(seed+=IV);
    unsigned int blocks = (len>>3);
	for (unsigned int i=0; i<blocks; i++){
		s[0] ^= (*(uint64_t*) data); data+=8;
		mwc192_next(s);
	}
	d = PAD;
	int r = len&7;
	if (r) {
		__builtin_memcpy(&d, data, r); data+=r;
    }
	s[0] ^= (d);
	mwc192_next(s);
	d = mix(s[1])-IV;
    PUT_U64<bswap>(d, (uint8_t *)out,  0);
	mwc192_next(s);
	d = mix(s[1])-IV;
    PUT_U64<bswap>(d, (uint8_t *)out,  8);
}
#define MWC_A  0x7ff8c871
static inline int64_t next(int64_t x, int r){
    return (x>>r) - ((uint32_t)x<<(32-r))*(int64_t)MWC_A;
}
template <bool bswap>
void mwc64s_hash(const void* in, size_t len, uint64_t seed, void* out){
    const uint8_t* data = (const uint8_t*)in;
    int64_t hash = (int64_t)unmix_lea(seed+IV);
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
#define MWC_A0  0xfffe59a7uLL//eb81bULL
#define MWC_INV 0x0001A65BB8CE0887u
#define MWC_PRIME ((MWC_A0<<32) -1)
static inline uint64_t _next(uint64_t x, int r){
    return ((uint32_t)x<<(32-r))*MWC_A0 + (x>>r);
}
template <bool bswap>
void mwc64_hash(const void* in, size_t len, uint64_t seed, void* out){
    const uint8_t* data = (const uint8_t*)in;
    int64_t hash = (int64_t)unmix_lea(seed+IV);
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
    PUT_U64<bswap>(d, (uint8_t *)out,  0);
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
   $.verification_LE = 0xBD4A9ED0,
   $.verification_BE = 0x9B779ACF,
   $.hashfn_native   = mwc128_hash<false>,
   $.hashfn_bswap    = mwc128_hash<true>
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
   $.hashfn_native   = mwc192_hash<false>,
   $.hashfn_bswap    = mwc192_hash<true>
 );
