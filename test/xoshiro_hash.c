/*! 
	[1] David Blackman and Sebastiano Vigna. 2018. Scrambled Linear Pseudorandom Number Generators. 
	3 May 2018, 41 pages. arxiv:1805.01407 To appear in ACM Transactions on Mathematical Software.
	[2] Austin Appleby. 2016. SMHasher. 8 Jan. 2016, https://github.com/aappleby/smhasher
	[3] Richard P. Brent. 2004. Note on Marsaglia’s Xorshift Random Number Generators. 
	Journal of Statistical Software, 11, 5, Aug., 1–5. coden:JSSOBK https://doi.org/10.18637/jss.v011.i05
	[4] Guy L. Steele Jr. and Sebastiano Vigna. 2021. LXM: better splittable pseudorandom number generators (and almost as fast). 
	Proc. ACM Program. Lang. 5, OOPSLA, Article 148 (October 2021), 31 pages. https://doi.org/10.1145/3485525
 */

#include <stdint.h>



static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}
// Update the XBG subgenerator (xoroshiro128v1_0)
static inline void xoroshiro128_next(uint64_t* s) {
	uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	s1 ^= s0;
	s0 = rotl(s0, 24);
	s[0] = s0 ^ s1 ^ (s1 << 16);
	s[1] = rotl(s1, 37);
}
/* 	Another structure for a PRNG is a very simple recurrence function 
	combined with a powerful output mixing function. This includes counter mode 
	block ciphers and non-cryptographic generators such as SplitMix64 */
/*  In 2014, Steele, Lea, and Flood presented SplitMix, 
	an object-oriented pseudorandom number generator (prng) that is quite fast 
	(9 64-bit arithmetic/logical operations per 64 bits generated) and also splittable.
 */
