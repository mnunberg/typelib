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

/**
 * @file
 * Double-linked list.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Element type to be embedded into the object to be stored within the list
 */
typedef struct tl_DLISTNODE_s tl_DLISTNODE;
struct tl_DLISTNODE_s {
    tl_DLISTNODE *next;
    tl_DLISTNODE *prev;
};

/**
 * List structure itself. This consists of the root element as well as the
 * size of the list.
 */
typedef struct {
    tl_DLISTNODE base;
    size_t size;
} tl_DLIST;

typedef int (*lcb_list_cmp_fn)(tl_DLISTNODE *a, tl_DLISTNODE *b);

/**
 * @brief Initialize the List
 * @param list
 */
void tl_dlist_init(tl_DLIST *list);

/**
 * @brief Prepend an item to the list
 * @param list
 * @param item
 */
void tl_dlist_prepend(tl_DLIST *list, tl_DLISTNODE *item);

/**
 * @brief Append an item to the list
 * @param list
 * @param item
 */
void tl_dlist_append(tl_DLIST *list, tl_DLISTNODE *item);

/**
 * @brief Remove an item from the list
 * @param list
 * @param item Item to remove. If the item is not a member of the list the
 * behavior is undefined.
 */
void tl_dlist_delete(tl_DLIST *list, tl_DLISTNODE *item);

/**
 * @brief Remove and return the first item in the list
 * @param list
 * @return The item, or NULL if the list is empty
 */
tl_DLISTNODE *tl_dlist_shift(tl_DLIST *list);

/**
 * @brief Remove and return the last item in the list
 * @param list
 * @return The item or NULL if the list is empty
 */
tl_DLISTNODE *tl_dlist_pop(tl_DLIST *list);

/**
 * @brief Search the list to determine if `item` is a member
 * @param list
 * @param item
 * @return nonzero if `item` is in the list, zero if the item is not present
 */
int tl_dlist_contains(tl_DLIST *list, tl_DLISTNODE *item);

/**
 * @brief Add an item to the list, ensuring sort order
 * @param list
 * @param item The item to add
 * @param cmp Comparison function to determine ordering
 *
 * @note This function must be used in all insertion operations on the list in
 * order for the list to remain properly valid
 */
void tl_dlist_add_sorted(tl_DLIST *list, tl_DLISTNODE *item, lcb_list_cmp_fn cmp);

/**
 * @brief Determine if the list is emtpy
 * @return nonzero if the list is empty
 */
#define TL_DLIST_EMPTY(list) \
    (&(list)->base == (list)->base.next && &(list)->base == (list)->base.prev)

/**
 * @brief Cast a list item to its underlying type
 * @param ptr a pointer to a tl_DLISTNODE
 * @param type the underlying type to cast to.
 * @param member name of the member of `type` which is the tl_DLISTNODE
 * @return a pointer to the underlying type
 * @note `ptr` must not be NULL
 */
#define TL_DLIST_ITEM(ptr, type, member) \
    ((type *) ((char *)(ptr) - offsetof(type, member)))


/**
 * @brief Iterate through all members of the list
 * @param pos a local tl_DLISTNODE to represent the current item of the list
 * @param list the list to iterate over
 *
 * @note While traversing the list, the list may not be modified. See
 *       @ref TL_DLIST_SAFE_FOR for an iteration format which allows modification
 *
 * Example:
 * @code{.c}
 * typedef struct {
 *   char *data;
 *   tl_DLISTNODE llnode;
 * } mytype;
 *
 * tl_DLISTNODE *cur;
 * TL_DLIST_FOR(cur, &list) {
 *   mytype *p = TL_DLIST_ITEM(p, mytype, llnode);
 *   // ...
 * }
 * @endcode
 */
#define TL_DLIST_FOR(pos, list) \
    for (pos = (list)->base.next; pos != &(list)->base; pos = pos->next)

/**
 * @brief Iterate through all members of the list, allowing modification of the list
 *
 * @param pos a local variable to represent the current item of the list
 * @param n a local variable to represent the next item in the list
 * @param list the list to iterate over.
 *
 * Example:
 * @code{.c}
 * typedef struct {
 *   char *data;
 *   tl_DLISTNODE llnode;
 * } my_type;
 *
 * tl_DLISTNODE *llcur, *llnext;
 * TL_DLIST_SAFE_FOR(llcur, llnext, &list) {
 *   tl_dlist_delete(&list, llcur);
 * }
 * @endcode
 *
 */
#define TL_DLIST_SAFE_FOR(pos, n, list) \
    for (pos = (list)->base.next, n = pos->next; pos != &(list)->base; pos = n, n = pos->next)

/**
 * @brief Determine if a given list element has a following element
 * @param ll the list of which the element is a member
 * @param item the element
 * @return nonzero if the item has a next element
 *
 * Example:
 * @code{.c}
 * tl_DLISTNODE *cur = TL_DLIST_HEAD(&list);
 * if (!cur) {
 *   return;
 * }
 * while (cur && TL_DLIST_HAS_NEXT(&list, cur)) {
 *   // ...
 *   cur = cur->next;
 * }
 * @endcode
 */
#define TL_DLIST_HAS_NEXT(ll, item) \
    ((item)->next != ll)

/**
 * @brief return a pointer to the last item in the list
 * @param list the list to check
 * @return a pointer to the last element, or NULL if the list is empty
 */
#define TL_DLIST_TAIL(list) \
    ((TL_DLIST_IS_EMPTY(list)) ? NULL : (list)->base.prev)

/**
 * @brief Return a pointer to the first item in the list
 * @param list the list to check
 * @return a pointer to the first element, or NULL if the list is empty.
 */
#define TL_DLIST_HEAD(list) \
    ((TL_DLIST_IS_EMPTY(list)) ? NULL : (list)->base.next)

#ifdef __cplusplus
}
#endif
#endif
