
/*! __Анатолий М. Георгиевский__, ИТМО, 2026 
    MWC64-hash хэш-функция на базе генератора случайных чисел
    Состоит из генератора и Avalanche-миксера. Миксер - обратимая функция.
    Генератор `next()` сдвигает заданное число бит в младшей части числа

    Хэш можно дописывать и считать параллельно по сегментам.
 */
#include <stdint.h>
#include <stddef.h>

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
#define mix mix64//mix_stafford13
#define unmix unmix64//unmix_stafford13
#define IV 	0x9e3779b97f4a7c15u
#define MWC_A0 0xfffeb81bULL
static inline uint64_t next(uint64_t x, int r){
    return ((uint32_t)x<<(32-r))*MWC_A0 + (x>>r);
}
uint64_t mwc64_hash(uint64_t hash, uint8_t* data, size_t data_len){
    hash = unmix(hash+IV);
    const uint64_t P = (MWC_A0<<32)-1;
    int i;
    for (i=0; i<data_len>>2; i++){
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
    return 0;
}
#endif