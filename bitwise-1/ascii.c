#include <ctype.h>
#include <nmmintrin.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ascii_checker(flag, str, fp)                                       \
    do {                                                                   \
        if (flag != fp(str, sizeof(str))) {                                \
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
    ascii_checker(true, str, is_ascii);

    for (int i = 0; i < 16; ++i) {
        str[i] = 128 + i;
        ascii_checker(false, str, is_ascii);
        str[i] = 0;
    }

    for (int i = 128; i < 135; ++i) {
        str[i] = 128;
        ascii_checker(false, str, is_ascii);
        str[i] = 0;
    }
    printf("Test \"is ascii\": PASS\n");
}

#define PACKED_BYTE(b) (((uint64_t)(b) & (0xff)) * 0x0101010101010101u)

static bool is_alphabet(const char *str, size_t size)
{
    if (size == 0)
        return false;
    size_t i = 0;
    while ((i + 8) <= size) {
        uint64_t chunk;
        memcpy(&chunk, str + i, 8);
        if ((chunk & PACKED_BYTE(128)) == 0) { /* is ASCII? */
            chunk |= PACKED_BYTE(0x20);
            uint64_t a = chunk + PACKED_BYTE(128 - 'a');
            uint64_t z = chunk + PACKED_BYTE(128 - 'z' - 1);
            if (PACKED_BYTE(128) != ((a ^ z) & PACKED_BYTE(128))) {
                return false;
            }
        } else {
            return false;
        }
        i += 8;
    }
    while (i < size) {
        char c = str[i];
        if ((c & 128) == 0) { /* is ASCII? */
            c |= 0x20;
            char a = c + (128 - 'a');
            char z = c + (128 - 'z' - 1);
            if (128 != ((a ^ z) & 128)) {
                return false;
            }
        } else {
            return false;
        }
        i++;
    }
    return true;
}

static bool is_alphabet_sse(const char *str, size_t size)
{
    const __m128i range =
        _mm_setr_epi8('a', 'z', 'A', 'Z', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    size_t i = 0;
    while ((i + 16) <= size) {
        const __m128i m = _mm_loadu_si128((__m128i *) (str + i));
        if (16 != _mm_cmpistri(range, m,
                               _SIDD_CMP_RANGES | _SIDD_NEGATIVE_POLARITY)) {
            return false;
        }
        i += 16;
    }
    size -= i;
    if (size) {
        const __m128i m = _mm_loadu_si128((__m128i *) (str + i));
        if (size != (size_t) _mm_cmpistri(
                        range, m, _SIDD_CMP_RANGES | _SIDD_NEGATIVE_POLARITY)) {
            return false;
        }
    }
    return true;
}

static void is_aphabet_tester(const char *key,
                              bool (*func)(const char *, size_t))
{
    char str[135] = {'\0'};
    for (int c = 65; c <= 90; ++c) {
        for (int i = 0; i < 135; ++i) {
            str[i] = (char) c;
        }
        ascii_checker(true, str, func);
    }
    for (int c = 97; c <= 122; ++c) {
        for (int i = 0; i < 135; ++i) {
            str[i] = (char) c;
        }
        ascii_checker(true, str, func);
    }

    for (int c = 0; c < 65; ++c) {
        for (int i = 0; i < 16; ++i) {
            str[i] = (char) c;
            ascii_checker(false, str, func);
            str[i] = 'A';
        }
        for (int i = 128; i < 135; ++i) {
            str[i] = (char) c;
            ascii_checker(false, str, func);
            str[i] = 'A';
        }
    }
    ascii_checker(true, str, func);

    for (int c = 91; c < 97; ++c) {
        for (int i = 0; i < 16; ++i) {
            str[i] = (char) c;
            ascii_checker(false, str, func);
            str[i] = 'A';
        }
        for (int i = 128; i < 135; ++i) {
            str[i] = (char) c;
            ascii_checker(false, str, func);
            str[i] = 'A';
        }
    }
    ascii_checker(true, str, func);

    for (int c = 123; c < 256; ++c) {
        for (int i = 0; i < 16; ++i) {
            str[i] = (char) c;
            ascii_checker(false, str, func);
            str[i] = 'A';
        }
        for (int i = 128; i < 135; ++i) {
            str[i] = (char) c;
            ascii_checker(false, str, func);
            str[i] = 'A';
        }
    }
    ascii_checker(true, str, func);

    printf("Test \"is alphabet\" %s version: PASS\n", key);
}

int main(void)
{
    is_ascii_tester();
    is_aphabet_tester("bitwise", is_alphabet);
    is_aphabet_tester("sse", is_alphabet_sse);
    return 0;
}
