# Хэш функция на базе генератора случайных чисел

 * _Анатолий М. Георгиевский_ , ИТМО, 2026 (https://github.com/AnatolyGeorgievski)
 * _ĀĿΞX_ (https://github.com/Alex20129), -- обсуждение принципов построения mwc-хэш и тестирование 

> Представляем метод синтеза некриптографической хэш-функции на алгоритмах MWC128.

Известно множество некриптографических функций, которые могут быть использованы для индексации баз данных и проверки контрольных сумм массивов данных. К таким функция предъявляются требования - низкая вероятность коллизий. Традиционно эта ниша занята алгоритмами CRC64. Линейное преобразование LCG или CRC не избавляет от коллизий. В сетевых протоколах и архиваторах используются различные функции среди, которых выделяется xxHash64. Интересует быстродействие функции на потоке данных, возможность считать параллельно с разбивкой данных на сегменты и векторизовать вычисления.

Современные хэш-функции строятся по принципу SPONGE-SQUEEZE: состоят из цикла впитывания данных и отжимания "губки". 

1. Линейный генератор с возможностью выполнять skip и jump на потоке данных.
2. Функция пермутации-миксер выходных данных.

Тестирование хэш-функций выполняется в пакете [SMHasher](https://github.com/aappleby/smhasher). Тесты ориентированы на выявление слабых бит в хэше и поиск коллизий.

В данной работе мы ссылаемся на опыт разработки LXM генератора [4], где авторы использовали множество типовых миксеров для придания функции генератора ГПСЧ необходимого качества, для прохождения тестов без коллизий.

В ходе разработки алгоритма мы повторили результаты [4] с использованием генератора `Xoroshiro128p` И миксера выходных значений предложенного авторами в работе. Кроме того протестировали ряд других миксеров. Исходный код подготовлен для теста SMHasher см. [xoroshiro-hash](test/xoshiro_hash.c). Для сравнения реализовали собственную версию функции xxHash64. 


**Алгоритм MWC128-hash**

```c
uint64_t mwc128_hash(const uint8_t *data, uint64_t len, uint64_t seed0) {
	uint128_t h = seed0+(uint128_t)IV;// + ((uint128_t)len<<64);
	h = (uint64_t)h*A1 + (h>>64);
	for (int i=0; i<len>>3; i++){
		h+=*(uint64_t*)data; data+=8;
		h = (uint64_t )h*A1 + (h>>64); // итерация MWC128
	}
	if (len&7) {
		int s = len&7;
		uint64_t d = PAD;
		memcpy(&d, data, s); data+=s;
		h+= (d);
		h = (uint64_t )h*(A1<<(64-(s*8))) + (h>>(s*8));
	}
	return mixer(h^(h>>64));
}
```
Параметры алгоритма: 
* `A1` - параметр генератора MWC128, где $P=A_1\cdot 2^{64} -1$ простое число.
* `IV` - вектор инициализации хэш
* `PAD` - дополнение данных 

В составе алгоритма использован миксер Lea's. Это не самый сильный миксер, но самый простой и оптимально подошел к нашей задаче. 
```c
// Doug Lea's mixing function
static inline uint64_t mixer(uint64_t h) {
  h *= 0xdaba0b6eb09322e3ull;
  h ^= h >> 32;
  h *= 0xdaba0b6eb09322e3ull;
  h ^= h >> 32;
  return h;
}
```

**Mixer parameters**

Отправной точкой исследования является вариант финального миксера из функции `MurmurHash3`. Кроме того в составе разных хэш-функций мы нашли много вариантов миксера

1. $x ←  x \oplus (x \gg a)$
2. $x ←  x \cdot Prime_1$
3. $x ←  x \oplus (x \gg b)$
4. $x ←  x \cdot Prime_2$
5. $x ←  x \oplus (x \gg c)$

Подобный миксер `Lea` использован в работе [4], `SplitMix64`  предлагается авторами в качестве рандомизатора SEED при инициализации `Xoroshiro128` и генераторов с большой разрядностью состояния. `Avalanche` миксер заимствован из функции `xxHash64`.

MurmurHash3 
| Mixer	      | a | Prime1 | b | Prime2 | c |
|-------------|---|--------|---|--------|---|
| MurmurHash3 |33 |0xff51afd7ed558ccd |33 |0xc4ceb9fe1a85ec53	|33 |
| Lea's       |32 |0xdaba0b6eb09322e3 |32 |0xdaba0b6eb09322e3	|32 |
| SplitMix64  |30 |0xbf58476d1ce4e5b9 |27 |0x94d049bb133111eb   |31 |
| Avalanche   |33 |0xC2B2AE3D27D4EB4F |29 |0x165667B19E3779F9   |32 |

[David Stafford's mixing function](http://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html) опубликованы в блоге.

Теблица David Stafford's Mixer 
| Mixer	| a | Prime1 | b | Prime2 | c |
|-------|---|--------|---|--------|---|
|Mix01	|31	|0x7fb5d329728ea185	|27	|0x81dadef4bc2dd44d	|33
|Mix02	|33	|0x64dd81482cbd31d7	|31	|0xe36aa5c613612997	|31
|Mix03	|31	|0x99bcf6822b23ca35	|30	|0x14020a57acced8b7	|33
|Mix04	|33	|0x62a9d9ed799705f5	|28	|0xcb24d0a5c88c35b3	|32
|Mix05	|31	|0x79c135c1674b9add	|29	|0x54c77c86f6913e45	|30
|Mix06	|31	|0x69b0bc90bd9a8c49	|27	|0x3d5e661a2a77868d	|30
|Mix07	|30	|0x16a6ac37883af045	|26	|0xcc9c31a4274686a5	|32
|Mix08	|30	|0x294aa62849912f0b	|28	|0x0a9ba9c8a5b15117	|31
|Mix09	|32	|0x4cd6944c5cc20b6d	|29	|0xfc12c5b19d3259e9	|32
|Mix10	|30	|0xe4c7e495f4c683f5	|32	|0xfda871baea35a293	|33
|Mix11	|27	|0x97d461a8b11570d9	|28	|0x02271eb7c6c4cd6b	|32
|Mix12	|29	|0x3cd0eb9d47532dfb	|26	|0x63660277528772bb	|33
|Mix13	|30	|0xbf58476d1ce4e5b9	|27	|0x94d049bb133111eb	|31
|Mix14	|30	|0x4be98134a5976fd3	|29	|0x3bc0993a5ad19a13	|31

## Результаты работы

* Алгоритм некриптографического хэша [MWC128-hash](test/mwc128_hash.c) с размером выходных данных 128 и 64 бита. 
* Алгоритм некриптографического хэша [Xoroshiro128-hash](test/xoshiro_hash.c) с длиной хэша 64 бита.

Параметры генератора и миксера требуют оптимизации, константы не рассчитывались, а заимствованы из других работ. Дальнейшее развитие темы: оптимизация, генератор RNS-MWC, использующий вектор из генераторов MWC, и векторный алгоритм параллельной генерации хэш.

[1] David Blackman and Sebastiano Vigna. 2018. Scrambled Linear Pseudorandom Number Generators. \
3 May 2018, 41 pages. [arxiv:1805.01407]() To appear in ACM Transactions on Mathematical Software.

[2] Austin Appleby. 2016. SMHasher. 8 Jan. 2016, https://github.com/aappleby/smhasher

[3] Richard P. Brent. 2004. Note on Marsaglia’s Xorshift Random Number Generators.\
Journal of Statistical Software, 11, 5, Aug., 1–5. coden:JSSOBK (https://doi.org/10.18637/jss.v011.i05)

[4] Guy L. Steele Jr. and Sebastiano Vigna. 2021. LXM: better splittable pseudorandom number generators (and almost as fast). 
Proc. ACM Program. Lang. 5, OOPSLA, Article 148 (October 2021), 31 pages. (https://doi.org/10.1145/3485525)



**Сборка теста**

```sh
$ git clone https://github.com/rurban/smhasher.git
$ cd smhasher
$ git submodule update --init --recursive
$ cmake -B build
$ cmake --build build -j 16 
; Если всё хорошо собралось, можно прогнать тесты для встроенной функции, их там много:
$ ./build/SMHasher prvhash64_64
```