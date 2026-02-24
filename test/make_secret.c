#include <stdint.h>
// Скопировано из WYhash 
// https://github.com/wangyi-fudan/wyhash/blob/master/wyhash.h
typedef unsigned int __attribute__((mode(TI)))   uint128_t;
static inline void _wymum(uint64_t *A, uint64_t *B){
  uint128_t r=*A; r*=*B; 
  *A=(uint64_t)r; *B=(uint64_t)(r>>64);
}
static inline uint64_t _wymix(uint64_t A, uint64_t B){ 
    _wymum(&A,&B); 
    return A^B; 
}
//The wyrand PRNG that pass BigCrush and PractRand
static inline uint64_t wyrand(uint64_t *seed){ 
    *seed+=0x2d358dccaa6c78a5ull; 
    return _wymix(*seed,*seed^0x8bb84b93962eacc9ull);}

//convert any 64 bit pseudo random numbers to uniform distribution [0,1). 
static inline double wy2u01(uint64_t r){ const double _wynorm=1.0/(1ull<<52); return (r>>12)*_wynorm;}
//fast range integer random number generation on [0,k) credit to Daniel Lemire. 
static inline uint64_t wy2u0k(uint64_t r, uint64_t k){ _wymum(&r,&k); return k; }
//convert any 64 bit pseudo random numbers to APPROXIMATE Gaussian distribution.
static inline double wy2gau(uint64_t r){ const double _wynorm=1.0/(1ull<<20); return ((r&0x1fffff)+((r>>21)&0x1fffff)+((r>>42)&0x1fffff))*_wynorm-3.0;}


// modified from https://github.com/going-digital/Prime64 
static	inline	unsigned long long	mul_mod(unsigned long long a, unsigned long long b, unsigned long long m) {
    unsigned long long r=0;
    while (b) {
        if (b & 1) {
            unsigned long long r2 = r + a;
            if (r2 < r) r2 -= m;
            r = r2 % m;
        }
        b >>= 1;
        if (b) {
            unsigned long long a2 = a + a;
            if (a2 < a) a2 -= m;
            a = a2 % m;
        }
    }
    return r;
}
static inline unsigned long long pow_mod(unsigned long long a, unsigned long long b, unsigned long long m) {
    unsigned long long r=1;
    while (b) {
        if (b&1) r=mul_mod(r,a,m);
        b>>=1;
        if (b) a=mul_mod(a,a,m);
    }
    return r;
}
unsigned sprp(unsigned long long n, unsigned long long a) {
    unsigned long long d=n-1;
    unsigned char s=0;
    while (!(d & 0xff)) { d>>=8; s+=8; }
    if (!(d & 0xf)) { d>>=4; s+=4; }
    if (!(d & 0x3)) { d>>=2; s+=2; }
    if (!(d & 0x1)) { d>>=1; s+=1; }
    unsigned long long b=pow_mod(a,d,n);
    if ((b==1) || (b==(n-1))) return 1;
    unsigned char r;
    for (r=1; r<s; r++) {
        b=mul_mod(b,b,n);
        if (b<=1) return 0;
        if (b==(n-1)) return 1;
    }
    return 0;
}
unsigned is_prime(unsigned long long n) {
    if (n<2||!(n&1)) return 0;
    if (n<4) return 1;
    if (!sprp(n,2)) return 0;
    if (n<2047) return 1;
    if (!sprp(n,3)) return 0;
    if (!sprp(n,5)) return 0;
    if (!sprp(n,7)) return 0;
    if (!sprp(n,11)) return 0;
    if (!sprp(n,13)) return 0;
    if (!sprp(n,17)) return 0;
    if (!sprp(n,19)) return 0;
    if (!sprp(n,23)) return 0;
    if (!sprp(n,29)) return 0;
    if (!sprp(n,31)) return 0;
    if (!sprp(n,37)) return 0;
    return 1;
}
//make your own secret
static void make_secret(uint64_t seed, uint64_t *secret){
  uint8_t c[] = {15, 23, 27, 29, 30, 39, 43, 45, 46, 51, 53, 54, 57, 58, 60, 71, 75, 77, 78, 83, 85, 86, 89, 90, 92, 99, 101, 102, 105, 106, 108, 113, 114, 116, 120, 135, 139, 141, 142, 147, 149, 150, 153, 154, 156, 163, 165, 166, 169, 170, 172, 177, 178, 180, 184, 195, 197, 198, 201, 202, 204, 209, 210, 212, 216, 225, 226, 228, 232, 240 };
  for(size_t i=0;i<4;i++){
    uint8_t ok;
    do{
      ok=1; secret[i]=0;
      for(size_t j=0;j<64;j+=8) secret[i]|=((uint64_t)c[wyrand(&seed)%sizeof(c)])<<j;
      if(secret[i]%2==0){ ok=0; continue; }      
      for(size_t j=0;j<i;j++) {
        if(__builtin_popcountll(secret[j]^secret[i])!=32){ 
            ok=0; break; 
        }
      }
      if(ok&&!is_prime(secret[i]))	ok=0;
    }while(!ok);
  }
}

#include <stdio.h>
int main(){
    uint64_t C1 = UINT64_C(0x9e3779b97f4a7c15);
//    uint64_t C1 = UINT64_C(0xcbf29ce484222325);
    uint64_t secrets[4];
    make_secret(C1, secrets);
    for (int i=0; i< 4; i++){
        printf("|0x%016llx| \n", secrets[i]);
    }

    uint64_t C2 = UINT64_C(0xa3b195354a39b70d);
    for (int i=0; i< 8; i++){
        printf("C%d = 0x%016llx| \n", 4<<i, C2*=C2);
    }

    return 0;
}