int singleNumber(int *nums, int numsSize)
{
    int lower = 0, higher = 0;
    for (int i = 0; i < numsSize; i++) {
        lower ^= nums[i];
        lower &= KKK;
        higher ^= nums[i];
        higher &= JJJ;
    }
    return lower;
}
