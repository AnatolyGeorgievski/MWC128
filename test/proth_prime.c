#include <stdint.h>
#include <stdbool.h>

#define BIT(x,n) (((x)>>(n))&1)
#define SWAP(x,y) do {    \
   typeof(x) _x = x;      \
   typeof(y) _y = y;      \
   x = _y;                \
   y = _x;                \
 } while(0)

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
/*! Возведение в степнь по модулю */
static uint64_t powm(const uint64_t b, uint64_t a, const uint64_t N)
{
	uint64_t r = b;
	uint64_t s = 1;
    while (a!=0) {
		if (a&1) 
			s = ((unsigned __int128)s*r)%N;
		r = ((unsigned __int128)r*r)%N;
		a>>=1;
	}
	return s;
}
/*! \brief Тест простоты для чисел Прота вида p = a*2^s + 1 */
int Proth_test (uint64_t p){
	uint64_t g = 3;
	while(jacobi(g, p)!=-1) g+=2;
	return powm(g, p-1, p)==1 && /* powm(g, (p-1)/2, p)!=1 */ powm(g, (p-1)/2, p)==(p-1);
}

#define MAX_SIZE 0x1FFFF
#define MAX_VAL  0x3FFFF
uint64_t prime[MAX_SIZE];
static bool is_prime(uint64_t a, int size){
	for (int i=0; i<size; i++)
		if (a%prime[i]==0) return a==prime[i];
	return true;
}
#include <stdio.h>
int main(int argc, char* argv[]){
	
	uint64_t a = 3;
	int i=0;
	prime[i++] = 2;
	for(; i<MAX_SIZE;a+=2){
		if (a>MAX_VAL) break;
		if (is_prime(a, i)) { 
			prime[i++] = a;
			//if (a> 0xFF00) printf("%04X,", a);
			//if (a>=0xFFFFF) break;
		}
	}
	printf("Max prime[%d] =%08x\n", i, prime[i-1]); 

	printf("test ab+1 primes\n");
	for (uint64_t k=1; k<=0x7FFFF; k++){
		a = (1uLL<<63) - (k<<32)+1;
		uint64_t a0 = (a-1)>>__builtin_ctz(a-1);
		if (is_prime(a0,i))
		if (is_prime(a,i))
		if (Proth_test(a))
			printf("0x%0llx, ", a);
	}
	return 0;
}