static inline uint64_t splitmix64(uint64_t *seed) {
    uint64_t z = (*seed += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}
/* David Stafford's mixing function
	http://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html

	Mixer parameters
MurmurHash3 mixer	33	0xff51afd7ed558ccd	33	0xc4ceb9fe1a85ec53	33

 Mixer	 Mixer operations
|Mix01	|31	|0x7fb5d329728ea185	|27	|0x81dadef4bc2dd44d	|33
|Mix02	|33	|0x64dd81482cbd31d7	|31	|0xe36aa5c613612997	|31
|Mix03	|31	|0x99bcf6822b23ca35	|30	|0x14020a57acced8b7	|33
|Mix04	|33	|0x62a9d9ed799705f5	|28	|0xcb24d0a5c88c35b3	|32
|Mix05	|31	|0x79c135c1674b9add	|29	|0x54c77c86f6913e45	|30
|Mix06	|31	|0x69b0bc90bd9a8c49	|27	|0x3d5e661a2a77868d	|30
|Mix07	|30	|0x16a6ac37883af045	|26	|0xcc9c31a4274686a5	|32
|Mix08	|30	|0x294aa62849912f0b	|28	|0x0a9ba9c8a5b15117	|31
|Mix09	|32	|0x4cd6944c5cc20b6d	|29	|0xfc12c5b19d3259e9	|32
|Mix10	|30	|0xe4c7e495f4c683f5	|32	|0xfda871baea35a293	|33
|Mix11	|27	|0x97d461a8b11570d9	|28	|0x02271eb7c6c4cd6b	|32
|Mix12	|29	|0x3cd0eb9d47532dfb	|26	|0x63660277528772bb	|33
|Mix13	|30	|0xbf58476d1ce4e5b9	|27	|0x94d049bb133111eb	|31
|Mix14	|30	|0x4be98134a5976fd3	|29	|0x3bc0993a5ad19a13	|31

*/
static inline uint64_t mix_stafford03(uint64_t h) {
  h ^= h >> 31;
  h *= 0x99bcf6822b23ca35;
  h ^= h >> 30;
  h *= 0x14020a57acced8b7;
  h ^= h >> 33;
  return h;
}
static inline uint64_t mix_stafford10(uint64_t h) {
  h ^= h >> 30;
  h *= 0xe4c7e495f4c683f5;
  h ^= h >> 32;
  h *= 0xfda871baea35a293;
  h ^= h >> 33;
  return h;
}
static inline uint64_t mix_stafford13(uint64_t h) {
  h ^= h >> 30;
  h *= 0xbf58476d1ce4e5b9ull;
  h ^= h >> 27;
  h *= 0x94d049bb133111ebull;
  h ^= h >> 31;
  return h;
}
// один шаг Lea's mix
static inline uint64_t fastmix(uint64_t x) {
	x *= 0xdaba0b6eb09322e3ull;
//    x *= 0xda942042e4dd58b5ULL;
    return x ^ (x>>32);
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
static inline uint64_t fastmix2(uint64_t x){
	x *= 0xbea225f9c5d17377ULL; 
	x ^= x >> 31; 
	x *= 0x94d049bb133111ebULL; 
	x ^= x >> 31;
	return x;
}

static inline uint64_t mwc128_next(uint64_t *s);
#define IV  0x9e3779b97f4a7c15u
#define PAD 0
#define STATE_SZ 2
uint64_t xoroshiro_hash(const uint8_t *data, uint64_t len, uint64_t seed) {
	uint64_t s[STATE_SZ];
	for (int i=0; i<STATE_SZ; i++)
		s[i] = unmix_lea(seed+=IV);
	for (int i=0; i<len>>3; i++) {
		s[0] ^= *(uint64_t*)data; data+=8;
		xoroshiro128_next(s);
	}
	if (len&7) {
		int r = len&7;
		uint64_t d = PAD;
		__builtin_memcpy(&d, data, r); data+=r;
		s[0] ^= d;
        xoroshiro128_next(s);
	}
 	return mix_lea(s[0]+s[1]);
}
//0xfffecd60, 0xfffeb81b, 0xfffe7369,0xfffe636a, 0xfffe59a7, 0xfffe29b9,0xfffe1d0b
#define MWC_A0 (uint64_t)0xfffeb81b //    const uint64_t P = (MWC_A0<<32)-1;
uint64_t mwc64_hash(const uint8_t* data, uint64_t len, uint64_t seed0){
	uint64_t hash = seed0+(0x9e3779b97f4a7c15<<32);
	hash = ((uint32_t)hash)*MWC_A0 + (hash>>32);
    int i;
    for (i=0; i<len>>2; i++){
        hash+= *(uint32_t*) data; data+=4;
        hash = ((uint32_t)hash)*MWC_A0 + (hash>>32);
    }
	if (len&3) {
		int s = len&3;
		uint32_t d = 0x01020304u;
		__builtin_memcpy(&d, data, s); data+=s;
		hash+= d;
		hash = ((uint32_t)hash)*MWC_A0 + (hash>>32);
	}
    return mix_stafford13(hash);
}


// xxh64_hash -- быстрая
typedef uint64_t uint64x4_t __attribute__((__vector_size__(32)));

typedef struct _HashCtx HashCtx_t;
struct _HashCtx {
    uint64_t state[4];
    uint8_t  block[32];
    int offset;
};
static const uint64_t Prime1 = 0x9E3779B185EBCA87ULL;
static const uint64_t Prime2 = 0xC2B2AE3D27D4EB4FULL;
static const uint64_t Prime3 = 0x165667B19E3779F9ULL;
static const uint64_t Prime4 = 0x85EBCA77C2B2AE63ULL;
static const uint64_t Prime5 = 0x27D4EB2F165667C5ULL;

#define ROTL64(x, n) ((x)<<n | (x)>>(64-n))
static inline uint64_t ROUND64(uint64_t x){
    return ROTL64(x*Prime2,31)*Prime1;
}
static inline uint64_t MERGE64(uint64_t hash, uint64_t x){
    return (hash ^ ROUND64(x))*Prime1 + Prime4;
}
// Миксер avalanche от xxHash64
static inline uint64_t avalanche(uint64_t hash){
    hash ^= hash >> 33;
    hash *= Prime2;
    hash ^= hash >> 29;
    hash *= Prime3;
    hash ^= hash >> 32;
    return hash;
}
uint64_t xxh64_hash(uint8_t* data, uint64_t data_len, uint64_t seed0)
{
	uint64_t hash = seed0; 
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
	return avalanche(hash);
}
