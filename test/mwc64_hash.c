
/*! __Анатолий М. Георгиевский__, ИТМО, 2026 
    MWC64-hash -- хэш-функция на базе генератора случайных чисел
    Состоит из генератора ГПСЧ и Avalanche-миксера. Миксер - обратимая функция.
    Генератор `next()` сдвигает заданное число бит в младшей части числа. 
    Функция `next()` может выдавать от 1-32 бит из потока ГПСЧ.

    Хэш можно дописывать и считать параллельно по сегментам.

Сборка тестирование:
    $ gcc -DTEST_MWC64_HASH -o test test/mwc64_hash.c
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
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
// Doug Lea's mixing function, fastmix дважды
static inline uint64_t mix_lea(uint64_t h) {
  const uint64_t M = 0xdaba0b6eb09322e3;
  h ^= h >> 32;
  h *= M;
  h ^= h >> 32;
  h *= M;
  h ^= h >> 32;
  return h;
}
static inline uint64_t unmix_lea(uint64_t h) {
  const uint64_t M = 0xa6f8e26927e132cb;
  h ^= h >> 32;
  h *= M;
  h ^= h >> 32;
  h *= M;
  h ^= h >> 32;
  return h;
}
// https://jonkagstrom.com/mx3/index.html
// https://github.com/jonmaiga/mx3 -- 33/28/39
static inline uint64_t mix3(uint64_t x) {
    const uint64_t M = 0xbea225f9eb34556d;
    x ^= x >> 32;
    x *= M;
    x ^= x >> 29;
    x *= M;
    x ^= x >> 32;
    x *= M;
    x ^= x >> 29;
    return x;
}
static inline uint64_t unmix3(uint64_t x) {
    const uint64_t M = 0xdd01f46a7e6ffc65;
    x ^= x >> 29;
    x ^= x >> 58;
    x *= M;
    x ^= x >> 32;
    x *= M;
    x ^= x >> 29;
    x ^= x >> 58;
    x *= M;
    x ^= x >> 32;
    return x;
}
static inline uint64_t mix_mxmxmx(uint64_t x) {
    const uint64_t M = 0xbea225f9eb34556d;
	x *= M;
	x ^= x >> 33;
	x *= M;
	x ^= x >> 29;
	x *= M;
	x ^= x >> 39;
	return x;
}
static inline uint64_t murmur64(uint64_t z) {
  z ^= (z >> 33);
  z *= 0xff51afd7ed558ccdull;
  z ^= (z >> 33);
  z *= 0xc4ceb9fe1a85ec53ull;
  return z ^ (z >> 33);
}
static inline uint64_t degski64(uint64_t z) {
  z ^= (z >> 32);
  z *= 0xd6e8feb86659fd93ull;
  z ^= (z >> 32);
  z *= 0xd6e8feb86659fd93ull;
  return z ^ (z >> 32);
}
static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}
static inline uint64_t rotr(const uint64_t x, int k) {
	return (x << (64 - k)) | (x >> k);
}
// https://mostlymangling.blogspot.com/2018/07/
// rotl(s0 * 5, 7) * 9;
static inline uint64_t rrmxmx(uint64_t v) {
    v ^= rotr(v, 49) ^ rotr(v, 24);
    v *= 0x9FB21C651E98DF25u;
    v ^= v >> 28;
    v *= 0x9FB21C651E98DF25u;
    return v ^ v >> 28;
}
#define mix mix_lea//mix_stafford13 -- прямая функция Avalanche mixer
#define unmix unmix_lea//unmix_stafford13 -- обратная
#define IV 	0x9e3779b97f4a7c15u
//#define MWC_A0 0xfffeb81bULL
#define MWC_A0  0xfffe59a7uLL//eb81bULL
#define MWC_INV 0x0001A65BB8CE0887u
#define MWC_PRIME ((MWC_A0<<32) -1)
#define MWC_MINUS_PRIME ((uint64_t)(-(MWC_PRIME)))
static inline uint64_t next(uint64_t x, int r){
    return ((uint32_t)x<<(32-r))*MWC_A0 + (x>>r);
}
typedef unsigned __int128 uint128_t;
static inline uint64_t  mwc_mod  (uint128_t ac, uint64_t M, uint64_t M_INV);
static inline uint128_t mwc_foldm(uint128_t h, uint128_t d, const uint64_t j1, const uint64_t j2, const uint64_t M);
static inline uint128_t mwc_merge(uint128_t h, uint128_t d, const uint64_t j1, const uint64_t j2, const uint64_t M);


#define SEGMENT_SIZE 2048
static uint128_t mwc64_hash_segment(uint128_t hash, const uint8_t *data);
uint64_t mwc64_hash(uint64_t hash, const uint8_t* data, size_t data_len){
    static const uint64_t j64  = UINT64_C(0xFFFCB350B8C98AF1);
    static const uint64_t j_64 = UINT64_C(0x00034CAF4736750F);
    static const uint64_t j96  = UINT64_C(0xB8C85A1586E21F87);
    static const uint64_t j128 = UINT64_C(0x86E141001432DA26);
    static const uint64_t j192 = UINT64_C(0xCE7CDBEB087AEFCE);
    static const uint64_t j256 = UINT64_C(0x4E5429F8166590FE);
    hash = unmix(hash+IV);
    int i=0;
    if (1 && data_len>=16) {
        uint128_t h = (uint128_t)hash;

        if (1 && data_len>=SEGMENT_SIZE){
            data_len-=SEGMENT_SIZE;
            h = mwc64_hash_segment(h, data);
            data+=SEGMENT_SIZE;
        }

        while (data_len>=8) {
            uint64_t d0 = (*(uint64_t*) data); data+=8; 
            h+= d0;
//            h = (h>>64 | h<<64) - (uint64_t)h * (uint128_t)j_64;// round mix 64
            h = (h>>64) + (uint64_t)h * (uint128_t)j64;// round mix 64
            data_len -= 8;
        }
        int r = data_len &= 7;
        if (r) {
            uint64_t d0 = (*(uint64_t*) data); data+=8;
            d0 &= ~0uLL>>(64-r*8);
            h+=d0;
            h = (h>>(r*8)) + ((uint64_t)h<<(64-r*8)) * (uint128_t)j64;
        }
        hash = mwc_mod(h, MWC_PRIME, MWC_INV);
    } else {
        for (i=0; i<data_len>>2; i++)
        {
            hash+= *(uint32_t*) data; data+=4;
            hash = next(hash, 32);
        }
        if (data_len&2){
            hash+= *(uint16_t*) data; data+=2;
            hash = next(hash, 16);
        }
        if (data_len&1){
            hash+= *(uint8_t*) data; data+=1;
            hash = next(hash, 8);
        }
    }
    return mix(hash)-IV;
}
/*! \brief Модульное сложение с неполным редуцированием */
static inline uint64_t mwc_add(uint64_t a, uint64_t b, uint64_t M){
    if (__builtin_add_overflow(a, b, &a))
        a -= M;
    return a;
}
/*! \brief Модульное сложение с редуцированием по модулю */
static inline uint64_t mwc_addm(uint64_t a, uint64_t b, uint64_t M){
    a += b;
    if (a<b || a>=M) 
        a -= M;
    return a;
}

