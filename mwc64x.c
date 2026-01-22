#include <stdint.h>
#include <math.h>
/* 

	\see https://cas.ee.ic.ac.uk/people/dt10/research/rngs-gpu-mwc64x.html
	\see https://prng.di.unimi.it/
Тестирование:
$ gcc -DTEST_MWC64X -march=native -O3 -o test mwc64x.c
$ time ./test.exe
real    0m8.055s

1) Тест на однородность распределения - строится гистограмма на 2^15 8192 точки, 
по ней считается среднее отклонение от среднего
2) Тест на заполнение - вероятность найти 1 в числе 32 бита - гауссова кривая
3) Тест на упаковку, последовательности 01 10 11 и 00

*/
/* А и M - одно число M = (A<<32)-1 */
enum{ MWC64X_A = 4294883355U };				// FFFEB81B
enum{ MWC64X_M = 18446383549859758079UL };  // FFFEB81AFFFFFFFF =A*B-1, B=1<<32
#define MWC_A0 0xfffeb81bULL
#define MWC_A1 0xffebb71d94fcdaf9 // MWC128
#define MWC_A2 0xffa04e67b3c95d86 // MWC192
#define MWC_A3 0xfff62cf2ccc0cdaf // MWC256
#if 0
#define MWC_A1 0xff3a275c007b8ee6 // MWC128
#define MWC_A3 0xff377e26f82da74a // MWC256
#endif


static const uint64_t MWC_M0[2] = { (MWC_A0<<32) - 1 };
static const uint64_t MWC_M1[2] = { 0xffffffffffffffff, MWC_A1 - 1 };
static const uint64_t MWC_M2[3] = { 0xffffffffffffffff, 0xffffffffffffffff, MWC_A2 - 1 };
static const uint64_t MWC_M3[4] = { 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, MWC_A3 - 1 };

/* The state must be initialized so that 0 < c < MWC_A1 - 1.
   For simplicity, we suggest to set c = 1 and x to a 64-bit seed.
uint64_t x, c;
*/

/*! \return (a+b) mod M */
static inline uint64_t mwc_addm(uint64_t a, uint64_t b, uint64_t M)
{
	uint64_t v;
	if( __builtin_add_overflow (a,b,&v)/*(v>=M) || (v<a) */)
		v -= M;// упрощенное редуцирование
	return v;
}
/*! \return (a-b) mod M */
static inline uint64_t mwc_subm(uint64_t a, uint64_t b, uint64_t M)
{
	uint64_t v;
	if( __builtin_sub_overflow (a,b,&v)/*(v>=M) || (v<a) */)
		v += M;// упрощенное редуцирование
	return v;
}
/*! \brief Специальный вид инверсии для алгоритма редуцирования */
static inline uint64_t INVL(uint64_t v)
{
    return ((unsigned __int128)(-v)<<64)/v;
}


/*! \brief Модульное умножение с неполным редуцированием 

	while (ac>>64) 
		ac-= (ac>>64)*M;


*/
static inline uint64_t mwc_mulm(uint64_t a, uint64_t b, uint64_t M)
{	
	unsigned __int128 ac;
	ac = (unsigned __int128)a*b;
//	if ((ac>>64)>=M) ac-= (unsigned __int128)M<<64;// старшая часть требует редуцирования
	ac-= (((ac>>64)*INVL(M)+ ac)>>64)*M;// Магия на основе редуцирования Баррета

	if (ac>>64) {
		ac -= M;
		//if (ac>>64) ac -= M;
	} 
	if ((uint64_t)ac>= M) ac -= M;
	return ac;
}
#if 0 // простой алгоритм для сравнения и отладки
static uint64_t mwc_mulm_(uint64_t a, uint64_t b, uint64_t M)
{	
	uint64_t r=0;
	while(a!=0){
		if(a&1)
			r=mwc_addm(r,b,M);
		b=mwc_addm(b,b,M);
		a=a>>1;
	}
	//if (r>=M) r -= M;// отложенное редуцирование
	return r;
}
#endif
/*! \brief Модульнуя операция возведения в степень с неполным редуцированием */
static uint64_t mwc_powm(uint64_t a, uint64_t e, uint64_t M)
{
	uint64_t sqr=a, acc=1;
	while(e!=0){
		if(e&1)
			acc=mwc_mulm(acc,sqr,M);
		sqr=mwc_mulm(sqr,sqr,M);
		e>>=1;
	}
	//if (acc>=M) acc -= M;// отложенное редуцирование
	return acc;
}
/*! \biref Инициализация потока */
uint64_t mwc64x_seed(uint32_t* state, uint64_t seed)
{
	enum{ MWC_BASEID = 4077358422479273989ULL };
	//uint64_t m=MWC_PowMod64(MWC64X_A, dist, MWC64X_M);
	//uint64_t x=MWC_MulMod64(MWC_BASEID, m, MWC64X_M);
	uint64_t x= MWC_BASEID;
	state[0] = x;//(uint32_t)(x/MWC64X_A);
	state[1] = x>>32;//(uint32_t)(x%MWC64X_A);
	return x;
}
/*! \brief Проупск сегмента в поле генерации псевдо-случайного числа
	\param distance -- отступ для следующего сегмента от начала потока
 */
