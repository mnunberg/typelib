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

    if (TLSTR_AVAIL(str) >= size) {
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

#if __STDC_VERSION__ >= 199901L || __GNUC__
#define fmt_resize(str, rv, sz) fmt_resize_c99(str, rv, sz)
#define TLVACOPY(dst, src) va_copy(dst, src)
#else
#define fmt_resize(str, rv, sz) fmt_resize_msc(str, rv, sz)
#define TLVACOPY(dst, src) dst = src
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
        TLVACOPY(cap, ap);
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

char *tl_strndup(const char *a, unsigned na)
{
    char *ret = malloc(na+1);
    if (!ret) {
        return NULL;
    }
    memcpy(ret, a, na);
    ret[na] = '\0';
    return ret;
}

int tl_str_subst(tl_STRING *str, const char *orig, int norig,
                 const char *repl, int nrepl)
{
    char *alloc_a = NULL, *alloc_b = NULL, *tmp, *last;
    tl_STRING tmpstr;
    int rv = -1;

    if (!str->nused) {
        return 0;
    }

    tl_str_init(&tmpstr);

    if (norig != -1) {
        if (!(alloc_a = tl_strndup(orig, norig))) {
            goto GT_DONE;
        }
        orig = alloc_a;
    } else {
        norig = strlen(orig);
    }

    if (nrepl != -1) {
        if (!(alloc_b = tl_strndup(repl, nrepl))) {
            goto GT_DONE;
        }
        repl = alloc_b;
    } else {
        nrepl = strlen(repl);
    }

    if (norig == 0) {
        rv = 0;
        goto GT_DONE;
    }

    last = tmp = str->base;
    while ((tmp = strstr(tmp, orig))) {
        if (tl_str_append(&tmpstr, last, tmp-last)) {
            goto GT_DONE;
        }

        if (tl_str_append(&tmpstr, repl, nrepl)) {
            goto GT_DONE;
        }
        tmp += norig;
        last = tmp;
    }

    if (tmp) {
        /* no replacement needed */
        rv = 0;
        goto GT_DONE;
    }

    /* append the remainder */
    if (tl_str_append(&tmpstr, last, tl_str_tail(str) - last)) {
        goto GT_DONE;
    }

    tl_str_cleanup(str);
    *str = tmpstr;
    tmpstr.base = 0;
    tmpstr.nalloc = 0;
    tmpstr.nused = 0;
    rv = 0;

    GT_DONE:
    free(alloc_a);
    free(alloc_b);
    tl_str_cleanup(&tmpstr);
    return rv;
}

int tl_str_substz(tl_STRING *str, const char *orig, const char *repl)
{
    return tl_str_subst(str, orig, -1, repl, -1);
}