/*! \brief Модульное умножение с неполным редуцированием */
static inline uint64_t mwc_mulm(uint64_t a, uint64_t b, uint64_t M)
{	
	unsigned __int128 ac;
	ac = (unsigned __int128)a*b;
	ac-= (((ac>>64)*MWC_INV + ac)>>64)*M;
	if (ac>>64) ac -= M;
//	if ((uint64_t)ac>= M) ac -= M;
	return ac;
}
/*! \brief модульная операция с отложенным редуцированием */
static inline uint64_t mwc_mod(uint128_t ac, uint64_t M, uint64_t M_INV) {	
	ac-= (((ac>>64)*M_INV + ac)>>64)*M;
	if (ac>>64) ac -= M;
	return ac;
}
/*! \brief Модульное умножение со сложением и неполным редуцированием */
static inline uint64_t mwc_maddm(uint64_t a, uint64_t b, uint64_t c, uint64_t M)
{	
	unsigned __int128 ac;
	ac = (unsigned __int128)a*b + c;
	ac-= (((ac>>64)*MWC_INV + ac)>>64)*M;
	if (ac>>64) ac -= M;
//	if ((uint64_t)ac>= M) ac -= M;
	return ac;
}
/*! \brief Модульное "схлопывание" folding, используются две сдвиговые константы на 
    дистанцию w1={64*n} и w2={64*n - 64}
   ${x, c} = x\cdot j_1 + c\cdot j_2$ аналогично операции folding в CRC

   Редуцирование по M = 2^{64} - 2^{32} +1, 
    {x,c} => x + ({c<<32} - c)
   */
