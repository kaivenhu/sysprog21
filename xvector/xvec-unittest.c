#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xvec.h"
#include "utime.h"

#define OPTION_CHECK "check"
#define OPTION_BENCHMARK "benchmark"

void check(void)
{
    v(float, 3, vec1);
    v(int, 2, vec2, 13, 42);
    v(double, 2, vec3, 99.99);

    printf("pos(vec2,0)=%d, pos(vec2,1)=%d\n", vec_pos(vec2, 0),
           vec_pos(vec2, 1));
    vec_push_back(vec2, 88);
    vec_reserve(vec2, 100);
    printf("capacity(vec1)=%zu, capacity(vec2)=%zu\n", vec_capacity(vec1),
           vec_capacity(vec2));
    printf("pos(vec2,2)=%d\n", vec_pos(vec2, 2));

#define display(v)                               \
    do {                                         \
        for (size_t i = 0; i < vec_size(v); i++) \
            printf("%.2f ", vec_pos(v, i));      \
        puts(v.on_heap ? "heap" : "stack");      \
    } while (0)

    display(vec1);
    vec_push_back(vec1, 0.0);
    display(vec1);
    vec_push_back(vec1, 1.1);
    display(vec1);
    vec_push_back(vec1, 2.2);
    display(vec1);
    vec_push_back(vec1, 3.3);
    display(vec1);
    vec_push_back(vec1, 4.4);
    display(vec1);
    vec_push_back(vec1, 5.5);
    display(vec1);
    vec_pop_back(vec1);
    display(vec1);
    vec_pop_back(vec1);
    display(vec1);
    vec_pop_back(vec1);
    display(vec1);
    vec_pop_back(vec1);
    display(vec1);
    vec_pop_back(vec1);
    display(vec1);
    vec_pop_back(vec1);
    display(vec1);

    for (int i = 0; i <= 4; ++i) {
        vec_insert(vec3, i, i);
        display(vec3);
    }
    for (int i = 0; i <= 4; ++i) {
        vec_insert(vec3, 1, i);
        display(vec3);
    }
    for (int i = 0; i <= 8; ++i) {
        vec_erase(vec3, 1);
        display(vec3);
    }
#undef display
}

static inline void cal_time(const unsigned int run,
                            const double t,
                            double *mean,
                            double *var)
{
    double delta = t - *mean;
    *mean += (delta / (double) run);
    *var += delta * (t - *mean);
}

#define INSERT_SIZE (1024*64)
#define INSERT_BASE 1024
#define INSERT_STEP 32

typedef struct BenchmarkResult {
    double mean[INSERT_SIZE/INSERT_STEP];
    double var[INSERT_SIZE/INSERT_STEP];
} BenchmarkResult;

void benchmark(void)
{
    struct timespec prev_t, cur_t;
    BenchmarkResult result;
    memset(&result, 0, sizeof(result));

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < INSERT_SIZE; j += INSERT_STEP) {
            v(int, 1, vec);
            clock_gettime(CLOCK_BOOTTIME, &prev_t);
            for (int k = 0; k < (j + INSERT_BASE); ++k) {
                vec_push_back(vec, k);
            }
            clock_gettime(CLOCK_BOOTTIME, &cur_t);
            cal_time(i + 1,
                     (double) timespec_to_ns(timespec_diff(prev_t, cur_t)),
                     &(result.mean[j/INSERT_STEP]), &(result.var[j/INSERT_STEP]));
        }
    }
    for (int i = 0; i < INSERT_SIZE; i+=INSERT_STEP) {
        printf("%3d %lf %lf\n", i + INSERT_BASE, result.mean[i/INSERT_STEP], result.var[i/INSERT_STEP]);
    }
}

void help(const char *name)
{
    printf("Usage : %s [%s|%s]\n", name, OPTION_CHECK, OPTION_BENCHMARK);
    exit(1);
}

int main(int argc, char *argv[])
{
    if (2 > argc || NULL == argv[1]) {
        help(argv[0]);
    }

    if (0 == strcmp(argv[1], OPTION_CHECK)) {
        check();
    } else if (0 == strcmp(argv[1], OPTION_BENCHMARK)) {
        benchmark();
    } else {
        help(argv[0]);
    }
    return 0;
}
