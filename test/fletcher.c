/*
 * Fletcher's checksum-based hashes
 * Copyright (C) 2026  Anatoly M. Georgievskii
// https://www.intel.com/content/www/us/en/developer/articles/technical/fast-computation-of-fletcher-checksums.html
 */
#include <stdint.h>
#include <stdlib.h>

typedef uint64_t uint64x2_t __attribute__((__vector_size__(16)));
//------------------------------------------------------------
// Hash based on 1 lane of ZFS's fletcher2 checksum. ZFS is always
// guaranteed blocks of multiples-of-128 bytes for checksums, so it
// does two of these on alternate sets of words.
static void fletcher2( const uint8_t * data, size_t len, uint64_t seed, uint8_t * out ) {
    uint64_t A[2] = {seed, 0};
    while (len >= 8) {
        len-=8;
        A[0] += *(uint64_t*)data; data+=8;
        A[1] += A[0];
    }
    while (len-->0) {
        A[0] += *data++;
        A[1] += A[0];
    }
    *(uint64_t*)(out  ) = A[1];
    *(uint64_t*)(out+8) = A[0];
}
// Hash based on 1 lane of ZFS's fletcher4 checksum. ZFS is always
// guaranteed blocks of multiples-of-128 bytes for checksums, so it
// does two of these on alternate sets of words.
void fletcher4( const uint8_t * data, size_t len, uint64_t seed, uint8_t * out ) {
    uint64_t A[4] = {seed, 0, 0, 0};
    while (len >= 4) {
        len-=4;
        A[0] += *(uint32_t*)data; data+=4;
        A[1] += A[0];
        A[2] += A[1];
        A[3] += A[2];
    }
    while (len-->0) {
        A[0] += *data++;
        A[1] += A[0];
        A[2] += A[1];
        A[3] += A[2];
    }
    *(uint64_t*)(out   ) = A[3];
    *(uint64_t*)(out+ 8) = A[0];
    *(uint64_t*)(out+16) = A[1];
    *(uint64_t*)(out+24) = A[2];
}
#define P UINT32_C(0xFFFFFFFF)
//#define P UINT32_C(0xfe9fffff)
uint64_t fletcher64( const uint8_t * data, size_t len, uint64_t seed ) {
    uint64_t c0 = seed + len, c1 = seed + len;

    while (len > 3) {
        // 92681 32-bit blocks can be processed without the possibility of c0 or c1 overflowing.
        size_t blklen = (len > 370724) ? 370724 : (len & ~3);
        const uint8_t * const endw = data + blklen;
        for (; data < endw; data += 4) {
            c0 +=*(uint32_t*)data; //data+=4;
            c1 += c0;
        }
        len -= blklen;
        c0   = c0 % P;
        c1   = c1 % P;
    }
    if (len > 0) {
        do {
            c0 += *data++;
            c1 += c0;
            len--;
        } while (len > 0);
        c0 = c0 % P;
        c1 = c1 % P;
    }

    return c1 << 32 | c0;
}

void zfs_fletcher2(const uint8_t *data, size_t len, uint64_t seed, uint8_t *out) {
	register uint64x2_t a = {0}, b={0};// добавил seed
	while (len>=16) {
        len-=16;
		a += *(uint64x2_t *)data; data+=16;
		b += a;
	}
    *(uint64_t *)(out   ) = a[0];
    *(uint64_t *)(out+ 8) = a[1];
    *(uint64_t *)(out+16) = b[0];
    *(uint64_t *)(out+24) = b[1];
}


#if 0
static void
fletcher_4_avx2(fletcher_4_ctx_t *ctx, zio_cksum_t *zcp)
{
    __builtin_memset(ctx->avx, 0, 4 * sizeof (zfs_fletcher_avx_t));

static void
fletcher_4_avx2_native(fletcher_4_ctx_t *ctx, const void *buf, uint64_t size)
{
	const uint64_t *ip = buf;
	const uint64_t *ipend = (uint64_t *)((uint8_t *)ip + size);

	FLETCHER_4_AVX2_RESTORE_CTX(ctx);

	do {
		asm volatile("vpmovzxdq %0, %%ymm4"::"m" (*ip));
		asm volatile("vpaddq %ymm4, %ymm0, %ymm0");
		asm volatile("vpaddq %ymm0, %ymm1, %ymm1");
		asm volatile("vpaddq %ymm1, %ymm2, %ymm2");
		asm volatile("vpaddq %ymm2, %ymm3, %ymm3");
	} while ((ip += 2) < ipend);

	asm volatile("vmovdqu %%ymm0, %0" : "=m" ((ctx)->avx[0]));	\
	asm volatile("vmovdqu %%ymm1, %0" : "=m" ((ctx)->avx[1]));	\
	asm volatile("vmovdqu %%ymm2, %0" : "=m" ((ctx)->avx[2]));	\
	asm volatile("vmovdqu %%ymm3, %0" : "=m" ((ctx)->avx[3]));	\
	asm volatile("vzeroupper");
}

	uint64_t A, B, C, D;

	A = ctx->avx[0].v[0] + ctx->avx[0].v[1] +
	    ctx->avx[0].v[2] + ctx->avx[0].v[3];
	B = 0 - ctx->avx[0].v[1] - 2 * ctx->avx[0].v[2] - 3 * ctx->avx[0].v[3] +
	    4 * ctx->avx[1].v[0] + 4 * ctx->avx[1].v[1] + 4 * ctx->avx[1].v[2] +
	    4 * ctx->avx[1].v[3];

	C = ctx->avx[0].v[2] + 3 * ctx->avx[0].v[3] - 6 * ctx->avx[1].v[0] -
	    10 * ctx->avx[1].v[1] - 14 * ctx->avx[1].v[2] -
	    18 * ctx->avx[1].v[3] + 16 * ctx->avx[2].v[0] +
	    16 * ctx->avx[2].v[1] + 16 * ctx->avx[2].v[2] +
	    16 * ctx->avx[2].v[3];

	D = 0 - ctx->avx[0].v[3] + 4 * ctx->avx[1].v[0] +
	    10 * ctx->avx[1].v[1] + 20 * ctx->avx[1].v[2] +
	    34 * ctx->avx[1].v[3] - 48 * ctx->avx[2].v[0] -
	    64 * ctx->avx[2].v[1] - 80 * ctx->avx[2].v[2] -
	    96 * ctx->avx[2].v[3] + 64 * ctx->avx[3].v[0] +
	    64 * ctx->avx[3].v[1] + 64 * ctx->avx[3].v[2] +
	    64 * ctx->avx[3].v[3];

	ZIO_SET_CHECKSUM(zcp, A, B, C, D);
}
#endif