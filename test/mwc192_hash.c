/*! __Анатолий М. Георгиевский, ИТМО__, 2026
 */

#include <stdint.h>
typedef unsigned int __attribute__((mode(TI)))   uint128_t;

// Doug Lea's mixing function, fastmix дважды
static inline uint64_t mix_lea(uint64_t h) {
  h ^= h >> 32;
  h *= 0xdaba0b6eb09322e3ull;
  h ^= h >> 32;
  h *= 0xdaba0b6eb09322e3ull;
  h ^= h >> 32;
  return h;
}
static inline uint64_t unmix_lea(uint64_t h) {
  h ^= h >> 32;
  h *= 0xa6f8e26927e132cb;
  h ^= h >> 32;
  h *= 0xa6f8e26927e132cb;
  h ^= h >> 32;
  return h;
}
static inline uint64_t mix64(uint64_t x){
	x  =  (x ^ (x >> 33)) * 0xff51afd7ed558ccdu;
	x  =  (x ^ (x >> 33)) * 0xc4ceb9fe1a85ec53u;
	return x ^ (x >> 33);
}
static uint64_t unmix64(uint64_t z) {
	z  =  (z ^ (z >> 33)) * 0x9cb4b2f8129337dbL;
	z  =  (z ^ (z >> 33)) * 0x4f74430c22a54005L;
	return z ^ (z >> 33); 
}
static inline uint64_t mix_stafford13(uint64_t h) {
  h ^= h >> 30;
  h *= 0xbf58476d1ce4e5b9ull;
  h ^= h >> 27;
  h *= 0x94d049bb133111ebull;
  h ^= h >> 31;
  return h;
}
static inline uint64_t unmix_stafford13(uint64_t h) {
  h ^= h >> 31;
  h ^= h >> 62;
  h *= 0x319642B2D24D8EC3ull;
  h ^= h >> 27;
  h ^= h >> 54;
  h *= 0x96de1b173f119089ull;
  h ^= h >> 30;
  h ^= h >> 60;
  return h;
}
#define IV 	0x9e3779b97f4a7c15u
#define PAD 0//0x0102030405060708u
#define STATE_SZ 3
#define unmix unmix_lea
#define mix mix_lea
#define MWC_A2 0xffa04e67b3c95d86u // MWC192, B2 = 1<<128


#pragma GCC optimize("unroll-loops")
//#pragma GCC optimize("omit-frame-pointer")

static inline void mwc192_next(uint64_t* state) {
	const uint128_t t = (unsigned __int128)MWC_A2 * state[0] + state[2];
	state[0] = state[1];
	state[1] = t;
	state[2] = t >> 64;
}
static inline void mwc192_align(uint64_t* state, int r) {

	const uint128_t t = ((unsigned __int128)MWC_A2) * (state[0]<<(64-r*8)) + (state[1]>>(r*8))+((uint128_t)state[2]<<(64-r*8));
	state[0] = state[1];
	state[1] = t;
	state[2] = t >> 64;
}
void mwc192_hash(const uint8_t *data, uint64_t len, uint64_t seed, uint64_t* result) {
	uint64_t s[STATE_SZ];
	for (int i=0; i<STATE_SZ; i++)
		s[i] = unmix(seed+=IV);
	for (int i=0; i<len>>3; i++){
		uint64_t d = (*(uint64_t*) data); data+=8;
		s[0] ^= d;
		mwc192_next(s);
	}
	if (len&7) {
		int r = len&7;
		uint64_t d = PAD;
		__builtin_memcpy(&d, data, r); data+=r;
		s[0]^= (d);
		mwc192_align(s, r);
	}
	mwc192_next(s);
	result[0] = mix(s[0]^s[1]);
	mwc192_next(s);
	result[1] = mix(s[0]^s[1]);
	// mwc192_next(s);
	// result[2] = mix(s[0]^s[1]);
	// mwc192_next(s);
	// result[3] = mix(s[0]^s[1]);
}
