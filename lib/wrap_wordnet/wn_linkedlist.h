
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

#ifndef WN_LINKED_LIST
#define WN_LINKED_LIST

#include "i/i_type.h" 
#include "wrap_wordnet/wn_array.h"


#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

typedef struct List_element 
{
    struct List_element* next;     /* Pointer to next element */
    struct List_element* prev; /* Pointer to previous element */
    void*                 contents; /* Pointer to anything */
}
List_element; 

typedef struct Linkedlist
{
    int length;
    List_element *head;
    List_element *tail;
} Linkedlist;

Linkedlist *create_linkedlist
(
    void
);

void free_linkedlist
(
    Linkedlist *list_ptr,
    void      (*free_element_fn)(void*)
);

int insert_head_element
(
    Linkedlist *list_ptr,
    List_element *element
);

int insert_head_element_with_contents
(
    Linkedlist *list_ptr,
    void       *contents
);

int append_element
(
    Linkedlist   *list_ptr,
    List_element *element
);

int append_element_with_contents
(
    Linkedlist *list_ptr,
    void *contents
);

void *remove_list_head
(
    Linkedlist *list_ptr
);

void *remove_list_element
(
    Linkedlist *list_ptr,
    const void *element
);

void *search_linkedlist
(
    const Linkedlist *list_ptr,
    int              (*key_comp_fn) (const void*, const void*),
    const void       *key_ptr
);

int from_list_to_array
(
    const Linkedlist *list_ptr,
    Array            **array_ptr_ptr
);

void print_linkedlist
(
    const Linkedlist * list_ptr, 
    void (*print_fn) (const void *)
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
