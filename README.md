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
\begin{equation}
x_{n}=(ax_{n-r}+c_{n-1}) \bmod{b},
\ c_{n}=\left\lfloor {\frac {ax_{n-r}+c_{n-1}}{b}}\right\rfloor
\end{equation}
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

Отступы (мещения сегментов) вычисляются по формуле $A^d \cdot x \bmod P$, где $d$ - отступ сегмента. 

Генератор последовательности:
```c
uint64_t next( uint64_t* state, const uint64_t A)
{
    uint64_t x = *state;
    *state = A*(uint32_t)(x) + (x>>32);
    return x;
}
```
Таблица констант A для преобразования из диапазона 0xFFFF-0x1FFFF:
```
fffefd4e,fffefaa5,fffef712,fffef592,fffef0e2,fffeea1f,fffee9ad,fffee7b2,
fffee4e8,fffee42b,fffee2b4,fffedf2a,fffede76,fffedb10,fffed89a,fffed462,
fffed3ae,fffed024,fffecd60,fffecc43,fffecbc5,fffeca90,fffec289,fffebaeb,
fffeb81b,fffeb6f8,fffeaf24,fffea942,fffea759,fffea405,fffea2f7,fffe9e7a,
fffe9e65,fffe9835,fffe982c,fffe9736,fffe9664,fffe944e,fffe930d,fffe915a,
fffe9001,fffe8e66,fffe8c9b,fffe83aa,fffe81f1,fffe80ad,fffe7fa2,fffe7a2c,
fffe7594,fffe7369,fffe6fee,fffe6feb,fffe636a,fffe6337,fffe60af,fffe5bd5,
fffe5a0a,fffe59b0,fffe59a7,fffe53e3,fffe51fd,fffe4fcf,fffe4858,fffe4456,
fffe42fd,fffe3b6e,fffe398b,fffe358f,fffe2a76,fffe29b9,fffe1d0b,fffe1c03,
fffe1b3a,fffe1702,fffe1495,fffe0f01,fffe03e2,fffe03c4,fffe00f4,

```
Для поиска значений константы $A$ использовались тесты:
1. $x^{p-1}\equiv 1 \bmod p$ тест Ферма
2. $x^{(Ab/2-2)}\equiv 1 \bmod (Ab/2-1)$ тест Ферма
3. $p$ - простое число, проверка по таблице
4. $Ab/2-1$ - простое число, по таблице
5. Делимость $p \bmod {24} = 23$, $A \bmod {3} = 0$
6. $\overline{A}-1$ - простое число

* Все тесты 1,2,3,4 - вероятностные, мог пропустить составное число
* Свойство 5 применяется для классификации простых чисел в тесте LLR
* Тест 6 я применяю из собственных наблюдений.

* Алгоритм генерации таблицы прстых чисел и проверка простоты по таблцие см. [prime.c](prime.c)

Алгоритм возведения в сепень для теста Ферма, 64-битыне числа:
```c
typedef unsigned int __attribute__((mode(TI)))   uint128_t;
static uint64_t powm(const uint64_t b, uint64_t a, const uint64_t P)
{
    uint64_t r = b;
    uint64_t s = 1;
    while (a!=0) {
		if (a&1) 
			s = ((uint128_t)s*r)%P;
		r = ((uint128_t)r*r)%P;
		a>>=1;
    }
    return s;
}
```
## MWC128

Для поиска констант A дающих "безопасные простые числа" (safe prime) разработал свой тест, который воспроизводит все пункты из пердыдущего раздела в больших числах, см. [mwc_prime.c](mwc_prime.c).
Тест вероятностный, выделяет огромное множество констант. 

## Тест Люка — Лемера — Ризеля

Тестом на простоту Ферма является утверждение $x^{p-1} = 1$ для любого x. Тест - вероятностный.

