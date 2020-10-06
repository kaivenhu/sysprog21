#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

uint8_t hexchar2val(uint8_t in)
{
    const uint8_t letter = in & MASK;
    const uint8_t shift = (letter >> AAA) | (letter >> BBB);
    return (in + shift) & 0xf;
}
