#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_SIZE 0xFFFF
#define MAX_VAL  0x1FFFF
uint32_t prime[MAX_SIZE];
/*! Тестирование простоты по таблице простых чисел */
static bool is_prime(uint32_t a, int size){
	for (int i=0; i<size; i++)
		if (a%prime[i]==0) return false;
	return true;
}
/*! Возведение в степнь по модулю */
static uint32_t powm(const uint32_t b, uint32_t a, const uint32_t N)
{
	uint32_t r = b;
	uint32_t s = 1;
    while (a!=0) {
		if (a&1) 
			s = ((uint64_t)s*r)%N;
		r = ((uint64_t)r*r)%N;
		a>>=1;
	}
	return s;
}
/*! Модульное умножение */
static uint32_t mulm(uint32_t a, uint32_t b, uint32_t N){
	return ((uint64_t)a*b)%N;
}
/*! Модульное возведение в квадрат */
static uint32_t sqrm(uint32_t v, uint32_t N){
	return ((uint64_t)v*v)%N;
}
/*! Модульное уполовинивание */
static uint32_t hlvm(uint32_t v, uint32_t N){
	return (v&1)? ((uint64_t)v+N)>>1: (v>>1);
}
/*! Модульное сложение */
static uint32_t addm(uint32_t a, uint32_t b, uint32_t N){
	uint32_t s = a+b;
	if (s < b || s >= N) s -= N;
	return s;
}
/*! Модульное вычитание */
static uint32_t subm(uint32_t a, uint32_t b, uint32_t N){
	uint32_t s;
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
	uint32_t u,v;
} lucas_t;
/*! Последовательность Люка: удвоение точки 2n */
static lucas_t lucas_dbl(lucas_t x, uint32_t N)
{
	lucas_t r;
	r.u = mulm(x.u, x.v, N);
	r.v = subm(sqrm(x.v, N), 2, N);
	return r;
}
/*! Последовательность Люка: добавление точки n+1 */
static lucas_t lucas_add(lucas_t a, uint32_t p, uint32_t d, uint32_t N)
{
	lucas_t r;
	r.u = hlvm(addm(mulm(a.u, p, N), a.v, N), N);
	r.v = hlvm(addm(mulm(a.u, d, N), mulm(a.v, p, N), N), N);
	return r;
}
/* умножение слева-направо v_k */ 
uint32_t lucas_mul(uint32_t p, uint32_t k, uint32_t N)
{
	lucas_t  v = {.u=1, .v=p};
	uint32_t d = subm(sqrm(p, N), 4, N);
	int i = 30-__builtin_clz(k);
	for(; i>=0; --i) { 
		v = lucas_dbl(v, N);
		if ((k>>i)&1) 
			v = lucas_add(v, p, d, N);
	}
	return v.v;
}
/*! \brief Тест LLR Люка-Лемеля-Ризеля 

	можно поменять местами (1) и (2), действует правило коммутативности
	V_m(V_k(P,Q), Q^k) = V_k(V_m(P,Q), Q^m)
 */
int llr_test(uint32_t k)
{
	int Q = 1, P=3;	
	uint32_t N  = (k<<16)-1;
	while( !(jacobi(P-2, N)==1 && jacobi(P+2, N)==-1)) P++;
	uint32_t  v = P;
// 2. 
	for(int i=2; i<16; i++)
		v = subm(sqrm(v, N), 2, N);
// 1.
#if 0
	uint32_t  p = v;
	uint32_t zv = 2;
	for(int i=1;i<k; i++){// надо сократить сложность!
		uint32_t y = subm(mulm(p,v,N), zv, N);
		zv = v; v = y;
	}
#else
	v = lucas_mul(v, k, N);
#endif
	//printf("Lucas V_{2^{n-2}}(V_k) = %x\n", v);
	return (v==0);
}
int main(int argc, char* argv[]){
	
	uint32_t a = 3;
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
		0x10000,0xFFEA, 0xFFD7, 0xFFBD, 
		0xFF9B, 0xFF81, 0xFF80, 0xFF7B, 
		0xFF75, 0xFF48, 0xFF3F, 0xFF3C,
		0xFF2C, 0xFF09, 0xFF03, 0xFF00,
		0xFF00, 0xFEE4, 0xFEA8, 0xFEA5,
		0xFEA0, 0xFE94, 0xFE8B, 0xFE72, 
		0xFE4E, 0xFE30, 0xFE22, 0xFE15, 
		0xFE04,
		0};
	for (int k=1; A[k]!=0; k++){
		a = (A[k]<<16)-1;
		if (is_prime(a,i))
		if (llr_test(A[k])) 
			printf("%2d: A=0x%04X prime =%08x %2d %d\n",k, A[k], a, a%24, A[k]%3);
		if (is_prime((A[k]<<15)-1,i)) // safe prime
			printf("..safe prime\n");
	}
	return 0;
}