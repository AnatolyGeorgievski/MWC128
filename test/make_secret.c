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

int _max_order(uint64_t x) {
    int count = 64; // 2^61
    do { 
		x = (x* x); // x ← x² mod (2⁶⁴ ^ 1)
		if (x <= 1) return 65-count; 
	} while (--count);
    return 64-count;
}
//make your own secret
static void make_secret(uint64_t seed, uint64_t *secret, int N){
// Это все возможные комбинации выбора 4-x бит из 8 бит.
// так мы синтезируем число с одинаковым числом 1 единиц
  uint8_t c[] = {15, 23, 27, 29, 30, 39, 43, 45, 46, 51, 53, 54, 57, 58, 60, 71, 75, 77, 78, 83, 85, 86, 89, 90, 92, 99, 101, 102, 105, 106, 108, 113, 114, 116, 120, 135, 139, 141, 142, 147, 149, 150, 153, 154, 156, 163, 165, 166, 169, 170, 172, 177, 178, 180, 184, 195, 197, 198, 201, 202, 204, 209, 210, 212, 216, 225, 226, 228, 232, 240 };
  for(size_t i=0;i<N;i++){
    uint8_t ok;
    do{
      ok=1; secret[i]=0;
      for(size_t j=0;j<64;j+=8) 
        secret[i]|=((uint64_t)c[wyrand(&seed)%sizeof(c)])<<j;
      if(secret[i]%2==0){ ok=0; continue; }      
      for(size_t j=0;j<i;j++) {
        if(__builtin_popcountll(secret[j]^secret[i])!=32){ 
            ok=0; break; 
        }
      }
      if(ok&&_max_order(secret[i])<62) ok= 0;
      if(ok&&!is_prime(secret[i]))	ok=0;
    }while(!ok);
  }
}
#include <stdio.h>
#include <stdbool.h>
int is_perfect(uint64_t x){
    int count = 12;
    do {
        x = x*x;
        if (__builtin_popcountll(x)!=32) break;
    } while (--count);
    return count;
}
static void make_secretN(uint64_t seed, int N){
// так мы синтезируем число с одинаковым числом 1 единиц
  uint8_t c[] = {15, 23, 27, 29, 30, 39, 43, 45, 46, 51, 53, 54, 57, 58, 60, 71, 75, 77, 78, 83, 85, 86, 89, 90, 92, 99, 101, 102, 105, 106, 108, 113, 114, 116, 120, 135, 139, 141, 142, 147, 149, 150, 153, 154, 156, 163, 165, 166, 169, 170, 172, 177, 178, 180, 184, 195, 197, 198, 201, 202, 204, 209, 210, 212, 216, 225, 226, 228, 232, 240 };

    int count = 512, i=2, ord;
    uint64_t s = 0,s2,s4,s8,s16;
    do {
        do {
            do
                s = c[i++%sizeof(c)];
            while ((s&1)==0);
            for(size_t j=8;j<64;j+=8) 
                s |=((uint64_t)c[wyrand(&seed)%sizeof(c)])<<j;
        } while ((ord = is_perfect(s))>4 || _max_order(s)!=62);
        printf("|0x%016llx| %2d | %d\n", s, __builtin_popcountll(s), 12-ord);
    } while(count--);

}
int main(){
    uint64_t C1 = UINT64_C(0x9e3779b97f4a7c15);
    uint64_t s0 = UINT64_C(0x9e3779b97f4a7c15);
//    uint64_t C1 = UINT64_C(0xcbf29ce484222325);
    make_secretN(s0, 512);
const int M = 8;
    uint64_t secrets[M];
    make_secret(C1, secrets, M);
    for (int i=0; i< M; i++){
        uint64_t C2 = secrets[i];
        int ord = _max_order(C2);
        printf("|0x%016llx| %2d\n", C2, ord);
        for (int i=0; i< 8; i++){
            C2=C2*C2;//_wymix(C2,C2);
            int ord = _max_order(C2);
            printf("#define C%d\tUINT64_C(0x%016llx) // %2d %2d\n", 4<<i, C2, ord, __builtin_popcountll(C2));
        }

    }

//    uint64_t C2 = UINT64_C(0xa3b195354a39b70d);
    uint64_t C2 = UINT64_C(0x938d8ea693c51b8b);//0x8b4e7465b45a2765; 0x8ee46636d4ac998d, 0x938d8ea693c51b8b
    printf("C%d\tUINT64_C(0x%016llx)// %2d %2d\n", 2, C2, _max_order(C2), __builtin_popcountll(C2));
    for (int i=0; i< 8; i++){
        C2=C2*C2;//_wymix(C2,C2);
        int ord = _max_order(C2);
        printf("#define C%d\tUINT64_C(0x%016llx) // %2d %2d\n", 4<<i, C2, ord, __builtin_popcountll(C2));
    }

    return 0;
}