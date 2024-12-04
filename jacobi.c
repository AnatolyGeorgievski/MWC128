/* a/n is represented as (a,n)

	\see https://en.wikipedia.org/wiki/Jacobi_symbol
	
Тесты LLR рассматривают два случая когда P ≡ 7 (mod 24) and the Lucas V_k(4,1). 


И вариант с подбором p: jacobi(p-2, n)=+1 and jacobi(p+2, n)=-1 для последовательности V_k(p,1) в случае A mod 3 =0

 */
#if 0
int jacobi_(int a, int n) {
    //assert(n > 0 && n%2 == 1);
    // Step 1
    a = (a % n + n) % n; // Handle (a < 0)
    // Step 3
    int t = 0;   // XOR of bits 1 and 2 determines sign of return value
    while (a != 0) {
        // Step 2
        while (a % 4 == 0)
            a /= 4;
        if (a % 2 == 0) {
            t ^= n;  // Could be "^= n & 6"; we only care about bits 1 and 2
            a /= 2;
        }
        // Step 4
        t ^= a & n & 2;  // Flip sign if a % 4 == n % 4 == 3
        int r = n % a;
        n = a;
        a = r;
    }
    if (n != 1)
        return 0;
    else if ((t ^ (t >> 1)) & 2)
        return -1;
    else
        return 1;
}
#endif

#include <stdio.h>
#include <stdint.h>
#define BIT(x,n) (((x)>>(n))&1)
#define SWAP(x,y) do {    \
   typeof(x) _x = x;      \
   typeof(y) _y = y;      \
   x = _y;                \
   y = _x;                \
 } while(0)
/*! \brief Расчет Символа Якоби (вычеты)

The Jacobi symbol, named after the esteemed German mathematician Carl Gustav Jacob Jacobi (1804-1851), is a generalization of the Legendre symbol, $({a \over b}) \in \{-1,0,1\}$, where for $a$ prime number $m$,

```math
({a \over m}) = 
\begin{cases}
 0 & if p | a \\
 1 & \text{if } a \equiv b2 \bmod p \text{for some } b,\\
−1 & \text{if } a ̸≡     b2 \bmod p for all b,
\end{cases}
```
* [1] "Optimized Computation of the Jacobi Symbol" Jonas Lindstrøm & Kostas Kryptos Chalkias 
	<https://eprint.iacr.org/2024/1054>

	\see <https://en.wikipedia.org/wiki/Jacobi_symbol>

 */

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
#ifdef TEST_JACOBI
int main(){
	int j = jacobi(1001,9907);
	printf("j=%d\n", j);
	printf("j=%d\n", jacobi(19,45));
	printf("j=%d\n", jacobi( 8,21));
	printf("j=%d\n", jacobi( 5,21));
	return 0;
}
#endif