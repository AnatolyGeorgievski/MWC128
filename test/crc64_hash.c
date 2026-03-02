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

CRC-32/BZIP2
	width=32 poly=0x04c11db7 init=0xffffffff refin=false refout=false xorout=0xffffffff check=0xfc891918 residue=0xc704dd7b name="CRC-32/BZIP2"
CRC-32/CKSUM
	width=32 poly=0x04c11db7 init=0x00000000 refin=false refout=false xorout=0xffffffff check=0x765e7680 residue=0xc704dd7b name="CRC-32/CKSUM"
CRC-32/ISCSI
	width=32 poly=0x1edc6f41 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xe3069283 residue=0xb798b438 name="CRC-32/ISCSI"
CRC-32/ISO-HDLC
	width=32 poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926 residue=0xdebb20e3 name="CRC-32/ISO-HDLC"
CRC-32/MEF
	width=32 poly=0x741b8cd7 init=0xffffffff refin=true refout=true xorout=0x00000000 check=0xd2c22f51 residue=0x00000000 name="CRC-32/MEF"
CRC-24/OPENPGP
	width=24 poly=0x864cfb init=0xb704ce refin=false refout=false xorout=0x000000 check=0x21cf02 residue=0x000000 name="CRC-24/OPENPGP"
*/
#include <stdint.h>

