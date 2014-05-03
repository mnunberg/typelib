/*
 * Copyright (c) 2006  Dustin Sallings <dustin@spy.net>
 */

#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tl_hashtable.h"

/* Table of 32 primes by their distance from the nearest power of two */
static size_t prime_size_table[] = {
    3, 7, 13, 23, 47, 97, 193, 383, 769, 1531, 3067, 6143, 12289, 24571, 49157,
    98299, 196613, 393209, 786433, 1572869, 3145721, 6291449, 12582917,
    25165813, 50331653, 100663291, 201326611, 402653189, 805306357,
    1610612741
};

#define TABLE_SIZE ((int)(sizeof(prime_size_table) / sizeof(int)))

struct genhash_entry_t {
    /** The key for this entry */
    void *key;
    /** Size of the key */
    size_t nkey;
    /** The value for this entry */
    void *value;
    /** Size of the value */
    size_t nvalue;
    /** Pointer to the next entry */
    struct genhash_entry_t *next;
};

struct _genhash {
    size_t size;
    struct tl_HASHOPS ops;
    struct genhash_entry_t *buckets[];
};

static size_t estimate_table_size(size_t est);


static void *dup_key(tl_HASHTABLE *h, const void *key, size_t klen)
{
    if (h->ops.dup_key != NULL) {
        return h->ops.dup_key(key, klen);
    } else {
        return (void *)key;
    }
}

static void *dup_value(tl_HASHTABLE *h, const void *value, size_t vlen)
{
    if (h->ops.dup_value != NULL) {
        return h->ops.dup_value(value, vlen);
    } else {
        return (void *)value;
    }
}

static void free_key(tl_HASHTABLE *h, void *key)
{
    if (h->ops.free_key != NULL) {
        h->ops.free_key(key);
    }
}

static void free_value(tl_HASHTABLE *h, void *value)
{
    if (h->ops.free_value != NULL) {
        h->ops.free_value(value);
    }
}

static size_t estimate_table_size(size_t est)
{
    size_t rv = 0;
    while (prime_size_table[rv] < est && rv + 1 < TABLE_SIZE) {
        rv++;
    }
    return prime_size_table[rv];
}

void tl_ht_free(tl_HASHTABLE *h)
{
    if (h != NULL) {
        tl_ht_clear(h);
        free(h);
    }
}

int tl_ht_store(tl_HASHTABLE *h, const void *k, size_t klen,
                  const void *v, size_t vlen)
{
    size_t n = 0;
    struct genhash_entry_t *p;

    assert(h != NULL);

    n = h->ops.hashfunc(k, klen) % h->size;
    assert(n < h->size);

    p = calloc(1, sizeof(struct genhash_entry_t));
    if (!p) {
        return -1;
    }

    p->key = dup_key(h, k, klen);
    p->nkey = klen;
    p->value = dup_value(h, v, vlen);
    p->nvalue = vlen;

    p->next = h->buckets[n];
    h->buckets[n] = p;
    return 0;
}

static struct genhash_entry_t *genhash_find_entry(tl_HASHTABLE *h,
                                                  const void *k,
                                                  size_t klen)
{
    size_t n = 0;
    struct genhash_entry_t *p;

    assert(h != NULL);
    n = h->ops.hashfunc(k, klen) % h->size;
    assert(n < h->size);

    p = h->buckets[n];
    for (p = h->buckets[n]; p && !h->ops.hasheq(k, klen, p->key, p->nkey); p = p->next);
    return p;
}

void *tl_ht_find(tl_HASHTABLE *h, const void *k, size_t klen)
{
    struct genhash_entry_t *p;
    void *rv = NULL;

    p = genhash_find_entry(h, k, klen);

    if (p) {
        rv = p->value;
    }
    return rv;
}

enum tl_UPDATETYPE tl_ht_update(tl_HASHTABLE *h, const void *k, size_t klen,
                                const void *v, size_t vlen)
{
    struct genhash_entry_t *p;
    enum tl_UPDATETYPE rv = 0;

    p = genhash_find_entry(h, k, klen);

    if (p) {
        free_value(h, p->value);
        p->value = dup_value(h, v, vlen);
        rv = MODIFICATION;
    } else {
        if (-1 == tl_ht_store(h, k, klen, v, vlen)) {
            rv = ALLOC_FAILURE;
        }
        rv = NEW;
    }

    return rv;
}

enum tl_UPDATETYPE tl_ht_funupdate(tl_HASHTABLE *h,
                                    const void *k,
                                    size_t klen,
                                    void * (*upd)(const void *,
                                                  const void *,
                                                  size_t *,
                                                  void *),
                                    void (*fr)(void *),
                                    void *arg,
                                    const void *def,
                                    size_t deflen)
{
    struct genhash_entry_t *p;
    enum tl_UPDATETYPE rv = 0;
    size_t newSize = 0;

    p = genhash_find_entry(h, k, klen);

    if (p) {
        void *newValue = upd(k, p->value, &newSize, arg);
        free_value(h, p->value);
        p->value = dup_value(h, newValue, newSize);
        fr(newValue);
        rv = MODIFICATION;
    } else {
        void *newValue = upd(k, def, &newSize, arg);
        tl_ht_store(h, k, klen, newValue, newSize);
        fr(newValue);
        rv = NEW;
    }

    (void)deflen;
    return rv;
}

static void free_item(tl_HASHTABLE *h, struct genhash_entry_t *i)
{
    assert(i);
    free_key(h, i->key);
    free_value(h, i->value);
    free(i);
}

