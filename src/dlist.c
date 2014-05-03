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

#include "tl_dlist.h"

void tl_dlist_init(tl_DLIST *list)
{
    list->base.prev = list->base.next = &list->base;
    list->size = 0;
}

static void
list_insert(tl_DLISTNODE *prev, tl_DLISTNODE *next, tl_DLISTNODE *item)
{
    item->next = next;
    item->prev = prev;
    prev->next = item;
    next->prev = item;
}

static void
list_eject(tl_DLISTNODE *prev, tl_DLISTNODE *next)
{
    next->prev = prev;
    prev->next = next;
}


void tl_dlist_prepend(tl_DLIST *list, tl_DLISTNODE *item)
{
    list_insert(&list->base, list->base.next, item);
    list->size++;
}

void tl_dlist_append(tl_DLIST *list, tl_DLISTNODE *item)
{
    list_insert(list->base.prev, &list->base, item);
    list->size++;
}


void tl_dlist_delete(tl_DLIST *list, tl_DLISTNODE *item)
{
    list_eject(item->prev, item->next);
    item->next = item->prev = NULL;
    list->size--;
}


tl_DLISTNODE *tl_dlist_shift(tl_DLIST *list)
{
    tl_DLISTNODE *item;

    if (TL_DLIST_EMPTY(list)) {
        return NULL;
    }
    item = list->base.next;
    tl_dlist_delete(list, item);
    return item;
}

tl_DLISTNODE *tl_dlist_pop(tl_DLIST *list)
{
    tl_DLISTNODE *item;

    if (TL_DLIST_EMPTY(list)) {
        return NULL;
    }
    item = list->base.prev;
    tl_dlist_delete(list, item);
    return item;
}

int tl_dlist_contains(tl_DLIST *list, tl_DLISTNODE *item)
{
    tl_DLISTNODE *ptr = list->base.next;

    while (ptr != &list->base && ptr != item) {
        ptr = ptr->next;
    }

    return (ptr == item) ? 1 : 0;
}

void tl_dlist_add_sorted(tl_DLIST *list, tl_DLISTNODE *item, lcb_list_cmp_fn cmp)
{
    tl_DLISTNODE *p;

    if (TL_DLIST_EMPTY(list)) {
        list_insert(list->base.prev, &list->base, item);
    } else {
        TL_DLIST_FOR(p, list) {
            if (cmp(item, p) < 0) {
                break;
            }
        }
        list_insert(p->prev, p, item);
    }
}
