# TYPELIB - Simple Types for C

This library is an extraction of various types made to cater for the
[libcouchbase](https://github.com/couchbase/libcouchbase.git) project.

It features some types which are generic and applicable to most use cases with
a reasonable performance profile. It is considered an alternative to something
more heavy like GLib or similar.

## Design Goals

* Individual containers/files should be embeddable without additional includes
  (except its self-contained include)

* Should be portable to C99 and 'C89+' compilers.

* Should provide a sufficiently generic API to fit the common use case


## Contents

* *tl_HASHTABLE* - a Hash Table
* *tl_STRING* - a dynamically expanding string type
* *tl_DLIST* - a doubly-linked intrusive list
* *tl_SLIST* - a singly-linked intrusive list
* *tl_NSET* - unique set of integers

## Using

To use you may either

* `#include <typelib/typelib.h>`
* Copy the individual `.c` and `.h` file(s) into your project
