/*!	\author __Анатолий М. Георгиевский, ИТМО__, 2026

Функцию Salsa можно представить как 
    S = P(S) -- операция смешивания слов и бит
Обратное преобразование
    S = P^{-1}(S) -- обратное операция смешивания слов и бит

Обратная операция получается если все действия в цикле выполнять 
в обратном порядке, а для операторов типа SHUFFLE выполнять 
обратное преобразование. В данном случае SHUFFLE выполняет 
ротацию слов влево или вправо, без потери информации - операция обратима.

    \see https://datatracker.ietf.org/doc/html/rfc7914.html
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define Nr 8 // число раундов

#define memcpy __builtin_memcpy
#ifndef ROTL
#define ROTL(v, n) (((v)<<(n)) ^ ((v)>>(32-(n))))
#endif
#ifndef ROTR
#define ROTR(v, n) (((v)>>(n)) ^ ((v)<<(32-(n))))
#endif
#define SHUFFLE(v, a,b,c,d) __builtin_shuffle((v), (uint32x4_t){a,b,c,d})
#define SHUFFLE2(v, w, a,b,c,d) __builtin_shuffle((v),(w), (uint32x4_t){a,b,c,d})
typedef uint32_t uint32x4_t __attribute__((__vector_size__(16)));
static void print_vec(uint32x4_t S){
	for(int i=0; i<4; i++)
		printf("%08X,", S[i]);
    printf("\n");
}
/*! \brief прямое преобразование salsa20_8 */ 
void salsa20_8(uint32x4_t *S, const uint32x4_t *S0)
{
	uint32x4_t x0,x1,x2,x3;
    x0 = S0[0];//^=Bx[0]; - операция XOR вынесена наружу
    x3 = S0[1];//^=Bx[1];
    x2 = S0[2];//^=Bx[2];
    x1 = S0[3];//^=Bx[3];
__asm volatile("# LLVM-MCA-BEGIN SALSA");
#pragma GCC unroll 8
	for (int i=0;i<2;i+=2) {
		x1 = SHUFFLE(x1, 1, 2, 3, 0);
		x2 = SHUFFLE(x2, 2, 3, 0, 1);
		x3 = SHUFFLE(x3, 3, 0, 1, 2);
		x1 ^= ROTL(x0 + x3, 7);
		x2 ^= ROTL(x1 + x0, 9);
		x3 ^= ROTL(x2 + x1,13);
		x0 ^= ROTL(x3 + x2,18);
		x3 = SHUFFLE(x3, 1, 2, 3, 0);
		x2 = SHUFFLE(x2, 2, 3, 0, 1);
		x1 = SHUFFLE(x1, 3, 0, 1, 2);
		x3 ^= ROTL(x0 + x1, 7);
		x2 ^= ROTL(x3 + x0, 9);
		x1 ^= ROTL(x2 + x3,13);
		x0 ^= ROTL(x1 + x2,18);
    }
__asm volatile("# LLVM-MCA-END SALSA");
    S[0] = S0[0] + x0;
	S[1] = S0[1] + x3;
	S[2] = S0[2] + x2;
	S[3] = S0[3] + x1;
}
/*! \brief обратное преобразование salsa20_8 */ 
void salsa20_8_rev(uint32x4_t *S, const uint32x4_t *S0)
{
	uint32x4_t x0,x1,x2,x3;
    x0 = S[0] - S0[0];
    x3 = S[1] - S0[1];
    x2 = S[2] - S0[2];
    x1 = S[3] - S0[3];
__asm volatile("# LLVM-MCA-BEGIN iSALSA");
#pragma GCC unroll 8
	for (int i=2;(i-=2)>=0;) {
		x0 ^= ROTL(x1 + x2,18);
		x1 ^= ROTL(x2 + x3,13);
		x2 ^= ROTL(x3 + x0, 9);
		x3 ^= ROTL(x0 + x1, 7);

		x1 = SHUFFLE(x1, 1, 2, 3, 0); 
		x2 = SHUFFLE(x2, 2, 3, 0, 1);
		x3 = SHUFFLE(x3, 3, 0, 1, 2);

		x0 ^= ROTL(x3 + x2,18);
		x3 ^= ROTL(x2 + x1,13);
		x2 ^= ROTL(x1 + x0, 9);
		x1 ^= ROTL(x0 + x3, 7);

		x3 = SHUFFLE(x3, 1, 2, 3, 0);
		x2 = SHUFFLE(x2, 2, 3, 0, 1);
		x1 = SHUFFLE(x1, 3, 0, 1, 2);
    }
__asm volatile("# LLVM-MCA-END iSALSA");
	S[0] = x0;
	S[1] = x3;
	S[2] = x2;
	S[3] = x1;
}

int main(){
    uint32x4_t B [4] = {0};
    uint32x4_t S [4] = {0x01020304,0x05060708,0x090A0B0C,0x0D0E0F10};
    uint32x4_t S0[4];
    for (int i=0; i<4; i++) S0[i] = S[i]^B[i];
    printf("forward\n");
	for (int i=0; i<4; i++) 
        print_vec(S0[i]);
    salsa20_8(S,S0);
    printf("reverse\n");
    salsa20_8_rev(S,S0);
	for (int i=0; i<4; i++) 
        print_vec(S[i]);

}