#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* Скопировано из WYhash, доработан критерий
    \see https://github.com/wangyi-fudan/wyhash/blob/master/wyhash.h
Сборка 
    $ gcc -O3 -march=native -o test test/make_secret.c test/proth_prime.c
 */
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


static inline uint64_t INVL(uint64_t v) {
    return ((unsigned __int128)(-v)<<64)/v;
}
static inline uint64_t INVL52(uint64_t v) {
    return ((unsigned __int128)((1uLL<<52)-v)<<52)/v;
}
/*! \brief модульная операция с отложенным редуцированием */
static inline uint64_t _mulm(uint64_t A, uint64_t B, uint64_t M, uint64_t M_INV) {	
    uint128_t ac = A*(uint128_t)B;
	ac-= (((ac>>64)*M_INV + ac)>>64)*M;
	if (ac>>64) {
        ac -= M;
        printf("$");
    }
	return ac;
}
#define PRIME UINT64_C(0xffffffffff000001)
static inline uint64_t _mix(uint64_t A, uint64_t B){ 
    const uint64_t P_INV = INVL(PRIME);
    return _mulm(A,B, PRIME, P_INV);
}
/*! Умножение по модулю */
static inline uint64_t mulm(const uint64_t b, uint64_t a, const uint64_t N) {
    return ((unsigned __int128)b*a)%N;
}
/*! Возведение в степнь по модулю */
static uint64_t powm(const uint64_t b, uint64_t a, const uint64_t N)
{
	uint64_t r = b;
	uint64_t s = 1;
    while (a!=0) {
		if (a&1) 
            s = mulm(s,r,N);
        r = mulm(r,r,N);
		a>>=1;
	}
	return s;
}
// Простое число является генератором группы по a^(P-1)/w = 1 mod N
static unsigned sprp(uint64_t n, uint64_t a) {
    uint64_t  d=n-1;
    unsigned char s=0;
    s = __builtin_ctzll(n-1);
    d = (n-1)>>s;
    uint64_t b=powm(a,d,n);
    if ((b==1) || (b==(n-1))) return 1;
// избыточно 
    unsigned char r;
    for (r=1; r<s; r++) {
        b=mulm(b,b,n);
        if (b<=1) return 0;
        if (b==(n-1)) return 1;
    }
    return 0;
}
const uint32_t prime[] = {2,3,5,7,11,13,17,19,23,29,31,37,
41,	43,	47,	53,	59,	61,	67,	71, 73,	79,	83,	89,	97,	101,
103,107,109,113,127,131,137,139,149,151,157,163,167,173,
179,181,191,193,197,199,211,223,227,229,233,239,241,251,257
};
/*! поиск простых чисел - мой тест 
См тестирование чисел Прота на простоту
 */