uint64_t mwc64x_skip(uint32_t* state, uint64_t distance)
{
	uint64_t m=mwc_powm(MWC64X_A, distance, MWC64X_M);
//	uint64_t x=state[0] + (uint64_t)state[1]<<32;//
	uint64_t x=state[0]*(uint64_t)MWC64X_A+state[1];
	x=mwc_mulm(x, m, MWC64X_M);
	//if (x>=MWC64X_M) x -= MWC64X_M;// отложенное редуцирование
//	state[0] = x;
//	state[1] = x>>32;
	state[0] = (uint32_t)(x/MWC64X_A);// замена на умножение и сдвиг
	state[1] = (uint32_t)(x%MWC64X_A);// остаток от деления
	return x;
}
/*! \brief Генерация входных данных для множетва потоков псевдо-случайного числа
	\param n_streams - число потоков
	\param distance - дистанция между потоками
 */
uint64_t mwc64x_streams(uint32_t* state, unsigned n_streams, uint64_t distance)
{
	uint64_t m=mwc_powm(MWC64X_A, distance, MWC64X_M);
	uint64_t x=state[0] + (uint64_t)state[1]<<32;//state[0]*(uint64_t)MWC64X_A+state[1];
	for (int i=0; i<n_streams; i++, state+=2){
		x=mwc_mulm(x, m, MWC64X_M);
		//if (x>=MWC64X_M) x -= MWC64X_M;// отложенное редуцирование
		state[0] = x;// (uint32_t)(x/MWC64X_A);// замена на умножение и сдвиг
		state[1] = x>>32;// (uint32_t)(x%MWC64X_A);// остаток от деления
	}
	return x;
}

/*! \brief Генерация псевдо-случайного числа. Один шаг алгоритма */
uint64_t mwc64x( uint64_t* state)
{
	uint64_t x = *state;
	*state = MWC_A0*(uint32_t)(x) + (x>>32);
    return x;//((x>>32) ^ (x&0xFFFFFFFFU));
}

/* This is a Goresky-Klapper generalized multiply-with-carry generator
   (see their paper "Efficient Multiply-with-Carry Random Number
   Generators with Maximal Period", ACM Trans. Model. Comput. Simul.,
   13(4), p. 310-321, 2003) with period approximately 2^127. While in
   general slower than a scrambled linear generator, it is an excellent
   generator based on congruential arithmetic.

   As all MWC generators, it simulates a multiplicative LCG with prime
   modulus m = 0xff002aae7d81a646007d084a4d80885f and multiplier given by
   the inverse of 2^64 modulo m. Note that the major difference with
   standard (C)MWC generators is that the modulus has a more general form.
   This additional freedom in the choice of the modulus fixes some of the
   theoretical issues of (C)MWC generators, at the price of one 128-bit
   multiplication, one 64-bit multiplication, and one 128-bit sum.
*/

#define GMWC_MINUSA0 0x7d084a4d80885f
#define GMWC_A0INV 0x9b1eea3792a42c61
#define GMWC_A1 0xff002aae7d81a646

/* The state must be neither all zero, nor x = 2^64 - 1, c = GMWC_A1 +
   GMWC_MINUS_A0. The condition 0 < c < GMWC_A1 + GMWC_MINUS_A0 is thus
   sufficient. */
/* The state must be initialized so that GMWC_MINUS_A0 <= c <= GMWC_A1.
   For simplicity, we suggest to set c = 1 and x to a 64-bit seed. */
