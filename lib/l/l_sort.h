
/* $Id: l_sort.h 21520 2017-07-22 15:09:04Z kobus $ */

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

#ifndef L_SORT_INCLUDED
#define L_SORT_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define USE_CURRENT_ATN_HANDLING       -10
#define USE_SORT_ATN_HANDLING          -11
#define DISABLE_ATN_HANDLING           -12

#define EQUAL_ITEMS            0
#define FIRST_ITEM_GREATER     1
#define SECOND_ITEM_GREATER    (-1)


int  kjb_sort                
(
    void*  array,
    int    num_elements,
    size_t element_size,
    int    (*cmp_fn) (const void*, const void*),
    int    interrupt_action 
);

int check_sort
(
    void*  array,
    int    num_elements,
    size_t element_size,
    int    (*cmp_fn) (const void *, const void *) 
);

int  int_sort                
(
    void*  array,
    int    num_elements,
    size_t element_size,
    size_t key_pos,
    int    interrupt_action 
);

long long_sort               
(
    void*  array,
    long   num_elements,
    size_t element_size,
    size_t key_pos,
    int    interrupt_action 
);

int  binary_search           
(
    const void* array,
    int         num_elements,
    size_t      element_size,
    int         (*cmp_fn) (const void*, const void*),
    const void* search_key_ptr 
);

int  int_binary_search       
(
    const void* array,
    int         num_elements,
    size_t      element_size,
    size_t      element_offset,
    int         search_key 
);

long long_binary_search      
(
    const void* array,
    long        num_elements,
    size_t      element_size,
    size_t      element_offset,
    long        search_key 
);

int  binary_search_int_array 
(
    const int* array,
    int        num_elements,
    int        search_key 
);

long binary_search_long_array
(
    const long* array,
    long        num_elements,
    long        search_key 
);

int  linear_search           
(
    const void* array,
    int         num_elements,
    size_t      element_size,
    int         (*cmp_fn) (const void*, const void*),
    const void* search_key_ptr 
);

int  int_linear_search       
(
    const void* array,
    int         num_elements,
    size_t      element_size,
    size_t      element_offset,
    int         search_key 
);

long long_linear_search      
(
    const void* array,
    long        num_elements,
    size_t      element_size,
    size_t      element_offset,
    long        search_key 
);

int  linear_search_int_array 
(
    const int* array,
    int        num_elements,
    int        search_elem 
);

long linear_search_long_array
(
    const long* array,
    long        num_elements,
    long        search_elem 
);

int  insert_into_sorted_array
(
    void*  array,
    int*   num_elements_ptr,
    size_t element_size,
    int    (*cmp_fn)(const void*, const void*),
    void*  new_element_ptr 
);

int get_last_sort_comparison_count(void);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

