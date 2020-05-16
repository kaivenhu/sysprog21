#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "xs.h"
#include "utime.h"

#define check(x, y)                                                   \
    do {                                                              \
        if (0 != strcmp(x, y)) {                                      \
            printf("Failed %s with %s on line %d\n", x, y, __LINE__); \
            exit(1);                                                  \
        }                                                             \
    } while (0)


void test(void)
{
    xs string = *xs_tmp("\n foobarbar");
    xs space = *xs_tmp("\n ");
    xs empty = *xs_tmp("");
    xs prefix = *xs_tmp("((("), suffix = *xs_tmp(")))");

    check(xs_data(&string), "\n foobarbar");
    xs_trim(&string, "\n ");
    check(xs_data(&string), "foobarbar");
    xs_concat(&string, &empty, &space);
    xs_concat(&string, &empty, &space);
    xs_concat(&string, &empty, &space);
    check(xs_data(&string), "foobarbar\n \n \n ");
    xs_trim(&string, "\n ");
    check(xs_data(&string), "foobarbar");
    xs_concat(&string, &prefix, &suffix);
    xs_concat(&string, &empty, &space);
    xs_concat(&string, &empty, &space);
    xs_concat(&string, &empty, &space);
    check(xs_data(&string), "(((foobarbar)))\n \n \n ");

    xs_trim(&string, "\n ");
    check(xs_data(&string), "(((foobarbar)))");
    xs_concat(&string, &prefix, &suffix);
    check(xs_data(&string), "((((((foobarbar))))))");
    xs_trim(&string, "()");
    check(xs_data(&string), "foobarbar");
    for (int i = 0; i < 3; ++i) {
        xs_concat(&string, &prefix, &suffix);
    }
    check(xs_data(&string), "(((((((((foobarbar)))))))))");
    xs_concat(&string, &prefix, &suffix);
    check(xs_data(&string), "((((((((((((foobarbar))))))))))))");
    xs_grow(&string, xs_capacity(&string) + 1);
    check(xs_data(&string), "((((((((((((foobarbar))))))))))))");
    xs_trim(&string, "()");
    check(xs_data(&string), "foobarbar");
    xs *copy = xs_copy(&string, &xs_literal_empty());
    check(xs_data(copy), "foobarbar");

    xs_free(copy);
    check(xs_data(&string), "foobarbar");

    xs_free(&string);
    xs_free(&prefix);
    xs_free(&suffix);
    xs_free(&space);
    xs_free(&empty);
}

void copy_on_write(void)
{
    xs empty = *xs_tmp("");
    xs *x = xs_new(&xs_literal_empty(), "0123456789abcdef");
    xs *y = xs_copy(x, &xs_literal_empty());
    xs *z = xs_copy(x, &xs_literal_empty());
    xs *w = xs_copy(x, &xs_literal_empty());
    check(xs_data(x), "0123456789abcdef");
    check(xs_data(y), "0123456789abcdef");
    check(xs_data(z), "0123456789abcdef");
    check(xs_data(w), "0123456789abcdef");

    xs_concat(y, &empty, x);
    xs_concat(z, x, x);
    xs_trim(w, "0123");
    check(xs_data(x), "0123456789abcdef");
    check(xs_data(y), "0123456789abcdef" "0123456789abcdef");
    check(xs_data(z), "0123456789abcdef" "0123456789abcdef" "0123456789abcdef");
    check(xs_data(w), "456789abcdef");

    w = xs_free(w);
    z = xs_free(z);
    w = xs_copy(y, &xs_literal_empty());
    z = xs_copy(y, &xs_literal_empty());
    check(xs_data(y), "0123456789abcdef" "0123456789abcdef");
    check(xs_data(w), "0123456789abcdef" "0123456789abcdef");
    check(xs_data(z), "0123456789abcdef" "0123456789abcdef");

    xs_concat(x, &empty, z);
    check(xs_data(x), "0123456789abcdef" "0123456789abcdef" "0123456789abcdef");
    xs_trim(x, "0");
    check(xs_data(x), "123456789abcdef" "0123456789abcdef" "0123456789abcdef");

    xs_free(x);
    xs_free(y);
    xs_free(z);
    xs_free(w);
    return;
}

