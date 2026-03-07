// LFSR: X^128 + X^126 + X^101 + X^99 + 1
/*! 
   \see https://www.pcg-random.org/pdf/hmc-cs-2014-0905.pdf
 */
#include <stdint.h>
#include <stdio.h>

static uint32_t inverse_u32(uint32_t a) {
    uint32_t x = a;
    x *= 2 - a * x;
    x *= 2 - a * x;
    x *= 2 - a * x;
    x *= 2 - a * x;
    return x;
}
// алгоритм деления
static uint64_t inverse_u64_(uint64_t a) {
    uint64_t x = a;
    uint64_t e;// ошибка
    while((e = 1 - a*x)!=0)
        x = x + x*e;
    return x;
}
static uint64_t inverse_u64(uint64_t a) {
    uint64_t x = a;
    x *= 2 - a * x;
    x *= 2 - a * x;
    x *= 2 - a * x;
    x *= 2 - a * x;
    x *= 2 - a * x;
    return x;
}
typedef unsigned __int128 uint128_t;
static uint128_t inverse_u128(uint128_t a) {
    uint128_t x = a;
    x *= 2 - a * x;
    x *= 2 - a * x;
    x *= 2 - a * x;
    x *= 2 - a * x;
    x *= 2 - a * x;
    x *= 2 - a * x;
    return x;
}
#define M       UINT64_C(0x5851F42D4C957F2D)
#define M_INV   UINT64_C(0xC097EF87329E28A5)
#define M_LEM   UINT64_C(0xda942042e4dd58b5)// Lehmer64 multiplier
#define GR      UINT64_C(0x9e3779b97f4a7c15) 
// PCG32_rev : a^{-1} =0xC097EF87329E28A5 )
#define UINT128_C(high, low)   \
    ( ((uint128_t)(uint64_t)(high) << 64) | (uint64_t)(low) )
#define M_LEM_INV UINT128_C(0x0cd365d2cb1a6a6c,0x8b838d0354ead59d)
#define M_PCG     UINT128_C(0x5851F42D4C957F2D,0x14057B7EF767814F)
#define M_PCG_INV UINT128_C(0x6cd664ad3d011f2b,0xe68db5facb22f5af)
#define M_PCG_INC UINT128_C(0x2360ED051FC65DA4,0x4385DF649FCCF645)

/*! 
 Используется вариант выходного миксера XSH
 */
uint32_t PCG_next(uint64_t *s){
    uint64_t x = s[0];
    x = x*M + GR;
    s[0] = x;
    return x^(x>>32);
}
uint32_t PCG_prev(uint64_t *s){
    uint64_t x = s[0];
    s[0] = (x-GR)*M_INV;
    return x^(x>>32);
}

uint64_t PCG64_next(uint64_t *s){
    uint128_t x = *(uint128_t*)s;
    x = x*M_LEM + GR;
    *(uint128_t*)s = x;
    return x ^ (x>>64);
}
uint64_t PCG64_prev(uint64_t *s){
    uint128_t x = *(uint128_t*)s;
    uint64_t r = x ^ (x>>64);
    x = (x-GR)*M_LEM_INV;
    *(uint128_t*)s = x;
    return r;
}
uint64_t PCG128_next(uint64_t *s){
    uint128_t x = *(uint128_t*)s;
    x = x*M_PCG + M_PCG_INC;
    *(uint128_t*)s = x;
    return x ^ (x>>64);
}
uint64_t PCG128_prev(uint64_t *s){
    uint128_t x = *(uint128_t*)s;
    uint64_t r = x ^ (x>>64);
    x = (x-M_PCG_INC)*M_PCG_INV;
    *(uint128_t*)s = x;
    return r;
}
// метод пропуска шагов
uint64_t PCG128_skip(uint64_t *s, int n){
}
uint64_t PCG128_jump(uint64_t *s, uint128_t jump){
    uint128_t x = *(uint128_t*)s;
//    x = jump*x + (jump-1)*(GR*M1_INV);
}


