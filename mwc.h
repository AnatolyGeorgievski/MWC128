#ifndef _MWC_H
#define _MWC_H
#include <stdint.h>
#include <stdbool.h>

typedef struct _PRNG PRNG_t;
// Объектная модель Reversible-PRNG, Hash-PRNG, Ctr-DRNG
/* Объектная модель генератора/хэш функции */
struct _PRNG {
// генерация однородного распределения. если функция не задана используется стандартный метод преобразования
    double   (*u01 )(uint64_t *state);
// генерация следующего состояния, с уменьшением числа раундов
    uint64_t (*next)(uint64_t *state, int rounds);
// генерация предыдущего состояния через определенное число внутренних раундов
    uint64_t (*prev)(uint64_t *state, int rounds);
// функция XOF генерации
    uint64_t (*hash)(const uint8_t *in, size_t len, uint64_t seed, uint8_t* out, size_t bytes);
    uint64_t (*jump)(uint64_t *state, int distance);
    uint64_t (*merge)(uint64_t *state1, uint64_t *state2, int distance);
    const char *name;
    const char *description;
    uint64_t check_value; // верификация
    uint8_t  bits;     // разрядность на выходе 1..32/64/128/256
    uint8_t  flags;
    uint16_t n_rounds; // максимальное число раундов/число циклов внутренней функции смешивания
    uint16_t state_sz; // размер состояния в байтах, кратно uint64_t
};
// динамическая регистрация
int register_prng(struct _PRNG* info);
// статическая регистрация модулей, через регистрацию статических конструкторов
 #define REGISTER_PRNG(N, ...)            \
static struct _PRNG prng_info_##N = {     \
    __VA_ARGS__                           \
};  \
static void __attribute__((constructor)) _init_##N(){\
    register_hash(&prng_info_##N);                   \
}

extern struct {
    bool verbose;
    bool quiet;
} prng_options;

// набор тестов
PRNG_t * prng_new(int id, int bits, uint64_t seed);
/*! Тест LeadingZeros / TrailingZeros */
double clz_test(PRNG_t* gen);
/*! Strict Avalanche Criterion (SAC) — Строгий критерий лавины
    Определение (по Webster & Tavares, 1985):
    Если перевернуть любой один бит на входе хеш-функции, то каждый бит на выходе должен измениться с вероятностью ровно 50% (0.5).
 */
double sac_test(PRNG_t* gen);
/*! Bit Independence Criterion (BIC) — Критерий независимости битов
    Определение:
    Когда переворачивается один входной бит i, то любые два бита на выходе j и k должны изменяться независимо друг от друга.
 */
//double bic_test(PRNG_t* gen);
/*! Тест "Gaps", "Runs"
    -- Классический тест на распределение интервалов между определёнными событиями в последовательности. 
    Тест "козырные тузы" Trump Aces: 
    -- «Trump aces» переводится на русский язык как «козырные тузы» (в контексте карточных игр). 
    Фраза состоит из слов trump (козырь) и aces (тузы), обозначая самые сильные карты в колоде. 
    Тест проверяет случайный или не случайный порядок появления специальных паттернов на выходе генератора. 
    Тест считается провальным, если в колоде 2^N фиксированное число "тузов" и интервалы следования тузов не является случайными.

    -- Дональд Трамп в интервью постоянно упоминает слово "Aced" и карты в игре политика, когда обсуждает результаты переговоров
    "Trump Aces" можно довольно естественно привязать к реальным высказываниям Дональда Трампа, хотя и не дословно.
Основная связь
    Трамп многократно употребляет слово "aced" — это именно от "ace" (туз в картах, высший балл, козырь), 
    и в английском оно часто несёт оттенок "выиграл по-крупному", "сдал идеально", "козырь в рукаве".
    -- "Trump Says He ‘Aced’ a Cognitive Test."
 * Gaps — общий тест на распределение интервалов между событиями.
 * Runs — тест на серии/пробеги (длина цепочек одинаковых значений).
 * Trump Aces — специализированный gaps-тест именно на сверхредкие экстремальные паттерны ("тузы"), 
    выявляет скрытые структуры в хвостах распределения в сериях по 2^{32}.
 */
