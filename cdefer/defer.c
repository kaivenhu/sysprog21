#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defer.h"

void test1(void)
{
    DEFER_INIT;
    char *x = malloc(32);
    Defer(free(x));

    Defer({
        printf("x   : %s\n", x);
        x[5] = '\0';
        ;
        printf("now : %s\n", x);
    });

    strcpy(x, "Hello world");
    Return;
}

int test2(void)
{
    DEFER_INIT;
    puts("0");

    Defer(puts("3"));
    Defer(puts("2"));
    puts("1");
    Return puts("4");
}

int main(void)
{
    test1();
    test2();
    return 0;
}
