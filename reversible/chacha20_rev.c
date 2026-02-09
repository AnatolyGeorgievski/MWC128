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

#define Nr 10 // число раундов

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
#define ROTL(v, n) (((v)<<(n)) ^ ((v)>>(32-(n))))

typedef uint32_t uint32x4_t __attribute__((__vector_size__(16)));
void ChaCha20(uint32x4_t *s, const uint32x4_t *S0) 
{
	uint32x4_t a = S0[0];
	uint32x4_t b = S0[1];
	uint32x4_t c = S0[2];
	uint32x4_t d = S0[3];
	int i;
__asm volatile("# LLVM-MCA-BEGIN ChaCha20");
	for (i=0;i<Nr;i++) {
		a += b; d = ROTL(d^a,16);
		c += d; b = ROTL(b^c,12);
		a += b; d = ROTL(d^a, 8);
		c += d; b = ROTL(b^c, 7);
		b = SHUFFLE(b, 1, 2, 3, 0);
		c = SHUFFLE(c, 2, 3, 0, 1);
		d = SHUFFLE(d, 3, 0, 1, 2);
		a += b; d = ROTL(d^a,16);
		c += d; b = ROTL(b^c,12);
		a += b; d = ROTL(d^a, 8);
		c += d; b = ROTL(b^c, 7);
		b = SHUFFLE(b, 3, 0, 1, 2);
		c = SHUFFLE(c, 2, 3, 0, 1);
		d = SHUFFLE(d, 1, 2, 3, 0);
	}
	s[0] += a;
	s[1] += b;
	s[2] += c;
	s[3] += d;
__asm volatile("# LLVM-MCA-END ChaCha20");
}

/*! \brief обратное преобразование salsa20_8 */ 
void ChaCha20_rev(uint32x4_t *S, const uint32x4_t *S0)
{
	uint32x4_t a,b,c,d;
    a = S[0] - S0[0];
    b = S[1] - S0[1];
    c = S[2] - S0[2];
    d = S[3] - S0[3];
__asm volatile("# LLVM-MCA-BEGIN iChaCha20");
	for (int i=Nr;i-->0;) {
		d = SHUFFLE(d, 3, 0, 1, 2);
		c = SHUFFLE(c, 2, 3, 0, 1);
		b = SHUFFLE(b, 1, 2, 3, 0);
		b = ROTR(b, 7)^c; c -= d;
		d = ROTR(d, 8)^a; a -= b;
		b = ROTR(b,12)^c; c -= d;
        d = ROTR(d,16)^a; a -= b;
		d = SHUFFLE(d, 1, 2, 3, 0);
		c = SHUFFLE(c, 2, 3, 0, 1);
		b = SHUFFLE(b, 3, 0, 1, 2);
		b = ROTR(b, 7)^c; c -= d;
		d = ROTR(d, 8)^a; a -= b;
		b = ROTR(b,12)^c; c -= d;
        d = ROTR(d,16)^a; a -= b;
    }
__asm volatile("# LLVM-MCA-END iChaCha20");
	S[0] = a;
	S[1] = b;
	S[2] = c;
	S[3] = d;
}

int main(){
    uint32x4_t B [4] = {0};
    uint32x4_t S [4] = {0x01020304,0x05060708,0x090A0B0C,0x0D0E0F10};
    uint32x4_t S0[4];
    for (int i=0; i<4; i++) S0[i] = S[i]^B[i];
    printf("forward\n");
	for (int i=0; i<4; i++) 
        print_vec(S0[i]);
    ChaCha20(S,S0);
    printf("reverse\n");
    ChaCha20_rev(S,S0);
	for (int i=0; i<4; i++) 
        print_vec(S[i]);

}