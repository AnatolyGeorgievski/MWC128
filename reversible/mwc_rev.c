#include <stdint.h>
#include <stdio.h>
uint64_t mwc64x( uint64_t* state, const uint64_t A)
{
	uint64_t x = *state;
	*state = A*(uint32_t)(x) + (x>>32);
    return x;
}
uint32_t mwc32x( uint32_t* state, const uint32_t A)
{
	uint32_t x = *state;
	*state = A*(uint16_t)(x) + (x>>16);
    return x;
}
uint32_t mwc32_prev(uint32_t *state, const uint32_t A){
    const uint32_t p = (A<<16) - 1u;
    const uint32_t w = 0x1p48/p;
	uint32_t x = *state;
    uint32_t q = ((uint64_t)x*w)>>32;
    uint64_t r =((x - (uint64_t)q*A)<<16) + q;
    x = (r-p< r)? r - p: r;
	*state = x;
	return x;
}

#include <math.h>
int main (){
    uint32_t A1 = 0xFE94;
	uint32_t s[1] = {1};
    uint32_t Nr = (A1<<15)-2;
	for (int k=0; k<Nr; k++){
		mwc32x(s, A1);
	}
    printf(" = %x\n", s[0]);
	for (int k=0; k<Nr; k++){
		mwc32_prev(s, A1);
	}
	printf(" = %x\n", s[0]);
	return 0;
}