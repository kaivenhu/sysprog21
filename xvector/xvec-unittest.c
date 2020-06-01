#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xvec.h"

#define OPTION_CHECK "check"

void check(void)
{
    v(float, 3, vec1);
    v(int, 2, vec2, 13, 42);

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

#undef display
}


int main(int argc, char *argv[])
{
    if (2 > argc || NULL == argv[1]) {
        printf("Usage : %s [%s]\n", argv[0], OPTION_CHECK);
        exit(1);
    }

    if (0 == strcmp(argv[1], OPTION_CHECK)) {
        check();
    } else {
        printf("Usage : %s [%s]\n", argv[0], argv[1]);
        exit(1);
    }
    return 0;
}