int powm_tst2(const uint64_t p,  int size){
	uint64_t a = p - 1;
	uint64_t b,s;
	for(int i=0; i< size; i++){
        b = prime[i];
		s = powm(b, a, p);
		if (s != 1) return 0;
	}
	return 1;
}
// Можно проверять по первым N простым числам
unsigned is_prime(unsigned long long n) {
    if (n<2||!(n&1)) return 0;
    if (n<4) return 1;
    if (!sprp(n,2)) return 0;
    if (n<2047) return 1;
// первые 6 простых чисел - это мой тест 
    if (!sprp(n,3)) return 0;
    if (!sprp(n,5)) return 0;
    if (!sprp(n,7)) return 0;
    if (!sprp(n,11)) return 0;
    if (!sprp(n,13)) return 0;
    if (!sprp(n,17)) return 0;
// избыточно 
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
//make your own secret
static void make_secret(uint64_t seed, uint64_t *secret, int N){
// Это все возможные комбинации выбора 4-x бит из 8 бит.
// так мы синтезируем число с одинаковым числом 1 единиц и нулей
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
extern int Proth_prime (uint64_t p);
uint64_t pow_2n(uint64_t x, int n){
    do {
        x = x*x;
    } while(--n);
    return x;
}
int is_perfect(uint64_t x){
    int count = 12;
    do {
        x = x*x;
        if (__builtin_popcountll(x)!=32) break;
    } while (--count);
    return count;
}
static void make_secretN(uint64_t seed, int Nr){
// так мы синтезируем число с одинаковым числом 1 единиц и ноликов в каждом байте
  uint8_t c[] = {15, 23, 27, 29, 30, 39, 43, 45, 46, 51, 53, 54, 57, 58, 60, 71, 75, 77, 78, 83, 85, 86, 89, 90, 92, 99, 101, 102, 105, 106, 108, 113, 114, 116, 120, 135, 139, 141, 142, 147, 149, 150, 153, 154, 156, 163, 165, 166, 169, 170, 172, 177, 178, 180, 184, 195, 197, 198, 201, 202, 204, 209, 210, 212, 216, 225, 226, 228, 232, 240 };

    int count = Nr, i=2, ord;
    uint64_t s = 0,s2,s4,s8,s16;
    do {
        do {
            do
                s = c[i++%sizeof(c)];
            while ((s&3)!=3);// младшие два бита обеспечивают попадание в группу max_order=62
            for(size_t j=8;j<64;j+=8) 
                s |=((uint64_t)c[wyrand(&seed)%sizeof(c)])<<j;
        } while ((ord = is_perfect(s))>4 || _max_order(s)!=62);
        int pp = Proth_prime(pow_2n(s, 30));
        printf("|0x%016llx| %2d | %d %s\n", s, __builtin_popcountll(s), 12-ord, pp?"p":"-");
    } while(count--);

}
int main(){
    uint64_t C1 = UINT64_C(0x9e3779b97f4a7c15);
    uint64_t s0 = UINT64_C(0x9e3779b97f4a7c15)*17;
//    uint64_t C1 = UINT64_C(0xcbf29ce484222325);
if(0)     make_secretN(s0, 512);
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
//    uint64_t C2 = UINT64_C(0x938d8ea693c51b8b);//0x8b4e7465b45a2765; 0x8ee46636d4ac998d, 0x938d8ea693c51b8b
//    uint64_t C2 = 0xb17269ac631da58b;// (10) 
//    uint64_t C2 = 0x99e4d4b2e459691d;// (10)
// |0x6c7165963aacd24b| 32 | 10
//    uint64_t C2 = 0xe866396a2d1b954b;// 32 | 10
//uint64_t C2 = 0xfffffffffffffc01;
// |0x563966a5b4994e4b| 32 | 10
uint64_t C2 = 0xc5470f4b59aab41b;// 32 | 10 
//    0x552e96659c1b8b4b () 0xb1c5e4b1accca9a5 0xc9965cd4398ecca3
    uint64_t C2_ = inverse_u64(C2);
    printf("C%d\tUINT64_C(0x%016llx)// %2d %2d inv=0x%016llx %lld\n", 2, C2, _max_order(C2), __builtin_popcountll(C2), C2_, C2_*C2);
    uint64_t C30 = pow_2n(C2, 30);
    int pp = Proth_prime(pow_2n(C2, 30));
    printf("C%d\tUINT64_C(0x%016llx)\n", 30, C30);
    for (int i=0; i< 62; i++){
        C2=_mix(C2,C2);//_wymix(C2,C2);
        int ord = _max_order(C2);
        uint64_t Ci = inverse_u64(C2);
        printf("#define C%d\tUINT64_C(0x%016llx) // inv =0x%016llx ord=%2d %2d %s %s\n", i+1, 
            C2, Ci, ord, __builtin_popcountll(C2), Proth_prime(C2)?"p":"-", Proth_prime(Ci)?"p":"-");
    }
    /*
Mersenne
prime 0xfffffffffffffc01 10
prime 0xfffffffffffff001 12
prime 0xffffffffff000001 24
prime 0xffffffff00000001 32
prime 0xfffffffc00000001 34
prime 0xffffff0000000001 40
    
    */
for (int i=1; i<52; i++){
    uint64_t p = (1uLL<<52)-(1uLL<<i) +1;
    if (is_prime(p)){
        uint64_t inv = INVL52(p);
        printf("prime 0x%016llx invl = 0x%016llx %d\n", p, inv, i);
    }
}
    return 0;
}