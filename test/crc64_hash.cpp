/*! CRC-64/XZ and similar hashes
    * Copyright (C) 2026  Anatoly M. Georgievskii
 
CRC-64/ECMA-182
    width=64 poly=0x42f0e1eba9ea3693 init=0x0000000000000000 refin=false refout=false xorout=0x0000000000000000 
    check=0x6c40df5f0b497347 residue=0x0000000000000000 name="CRC-64/ECMA-182"
CRC-64/GO-ISO
    width=64 poly=0x000000000000001b init=0xffffffffffffffff refin=true refout=true xorout=0xffffffffffffffff 
    check=0xb90956c775a41001 residue=0x5300000000000000 name="CRC-64/GO-ISO"
CRC-64/MS
    width=64 poly=0x259c84cba6426349 init=0xffffffffffffffff refin=true refout=true xorout=0x0000000000000000 
    check=0x75d4b74f024eceea residue=0x0000000000000000 name="CRC-64/MS"
CRC-64/NVME
    width=64 poly=0xad93d23594c93659 init=0xffffffffffffffff refin=true refout=true xorout=0xffffffffffffffff 
    check=0xae8b14860a799888 residue=0xf310303b2b6f6e42 name="CRC-64/NVME"
CRC-64/REDIS
    width=64 poly=0xad93d23594c935a9 init=0x0000000000000000 refin=true refout=true xorout=0x0000000000000000 
    check=0xe9c6d914c4b8d9ca residue=0x0000000000000000 name="CRC-64/REDIS"
CRC-64/WE
    width=64 poly=0x42f0e1eba9ea3693 init=0xffffffffffffffff refin=false refout=false xorout=0xffffffffffffffff 
    check=0x62ec59e3f1a4f00a residue=0xfcacbebd5931a992 name="CRC-64/WE"
CRC-64/XZ
    width=64 poly=0x42f0e1eba9ea3693 init=0xffffffffffffffff refin=true refout=true xorout=0xffffffffffffffff 
    check=0x995dc9bbdf1939fa residue=0x49958c9abd7d353f name="CRC-64/XZ"
*/
#include <stdint.h>

#define CRC64_WE_CHECK  UINT64_C(0x62ec59e3f1a4f00a)
#define CRC64_XZ_CHECK  UINT64_C(0x995dc9bbdf1939fa)

#ifdef __ARM_NEON
#include <arm_neon.h>
static inline
poly64x2_t CL_MUL128(poly64x2_t a, poly64x2_t b, const int c)
{
/* if (c==0x11) {

	return (poly64x2_t) vmull_hight_p64 ( __t1, __t2);
} else */
{
	poly64_t __t1 = (poly64_t)vgetq_lane_p64(a, c & 1);
	poly64_t __t2 = (poly64_t)vgetq_lane_p64(b,(c>>4) & 1);

	return (poly64x2_t) __builtin_arm_crypto_vmullp64 ( __t1,  __t2);
}
//    return (v2du)__builtin_arm_crypto_vmullp64(vgetq_lane_u64(a, c & 0x1),vgetq_lane_u64(b, (c & 0x10)?1:0));
}
static inline uint8x16_t LOAD128U(uint8_t* p) {
	return vld1q_u8(p);
}
static inline poly64x2_t SLL128U(poly64x2_t a, const int bits) {
	return (poly64x2_t) vextq_u8((uint8x16_t)a,(uint8x16_t){0}, bits>>3);
	//return (v2du){(uint64_t)a[0]<<bits, (uint64_t)a[0]>>(64-bits) | (uint64_t)a[1]<<(bits)};
}
static inline poly64x2_t SRL128U(poly64x2_t a, const int bits) {
	return (poly64x2_t) vextq_u8((uint8x16_t){0},(uint8x16_t)a, (128-bits)>>3);
//	return (v2du){(uint64_t)a[0]>>bits  | (uint64_t)a[1]<<(64-bits), (uint64_t)a[1]>>(bits)};
}
static inline uint8x16_t REVERSE(uint8x16_t v) {
	v = vrev64q_u8(v);
	return vextq_u8(v,v,8);
//	uint64x2_t t = (uint64x2_t)vrev64q_u8((uint8x16_t)x);
//	return (v16qi) vcombine_u64(vgetq_lane_u64(t,1), vgetq_lane_u64(t,0));
//	return (v16qi)(v2du){(uint64_t)vrev64_u8((uint8x8_t) vgetq_lane_u64(t,1)), (uint64_t) vrev64_u8((uint8x8_t)vgetq_lane_u64(t,0))};
}
#else
	#include <x86intrin.h>