static inline uint128_t mwc_foldm(uint128_t h, uint128_t d, const uint64_t j1, const uint64_t j2, const uint64_t M){
    if (__builtin_add_overflow(h, d, &h)) {
        h -= (uint128_t)M<<64;
        printf("$");
    }
    uint128_t t;
    t = (uint64_t)h * (uint128_t)j2;// k
    h = (h>>64) * (uint128_t)j1; // k-64
    if (__builtin_add_overflow(h, t, &h)){
        h -= (uint128_t)M<<64;
        printf("%%");
    }
    return h;
}
/*! \brief выравнивание и слияние элементов вектора */ 
static inline uint128_t mwc_merge(uint128_t h, uint128_t d, const uint64_t j1, const uint64_t j2, const uint64_t M){
    uint128_t t;
    t = (uint64_t)h * (uint128_t)j2;// k
    h = (h>>64) * (uint128_t)j1; // k-64
    if (__builtin_add_overflow(h, t, &h))
        h -= (uint128_t)M<<64;
    if (__builtin_add_overflow(h, d, &h))
        h -= (uint128_t)M<<64;
    return h;
}

/*! Слияние хешей двух сегментов с коррекцией переполнения
    по модулю MWC_PRIME
 */
uint64_t mwc64_merge(uint64_t h1, uint64_t h2, uint64_t jump){
    h1 = unmix(h1+IV);
    h2 = unmix(h2+IV);
    if (jump)
        h1 = mwc_maddm(h1, jump, h2, MWC_PRIME);
    else
        h1 = mwc_addm(h1, h2, MWC_PRIME);
    return mix(h1)-IV;
}

#define J_64 UINT64_C(0x0001A65900000001)
#define J_128 UINT64_C(0xE1B1274AB8CE0887)

#define J0   UINT64_C(1)
#define J64  UINT64_C(0xFFFCB350B8C98AF1)
#define J128 UINT64_C(0x86E141001432DA26)
#define J192 UINT64_C(0xAD97A763E40AF999)
#define J256 UINT64_C(0x4E5429F8166590FE)
static uint128_t mwc64_hash_segment(uint128_t hash, const uint8_t *data){
    int data_len = SEGMENT_SIZE;
    uint128_t h[2] = {hash};
    if (0) {
        while (data_len>=16) {
            h[0]+= (*(uint64_t*) data); data+=8; 
            h[0] = (h[0]>>64)*(uint128_t)J64 + (uint64_t)h[0] * (uint128_t)J128;// round mix 64
            h[1]+= (*(uint64_t*) data); data+=8; 
            h[1] = (h[1]>>64)*(uint128_t)J64 + (uint64_t)h[1] * (uint128_t)J128;// round mix 64
            data_len -= 16;
        }
        h[1] = (h[1]>>64)*(uint128_t)J_128 + (uint64_t)h[1] * (uint128_t)J_64;
        if (__builtin_add_overflow(h[0],h[1],&h[0])) h[0] -= (uint128_t)MWC_PRIME<<64;
//        if ((h[0]>>64)>=MWC_PRIME) h[0] -= (uint128_t)MWC_PRIME<<64;
        h[0] = mwc_mod(h[0],MWC_PRIME, MWC_INV);
    }
    while (data_len>=16) {
        data_len-=16;
        uint128_t d = (*(uint128_t*) data); data+=16; 
//        d = (uint64_t)d + (d>>64)*(uint128_t)MWC_MINUS_PRIME;// ленивое редуцирование
        h[0] = mwc_foldm(h[0], d, J64, J128, MWC_PRIME);
    }
//        h[0] = mwc_foldm(h[0], 0, J_128, J_64, MWC_PRIME);

    // uint64_t h1 = mwc_mod(h[1], MWC_PRIME, MWC_INV);
    // uint64_t h0 = mwc_mod(h[0], MWC_PRIME, MWC_INV);
    // h0 = mwc_maddm(h1, J_64, h0, MWC_PRIME);
    // return h0;

    // h[1] = (h[1]>>64)*(uint128_t)J_128 + (uint64_t)h[1] * (uint128_t)J_64;
    // if (__builtin_add_overflow(h[0],h[1],&h[0])) h[0] -= (uint128_t)MWC_PRIME<<64;
    return h[0];
#if 0
    uint128_t h[4] = {hash};
    for(int i=0; i<SEGMENT_SIZE; i+=32, data+=32) {
        for(int k=0; k<4; k++){
            uint64_t d = *(uint64_t*)(data+8*k);
            h[k] = mwc_foldm(h[k], d, J192, J256, MWC_PRIME);
        }
    }
    h[0] = mwc_merge(h[0],h[2], J64, J128, MWC_PRIME);
    h[1] = mwc_merge(h[1],h[3], J64, J128, MWC_PRIME);
    h[0] = mwc_merge(h[0],h[1], J0,  J64,  MWC_PRIME);
    return h[0];
#endif
}