int tl_ht_del(tl_HASHTABLE *h, const void *k, size_t klen)
{
    struct genhash_entry_t *deleteme = NULL;
    size_t n = 0;
    int rv = 0;

    assert(h != NULL);
    n = h->ops.hashfunc(k, klen) % h->size;
    assert(n < h->size);

    if (h->buckets[n] != NULL) {
        /* Special case the first one */
        if (h->ops.hasheq(h->buckets[n]->key, h->buckets[n]->nkey, k, klen)) {
            deleteme = h->buckets[n];
            h->buckets[n] = deleteme->next;
        } else {
            struct genhash_entry_t *p = NULL;
            for (p = h->buckets[n]; deleteme == NULL && p->next != NULL; p = p->next) {
                if (h->ops.hasheq(p->next->key, p->next->nkey, k, klen)) {
                    deleteme = p->next;
                    p->next = deleteme->next;
                }
            }
        }
    }
    if (deleteme != NULL) {
        free_item(h, deleteme);
        rv++;
    }

    return rv;
}

int tl_ht_delall(tl_HASHTABLE *h, const void *k, size_t klen)
{
    int rv = 0;
    while (tl_ht_del(h, k, klen) == 1) {
        rv++;
    }
    return rv;
}

void tl_ht_iter(tl_HASHTABLE *h,
                  void (*iterfunc)(const void *key, size_t nkey,
                                   const void *val, size_t nval,
                                   void *arg), void *arg)
{
    size_t i = 0;
    struct genhash_entry_t *p = NULL;
    assert(h != NULL);

    for (i = 0; i < h->size; i++) {
        for (p = h->buckets[i]; p != NULL; p = p->next) {
            iterfunc(p->key, p->nkey, p->value, p->nvalue, arg);
        }
    }
}

int tl_ht_clear(tl_HASHTABLE *h)
{
    size_t i = 0;
    int rv = 0;
    assert(h != NULL);

    for (i = 0; i < h->size; i++) {
        while (h->buckets[i]) {
            struct genhash_entry_t *p = NULL;
            p = h->buckets[i];
            h->buckets[i] = p->next;
            free_item(h, p);
        }
    }

    return rv;
}

static void count_entries(const void *key,
                          size_t klen,
                          const void *val,
                          size_t vlen,
                          void *arg)
{
    int *count = (int *)arg;
    (*count)++;
    (void)key;
    (void)klen;
    (void)val;
    (void)vlen;
}

int tl_ht_size(tl_HASHTABLE *h)
{
    int rv = 0;
    assert(h != NULL);
    tl_ht_iter(h, count_entries, &rv);
    return rv;
}

int tl_ht_sizekey(tl_HASHTABLE *h, const void *k, size_t klen)
{
    int rv = 0;
    assert(h != NULL);
    tl_ht_iterkey(h, k, klen, count_entries, &rv);
    return rv;
}

void tl_ht_iterkey(tl_HASHTABLE *h, const void *key, size_t klen,
                      void (*iterfunc)(const void *key, size_t klen,
                                       const void *val, size_t vlen,
                                       void *arg), void *arg)
{
    size_t n = 0;
    struct genhash_entry_t *p = NULL;

    assert(h != NULL);
    n = h->ops.hashfunc(key, klen) % h->size;
    assert(n < h->size);

    for (p = h->buckets[n]; p != NULL; p = p->next) {
        if (h->ops.hasheq(key, klen, p->key, p->nkey)) {
            iterfunc(p->key, p->nkey, p->value, p->nvalue, arg);
        }
    }
}

int tl_ht_strhash(const void *p, size_t nkey)
{
    int rv = 5381;
    int i = 0;
    char *str = (char *)p;

    for (i = 0; i < (int)nkey; i++) {
        assert(str[i]);
        rv = ((rv << 5) + rv) ^ str[i];
    }

    return rv;
}

tl_HASHTABLE* tl_ht_new(size_t est, struct tl_HASHOPS ops) {
    tl_HASHTABLE *rv = NULL;
    size_t size = 0;
    if (est < 1) {
        return NULL ;
    }
    assert(ops.hashfunc != NULL);
    assert(ops.hasheq != NULL);
    assert((ops.dup_key != NULL && ops.free_key != NULL) || ops.free_key == NULL);
    assert((ops.dup_value != NULL && ops.free_value != NULL) || ops.free_value == NULL);
    size = estimate_table_size(est);
    rv = calloc(1,
                sizeof(tl_HASHTABLE)
                        + (size * sizeof(struct genhash_entry_t*)));
    if (rv == NULL ) {
        return NULL ;
    }
    rv->size = size;
    rv->ops = ops;
    return rv;
}


/**
 * Convenience functions for creating string-based hashes
 */
static int
hasheq_strp(const void *a, size_t na, const void *b, size_t nb)
{
    if (na != nb) {
        return 0;
    }
    return memcmp(a, b, na) == 0;
}

static struct tl_HASHOPS hashops_nocopy = {
    tl_ht_strhash,
    hasheq_strp,
    NULL,
    NULL,
    NULL,
    NULL
};

static int u32_hash(const void *p, size_t n)
{
    (void)p;
    return n;
}

static int u32_eq(const void *a, size_t na, const void *b, size_t nb)
{
    (void)a; (void)b;
    return na == nb;
}

static struct tl_HASHOPS hashops_u32 = {
        u32_hash,
        u32_eq,
        NULL,
        NULL,
        NULL,
        NULL
};

tl_pHASHTABLE
tl_ht_stringnc_new(size_t est)
{
    return tl_ht_new(est, hashops_nocopy);
}

tl_pHASHTABLE
tl_ht_szt_new(size_t est)
{
    return tl_ht_new(est, hashops_u32);
}