typedef uint64_t v2du __attribute__((__vector_size__(16)));
typedef char     v16qi __attribute__((__vector_size__(16)));

typedef uint64_t poly64x2_t __attribute__((__vector_size__(16)));
typedef uint8_t  uint8x16_t __attribute__((__vector_size__(16)));

static inline
v2du CL_MUL128(v2du a, v2du b, const int c) __attribute__ ((__target__("pclmul")));
static inline v2du CL_MUL128(v2du a, v2du b, const int c) {
    return (v2du)_mm_clmulepi64_si128((__m128i)a,(__m128i)b,c);
}
static inline poly64x2_t XOR128(poly64x2_t a, poly64x2_t b) {
	return (poly64x2_t)_mm_xor_si128((__m128i)a,(__m128i)b);
}
static inline v16qi LOAD128U(const uint8_t* p) {
    return (v16qi)_mm_loadu_si128((const __m128i_u*)p);
}
static inline poly64x2_t SLL128U(poly64x2_t a, const int bits) {
    return (poly64x2_t)__builtin_ia32_pslldqi128((__m128i)a, bits);
}
static const uint8x16_t BSWAP_MASK = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
static inline uint8x16_t REVERSE(uint8x16_t x) {
    return __builtin_shuffle (x,BSWAP_MASK);
}
static inline
poly64x2_t LOADZU(const uint8_t* src, int len)
{
#if defined(__AVX512VL__) && defined(__AVX512BW__)// AVX512VL + AVX512BW
	__mmask16 mm = ~0;
	return (poly64x2_t)_mm_maskz_loadu_epi8 (mm>>(-len & 0xF), src);
#else
	poly64x2_t x;
	x^=x;
	__builtin_memcpy((uint8_t*)&x, src, len&0xF);
	return x;
#endif
}
#endif



#define PRIME UINT64_C(0x42F0E1EBA9EA3693)	
#define P_rev UINT64_C(0xC96C5795D7870F42)

// Структура коэффициентов
typedef struct _CRC_ctx CRC_ctx_t;
struct _CRC_ctx {
	poly64x2_t K34[16];
	poly64x2_t K12;//!< fold by 1 (128 bits)
	poly64x2_t KBP;//!< final reduction: Barrett's constant and Polynom
#if 0
    poly64x2_t KF2;//!< fold by 2
	poly64x2_t KF3;//!< fold by 3
	poly64x2_t KF4;//!< fold by 4
	poly64x2_t KF5;//!< fold by 4
#endif
} __attribute__((aligned(16)));