/*! \brief Модульнуя операция возведения в степень с неполным редуцированием */
static uint64_t mwc_powm(uint64_t a, uint64_t e, uint64_t M)
{
	uint64_t sqr=a, acc=1;
	while(e!=0){
		if(e&1)
			acc=mwc_mulm(acc,sqr,M);
		sqr=mwc_mulm(sqr,sqr,M);
		e>>=1;
	}
	if (acc>=M) acc -= M;// отложенное редуцирование
	return acc;
}
/*! \brief пропуск сегмента, дистанция задается по числу байт 
    Максимальный период мультипликативной группы (Prime -1), 
    период может быть меньше, например (Prime-1)/w
 */
uint64_t mwc64_skip(uint64_t hash, uint64_t distance) {
    hash = unmix(hash+IV);
	uint64_t m=mwc_powm(2, MWC_PRIME-1-(distance<<3), MWC_PRIME);
	uint64_t x=mwc_mulm(hash, m, MWC_PRIME);
	return mix(x)-IV;
}
uint64_t mwc64_jump(uint64_t hash, uint64_t jump) {
    hash = unmix(hash+IV);
	uint64_t x=mwc_mulm(hash, jump, MWC_PRIME);
	return mix(x)-IV;
}
/*! \brief референсная реализация */
uint64_t mwc64_hash_8(uint64_t hash, uint8_t* data, size_t data_len){
    hash = unmix(hash+IV);
    for (int i=0; i<data_len; i++){
        hash+= *(uint8_t*) data; data+=1;
        hash = next(hash, 8); // шаг 8 бит
    }
    return mix(hash)-IV;
}

#define MWC_A1 0xffe118abuLL
static inline uint64_t mwc64_next(uint64_t s, int r, uint64_t A) {
    return A*((uint32_t)s<<(32-r)) + (s>>r);
}
uint64_t mwc64r2_hash(const uint8_t* data, uint64_t len, uint64_t seed){
	uint64_t s[2];
    s[0] = unmix(seed+=IV);
    s[1] = unmix(seed+=IV);
    int i;
    while (len>=8){
        len-=8;
      uint64_t d0 = *(uint32_t*) data; data+=4;
      uint64_t d1 = *(uint32_t*) data; data+=4;
      uint64_t s0 = s[1], s1 = s[0]; //SHUFFLE
      s[0]^= mwc64_next(s0^d0, 32, MWC_A0);
      s[1]^= mwc64_next(s1^d1, 32, MWC_A1);
    }
    while (len>=4) {
        len -= 4;
        uint64_t d = *(uint32_t*) data; data+=4;
        s[0] ^= mwc64_next(s[1]^d, 32, MWC_A0);
    }
    if (len&3) {
        int r = len&3;
        uint32_t d = 0;
        __builtin_memcpy(&d, data, r); data+=r;
        s[0] ^= mwc64_next(s[0]^d, 8*r, MWC_A0);
    }
    return mix(s[0]^s[1]);
}

