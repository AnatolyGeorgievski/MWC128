//𝑝 = 2^{64} -2^{32} + 1
// https://project-george.blogspot.com/2025/09/
#include <stdint.h>
#include <stddef.h>

// Doug Lea's mixing function, fastmix дважды
static inline uint64_t mix_lea(uint64_t h) {
  const uint64_t M = 0xdaba0b6eb09322e3u;
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

#define MWC_A  0x7ff8c871
static inline int64_t next(int64_t x, int r){
    return (x>>r) - ((uint32_t)x<<(32-r))*(int64_t)MWC_A;
}
#define IV 	0x9e3779b97f4a7c15u
uint64_t mwc64s_hash(uint64_t seed, uint8_t* data, size_t data_len){
    int64_t hash = (int64_t)unmix_lea(seed+IV);
    for (int i=0; i<data_len>>2; i++){
        hash += *(uint32_t*) data; data+=4;
        hash  = next(hash, 32);
    }
    if (data_len&2){
        hash += *(uint16_t*) data; data+=2;
        hash  = next(hash, 16);
    }
    if (data_len&1){
        hash += *(uint8_t*) data; data+=1;
        hash  = next(hash, 8);
    }
    return mix_lea((uint64_t)hash)-IV;
}

typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;
// signed montgomery
// https://eprint.iacr.org/2018/039.pdf
int64_t mod_mont(int128_t a, int64_t q, int64_t qm){
    int64_t m = (uint64_t)a*qm;
    int64_t t =(m*(int128_t)q)>>64;
    a = (a>>64) - t;
    return a;
}
int64_t mod_mont2(int128_t a, int64_t q, int64_t qm){
    a = (a>>32) - (uint32_t)a*(int128_t)q;
    a = (a>>32) - (uint32_t)a*(int128_t)q;
//    if (a<0) a+=q;
    return a;
}
int64_t mod_gold(int128_t a, int64_t q, int64_t qm){
    int64_t m = (uint64_t)a + ((uint64_t)a<<32);// 0000000100000001
    int64_t t = m + (((int128_t)m*qm)>>64);// ffffffff00000001

    return (a>>64) - t;
}
/*! \brief Функция для вычисления a^{-1} mod 2^{64} */
static uint64_t inverse_u64(uint64_t a) {
    uint64_t x = a;
    // 5 итераций — стандарт для 64 бит
    x *= 2ULL - a * x;
    x *= 2ULL - a * x;
    x *= 2ULL - a * x;
    x *= 2ULL - a * x;
    x *= 2ULL - a * x;
    return x;
}
#include <stdio.h>
int main(){
    int64_t q  = ((int64_t)MWC_A<<32) +1;
    int64_t qm = inverse_u64(q);
    printf("%016llx %016llx %d\n", q, qm, q*qm);
    q  = -(1uLL<<32) +1;
    qm = inverse_u64(q);
    printf("%016llx %016llx %d\n", q, qm, q*qm);
    int32_t primes[] = {MWC_A};
        printf("Signed Montgomery test\n");
        for (int k = 0; k< sizeof(primes)/sizeof(primes[0]); k++) {
            int64_t p = ((int64_t)primes[k]<<32) +1;
            //if ((p>>31)!=0) continue;
            int64_t pi = inverse_u64(p);
            printf ("p=%016llx pi = %016llx %d \n",p, pi, p*(pi));
            for (int64_t a =-p/2; a<p/4; a++)
            {
                int64_t b  = ((int128_t)a<<64)%p;
                int128_t a2 = (int128_t)a*b;

                int64_t r  = mod_mont(a2, p, pi);
                if (r<0) r+=p;

                int64_t r2 = ((int128_t)a*a)%p;
                if (r2!=r) {
                    printf ("r = %d %d p=%016llx \n", r,r2,a);
                     break;
                }
            }
        }
    return 0;
}