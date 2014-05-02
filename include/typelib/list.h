/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *     Copyright 2013 Couchbase, Inc.
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

#ifndef LIBCOUCHBASE_LIST_H
#define LIBCOUCHBASE_LIST_H 1
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct tl_DLISTNODE_s tl_DLISTNODE;
struct tl_DLISTNODE_s {
    tl_DLISTNODE *next;
    tl_DLISTNODE *prev;
};

typedef struct {
    tl_DLISTNODE base;
    size_t size;
} tl_DLIST;

typedef int (*lcb_list_cmp_fn)(tl_DLISTNODE *a, tl_DLISTNODE *b);

void tl_dlist_init(tl_DLIST *list);
void tl_dlist_prepend(tl_DLIST *list, tl_DLISTNODE *item);
void tl_dlist_append(tl_DLIST *list, tl_DLISTNODE *item);
void tl_dlist_delete(tl_DLIST *list, tl_DLISTNODE *item);
tl_DLISTNODE *tl_dlist_shift(tl_DLIST *list);
tl_DLISTNODE *tl_dlist_pop(tl_DLIST *list);
int tl_dlist_contains(tl_DLIST *list, tl_DLISTNODE *item);
void tl_dlist_add_sorted(tl_DLIST *list, tl_DLISTNODE *item, lcb_list_cmp_fn cmp);

#define TL_DLIST_EMPTY(list) \
    (&(list)->base == (list)->base.next && &(list)->base == (list)->base.prev)

#define TL_DLIST_ITEM(ptr, type, member) \
    ((type *) ((char *)(ptr) - offsetof(type, member)))

#define TL_DLIST_FOR(pos, list) \
    for (pos = (list)->base.next; pos != &(list)->base; pos = pos->next)


#define TL_DLIST_SAFE_FOR(pos, n, list) \
    for (pos = (list)->base.next, n = pos->next; pos != &(list)->base; pos = n, n = pos->next)

#define TL_DLIST_HAS_NEXT(ll, item) \
    ((item)->next != ll)

#define TL_DLIST_TAIL(list) \
    ((TL_DLIST_IS_EMPTY(list)) ? NULL : (list)->base.prev)

#define TL_DLIST_HEAD(list) \
    ((TL_DLIST_IS_EMPTY(list)) ? NULL : (list)->base.next)

#ifdef __cplusplus
}
#endif
#endif
