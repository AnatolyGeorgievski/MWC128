/* Этот файл используется как include
<code>
	#define MP_SIZE 2
	#include "mp.c"
</code>

 */
#include <stdint.h>

#ifndef MP_SIZE
#warning "MP_SIZE set to default =4"
#define MP_SIZE 4
#endif

/* Multi-Precision modular arithmetic library
   \see https://gcc.gnu.org/onlinedocs/gccint/Machine-Modes.html
   \see https://github.com/gcc-mirror/gcc/blob/master/gcc/machmode.def

Basic integer modes.  We go up to TI in generic code (128 bits).
   TImode is needed here because the some front ends now genericly
   support __int128.  If the front ends decide to generically support
   larger types, then corresponding modes must be added here.  The
   name OI is reserved for a 256-bit type (needed by some back ends).

INT_MODE (QI, 1);
INT_MODE (HI, 2);
INT_MODE (SI, 4);
INT_MODE (DI, 8);
INT_MODE (TI, 16);   
 */
typedef   signed int __attribute__((mode(TI)))    int128_t;
typedef unsigned int __attribute__((mode(TI)))   uint128_t;
/*! \brief Модульное умножение с неполным редуцированием 
*/
/*! \brief Вычитание больших числе с переносом 
	\return перенос от старшего разряда, положительное число 0 или 1
 */
static inline int64_t mp_sub(uint64_t *a, const uint64_t *b)
{
	int64_t cy=0;
	for (int i=0; i<MP_SIZE; i++) {
		int128_t ac = (uint128_t)a[i] - (uint128_t)b[i] + cy;
		a[i] = ac;
		cy = ac>>64;
	}
	return cy;
}
//
static inline int64_t mp_sub_x(uint64_t *a, const uint64_t *b, int offset)
{
	int64_t cy=0;
	for (int i=offset; i<MP_SIZE; i++) {
		int128_t ac = (uint128_t)a[i] - (uint128_t)b[i-offset] + cy;
		a[i] = ac;
		cy = ac>>64;
	}
	return cy;
}
/*! \brief Сложение больших чисел с переносом 
 */
static inline uint64_t mp_add(uint64_t *a, const uint64_t *b)
{
	uint64_t cy=0;
	for (int i=0; i<MP_SIZE; i++) {
		int128_t ac = (uint128_t)a[i] + (uint128_t)b[i] + cy;
		a[i] = ac;
		cy = ac>>64;
	}
	return cy;
}
static inline void mp_mov(uint64_t *a, const uint64_t *b)
{
	for (int i=0; i<MP_SIZE; i++) {
		a[i] = b[i];
	}
}
static inline void mp_clr(uint64_t *a)
{
	for (int i=0; i<MP_SIZE; i++) {
		a[i] = 0;
	}
}

/*! \brief Модульное умножение большого числа на скаляр
 */
static inline uint64_t mp_mul_ui(uint64_t *r,  const uint64_t *b, const uint64_t a)
{
	uint64_t cy=0;
	for (int i=0; i<MP_SIZE; i++) {
		uint128_t ac = (uint128_t)a*b[i] + cy;
		r[i] = ac;
		cy = ac>>64;
	}
	return cy;
}
/*! \brief Модульное умножение большого числа на скаляр, результат суммируется
	\return пенос - старшие разряды числа, 64 бита
 */
static inline uint64_t mp_mac_ui(uint64_t *r,  const uint64_t *b, uint64_t a)
{
	uint64_t cy=0;
	for (int i=0; i<MP_SIZE; i++) {
		uint128_t ac = (uint128_t)a*b[i] + (uint128_t)r[i] + cy;
		r[i] = ac;
		cy = ac>>64;
	}
	return cy;
}
static inline int64_t mp_msb_ui(uint64_t *r,  const uint64_t *b, uint64_t a)
{
	int64_t cy=0;
	for (int i=0; i<MP_SIZE; i++) {
		int128_t ac = (uint128_t)r[i] - (uint128_t)a*b[i] + cy;
		r[i] = ac;
		cy = ac>>64;
	}
	return cy;
}
/*! \brief сравнение больших чисел */
static inline int mp_ge(uint64_t *a,  const uint64_t *b)
{
	int i;
	for (i=MP_SIZE-1; i>0; --i) {
		if (a[i]!=b[i]) break;
	}
	return a[i]>=b[i];
}
/*! \brief сравнение со сдвигом */
static inline int mp_ge_x(uint64_t *a, uint64_t ac, const uint64_t *b)
{
	int i;
	if (ac != b[MP_SIZE-1]) return ac>=b[MP_SIZE-1];
	for (int i=MP_SIZE-2; i>0; --i) {
		if (a[i]!=b[i]) break;
	}
	return a[i]>=b[i];
}
/*! \brief сравнение больших чисел */
static inline int64_t mp_cmp(uint64_t *a,  uint64_t *b)
{
	int64_t diff;
	for (int i=MP_SIZE-1; i>=0; --i) {
		diff = a[i] - b[i];
		if (diff) break;
	}
	return diff;
}