void token(void)
{
    const char *tok = NULL;
    char *saveptr = NULL;
    xs *x = xs_new(&xs_literal_empty(), "   ; aaa ; ; bbb; ccc");
    xs *y = xs_copy(x, &xs_literal_empty());

    check(xs_data(y), "   ; aaa ; ; bbb; ccc");

    tok = xs_tok_r(x, " ;", &saveptr);
    check(tok, "aaa");
    tok = xs_tok_r(NULL, " ;", &saveptr);
    check(tok, "bbb");
    tok = xs_tok_r(NULL, " ;", &saveptr);
    check(tok, "ccc");
    tok = xs_tok_r(NULL, " ;", &saveptr);
    check(xs_data(x), "   ; aaa");
    check(xs_data(y), "   ; aaa ; ; bbb; ccc");
    if (tok) {
        printf("invalid token %d\n", __LINE__);
    }
    x = xs_free(x);
    y = xs_free(y);
    x = xs_new(&xs_literal_empty(), "   ; ddd;");
    tok = xs_tok_r(x, " ;", &saveptr);
    check(tok, "ddd");
    tok = xs_tok_r(NULL, " ;", &saveptr);
    check(xs_data(x), "   ; ddd");
    if (tok) {
        printf("invalid token %d\n", __LINE__);
    }
    y = xs_new(&xs_literal_empty(), "   ; ;");
    tok = xs_tok_r(y, " ;", &saveptr);
    if (tok) {
        printf("invalid token %d\n", __LINE__);
    }
    check(xs_data(y), "   ; ;");
    xs_free(x);
    xs_free(y);
}

void checker(void)
{
    test();
    copy_on_write();
    token();
    printf("PASS !!!\n");
}

#define XS_ARRAY_SIZE 5000
#define MAX_STRING_ORDER 12
#define MAX_STRING_SIZE (1 << MAX_STRING_ORDER)

static void cow_benchmark(int run, int size)
{
    assert(0 < run && 0 < size);
    xs *x = NULL;
    char *str = malloc(sizeof(char) * size);
    xs **arr_ptr = (xs **) calloc(XS_ARRAY_SIZE, sizeof(xs *));

    for (int i = 0; i < size; ++i)
        str[i] = 'a' + (char) (i % 26);

    str[size - 1] = '\0';
    x = xs_new(&xs_literal_empty(), str);

    for (int i = 0; i < XS_ARRAY_SIZE; ++i) {
        arr_ptr[i] = (xs *) calloc(1, sizeof(xs));
        *(arr_ptr[i]) = xs_literal_empty();
    }

    for (int i = 0 ; i < XS_ARRAY_SIZE; ++i)
        arr_ptr[i] = xs_copy(x, arr_ptr[i]);

    for (int j = 0; j < run; ++j)
        for (int i = 0 ; i < XS_ARRAY_SIZE; ++i)
            check(xs_data(arr_ptr[i]), str);

    for (int i = 0; i < XS_ARRAY_SIZE; ++i) {
        xs_free(arr_ptr[i]);
        free(arr_ptr[i]);
    }
    xs_free(x);
    free(str);
    free(arr_ptr);
}

void locality(void)
{
    cow_benchmark(1000, MAX_STRING_SIZE);
}

typedef struct BenchmarkResult {
    double mean[MAX_STRING_ORDER];
    double var[MAX_STRING_ORDER];
} BenchmarkResult;

static inline void cal_time(const unsigned int run,
                            const double t,
                            double *mean,
                            double *var)
{
    double delta = t - *mean;
    *mean += (delta / (double) run);
    *var += delta * (t - *mean);
}

void benchmark(void)
{
    struct timespec prev_t, cur_t;
    BenchmarkResult result;
    memset(&result, 0, sizeof(result));

    for (int i = 0; i < 1000; ++i) {
        for (int j = 0; j < MAX_STRING_ORDER; ++j) {
            clock_gettime(CLOCK_BOOTTIME, &prev_t);
            cow_benchmark(1, (1 << (j + 1)));
            clock_gettime(CLOCK_BOOTTIME, &cur_t);
            cal_time(i + 1,
                    (double) timespec_to_ns(timespec_diff(prev_t, cur_t)),
                    &(result.mean[j]), &(result.var[j]));
        }
    }
    for (int i = 0; i < MAX_STRING_ORDER; ++i) {
        printf("%d %lf %lf\n", (1 << (i + 1)), result.mean[i],
                result.var[i]);
    }
}

int main(int argc, char *argv[])
{
    if (2 > argc && NULL == argv[1]) {
        printf("Usage: %s <option>\n", argv[0]);
        exit(1);
    }
    if (0 == strcmp(argv[1], "check")) {
        checker();
    } else if (0 == strcmp(argv[1], "locality")) {
        locality();
    } else if (0 == strcmp(argv[1], "benchmark")) {
        benchmark();
    }

    return 0;
}
