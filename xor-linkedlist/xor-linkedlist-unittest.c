#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "xor-linkedlist.h"
#include "utime.h"

#define OPTION_CHECK "check"
#define OPTION_RAMPING "ramping"

int cmp(const void *a, const void *b)
{
    return *(int *) a - *(int *) b;
}


void check_sorted(list *head, int *result, size_t num, const char *key)
{
    list *prev = NULL;
    list *cur = head;
    size_t i = 0;

    qsort(result, num, sizeof(int), cmp);

    for (i = 0; i < num && cur; ++i) {
        list *tmp = cur;

        if (result[i] != cur->data) {
            printf("%s failed, output is %d, result is %d at index %lu\n", key,
                   cur->data, result[i], i);
            exit(1);
        }

        cur = XOR(cur->addr, prev);
        prev = tmp;
    }
    if (num != i || cur) {
        printf("%s failed, length %lu, %lu not match\n", key, i, num);
        exit(1);
    }
}

list *create_list(const int *data, size_t num)
{
    list *ret = NULL;
    for (size_t i = 0; i < num; ++i) {
        insert_node(&ret, data[i]);
    }
    return ret;
}

void check(void)
{
    list *data;
    int r1[7] = {1, 2, 3, 4, 5, 6, 7};
    data = sort(create_list(r1, 7));

    check_sorted(data, r1, 7, "asc");
    delete_list(data);

    int r2[7] = {7, 6, 5, 4, 3, 2, 1};
    data = sort(create_list(r2, 7));

    check_sorted(data, r2, 7, "desc");
    delete_list(data);

    int r3[7] = {7, 2, 1, 3, 4, 6, 5};
    data = sort(create_list(r3, 7));

    check_sorted(data, r3, 7, "desc");
    delete_list(data);


    printf("Basic test is PASS !!!\n");
}

void randomcheck(void)
{
    for (int i = 0; i < 100; ++i) {
        size_t num = (rand() % 1000) + 1;
        int *x = (int *) calloc(num, sizeof(int));
        for (size_t j = 0; j < num; ++j)
            x[j] = (rand() % 10000) - 5000;

        list *data = sort(create_list(x, num));
        check_sorted(data, x, num, "rand");
        delete_list(data);
        free(x);
    }
    printf("Random test is PASS !!!\n");
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

#define MAX_MERGE_SIZE 11
#define MAX_ARRAY_LENGTH (65536)

typedef struct RampingResult {
    double mean[MAX_MERGE_SIZE];
    double var[MAX_MERGE_SIZE];
} RampingResult;

void ramping(void)
{
    struct timespec prev_t, cur_t;
    RampingResult result;
    memset(&result, 0, sizeof(result));

    for (int i = 0; i < 5; ++i) {
        int *x = (int *) calloc(MAX_ARRAY_LENGTH, sizeof(int));

        for (size_t j = 0; j < MAX_ARRAY_LENGTH; ++j)
            x[j] = (rand() % 10000) - 5000;

        for (int j = 1; j < MAX_MERGE_SIZE; ++j) {
            list *data = create_list(x, MAX_ARRAY_LENGTH);
            clock_gettime(CLOCK_BOOTTIME, &prev_t);
            data = __merge_sort(data, 1 << j);
            clock_gettime(CLOCK_BOOTTIME, &cur_t);
            cal_time(i + 1,
                     (double) timespec_to_ns(timespec_diff(prev_t, cur_t)),
                     &(result.mean[j]), &(result.var[j]));
            check_sorted(data, x, MAX_ARRAY_LENGTH, "rand");
            delete_list(data);
        }

        free(x);
    }
    for (int i = 1; i < MAX_MERGE_SIZE; ++i) {
        printf("%3d %lf %lf\n", 1 << i, result.mean[i], result.var[i]);
    }
}

int main(int argc, char *argv[])
{
    if (2 > argc && NULL == argv[1]) {
        printf("Usage : %s [%s]\n", argv[0], OPTION_CHECK);
        exit(1);
    }

    if (0 == strcmp(argv[1], OPTION_CHECK)) {
        check();
        randomcheck();
    } else if (0 == strcmp(argv[1], OPTION_RAMPING)) {
        ramping();
    } else {
        printf("Usage : %s [%s]\n", argv[0], OPTION_CHECK);
        exit(1);
    }
    return 0;
}
