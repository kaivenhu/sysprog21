#ifndef XSTRING_H_
#define XSTRING_H_
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct RefCounted {
    size_t refcnt;
    char data[1];
} RefCounted;

typedef union {
    /* allow strings up to 15 bytes to stay on the stack
     * use the last byte as a null terminator and to store flags
     * much like fbstring:
     * https://github.com/facebook/folly/blob/master/folly/docs/FBString.md
     */
    char data[16];

    struct {
        uint8_t filler[15],
            /* how many free bytes in this stack allocated string
             * same idea as fbstring
             */
            space_left : 4,
            /* if it is on heap, set to 1 */
            is_ptr : 1, flag1 : 1, flag2 : 1, flag3 : 1;
    };

    /* heap allocated */
    struct {
        char *ptr;
        /* supports strings up to 2^54 - 1 bytes */
        size_t size : 54,
            /* capacity is always a power of 2 (unsigned)-1 */
            capacity : 6;
        /* the last 4 bits are important flags */
    };
} xs;

static inline bool xs_is_ptr(const xs *x)
{
    return x->is_ptr;
}

static inline RefCounted *refcount_fromxs(const xs *p)
{
    assert(xs_is_ptr(p));
    return (RefCounted *) ((void *) p->ptr - offsetof(RefCounted, data));
}

static inline size_t refcount_get(const xs *p)
{
    return xs_is_ptr(p) ? ((RefCounted *) refcount_fromxs(p))->refcnt : 1;
}

static inline void refcount_increment(const xs *p)
{
    ++((RefCounted *) refcount_fromxs(p))->refcnt;
}

static inline void refcount_decrement(const xs *p)
{
    --((RefCounted *) refcount_fromxs(p))->refcnt;
}
static inline size_t xs_size(const xs *x)
{
    return xs_is_ptr(x) ? x->size : (size_t)(15 - x->space_left);
}
static inline char *xs_data(const xs *x)
{
    return xs_is_ptr(x) ? (char *) x->ptr : (char *) x->data;
}
static inline size_t xs_capacity(const xs *x)
{
    return xs_is_ptr(x) ? ((size_t) 1 << x->capacity) - 1 : 15;
}
static inline bool xs_is_free(const xs *x)
{
    return (0 == refcount_get(x));
}

#define xs_literal_empty() \
    (xs) { .space_left = 15 }

static inline int ilog2(uint32_t n)
{
    return 32 - __builtin_clz(n) - 1;
}

xs *xs_new(xs *x, const void *p);

/* Memory leaks happen if the string is too long but it is still useful for
 * short strings.
 * "" causes a compile-time error if x is not a string literal or too long.
 */
#define xs_tmp(x)                                          \
    ((void) ((struct {                                     \
         _Static_assert(sizeof(x) <= 16, "it is too big"); \
         int dummy;                                        \
     }){1}),                                               \
     xs_new(&xs_literal_empty(), "" x))

static inline xs *xs_newempty(xs *x)
{
    *x = xs_literal_empty();
    return x;
}

static inline xs *xs_free(xs *x)
{
    if (xs_is_ptr(x)) {
        refcount_decrement(x);
        if (xs_is_free(x)) {
            free(refcount_fromxs(x));
        }
    }
    return xs_newempty(x);
}

xs *xs_grow(xs *x, size_t len);
xs *xs_concat(xs *string, const xs *prefix, const xs *suffix);
xs *xs_trim(xs *x, const char *trimset);
xs *xs_copy(xs *src, xs *dest);
char *xs_tok_r(xs *x, const char *delim, char **saveptr);

#endif /* XSTRING_H_ */
