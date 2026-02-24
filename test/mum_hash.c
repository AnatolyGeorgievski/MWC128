#include <stdint.h>
/*!
Операция смешивания это `rlo:rhi := d0*K1 + d1*K2`
Операция смешивания это `rlo:rhi := d0*K1 + d1*K2 + rlo*K3 + rhi*K4`
Операция Folding:
```
  rlo:rhi += d0:d1
  if (rhi<d1) rhi-=PRIME;
  rlo:rhi := NEXT(rlo:rhi) = rlo*K3 + rhi + d
  rlo:rhi := rlo*K3 + rhi + d
```
Можно по модулю 2^64-2^{32}+1. Мы редуцируем MWC по модулю A*2^{32} -1
 */
typedef unsigned int __attribute__((mode(TI)))   uint128_t;
static inline uint64_t mum (uint64_t v, uint64_t p) {
    uint64_t hi, lo;
    uint128_t r = (uint128_t) v * p;
    hi = (uint64_t) (r >> 64);
    lo = (uint64_t) r;
    return hi + lo; // редуцирование по модулю 2^{64}-1. 
}

static inline uint64_t mumix (uint64_t v, uint64_t p) {
    uint64_t hi, lo;
    uint128_t r = (uint128_t) v * p;
    hi = (uint64_t) (r >> 64);
    lo = (uint64_t) r;
    return hi ^ lo;
}

static const uint64_t primes[] = {
  UINT64_C(0x9ebdcae10d981691), UINT64_C(0x32b9b9b97a27ac7d), UINT64_C(0x29b5584d83d35bbd), UINT64_C(0x4b04e0e61401255f),
  UINT64_C(0x25e8f7b1f1c9d027), UINT64_C(0x80d4c8c000f3e881), UINT64_C(0xbd1255431904b9dd), UINT64_C(0x8a3bd4485eee6d81),
  UINT64_C(0x3bc721b2aad05197), UINT64_C(0x71b1a19b907d6e33), UINT64_C(0x525e6c1084a8534b), UINT64_C(0x9e4c2cd340c1299f),
  UINT64_C(0xde3add92e94caa37), UINT64_C(0x7e14eadb1f65311d), UINT64_C(0x3f5aa40f89812853), UINT64_C(0x33b15a3b587d15c9),
};

/*! Генератор случайных чисел */
#define MUM_SZ 2 // размер состояния 1-16:64 ... 1024
void mum_next(uint64_t* state) {
    for (int i = 0; i < MUM_SZ - 1; i++)
        state[i] ^= mum (state[i + 1], primes[i]);
    state[MUM_SZ - 1] ^= mum(state[0], primes[MUM_SZ - 1]);
}
#define IV    UINT64_C(0x60bee2bee120fc15)
#define MUM_S UINT64_C(0x82d2e9550235efc5)
#define MUM_C UINT64_C(0xa3b195354a39b70d)
/*! Функция MUM хэш  */
void mum_hash(const uint8_t* data, size_t len, uint64_t seed, uint8_t* out) {
    uint64_t state[MUM_SZ];
    for (int i=0; i<MUM_SZ; i++)
        state[i] = mumix(seed+=IV, MUM_S);
    int i =0;
    while (len>=16) {
        unsigned int i1 = i+1==MUM_SZ? 0:i+1;
        uint64_t d0 = *(uint64_t*)data; data+=8;
        uint64_t d1 = *(uint64_t*)data; data+=8;
        state[i]^=mum(d0 ^ primes[i1], d1 ^ primes[i]);
        len-=16; i=i1;
    }
    *(uint64_t*)out = mumix(state[0]^state[1], MUM_C);
}