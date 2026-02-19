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