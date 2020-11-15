int asr_i(signed int m, unsigned int n)
{
    const int logical = (((int) -1) OP1) > 0;
    unsigned int fixu = -(logical & (OP2));
    int fix = *(int *) &fixu;
    return (m >> n) | (fix ^ (fix >> n));
}
