#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define BITS   64

// Пример миксера (замени на свой)
static uint64_t splitmix64(uint64_t *state) {
    uint64_t z = (*state += 0x9e3779b97f4a7c15ull);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    return z ^ (z >> 31);
}
// Doug Lea's mixing function, fastmix дважды
static inline uint64_t mix_lea(uint64_t h) {
    const uint64_t C = 0xdaba0b6eb09322e3;
//    const uint64_t C = 0x9fb21c651e98df25L;
  h ^= h >> 32;
  h *= C;
  h ^= h >> 28;
  h *= C;
  h ^= h >> 33;
  return h;
}
static inline uint64_t unmix_lea(uint64_t h) {
    const uint64_t C = 0xa6f8e26927e132cb;
  h ^= h >> 32;
  h *= C;
  h ^= h >> 32;
  h *= C;
  h ^= h >> 32;
  return h;
}

static inline uint64_t mix_stafford13(uint64_t h) {
  h ^= h >> 30;
  h *= 0xbf58476d1ce4e5b9ull;
  h ^= h >> 27;
  h *= 0x94d049bb133111ebull;
  h ^= h >> 31;
  return h;
}


#define A0 0xffebb71d94fcdaf9ull
//#define A0 0xff3a275c007b8ee6ull
uint64_t mwc128(uint64_t* s){
    uint64_t r = s[0]^s[1];
    unsigned __int128 t = (unsigned __int128)A0 * s[0] + s[1];
    s[0] = t;
    s[1] = t>>64;
    return r;
}
static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) ^ (x >> (64 - k));
}
uint64_t xoroshiro128_next(uint64_t *s)
{
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t r = s0 + s1;
	s1 ^= s0;
	s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
	s[1] = rotl(s1, 37); // c
	return r;
}

/*!  \brief измерение среднеквадратичного эффекта 

uint64_t next(uint64_t *state, uint64_t gamma) {
  uint64_t s = *state;
  *state += gamma;
  return mix(s);
}
// This is fed to TestU01 by
static void runTestsInt(unsigned int (*prng)(void), char name[]) {
  unif01_Gen* gen = unif01_CreateExternGenBits(name, prng);
  bbattery_BigCrush(gen);

  unif01_DeleteExternGenBits(gen);
}
 */
double sac(uint64_t (*mixer) (uint64_t), double * mean_bias, int expNr)
{
    uint64_t Nr = 1uLL<<expNr;
    uint64_t counts[BITS][BITS] = {0};
    uint64_t seed[2] = {0x517cc1b727220a94ull,1};  // любой хороший seed

    for (uint64_t trial = 0; trial < Nr; trial++) {
        uint64_t x  = xoroshiro128_next(seed);   // или mwc128...
//        uint64_t x  = mwc128(seed);   // или mwc128...
        uint64_t hx = mixer(x);

        for (int i = 0; i < BITS; i++) {
            uint64_t xi  = x ^ (1ULL << i);
            uint64_t hxi = mixer(xi);
            uint64_t diff = hx ^ hxi;

            for (int j = 0; j < BITS; j++) {
                if (diff & (1ULL << j))
                    counts[i][j]++;
            }
        }
    }

    double max_bias  = 0.0;
    double sum_bias  = 0.0;
    double sum_sqr   = 0.0;

    int total = 0;

    for (int i = 0; i < BITS; i++) {
        for (int j = 0; j < BITS; j++) {
            double p    = (double)counts[i][j] / Nr;
            double bias = (p - 0.5);
            sum_bias += fabs(bias);
            sum_sqr  += bias*bias;
            if (bias > max_bias) max_bias = bias;
            total++;
        }
    }

    *mean_bias = sum_bias / total;
//    return max_bias;
    return sqrt(sum_sqr / total);
}

