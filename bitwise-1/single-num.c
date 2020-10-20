#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define checker(ans, nums, size)                                         \
    do {                                                                 \
        int ret = 0;                                                     \
        if (ans != (ret = singleNumber(nums, size))) {                   \
            printf("Failed !!! ret: %d, ans: %d on line %d\n", ret, ans, \
                   __LINE__);                                            \
            exit(1);                                                     \
        }                                                                \
    } while (0)

#define generalChecker(k, ans, nums, size)                               \
    do {                                                                 \
        int ret = 0;                                                     \
        if (ans != (ret = generalSingleNumber(k, nums, size))) {         \
            printf("Failed !!! ret: %d, ans: %d on line %d\n", ret, ans, \
                   __LINE__);                                            \
            exit(1);                                                     \
        }                                                                \
    } while (0)

int generalSingleNumber(const int k, int *nums, int numsSize)
{
    assert(k > 0);
    int msb = (sizeof(int) << 3) - __builtin_clz(k);
    int *x = (int *) calloc(msb, sizeof(int));
    int *op = (int *) calloc(msb, sizeof(int));
    for (int i = msb - 1; i >= 0; --i) {
        op[i] = 0xFFFFFFFF * !((1 << i) & k);
    }
    for (int n = 0; n < numsSize; ++n) {
        for (int i = msb - 1; i >= 0; --i) {
            int carry = nums[n];
            for (int j = i - 1; j >= 0; --j) {
                carry &= x[j];
            }
            x[i] ^= carry;
        }

        int mask = 0xFFFFFFFF;
        for (int i = msb - 1; i >= 0; --i) {
            mask &= (x[i] ^ op[i]);
        }
        mask = ~mask;
        for (int i = msb - 1; i >= 0; --i) {
            x[i] &= mask;
        }
    }
    return x[0];
}

int singleNumber(int *nums, int numsSize)
{
    int lower = 0, higher = 0;
    for (int i = 0; i < numsSize; i++) {
        lower ^= nums[i];
        lower &= ~higher;
        higher ^= nums[i];
        higher &= ~lower;
    }
    return lower;
}

void threeNumChecker(void)
{
    int x1[] = {0, 1, 0, 1, 0, 1, 3};
    int x2[] = {2, 1, 2, 2};
    int x3[] = {999};
    checker(3, x1, sizeof(x1) / sizeof(int));
    checker(1, x2, sizeof(x2) / sizeof(int));
    checker(999, x3, sizeof(x3) / sizeof(int));
    generalChecker(3, 3, x1, sizeof(x1) / sizeof(int));
    generalChecker(3, 1, x2, sizeof(x2) / sizeof(int));
    generalChecker(3, 999, x3, sizeof(x3) / sizeof(int));
    printf("Test \"three num checker\": PASS\n");
}

void fourNumChecker(void)
{
    int x1[] = {0, 1, 0, 1, 0, 1, 0, 1, 3};
    int x2[] = {2, 1, 2, 2, 2};
    int x3[] = {999};
    int x4[] = {0xF, 0xF, 0xFF, 0xF, 0xF};
    int x5[] = {2, 0, 2, 2, 2};
    generalChecker(4, 3, x1, sizeof(x1) / sizeof(int));
    generalChecker(4, 1, x2, sizeof(x2) / sizeof(int));
    generalChecker(4, 999, x3, sizeof(x3) / sizeof(int));
    generalChecker(4, 0xFF, x4, sizeof(x4) / sizeof(int));
    generalChecker(4, 0, x5, sizeof(x5) / sizeof(int));
    printf("Test \"four num checker\": PASS\n");
}

void fiveNumChecker(void)
{
    int x1[] = {0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 3};
    int x2[] = {2, 1, 2, 2, 2, 2};
    int x3[] = {999};
    int x4[] = {0xF, 0xF, 0xF, 0xFF, 0xF, 0xF};
    int x5[] = {2, 0, 2, 2, 2, 2};
    generalChecker(5, 3, x1, sizeof(x1) / sizeof(int));
    generalChecker(5, 1, x2, sizeof(x2) / sizeof(int));
    generalChecker(5, 999, x3, sizeof(x3) / sizeof(int));
    generalChecker(5, 0xFF, x4, sizeof(x4) / sizeof(int));
    generalChecker(5, 0, x5, sizeof(x5) / sizeof(int));
    printf("Test \"five num checker\": PASS\n");
}

void sevenNumChecker(void)
{
    int x1[] = {0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 3, 0, 0, 1, 1};
    int x2[] = {2, 1, 2, 2, 2, 2, 2, 2};
    int x3[] = {999};
    int x4[] = {0xF, 0xF, 0xF, 0xFF, 0xF, 0xF, 0xF, 0xF};
    int x5[] = {2, 0, 2, 2, 2, 2, 2, 2};
    generalChecker(7, 3, x1, sizeof(x1) / sizeof(int));
    generalChecker(7, 1, x2, sizeof(x2) / sizeof(int));
    generalChecker(7, 999, x3, sizeof(x3) / sizeof(int));
    generalChecker(7, 0xFF, x4, sizeof(x4) / sizeof(int));
    generalChecker(7, 0, x5, sizeof(x5) / sizeof(int));
    printf("Test \"five num checker\": PASS\n");
}

int main(void)
{
    threeNumChecker();
    fourNumChecker();
    fiveNumChecker();
    sevenNumChecker();
    return 0;
}