#define CRC64_MS_CHECK   UINT64_C(0x75d4b74f024eceea)
#define CRC64_WE_CHECK   UINT64_C(0x62ec59e3f1a4f00a)
#define CRC64_XZ_CHECK   UINT64_C(0x995dc9bbdf1939fa)
#define CRC64_ECMA_CHECK UINT64_C(0x6c40df5f0b497347)
#define CRC64_NVME_CHECK UINT64_C(0xae8b14860a799888)
#define CRC64_GO_CHECK   UINT64_C(0xb90956c775a41001)
#define CRC32K_CHECK   0xd2c22f51
#define CRC32C_CHECK   0xe3069283
#define CRC32B_CHECK   0xcbf43926
#define CRC32_CHECK    0xfc891918
#define CRC24_PGP_INIT    0xb704ce
#define CRC24_PGP_CHECK   0x21cf02
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
static inline uint8x16_t LOAD128U(const uint8_t* p) {
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

typedef  int64_t v2di __attribute__((__vector_size__(16)));
typedef uint64_t v2du __attribute__((__vector_size__(16)));
typedef uint32_t v4su __attribute__((__vector_size__(16)));
typedef uint8_t  v16qu __attribute__((__vector_size__(16)));
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
#endif

// Структура коэффициентов
typedef struct _CRC_ctx CRC_ctx_t;
struct _CRC_ctx {
	poly64x2_t K34[16];
	poly64x2_t K12;//!< fold by 1 (128 bits)
	poly64x2_t KBP;//!< final reduction: Barrett's constant and Polynom
#if 1
	poly64x2_t KF2;//!< fold by 2
	poly64x2_t KF3;//!< fold by 3
	poly64x2_t KF4;//!< fold by 4
	poly64x2_t KF5;//!< fold by 4
#endif
};

static const struct _CRC_ctx CRC32B_ctx= {
.KBP = {0xB4E5B025F7011641, 0x1DB710641},
.KF5 = {0x1C279815, 0xAE0B5394},
.KF4 = {0x8F352D95, 0x1D9513D7},
.KF3 = {0x3DB1ECDC, 0xAF449247},
.KF2 = {0xF1DA05AA, 0x81256527},
.K12 = {0xAE689191, 0xCCAA009E},
.K34 = {
[ 1] = {0x3F036DC2, 0x40B3A940},// x^{-25}, x^{-89}
[ 2] = {0x7555A0F1, 0x769CF239},// x^{-17}, x^{-81}
[ 3] = {0xCACF972A, 0x5F7314FA},// x^{-9}, x^{-73}
[ 4] = {0xDB710641, 0x5D376816},// x^{-1}, x^{-65}
[ 5] = {0x01000000, 0xF4898239},// x^{7}, x^{-57}
[ 6] = {0x00010000, 0x5FF1018A},// x^{15}, x^{-49}
[ 7] = {0x00000100, 0x0D329B3F},// x^{23}, x^{-41}
[ 8] = {0x00000001, 0xB66B1FA6},// x^{31}, x^{-33}
[ 9] = {0x77073096, 0x3F036DC2},// x^{39}, x^{-25}
[10] = {0x191B3141, 0x7555A0F1},// x^{47}, x^{-17}
[11] = {0x01C26A37, 0xCACF972A},// x^{55}, x^{-9}
[12] = {0xB8BC6765, 0xDB710641},// x^{63}, x^{-1}
[13] = {0x3D6029B0, 0x01000000},// x^{71}, x^{7}
[14] = {0xCB5CD3A5, 0x00010000},// x^{79}, x^{15}
[15] = {0xA6770BB4, 0x00000100},// x^{87}, x^{23}
[ 0] = {0xCCAA009E, 0x00000001},// x^{95}, x^{31}
}};

// CRC-32C (Castagnoli)
static const struct _CRC_ctx CRC32C_ctx= {
.KBP = {0x4869EC38DEA713F1, 0x105EC76F1},
.KF5 = {0x083A6EEC, 0x39D3B296},
.KF4 = {0x740EEF02, 0x9E4ADDF8},
.KF3 = {0x1C291D04, 0xDDC0152B},
.KF2 = {0x3DA6D0CB, 0xBA4FC28E},
.K12 = {0xF20C0DFE, 0x493C7D27},
.K34 = {
[ 1] = {0xBF818109, 0xF838CD50},// x^{-25}, x^{-89}
[ 2] = {0x780D5A4D, 0x51DDE21E},// x^{-17}, x^{-81}
[ 3] = {0xFE2B5C35, 0xBC77A5AA},// x^{-9}, x^{-73}
[ 4] = {0x05EC76F1, 0xC915EA3B},// x^{-1}, x^{-65}
[ 5] = {0x01000000, 0xA9A3F760},// x^{7}, x^{-57}
[ 6] = {0x00010000, 0x616F3095},// x^{15}, x^{-49}
[ 7] = {0x00000100, 0xA738873B},// x^{23}, x^{-41}
[ 8] = {0x00000001, 0xA9CDDA0D},// x^{31}, x^{-33}
[ 9] = {0xF26B8303, 0xBF818109},// x^{39}, x^{-25}
[10] = {0x13A29877, 0x780D5A4D},// x^{47}, x^{-17}
[11] = {0xA541927E, 0xFE2B5C35},// x^{55}, x^{-9}
[12] = {0xDD45AAB8, 0x05EC76F1},// x^{63}, x^{-1}
[13] = {0x38116FAC, 0x01000000},// x^{71}, x^{7}
[14] = {0xEF306B19, 0x00010000},// x^{79}, x^{15}
[15] = {0x68032CC8, 0x00000100},// x^{87}, x^{23}
[ 0] = {0x493C7D27, 0x00000001},// x^{95}, x^{31}
}};
// CRC-32K/BACnet (Koopman)
static const struct _CRC_ctx CRC32K_ctx= {
.KBP = {0xC25DD01C17D232CD, 0x1D663B05D},
.KF5 = {0x46CC6B97, 0x7FD4456B},
.KF4 = {0x1609284B, 0xBE6D8F38},
.KF3 = {0x97259F1A, 0x63C7D97F},
.KF2 = {0x9C899030, 0xADFA5198},
.K12 = {0x7B4BC878, 0x9D65B2A5},// x^{159}, x^{95}
.K34 = {
[ 1] = {0x91F9A353, 0x13F534A1},// x^{-25}, x^{-89}
[ 2] = {0x9B1BE78B, 0xAC4A47F5},// x^{-17}, x^{-81}
[ 3] = {0xC790B954, 0x7A51D862},// x^{-9}, x^{-73}
[ 4] = {0xD663B05D, 0x5F572A23},// x^{-1}, x^{-65}
[ 5] = {0x01000000, 0xBC7F040C},// x^{7}, x^{-57}
[ 6] = {0x00010000, 0x61A83B55},// x^{15}, x^{-49}
[ 7] = {0x00000100, 0x40504C15},// x^{23}, x^{-41}
[ 8] = {0x00000001, 0x35E95875},// x^{31}, x^{-33}
[ 9] = {0x9695C4CA, 0x91F9A353},// x^{39}, x^{-25}
[10] = {0x24901FAA, 0x9B1BE78B},// x^{47}, x^{-17}
[11] = {0x80475843, 0xC790B954},// x^{55}, x^{-9}
[12] = {0x18C5564C, 0xD663B05D},// x^{63}, x^{-1}
[13] = {0x14946D10, 0x01000000},// x^{71}, x^{7}
[14] = {0x83DB9B51, 0x00010000},// x^{79}, x^{15}
[15] = {0x6041FC7A, 0x00000100},// x^{87}, x^{23}
[ 0] = {0x9D65B2A5, 0x00000001},// x^{95}, x^{31}
}};
const struct _CRC_ctx CRC32_ctx= {
.KBP = {0x04D101DF481B4E5A, 0x04C11DB700000000},
.KF4 = {0xE6228B11, 0x8833794C},
.KF3 = {0x8C3828A8, 0x64BF7A9B},
.KF2 = {0x75BE46B7, 0x569700E5},
.K12 = {0xE8A45605, 0xC5B9CD4C},// x^{128}, x^{192}
.K34 = {
[ 1] = {0x052B9A0400000000, 0x876D81F800000000},// x^{-88}, x^{-24}
[ 2] = {0x3C5F6F6B00000000, 0x1ACA48EB00000000},// x^{-80}, x^{-16}
[ 3] = {0xBE519DF400000000, 0xA9D3E6A600000000},// x^{-72}, x^{-8}
[ 4] = {0xD02DD97400000000, 0x0000000100000000},// x^{-64}, x^{ 0}
[ 5] = {0x3C423FE900000000, 0x0000010000000000},// x^{-56}, x^{ 8}
[ 6] = {0xA3011FF400000000, 0x0001000000000000},// x^{-48}, x^{16}
[ 7] = {0xFD7384D700000000, 0x0100000000000000},// x^{-40}, x^{24}
[ 8] = {0xCBF1ACDA00000000, 0x04C11DB700000000},// x^{-32}, x^{32}
[ 9] = {0x876D81F800000000, 0xD219C1DC00000000},// x^{-24}, x^{40}
[10] = {0x1ACA48EB00000000, 0x01D8AC8700000000},// x^{-16}, x^{48}
[11] = {0xA9D3E6A600000000, 0xDC6D9AB700000000},// x^{-8}, x^{56}
[12] = {0x0000000100000000, 0x490D678D00000000},// x^{ 0}, x^{64}
[13] = {0x0000010000000000, 0x1B280D7800000000},// x^{ 8}, x^{72}
[14] = {0x0001000000000000, 0x4F57681100000000},// x^{16}, x^{80}
[15] = {0x0100000000000000, 0x5BA1DCCA00000000},// x^{24}, x^{88}
[ 0] = {0x04C11DB700000000, 0xF200AA6600000000},// x^{32}, x^{96}
}};
static const struct _CRC_ctx CRC24_ctx= {
.KBP = {0xF845FE2493242DA4, 0x864CFB0000000000},
.KF4 = {0x7DB43E, 0xB937A7},
.KF3 = {0x01CD94, 0x3B20E3},
.KF2 = {0xCB800E, 0xD15ED7},
.K12 = {0x6243DA, 0xB22B31},// x^{128}, x^{192}
.K34 = {
[ 1] = {0x2471670000000000, 0x7190920000000000},// x^{-96}, x^{-32}
[ 2] = {0xEE50080000000000, 0xC6E2490000000000},// x^{-88}, x^{-24}
[ 3] = {0xCE8F4A0000000000, 0xD19E9A0000000000},// x^{-80}, x^{-16}
[ 4] = {0x1D1CA30000000000, 0xF77C040000000000},// x^{-72}, x^{-8}
[ 5] = {0x6DC6AA0000000000, 0x0000010000000000},// x^{-64}, x^{ 0}
[ 6] = {0x67F3180000000000, 0x0001000000000000},// x^{-56}, x^{ 8}
[ 7] = {0x79152C0000000000, 0x0100000000000000},// x^{-48}, x^{16}
[ 8] = {0xE2DD700000000000, 0x864CFB0000000000},// x^{-40}, x^{24}
[ 9] = {0x7190920000000000, 0x668F480000000000},// x^{-32}, x^{32}
[10] = {0xC6E2490000000000, 0x8309D70000000000},// x^{-24}, x^{40}
[11] = {0xD19E9A0000000000, 0x3609520000000000},// x^{-16}, x^{48}
[12] = {0xF77C040000000000, 0xD9FE8C0000000000},// x^{-8}, x^{56}
[13] = {0x0000010000000000, 0x36EB3D0000000000},// x^{ 0}, x^{64}
[14] = {0x0001000000000000, 0x3B918C0000000000},// x^{ 8}, x^{72}
[15] = {0x0100000000000000, 0xF50BAF0000000000},// x^{16}, x^{80}
[ 0] = {0x864CFB0000000000, 0xFD7E0C0000000000},// x^{24}, x^{88}
}};
static const struct _CRC_ctx CRC16_ctx= {
.KBP = {0x11303471A041B343, 0x1021000000000000},
.KF4 = {0x13FC, 0x8832},
.KF3 = {0xCDE2, 0x2535},
.KF2 = {0x8E29, 0x26AA},
.K12 = {0xAEFC, 0x650B},// x^{128}, x^{192}
.K34 = {
[ 1] = {0xCBF3000000000000, 0x2AE4000000000000},// x^{-104}, x^{-40}
[ 2] = {0x9B27000000000000, 0x6128000000000000},// x^{-96}, x^{-32}
[ 3] = {0x15D2000000000000, 0x5487000000000000},// x^{-88}, x^{-24}
[ 4] = {0x9094000000000000, 0x9D71000000000000},// x^{-80}, x^{-16}
[ 5] = {0x17B9000000000000, 0x2314000000000000},// x^{-72}, x^{-8}
[ 6] = {0xDBD6000000000000, 0x0001000000000000},// x^{-64}, x^{ 0}
[ 7] = {0xAC16000000000000, 0x0100000000000000},// x^{-56}, x^{ 8}
[ 8] = {0x6266000000000000, 0x1021000000000000},// x^{-48}, x^{16}
[ 9] = {0x2AE4000000000000, 0x3331000000000000},// x^{-40}, x^{24}
[10] = {0x6128000000000000, 0x3730000000000000},// x^{-32}, x^{32}
[11] = {0x5487000000000000, 0x76B4000000000000},// x^{-24}, x^{40}
[12] = {0x9D71000000000000, 0xAA51000000000000},// x^{-16}, x^{48}
[13] = {0x2314000000000000, 0x45A0000000000000},// x^{-8}, x^{56}
[14] = {0x0001000000000000, 0xB861000000000000},// x^{ 0}, x^{64}
[15] = {0x0100000000000000, 0x47D3000000000000},// x^{ 8}, x^{72}
[ 0] = {0x1021000000000000, 0xEB23000000000000},// x^{16}, x^{80}
}};


static const struct _CRC_ctx CRC64MS_ctx __attribute__((aligned(16))) = {
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
.KF2 = {0x2188097F5687B43CuLL,  0x70BD522114FACEB8uLL},// x^{319}, x^{255}
.KF3 = {0x717984ED338C465FuLL,  0x4AA4564B4042092BuLL},// x^{447}, x^{383}
.KF4 = {0xD3E2DC3A51DACEE1uLL,  0xA62BC2D50BF03C03uLL},// x^{575}, x^{511}
};
// Обратные числа 0x92D8AF2BAF0E1E85	0xA17870F5D4F51B49
static const struct _CRC_ctx CRC64WE_ctx __attribute__((aligned(16))) ={
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
.KF2 = {0x571BEE0A227EF92BuLL,  0x44BEF2A201B5200CuLL},// x^{256}, x^{320}
.KF3 = {0x54819D8713758B2CuLL,  0x4A6B90073EB0AF5AuLL},// x^{384}, x^{448}
.KF4 = {0x5F6843CA540DF020uLL,  0xDDF4B6981205B83FuLL},// x^{512}, x^{576}
};
static const struct _CRC_ctx CRC64XZ_ctx __attribute__((aligned(16))) = {
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
  {0x9C3E466C172963D5ULL, 0x92D8AF2BAF0E1E85ULL},// Inv, Prime
.KF2 = {0x60095B008A9EFA44uLL,  0x3BE653A30FE1AF51uLL},// x^{319}, x^{255}
.KF3 = {0xB5EA1AF9C013ACA4uLL,  0x69A35D91C3730254uLL},// x^{447}, x^{383}
.KF4 = {0x6AE3EFBB9DD441F3uLL,  0x081F6054A7842DF4uLL},// x^{575}, x^{511}
};
static const struct _CRC_ctx CRC64NV_ctx __attribute__((aligned(16))) = {
{
[ 0] = {0x21E9761E252621ACULL, 0x0000000000000001ULL},// x^{127}, x^{63}
[ 1] = {0x0100000000000000ULL, 0xA15032FE8971C4E1ULL},// x^{7}, x^{-57}
[ 2] = {0x0001000000000000ULL, 0xC338171909E5C365ULL},// x^{15}, x^{-49}
[ 3] = {0x0000010000000000ULL, 0x90540966DA6F2858ULL},// x^{23}, x^{-41}
[ 4] = {0x0000000100000000ULL, 0x09369D3BFC1ED3B3ULL},// x^{31}, x^{-33}
[ 5] = {0x0000000001000000ULL, 0x92F7B5A05E2BFC6EULL},// x^{39}, x^{-25}
[ 6] = {0x0000000000010000ULL, 0xB7AA3B285B728399ULL},// x^{47}, x^{-17}
[ 7] = {0x0000000000000100ULL, 0x3B49DA55548FA3BBULL},// x^{55}, x^{-9}
[ 8] = {0x0000000000000001ULL, 0x34D926535897936BULL},// x^{63}, x^{-1}
[ 9] = {0x7F6EF0C830358979ULL, 0x0100000000000000ULL},// x^{71}, x^{7}
[10] = {0x8776A97D73BDDF69ULL, 0x0001000000000000ULL},// x^{79}, x^{15}
[11] = {0xFF6E4E1F4E4038BEULL, 0x0000010000000000ULL},// x^{87}, x^{23}
[12] = {0x8211147CBAF96306ULL, 0x0000000100000000ULL},// x^{95}, x^{31}
[13] = {0x373D15F784905D1EULL, 0x0000000001000000ULL},// x^{103}, x^{39}
[14] = {0xE9742A79EF04A5D4ULL, 0x0000000000010000ULL},// x^{111}, x^{47}
[15] = {0xFC5D27F6BF353971ULL, 0x0000000000000100ULL},// x^{119}, x^{55}
},
	{0xEADC41FD2BA3D420ULL,0x21E9761E252621ACULL},
	{0x27ECFA329AEF9F77ULL,0x34D926535897936BULL},
.KF2 = {0xB0BC2E589204F500uLL,  0xE1E0BB9D45D7A44CuLL},// x^{319}, x^{255}
.KF3 = {0xBDD7AC0EE1A4A0F0uLL,  0xA3FFDC1FE8E82A8BuLL},// x^{447}, x^{383}
.KF4 = {0x0C32CDB31E18A84AuLL,  0x62242240ACE5045AuLL},// x^{575}, x^{511}

};
static const struct _CRC_ctx CRC64GO_ctx __attribute__((aligned(16))) = {
{
[ 0] = {0xF500000000000001ULL, 0x0000000000000001ULL},// x^{127}, x^{63}
[ 1] = {0x0100000000000000ULL, 0x11C71C71C71C71C7ULL},// x^{7}, x^{-57}
[ 2] = {0x0001000000000000ULL, 0xB001C71C71C71C71ULL},// x^{15}, x^{-49}
[ 3] = {0x0000010000000000ULL, 0x400001C71C71C71CULL},// x^{23}, x^{-41}
[ 4] = {0x0000000100000000ULL, 0x10000001C71C71C7ULL},// x^{31}, x^{-33}
[ 5] = {0x0000000001000000ULL, 0xB000000001C71C71ULL},// x^{39}, x^{-25}
[ 6] = {0x0000000000010000ULL, 0x400000000001C71CULL},// x^{47}, x^{-17}
[ 7] = {0x0000000000000100ULL, 0x10000000000001C7ULL},// x^{55}, x^{-9}
[ 8] = {0x0000000000000001ULL, 0xB000000000000001ULL},// x^{63}, x^{-1}
[ 9] = {0x01B0000000000000ULL, 0x0100000000000000ULL},// x^{71}, x^{7}
[10] = {0x0001B00000000000ULL, 0x0001000000000000ULL},// x^{79}, x^{15}
[11] = {0x000001B000000000ULL, 0x0000010000000000ULL},// x^{87}, x^{23}
[12] = {0x00000001B0000000ULL, 0x0000000100000000ULL},// x^{95}, x^{31}
[13] = {0x0000000001B00000ULL, 0x0000000001000000ULL},// x^{103}, x^{39}
[14] = {0x000000000001B000ULL, 0x0000000000010000ULL},// x^{111}, x^{47}
[15] = {0x00000000000001B0ULL, 0x0000000000000100ULL},// x^{119}, x^{55}
},
.KBP = {0xB000000000000001ULL, 0xB000000000000001ULL},
.K12 = {0x6B70000000000001uLL, 0xF500000000000001uLL},// x^{191}, x^{127}
.KF2 = {0x1B1AB00000000001uLL, 0xA011000000000001uLL},// x^{319}, x^{255}
.KF3 = {0x76DB6C7000000001uLL, 0xE145150000000001uLL},// x^{447}, x^{383}
.KF4 = {0x01B001B1B0000001uLL, 0xB100010100000001uLL},// x^{575}, x^{511}
};

uint64_t CRC64_update_N(const CRC_ctx_t *ctx, uint64_t crc, uint8_t* data, int len){
	poly64x2_t c = {0, crc};
	int blocks = (len+15) >> 4;
    if (1 && blocks>7) {// fold by 4x128 bits
        poly64x2_t c1 = {0}, c2 = {0}, c3 = {0};
__asm volatile("# LLVM-MCA-BEGIN CRC64_update_N_fold4");
        do {
			c ^= (poly64x2_t)REVERSE((uint8x16_t)LOAD128U((void*)(data   )));
			c1^= (poly64x2_t)REVERSE((uint8x16_t)LOAD128U((void*)(data+16)));
			c2^= (poly64x2_t)REVERSE((uint8x16_t)LOAD128U((void*)(data+32)));
			c3^= (poly64x2_t)REVERSE((uint8x16_t)LOAD128U((void*)(data+48)));
            c  = CL_MUL128(c , ctx->KF4, 0x00) ^ CL_MUL128(c , ctx->KF4, 0x11);
            c1 = CL_MUL128(c1, ctx->KF4, 0x00) ^ CL_MUL128(c1, ctx->KF4, 0x11);
            c2 = CL_MUL128(c2, ctx->KF4, 0x00) ^ CL_MUL128(c2, ctx->KF4, 0x11);
            c3 = CL_MUL128(c3, ctx->KF4, 0x00) ^ CL_MUL128(c3, ctx->KF4, 0x11);
            blocks-=4, data+=64;
        } while(blocks>7);
__asm volatile("# LLVM-MCA-END CRC64_update_N_fold4");
		c ^= (poly64x2_t)REVERSE((uint8x16_t)LOAD128U((void*)(data   )));
		c1^= (poly64x2_t)REVERSE((uint8x16_t)LOAD128U((void*)(data+16)));
		c2^= (poly64x2_t)REVERSE((uint8x16_t)LOAD128U((void*)(data+32)));
        c  = c3
		   ^ CL_MUL128(c , ctx->KF3, 0x00) ^ CL_MUL128(c , ctx->KF3, 0x11)
           ^ CL_MUL128(c1, ctx->KF2, 0x00) ^ CL_MUL128(c1, ctx->KF2, 0x11)
           ^ CL_MUL128(c2, ctx->K12, 0x00) ^ CL_MUL128(c2, ctx->K12, 0x11);
        blocks-=3, data+=48;
    }
	while (--blocks>0){
		c^= (poly64x2_t) REVERSE((uint8x16_t)LOAD128U(data)); data+=16;
		c = CL_MUL128(c, ctx->K12, 0x11) 
		  ^ CL_MUL128(c, ctx->K12, 0x00);
	}
	poly64x2_t v={0};
	len &= 15;
	if (len) {
		__builtin_memcpy(&v, data, len);
	} else
		v = (poly64x2_t)LOAD128U(data);
	c^= (poly64x2_t) REVERSE((uint8x16_t)v);
	c = CL_MUL128(c, ctx->K34[len], 0x11) 
	  ^ CL_MUL128(c, ctx->K34[len], 0x00);

    poly64x2_t 
	t = CL_MUL128(c, ctx->KBP, 0x01) ^c;
	c^= CL_MUL128(t, ctx->KBP, 0x11) ;
	return c[0];
}
uint64_t CRC64B_update_N(const CRC_ctx_t *ctx, uint64_t crc, uint8_t* data, int len){
	poly64x2_t c = {crc,0};
	
	int blocks = (len+15) >> 4;
    if (1 && blocks>7) {// fold by 4x128 bits
        poly64x2_t c1 = {0}, c2 = {0}, c3 = {0};
__asm volatile("# LLVM-MCA-BEGIN CRC64B_update_N_fold4");
        do {
			c ^= (poly64x2_t)LOAD128U((void*)(data   ));
			c1^= (poly64x2_t)LOAD128U((void*)(data+16));
			c2^= (poly64x2_t)LOAD128U((void*)(data+32));
			c3^= (poly64x2_t)LOAD128U((void*)(data+48));
            c  = CL_MUL128(c , ctx->KF4, 0x00) ^ CL_MUL128(c , ctx->KF4, 0x11);
            c1 = CL_MUL128(c1, ctx->KF4, 0x00) ^ CL_MUL128(c1, ctx->KF4, 0x11);
            c2 = CL_MUL128(c2, ctx->KF4, 0x00) ^ CL_MUL128(c2, ctx->KF4, 0x11);
            c3 = CL_MUL128(c3, ctx->KF4, 0x00) ^ CL_MUL128(c3, ctx->KF4, 0x11);
            blocks-=4, data+=64;
        } while(blocks>7);
__asm volatile("# LLVM-MCA-END CRC64B_update_N_fold4");
        c ^= (poly64x2_t)LOAD128U((void*)(data   ));
        c1^= (poly64x2_t)LOAD128U((void*)(data+16));
        c2^= (poly64x2_t)LOAD128U((void*)(data+32));
        c  = c3
		   ^ CL_MUL128(c , ctx->KF3, 0x00) ^ CL_MUL128(c , ctx->KF3, 0x11)
		   ^ CL_MUL128(c1, ctx->KF2, 0x00) ^ CL_MUL128(c1, ctx->KF2, 0x11)
           ^ CL_MUL128(c2, ctx->K12, 0x00) ^ CL_MUL128(c2, ctx->K12, 0x11);
        blocks-=3, data+=48;
    }
    if (0 && blocks>3) {// fold by 2x128 bits
        poly64x2_t c1 = {0};
__asm volatile("# LLVM-MCA-BEGIN CRC64B_update_N_fold2");
        do {
			c ^= (poly64x2_t)LOAD128U((void*)(data   ));
			c1^= (poly64x2_t)LOAD128U((void*)(data+16));
            //c  = CL_MUL128(c, ctx->K12, 0x00) ^ CL_MUL128(c, ctx->K12, 0x11);
            c  = CL_MUL128(c, ctx->KF2, 0x00) ^ CL_MUL128(c, ctx->KF2, 0x11);
            c1 = CL_MUL128(c1, ctx->KF2, 0x00) ^ CL_MUL128(c1, ctx->KF2, 0x11);
            blocks-=2, data+=32;
        } while(blocks>3);
__asm volatile("# LLVM-MCA-END CRC64B_update_N_fold2");
        c ^= (poly64x2_t)LOAD128U((void*)data);
        c  = c1 ^ CL_MUL128(c, ctx->K12, 0x00) ^ CL_MUL128(c, ctx->K12, 0x11);
        blocks-=1,  data+=16;
    }
	while (--blocks>0){
		c ^= (poly64x2_t)LOAD128U(data); data+=16;
		c = CL_MUL128(c, ctx->K12, 0x11) // 192
		  ^ CL_MUL128(c, ctx->K12, 0x00);// 128
	}
	len &= 15;
	if (len){
		poly64x2_t v = {0};
		__builtin_memcpy(&v, data, len);
		c^= v;
	} else 
		c^= (poly64x2_t)LOAD128U(data); 
	c = CL_MUL128(c, ctx->K34[len], 0x00) 
	  ^ CL_MUL128(c, ctx->K34[len], 0x11);

	poly64x2_t t = CL_MUL128(c, ctx->KBP, 0x00);
	c ^= SLL128U(t,64) ^ CL_MUL128(t, ctx->KBP, 0x10);
	return c[1];
}

/*! \brief Вычисление CRC8-CRC64
	\param crc Начальное значение суммы. При загрузке должно выполняться выравнивание по старшему биту (MSB).

*/
static
uint64_t 	CRC32_update_N(const struct _CRC_ctx * ctx,  uint64_t crc, const uint8_t *data, int len){
	poly64x2_t c = {0, crc};
	int blocks = (len+15) >> 4;
    if (1 && blocks>7) {// fold by 4x128 bits
        poly64x2_t c1 = {0}, c2 = {0}, c3 = {0};
__asm volatile("# LLVM-MCA-BEGIN CRC64_update_N_fold4");
        do {
			c ^= (poly64x2_t)REVERSE((uint8x16_t)LOAD128U((void*)(data   )));
			c1^= (poly64x2_t)REVERSE((uint8x16_t)LOAD128U((void*)(data+16)));
			c2^= (poly64x2_t)REVERSE((uint8x16_t)LOAD128U((void*)(data+32)));
			c3^= (poly64x2_t)REVERSE((uint8x16_t)LOAD128U((void*)(data+48)));
            c  = CL_MUL128(c , ctx->KF4, 0x00) ^ CL_MUL128(c , ctx->KF4, 0x11);
            c1 = CL_MUL128(c1, ctx->KF4, 0x00) ^ CL_MUL128(c1, ctx->KF4, 0x11);
            c2 = CL_MUL128(c2, ctx->KF4, 0x00) ^ CL_MUL128(c2, ctx->KF4, 0x11);
            c3 = CL_MUL128(c3, ctx->KF4, 0x00) ^ CL_MUL128(c3, ctx->KF4, 0x11);
            blocks-=4, data+=64;
        } while(blocks>7);
__asm volatile("# LLVM-MCA-END CRC64_update_N_fold4");
		c ^= (poly64x2_t)REVERSE((uint8x16_t)LOAD128U((void*)(data   )));
		c1^= (poly64x2_t)REVERSE((uint8x16_t)LOAD128U((void*)(data+16)));
		c2^= (poly64x2_t)REVERSE((uint8x16_t)LOAD128U((void*)(data+32)));
        c  = c3
		   ^ CL_MUL128(c , ctx->KF3, 0x00) ^ CL_MUL128(c , ctx->KF3, 0x11)
           ^ CL_MUL128(c1, ctx->KF2, 0x00) ^ CL_MUL128(c1, ctx->KF2, 0x11)
           ^ CL_MUL128(c2, ctx->K12, 0x00) ^ CL_MUL128(c2, ctx->K12, 0x11);
        blocks-=3, data+=48;
    }
__asm volatile("# LLVM-MCA-BEGIN CRC64_update_N");
    while (--blocks>0) {// fold by 128 bits
		c^= (poly64x2_t)REVERSE((uint8x16_t)LOAD128U((void*)(data)));
		c = CL_MUL128(c, ctx->K12, 0x00) ^ CL_MUL128(c, ctx->K12, 0x11);
		blocks-=1, data+=16;
    }
__asm volatile("# LLVM-MCA-END CRC64_update_N");
	poly64x2_t v;
	len &= 15;
	if (len){
		v = (poly64x2_t){0};
		__builtin_memcpy(&v, data, len);
	} else
		v = (poly64x2_t)LOAD128U(data);
	c^= (poly64x2_t)REVERSE((uint8x16_t)v);
	// final reduction 128 bit
	c = CL_MUL128(c, ctx->K34[len], 0x11) // 128-32
	  ^ CL_MUL128(c, ctx->K34[len], 0x00);// 64-32
	// Barrett's reduction
	poly64x2_t t;
	t  = CL_MUL128(c, ctx->KBP, 0x01)^c;//(uint64x2_t){0,c[1]};
	c ^= CL_MUL128(t, ctx->KBP, 0x11);//^(uint64x2_t){0,t[1]}; -- единица в старшем разряде Prime
//	printf("%016llx %016llx\n", c[0],c[1]);
	return c[0];
}
static 
uint64_t 	CRC32B_update_N(const struct _CRC_ctx * ctx,  uint64_t crc, const uint8_t *data, int len){
	poly64x2_t c = {crc};
	int blocks = (len+15) >> 4;
    if (1 && blocks>7) {// fold by 4x128 bits
        poly64x2_t c1 = {0}, c2 = {0}, c3 = {0};
__asm volatile("# LLVM-MCA-BEGIN CRC64B_update_N_fold4");
        do {
			c ^= (poly64x2_t)LOAD128U((void*)(data   ));
			c1^= (poly64x2_t)LOAD128U((void*)(data+16));
			c2^= (poly64x2_t)LOAD128U((void*)(data+32));
			c3^= (poly64x2_t)LOAD128U((void*)(data+48));
            c  = CL_MUL128(c , ctx->KF4, 0x00) ^ CL_MUL128(c , ctx->KF4, 0x11);
            c1 = CL_MUL128(c1, ctx->KF4, 0x00) ^ CL_MUL128(c1, ctx->KF4, 0x11);
            c2 = CL_MUL128(c2, ctx->KF4, 0x00) ^ CL_MUL128(c2, ctx->KF4, 0x11);
            c3 = CL_MUL128(c3, ctx->KF4, 0x00) ^ CL_MUL128(c3, ctx->KF4, 0x11);
            blocks-=4, data+=64;
        } while(blocks>7);
__asm volatile("# LLVM-MCA-END CRC64B_update_N_fold4");
        c ^= (poly64x2_t)LOAD128U((void*)(data   ));
        c1^= (poly64x2_t)LOAD128U((void*)(data+16));
        c2^= (poly64x2_t)LOAD128U((void*)(data+32));
        c  = c3
		   ^ CL_MUL128(c , ctx->KF3, 0x00) ^ CL_MUL128(c , ctx->KF3, 0x11)
		   ^ CL_MUL128(c1, ctx->KF2, 0x00) ^ CL_MUL128(c1, ctx->KF2, 0x11)
           ^ CL_MUL128(c2, ctx->K12, 0x00) ^ CL_MUL128(c2, ctx->K12, 0x11);
        blocks-=3, data+=48;
    }
    if (0 && blocks>3) {// fold by 2x128 bits
        poly64x2_t c1 = {0};
__asm volatile("# LLVM-MCA-BEGIN CRC64B_update_N_fold2");
        do {
			c ^= (poly64x2_t)LOAD128U((void*)(data   ));
			c1^= (poly64x2_t)LOAD128U((void*)(data+16));
            //c  = CL_MUL128(c, ctx->K12, 0x00) ^ CL_MUL128(c, ctx->K12, 0x11);
            c  = CL_MUL128(c, ctx->KF2, 0x00) ^ CL_MUL128(c, ctx->KF2, 0x11);
            c1 = CL_MUL128(c1, ctx->KF2, 0x00) ^ CL_MUL128(c1, ctx->KF2, 0x11);
            blocks-=2, data+=32;
        } while(blocks>3);
__asm volatile("# LLVM-MCA-END CRC64B_update_N_fold2");
        c ^= (poly64x2_t)LOAD128U((void*)data);
        c  = c1 ^ CL_MUL128(c, ctx->K12, 0x00) ^ CL_MUL128(c, ctx->K12, 0x11);
        blocks-=1,  data+=16;
    }
__asm volatile("# LLVM-MCA-BEGIN CRC64B_update_N");
    while (--blocks>0) {// fold by 128 bits
		poly64x2_t v = (poly64x2_t)LOAD128U((void*)data); data+=16;
		c^= v; 
		c = CL_MUL128(c, ctx->K12, 0x00) ^ CL_MUL128(c, ctx->K12, 0x11);
    }
__asm volatile("# LLVM-MCA-END CRC64B_update_N");
	len &= 15;
	if (len){
		poly64x2_t v={0};
		__builtin_memcpy(&v, data, len);
		c^= v;
	} else
		c^= (poly64x2_t)LOAD128U((void*)data);
	c = CL_MUL128(c, ctx->K34[len], 0x00) // 15+64
	  ^ CL_MUL128(c, ctx->K34[len], 0x11);// 15
	poly64x2_t t;
	t  = CL_MUL128(c, ctx->KBP, 0x00);
	c ^= CL_MUL128(t, ctx->KBP, 0x10);
	return c[1];
}
uint32_t update_crc32b(uint32_t crc, const uint8_t * data, size_t length){
    return CRC32B_update_N(&CRC32B_ctx, crc, data, length);
}
uint32_t update_crc32k(uint32_t crc, const uint8_t * data, size_t length){
    return CRC32B_update_N(&CRC32K_ctx, crc, data, length);
}
uint32_t update_crc32c(uint32_t crc, const uint8_t * data, size_t length){
    return CRC32B_update_N(&CRC32C_ctx, crc, data, length);
}
uint32_t update_crc32(uint32_t crc, const uint8_t * data, size_t length){
	uint64_t crc64	= (uint64_t)crc<<32;
	crc64 = CRC32_update_N(&CRC32_ctx, crc64, data, length);
	return crc64>>32;
}
uint32_t update_crc24(uint32_t crc, const uint8_t * data, size_t length){
	uint64_t crc64	= (uint64_t)crc<<40;
	crc64 = CRC32_update_N(&CRC24_ctx, crc64, data, length);
	return crc64>>40;
}
uint32_t update_crc16(uint32_t crc, const uint8_t * data, size_t length){
	uint64_t crc64	= (uint64_t)crc<<48;
	crc64 = CRC32_update_N(&CRC16_ctx, crc64, data, length);
	return crc64>>48;
}
#ifdef TEST_CRC64
#include <stdio.h>
#include <inttypes.h>
int main (){
    uint8_t crc_check[] ="123456789";
	uint8_t*data = malloc(256*1024);
	for (int i=0; i<256*1024; i++) {
		data[i] = i &0xFF;
	}
	int i, len =128*1024;
	uint64_t ts, t_min;
    uint64_t crc64;
//	crc64 = ~0ULL;

	int count = 8;
	t_min = ~0; crc64=~0uLL;
	do{// методика тестирования производительности
		__builtin_ia32_lfence();
		ts = __builtin_ia32_rdtsc();
		crc64 = CRC64_update_N(&CRC64WE_ctx, crc64, &data[0], len);
		ts = __builtin_ia32_rdtsc()-ts;
		if (t_min > ts) t_min = ts;
	} while(--count);
	printf("CRC64/WE = %016"PRIX64" (xN) %.2f bytes/clk %.1f GB/s @ 3.5 GHz\n", crc64 ^ ~0ULL, (double)(len)/(t_min), (double)(len)/(t_min)*3.5);

	crc64 = CRC64_update_N(&CRC64WE_ctx, ~0ULL, crc_check, 9);
	printf("CRC64/WE check = %016"PRIX64" (xN) %s\n", crc64 ^ ~0ULL, (crc64 ^ ~0ULL) == CRC64_WE_CHECK?"PASS":"FAIL");
	
	crc64 = CRC64_update_N(&CRC64WE_ctx, 0, crc_check, 9);
	printf("CRC-64/ECMA-182 check = %016"PRIX64" (xN) %s\n", crc64, (crc64) == CRC64_ECMA_CHECK?"PASS":"FAIL");


	count = 8;
	t_min = ~0; crc64 = ~0uLL;
	do{// методика тестирования производительности
		__builtin_ia32_lfence();
		ts = __builtin_ia32_rdtsc();
		crc64 = CRC64B_update_N(&CRC64XZ_ctx,crc64, data, len);
		ts = __builtin_ia32_rdtsc() - ts;
		if (t_min > ts) t_min = ts;
	} while(--count);
	printf("CRC64/XZ = %016"PRIX64" (xN) %.2f bytes/clk %.1f GB/s @ 3.5 GHz\n", crc64 ^ ~0ULL, (double)(len)/(t_min), (double)(len)/(t_min)*3.5);

	crc64 = CRC64B_update_N(&CRC64XZ_ctx, ~0ULL, crc_check, 9);
	printf("CRC64/XZ check = %016"PRIX64" (xN) %s\n", crc64 ^ ~0ULL, (crc64 ^ ~0ULL) == CRC64_XZ_CHECK?"PASS":"FAIL");

	crc64 = CRC64B_update_N(&CRC64NV_ctx, ~0ULL, crc_check, 9);
	printf("CRC64/NV check = %016"PRIX64" (xN) %s\n", crc64 ^ ~0ULL, (crc64 ^ ~0ULL) == CRC64_NVME_CHECK?"PASS":"FAIL");

	crc64 = CRC64B_update_N(&CRC64MS_ctx, ~0ULL, crc_check, 9);
	printf("CRC64/MS check = %016"PRIX64" (xN) %s\n", crc64, (crc64) == CRC64_MS_CHECK?"PASS":"FAIL");

	crc64 = CRC64B_update_N(&CRC64GO_ctx, ~0ULL, crc_check, 9);
	printf("CRC64/GO check = %016"PRIX64" (xN) %s\n", crc64 ^ ~0ULL, (crc64 ^ ~0ULL) == CRC64_GO_CHECK?"PASS":"FAIL");
	uint32_t crc32;
	crc32 = update_crc32k(~0u, crc_check, 9);
	printf("CRC32B/K check = %08"PRIX32" (xN) %s\n", crc32 ^ 0U, (crc32 ^ 0U) == CRC32K_CHECK?"PASS":"FAIL");
	crc32 = update_crc32c(~0u, crc_check, 9);
	printf("CRC32B/C check = %08"PRIX32" (xN) %s\n", crc32 ^ ~0U, (crc32 ^ ~0U) == CRC32C_CHECK?"PASS":"FAIL");
	crc32 = update_crc32b(~0u, crc_check, 9);
	printf("CRC32B   check = %08"PRIX32" (xN) %s\n", crc32 ^ ~0U, (crc32 ^ ~0U) == CRC32B_CHECK?"PASS":"FAIL");

	count = 8;
	t_min = ~0; crc32 = ~0;
	do{
		__builtin_ia32_lfence();
		ts = __builtin_ia32_rdtsc();
		crc32 = update_crc32b(crc32, data, len);
		ts = __builtin_ia32_rdtsc() - ts;
		if (t_min > ts) t_min = ts;
	} while(--count);
	printf("CRC32B      = %08"PRIX32" (xN) %.2f bytes/clk %.1f GB/s @ 3.5 GHz\n", crc32 ^ ~0ULL, (double)(len)/(t_min), (double)(len)/t_min*3.5);
	count = 8;
	t_min = ~0; crc32 = ~0;
	do{
		__builtin_ia32_lfence();
		ts = __builtin_ia32_rdtsc();
		crc32 = update_crc32(crc32, data, len);
		ts = __builtin_ia32_rdtsc() - ts;
		if (t_min > ts) t_min = ts;
	} while(--count);
	printf("CRC32/CKSUM = %08"PRIX32" (xN) %.2f bytes/clk %.1f GB/s @ 3.5 GHz\n", crc32 ^ ~0ULL, (double)(len)/(t_min), (double)(len)/t_min*3.5);


	crc32 = update_crc32(~0u, crc_check, 9);
	printf("CRC32/BZIP2  check = %08"PRIX32" (xN) %s\n", crc32 ^ ~0U, (crc32 ^ ~0U) == CRC32_CHECK?"PASS":"FAIL");

	ts = __builtin_ia32_rdtsc();
	crc32 = update_crc24(CRC24_PGP_INIT, crc_check, 9);
	ts-= __builtin_ia32_rdtsc();

	printf("CRC-24/PGP = %06X (xN) %u cycles %s\n", crc32, -ts, crc32==CRC24_PGP_CHECK?"PASS":"FAIL");
    return 0;
}
#endif