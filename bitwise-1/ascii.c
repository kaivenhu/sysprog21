#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ascii_checker(flag, str)                                           \
    do {                                                                   \
        if (flag != is_ascii(str, sizeof(str))) {                          \
            printf("Failed with str: %s, ret: %d on line %d\n", str, flag, \
                   __LINE__);                                              \
            exit(1);                                                       \
        }                                                                  \
    } while (0)

bool is_ascii(const char str[], size_t size)
{
    if (size == 0)
        return false;
    unsigned int i = 0;
    while ((i + 8) <= size) {
        uint64_t payload;
        memcpy(&payload, str + i, 8);
        if (payload & 0x8080808080808080)
            return false;
        i += 8;
    }
    while (i < size) {
        if (str[i] & 0x80)
            return false;
        i++;
    }
    return true;
}

static void is_ascii_tester(void)
{
    char str[135] = {'\0'};
    for (int i = 0; i < 128; ++i) {
        str[i] = (char) i;
    }
    ascii_checker(true, str);

    for (int i = 0; i < 16; ++i) {
        str[i] = 128 + i;
        ascii_checker(false, str);
        str[i] = 0;
    }

    for (int i = 128; i < 135; ++i) {
        str[i] = 128;
        ascii_checker(false, str);
        str[i] = 0;
    }
    printf("Test \"is ascii\": PASS\n");
}

int main(void)
{
    is_ascii_tester();
    return 0;
}
