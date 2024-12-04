// prime.c
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_SIZE 0x7FFFF
#define MAX_VAL  0x1FFFFF
uint32_t prime[MAX_SIZE];
static bool is_prime(uint32_t a, int size){
	for (int i=0; i<size; i++)
		if (a%prime[i]==0) return false;
	return true;
}
int main(int argc, char* argv[]){
	
	uint32_t a = 3;
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
	//if (is_prime(5778,i)) printf("prime =%d\n",5778);
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
	for (int k=0; A[k]!=0; k++){
		a = (A[k]<<16)-1;
		if (is_prime(a,i)) printf("%2d: prime =%08x %2d %d\n",k,a, a%24, A[k]%3);
		if (is_prime((A[k]<<15)-1,i)) printf("..ok\n");
	}
if (0){
	printf("Lucas_V(4,1, k) mod P=\n"); 
	int Q = 1, P=4;
	uint32_t N  = (0xFEA0<<16)-1;
	uint32_t zu = 2, u = P;
	for(int i=2;i<0xFEA0; i++){
		uint32_t y = ((uint64_t)P*u - Q*zu)%N;
		zu = u; u = y;
		printf("%d,",y);
	}
	printf("\n");
}
	return 0;
}