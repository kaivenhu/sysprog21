#ifndef XVECTOR_H_
#define XVECTOR_H_
#include <assert.h>
#include <math.h>
#include <stddef.h>

#define POW_FACTOR 1.5

/* vector with small buffer optimization */
#define STRUCT_BODY(type)                                                  \
    struct {                                                               \
        size_t size : 54, on_heap : 1, capacity : 6, flag1 : 1, flag2 : 1, \
            flag3 : 1;                                                     \
        type *ptr;                                                         \
    }

#ifdef POW_FACTOR
#define STACK_SIZE(s) 4

#define to_capacity(n) \
    ((size_t) ceil(pow(POW_FACTOR, (double) n)) * STACK_SIZE(n))

#else /* POW_FACTOR */
#define STACK_SIZE(s)    \
    (0 == (s & (s - 1))) \
        ? s              \
        : (size_t) 1 << (64 - __builtin_clzl(s)) /* next power of 2 */

#define to_capacity(n) ((size_t)(1 << n))
#endif /* POW_FACTOR */

#define v(t, s, name, ...)                                              \
    union {                                                             \
        STRUCT_BODY(t);                                                 \
        struct {                                                        \
            size_t filler;                                              \
            t buf[STACK_SIZE(s)];                                       \
        };                                                              \
    } name __attribute__((cleanup(vec_free))) = {.buf = {__VA_ARGS__}}; \
    name.size = sizeof((__typeof__(name.buf[0])[]){0, __VA_ARGS__}) /   \
                    sizeof(name.buf[0]) - 1;                            \
    name.capacity = sizeof(name.buf) / sizeof(name.buf[0])

#define vec_size(v) v.size
#define vec_capacity(v) \
    (v.on_heap ? to_capacity(v.capacity) : sizeof(v.buf) / sizeof(v.buf[0]))

#define vec_data(v) (v.on_heap ? v.ptr : v.buf) /* always contiguous buffer */

#define vec_elemsize(v) sizeof(v.buf[0])
#define vec_pos(v, n) ((__typeof__(v.buf[0]) *) __vec_pos(&v, n))[n]

#define vec_reserve(v, n) __vec_reserve(&v, n, vec_elemsize(v), vec_capacity(v))
#define vec_push_back(v, e)                                            \
    __vec_push_back(&v, &(__typeof__(v.buf[0])[]){e}, vec_elemsize(v), \
                    vec_capacity(v))
#define vec_insert(v, i, e)                                            \
    __vec_insert(&v, i, &(__typeof__(v.buf[0])[]){e}, vec_elemsize(v), \
                 vec_capacity(v))

#define vec_pop_back(v) (void) (v.size -= !!v.size)
#define vec_erase(v, i) __vec_erase(&v, i, vec_elemsize(v))

__attribute__((nonnull)) void vec_free(void *p);

__attribute__((nonnull)) void __vec_reserve(void *vec,
                                            size_t n,
                                            size_t elemsize,
                                            size_t capacity);
__attribute__((nonnull)) void __vec_push_back(void *restrict vec,
                                              void *restrict e,
                                              size_t elemsize,
                                              size_t capacity);
__attribute__((nonnull)) void __vec_insert(void *restrict vec,
                                           size_t index,
                                           void *restrict e,
                                           size_t elemsize,
                                           size_t capacity);
__attribute__((nonnull)) void __vec_erase(void *restrict vec,
                                          size_t index,
                                          size_t elemsize);

static inline __attribute__((nonnull)) void *__vec_pos(void *vec, size_t n)
{
    union {
        STRUCT_BODY(void);
        struct {
            size_t filler;
            char buf[];
        };
    } *v = vec;
    assert(n < v->size);
    return v->on_heap ? v->ptr : v->buf;
}

#endif /* XVECTOR_H_ */
