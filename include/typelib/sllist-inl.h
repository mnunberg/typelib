#include "sllist.h"
#include <stdlib.h>
#include <assert.h>

#ifndef INLINE
#ifdef _MSC_VER
#define INLINE __inline
#elif __GNUC__
#define INLINE __inline__
#else
#define INLINE inline
#endif /* MSC_VER */
#endif /* !INLINE */


static INLINE int
sllist_contains(tl_SLIST *list, tl_SLNODE *item)
{
    tl_SLNODE *ll;
    TL_SL_ITERBASIC(list, ll) {
        if (item == ll) {
            return 1;
        }
    }
    return 0;
}

static INLINE unsigned
sllist_get_size(tl_SLIST *list)
{
    unsigned ret = 0;
    tl_SLNODE *ll;
    TL_SL_ITERBASIC(list, ll) {
        ret++;
    }
    return ret;
}

/* #define SLLIST_DEBUG */

#ifdef SLLIST_DEBUG
#define slist_sanity_insert(l, n) assert(!slist_contains(l, n))
#else
#define slist_sanity_insert(l, n)
#endif

static INLINE void
slist_iter_init_at(tl_SLNODE *node, tl_SLITER *iter)
{
    iter->cur = node->next;
    iter->prev = node;
    iter->removed = 0;

    if (iter->cur) {
        iter->next = iter->cur->next;
    } else {
        iter->next = NULL;
    }
}

static INLINE void
slist_iter_init(const tl_SLIST *list, tl_SLITER *iter)
{
    slist_iter_init_at((tl_SLNODE *)&list->first, iter);
}

static INLINE void
slist_iter_incr(tl_SLIST *list, tl_SLITER *iter)
{
    if (!iter->removed) {
        iter->prev = iter->prev->next;
    } else {
        iter->removed = 0;
    }

    if ((iter->cur = iter->next)) {
        iter->next = iter->cur->next;
    } else {
        iter->next = NULL;
    }

    assert(iter->cur != iter->prev);

    (void)list;
}

static INLINE void
sllist_iter_remove(tl_SLIST *list, tl_SLITER *iter)
{
    iter->prev->next = iter->next;

    /** GCC strict aliasing. Yay. */
    if (iter->prev->next == NULL && (void *)iter->prev == (void *)&list->first) {
        list->last = NULL;
    } else if (iter->cur == list->last && iter->next == NULL) {
        /* removing the last item */
        list->last = iter->prev;
    }
    iter->removed = 1;
}

static INLINE void
sllist_remove_head(tl_SLIST *list)
{
    if (!list->first) {
        return;
    }

    list->first = list->first->next;

    if (!list->first) {
        list->last = NULL;
    }
}

static INLINE void
sllist_remove(tl_SLIST *list, tl_SLNODE *item)
{
    tl_SLITER iter;
    TL_SL_FOREACH(list, &iter) {
        if (iter.cur == item) {
            sllist_iter_remove(list, &iter);
            return;
        }
    }
    abort();
}

static INLINE void
sllist_append(tl_SLIST *list, tl_SLNODE *item)
{
    if (TL_SL_EMPTY(list)) {
        list->first = list->last = item;
        item->next = NULL;
    } else {
        slist_sanity_insert(list, item);
        list->last->next = item;
        list->last = item;
    }
    item->next = NULL;
}

static INLINE void
sllist_prepend(tl_SLIST *list, tl_SLNODE *item)
{
    if (TL_SL_EMPTY(list)) {
        list->first = list->last = item;
    } else {
        slist_sanity_insert(list, item);
        item->next = list->first;
        list->first = item;
    }
}

static void
sllist_insert(tl_SLIST *list, tl_SLNODE *prev, tl_SLNODE *item)
{
    item->next = prev->next;
    prev->next = item;
    if (item->next == NULL) {
        list->last = item;
    }
}

static INLINE void
sllist_insert_sorted(tl_SLIST *list, tl_SLNODE *item,
                     int (*compar)(tl_SLNODE*, tl_SLNODE*))
{
    tl_SLITER iter;
    TL_SL_FOREACH(list, &iter) {
        int rv = compar(item, iter.cur);
        /** if the item we have is before the current, prepend it here */
        if (rv <= 0) {
            sllist_insert(list, iter.prev, item);
            return;
        }
    }
    sllist_append(list, item);
}
