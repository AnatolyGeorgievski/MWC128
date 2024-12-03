# MWC128 - Multiply With Carry Рseudo-Random Number Generators  (PRNG)

* __Анатолий М. Георгиевский, ИТМО__

Алгоритмы данной группы используются для генерации последовательности псеводслучайных чисел с однородным распределением.

Методы пригодны для параллельной реализации в задачах моделирования физических процессов. Основной показатель применимости методов для параллельных (высокопроизводительных) вычислений - это возможность получения последовательности с заданным отступом для распределения задачи между вычислительными ядрами, чтобы результат параллельной обработки давал в точности такой же результат, как и последовательное вычисление. 

В данном проекте даются методы вычисления отсупов основанные на модульной арифметике большой разрядности (multi-prcision, см. [mp.c](mp.c)). 

## Теория

Последовательность MWC — это последовательность пар $x_{n},c_{n}$, определяемых
```math
x_{n}=(ax_{n-1}+c_{n-1})\,{\bmod {\,}}b,\ c_{n}=\left\lfloor {\frac {ax_{n-1}+c_{n-1}}{b}}\right\rfloor
```

Последовательность с запаздыванием $r$ является обобщением последовательности с запаздыванием на 1, 
позволяющим использовать более длинные периоды [2]. Последовательность MWC с запаздыванием $r$ — это последовательность пар $x_{n},c_{n}$ (для $n>r$), определяемых
```math
x_{n}=(ax_{n-r}+c_{n-1})\,{\bmod {\,}}b,\ c_{n}=\left\lfloor {\frac {ax_{n-r}+c_{n-1}}{b}}\right\rfloor
```


Утверждение #1. $(x+c b)A \bmod (Ab - 1) \equiv xA + c$

Доказательство:
$(x+cb)A  = xA + c(Ab-1) + c = xA + c(Ab-1) + c, \bmod (Ab - 1)$
Средняя часть обращается в ноль, потому что кратно модулю. 

Утверждение #2. $Ab \bmod (Ab - 1) = 1$. $А$ - обратное число для $b$

