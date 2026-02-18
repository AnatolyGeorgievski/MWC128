/*! __Анатолий М. Георгиевский, ИТМО__, 2026

	[1] David Blackman and Sebastiano Vigna. 2018. Scrambled Linear Pseudorandom Number Generators. 
	3 May 2018, 41 pages. arxiv:1805.01407 To appear in ACM Transactions on Mathematical Software.
	[4] Guy L. Steele Jr. and Sebastiano Vigna. 2021. LXM: better splittable pseudorandom number generators (and almost as fast). 
	Proc. ACM Program. Lang. 5, OOPSLA, Article 148 (October 2021), 31 pages. https://doi.org/10.1145/3485525
 */

#include <stdint.h>
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
static inline uint64_t mix_stafford07(uint64_t h) {
  h ^= h >> 30;
  h *= 0x16a6ac37883af045;
  h ^= h >> 26;
  h *= 0xcc9c31a4274686a5;
  h ^= h >> 32;
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
static inline uint64_t unmix_stafford10(uint64_t h) {
  h ^= h >> 33;
  h *= 0x02054AEE6574CB9Bull;
  h ^= h >> 32;
  h *= 0x49e0439cd61fd05dull;
  h ^= h >> 30;
  h ^= h >> 60;
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
static inline uint64_t splitmix64(uint64_t z) {
//    uint64_t z = (*seed += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}
static inline uint64_t unsplitmix64(uint64_t z) {
	z ^= (z >> 31);
	z ^= (z >> 62);
    z *= 0x319642B2D24D8EC3;
	z ^= (z >> 27);
	z ^= (z >> 54);
    z *= 0x96de1b173f119089;
	z ^= (z >> 30);
	z ^= (z >> 60);
//  (*seed -= 0x9e3779b97f4a7c15);
    return z;
}

// один шаг Lea's mix
static inline uint64_t fastmix(uint64_t x) {
	x *= 0xdaba0b6eb09322e3ull;
//    x *= 0xda942042e4dd58b5ULL;
    return x ^ (x>>32);
}
static inline uint64_t unfastmix(uint64_t x) {
	x = (x ^ (x>>32)) * 0xa6f8e26927e132cb;
    return x;
}

static inline uint64_t fastmix2(uint64_t x){
	x *= 0xbea225f9c5d17377ULL; 
	x ^= x >> 31; 
	x *= 0x94d049bb133111ebULL; 
	x ^= x >> 31;
	return x;
}

static inline uint64_t wyfinal(uint64_t h) {
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdull;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ull;
    h ^= h >> 33;
    return h;
}
// MurmurHash3 64-bit avalanche mixer
// http://dx.doi.org/10.1145/2714064.2660195
// Original MurmurHash3 implementation: https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
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
#define MWC_A1 		(uint128_t)0xffebb71d94fcdaf9ull // MWC128  число A1, B1 = (1<<64)
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
#define IV 	0x9e3779b97f4a7c15u
#define PAD 0//0x0102030405060708u
#define STATE_SZ 2
#define unmix unmix_lea
#define mix mix_lea
static inline void mwc128_next(uint64_t* state) {
	const unsigned __int128 t = (unsigned __int128)MWC_A1 * state[0] + state[1];
	state[0] = t;
	state[1] = t >> 64;
}
static inline void mwc128_align(uint64_t* state, int r) {
	const unsigned __int128 t = ((unsigned __int128)MWC_A1) * (state[0]<<(64-r*8)) + (state[1]>>(r*8));
	state[0] = t;
	state[1] = t >> 64;
}
void mwc128_hash(const uint8_t *data, uint64_t len, uint64_t seed, uint64_t* result) {
	uint64_t s[STATE_SZ];
	for (int i=0; i<STATE_SZ; i++)
		s[i] = unmix(seed+=IV);
	for (int i=0; i<len>>3; i++){
		uint64_t d = (*(uint64_t*) data); data+=8;
		s[0] ^= d;
		mwc128_align(s,8);
	}
	if (len&7) {
		int r = len&7;
		uint64_t d = PAD;
		__builtin_memcpy(&d, data, r); data+=r;
		s[0]^= (d);
		mwc128_align(s,r);
	}
	result[0] = mix(s[0]^s[1]);
}
#if defined(TEST_MWC128_HASH)
uint64_t inverse_uint64(uint64_t a) {
    uint64_t x = a;
    
    // 5 итераций — стандарт для 64 бит
    x *= 2ULL - a * x;
    x *= 2ULL - a * x;
    x *= 2ULL - a * x;
    x *= 2ULL - a * x;
    x *= 2ULL - a * x;
    
    return x;
}
uint64_t reverse_xor_shift(uint64_t h, unsigned int m) {
    h ^= (h >> m);
    if (2*m < 64) h ^= (h >> (2*m));
    if (4*m < 64) h ^= (h >> (4*m));
//	. . .
    return h;
}
#include <stdio.h>
int main(){
	uint64_t x = 0x123456789abcdefe;
	uint64_t h = x;
	h ^= (h >> 33);
	h ^= (h >> 33);
	h = unmix64(mix64(h));
	printf("h=%016llx\n",x);
	printf("Mix64:   h=%016llx\n",h);
	h = unmix_stafford10(mix_stafford10(x));
	printf("Mix10:   h=%016llx\n",h);
	h = unmix_stafford13(mix_stafford13(x));
	printf("Mix13:   h=%016llx\n",h);
	h = unfastmix(fastmix(x));
	printf("FastMix: h=%016llx\n",h);
	h = unmix_lea(mix_lea(x));
	printf("Mix_Lea: h=%016llx\n",h);
	h = unsplitmix64(splitmix64(x));
	printf("SplitMix:h=%016llx\n",h);

// 0xff51afd7ed558ccdL * 0x4f74430c22a54005L == 1 
// 0xc4ceb9fe1a85ec53L * 0x9cb4b2f8129337dbL == 1
	uint64_t b = 0xff51afd7ed558ccd;
	uint64_t ib = inverse_uint64(b);
	printf("b=%016llx ~b=%016llx %d\n",b, ib, (uint64_t)(b*ib));
	b = 0xc4ceb9fe1a85ec53;
	ib = inverse_uint64(b);
	printf("b=%016llx ~b=%016llx %d\n",b, ib, (uint64_t)(b*ib));
	b = 0xdaba0b6eb09322e3;
	ib = inverse_uint64(b);
	printf("b=%016llx ~b=%016llx %d\n",b, ib, (uint64_t)(b*ib));
if (1){// синтез констант для обратного unMix64
	struct _op {
		const char* name;
		int a;
		uint64_t prime1;
		int b;
		uint64_t prime2;
		int c;
	} op[] = {
{"MurmurHash3", 33, 0xff51afd7ed558ccd, 33, 0xc4ceb9fe1a85ec53, 33},
{"Lea's",       32, 0xdaba0b6eb09322e3, 32, 0xdaba0b6eb09322e3, 32},
{"SplitMix64",  30, 0xbf58476d1ce4e5b9, 27, 0x94d049bb133111eb, 31},
{"Avalanche",   33, 0xC2B2AE3D27D4EB4F, 29, 0x165667B19E3779F9, 32},

	{"Mix01",31 ,0x7fb5d329728ea185 ,27 ,0x81dadef4bc2dd44d ,33},
	{"Mix02",33	,0x64dd81482cbd31d7	,31	,0xe36aa5c613612997	,31},
	{"Mix03",31	,0x99bcf6822b23ca35	,30	,0x14020a57acced8b7	,33},
	{"Mix04",33	,0x62a9d9ed799705f5	,28	,0xcb24d0a5c88c35b3	,32},
	{"Mix05",31	,0x79c135c1674b9add	,29	,0x54c77c86f6913e45	,30},
	{"Mix06",31	,0x69b0bc90bd9a8c49	,27	,0x3d5e661a2a77868d	,30},
	{"Mix07",30	,0x16a6ac37883af045	,26	,0xcc9c31a4274686a5	,32},
	{"Mix08",30	,0x294aa62849912f0b	,28	,0x0a9ba9c8a5b15117	,31},
	{"Mix09",32	,0x4cd6944c5cc20b6d	,29	,0xfc12c5b19d3259e9	,32},
	{"Mix10",30	,0xe4c7e495f4c683f5	,32	,0xfda871baea35a293	,33},
	{"Mix11",27	,0x97d461a8b11570d9	,28	,0x02271eb7c6c4cd6b	,32},
	{"Mix12",29	,0x3cd0eb9d47532dfb	,26	,0x63660277528772bb	,33},
	{"Mix13",30	,0xbf58476d1ce4e5b9	,27	,0x94d049bb133111eb	,31},
	{"Mix14",30	,0x4be98134a5976fd3	,29	,0x3bc0993a5ad19a13	,31},
	};
	uint64_t C1,C2;
	int sz = sizeof(op)/sizeof(struct _op);
	for (int i=0; i< sz; i++){
		C1 = inverse_uint64(op[i].prime1);
		C2 = inverse_uint64(op[i].prime2);
		// printf("P1=0x%016llX: ~P1= 0x%016llX %d\t", op[i].prime1, C1, op[i].prime1*C1);
		// printf("P2=0x%016llX: ~P2= 0x%016llX %d\n", op[i].prime2, C2, op[i].prime2*C2);
		printf("|un%s | |0x%016llx | |0x%016llX |\n", op[i].name, C1,C2);
	}
}

	return 0;
}
#endif