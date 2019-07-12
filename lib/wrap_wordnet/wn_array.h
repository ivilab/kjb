
/* @id */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
* =========================================================================== */

#ifndef _INCLUDED_ARRAY_H_
#define _INCLUDED_ARRAY_H_

#include "i/i_type.h" 

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

typedef struct Array 
{
    void **elements;
    int  length;
}
Array; 

Array *create_array
(
    int length
);

void free_array
(
    Array  *array_ptr,
    void   (*free_element_fn)(void*)
);

void free_array_array
(
    Array  *array_array_ptr,
    void   (*free_array_fn)(void*)
);

int search_array
(
    const Array       *array_ptr,
    int              (*key_comp_fn) (const void*, const void*),
    const void       *key_ptr
);

int merge_arrays
(
    const Array *a_ptr,
    const Array *b_ptr,
    Array       **c_ptr_ptr
);

void print_array
(
    const Array * array_ptr, 
    void (*print_fn) (const void *)
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}

#endif

#endif