> Если $p = ab^r − 1$ является простым числом, то малая теорема Ферма гарантирует, что порядок любого элемента должен делиться на $p − 1 = ab^r − 2$, поэтому один из способов обеспечить большой порядок — выбрать $a$ так, 
чтобы $p$ было «безопасным простым числом», то есть p и $(p − 1)/2 = ab^r/2 − 1$ были простыми числами. В таком случае для $b = 2^{32}$ и $r = 1$ период будет равен $ab^r/2 − 1$, что приближается к $2^{63}$, что на практике может быть приемлемо большим подмножеством числа возможных 32-битных пар $(x, c)$.
-- перевод с [Wiki](https://en.wikipedia.org/wiki/Multiply-with-carry_pseudorandom_number_generator)

## MWC64x

Алгоритм разработан благодяря онлайн публикации [MWC64x](https://cas.ee.ic.ac.uk/people/dt10/research/rngs-gpu-mwc64x.html). К алгоритму приписал модульную арифметику с вычислением отступов для многопотокового вычисления. 

Генератор последовательности:
```c
uint64_t next( uint64_t* state, const uint64_t A)
{
    uint32_t x = *state;
    *state = A*(uint32_t)(x) + (x>>32);
    return x;
}
```
Таблица констант A для преобразования из диапазона 0xFFFF-0x1FFFF:
```
fffefd4e,fffefaa5,fffefa9c,fffef86b,fffef712,fffef592,fffef370,fffef0e2,
fffef0d6,fffeed07,fffeea1f,fffee9ad,fffee7d0,fffee7b2,fffee749,fffee500,
fffee4e8,fffee42b,fffee2f3,fffee2b4,fffedf2a,fffede76,fffedb10,fffed89a,
fffed885,fffed5b2,fffed462,fffed3ae,fffed024,fffed01b,fffecd60,fffecc43,
fffecbc5,fffeca90,fffec8b9,fffec7ff,fffec658,fffec39a,fffec289,fffebfd7,
fffebef9,fffebaeb,fffeb81b,fffeb6f8,fffeb512,fffeb37a,fffeb2b1,fffeaf24,
fffeaced,fffea942,fffea759,fffea405,fffea2f7,fffea2b2,fffea23a,fffe9e7a,
fffe9e65,fffe9c91,fffe9c8b,fffe9988,fffe9835,fffe982c,fffe981d,fffe9736,
fffe96dc,fffe9664,fffe953e,fffe944e,fffe9442,fffe930d,fffe915a,fffe9001,
fffe8e66,fffe8d37,fffe8c9b,fffe8c92,fffe8665,fffe863e,fffe83aa,fffe8224,
fffe81f1,fffe8173,fffe80ad,fffe7fa2,fffe7a2c,fffe7594,fffe7369,fffe700c,
fffe6fee,fffe6feb,fffe695b,fffe6946,fffe636a,fffe6337,fffe60af,fffe5c02,
fffe5bd5,fffe5a0a,fffe59bf,fffe59b0,fffe59a7,fffe53e3,fffe51fd,fffe4fcf,
fffe4d86,fffe4d0e,fffe49d5,fffe4858,fffe477d,fffe44fb,fffe4456,fffe42fd,
fffe412c,fffe4048,fffe3b6e,fffe3a33,fffe39b2,fffe398b,fffe358f,fffe31b4,
fffe30ac,fffe2a76,fffe29b9,fffe2305,fffe1d0b,fffe1c03,fffe1b3a,fffe1720,
fffe1702,fffe161b,fffe151c,fffe1495,fffe123d,fffe10f3,fffe1054,fffe0f01,
```
Для поиска значений константы $A$ использовались тесты:
1. $x^{p-1}=1$ тест Ферма
2. $x^{(p/2-1)}=1$ хотя для $x=2$ или $x=3$. тест Ферма
3. $p$ - простое число, проверка по таблице
4. $p/2-1$ - простое число, по таблице
5. Делимость $p \bmod {24} = 23$, $a \bmod {3} = 0$
6. $\overline{a}-1$ - простое число

* Все тесты 1,2,3,4 - вероятностные, мог пропустить составное число

* Алгоритм генерации таблицы прстых чисел и проверка простоты по таблцие см. [prime.c](prime.c)

Алгоритм возведения в сепень для теста Ферма, 64-битыне числа:
```c
typedef unsigned int __attribute__((mode(TI)))   uint128_t;
static uint64_t powm(const uint64_t b, uint64_t a, const uint64_t P)
{
    uint64_t r;
    r = b;
    uint64_t s = 1;
    int i;
    while (a!=0) {
	if (a&1) 
		s = ((uint128_t)s*r)%P;
	r = ((uint128_t)r*r)%P;
	a>>=1;
    }
    return s;
}
```
  
## MWC32

Практическое решение задачи поиска константы для 32 битных чисел дано двумя методами. Оба метода реализованы путем расчета *всей* последовательности.
1. Путем тестирования множества простых чисел в диапазоне 512-1023
2. Путем последовательного перебора чисел в диапазоне [0..1023] (дата майниг)

Критерием выбора константы $A$ для генератора является длина пероида повторения последовательности, которая дается формулой $ab^r/2 − 2$.

Тестом на простоту Ферма является утверждение $x^{p-1} = 1$ для любого x.
Для чисел вида $A\cdot 2^{n} -1$ тестом простоты является LLR([Тест Люка — Лемера — Ризеля](https://en.wikipedia.org/wiki/Lucas%E2%80%93Lehmer%E2%80%93Riesel_test)), as a special case of the Morrison test. Реализация теста,см. проект [PRST by Pavel Atnashev](https://github.com/patnashev/prst)

Другой набор тестов включает однородность и возможность сжатия (муаровый узор на "салфетках"). Для исследования однородности использовалось построение изображений по большому числу циклов. Тесты наглядно показывают отсуствие водяных знаков при использовании младших 16-20 бит. Тест при числе проходов 0x7F (8 бит, grayscale):

![тест на однородность заполнения](test.png)

Для повышения однородности следует использовать только четные значения или только нечетные вызовы функции. 
Для A=0xFE94 при большом колчиестве циклов 0x7FFF, Формат файла (Grayscale 16 бит):
![тест на однородность заполнения](0xFE94.png)

"Безопасным простым" является P=0xfe9fffff (A=FEA0) с периодом повтора 0x7f4ffffe.
Генератор последовательности:
```c
uint32_t next( uint32_t* state, const uint32_t A)
{
	uint32_t x = *state;
	*state = A*(uint16_t)(x) + (x>>16);
    return x;
}
```
Ниже привожу таблицу всех простых чисел $P= (A<<16)-1$ c максимальным периодом повтора.
```
A=FFEA i=7ff4fffe ( 21), P mod 24 =23, A mod 3 =0
A=FFD7 i=7feb7ffe ( 40), P mod 24 = 7, A mod 3 =2
A=FFBD i=7fde7ffe ( 66), P mod 24 =23, A mod 3 =0
A=FFA8 i=7fd3fffe ( 87), P mod 24 =23, A mod 3 =0
A=FF9B i=7fcd7ffe (100), P mod 24 = 7, A mod 3 =2
A=FF81 i=7fc07ffe (126), P mod 24 =23, A mod 3 =0
A=FF80 i=7fbffffe (127), P mod 24 = 7, A mod 3 =2
A=FF7B i=7fbd7ffe (132), P mod 24 =23, A mod 3 =0
A=FF75 i=7fba7ffe (138), P mod 24 =23, A mod 3 =0
A=FF48 i=7fa3fffe (183), P mod 24 =23, A mod 3 =0
A=FF3F i=7f9f7ffe (192), P mod 24 =23, A mod 3 =0
A=FF3C i=7f9dfffe (195), P mod 24 =23, A mod 3 =0
A=FF2C i=7f95fffe (211), P mod 24 = 7, A mod 3 =2
A=FF09 i=7f847ffe (246), P mod 24 =23, A mod 3 =0
A=FF03 i=7f817ffe (252), P mod 24 =23, A mod 3 =0
A=FF00 i=7f7ffffe (255), P mod 24 =23, A mod 3 =0
A=FEEB i=7f757ffe (276), P mod 24 =23, A mod 3 =0
A=FEE4 i=7f71fffe (283), P mod 24 = 7, A mod 3 =2
A=FEA8 i=7f53fffe (343), P mod 24 = 7, A mod 3 =2
A=FEA5 i=7f527ffe (346), P mod 24 = 7, A mod 3 =2
A=FEA0 i=7f4ffffe (351), P mod 24 =23, A mod 3 =0
A=FE94 i=7f49fffe (363), P mod 24 =23, A mod 3 =0
A=FE8B i=7f457ffe (372), P mod 24 =23, A mod 3 =0
A=FE72 i=7f38fffe (397), P mod 24 = 7, A mod 3 =2
A=FE4E i=7f26fffe (433), P mod 24 = 7, A mod 3 =2
A=FE30 i=7f17fffe (463), P mod 24 = 7, A mod 3 =2
A=FE22 i=7f10fffe (477), P mod 24 =23, A mod 3 =0
A=FE15 i=7f0a7ffe (490), P mod 24 = 7, A mod 3 =2
A=FE04 i=7f01fffe (507), P mod 24 =23, A mod 3 =0
```
