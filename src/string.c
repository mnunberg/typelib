#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "tl_string.h"

static void ensure_cstr(tl_STRING *str);

#define TLSTR_AVAIL(s) (s)->nalloc - (s)->nused

int tl_str_init(tl_STRING *str)
{
    str->base = NULL;
    str->nalloc = 0;
    str->nused = 0;
    return 0;
}

void tl_str_cleanup(tl_STRING *str)
{
    if (str->base == NULL) {
        return;
    }
    free(str->base);
    memset(str, 0, sizeof(*str));
}

void tl_str_clear(tl_STRING *str)
{
    str->nused = 0;
    if (str->nalloc) {
        ensure_cstr(str);
    }
}

void tl_str_added(tl_STRING *str, size_t nused)
{
    str->nused += nused;
    assert(str->nused <= str->nalloc);
    ensure_cstr(str);
}

int tl_str_reserve(tl_STRING *str, size_t size)
{
    size_t newalloc;
    char *newbuf;

    /** Set size to one greater, for the terminating NUL */
    size++;
    if (!size) {
        return -1;
    }

    if (str->nalloc - str->nused >= size) {
        return 0;
    }

    newalloc = str->nalloc;
    if (!newalloc) {
        newalloc = 1;
    }

    while (newalloc - str->nused < size) {
        if (newalloc * 2 < newalloc) {
            return -1;
        }

        newalloc *= 2;
    }

    newbuf = realloc(str->base, newalloc);
    if (newbuf == NULL) {
        return -1;
    }

    str->base = newbuf;
    str->nalloc = newalloc;
    return 0;
}

static void ensure_cstr(tl_STRING *str)
{
    str->base[str->nused] = '\0';
}

int tl_str_append(tl_STRING *str, const void *data, size_t size)
{
    if (tl_str_reserve(str, size)) {
        return -1;
    }

    memcpy(str->base + str->nused, data, size);
    str->nused += size;
    ensure_cstr(str);
    return 0;
}

int tl_str_appendz(tl_STRING *str, const char *s)
{
    return tl_str_append(str, s, strlen(s));
}

void tl_str_erase_end(tl_STRING *str, size_t to_remove)
{
    assert(to_remove <= str->nused);
    str->nused -= to_remove;
    ensure_cstr(str);
}

void tl_str_erase_begin(tl_STRING *str, size_t to_remove)
{
    assert(to_remove <= str->nused);
    if (!to_remove) {
        str->nused = 0;
        return;
    }

    memmove(str->base, str->base + to_remove, str->nused - to_remove);
    str->nused -= to_remove;
    ensure_cstr(str);
}

void tl_str_transfer(tl_STRING *from, tl_STRING *to)
{
    assert(to->base == NULL);
    *to = *from;
    memset(from, 0, sizeof(*from));
}

/*
 * These functions return 0 on success, -1 on error, and 1 to loop again
 */
static int
fmt_resize_c99(tl_STRING *str, int rv, int sz)
{
    if (rv < 0) {
        return -1;
    } else if (rv < sz) {
        return 0;
    } else {
        if (tl_str_reserve(str, rv)) {
            return -1;
        }
        return 1;
    }
}

static int
fmt_resize_msc(tl_STRING *str, int rv, int sz)
{
    if (rv > -1 && rv < sz) {
        return 0;
    } else {
        /** try to reserve more space */
        if (TLSTR_AVAIL(str) + 4096 < TLSTR_AVAIL(str)) {
            return -1;
        }
        if (tl_str_reserve(str, TLSTR_AVAIL(str) + 4096)) {
            return -1;
        }
    }
    return 1;
}

#if __STDC_VERSION__ >= 199901L
#define fmt_resize(str, rv, sz) fmt_resize_c89(str, rv, sz)
#else
#define fmt_resize(str, rv, sz) fmt_resize_msc(str, rv, sz)
#endif

#define FMT_RESIZE(str, rv, sz) do { \
    rv = fmt_resize(str, rv, sz); \
    if (0) { fmt_resize_msc(NULL,0,0); fmt_resize_c99(NULL,0,0); } \
} while (0);

int tl_str_appendv(tl_STRING *str, const char *fmt, va_list ap)
{
    int sz;
    do {
        int rv;
        va_list cap;
        va_copy(cap, ap);
        sz = TLSTR_AVAIL(str);
        rv = vsnprintf(str->base + str->nused, sz, fmt, cap);
        va_end(cap);
        FMT_RESIZE(str, rv, sz);
        if (rv != 1) {
            return rv;
        }
    } while (1);

    /* no return here */
    return -1;
}

int tl_str_appendf(tl_STRING *str, const char *fmt, ...)
{
    int rv;
    va_list ap;
    va_start(ap, fmt);
    rv = tl_str_appendv(str, fmt, ap);
    va_end(ap);
    return rv;
}

int tl_asprintf(char **strp, const char *fmt, ...)
{
    va_list ap;
    tl_STRING str;
    int rv;

    if (tl_str_init(&str)) {
        return -1;
    }

    va_start(ap, fmt);
    rv = tl_str_appendv(&str, fmt, ap);
    va_end(ap);

    if (rv) {
        tl_str_cleanup(&str);
        return -1;
    }

    *strp = str.base;
    return str.nused;
}
