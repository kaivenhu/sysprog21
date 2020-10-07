#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define hexchar_checker(num, c)                                             \
    do {                                                                    \
        if (num != hexchar2val(c)) {                                        \
            printf("Failed with char: %c, result: %d on line %d\n", c, num, \
                   __LINE__);                                               \
            exit(1);                                                        \
        }                                                                   \
    } while (0)

#define hexstr_checker(num, s)                                                 \
    do {                                                                       \
        uint32_t ans = 0;                                                      \
        if (num != (ans = hexstr2val(s, strlen(s)))) {                         \
            printf("Failed with str: %s, ret: %u, result: %u on line %d\n", s, \
                   ans, num, __LINE__);                                        \
            exit(1);                                                           \
        }                                                                      \
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

#define PACKED_BYTE(b) (((uint64_t)(b) & (0xff)) * 0x0101010101010101u)

uint64_t hexstr2val(const char *str, size_t size)
{
    assert(size && size <= 8);
    uint64_t hex = 0;
    for (size_t i = 0; i < size; ++i) {
        hex |= (uint64_t) str[i] << ((size - i - 1) << 3);
    }
    const uint64_t letter = hex & PACKED_BYTE(0x40);
    const uint64_t shift = (letter >> 3) | (letter >> 6);
    const uint64_t raw = (hex + shift) & PACKED_BYTE(0x0F);
    hex = 0;
    for (size_t i = 0; i < size; ++i) {
        hex |= (raw >> (i << 2)) & (0xF << (i << 2));
    }
    return hex;
}

void hexstr2val_tester(void)
{
    char str[16] = {'\0'};
    char format[8] = {'\0'};
    hexstr_checker(0, "0");
    hexstr_checker(0, "00");
    hexstr_checker(0, "00000000");
    hexstr_checker(0xF, "f");
    hexstr_checker(0xF, "0f");
    hexstr_checker(0xF, "0000000F");
    hexstr_checker(0xFFFFFFFF, "FFFFFFFF");

    for (uint32_t i = 0; i < 0xFFFFFFFF; ++i) {
        for (int f = 1; f <= 8; ++f) {
            snprintf(format, sizeof(format), "%%0%dX", f);
            snprintf(str, sizeof(str), format, i);
            hexstr_checker(i, str);
        }
    }
    printf("Test \"hex str to value\": PASS\n");
}

int main(void)
{
    hexchar2val_tester();
    hexstr2val_tester();
    return 0;
}