double runs_test(PRNG_t* gen);
double gaps_test(PRNG_t* gen);
double aces_test(PRNG_t* gen);
/*! Тест сложности _difficulty/hashrate_ 

    Возвращает p-value
    Параметры теста: 
        options - расчет статистики KS, AD, Chi2; расчет диаграммы CLZ и CTZ
        Nr   - число итераций теста, Nr = (1<<n)
        count- общее число итераций
        eps  - граница теста eps < p-value < 1-eps
        bins - группирование результатов по битовой сложности
        hist - накопление результатов теста (диаграмма распределения _hashrate_)
        difficulty - суммарная сложность
 */
double diff_test(const char* name, uint64_t (*next)(void*), uint64_t* state, uint64_t *sum, int Nr);
/*! Тест производительности в цикле 
 */
double speed_test(PRNG_t* gen);
/*! *Carry propagation/bit probability diffusion* через mixer-функцию. 
    Считая что на входе все биты распределены независимо с некоторой вероятностью в каждом бите, 
    метод дает распределение вероятностей на выходе.

    Для CTR-hash функций (hash в CTR-режиме), где на вход подаётся счётчик, gamma - инкремент, обычное целое число:
    ctr += 1 или специальная Weyl-константа. 
    Тест применяется в форме hash(ctr || nonce ) или hash(ctr + IV) или hash(ctr ^ key).
    вероятности переносов (carry bits) при сложении играют ключевую роль в диффузии.
 */

/*! *Adaptive Trail Search*
    Адаптивный best-first trail search с priority queue, difficulty metrics, nibble permutations и adaptive heuristics. 
    Подходит для расширения фреймворков тестирования хеш-функций и PRNG миксеров. Он дополняет существующие тесты 
    (SAC, BIC, Avalanche, Gaps, Runs, Trump Aces, carry propagation), фокусируясь на дифференциальном криптоанализе: 
    поиск "хороших" дифференциальных трасс (trails) с высокой вероятностью (Pr(dY|dX) >> 2^{-n}), 
    где n — разрядность исследуемого окна.
 */
/*! threading model -- модель управления тестами, подразумевает запуск joinable функций со своими смещениями, 
    полученными методом jump(2^n), с последующим объединением результатов теста.  
    thread pool, формирует очередь тестирования с ограничением числа одновременно запущенных тестов. 
    Тест возвращает результат `float` AS int, `user_data` - содержит параметры теста, включая функцию генератор и ее состояние.
 */
// #include <threads.h>
/*
thrd_pool_new — создаёт пул с N потоками
thrd_pool_enqueue — добавляет задачу, возвращает её id (порядковый номер)
thrd_pool_wait — ждёт, пока все текущие и поставленные задачи завершат по списку идентификаторов
thrd_pool_free — корректно завершает все потоки и освобождает ресурсы
*/
typedef struct _thrd_pool thrd_pool_t;
thrd_pool_t *thrd_pool_new(int N_threads);
int   thrd_pool_enqueue(thrd_pool_t *pool, int (*)(void* args), void* args);
int   thrd_pool_wait   (thrd_pool_t *pool, int* ids);
void  thrd_pool_free   (thrd_pool_t *pool);
/*! Графические методы, сохранение в форме таблицы или 2D- изображения. */


typedef uint64_t (*cb_next)(void*);
static inline double difficulty(uint64_t x) {
	return 1/((double)x+0.5);
}
double bic_test(uint64_t (*next)(void*), uint64_t* state, uint64_t *sum, int expN);
double dif_test(uint64_t (*next)(void*), uint64_t* state, uint64_t *sum, int Nr);

#endif //_MWC_H