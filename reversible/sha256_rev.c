// операция rol(x,n) == x*2^n + x*2^{n-32}
/* rol - циркулянт 
0 0 0 1 x0
1 0 0 0 x1
0 1 0 0 x2
0 0 1 0 x3
rol2:
0 0 1 0 x0
0 0 0 1 x1
1 0 0 0 x2
0 1 0 0 x3
rol1 ^ rol2
0 0 1 1 x0
1 0 0 1 x1
1 1 0 0 x2
0 1 1 0 x3

x & y ^ x ^ y  = x | y
0 0 0
0 1 1
1 0 1
1 1 1

x & y
*/

#define rotr(x,n) (((x)>>n) ^ ((x)<<(32-n)))
#define sigma0(x) (rotr(x, 7) ^ rotr(x,18) ^ ((x)>> 3))
#define sigma1(x) (rotr(x,17) ^ rotr(x,19) ^ ((x)>>10))
#define Sum0(x)		( rotr(x,2) ^ rotr(x,13) ^ rotr(x,22))
#define Sum1(x)		( rotr(x,6) ^ rotr(x,11) ^ rotr(x,25))

#define Ch(x, y, z)  ((x&y) ^ (~x&z))  /* bitselect(z, y, x) */
#define Maj(x, y, z) /*bitselect(z, y, z ^ x)*/((x&y) ^ (x&z) ^ (y&z))
//#define Maj(x, y, z) /*bitselect(x&y, x|y, z)*/((z&~x&~y) | (~z&x&y))

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


static const uint32_t K[64]={
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static const uint32_t H0[] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

void SHA256(uint32_t *H, const uint32_t *M, uint32_t *W_state){
    uint32_t W[16];
    int t;
	uint32_t a = H[0],b = H[1],c = H[2],d = H[3],
		     e = H[4],f = H[5],g = H[6],h = H[7];
	if(1)
	printf ("forward:  %08X %08X %08X %08X %08X %08X %08X %08X\n", a,b,c,d,e,f,g,h);
#define ROUND(a,b,c,d,e,f,g,h,i) ({\
	h += K[i] + W[i&15];  \
	uint32_t x = Sum1(e) + Ch (e,f,g); \
	uint32_t y = Sum0(a) + Maj(a,b,c); \
	d += x + h; \
	h += x + y; \
	})
    { t=0;
		W[0] = M[0];
        ROUND(a,b,c,d,e,f,g,h,t+0);
		W[1] = M[1];
        ROUND(h,a,b,c,d,e,f,g,t+1);
		W[2] = M[2];
        ROUND(g,h,a,b,c,d,e,f,t+2);
		W[3] = M[3];
        ROUND(f,g,h,a,b,c,d,e,t+3);
		W[4] = M[4];
        ROUND(e,f,g,h,a,b,c,d,t+4);
		W[5] = M[5];
        ROUND(d,e,f,g,h,a,b,c,t+5);
		W[6] = M[6];
        ROUND(c,d,e,f,g,h,a,b,t+6);
		W[7] = M[7];
        ROUND(b,c,d,e,f,g,h,a,t+7);

		W[8] = M[8];
        ROUND(a,b,c,d,e,f,g,h,t+8);
		W[9] = M[9];
        ROUND(h,a,b,c,d,e,f,g,t+9);
		W[10] = M[10];
        ROUND(g,h,a,b,c,d,e,f,t+10);
		W[11] = M[11];
        ROUND(f,g,h,a,b,c,d,e,t+11);
		W[12] = M[12];
        ROUND(e,f,g,h,a,b,c,d,t+12);
		W[13] = M[13];
        ROUND(d,e,f,g,h,a,b,c,t+13);
		W[14] = M[14];
        ROUND(c,d,e,f,g,h,a,b,t+14);
		W[15] = M[15];
        ROUND(b,c,d,e,f,g,h,a,t+15);
    }

