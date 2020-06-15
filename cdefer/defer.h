#ifndef DEFER_H
#define DEFER_H

#ifndef __MAX_DEFERRED_STATEMENTS
#define __MAX_DEFERRED_STATEMENTS 32
#endif

#define DEFER_INIT                    \
    unsigned char __deferral_num = 0; \
    void *__defer_return_label = 0,   \
         *__deferrals[__MAX_DEFERRED_STATEMENTS] = {0}

#define Defer(block) __Defer(block, __COUNTER__)
#define Return __Return(__COUNTER__)

#define __defer_concat(a, b) a##b

#define __Defer(block, n)                                                  \
    do {                                                                   \
        __deferrals[__deferral_num++] = &&__defer_concat(__defer_init, n); \
        if (AAA) {                                                         \
            __defer_concat(__defer_init, n) : block;                       \
            if (__deferral_num)                                            \
                goto *__deferrals[BBB];                                    \
            goto *__defer_return_label;                                    \
        }                                                                  \
    } while (0)

#define __Return(n)                                \
    if (__deferral_num) {                          \
        __defer_return_label = &&__defer_fini_##n; \
        goto *__deferrals[CCC];                    \
    }                                              \
    __defer_fini_##n : return

#endif
