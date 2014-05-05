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

#ifndef LCB_STRING_H
#define LCB_STRING_H

#include <stddef.h>
#include <stdarg.h>

/**
 * Simple string type.
 *
 * This structure is designed mainly for ease of use when dealing with actual
 * "string" data - i.e. data which must be null-terminated and contiguous.
 *
 * This won't replace the ringbuffer structure as this string's removal and
 * copying operations are comparatively expensive.
 *
 * Note that all API functions which update the position of the buffer ALSO
 * add a trailing NUL byte at the end.
 */
typedef struct {
    /** Buffer that's allocated */
    char *base;

    /** Number of bytes allocated */
    size_t nalloc;

    /** Number of bytes used */
    size_t nused;
} tl_STRING;

#ifdef __cplusplus
extern "C" {
#endif

int tl_str_init(tl_STRING *str);

/**
 * Free any storage associated with the string. The string's state will be
 * empty as if string_init() had just been called.
 */
void tl_str_cleanup(tl_STRING *str);

/**
 * Clear the contents of the string, but don't free the underlying buffer
 */
void tl_str_clear(tl_STRING *str);


/**
 * Indicate that bytes have been added to the string. This is used in conjunction
 * with reserve(). As such, the number of bytes added should not exceed the
 * number of bytes passed to reserver.
 *
 * @param str the string
 * @param nbytes the number of bytes added
 */
void tl_str_added(tl_STRING *str, size_t nbytes);

/**
 * Reserve an amount of free bytes within the string. When this is done,
 * up to @c size bytes may be added to the string starting at @c base+str->nbytes
 */
int tl_str_reserve(tl_STRING *str, size_t size);


/**
 * Adds data to the string.
 * @param data the data to copy
 * @param size the size of data to copy
 */
int tl_str_append(tl_STRING *str, const void *data, size_t size);

/**
 * Adds a C-style string
 * @param str the target lcb_string
 * @param zstr a NUL-terminated string to add
 */
int tl_str_appendz(tl_STRING *str, const char *zstr);

/**
 * Appends to the string using printf-style formatting
 * @param str
 * @param fmt The format to use
 * @param ... format arguments
 * @return 0 if successful, -1 on error
 */
int tl_str_appendf(tl_STRING *str, const char *fmt, ...);

int tl_str_appendv(tl_STRING *str, const char *fmt, va_list ap);

/**
 * Removes bytes from the end of the string. The resultant string will be
 * NUL-terminated
 * @param str the string to operate on
 * @param to_remove the number of bytes to trim from the end
 */
void tl_str_erase_begin(tl_STRING *str, size_t to_remove);


/**
 * Removes bytes from the beginning of the string. The resultant string will
 * be NUL-terminated.
 * @param str the string to operate on
 * @param to_remove the number of bytes to remove from the beginning of
 * the string.
 */
void tl_str_erase_end(tl_STRING *str, size_t to_remove);

/**
 * Transfers ownership of the underlying buffer contained within the structure
 * 'to' to the structure 'from', as such, 'from' becomes a newly initialized
 * empty string structure and 'to' contains the existing buffer.
 *
 * @param from the string which contains the existing buffer
 * @param to a new string structure which contains no buffer. It will receive
 * from's buffer
 */
void tl_str_transfer(tl_STRING *from, tl_STRING *to);

/**
 * Substitute all occurrences of 'orig' with 'repl'
 * @param str
 * @param orig string to search for
 * @param repl replacement string
 * @return 0 on success, -1 on allocation failure
 */
int tl_str_substz(tl_STRING *str, const char *orig, const char *repl);

/**
 * String substitution
 * @param str
 * @param orig String to search for
 * @param norig Size of string (-1 if NUL-terminated)
 * @param repl Replacement string
 * @param nrepl Size of replacement (-1 if NUL terminated)
 * @return 0 on success, -1 on allocation failure.
 */
int tl_str_subst(tl_STRING *str, const char *orig, int norig,
                 const char *repl, int nrepl);

#define tl_str_tail(str) ((str)->base + (str)->nused)

/** Utility functions. These functions wrap existing functionality */

/**
 * Functions like glibc's asprintf
 * @param[out] strp
 * @param fmt
 * @return
 */
int tl_asprintf(char **strp, const char *fmt, ...);
char *tl_strndup(const char *s, unsigned n);

/** Detach the pointer so that it points to standalone memory */
#define TL_STRSPLIT_DETACH 1

/** Replace the terminator with a NULL */
#define TL_STRSPLIT_ZREPLACE 2

/** @brief Simple structure describing a string and its length */
typedef struct {
    char *buf; /**< string*/
    unsigned length; /**< length of string*/
} tl_STRLOC;

/**
 * @brief Split a string based on delimiters.
 *
 * @param s The string to split
 * @param delim The delimited to split on
 * @param[in,out] loc Pointer to an array of tl_STRLOC objects
 * @param[in,out] nloc Pointer to a size of the array
 * @param options Options for string allocation/manipulation
 * @return 0 on success, nonzero on failure.
 *
 * This function splits a string based on a single n-character token. The
 * strings between the token are extracted and stored in the `uloc` array.
 *
 * If the `nloc` parameter on invocation is _not_ 0 then it is assumed that
 * `loc` points to a user-allocated array and the underlying storage will
 * not be modified. In this case if there are insufficient elements in the
 * array the function will return `-1` and `*nloc` will be set to -1 to indicate
 * that more elements are needed. Otherwise `*loc` will be dynamically allocated
 * (using malloc()) to contain as many tl_STRLOC elements as required. If
 * `*loc` is not allocated by the caller, it must be released with free() when
 * done.
 *
 * If `options` is 0 then each element of `*loc` will be initialized so that
 * tl_STRLOC::buf points to an offset into `s`. `s` will not be modified and
 * thus tl_STRLOC::buf may not be `NUL`-terminated. If options is
 * `TL_STRSPLIT_ZREPLACE` then each tl_STRLOC::buf pointer will be
 * `NUL`-terminated (with each occurrence of `delim` having its first character
 * set to `\0`. If options is `TL_STRSPLIT_DETACH` then each tl_STRLOC::buf will
 * contain an individually allocated string (via malloc()) which will be
 * `NUL`-terminated, and `s` will remain untouched. If `TL_STRSPLIT_DETACH` is
 * specified then the caller must free each tl_STRLOC::buf pointer using free()
 * when it is no longer required.
 *
 *
 * In all cases, the current value of `nloc` will be considered to be valid and
 * the valid contents of `loc` will not overrun `nloc`.
 */
int tl_strsplit(char *s, const char *delim, tl_STRLOC **loc, int *nloc, int options);

#ifdef __cplusplus
}
#endif /** __cplusplus */
#endif /* LCB_STRING_H */
