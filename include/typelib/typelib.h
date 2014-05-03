#ifndef LCB_TYPELIB_H
#define LCB_TYPELIB_H

/**
 * @file Various type utilities for libcouchbase.
 *
 * This primarily includes those chunks of code which are provided as simple
 * utilities for things lacking in the C language. These are present in C++
 * and most other higher level languages.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "tl_hashtable.h"
#include "tl_dlist.h"
#include "tl_slist.h"
#include "tl_nset.h"
#include "tl_string.h"

#ifdef __cplusplus
}
#endif
#endif
