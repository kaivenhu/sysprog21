#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "xvec.h"

/* This function attribute specifies function parameters that are not supposed
 * to be null pointers. This enables the compiler to generate a warning on
 * encountering such a parameter.
 */
__attribute__((nonnull)) void vec_free(void *p)
{
    STRUCT_BODY(void) *s = p;
    if (s->on_heap)
        free(s->ptr);
}

static inline int ilog2(size_t n)
{
    return 64 - __builtin_clzl(n) - ((n & (n - 1)) == 0);
}

static inline size_t expand_capacity(size_t c __attribute__((unused)))
{
#ifdef POW_FACTOR
    return 1;
#else  /* POW_FACTOR */
    return ilog2(c) + 1;
#endif /* POW_FACTOR */
}

__attribute__((nonnull)) void __vec_reserve(void *vec,
                                            size_t n,
                                            size_t elemsize,
                                            size_t capacity)
{
    union {
        STRUCT_BODY(void);
        struct {
            size_t filler;
            char buf[];
        };
    } *v = vec;

    if (n > capacity) {
        if (v->on_heap) {
            v->ptr = realloc(v->ptr,
                             elemsize * (size_t) 1 << (v->capacity = ilog2(n)));
        } else {
            void *tmp =
                malloc(elemsize * (size_t) 1 << (v->capacity = ilog2(n)));
            memcpy(tmp, v->buf, elemsize * v->size);
            v->ptr = tmp;
            v->on_heap = 1;
        }
    }
}

__attribute__((nonnull)) void __vec_push_back(void *restrict vec,
                                              void *restrict e,
                                              size_t elemsize,
                                              size_t capacity)
{
    union {
        STRUCT_BODY(char);
        struct {
            size_t filler;
            char buf[];
        };
    } *v = vec;

    if (v->on_heap) {
        if (v->size == capacity)
            v->ptr = realloc(v->ptr, elemsize * to_capacity(++v->capacity));
        memcpy(&v->ptr[v->size++ * elemsize], e, elemsize);
    } else {
        if (v->size == capacity) {
            void *tmp =
                malloc(elemsize *
                       to_capacity((v->capacity = expand_capacity(capacity))));
            memcpy(tmp, v->buf, elemsize * v->size);
            v->ptr = tmp;
            v->on_heap = 1;
            memcpy(&v->ptr[v->size++ * elemsize], e, elemsize);
        } else
            memcpy(&v->buf[v->size++ * elemsize], e, elemsize);
    }
}

__attribute__((nonnull)) void __vec_insert(void *restrict vec,
                                           size_t index,
                                           void *restrict e,
                                           size_t elemsize,
                                           size_t capacity)
{
    char *ptr = NULL;
    union {
        STRUCT_BODY(char);
        struct {
            size_t filler;
            char buf[];
        };
    } *v = vec;

    assert(index < v->size);

    if (v->on_heap) {
        if (v->size == capacity)
            v->ptr = realloc(v->ptr, elemsize * to_capacity(++v->capacity));
        ptr = v->ptr;
    } else {
        if (v->size == capacity) {
            void *tmp =
                malloc(elemsize *
                       to_capacity((v->capacity = expand_capacity(capacity))));
            memcpy(tmp, v->buf, elemsize * v->size);
            v->ptr = tmp;
            v->on_heap = 1;
            ptr = v->ptr;
        } else {
            ptr = v->buf;
        }
    }
    memmove(&ptr[(1 + index) * elemsize], &ptr[index * elemsize],
            (v->size - index) * elemsize);
    memcpy(&ptr[index * elemsize], e, elemsize);
    ++(v->size);
}

__attribute__((nonnull)) void __vec_erase(void *restrict vec,
                                          size_t index,
                                          size_t elemsize)
{
    union {
        STRUCT_BODY(char);
        struct {
            size_t filler;
            char buf[];
        };
    } *v = vec;
    assert(index < v->size);

    char *ptr = (v->on_heap) ? v->ptr : v->buf;

    memmove(&ptr[index * elemsize], &ptr[(1 + index) * elemsize],
            (v->size - index) * elemsize);
    --(v->size);
}