uint64_t inline gmwc128_next(uint64_t *s) {
	const unsigned __int128 t = (unsigned __int128)GMWC_A1 * s[0] + s[1];
	uint64_t x = GMWC_A0INV * (uint64_t)t;
	s[0] = x;
	s[1] = (t + (unsigned __int128)GMWC_MINUSA0 * x) >> 64;
	return x;
}
#undef GMWC_MINUSA0
#undef GMWC_A0INV
#define GMWC_MINUSA0 0x54c3da46afb70f
#define GMWC_A0INV 0xbbf397e9a69da811
#define GMWC_A3 0xff963a86efd088a2
uint64_t inline gmwc256_next(uint64_t *s) {
	const unsigned __int128 t = (unsigned __int128)GMWC_A3 * s[0] + s[3];
	uint64_t x = GMWC_A0INV * (uint64_t)t;
	s[0] = s[1];
	s[1] = s[2];
	s[2] = x;
	s[3] = (t + (unsigned __int128)GMWC_MINUSA0 * x) >> 64;
	return x;
}

/* преобразование чисел в формат float32 дает распределение [0,1) */
static inline float u64_float(uint64_t x) {
	return ((uint32_t)((x>>32) ^ (x&0xFFFFFFFFU)) >> 8) * 0x1.0p-24;
}
static inline double u64_double(uint64_t x) {
	return (x >> 11) * 0x1.0p-53;
}
float uniform (uint64_t *state) {
	return u64_float(mwc64x(state));
}
/* \brief Box-Muller transform
The standard Box–Muller transform generates values from the standard normal distribution 
(i.e. standard normal deviates) with mean 0 and standard deviation 1.
В данном алгоритме применил трюк (1-U) дает распредение (0,1]
 */
#define M_PI 3.14159265358979323846
float gaussian(uint64_t *state)
{
	float u1;
	do {
		u1 = uniform(state);		// однородное распрелеление [ 0,1)
	} while (0);//(u1==0);
	float u2 = uniform(state);// однородное распрелеление [0,1)
    return sqrt(-2.0f*log(1.0f-u1))*cos(2*M_PI*u2);// см функцию sincos() и cospi()
}
float exponent2(uint64_t *state)
{
	float u1;
	do {
		u1 = uniform(state);		// однородное распрелеление [ 0,1)
	} while (0);//(u1==0);
	float u2 = uniform(state);// однородное распрелеление [0,1)
	float r = sqrt(-2.0f*log(1.0f-u1));
    u1 = r *cos(2*M_PI*u2);// см функцию sincos() и cospi()
    u2 = r *sin(2*M_PI*u2);// см функцию sincos() и cospi()
	return u1*u1+u2*u2;
}
float exponent(uint64_t *state)
{
	float u1 = uniform(state);		// однородное распрелеление [ 0,1)
	return -log(1.0f-u1);
}

float maxwell_(uint64_t *state)
{

	float x =1.0f- uniform(state);		// однородное распрелеление [ 0,1)
	float y =1.0f- uniform(state);		// однородное распрелеление [ 0,1)
	x = sqrt(-log(x))*(x-0.5);
	//y = sqrt(-2*log(y));
	return x;
}

/* Marsaglia_polar_method

 */
float gaussian_(uint64_t *state)
{
	float fac, rsq, v1,v2,first;
	do{
		v1 = 2.0*uniform(state)-1.0;
		v2 = 2.0*uniform(state)-1.0;
		rsq = v1*v1 + v2*v2;
    } while ((rsq >= 1.0) || (rsq == 0.0));
    fac = sqrt(-2.0*log(rsq)/rsq);
    //second= v1*fac;
	first = v2*fac;
	return first;
}
float maxwell(uint64_t *state)
{
	float x = gaussian(state);
	float y = gaussian(state);
	float z = gaussian(state);
	return sqrtf(x*x + y*y + z*z)*0.5f;
}
#ifdef TEST_MWC64X 