// Обратные числа 0x92D8AF2BAF0E1E85	0xA17870F5D4F51B49
static const CRC_ctx_t CRC64WE_ctx ={
  {
    {0x42F0E1EBA9EA3693ULL, 0x05F5C3C7EB52FAB6ULL},// x^{64}, x^{128}
    {0x158FE402EE664E3CULL, 0x0000000000000100ULL},// x^{-56}, x^{8}
    {0xE21AFDBF510763A3ULL, 0x0000000000010000ULL},// x^{-48}, x^{16}
    {0x7F996AACE22A901AULL, 0x0000000001000000ULL},// x^{-40}, x^{24}
    {0xF56CAF049927DBCAULL, 0x0000000100000000ULL},// x^{-32}, x^{32}
    {0xE1D4EDE2A60FCB9FULL, 0x0000010000000000ULL},// x^{-24}, x^{40}
    {0x7698156710BCF7AFULL, 0x0001000000000000ULL},// x^{-16}, x^{48}
    {0x24854997BA2F81E7ULL, 0x0100000000000000ULL},// x^{-8}, x^{56}
    {0x0000000000000001ULL, 0x42F0E1EBA9EA3693ULL},// x^{0},  x^{64}
    {0x0000000000000100ULL, 0xAF052A6B538EDF09ULL},// x^{8},  x^{72}
    {0x0000000000010000ULL, 0x23EEF79F3AD718C7ULL},// x^{16}, x^{80}
    {0x0000000001000000ULL, 0xE59C4CF90CE5976BULL},// x^{24}, x^{88}
    {0x0000000100000000ULL, 0x770A6888F4A2EF70ULL},// x^{32}, x^{96}
    {0x0000010000000000ULL, 0xF40847980DDD6874ULL},// x^{40}, x^{104}
    {0x0001000000000000ULL, 0xC7CC909DF556430CULL},// x^{48}, x^{112}
    {0x0100000000000000ULL, 0x6E4D3E593561EE88ULL},// x^{56}, x^{120}
  },
  {0x05F5C3C7EB52FAB6ULL, 0x4EB938A7D257740EULL}, // 128 192
  {0x578D29D06CC4F872ULL, 0x42F0E1EBA9EA3693ULL},
};
static const CRC_ctx_t CRC64XZ_ctx = {
  {
    {0xDABE95AFC7875F40ULL, 0x0000000000000001ULL},// x^{127}, x^{63}
    {0x0100000000000000ULL, 0x78E4CCEE804FE350ULL},// x^{7} , x^{-57}
    {0x0001000000000000ULL, 0x19556E3E5470AE0BULL},// x^{15}, x^{-49}
    {0x0000010000000000ULL, 0xB012A88E6AAD33FCULL},// x^{23}, x^{-41}
    {0x0000000100000000ULL, 0xA7B7C93241EA6D5EULL},// x^{31}, x^{-33}
    {0x0000000001000000ULL, 0x617F4FE12060498BULL},// x^{39}, x^{-25}
    {0x0000000000010000ULL, 0x7906D53A625E2C59ULL},// x^{47}, x^{-17}
    {0x0000000000000100ULL, 0x5DDB47907C2B5CCDULL},// x^{55}, x^{-9}
    {0x0000000000000001ULL, 0x92D8AF2BAF0E1E85ULL},// x^{63}, x^{-1}
    {0xB32E4CBE03A75F6FULL, 0x0100000000000000ULL},// x^{71}, x^{7}
    {0x54E979925CD0F10DULL, 0x0001000000000000ULL},// x^{79}, x^{15}
    {0x3F0BE14A916A6DCBULL, 0x0000010000000000ULL},// x^{87}, x^{23}
    {0x1DEE8A5E222CA1DCULL, 0x0000000100000000ULL},// x^{95}, x^{31}
    {0x5C2D776033C4205EULL, 0x0000000001000000ULL},// x^{103}, x^{39}
    {0x6184D55F721267C6ULL, 0x0000000000010000ULL},// x^{111}, x^{47}
    {0x22EF0D5934F964ECULL, 0x0000000000000100ULL},// x^{119}, x^{55}
  },
  {0xE05DD497CA393AE4ULL, 0xDABE95AFC7875F40ULL},// x^{191}, x^{127}
  {0x9C3E466C172963D5ULL, 0x92D8AF2BAF0E1E85ULL},
};
static const CRC_ctx_t CRC64MS_ctx = {
	{
		{0xCEF05CCA14BBF4DFULL, 0x0000000000000001ULL},// x^{127}, x^{63}
		{0x0100000000000000ULL, 0x010A3D6B7CC0B2ECULL},// x^{7}, x^{-57}
		{0x0001000000000000ULL, 0x9F7D1E6DCBF6E341ULL},// x^{15}, x^{-49}
		{0x0000010000000000ULL, 0x41F5B48E12CF3BD8ULL},// x^{23}, x^{-41}
		{0x0000000100000000ULL, 0x1B3559DE6944FB95ULL},// x^{31}, x^{-33}
		{0x0000000001000000ULL, 0x3A6DB73DA8C87582ULL},// x^{39}, x^{-25}
		{0x0000000000010000ULL, 0x82EFFE97C3A15203ULL},// x^{47}, x^{-17}
		{0x0000000000000100ULL, 0x1898D6192C7F5369ULL},// x^{55}, x^{-9}
		{0x0000000000000001ULL, 0x258C84CBA6427349ULL},// x^{63}, x^{-1}
		{0x0809E8A2969451E9ULL, 0x0100000000000000ULL},// x^{71}, x^{7}
		{0xB75A5790CED9A1EFULL, 0x0001000000000000ULL},// x^{79}, x^{15}
		{0x87D177E08BF80869ULL, 0x0000010000000000ULL},// x^{87}, x^{23}
		{0x2513CD6A5FE5F412ULL, 0x0000000100000000ULL},// x^{95}, x^{31}
		{0x90A848A12E3258B6ULL, 0x0000000001000000ULL},// x^{103}, x^{39}
		{0x064D835218FBBF73ULL, 0x0000000000010000ULL},// x^{111}, x^{47}
		{0xF5504FE61DB9C5AFULL, 0x0000000000000100ULL},// x^{119}, x^{55}
	},
	{0xFD5D7A0700B5BA38ULL, 0xCEF05CCA14BBF4DFULL},// x^{191}, x^{127}
	{0xD7EB06822197A109ULL, 0x258C84CBA6427349ULL},// Inv, Prime
};

