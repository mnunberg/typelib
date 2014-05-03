#ifndef LCB_SLIST_H
#define LCB_SLIST_H

struct tl_SLNODE_s;
typedef struct tl_SLNODE_s {
    struct tl_SLNODE_s *next;
} tl_SLNODE;

typedef struct {
    tl_SLNODE *first;
    tl_SLNODE *last;
} tl_SLIST;

/**
 * Indicates whether the list is empty or not
 */
#define TL_SL_EMPTY(list) ((list)->last == NULL)

#define TL_SL_SINGLETON(list) ((list)->first && (list)->first == (list)->last)

typedef struct tl_SLITER {
    tl_SLNODE *cur;
    tl_SLNODE *prev;
    tl_SLNODE *next;
    int removed;
} tl_SLITER;

#define sllist_iter_end(list, iter) ((iter)->cur == NULL)

#define TL_SLITEM(ptr, type, member) \
        ((type *) ((char *)(ptr) - offsetof(type, member)))

#define TL_SL_FOREACH(list, iter) \
    for (slist_iter_init(list, iter); \
            !sllist_iter_end(list, iter); \
            slist_iter_incr(list, iter))

#define TL_SL_ITERBASIC(list, elem) \
        for (elem = (list)->first; elem; elem = elem->next)

#endif
