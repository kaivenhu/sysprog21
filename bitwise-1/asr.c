#include <stdio.h>
#include <stdlib.h>

#define asr_checker(m, n, ans)                     \
    do {                                           \
        int ret = 0;                               \
        if (ans != (ret = asr_i(m, n))) {          \
            printf(                                \
                "Failed with ans: %d, ret: %d on " \
                "M: %d, N: %u, line %d\n",         \
                ans, ret, m, n, __LINE__);         \
            exit(1);                               \
        }                                          \
    } while (0)

int asr_i(signed int m, unsigned int n)
{
    const int logical = (((int) -1) >> 1) > 0;
    unsigned int fixu = -(logical & (m < 0));
    int fix = *(int *) &fixu;
    return (m >> n) | (fix ^ (fix >> n));
}

int main(void)
{
    asr_checker(-5, 1, -3);
    asr_checker(5, 2, 5 >> 2);
    asr_checker(0xFFFFF, 0xF, 0xFFFFF >> 0xF);
    asr_checker(-20, 3, -3);
    printf("PASS !!!\n");
    return 0;
}
