#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define hexchar_checker(num, c)                                             \
    do {                                                                    \
        if (num != hexchar2val(c)) {                                        \
            printf("Failed with char: %c, result: %d on line %d\n", c, num, \
                   __LINE__);                                               \
            exit(1);                                                        \
        }                                                                   \
    } while (0)

uint8_t hexchar2val(uint8_t in)
{
    const uint8_t letter = in & 0x40;
    const uint8_t shift = (letter >> 3) | (letter >> 6);
    return (in + shift) & 0xf;
}

void hexchar2val_tester(void)
{
    for (unsigned int i = (unsigned int) '0'; i <= (unsigned int) '9'; ++i) {
        hexchar_checker((i - '0'), (char) i);
    }
    for (unsigned int i = (unsigned int) 'a'; i <= (unsigned int) 'f'; ++i) {
        hexchar_checker((i - 'a') + 10, (char) i);
    }
    for (unsigned int i = (unsigned int) 'A'; i <= (unsigned int) 'F'; ++i) {
        hexchar_checker((i - 'A') + 10, (char) i);
    }
    printf("Test \"hex char to value\": PASS\n");
}

int main(void)
{
    hexchar2val_tester();
    return 0;
}