#if 1//def TEST_MWC64_HASH
uint64_t mwc64_hash_32(uint64_t hash, uint8_t* data, size_t data_len){
    hash = unmix(hash+IV);
    while (data_len>=4){
        hash+= *(uint32_t*) data; data+=4;
        hash = next(hash, 32);
        data_len-=4;
    }
    if (data_len&2){
        hash+= *(uint16_t*) data; data+=2;
        hash = next(hash, 16);
    }
    if (data_len&1){
        hash+= *(uint8_t*) data; data+=1;
        hash = next(hash, 8);
    }
    return mix(hash)-IV;
}
uint64_t mwc64_hash_16(uint64_t hash, uint8_t* data, size_t data_len){
    hash = unmix(hash+IV);
    for (int i=0; i<data_len>>1; i++){
        hash+= *(uint16_t*) data; data+=2;
        hash = next(hash, 16);
    }
    if (data_len&1){
        hash+= *(uint8_t*) data; data+=1;
        hash = next(hash, 8);
    }
    return mix(hash)-IV;
}
#include <stdio.h>
#include <stdlib.h>
/*! \brief Специальный вид инверсии для алгоритма редуцирования */
static inline uint64_t INVL(uint64_t v) {
    return ((unsigned __int128)(-v)<<64)/v;
}
int main() {
    uint8_t str[] = "1234567890abcdef";
    uint64_t seed = 0x1234567812345678u;
    uint64_t h1 = mwc64_hash(seed, str, 11);
    uint64_t h8 = mwc64_hash_8(seed, str, 11);
    uint64_t h16= mwc64_hash_16(seed, str, 11);
    if (h1 == h8) printf("1:MWC64 ..ok\n");
    if (h1 ==h16) printf("1:MWC64 ..ok\n");
    h1 = mwc64_hash_8(seed, str, 5);
    h1 = mwc64_hash_8(h1, str+5, 6);
    if (h1 == h8) printf("2:MWC64 ..ok\n");
    h1 = mwc64_hash(seed, str, 5);
    h1 = mwc64_hash(h1, str+5, 6);
    if (h1 == h8) printf("3:MWC64 ..ok\n");
    h1 = mwc64_hash(h1, str, 9);
    printf ("0x%08X\n", (uint32_t)h1 );
if (1) {
    printf ("INV 0x%016llX\n", INVL(MWC_PRIME) );
        uint64_t p=mwc_powm(2, (32u*2), MWC_PRIME);
        printf ("jump(%3d) 0x%016llX\n", -32u*2, p);
        uint64_t q=mwc_powm(2, (32u*4), MWC_PRIME);
        printf ("jump(%3d) 0x%016llX\n", -32u*4, q);
    for (int i=0; i<8;i++) {// таблица переходов
        uint64_t p=mwc_powm(2, MWC_PRIME-1-(32u*i), MWC_PRIME);
        printf ("jump(%3d) 0x%016llX\n", 32u*i, p);
        uint64_t q=mwc_powm(2, MWC_PRIME-1-(32u*i + 64), MWC_PRIME);
        printf ("jump(%3d) 0x%016llX\n", 32u*i + 64, q);
    }

    uint8_t str1[] = "123\x00\x00\x00\x00";
    uint8_t str2[] = "\x00\x00\x00""a\x00\x00\x00";
    uint8_t str4[] = "\x00\x00\x00\x00""bcd";
    uint8_t str3[] = "123abcd";
    uint8_t str5[] = 
        "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
        "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
        "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        ;
    uint64_t h1, h2, h3;
    h1 = mwc64_hash(seed, str1, 3);
    h1 = mwc64_skip(h1, 4);// дистанция в байтах
//    h1 = mwc64_jump(h1, MWC_A0);
    h2 = mwc64_hash( -IV, str2, 7);
    h3 = mwc64_hash( -IV, str4, 7);
    h1 = mwc64_merge(h1, h2, 0);
    h1 = mwc64_merge(h1, h3, 0);
    h2 = mwc64_hash(seed, str3, 7);
    printf ("0x%016llX\n", h1 );
    printf ("0x%016llX %s\n", h2, h1==h2?"PASS":"FAIL" );
    h1 = mwc64_hash_8(seed, str5, 725);
    h3 = mwc64_hash_32(seed, str5, 725);
    h2 = mwc64_hash(seed, str5, 725);
//    h2 = mwc64_skip(h2, 8);
    printf ("0x%016llX\n", h1 );
    printf ("0x%016llX\n", h3 );
    printf ("0x%016llX %s\n", h2, h1==h2?"PASS":"FAIL" );
    size_t len = 256*1024;
    uint8_t *str = malloc(len);
    for (int i=0;i<256*1024; i++) str[i] = i;
    h1 = mwc64_hash(seed, str, len);
    h2 = mwc64_hash_32(seed, str, len);
    printf ("0x%016llX %s\n", h1, h1==h2?"PASS":"FAIL" );
}
    return 0;
}
#endif