static uint64_t xorshift64s1_A3_mix(uint64_t x) {
	x ^= x >> 19; // c
	x ^= x << 29; // b
	x ^= x >>  8; // a
	return x * UINT64_C(0xd605bbb58c8abbfd);
}
//#define mixer mix_stafford13
int main(void) 
{
	struct _op {
		const char* name;
		int a;
		uint64_t prime1;
		int b;
		uint64_t prime2;
		int c;
	} op[] = {

#if 1
{"M^49614", 32,0xe5ef0457f6d7f895u,29, 0xaa1d0a6feb0712bdu,32},
{"M^49664", 32,0xe0a2078be06803fdu,29, 0xd6ee59b37ec47155u,32},
{"M^49674", 32,0xdeede9d2049d0805u,29, 0xc438cbdd6e0684cdu,32},
{"M^49676", 32,0xdde4704e8853402du,29, 0x97476d5800090fa5u,32},
{"M^49692", 32,0xdef9b3e4f4f8d66du,29, 0xc010af276efca365u,32},
{"M^49720", 32,0xfdd5508dc891adddu,29, 0x9f795cad1dda1275u,32},
{"M^49724", 32,0xe08a1c6ef417f2edu,29, 0x969fbfaabc723ae5u,32},
{"M^49726", 32,0xe517429390b74255u,29, 0x8b802bc3c27baafdu,32},
{"M^49740", 32,0xfec7f9fdca99792du,29, 0xfce2e08f9c2c3ea5u,32},
{"M^49780", 32,0xcadc9073136fc6cdu,29, 0xe1da85e06fa99605u,32},
{"M^49792", 32,0xdd973101001a95fdu,29, 0xd839ffc0e049ef55u,32},
{"M^49822", 32,0xd0e423f079fec3d5u,29, 0xdaa84663f49e5d7du,32},
{"M^49848", 32,0xc94719db83b8ffddu,29, 0xee362b98c5965075u,32},
{"M^49868", 32,0xd9fb16f68b64eb2du,29, 0xba59cd0c3e599ca5u,32},
{"M^49886", 32,0xf5aad9efd3e004d5u,29, 0xef2813c340c9147du,32},
{"M^49894", 32,0xd931ba17e9f6b6f5u,29, 0x89ea4004ad21c55du,32},
{"M^49932", 32,0xf7f8f8c00bea242du,29, 0xb505dff89263cba5u,32},
{"M^49952", 32,0xf35aacca6861ec7du,29, 0x87e1ddbc90ffecd5u,32},
{"M^49956", 32,0xcef2c1820709c38du,29, 0xdc4dd2bcea873745u,32},
{"M^49970", 32,0xeede649a7d8734a5u,29, 0x838ea425ef2d932du,32},
{"M^49974", 32,0xd2dd66c55b311835u,29, 0xd6170778012b7a1du,32},
{"M^49980", 32,0xfda1069b1f0dd6edu,29, 0xbe1cc63f4a93f6e5u,32},
{"M^49994", 32,0xdbd2cfea71ea7d05u,29, 0x871af1c515cfc7cdu,32},
{"M^50016", 32,0xfce0a5b80baf357du,29, 0x854b1a9628d6abd5u,32},
{"M^50020", 32,0xdab710c16caddc8du,29, 0xdab7405cc9694645u,32},
{"M^50026", 32,0xf18d4d6d4509b585u,29, 0xa075c8008c1ffb4du,32},
{"M^50034", 32,0xc47c41ce0f0205a5u,29, 0xffd11cb1b5b55a2du,32},
{"M^50050", 32,0xf11631fdae1ac1e5u,29, 0xb606de83361493edu,32},
{"M^50056", 32,0xe2983050e41b8d1du,29, 0x9539e790e4f59d35u,32},
{"M^50074", 32,0xf23b3d7b5547a245u,29, 0x84bcc6bcfbe7208du,32},
{"M^50076", 32,0xf46933c1e3a8ac6du,29, 0x91c1a67c5c163d65u,32},
{"M^50086", 32,0xdc057fe2fccd59f5u,29, 0xf8bf0ba534dfca5du,32},
{"M^50108", 32,0xe2a747c02d86c8edu,29, 0xb1a1a0c198f2d4e5u,32},
{"M^50114", 32,0xd3f06d83da1dd2e5u,29, 0xafafabe2a50e9aedu,32},
{"M^50116", 32,0xf0b48573cd6f620du,29, 0xa74879b06868bcc5u,32},
{"M^50144", 32,0xcf18a9c572f8c77du,29, 0xe15643091e7b29d5u,32},
{"M^50162", 32,0xc948f88ecadea7a5u,29, 0xfc65738ab803e82du,32},
{"M^50176", 32,0xf324b76868aa4bfdu,29, 0xc7eb947d50926955u,32},
{"M^50206", 32,0xcd7381d9831949d5u,29, 0xce1cd12fd109a77du,32},
{"M^50216", 32,0xc4714d16880d739du,29, 0xdcec310ab263aab5u,32},
{"M^50232", 32,0xe760b46893a6f5ddu,29, 0xc30816eb7b830a75u,32},

{"M^65062", 32,0xbeef9bdc186003f5u,29, 0xd672aed0a91a505du,32},
{"M^65072", 32,0xcc8efbc4734612bdu,29, 0xb14d24240750f895u,32},
{"M^65078", 32,0xfe599e61d0298435u,29, 0xf656f58ef3b0ae1du,32},
{"M^65082", 32,0xf1624afe78bf44c5u,29, 0x87d63098dfef9a0du,32},
{"M^65090", 32,0xce4b275d3da65ce5u,29, 0xdf51ddd4654c00edu,32},
{"M^65100", 32,0xb42d0766c0c6e92du,29, 0x8ef5bb172fc04ea5u,32},
{"M^65102", 32,0xf675f27a42d3ea95u,29, 0xbf66dd0d513890bdu,32},
{"M^65110", 32,0xecf766c34b31f4b5u,29, 0xdf47b520db62599du,32},
{"M^65116", 32,0x85e7e791e205bf6du,29, 0xa4a3f676ed2b2265u,32},
{"M^65118", 32,0xf1603dfa7aff72d5u,29, 0xfdae9d43e0dc367du,32},
{"M^65124", 32,0x9d425b172d4ae88du,29, 0x8747547fb2351a45u,32},
{"M^65126", 32,0xc5f8822331b8e4f5u,29, 0xccccd802e4f5a75du,32},
{"M^65130", 32,0x8ce55c318ba0e185u,29, 0xb3ac2565da1def4du,32},
{"M^65144", 32,0x9005125084f146ddu,29, 0x9c7cbd5ab4a6c175u,32},
{"M^65148", 32,0x8e49640b5f875bedu,29, 0x8a98f8d209df39e5u,32},
{"M^65150", 32,0xfe18ee9164f9f355u,29, 0xa4eec3fe624a71fdu,32},
{"M^65156", 32,0xaa5298505246d50du,29, 0xeef8a224330d01c5u,32},
{"M^65174", 32,0x9c4f9598060895b5u,29, 0xd96453e51469709du,32},
{"M^65194", 32,0xfa62021cc39d1285u,29, 0xb497649bcaa2164du,32},
{"M^65206", 32,0xf0642fd25016c635u,29, 0xf33d073a397edc1du,32},
{"M^65208", 32,0xeec5e9d2e1576fddu,29, 0xed5470fdb7836075u,32},
{"M^65214", 32,0xda040931146cb455u,29, 0xe00324fddb52a8fdu,32},
{"M^65220", 32,0xc7c0e2aac3566e0du,29, 0xad7fbca8236e90c5u,32},
{"M^65226", 32,0xea3521a5777c0b05u,29, 0xcb60c8b2f2a809cdu,32},
{"M^65244", 32,0xc5e9af3fd34bb16du,29, 0x83a137025ccf0065u,32},
{"M^65246", 32,0xaacef2e42c24f4d5u,29, 0xf193658c2eaca47du,32},
{"M^65256", 32,0xe8c25cbf6a0b569du,29, 0x961fcd63e0dddfb5u,32},
{"M^65268", 32,0xf7ddb24d4574e8cdu,29, 0xa3e4430db6654405u,32},
{"M^65274", 32,0xcc1bebf31e8d97c5u,29, 0xc2193e610b2bcf0du,32},
{"M^65280", 32,0xcb23a4f14ba597fdu,29, 0xa646e0c2d8667d55u,32},
{"M^65284", 32,0xafdab61d295b070du,29, 0xd37289501d3d1fc5u,32},
{"M^65288", 32,0xb9ee1a6c019e2b1du,29, 0xc12e9f0aef35ef35u,32},
{"M^65296", 32,0xef3e6560015cd23du,29, 0xb71afd2cffaed515u,32},
{"M^65306", 32,0x915feaa47f54b045u,29, 0xcc92ea3776a6e28du,32},
{"M^65308", 32,0xf8c194be706e2a6du,29, 0xb1d5e5490a346f65u,32},
{"M^65312", 32,0xa07f6f8dfffe5c7du,29, 0xf19e2727723afcd5u,32},
{"M^65326", 32,0xde60806605544e15u,29, 0xc8c6e5576352113du,32},
{"M^65344", 32,0xc031e14ae41060fdu,29, 0xcfb7d36a2f8ebc55u,32},
{"M^65356", 32,0x916903047d71cd2du,29, 0xcf92ee89260f0aa5u,32},
{"M^65366", 32,0x91eb3725edba78b5u,29, 0x8f54db87f39cb59du,32},
{"M^65372", 32,0xb80be386d1e5a36du,29, 0xd87aac89e4a6de65u,32},
{"M^65378", 32,0xa6a566bd122e0965u,29, 0xfc78e903456e006du,32},
{"M^65382", 32,0xc54e9233983e68f5u,29, 0xc0e350e0b215035du,32},
{"M^65384", 32,0xaecf77310c9e289du,29, 0xee8c75a05ac99db5u,32},
{"M^65408", 32,0xa9c63b3a73e029fdu,29, 0xf187af79e033fb55u,32},
{"M^65420", 32,0x925f503e92d1062du,29, 0xc526ace7a7e339a5u,32},
{"M^65428", 32,0xbf640bab82d3274du,29, 0xd4c5668096e66985u,32},
{"M^65442", 32,0x94cb59a8b57a9a65u,29, 0xfa481818674d876du,32},
{"M^65472", 32,0xa45b4ec28c14f2fdu,29, 0x95d0e210a1563a55u,32},
{"M^65484", 32,0xde72169606453f2du,29, 0x99b21bbe930468a5u,32},
{"M^65494", 32,0xd2195231102cbab5u,29, 0x9315839749d7e39du,32},
{"M^65502", 32,0xb3a78477de4bf8d5u,29, 0xb5a0fbe85509807du,32},
{"M^65504", 32,0xfec6c44926a5377du,29, 0xb86cfe29df4639d5u,32},
{"M^65508", 32,0xa0ab9aadc5e47e8du,29, 0xf8a604daccda7445u,32},
{"M^65514", 32,0x8abc5374692d0785u,29, 0xa7bfc938dd51d94du,32},
{"M^65516", 32,0xf03e7c3229973badu,29, 0xb481ef3bca41e025u,32},
{"M^65520", 32,0xe68dd6cf5fd8d1bdu,29, 0xc57daf346cf7f195u,32},
{"M^65526", 32,0x8e2913a6b6c36b35u,29, 0xaa9c736a9019cf1du,32},
{"M^65532", 32,0x86b23e9f811431edu,29, 0xcadf13806dcdd3e5u,32},


#endif

{"Custom 1",    32, 0xbf58476d1ce4e5b9, 28, 0x94d049bb133111eb, 33},// mix13
{"Custom LXM2", 32, 0xd605bbb58c8abbfd, 29, 0xad4d2974b2f97955, 32},//
{"Custom2",     32, 0xdaba0b6eb09322e3, 33, 0xa6f8e26927e132cb, 32},
{"SplitMix64",  30, 0xbf58476d1ce4e5b9, 27, 0x94d049bb133111eb, 31},
//{"MurmurHash3", 33, 0xff51afd7ed558ccd, 33, 0xc4ceb9fe1a85ec53, 33},
{"Custom2",     32, 0xda942042e4dd58b5, 32, 0xda942042e4dd58b5, 32},
{"Doug Lea's",  32, 0xdaba0b6eb09322e3, 32, 0xdaba0b6eb09322e3, 32},
{"unmix Lea's", 32, 0xa6f8e26927e132cb, 32, 0xa6f8e26927e132cb, 32},
{"Avalanche",   33, 0xC2B2AE3D27D4EB4F, 29, 0x165667B19E3779F9, 32},
{"Mix3",        32, 0xbea225f9eb34556d, 29, 0xbea225f9eb34556d, 32},

	{"Mix01",31 ,0x7fb5d329728ea185 ,27 ,0x81dadef4bc2dd44d ,33},
	{"Mix02",33	,0x64dd81482cbd31d7	,31	,0xe36aa5c613612997	,31},
	{"Mix03",31	,0x99bcf6822b23ca35	,30	,0x14020a57acced8b7	,33},
	{"Mix04",33	,0x62a9d9ed799705f5	,28	,0xcb24d0a5c88c35b3	,32},
	{"Mix05",31	,0x79c135c1674b9add	,29	,0x54c77c86f6913e45	,30},
	{"Mix06",31	,0x69b0bc90bd9a8c49	,27	,0x3d5e661a2a77868d	,30},
	{"Mix07",30	,0x16a6ac37883af045	,26	,0xcc9c31a4274686a5	,32},
	{"Mix08",30	,0x294aa62849912f0b	,28	,0x0a9ba9c8a5b15117	,31},
	{"Mix09",32	,0x4cd6944c5cc20b6d	,29	,0xfc12c5b19d3259e9	,32},
	{"Mix10",30	,0xe4c7e495f4c683f5	,32	,0xfda871baea35a293	,33},
	{"Mix11",27	,0x97d461a8b11570d9	,28	,0x02271eb7c6c4cd6b	,32},
	{"Mix12",29	,0x3cd0eb9d47532dfb	,26	,0x63660277528772bb	,33},
	{"Mix13",30	,0xbf58476d1ce4e5b9	,27	,0x94d049bb133111eb	,31},
	{"Mix14",30	,0x4be98134a5976fd3	,29	,0x3bc0993a5ad19a13	,31},
	};
    struct _op *m = NULL;
    uint64_t mixer(uint64_t x){
        x ^= x>>m->a; x*=m->prime1;
//        x ^= x>>m->b; x*=m->prime2;
        x ^= x>>m->c;
        return x;
    }

    printf("SAC  (N=2^%.0f):\n", 22);
    double mean_bias, max_bias;

        max_bias = sac(xorshift64s1_A3_mix, &mean_bias, 20);
        printf("|%-12s| %9.6f | %9.6f |\n", "A3 mix", max_bias, mean_bias);


    int sz = sizeof(op)/sizeof(struct _op);
    printf("|%-12s| %9s | %9s |\n", "Mixer", "Max bias","Mean bias");
    double min_bias = 1;
    for (int i=0; i<sz;i++){
        m = op+i;
        max_bias = sac(mixer, &mean_bias, 20);
        int ok = mean_bias< min_bias;
        if (ok) min_bias = mean_bias;
        printf("|%-12s| %9.6f | %9.6f | %s\n", m->name, max_bias, mean_bias, ok? "min":">");
    }
    return 0;
}