#include "Platform.h"
#include "Hashlib.h"
#include "Mathmult.h"

static FORCE_INLINE uint64_t nomix( uint64_t A, uint64_t B) {
    (void)B;
    return A;
}
static FORCE_INLINE uint64_t _mum( uint64_t A, uint64_t B) {
    uint64_t rlo, rhi;
    MathMult::mult64_128(rlo, rhi, B, A);
    return (rlo + rhi) ^ ROTL64(rhi,21);
}
#define MC 	        UINT64_C(0xa3b195354a39b70d)
#define MUM_C       UINT64_C(0xa3b195354a39b70d)
#define MUM_S       UINT64_C(0x82d2e9550235efc5)
//#define IV UINT64_C(~0)
#define IV 	        UINT64_C(0x9e3779b97f4a7c15) // хорошая аддитивная константа golden ratio
template <bool bswap>
void CRC64WE_update_N(const void* in, uint64_t len, uint64_t crc, void* out){
    const uint8_t* data = (const uint8_t*)in;
    crc = _mum(crc^IV, MUM_C);
	poly64x2_t c = {0, crc};
	int blocks = (len+15) >> 4;
	while (--blocks>0){
        poly64x2_t v = (poly64x2_t) LOAD128U(data); data+=16;
        if(!bswap) v = (poly64x2_t) REVERSE((uint8x16_t)v);
		c^= v; 
		c = CL_MUL128(c, CRC64WE_ctx.K12, 0x11) 
		  ^ CL_MUL128(c, CRC64WE_ctx.K12, 0x00);
	}
	poly64x2_t v = {0};
	len &= 15;
	if (len) {
		v = LOADZU(data, len);//__builtin_memcpy(&v, data, len);
	} else
		v = (poly64x2_t)LOAD128U(data);
    if(!bswap) v = (poly64x2_t) REVERSE((uint8x16_t)v);
    c^= v;
	c = CL_MUL128(c, CRC64WE_ctx.K34[len], 0x11) 
	  ^ CL_MUL128(c, CRC64WE_ctx.K34[len], 0x00);
	poly64x2_t 
	t = CL_MUL128(c, CRC64WE_ctx.KBP, 0x01) ^c;
	c^= CL_MUL128(t, CRC64WE_ctx.KBP, 0x11) ;
    crc = _mum(c[0],MUM_C);
    PUT_U64<bswap>(crc, (uint8_t *)out,  0);
}

