#include <stdint.h>
#include <stdio.h>
/*! \brief наибольший общий делитель GCD, бинарный алгоритм */
uint32_t gcd(uint32_t u, uint32_t v) {
    if (u == 0) {
        return v;
    } else if (v == 0) {
        return u;
    }
    int i = __builtin_ctz(u);  u >>= i;
    int j = __builtin_ctz(v);  v >>= j;
    int k = (i<j)?i:j;

    while(u!=v) {// u,v нечетные
        if (u > v){// _swap(&u, &v);
			u -= v; // u теперь четное
			u >>= __builtin_ctz(u);
        } else {
			v -= u; // v теперь четное
			v >>= __builtin_ctz(v);
		}
    }
	return v << k;
}



int main(){
	printf("=%d\n", gcd(25,15));
	printf("=%d\n", gcd(13,39));
	printf("=%d\n", gcd(12,36));
	printf("=%d\n", gcd(36,2));
	printf("=%d\n", gcd(36,37));
	return 0;
}