if (1) {
	printf("forward:  W[] =");
	for (int i=0; i<15; i++) printf ("%02X ", W[i]);
	printf("\n");
}

	for (t=16; t<64; t+=16)
    {
		W[0] += sigma1(W[14]) + W[ 9] + sigma0(W[1]);
        ROUND(a,b,c,d,e,f,g,h,t+0);
		W[1] += sigma1(W[15]) + W[10] + sigma0(W[2]);
        ROUND(h,a,b,c,d,e,f,g,t+1);
		W[2] += sigma1(W[ 0]) + W[11] + sigma0(W[3]);
        ROUND(g,h,a,b,c,d,e,f,t+2);
		W[3] += sigma1(W[ 1]) + W[12] + sigma0(W[4]);
        ROUND(f,g,h,a,b,c,d,e,t+3);
		W[4] += sigma1(W[ 2]) + W[13] + sigma0(W[5]);
        ROUND(e,f,g,h,a,b,c,d,t+4);
		W[5] += sigma1(W[ 3]) + W[14] + sigma0(W[6]);
        ROUND(d,e,f,g,h,a,b,c,t+5);
		W[6] += sigma1(W[ 4]) + W[15] + sigma0(W[7]);
        ROUND(c,d,e,f,g,h,a,b,t+6);
		W[7] += sigma1(W[ 5]) + W[ 0] + sigma0(W[8]);
        ROUND(b,c,d,e,f,g,h,a,t+7);
		
		W[ 8] += sigma1(W[ 6]) + W[ 1] + sigma0(W[ 9]);
        ROUND(a,b,c,d,e,f,g,h,t+ 8);
		W[ 9] += sigma1(W[ 7]) + W[ 2] + sigma0(W[10]);
        ROUND(h,a,b,c,d,e,f,g,t+ 9);
		W[10] += sigma1(W[ 8]) + W[ 3] + sigma0(W[11]);
        ROUND(g,h,a,b,c,d,e,f,t+10);
		W[11] += sigma1(W[ 9]) + W[ 4] + sigma0(W[12]);
        ROUND(f,g,h,a,b,c,d,e,t+11);
		W[12] += sigma1(W[10]) + W[ 5] + sigma0(W[13]);
        ROUND(e,f,g,h,a,b,c,d,t+12);
		W[13] += sigma1(W[11]) + W[ 6] + sigma0(W[14]);
        ROUND(d,e,f,g,h,a,b,c,t+13);
		W[14] += sigma1(W[12]) + W[ 7] + sigma0(W[15]);
        ROUND(c,d,e,f,g,h,a,b,t+14);
		W[15] += sigma1(W[13]) + W[ 8] + sigma0(W[ 0]);
        ROUND(b,c,d,e,f,g,h,a,t+15);
	}
#undef ROUND
    H[0]+=a,H[1]+=b,H[2]+=c,H[3]+=d,
	H[4]+=e,H[5]+=f,H[6]+=g,H[7]+=h;	

	printf ("%08X %08X %08X %08X %08X %08X %08X %08X <\n", a,b,c,d,e,f,g,h);
	if (W_state) {
		for (int i=0; i<16; i++)
			W_state[i] = W[i];
	}
}

void SHA256_rev(uint32_t *H, const uint32_t *H0, const uint32_t *M, const uint32_t* W_state){
    uint32_t W[16];//={0};
	for (int i=0; i<16; i++)
		W[i] = W_state[i];


	int t;
	uint32_t a = H[0]-H0[0],b = H[1]-H0[1],c = H[2]-H0[2],d = H[3]-H0[3],
		     e = H[4]-H0[4],f = H[5]-H0[5],g = H[6]-H0[6],h = H[7]-H0[7];
// пошли обратно:
	H[0]-=a,H[1]-=b,H[2]-=c,H[3]-=d,
	H[4]-=e,H[5]-=f,H[6]-=g,H[7]-=h;	

#define ROUND(a,b,c,d,e,f,g,h,i) ({\
	uint32_t x = Sum1(e) + Ch (e,f,g); \
	uint32_t y = Sum0(a) + Maj(a,b,c); \
	h -= x + y; \
	d -= x + h; \
	h -= K[i] + W[i&15]; \
	})

    for (t=64-16; t>=16; t-=16)
    {
        ROUND(b,c,d,e,f,g,h,a,t+15);
		W[15] -= sigma1(W[13]) + W[ 8] + sigma0(W[ 0]);
		ROUND(c,d,e,f,g,h,a,b,t+14);
		W[14] -= sigma1(W[12]) + W[ 7] + sigma0(W[15]);
        ROUND(d,e,f,g,h,a,b,c,t+13);
		W[13] -= sigma1(W[11]) + W[ 6] + sigma0(W[14]);
        ROUND(e,f,g,h,a,b,c,d,t+12);
		W[12] -= sigma1(W[10]) + W[ 5] + sigma0(W[13]);
        ROUND(f,g,h,a,b,c,d,e,t+11);
		W[11] -= sigma1(W[ 9]) + W[ 4] + sigma0(W[12]);
        ROUND(g,h,a,b,c,d,e,f,t+10);
		W[10] -= sigma1(W[ 8]) + W[ 3] + sigma0(W[11]);
        ROUND(h,a,b,c,d,e,f,g,t+ 9);
		W[ 9] -= sigma1(W[ 7]) + W[ 2] + sigma0(W[10]);
        ROUND(a,b,c,d,e,f,g,h,t+ 8);
		W[ 8] -= sigma1(W[ 6]) + W[ 1] + sigma0(W[ 9]);

        ROUND(b,c,d,e,f,g,h,a,t+7);
		W[7] -= sigma1(W[ 5]) + W[ 0] + sigma0(W[8]);
        ROUND(c,d,e,f,g,h,a,b,t+6);
		W[6] -= sigma1(W[ 4]) + W[15] + sigma0(W[7]);
        ROUND(d,e,f,g,h,a,b,c,t+5);
		W[5] -= sigma1(W[ 3]) + W[14] + sigma0(W[6]);
        ROUND(e,f,g,h,a,b,c,d,t+4);
		W[4] -= sigma1(W[ 2]) + W[13] + sigma0(W[5]);
        ROUND(f,g,h,a,b,c,d,e,t+3);
		W[3] -= sigma1(W[ 1]) + W[12] + sigma0(W[4]);
        ROUND(g,h,a,b,c,d,e,f,t+2);
		W[2] -= sigma1(W[ 0]) + W[11] + sigma0(W[3]);
        ROUND(h,a,b,c,d,e,f,g,t+1);
		W[1] -= sigma1(W[15]) + W[10] + sigma0(W[2]);
        ROUND(a,b,c,d,e,f,g,h,t+0);
		W[0] -= sigma1(W[14]) + W[ 9] + sigma0(W[1]);
	}