template <bool bswap>
void CRC64XZ_update_N(const void* in, uint64_t len, uint64_t crc, void* out){
    const uint8_t* data = (const uint8_t*)in;
    crc = crc^IV;
	poly64x2_t c = {crc, 0};
	int blocks = (len+15) >> 4;
	while (--blocks){
        poly64x2_t v = (poly64x2_t) LOAD128U(data); data+=16;
        if (bswap) v = (poly64x2_t) REVERSE((uint8x16_t)v);
		c^= v;
		c = CL_MUL128(c, CRC64XZ_ctx.K12, 0x11) 
		  ^ CL_MUL128(c, CRC64XZ_ctx.K12, 0x00);
	}
    poly64x2_t v = {0};
	len &= 15;
	if (len){
		v = LOADZU(data, len);//__builtin_memcpy(&v, data, len);
	} else 
		v = (poly64x2_t)LOAD128U(data);
    if(bswap) v = (poly64x2_t) REVERSE((uint8x16_t)v);
    c^= v;
	c = CL_MUL128(c, CRC64XZ_ctx.K34[len], 0x00) 
	  ^ CL_MUL128(c, CRC64XZ_ctx.K34[len], 0x11);
	poly64x2_t t = CL_MUL128(c, CRC64XZ_ctx.KBP, 0x00);
	c ^= SLL128U(t,64) ^ CL_MUL128(t, CRC64XZ_ctx.KBP, 0x10);
    crc = c[1]^IV;
    PUT_U64<bswap>(crc, (uint8_t *)out,  0);
}
template <bool bswap>
void CRC64MS_update_N(const void* in, uint64_t len, uint64_t crc, void* out){
    const uint8_t* data = (const uint8_t*)in;
    crc = crc^IV;
	poly64x2_t c = {crc, 0};
	int blocks = (len+15) >> 4;
	while (--blocks){
        poly64x2_t v = (poly64x2_t) LOAD128U(data); data+=16;
        if (bswap) v = (poly64x2_t) REVERSE((uint8x16_t)v);
		c^= v;
		c = CL_MUL128(c, CRC64MS_ctx.K12, 0x11) 
		  ^ CL_MUL128(c, CRC64MS_ctx.K12, 0x00);
	}
    poly64x2_t v = {0};
	len &= 15;
	if (len){
		v = LOADZU(data, len);//__builtin_memcpy(&v, data, len);
	} else 
		v = (poly64x2_t)LOAD128U(data);
    if(bswap) v = (poly64x2_t) REVERSE((uint8x16_t)v);
    c^= v;
	c = CL_MUL128(c, CRC64MS_ctx.K34[len], 0x00) 
	  ^ CL_MUL128(c, CRC64MS_ctx.K34[len], 0x11);
	poly64x2_t t = CL_MUL128(c, CRC64MS_ctx.KBP, 0x00);
	c ^= SLL128U(t,64) ^ CL_MUL128(t, CRC64MS_ctx.KBP, 0x10);
    crc = c[1]^IV;
    PUT_U64<bswap>(crc, (uint8_t *)out,  0);
}

REGISTER_FAMILY(CRC64,
   $.src_url    = "https://github.com/AnatolyGeorgievski/MWC128/",
   $.src_status = HashFamilyInfo::SRC_ACTIVE
 );

REGISTER_HASH(CRC64_XZ,
   $.desc       = "64-bit CRC-64/XZ",
   $.impl       = "hwclmul",
   $.hash_flags =
        FLAG_HASH_CLMUL_BASED,
   $.impl_flags =
        FLAG_IMPL_MULTIPLY_64_128   |
        FLAG_IMPL_LICENSE_PUBLIC_DOMAIN,
   $.bits = 64,
   $.verification_LE = 0x3EBAAF43,
   $.verification_BE = 0x4B032B63,
   $.hashfn_native   = CRC64XZ_update_N<false>,
   $.hashfn_bswap    = CRC64XZ_update_N<true>
 );
REGISTER_HASH(CRC64_MS,
   $.desc       = "64-bit CRC-64/MS",
   $.impl       = "hwclmul",
   $.hash_flags =
        FLAG_HASH_CLMUL_BASED,
   $.impl_flags =
        FLAG_IMPL_MULTIPLY_64_128   |
        FLAG_IMPL_LICENSE_PUBLIC_DOMAIN,
   $.bits = 64,
   $.verification_LE = 0x3EBAAF43,
   $.verification_BE = 0x4B032B63,
   $.hashfn_native   = CRC64MS_update_N<false>,
   $.hashfn_bswap    = CRC64MS_update_N<true>
 );
REGISTER_HASH(CRC64_WE,
   $.desc       = "64-bit CRC-64/WE",
   $.impl       = "hwclmul",
   $.hash_flags =
        FLAG_HASH_CLMUL_BASED,
   $.impl_flags =
        FLAG_IMPL_MULTIPLY_64_128   |
        FLAG_IMPL_LICENSE_PUBLIC_DOMAIN,
   $.bits = 64,
   $.verification_LE = 0xF28DCE16,
   $.verification_BE = 0x3258320A,
   $.hashfn_native   = CRC64WE_update_N<false>,
   $.hashfn_bswap    = CRC64WE_update_N<true>
 );