/*
PCG_DEFINE_CONSTANT(uint8_t,  default, multiplier, 141U)
PCG_DEFINE_CONSTANT(uint8_t,  default, increment,  77U)

PCG_DEFINE_CONSTANT(uint16_t, default, multiplier, 12829U)
PCG_DEFINE_CONSTANT(uint16_t, default, increment,  47989U)

PCG_DEFINE_CONSTANT(uint32_t, default, multiplier, 747796405U)
PCG_DEFINE_CONSTANT(uint32_t, default, increment,  2891336453U)

PCG_DEFINE_CONSTANT(uint64_t, default, multiplier, 6364136223846793005ULL)
PCG_DEFINE_CONSTANT(uint64_t, default, increment,  1442695040888963407ULL)

PCG_DEFINE_CONSTANT(pcg128_t, default, multiplier,
        PCG_128BIT_CONSTANT(2549297995355413924ULL,4865540595714422341ULL))
PCG_DEFINE_CONSTANT(pcg128_t, default, increment,
        PCG_128BIT_CONSTANT(6364136223846793005ULL,1442695040888963407ULL))
 */
static uint64_t div_c64(uint64_t b, int *nd);
static uint64_t div_c  (uint64_t b, int *nd);
int main(){
    uint32_t M32 = 2891336453U;
    uint32_t M32_inv = inverse_u32(M32);
    printf("INV %08x %s\n", M32_inv, M32_inv*M32 == 1?"..ok":"..fail");
    uint64_t M_inv = inverse_u64(M);
    printf("INV %016llx %s\n", M_inv, M_inv*M == 1?"..ok":"..fail");
    int n;
    uint64_t M_div = div_c64(M-1, &n);
    printf("DIV (A*%016llx)>>%d\n", M_div, n-64);
    uint64_t g = 1;
    int i, count = 0xFFFFFFF;
    for (i=0; i<count; i++, g*=M)
        if (((((g*(uint128_t)M_div)>>64)+g)>>(n-64)) != g/(M-1))
            break;
    printf("DIV ..%s\n", (i==count)? "ok":"fail");

    uint128_t M128 = UINT64_C(0xDA942042E4DD58B5);
    uint128_t M128_inv = inverse_u128(M128);
    printf("INV %016llx,%016llx %s\n", (uint64_t)(M128_inv>>64), (uint64_t)(M128_inv), M128_inv*M128 == 1?"..ok":"..fail");

    uint128_t P128 = UINT128_C(0x5851F42D4C957F2D,0x14057B7EF767814F);
    uint128_t P128_inv = inverse_u128(P128);
    printf("INV %016llx,%016llx %s\n", (uint64_t)(P128_inv>>64), (uint64_t)(P128_inv), P128_inv*P128 == 1?"..ok":"..fail");

    uint64_t s[2]={-1,1};
    for (i = 0; i< 255; i++)
        PCG_next(s);
    for (; i-->0;)
        PCG_prev(s);
	if (s[0] ==-1 && s[1]==1) printf("PCG   ..ok\n");

    for (i = 0; i< 255; i++)
        PCG64_next(s);
    for (; i-->0;)
        PCG64_prev(s);
	if (s[0] ==-1 && s[1]==1) printf("PCG64 ..ok\n");

    for (i = 0; i< 255; i++)
        PCG128_next(s);
    for (; i-->0;)
        PCG128_prev(s);
	if (s[0] ==-1 && s[1]==1) printf("PCG128..ok\n");

    return 0;
}

/*! \brief Деление на константу 
    A/B == ((A*C0)>>64 + A)>>(n-64)
 */
static uint64_t div_c64(uint64_t b, int *nd) 
{
	unsigned __int128 C;
	int k = __builtin_ctzll(b);// количество ноликов в младщей части числа
	b>>=k;
	if (b==1) {
		*nd = 64;
		C = (unsigned __int128)1<<64;
	} else
	{
		*nd = 128 - __builtin_clzll(b);// количество ноликов в старшей части числа
		C = (unsigned __int128)((((unsigned __int128)1<<64)-b)<<(*nd-64))/b 
          +((unsigned __int128)1<<(*nd-64))+1;
	}
	*nd+=k;
	return C;
}