/*! \brief Специальный вид инверсии для алгоритма редуцирования 
Это инверсия получается методом ((1<<128) - (P<<64))/P - тут мы сначала редуцировали степень (1<<128)
 */
static inline uint64_t INVL(uint64_t v)
{
#if 0
    return ((uint128_t)(-v)<<64)/v;
#else 
	return (~(uint128_t)0)/v;
#endif
}
/*! \brief Сложение с неполным редуцированем
 */
static inline void mp_addm(uint64_t* r, uint64_t* a, const uint64_t* P)
{
	uint64_t cy = mp_add(r,a);
	if (cy) mp_sub(r,P);
}
// этот вариант для отладки
static inline void mp_mulm_u_(uint64_t *r, uint64_t *b, uint64_t a, const uint64_t* P)
{	
	for (int i=0; i<MP_SIZE; i++) r[i]=0;
	while(a!=0){
		if(a&1)
			mp_addm(r,b,P);
		mp_addm(b,b,P);
		a=a>>1;
	}
	//if (r>=M) r -= M;// отложенное редуцирование
	if (mp_ge(r, P)) mp_sub(r, P);
}

/*! \brief Редуцирование по модулю простого числа
	\param r -- на входе значение, на выходе - редуцированое число, с учетом переноса
	\param rc - перенос результата в старшие разряды (старшие 64 бита)
	\param P -- модуль 
	
Редуцирование для чисел разрядностью в одно слово:	
uint reduce(uint a) {
    q:= a / P  // Division implicitly returns the floor of the result.
    return a - q * P
}
Следующий шаг - замена деления на приближенное вычисление умножения на обратное число U
	q := (a * U) >> k
Поскольку обратное число вычисляется приближенно, то необходимо корректировать результат - добавляется одна проверка
uint reduce(uint a){
    q := (a * U) >> k
    a -= q * P
    if (a >= P) {
        a -= P, q++
    }
    return a
}

Дано: 
L - разрядность слова 64 бит, S - разрядность числа 2^{S-1}<= P < 2^S
U = floor({2^(S+L)-P*2^L}/P) + 2^L
Алгоритм:
1. A = A mod P*2^S
2. c = A/2^S
   q = (c * U)>>L + c
3. R = A - q*P 
4. if (R < 0 )   R = R + P, q--; -- может быть не реализуется
4. if (R >= 2^S) R = R - P, q++; 
5. if (R >= P)   R = R - P, q++;
Результат:
Остаток от деления R = A mod P 
 */
static int err_count=0;
static void mp_mod_ui(uint64_t *r, uint64_t rc, const uint64_t* P)
{
	uint64_t x[MP_SIZE];
	uint64_t q;
/* нужно чтобы результат деления был меньше 2^L, надо доказать!! 
P = A*B-1, мы используем простые числа P в такой форме где B=2^{S-L}
При вычислении константы барретта используется тождество:
U = floor({2^2L}/A) == floor((2^2L - A*2^L)/A) + 2^L 
 */
	if (mp_ge_x(r+1,rc,P)){// сравнение со смещением индекса
		rc -= P[MP_SIZE-1];
		rc += mp_sub_x(r, P, 1);
	}
#if defined(MWC_P_INVL)
	q = ((rc*(uint128_t)MWC_P_INVL)>>64) + rc;// число 2^63 < P_INV < 2^64, сташий бит = 1<<64, INVL - малдшая часть числа
#elif 1//defined(MWC_A1) -- этот вариант рабочий
	uint128_t q2  = ((uint128_t)rc<<64) | r[MP_SIZE-1];
	q = q2/(P[MP_SIZE-1]+1); 
	/* делением тоже можно получить, но длинное деление эмулируется!
	На платформе x86_64 целочисленное деление 128 бит/ 64 бит - возвращает 64 бит - осаток от деления
	Но вызывает функцию __udivti3, не всегда компилится в инструкцию DIV
	\see https://github.com/gcc-mirror/gcc/blob/master/libgcc/udivmodsi4.c
	*/
#else // этот вариант не годится если разрядность P меньше 64
	q = ((rc*(uint128_t)INVL(P[MP_SIZE-1]+1))>>64) + rc;
#endif
	int64_t cy = rc - mp_mul_ui(x, P, q);// Магия на основе редуцирования Баррета R = A - q*P
	cy+= mp_sub(r, x);// r = r - q*P
	if (cy<0) {
		cy+= mp_add(r, P);
		err_count++;
	} else {
		if (cy>0) {// коррекция результата
			cy+= mp_sub(r, P);
			err_count++;
		}
		if (cy>0 || mp_ge(r, P)) {
			cy+= mp_sub(r, P);
			err_count++;
		}
	}
}
/*! \brief Сдвиг влево на слово (64бит) с редуцированием */
static void mp_shlm(uint64_t* r, const uint64_t* P)
{
	uint64_t rc = r[MP_SIZE-1];
	int i;
	for (i=MP_SIZE-1; i>0; --i)
		r[i] = r[i-1];
	r[0] = 0;
	mp_mod_ui(r, rc, P);
}
/*! \brief Сдвиг вправо на слово (64бит) с редуцированием */
static void mp_shrm(uint64_t* r, const uint64_t* P)
{
	uint64_t rc = mp_mac_ui(r, P, r[0]);// Вероятно это работатет только потому что P = AB-1
	int i;
	for (i=0; i<MP_SIZE-1; i++)
		r[i] = r[i+1];
	r[i] = rc;
}
/*! \brief Модульное уполовинивание 
	\param r - вектор, на выходе результат
	\param P - модуль
 */
