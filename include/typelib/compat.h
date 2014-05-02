#include "typelib.h"

/** Define definitions for all types */
#ifdef __cplusplus
extern "C" {
#endif

/** hashset */
typedef tl_pNSET hashset_t;
#define hashset_create tl_nset_new
#define hashset_destroy tl_nset_free
#define hashset_add tl_nset_add
#define hashset_is_member tl_nset_contains
#define hashset_remove tl_nset_del
#define hashset_num_items tl_nset_count
#define hashset_get_items tl_nset_items

/** string */
typedef tl_STRING lcb_string;
#define lcb_string_init tl_str_init
#define lcb_string_append tl_str_append
#define lcb_string_appendz tl_str_appendz
#define lcb_string_erase_beginning tl_str_erase_begin
#define lcb_string_erase_end tl_str_erase_end
#define lcb_string_clear tl_str_clear
#define lcb_string_release tl_str_cleanup
#define lcb_string_reserve tl_str_reserve
#define lcb_string_tail tl_str_tail
#define lcb_string_added tl_str_added

/** sllist */
typedef tl_SLNODE sllist_node;
typedef tl_SLIST sllist_root;
typedef tl_SLITER sllist_iterator;

#define SLLIST_IS_EMPTY TL_SL_EMPTY
#define SLLIST_ITEM TL_SLITEM
#define SLLIST_ITERBASIC TL_SL_ITERBASIC
#define SLLIST_ITERFOR TL_SL_FOREACH
#define SLLIST_FOREACH TL_SL_ITERBASIC


#ifdef __cplusplus
}
#endif
