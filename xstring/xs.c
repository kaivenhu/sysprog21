#include <string.h>

#include "xs.h"

static inline char *refcount_create(size_t size)
{
    RefCounted *rc = malloc(sizeof(RefCounted) + size - 1);
    rc->refcnt = 1;
    return &(rc->data[0]);
}

static inline char *refcount_realloc(const xs *x, size_t size)
{
    char *p = NULL;
    assert(1 == refcount_get(x));
    RefCounted *src = refcount_fromxs(x);
    p = refcount_create(size);
    memcpy(p, src->data, x->size + 1);
    free(src);
    return p;
}

xs *xs_new(xs *x, const void *p)
{
    *x = xs_literal_empty();
    size_t len = strlen(p) + 1;
    if (len > 16) {
        x->capacity = ilog2(len) + 1;
        x->size = len - 1;
        x->is_ptr = true;
        x->ptr = refcount_create((size_t) 1 << x->capacity);
        memcpy(x->ptr, p, len);
    } else {
        memcpy(x->data, p, len);
        x->space_left = 15 - (len - 1);
    }
    return x;
}

/* grow up to specified size */
xs *xs_grow(xs *x, size_t len)
{
    if (len <= xs_capacity(x))
        return x;
    len = ilog2(len) + 1;
    if (xs_is_ptr(x))
        x->ptr = refcount_realloc(x, (size_t) 1 << len);
    else {
        char buf[16];
        memcpy(buf, x->data, 16);
        x->ptr = refcount_create((size_t) 1 << len);
        memcpy(x->ptr, buf, 16);
    }
    x->is_ptr = true;
    x->capacity = len;
    return x;
}

static inline xs *xs_cow(xs *x)
{
    if (1 < refcount_get(x)) {
        xs tmps = xs_literal_empty();
        xs_grow(&tmps, xs_capacity(x));
        memcpy(xs_data(&tmps), xs_data(x), xs_size(x) + 1);
        xs_free(x);
        *x = tmps;
    }
    return x;
}

xs *xs_concat(xs *string, const xs *prefix, const xs *suffix)
{
    size_t pres = xs_size(prefix), sufs = xs_size(suffix),
           size = xs_size(string), capacity = xs_capacity(string);

    char *pre = xs_data(prefix), *suf = xs_data(suffix),
         *data = xs_data(string);

    if (size + pres + sufs <= capacity) {
        data = xs_data(xs_cow(string));
        memmove(data + pres, data, size);
        memcpy(data, pre, pres);
        memcpy(data + pres + size, suf, sufs + 1);
    } else {
        xs tmps = xs_literal_empty();
        xs_grow(&tmps, size + pres + sufs);
        char *tmpdata = xs_data(&tmps);
        memcpy(tmpdata + pres, data, size);
        memcpy(tmpdata, pre, pres);
        memcpy(tmpdata + pres + size, suf, sufs + 1);
        xs_free(string);
        *string = tmps;
    }
    if (xs_is_ptr(string)) {
        string->size = (size + pres + sufs);
    } else {
        string->space_left = 15 - (size + pres + sufs);
    }
    return string;
}

xs *xs_trim(xs *x, const char *trimset)
{
    if (!trimset[0])
        return x;

    char *dataptr = xs_data(x), *orig = dataptr;

    /* similar to strspn/strpbrk but it operates on binary data */
    uint8_t mask[32] = {0};

#define check_bit(byte) (mask[(uint8_t) byte / 8] & 1 << (uint8_t) byte % 8)
#define set_bit(byte) (mask[(uint8_t) byte / 8] |= 1 << (uint8_t) byte % 8)

    size_t i, slen = xs_size(x), trimlen = strlen(trimset);

    for (i = 0; i < trimlen; i++)
        set_bit(trimset[i]);
    for (i = 0; i < slen; i++)
        if (!check_bit(dataptr[i]))
            break;
    for (; slen > 0; slen--)
        if (!check_bit(dataptr[slen - 1]))
            break;
    dataptr += i;
    slen -= i;

    orig = xs_data(xs_cow(x));

    /* reserved space as a buffer on the heap.
     * Do not reallocate immediately. Instead, reuse it as possible.
     * Do not shrink to in place if < 16 bytes.
     */
    memmove(orig, dataptr, slen);
    /* do not dirty memory unless it is needed */
    if (orig[slen])
        orig[slen] = 0;

    if (xs_is_ptr(x))
        x->size = slen;
    else
        x->space_left = 15 - slen;
    return x;
}

char *xs_tok_r(xs *x, const char *delim, char **saveptr)
{
    assert(saveptr && delim);
    if (!delim[0])
        return xs_data(x);

    char *orig = (x) ? xs_data(xs_cow(x)) : *saveptr;

    /* similar to strspn/strpbrk but it operates on binary data */
    uint8_t mask[32] = {0};

    size_t i, start, slen = strlen(orig), delimlen = strlen(delim);

    for (i = 0; i < delimlen; ++i)
        set_bit(delim[i]);

    for (i = 0; i < slen; ++i) {
        if (!check_bit(orig[i]))
            break;
    }
    if (!*(orig + i))
        return NULL;
    start = i;

    for (; i < slen; ++i) {
        if (check_bit(orig[i])) {
            orig[i] = '\0';
            ++i;
            break;
        }
    }

    *saveptr = orig + i;

    return orig + start;
#undef check_bit
#undef set_bit
}

xs *xs_copy(xs *src, xs *dest)
{
    *dest = *src;

    if (xs_is_ptr(dest)) {
#ifdef DISABLE_COW
        dest->ptr = refcount_create((size_t) 1 << src->capacity);
        memcpy(dest->ptr, src->ptr, xs_size(src) + 1);
#else  /* DISABLE_COW */
        refcount_increment(dest);
#endif /* DISABLE_COW */
    }

    return dest;
}
