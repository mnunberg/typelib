/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *     Copyright 2012 Couchbase, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#ifndef LIBCOUCHBASE_HASHSET_H
#define LIBCOUCHBASE_HASHSET_H 1

#ifdef __cplusplus
extern "C" {
#endif

    struct tl_SET {
        size_t nbits;
        size_t mask;

        size_t capacity;
        size_t *items;
        size_t nitems;
    };

    typedef struct tl_SET *tl_pNSET;

    /* create hashset instance */
    tl_pNSET tl_nset_new(void);

    /* destroy hashset instance */
    void tl_nset_free(tl_pNSET set);

    size_t tl_nset_count(tl_pNSET set);

    /**
     * Makes a list of items inside the hashset.
     * @param set the hashset
     * @param itemlist an allocated array large enough to hold the number
     * of items in the hashset (use hashset_num_items). If this is NULL
     * then a list is allocated for you
     *
     * @return the item list. If the @itemlist param argument was
     * not-null, then this is the same list; if it was NULL then this
     * is an allocated list which should be released using @itemlist
     * free(). It will return NULL if the hashset is empty or memory
     * allocation for @itemlist was failed.
     */
    void **tl_nset_items(tl_pNSET set, void **itemlist);


    /* add item into the hashset.
     *
     * @note 0 and 1 is special values, meaning nil and deleted items. the
     *       function will return -1 indicating error.
     *
     * returns zero if the item already in the set and non-zero otherwise
     */
    int tl_nset_add(tl_pNSET set, void *item);

    /* remove item from the hashset
     *
     * returns non-zero if the item was removed and zero if the item wasn't
     * exist
     */
    int tl_nset_del(tl_pNSET set, void *item);

    /* check if existence of the item
     *
     * returns non-zero if the item exists and zero otherwise
     */
    int tl_nset_contains(tl_pNSET set, void *item);

#ifdef __cplusplus
}
#endif

#endif
