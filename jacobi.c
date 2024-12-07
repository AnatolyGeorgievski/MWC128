/* a/n is represented as (a,n)

	\see https://en.wikipedia.org/wiki/Jacobi_symbol
	
Тесты LLR рассматривают два случая:
1) когда P ≡  7 (mod 24) and the Lucas V_k(4,1). 
2) когда P ≡ 23 (mod 24) с подбором p: jacobi(p-2, n)=+1 and jacobi(p+2, n)=-1 
для последовательности Люка V_k(p,1) в случае A mod 3 =0

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
 0 & if a \equiv 0   (\bmod p) \\
 1 & if a \equiv b^2 (\bmod p) \text{for some } b,\\
−1 & if a ̸≡     b2 (\bmod p) for all b,
\end{cases}
```
* [1] "Optimized Computation of the Jacobi Symbol" Jonas Lindstrøm & Kostas Kryptos Chalkias 
	<https://eprint.iacr.org/2024/1054>

> Символ Якоби является важным примитивом в криптографических приложениях, таких как проверка простоты, факторизация целых чисел и различные схемы шифрования. 

* [2] "A simpler alternative to Lucas–Lehmer–Riesel primality test" Pavel Atnashev
	<https://eprint.iacr.org/2023/195>
	
> В этой статье рассматривается применение теста Моррисона на простоту к числам вида $k \cdot 2^n-1$ и находится простая общая формула, эквивалентная тесту Люка — Лемера и тесту  Люка — Лемера — Ризеля на простоту.

> Реализация теста Моррисона с Q = -1 и последовательностью Люка V математически неотличима
от реализации теста Люка–Лемера–Ризеля с исходным значением R¨odseth.

```math
V_{2^{n-2}}(V_k(P_R, Q_R)) \equiv (\bmod{N}), \quad Q_R = 1, (\frac{P_R-2}{N})=1, \quad (\frac{P_R+2}{N})=-1
```


Тест Люка–Лемера–Ризеля Примет вид
$$V_{2^{n-2}}(V_k(V_2)) = V_{k\cdot 2^{n-1}} = V_{(N+1)/2} \equiv (\bmod{N})$$
где
$$V_2 = P^2-2Q = P_R$$
	
\see <https://en.wikipedia.org/wiki/Jacobi_symbol>

D = P^2 - 4Q
\sqrt{D} = D^(N+1)/2

\alpha, \beta = (P \pm \sqrt{D})/2
V_n = \alpha^n + \beta^n
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
#if 0
/* Удвоение */
static uint64_t v_dub(uint64_t v, uint64_t N){
	return ((uint128_t)v*v - 2)%N;
}
#endif
typedef unsigned int __attribute__((mode(TI)))   uint128_t;
static uint64_t sqrm(uint64_t v, uint64_t N){
	return ((uint128_t)v*v)%N;
}
#ifdef TEST_JACOBI
int main(){
	int j = jacobi(1001,9907);
	printf("j=%d\n", j);
	printf("j=%d\n", jacobi(19,45));
	printf("j=%d\n", jacobi( 8,21));
	printf("j=%d\n", jacobi( 5,21));
	
	unsigned int p = 1; 
	uint64_t n = (0xFEA0<<16)-1;
	printf("j(%d,n) = %d\n", 3, jacobi(3,n));
	while( !(jacobi(p*p-4, n)==1 && jacobi(p*p, n)==-1)) p++;
	printf("p = %d, P_R=%d\n", p, sqrm(p,n)+2);
	p=3;
	while( !(jacobi(p-2, n)==1 && jacobi(p+2, n)==-1)) p++;
	printf("P_R=%d\n", p);


	n = 0xFFFEB81AFFFFFFFFull;
	p=2;
	while( !(jacobi(p*p-4, n)==1 && jacobi(p*p, n)==-1)) p++;
	printf("p = %d, P_R=%d D=%d\n", p, p*p-2, p*p);
	p=3;
	while( !(jacobi(p-2, n)==1 && jacobi(p+2, n)==-1)) p++;
	printf("P_R=%d\n", p);
	printf("j(%d,n) = %d\n", 9, jacobi(9,n));
	return 0;
}
#endif