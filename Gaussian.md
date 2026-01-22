# Однородное и нормальное распределение случайной величины

* __Анатолий М. Георгиевский, ИТМО__

## Однородное распределение
Преобразование чисел в формат Float32 дает однородное распределение $[0,1)$ вероятности

```c
static inline float u64_float(uint64_t x) {
	return ((x&0xFFFFFFFFU) >> 8) * 0x1.0p-24;
}
float uniform (uint64_t *state) {
	return u64_float(mwc64x(state));
}
```
Аналогично получается преобразование в формат `double`, и системные форматы включая: `_Float32x`, `_Float32` и `_Float16`, с учетом разрядности мантиссы. Из личного опыта, рекомендуется использовать только младшую часть числа 32-бита, чтобы избежать водяных знаков, так как старшая и младшая часть числа связаны константой преобразования. При этом для улучшения однородности распределения можно использовать только нечетные или только нечетные вызовы функции. Для получения однородного распределения чисел в формате `double`, `_Float64`, `_Float64x` рекомендуется использовать генератор MWC128.

## Преобразование Box-Muller

> The standard Box–Muller transform generates values from the standard normal distribution (i.e. standard normal deviates) with mean 0 and standard deviation 1.

Пусть $r$ и $\varphi$ — независимые случайные величины, равномерно распределённые на интервале $(0, 1]$. Вычислим $z_{0}$ и $z_{1}$ по формулам

$$z_{0}=\cos(2\pi \varphi )\sqrt {-2\ln r}~,$$

$$z_{1}=\sin(2\pi \varphi )\sqrt {-2\ln r}~.$$

Тогда $z_{0}$ и $z_{1}$ будут независимы и распределены нормально с математическим ожиданием 0 и дисперсией 1.

```c
float gaussian(uint64_t *state)
{
    float u1 = uniform(state); // однородное распределение [0,1)
	float u2 = uniform(state); // однородное распределение [0,1)
    return sqrt(-2.0f*log(1.0f-u1))*cos(2*M_PI*u2);
}
```
В данном алгоритме применил трюк (1-U), что дает распределение $(0,1]$.

## Метод обратного преобразования

Общая идея для получения заданной функции распределения - применить обратную функцию для отображения интервала выходных значений. 
Так, например, можно синтезировать функцию с экспоненциальным распределением вероятности.
```c
float exponent(uint64_t *state) {
	float u1 = uniform(state);
	return -log(1.0f-u1);
}
```
![An animation of how inverse transform sampling generates normally distributed random values from uniformly distributed random values](https://upload.wikimedia.org/wikipedia/commons/c/cc/Inverse_Transform_Sampling_Example.gif) 

By <a href="//commons.wikimedia.org/w/index.php?title=User:Davidjessop&amp;action=edit&amp;redlink=1" class="new" title="User:Davidjessop (page does not exist)">Davidjessop</a> - <span class="int-own-work" lang="en">Own work</span>, <a href="https://creativecommons.org/licenses/by-sa/4.0" title="Creative Commons Attribution-Share Alike 4.0">CC BY-SA 4.0</a>, <a href="https://commons.wikimedia.org/w/index.php?curid=100369573">Link</a>

> An animation of how inverse transform sampling generates normally distributed random values from uniformly distributed random values

## Распределение Максвелла

Распределение Максвелла получается из идеи квадратичного суммирования функций в евклидовом пространстве. Данный метод закрывает потребности синтеза начальных условий в классической динамике и молекулярной динамике. Так начальное условие будет соответствовать нормальному распределению проекций скорости молекул и подчиняться распределению максвелла по скоростям.
```c
float maxwell(uint64_t *state)
{
	float x = gaussian(state);
	float y = gaussian(state);
	float z = gaussian(state);
	return sqrtf(x*x + y*y + z*z);
}
```
Распределение Максвелла записывается для модуля скорости частицы и имеет плотность:
```math
f_{v}(x)=Bx^{2}\exp \left[-\beta x^{2}\right]\,\,(x\geq 0)
```
и $f_{v}(x)=0\,\,(x<0)$, 
где $x$ — формальная переменная, фактор $\beta >0$ определяется типом частиц и температурой, а множитель $B$ подбирается в зависимости от 
$\beta$ для обеспечения нормировки. Именно это выражение считается максвелловским распределением в математике.
