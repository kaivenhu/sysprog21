#include <stdio.h>
#include <string.h>

#include "xs.h"

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

    xs_free(&string);
    xs_free(&prefix);
    xs_free(&suffix);
    xs_free(&space);
    xs_free(&empty);
}


int main()
{
    test();

    printf("PASS !!!\n");
    return 0;
}
