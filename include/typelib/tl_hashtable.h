/*
 * Generic hash table implementation.
 *
 * Copyright (c) 2006  Dustin Sallings <dustin@spy.net>
 */

#ifndef GENHASH_H
#define GENHASH_H 1
#include <stddef.h>

/*! \mainpage genhash
 *
 * \section intro_sec Introduction
 *
 * genhash is a generic hash table implementation in C.  It's
 * well-tested, freely available (MIT-license) and does what you need.
 *
 * \section docs_sec API Documentation
 *
 * Jump right into <a href="group___core.html">the API docs</a> to get started.
 */

/**
 * \defgroup Core genhash core
 */

/**
 * \addtogroup Core
 * @{
 */

/**
 * Operations on keys and values in the hash table.
 */
struct tl_HASHOPS {
    /**
     * Function to compute a hash for the given value.
     */
    int (*hashfunc)(const void *, size_t);
    /**
     * Function that returns true if the given keys are equal.
     */
    int (*hasheq)(const void *, size_t, const void *, size_t);
    /**
     * Function to duplicate a key for storage.
     */
    void *(*dup_key)(const void *, size_t);
    /**
     * Function to duplicate a value for storage.
     */
    void *(*dup_value)(const void *, size_t);
    /**
     * Function to free a key.
     */
    void (*free_key)(void *);
    /**
     * Function to free a value.
     */
    void (*free_value)(void *);
};

/**
 * The hash table structure.
 */
typedef struct _genhash tl_HASHTABLE, tl_HASHTABLE, *tl_pHASHTABLE;

/**
 * Type of update performed by an update function.
 */
enum tl_UPDATETYPE {
    MODIFICATION, /**< This update is modifying an existing entry */
    NEW,           /**< This update is creating a new entry */
    ALLOC_FAILURE
};

/**
 * Create a new generic hashtable.
 *
 * @param est the estimated number of items to store (must be > 0)
 * @param ops the key and value operations
 *
 * @return the new tl_HASHTABLE or NULL if one cannot be created
 */
tl_pHASHTABLE
tl_ht_new(size_t est, struct tl_HASHOPS ops);

tl_pHASHTABLE
tl_ht_stringnc_new(size_t est);

tl_pHASHTABLE
tl_ht_szt_new(size_t est);

/**
 * Free a gen hash.
 *
 * @param h the genhash to free (may be NULL)
 */
void
tl_ht_free(tl_HASHTABLE *h);

/**
 * Store an item.
 *
 * @param h the genhash
 * @param k the key
 * @param v the value
 */
int
tl_ht_store(tl_HASHTABLE *h, const void *k, size_t nk, const void *v, size_t nv);

/**
 * Get the most recent value stored for the given key.
 *
 * @param h the genhash
 * @param k the key
 *
 * @return the value, or NULL if one cannot be found
 */
void *
tl_ht_find(tl_HASHTABLE *h, const void *k, size_t klen);

/**
 * Delete the most recent value stored for a key.
 *
 * @param h the genhash
 * @param k the key
 *
 * @return the number of items deleted
 */
int
tl_ht_del(tl_HASHTABLE *h, const void *k, size_t klen);

/**
 * Delete all mappings of a given key.
 *
 * @param h the genhash
 * @param k the key
 *
 * @return the number of items deleted
 */
int
tl_ht_delall(tl_HASHTABLE *h, const void *k, size_t klen);

/**
 * Create or update an item in-place.
 *
 * @param h the genhash
 * @param k the key
 * @param v the new value to store for this key
 *
 * @return an indicator of whether this created a new item or updated
 *         an existing one
 */
enum tl_UPDATETYPE tl_ht_update(tl_HASHTABLE *h, const void *k, size_t klen,
                                const void *v, size_t vlen);

/**
 * Create or update an item in-place with a function.
 *
 * @param h hashtable
 * @param key the key of the item
 * @param upd function that will be called with the key and current
 *        value.  Should return the new value.
 * @param fr function to free the return value returned by the update
 *        function
 * @param def default value
 *
 * @return an indicator of whether this created a new item or updated
 *         an existing one
 */
enum tl_UPDATETYPE
tl_ht_funupdate(tl_HASHTABLE *h,
                const void *key, size_t klen,
                void * (*upd)(const void *k, const void *oldv, size_t *ns, void *a),
                void (*fr)(void *), void *arg, const void *def, size_t deflen);

typedef void (*tl_HASHITER_cb)(const void *k, size_t nk, const void *v, size_t nv,
        void *arg);

/**
 * Iterate all keys and values in a hash table.
 *
 * @param h the genhash
 * @param iterfunc a function that will be called once for every k/v pair
 * @param arg an argument to be passed to the iterfunc on each iteration
 */
void tl_ht_iter(tl_HASHTABLE *h, tl_HASHITER_cb iterfunc, void *arg);

/**
 * Iterate all values for a given key in a hash table.
 *
 * @param h the genhash
 * @param key the key to iterate
 * @param iterfunc a function that will be called once for every k/v pair
 * @param arg an argument to be passed to the iterfunc on each iteration
 */
void tl_ht_iterkey(tl_HASHTABLE *h, const void *key, size_t nkey,
                   tl_HASHITER_cb iterfunc, void *arg);

/**
 * Get the total number of entries in this hash table.
 *
 * @param h the genhash
 *
 * @return the number of entries in the hash table
 */
int tl_ht_size(tl_HASHTABLE *h);

/**
 * Remove all items from a genhash.
 *
 * @param h the genhash
 *
 * @return the number of items removed
 */
int tl_ht_clear(tl_HASHTABLE *h);

/**
 * Get the total number of entries in this hash table that map to the given
 * key.
 *
 * @param h the genhash
 * @param k a key
 *
 * @return the number of entries keyed with the given key
 */
int tl_ht_sizekey(tl_HASHTABLE *h, const void *k, size_t nkey);

/**
 * Convenient hash function for strings.
 *
 * @param k a null-terminated string key.
 *
 * @return a hash value for this string.
 */
int tl_ht_strhash(const void *k, size_t nkey);

/**
 * @}
 */

#endif /* GENHASH_H */
