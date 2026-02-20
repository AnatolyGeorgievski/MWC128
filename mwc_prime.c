#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#define MP_SIZE 2
#include "mp.c" // шаблон класса для заданной размерности MP_SIZE

/*! \brief тест на равенство */
static inline int mp_equ_ui(const uint64_t* s, uint64_t a){
	for (int i=1;i<MP_SIZE; i++) if(s[i]!=0) return 0;
	return (a==s[0]);
}
#define MAX_SIZE 0x1FFFF
uint32_t prime[MAX_SIZE];
uint32_t prime_size=0;
static bool is_prime(uint64_t a, int size){
	for (int i=0; i<size && (a>>1)>=prime[i]; i++)
		if (a%prime[i]==0) return false;
	return true;
}
void genprimes()
{
	printf("generate primes..");
	uint32_t a = 3;
	int i=0;
	prime[i++] = 2;
	for(; i<MAX_SIZE;a+=2){
		if (is_prime(a, i)) { 
			prime[i++] = a;
			//if (a> 0xFF00) printf("%04X,", a);
			// if (a>=0x7FFFF) break;
		}
	}
	printf("%d\n", i);
	prime_size = i;
}
/*! вычисление остатка от деления большого числа */
uint32_t rem(const uint64_t* a, uint32_t b){
	
	uint64_t r = 0;
	for(int i=MP_SIZE-1; i>=0; --i){
		r = ((a[i]>>32)+ (r<<32))%b;
		r = ((UINT64_C(0xFFFFFFFF)&a[i])+(r<<32))%b;
	}
	return r;
}
static bool mp_is_prime(uint64_t* a, int size){
	for (int i=0; i<size; i++)
		if (rem(a,prime[i])==0) return false;
	return true;
}
/* Сдвиг вправо */
static void mp_shr(uint64_t* p, int sh){
	int64_t cy = 0;
	for (int i = MP_SIZE-1; i>=0; --i){
		uint64_t v = p[i];
		p[i] = (v>>sh) | (cy<<(64-sh));
		cy = v;
	}
}
int powm_tst2(const uint64_t* p,  int size){
	uint64_t a[MP_SIZE];
	uint64_t b[MP_SIZE];
	uint64_t s[MP_SIZE];
	mp_mov(a, p);
	mp_clr(b); b[0] = 2;
	a[0] -=1;// переноса не предвидится
	for(int i=0; i< size; i++){
		mp_powm(s, b, a, p);
		if (!mp_equ_ui(s, 1u)) return 0;
		b[0] = prime[i+1];
	}
	return 1;
}
bool has_max_order(uint64_t x) {
    int count = 61; // 2^61
    do { 
		x = x * x; // x ← x² mod 2⁶⁴
		if (x == 1) return false; 
	} while (--count);
    return true;
}
bool has_max_order32(uint32_t x) {
    int count = 29;
    do { 
		x = x * x; // x ← x² mod 2^{32}
		if (x == 1) return false; 
	} while (--count);
    return true;
}
int main(){
	genprimes();
	uint64_t a;
	uint64_t p[MP_SIZE] = {-1, -1};
if (1) {// генерация констант для MWC128
	a = UINT64_C(0xffebb71d94fcdaf9);
	p[MP_SIZE-1]  = a-1;
	if (powm_tst2(p, 25)){
		printf("%016llx ..prime\n", a);
	}
	if ((a%3)==0 && is_prime((~a)-1, prime_size)) printf(" ..ok\n");
	mp_shr(p,1);
	if (powm_tst2(p, 2)) printf("P/2 ..prime\n");
	if (0){// другой вариант стартовой точки
		a = UINT64_C(0xff3a275c007b8ee6);
		p[MP_SIZE-1]  = a-1;p[0]=-1;
		mp_shr(p,1);
		if (powm_tst2(p, 2)) printf("P/2 ..prime\n");
	}
	if(a%3==0) 
	for (uint64_t k=0; k<0x3FFFFFF; k++, a+=3){// вычисление начиная с данного a
		p[MP_SIZE-1]  = a-1;p[0]=-1;
		if (powm_tst2(p, 5))// тест Ферма для первых 5 простых чисел
		//if ((a%3)==0) -- все делятся на 3
		//if (rem(p, 24)==23) -- все делятся
		if (has_max_order(a)) // максимальный порядок группы {Z/2^{64}Z}+
		if (is_prime((~a)-1, prime_size)) // -- мой тест, выбор чисел A может быть основан на множесте простых чисел
		if (mp_is_prime(p, prime_size))
		{// printf("!");
			mp_shr(p,1);
			if (mp_is_prime(p, prime_size) && powm_tst2(p, 5))// тест на safe prime
				printf("%016llx ..prime\n", a);
		}
	}
}
if (0) {// генерация констант для MWC64r1 
	a = 0xFFFFFFFFuLL;
	while (a%3 !=0) a -= 1;
	if(a%3==0) 
	for (uint64_t k=0; k<0x3FFFFFF; k++, a-=3){
		p[MP_SIZE-1]  = a-1;p[0]=-1;
		if (powm_tst2(p, 5))// тест Ферма для первых 5 простых чисел
		//if ((a%3)==0) -- все делятся на 3
		//if (rem(p, 24)==23) -- все делятся
		if (is_prime((uint32_t)(~a)-1, prime_size)) // -- мой тест, выбор чисел A может быть основан на множесте простых чисел
		if (mp_is_prime(p, prime_size))
		{// printf("!");
			mp_shr(p,1);
			if (mp_is_prime(p, prime_size) && powm_tst2(p, 5))// тест на safe prime
				printf("%016llx ..prime\n", a);
		}
	}
}
if (1) {// генерация констант для MWC64r2
	a = (0xFFFFFFFFuLL)<<32;
	while ((a>>32)%3 !=0) a -= (1uLL<<32);

	if((a>>32)%3==0) 
	for (uint64_t k=0; k<0x3FFFFFF; k++, a+=(3uLL<<32)){// вычисление начиная с данного a
		p[MP_SIZE-1]  = a-1;p[0]=-1;
		if (powm_tst2(p, 5))// тест Ферма для первых 5 простых чисел
		//if ((a%3)==0) -- все делятся на 3
		//if (rem(p, 24)==23) -- все делятся
		//if (has_max_order32(a>>32)) // максимальный порядок группы {Z/2^{64}Z}+
		if (is_prime((~(a>>32))-1, prime_size)) // -- мой тест, выбор чисел A может быть основан на множестве простых чисел
		if (mp_is_prime(p, prime_size))
		{// printf("!");
			mp_shr(p,1);
			if (mp_is_prime(p, prime_size) && powm_tst2(p, 5))// тест на safe prime
				printf("%016llx ..prime\n", a);
		}
	}
}
	return 0;
}