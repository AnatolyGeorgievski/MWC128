#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef unsigned int __attribute__((mode(TI)))   uint128_t;

#define MAX_SIZE 0xFFFF
#define MAX_VAL  0x1FFFF
uint32_t prime[MAX_SIZE];
/*! Тестирование простоты по таблице простых чисел */
static bool is_prime(uint64_t a, int size){
	for (int i=0; i<size; i++)
		if (a%prime[i]==0) return false;
	return true;
}
/*! Возведение в степнь по модулю */
static uint64_t powm(const uint64_t b, uint64_t a, const uint64_t N)
{
	uint64_t r = b;
	uint64_t s = 1;
    while (a!=0) {
		if (a&1) 
			s = ((uint128_t)s*r)%N;
		r = ((uint128_t)r*r)%N;
		a>>=1;
	}
	return s;
}
/*! Модульное умножение */
static uint64_t mulm(uint64_t a, uint64_t b, uint64_t N){
	return ((uint128_t)a*b)%N;
}
/*! Модульное возведение в квадрат */
static uint64_t sqrm(uint64_t v, uint64_t N){
	return ((uint128_t)v*v)%N;
}
/*! Модульное уполовинивание */
static uint64_t hlvm(uint64_t v, uint64_t N){
	return (v&1)? ((uint128_t)v+N)>>1: (v>>1);
}
/*! Модульное сложение */
static uint64_t addm(uint64_t a, uint64_t b, uint64_t N){
	uint64_t s = a+b;
	if (s < b || s >= N) s -= N;
	return s;
}
/*! Модульное вычитание */
static uint64_t subm(uint64_t a, uint64_t b, uint64_t N){
	uint64_t s;
	if (a < b) s = N - b + a;
	else s = a - b;
	return s;
}
#define BIT(x,n) (((x)>>(n))&1)
#define SWAP(x,y) do {    \
   typeof(x) _x = x;      \
   typeof(y) _y = y;      \
   x = _y;                \
   y = _x;                \
 } while(0)
/*! Символ Якоби */
int jacobi(uint64_t a, uint64_t m) 
{
	a = a%m;
	int t = 1;
	unsigned m1= BIT(m,1);
	while (a!=0){
		int z = __builtin_ctzll(a);
		a = a>>z;
		unsigned a1= BIT(a,1);
		if((BIT(z,0)&(m1^BIT(m,2))) ^ (a1&m1)) t = -t;
		SWAP(a,m);
		m1= a1;
		a = a%m;
	}
	if (m!=1) return 0;
	return t;
}
/* 

U_{2n} = V_n U_n
V_{2n} = V_n^2 -2Q^n

2U_{n+1}= P U_n +   V_n
2V_{n+1}= P V_n + D U_n 
-- константы: P, Q, D = P^2 - 4Q
 */
typedef struct {
	uint64_t u,v;
} lucas_t;
/*! Последовательность Люка: удвоение точки 2n */
static lucas_t lucas_dbl(lucas_t x, uint64_t N)
{
	lucas_t r;
	r.u = mulm(x.u, x.v, N);
	r.v = subm(sqrm(x.v, N), 2, N);
	return r;
}
/*! Последовательность Люка: добавление точки n+1 */
static lucas_t lucas_add(lucas_t a, uint64_t p, uint64_t d, uint64_t N)
{
	lucas_t r;
	r.u = hlvm(addm(mulm(a.u, p, N), a.v, N), N);
	r.v = hlvm(addm(mulm(a.u, d, N), mulm(a.v, p, N), N), N);
	return r;
}
/* умножение слева-направо v_k */ 
static uint64_t lucas_mul(uint64_t p, uint64_t k, uint64_t N)
{
	lucas_t  v = {.u=1, .v=p};
	uint64_t d = subm(sqrm(p, N), 4, N);
	int i = 30-__builtin_clz(k);
	for(; i>=0; --i) { 
		v = lucas_dbl(v, N);
		if ((k>>i)&1) 
			v = lucas_add(v, p, d, N);
	}
	return v.v;
}
/*! \brief Тест LLR Люка-Лемеля-Ризеля 
 */
int llr_test(uint64_t k)
{
	int Q = 1, P=3, n=32;	
	uint64_t N = (k<<n)-1;
	while( !(jacobi(P-2, N)==1 && jacobi(P+2, N)==-1)) P++;
	uint64_t v = P;
	v = lucas_mul(v, k, N);
	for(int i=2; i<n; i++)
		v = subm(sqrm(v, N), 2, N);
	return (v==0);
}
int main(int argc, char* argv[]){
	
	uint64_t a = 3;
	int i=0;
	prime[i++] = 2;
	for(; i<MAX_SIZE;a+=2){// таблица простых чисел
		if (a>MAX_VAL) break;
		if (is_prime(a, i)) { 
			prime[i++] = a;
		}
	}
	printf("Max prime[%d] =%08x\n", i, prime[i-1]); 
	uint32_t A[] = {
0xfffefd4e,0xfffefaa5,0xfffef712,0xfffef592,0xfffef0e2,0xfffeea1f,0xfffee9ad,0xfffee7b2,
0xfffee4e8,0xfffee42b,0xfffee2b4,0xfffedf2a,0xfffede76,0xfffedb10,0xfffed89a,0xfffed462,
0xfffed3ae,0xfffed024,0xfffecd60,0xfffecc43,0xfffecbc5,0xfffeca90,0xfffec289,0xfffebaeb,
0xfffeb81b,0xfffeb6f8,0xfffeaf24,0xfffea942,0xfffea759,0xfffea405,0xfffea2f7,0xfffe9e7a,
0xfffe9e65,0xfffe9835,0xfffe982c,0xfffe9736,0xfffe9664,0xfffe944e,0xfffe930d,0xfffe915a,
0xfffe9001,0xfffe8e66,0xfffe8c9b,0xfffe83aa,0xfffe81f1,0xfffe80ad,0xfffe7fa2,0xfffe7a2c,
0xfffe7594,0xfffe7369,0xfffe6fee,0xfffe6feb,0xfffe636a,0xfffe6337,0xfffe60af,0xfffe5bd5,
0xfffe5a0a,0xfffe59b0,0xfffe59a7,0xfffe53e3,0xfffe51fd,0xfffe4fcf,0xfffe4858,0xfffe4456,
0xfffe42fd,0xfffe3b6e,0xfffe398b,0xfffe358f,0xfffe2a76,0xfffe29b9,0xfffe1d0b,0xfffe1c03,
0xfffe1b3a,0xfffe1702,0xfffe1495,0xfffe0f01,0xfffe03e2,0xfffe03c4,0xfffe00f4,
		0};
	for (int k=0; A[k]!=0; k++){
		a = ((uint64_t)A[k]<<32)-1;
		if (llr_test(A[k])) 
			printf("%2d: A=0x%08X prime =%016llx %2d %d\n",k, A[k], a, a%24, A[k]%3);
		if (is_prime(((uint64_t)A[k]<<31)-1,i)) // safe prime
			printf("..safe prime\n");
	}
	return 0;
}