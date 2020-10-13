#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define mod_checker(ans, x)                                                    \
    do {                                                                       \
        uint32_t ret = 0;                                                      \
        if (ans != (ret = quickmod(x))) {                                      \
            printf("Failed with x: %u, ret: %u, ans: %u on line %d\n", x, ret, \
                   ans, __LINE__);                                             \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

#define div_checker(ans, x)                                                    \
    do {                                                                       \
        bool ret = 0;                                                          \
        if (ans != (ret = divisible(x))) {                                     \
            printf("Failed with x: %u, ret: %u, ans: %u on line %d\n", x, ret, \
                   ans, __LINE__);                                             \
            exit(1);                                                           \
        }                                                                      \
    } while (0)


const uint32_t D = 3;
#define M ((uint64_t)(UINT64_C(0xFFFFFFFFFFFFFFFF) / (D) + 1))

/* compute (n mod d) given precomputed M */
uint32_t quickmod(uint32_t n)
{
    uint64_t quotient = ((__uint128_t) M * n) >> 64;
    return n - quotient * D;
}

void quickmod_tester(void)
{
    for (uint32_t i = 0; i < 0xFFFFFFFF; ++i) {
        mod_checker(i % D, i);
    }
    mod_checker(0xFFFFFFFF % D, 0xFFFFFFFF);
    printf("Test quickmode is PASS !!!\n");
}

bool divisible(uint32_t n)
{
    return (n * M) <= (M - 1);
}

void divisible_tester(void)
{
    for (uint32_t i = 0; i < 0xFFFFFFFF; ++i) {
        div_checker(!(i % D), i);
    }
    div_checker(!(0xFFFFFFFF % D), 0xFFFFFFFF);
    printf("Test divisible is PASS !!!\n");
}

int main(void)
{
    quickmod_tester();
    divisible_tester();
    return 0;
}