static void mp_hlvm(uint64_t* r, const uint64_t* P)
{
	int64_t cy = 0;
	if (r[0]&1){
		cy = mp_add(r,P);
	}
	// Сдвиг
	for (int i = MP_SIZE-1; i>=0; --i){
		uint64_t v = r[i];
		r[i] = (v>>1) | (cy<<63);
		cy = v;
	}
}
/*! \brief Модульное удвоение */
static void mp_dubm(uint64_t* r, const uint64_t* P)
{
	int64_t cy = 0;
	// Сдвиг
	for (int i = 0; i<MP_SIZE; i++){
		uint64_t v = r[i];
		r[i] = (v<<1) | (cy);
		cy = v>>63;
	}
	if (cy>0 || mp_ge(r,P)){
		cy += mp_sub(r,P);
	}
}

static void mp_mulm_ui(uint64_t * r, const uint64_t * b, const uint64_t A, const uint64_t* P)
{
	uint64_t rc = mp_mul_ui(r, b, A);
	mp_mod_ui(r, rc, P);
}
static void mp_macm_ui(uint64_t * r, const uint64_t * b, const uint64_t A, const uint64_t* P)
{
	uint64_t rc = mp_mac_ui(r, b, A);
	mp_mod_ui(r, rc, P);
}
static void mp_mulm(uint64_t * s, const uint64_t * b, const uint64_t* a, const uint64_t* P)
{
	uint64_t r[MP_SIZE]={0};
	int i;
    for (i=MP_SIZE-1; i>=0; --i)
		if (a[i]!=0) break;
	if (i>=0){
		mp_mulm_ui(r, b, a[i], P);
		for (i=i-1; i>=0; --i){	
			mp_shlm(r, P);// сдвиг влево на слово
			if (a[i]!=0) 
				mp_macm_ui(r, b, a[i], P);
		}
		mp_mov(s,r);
	} else 
		mp_clr(s);
}
static void mp_powm_ui(uint64_t * s, const uint64_t * b, uint64_t a, const uint64_t* P)
{
	uint64_t r[MP_SIZE];
	mp_mov(r, b);
	mp_clr(s); s[0] = 1;
	int i;
    while (a!=0) {
		if (a&1) 
			mp_mulm(s,s,r,P);
		mp_mulm(r,r,r,P);
		a>>=1;
	}
}
#if 1
/*! Вариант возведения в степень слева направо */
static void mp_powm(uint64_t * r, const uint64_t * x, uint64_t * a, const uint64_t* P)
{
    uint64_t s[MP_SIZE];
    mp_mov(s, x);
	int k;
	for (k=MP_SIZE-1;k>=0; --k)
		if (a[k]!=0) break;

	uint64_t e = a[k];
    int i = 64 - __builtin_clzll(e);
    mp_mov(r, s);
    for (i=i-2;i>=0; i--){
        mp_mulm(r, r, r, P);// modular squaring
        if ((e>>i)&1) {
            mp_mulm(r, r, s, P);
        }
    }
	for (k=k-1;k>=0; --k)
	{
		e = a[k];
		for (i=63;i>=0; i--){
			mp_mulm(r, r, r, P);// modular squaring
			if ((e>>i)&1) {
				mp_mulm(r, r, s, P);
			}
		}
		
	}
}
#endif