Для чисел вида $A\cdot 2^{n} -1$ тестом простоты является LLR([Тест Люка — Лемера — Ризеля](https://en.wikipedia.org/wiki/Lucas%E2%80%93Lehmer%E2%80%93Riesel_test)), как специальный случай теста Моррисона. 
Реализация теста,см. проект [PRST by Pavel Atnashev](https://github.com/patnashev/prst)

Тесты LLR рассматривают два случая для поиска начального значения $u_0$ для реккурентной последовательности Люка $V_{2^{n-2}}(u_0)$:
1. когда $P ≡  7 (\bmod 24)$ последовательность Люка (Lucas) c начальным значением $V_k(4,1)$. 
2. когда $P ≡ 23 (\bmod 24)$ с подбором такого p: `jacobi(p-2, n)=+1` and `jacobi(p+2, n)=-1` 
для последовательности Люка $V_k(p,1)$

Последовательность $V_k(P,Q)$ задается реккурентным соотношением:
```math
V_k = P\cdot V_{k-1} - Q V_{k-2}
```
и удволетворяет ряду свойств:
```math
\begin{equation}
V_{2n} =V_n^2 - 2Q^n~; \\
V_{m+n} =V_m V_n - Q^n V_{m-n}~; \\
V_{mn}(P,Q) =V_m(V_n(P,Q), Q^n) = V_n(V_m(P,Q), Q^m)
\end{equation}
```
Операция сложения и умножения индексов посделовательности коммутативна. 


*Характеристическим многочленом* последовательностей Люка $\{V_{n}(P,Q)\}$ является $x^{2}-P\cdot x+Q$. 
Его дискриминант $D=P^{2}-4Q$ предполагается не равным нулю. 
Корни характеристического многочлена:\
$\alpha =\frac{P+{\sqrt {D}}}{2}$ и $\beta =\frac{P-{\sqrt {D}}}{2}$ \
можно использовать для получения явных формул:
$V_{n}(P,Q)=\alpha^{n}+\beta^{n}$.

*Как в модульной арифметике считать корень квадратный - не очевидно. Определение корня, как и обратного числа, можно дать через операцию возведения в степень. Проверкой является прямая операция возведения в квадрат и умножения на обратное число.* 

Подробнее см. [Алгори́тм Тоне́лли — Ше́нкса](https://en.wikipedia.org/wiki/Tonelli%E2%80%93Shanks_algorithm)

Для поиска начального значения $P$ используется [Символ Якоби](https://en.wikipedia.org/wiki/Jacobi_symbol). Реализация алгоритма [1] см. [jacobi.c](jacobi.c)

* [1] "Optimized Computation of the Jacobi Symbol" Jonas Lindstrøm & Kostas Kryptos Chalkias 
	<https://eprint.iacr.org/2024/1054>

> Символ Якоби является важным примитивом в криптографических приложениях, таких как проверка простоты, факторизация целых чисел.

* [2] "A simpler alternative to Lucas–Lehmer–Riesel primality test" Pavel Atnashev
  	<https://eprint.iacr.org/2023/195>

> В этой статье рассматривается применение теста Моррисона на простоту к числам вида $k \cdot 2^n-1$ и находится простая общая формула, эквивалентная тесту Люка — Лемера и тесту  Люка — Лемера — Ризеля на простоту.

```math
V_{2^{n-2}}(V_k(P_R, Q_R)) \equiv 0 (\bmod{N}), \quad Q_R = 1, \left(\frac{P_R-2}{N}\right)=1, \quad \left(\frac{P_R+2}{N}\right)=-1
```
Тест Люка–Лемера–Ризеля примет вид
$$V_{2^{n-2}}(V_k(V_2)) = V_{k\cdot 2^{n-1}} = V_{(N+1)/2} \equiv 0 (\bmod{N})$$
где
$$V_2 = P^2-2Q = P_R~, Q_R = Q^2$$

Тест Моррисона дается реккурентной формулой последовательности Люка, с начальными значениями $P$ и $Q=-1$.

Ниже привожу реализацию теста LLR для простых чисел вида $k\cdot 2^{16} -1$ с небольшой разрядностью
```c
int llr_test(uint32_t P, uint32_t k)
{
	const int Q = 1, n=16;	
	uint32_t N  = (k<<n)-1;
	while( !(jacobi(P-2, N)==1 && jacobi(P+2, N)==-1)) P++;
	uint32_t  v = P;
// 1.
	uint32_t  p = v;
	uint32_t zv = 2;
	for(int i=1;i<k; i++){
		uint32_t y = subm(mulm(p,v,N), zv, N);
		zv = v; v = y;
	}
// 2.
	for(int i=2; i<n; i++)
		v = subm(sqrm(v, N), 2, N);
	return (v==0);
}
```
В расчете используются: модульное умножение `mulm`, возведение в квадрат `sqrm`, и модульное вычитание `subm`. 

Алгоритм разбит на две части одна считает $V_k$, вторая $V_{2^{n-2}}$. 
Эти части можно менять местами по правилу коммутативности 
```math
V_{2^{n-2}}\left(V_k(P,Q), Q^k \right)=V_k\left(V_{2^{n-2}}(P,Q), Q^{2^{n-2}} \right).
```
Расчет $V_k$ сделан крайне не эффективно и требует $k$ циклов, что не допустимо для больших чесел. 

Для расчета больших чисел следует использовать алгоритмы с вычислительной сложностью $\log(k)$.

В модульной арифметике применется несколько вариантов алгоритмов умножения: бинарный слева-направо, справо-налево. И еще много вариантов ускорения за счет использования таблиц, в частности window-NAF и sliding-window и пр.,
подробнее см. [методы умножения в эллиптической криптографии](https://en.wikipedia.org/wiki/Elliptic_curve_point_multiplication#Windowed_method).

Для начала можно рассмотреть "простой" алгоритм умножения слева-направа.
```
   Q ← point_init(P)
   for j ← i − 1 downto 0 do
       Q ← point_double(Q, N)
       if (k[j] != 0)
           Q ← point_add(Q, P,D,N)
   return Q
```
Здесь мы вынужедены перейти к уравнениям в переменных $(U,V)$, для расчета понадобятся две функции: удвоение и сложение точек.
Инициализация
$$U_1 = 1~, V_1 = P~, D = P^2 -4Q$$

Метод удвоения:
```math
V_{2n} = V_n^2 -2 Q^n~,\quad U_{2n} = V_n U_n
```

Метод сложения точек:
```math
2V_{m+n} = V_m V_n + D U_m U_n~,\quad 2U_{m+n} = U_m V_n + V_m U_n
```

Метод сложения точек для случая $n+1$:
```math
2V_{n+1} = P V_n + D U_n~,\quad 2U_{n+1} = V_n + P U_n
```

## Вычисление квадратного корня по модулю простого числа

Корнем квадратным $a$ называется такое число $x$, для которого выполняется тождество:
$x^2 \equiv a (\bmod{N})$, где $n$ — квадратичный вычет по модулю $p$, а $p$ — нечётное простое число.

Для данного $n$ и простого числа $p>2$ (которое всегда будет нечетным), критерий Эйлера гласит, что
$n$ имеет квадратный корень (т.е. $n$ - квадратичный вычет) тогда и только тогда $n^{\frac {p-1}{2}}\equiv 1{\pmod {p}}$.
Напротив, если число $z$ не имеет квадратного корня (имеет квадратичный не-вычет), тогда: $z^{\frac {p-1}{2}}\equiv -1{\pmod {p}}$.

## MWC32

Практическое решение задачи поиска константы для 32 битных чисел дано двумя методами. Оба метода реализованы путем расчета *всей* последовательности.
1. Путем тестирования множества простых чисел в диапазоне 512-1023
2. Путем последовательного перебора чисел в диапазоне [0..1023] (дата майниг)

Критерием выбора константы $A$ для генератора является длина пероида повторения последовательности, которая дается формулой $ab^r/2 − 2$.

Другой набор тестов включает однородность и возможность сжатия (муаровый узор на "салфетках"). Для исследования однородности использовалось построение изображений по большому числу циклов. Тесты наглядно показывают отсуствие водяных знаков при использовании младших 16-20 бит. Тест при числе проходов 0x7F (8 бит, grayscale):

![тест на однородность заполнения](test.png)

Для повышения однородности следует использовать только четные вызовы функции или только нечетные вызовы функции. 
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
Ниже привожу таблицу простых чисел $P= A \cdot 2^{16} -1$ c максимальным периодом повтора.
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
Все числа из таблицы проходят [тест LLR](mwc32_llr.c).