/* Marsaglia multiply-with-carry generator */
/* uint64_t x, c -- состояние */
uint64_t mwc128_next(uint64_t* state) {
	const uint64_t result = state[0];
	const unsigned __int128 t = (unsigned __int128)MWC_A1 * state[0] + state[1];
	state[0] = t;
	state[1] = t >> 64;
	return result;
}
/* uint64_t x, y, c -- состояние */
uint64_t mwc192_next(uint64_t* state) {
	const uint64_t result = state[1];
	const unsigned __int128 t = (unsigned __int128)MWC_A2 * state[0] + state[2];
	state[0] = state[1];
	state[1] = t;
	state[2] = t >> 64;
	return result;
}
/* uint64_t x, y, z, c -- состояние */
uint64_t mwc256_next(uint64_t* state) {
	const uint64_t result = state[2];
	const unsigned __int128 t = (unsigned __int128)MWC_A3 * state[0] + state[3];
	state[0] = state[1];
	state[1] = state[2];
	state[2] = t;
	state[3] = t >> 64;
	return result;
}



#define mrg31k3p_M1 2147483647             /* 2^31 - 1 */
#define mrg31k3p_M2 2147462579             /* 2^31 - 21069 */

#define mrg31k3p_MASK12 511                /* 2^9 - 1 */
#define mrg31k3p_MASK13 16777215           /* 2^24 - 1 */
#define mrg31k3p_MASK2 65535               /* 2^16 - 1 */
#define mrg31k3p_MULT2 21069