if (1) {
	printf("backward: W[] =");
	for (int i=0; i<15; i++) printf ("%02X ", W[i]);
	printf("\n");
}
    { t=0;
        ROUND(b,c,d,e,f,g,h,a,t+15);
		//W[15] = M[15];
        ROUND(c,d,e,f,g,h,a,b,t+14);
		//W[14] = M[14];
        ROUND(d,e,f,g,h,a,b,c,t+13);
		//W[13] = M[13];
        ROUND(e,f,g,h,a,b,c,d,t+12);
		//W[12] = M[12];
		
        ROUND(f,g,h,a,b,c,d,e,t+11);
		//W[11] = M[11];
        ROUND(g,h,a,b,c,d,e,f,t+10);
		//W[10] = M[10];
        ROUND(h,a,b,c,d,e,f,g,t+9);
		//W[9] = M[9];
        ROUND(a,b,c,d,e,f,g,h,t+8);
		//W[8] = M[8];

        ROUND(b,c,d,e,f,g,h,a,t+7);
		//W[7] = M[7];
        ROUND(c,d,e,f,g,h,a,b,t+6);
		//W[6] = M[6];
        ROUND(d,e,f,g,h,a,b,c,t+5);
		//W[5] = M[5];
        ROUND(e,f,g,h,a,b,c,d,t+4);
		//W[4] = M[4];

        ROUND(f,g,h,a,b,c,d,e,t+3);
		//W[3] = M[3];
        ROUND(g,h,a,b,c,d,e,f,t+2);
		//W[2] = M[2];
        ROUND(h,a,b,c,d,e,f,g,t+1);
		//W[1] = M[1];
        ROUND(a,b,c,d,e,f,g,h,t+0);
		//W[0] = M[0];
    }
	H[0]=a,H[1]=b,H[2]=c,H[3]=d,
	H[4]=e,H[5]=f,H[6]=g,H[7]=h;

	if(1)
	printf ("backward: %08X %08X %08X %08X %08X %08X %08X %08X\n", a,b,c,d,e,f,g,h);
	//uint32_t H1[8] = {}
	if (__builtin_memcmp(W, M, 32)==0)printf ("ok\n");
	//if (H0[0] == a && H0[1]==b && H0[2]==c && H0[3]==d && H0[7]==h)printf ("ok\n");
}
/*
void ssha256(uint8_t *tag, uint8_t *msg, int mlen, uint8_t *salt, int slen)
{
    HashCtx ct;
    HashCtx *ctx = &ct;
    SHA256_init(ctx);
    SHA_update(ctx,  msg, mlen);
    SHA_update(ctx, salt, slen);
    SHA_final (ctx, tag, 32);
} */

int main (){

	uint32_t Hello[16]= {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	uint32_t H[8];
	uint32_t W[16];
	for(int k=0; k<3; k++){
		for (int i=0; i<8; i++) H[i] = H0[i];
		Hello[0] =k*0x123456;
		SHA256(H, Hello, W);
		SHA256_rev(H, H0, Hello, W);
	}
	return 0;
}