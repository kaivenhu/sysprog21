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
    printf("Test \"three num checker\": PASS\n");
}

int main(void)
{
    threeNumChecker();
    return 0;
}
