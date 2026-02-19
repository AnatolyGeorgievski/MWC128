/*! __Анатолий М. Георгиевский, ИТМО__, 2026
 */

#include <stdint.h>
typedef unsigned int __attribute__((mode(TI)))   uint128_t;
static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

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
/*!
L. Hars, G. Petruska: “Pseudorandom Recursions II.” EURASIP Journal
on Embedded Systems 2012, 2012:1 doi:10.1186/1687-3963-2012-1. 
 */
static inline uint64_t mix_rax(uint64_t x) {
	const int L=4,R=9;
	x+= 0x3779884922721DEBu;
	x = (x ^ rotl(x,L) ^ rotl(x,R)) + 0x49A8D5B36969F969u;
	x = (x ^ rotl(x,L) ^ rotl(x,R)) + 0x6969F96949A8D5B3u;
	x = (x ^ rotl(x,L) ^ rotl(x,R));
	return x;
}
#define IV 	0x9e3779b97f4a7c15u
#define PAD 0//x0102030405060708u
#define STATE_SZ 4
#define unmix unmix_lea
#define mix mix_lea
#define MWC_A3 0xfff62cf2ccc0cdafu // MWC256
//#define MWC_A3 0xff377e26f82da74au
static inline uint64_t mwc256_next(uint64_t* state) {
	const uint128_t t = (unsigned __int128)MWC_A3 * state[0] + state[3];
	state[0] = state[1];
	state[1] = state[2];
	state[2] = t;
	state[3] = t >> 64;
}
static inline void mwc256_align(uint64_t* state, int r) {
	const uint128_t t = (unsigned __int128)MWC_A3 * (state[0]<<(64-(r*8))) + (state[3]>>(r*8));
	state[0] = state[1];
	state[1] = state[2];
	state[2] = t;
	state[3] = t >> 64;
}
uint64_t mwc256_hash(const uint8_t *data, uint64_t len, uint64_t seed, uint64_t* result) {
	uint64_t s[STATE_SZ];
	for (int i=0; i<STATE_SZ; i++)
		s[i] = unmix(seed+=IV);
	for (int i=0; i<len>>3; i++){
		uint64_t d = (*(uint64_t*) data); data+=8;
		s[0] += d;
		mwc256_next(s);
	}
	if (len&7) {
		int r = len&7;
		uint64_t d = PAD;
		__builtin_memcpy(&d, data, r); data+=r;
		s[0]+= (d);
		mwc256_align(s, r);
	}
	mwc256_next(s);
	mwc256_next(s);
	result[0] = mix(s[0]^s[1]);
}