#define mrg31k3p_NORM_cl_double 4.656612873077392578125e-10  /* 1/2^31 */
#define mrg31k3p_NORM_cl_float  4.6566126e-10
uint32_t clrngMrg31k3pNextState(uint32_t * state)
{
	uint32_t* g1 = state;
	uint32_t* g2 = state+3;
	
	uint32_t y1, y2;

	// first component
	y1 = ((g1[1] & mrg31k3p_MASK12) << 22) + (g1[1] >>  9)
	   + ((g1[2] & mrg31k3p_MASK13) <<  7) + (g1[2] >> 24);

	if (y1 >= mrg31k3p_M1) y1 -= mrg31k3p_M1;

	y1 += g1[2];
	if (y1 >= mrg31k3p_M1) y1 -= mrg31k3p_M1;
	
	g1[2] = g1[1];
	g1[1] = g1[0];
	g1[0] = y1;

	// second component
	y1 = ((g2[0] & mrg31k3p_MASK2) << 15) + (mrg31k3p_MULT2 * (g2[0] >> 16));
	if (y1 >= mrg31k3p_M2)	y1 -= mrg31k3p_M2;
	y2 = ((g2[2] & mrg31k3p_MASK2) << 15) + (mrg31k3p_MULT2 * (g2[2] >> 16));
	if (y2 >= mrg31k3p_M2)	y2 -= mrg31k3p_M2;
	y2 += g2[2];
	if (y2 >= mrg31k3p_M2)	y2 -= mrg31k3p_M2;
	y2 += y1;
	if (y2 >= mrg31k3p_M2)	y2 -= mrg31k3p_M2;
	g2[2] = g2[1];
	g2[1] = g2[0];
	g2[0] = y2;

	if (g1[0] <= g2[0])
		return (g1[0] - g2[0] + mrg31k3p_M1);
	else
		return (g1[0] - g2[0]); 
}

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
int main (){
//	for (int i=0; i<32; i++ ) printf (">%08X %08X %08X\n", (1UL<<i), sigma0((1UL<<i)), sigma1((1UL<<i)));
//	for (uint32_t i=1; i<0xFFFF; i+=(1>>7)|(1>>18) )
#define N 4096
#define Ng 256
	uint32_t hyst[N];
	uint32_t hystf[N];
	uint32_t hystg[Ng];
	uint32_t hystb[33];
	for (int i=0; i<N; i++ )hyst[i]=0;
	for (int i=0; i<N; i++ )hystf[i]=0;
	for (int i=0; i<Ng; i++ )hystg[i]=0;
	for (int i=0; i<33; i++ )hystb[i]=0;

	int count=0;
	uint32_t nonce0 =1U;
	uint64_t state =~0ULL;
	uint32_t state2[2] = {~0U, ~0U};
	uint32_t state3p[6]= {451,2,345,4,7,6};
	
	uint32_t* M = malloc((1UL<<(32-5))*4);
	for (int i=0; i<(1ULL<<(32-5)); i++ )M[i]=0;


	mwc64x_seed(state2, 1);
	state = ((uint64_t)state2[1]<<32) | (state2[0]);
	uint64_t x0;

	int fcol=0;
	int gcol=0;
	int bits = 20;
	int skip=3;
	
	for (x0=1; x0<(1ULL<<bits); x0++ ){
//		state =  x0;
		nonce0 = mwc64x(&state);
		int p = __builtin_popcount(nonce0);
		hystb[p] ++;
		
		float f = u64_float(nonce0);
		int32_t idx = (int32_t)(f*(N));
		if (idx>=0 && idx<N) 
			hystf[idx]++;
		else
			fcol++;
#if 1
		//f = gaussian(&state); skip = 3; // внутри два числа
		f = maxwell(&state); skip = 7; // внутри два числа
		idx = (int32_t)(f*(Ng/2));
		if (idx>=0 && idx<Ng) 
			hystg[idx]++;
		else
			gcol++;
		
//		nonce0 = mwc64x(&state);
//		nonce0 = mwc64x(&state);
//		nonce0 = mwc64x(&state);
//		nonce0 = mwc64x(&state);
		mwc64x_skip(state2, skip);
		if (1) {// Тест функции _skip
			if ((uint32_t)state!=state2[0] || (uint32_t)(state>>32)!=state2[1]){
				printf ("# %X %08X %08X %08X\n", (uint32_t)x0, (uint32_t)state, state2[0], state2[1]);
				_Exit(1);
			}
		}
#endif
//		nonce0 = clrngMrg31k3pNextState(state3p);
		if (M[nonce0>>5]&(1UL<<(nonce0&31))) {
			count++;
			//printf ("# %X %X %08X\n", count, (uint32_t)x0, nonce0);
			//if (count>8192) break;
		}
		M[nonce0>>5] |= (1UL<<(nonce0&31));
		hyst[nonce0>>(32-12)] ++;
	}
	//printf ("# %08X %08X\n", count, (uint32_t)x0);
	unsigned AVG = 1UL<<(bits-12);
	uint64_t xi=0;
	if (1) for (int i=0; i<N; i++){
		if (0){
			printf ("%d ", hyst[i]);
			if ((i&15)==15) printf ("\n");
		}
		uint32_t d = AVG - hyst[i];
		xi += (d*d);
	}
	float dev = sqrt((double)xi/(1UL<<12))/AVG;
	printf ("std dev %f %d\n", dev, count);

	if (1) for (int i=0; i<33; i++){
		printf (" %d", hystb[i]);
	}
	printf ("\n");
	printf ("\n");
	uint64_t avg = 0;
	if (1) for (int i=0; i<N; i++){
		avg += hystf[i];
//		printf (" %d", hystf[i]);
	}
	double avgd = (double)avg/N;
	double xid=0;
	for (int i=0; i<N; i++){
		double d = (avgd - hystf[i]);
		xid += (d*d);
	}
	dev = sqrt((double)xid/N)/avgd;
	printf ("fcol=%d dev=%f -- \n", fcol, dev);
	printf ("\n");

	for (int i=0; i<Ng; i++){
			printf ("%d\n", hystg[i]);
			//if ((i&7)==7) printf ("\n");
	}
	printf ("fcol=%d dev=%f -- \n", gcol, dev);
if (1){//
	uint64_t r = mwc_powm(MWC64X_A, MWC64X_M-1, MWC64X_M);
	printf ("prime test= %08X\n", r);
} 

	state  = 12345 + (1ULL<<32);
	state2[0] = 12345, state2[1]= 1;
	for (x0=1; x0<(1ULL<<5); x0++ ){

		nonce0 = mwc64x(&state);
		nonce0 = mwc64x(&state);
		nonce0 = mwc64x(&state);

		mwc64x_skip(state2, 3);
		if (1) if ((uint32_t)state!=state2[0] || (uint32_t)(state>>32)!=state2[1])
		{
			printf ("# %X %08X %08X %08X\n", (uint32_t)x0, (uint32_t)state, state2[0], state2[1]);
		}
	}
if (1){
	uint64_t v = -0x7d084a4d80885f;
	//uint64_t iv = (((unsigned __int128)(-v)<<64)+(0ULL))/v;
	uint64_t iv = (((unsigned __int128)(-v)<<64)+(~0ULL))/v;
	uint64_t t  = ((unsigned __int128)v*iv)>>64;
	printf("v %016llX %016llX %016llX\n", v, iv, t);// FF82F7B5B27F77A1
}
	return 0;
}
#endif
