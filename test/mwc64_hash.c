
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
static inline uint64_t next(uint64_t x, int r){
    return ((uint32_t)x<<(32-r))*MWC_A0 + (x>>r);
}
uint64_t mwc64_hash(uint64_t hash, uint8_t* data, size_t data_len){
    hash = unmix(hash+IV);
#if 0
    for (int i=0; i<data_len>>3; i++){
        uint64_t d = *(uint64_t*) data; data+=8;
        if (__builtin_uaddl_overflow (hash, d, &hash))
            hash -= MWC_PRIME;
        // if(hash<d) hash -= MWC_PRIME;
        // else
        // if(hash>=MWC_PRIME) hash -= MWC_PRIME;
        hash = next(hash, 32);
        hash = next(hash, 32);
    }
    if (data_len&4)
#else
    for (int i=0; i<data_len>>2; i++)
#endif
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
    return mix(hash)-IV;
}
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
static inline uint64_t mwc_maddm(uint64_t a, uint64_t b, uint64_t c, uint64_t M)
{	
	unsigned __int128 ac;
	ac = (unsigned __int128)a*b + c;
	ac-= (((ac>>64)*MWC_INV + ac)>>64)*M;
	if (ac>>64) ac -= M;
//	if ((uint64_t)ac>= M) ac -= M;
	return ac;
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
// дистанция по числу бит
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
#ifdef TEST_MWC64_HASH
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
uint64_t mwc64_hash_8(uint64_t hash, uint8_t* data, size_t data_len){
    hash = unmix(hash+IV);
    for (int i=0; i<data_len; i++){
        hash+= *(uint8_t*) data; data+=1;
        hash = next(hash, 8);
    }
    return mix(hash)-IV;
}
#include <stdio.h>
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
    for (int i=0; i<8;i++) {// таблица переходов
        uint64_t m=mwc_powm(MWC_A0, 1uLL<<(i), MWC_PRIME);
        printf ("jump(2^%d) 0x%016llX\n", (i+2), m);
    }
    uint8_t str1[] = "123\x00\x00\x00\x00";
    uint8_t str2[] = "\x00\x00\x00""a\x00\x00\x00";
    uint8_t str4[] = "\x00\x00\x00\x00""bcd";
    uint8_t str3[] = "123abcd";
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
}
    return 0;
}
#endif