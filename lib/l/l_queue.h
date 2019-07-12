
/* $Id: l_queue.h 15908 2013-10-26 20:23:11Z kobus $ */

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

#ifndef L_QUEUE_INCLUDED
#define L_QUEUE_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define FIRST_KEY_IS_GREATER          1
#define SECOND_KEY_IS_GREATER         (-1)
#define EQUAL_KEYS                    0
#define SELECT_ELEMENT                0

#define NOT_INSERTED     NOT_FOUND

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                                   Queue_element
 *
 * Linked list type
 *
 * This type is linked list type for the KJB library.
 *
 * Index: queues, stacks, linked lists
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Queue_element
{
    struct Queue_element* next;     /* Pointer to next element */
    struct Queue_element* previous; /* Pointer to previous element */
    void*                 contents; /* Pointer to anything */
}
Queue_element;

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_queue
(
    Queue_element** head_ptr_ptr,
    Queue_element** tail_ptr_ptr,
    void            (*free_element_fn)(void*)
);

void free_queue_element
(
    Queue_element* element_ptr,
    void           (*free_element_fn)(void*)
);

#ifdef TRACK_MEMORY_ALLOCATION

#   define insert_into_queue(x,y, z) \
                debug_insert_into_queue(x, y, z, __FILE__, __LINE__)

#   define insert_at_end_of_queue(x, y, z) \
                debug_insert_at_end_of_queue(x, y, z, __FILE__, __LINE__)

#   define alloc_insert_into_queue(w, x, y, z) \
                debug_alloc_insert_into_queue(w, x, y, z, __FILE__, __LINE__)

#   define alloc_insert_at_end_of_queue(w, x, y, z) \
                debug_alloc_insert_at_end_of_queue(w, x, y, z,                 \
                                                   __FILE__, __LINE__)


    int debug_insert_into_queue
    (
        Queue_element** head_ptr_ptr,
        Queue_element** tail_ptr_ptr,
        void*           contents_ptr,
        const char*     file_name,
        int             line_number
    );


    int debug_insert_at_end_of_queue
    (
        Queue_element** head_ptr_ptr,
        Queue_element** tail_ptr_ptr,
        void*           contents_ptr,
        const char*     file_name,
        int             line_number
    );

    int debug_alloc_insert_into_queue
    (
        Queue_element** ,
        Queue_element** ,
        void*           ,
        long            ,
        const char*     file_name,
        int             line_number
    );

    int debug_alloc_insert_at_end_of_queue
    (
        Queue_element** ,
        Queue_element** ,
        void*           ,
        long            ,
        const char*     file_name,
        int             line_number
    );

#else
    int insert_into_queue(Queue_element**, Queue_element**, void*);

    int insert_at_end_of_queue
    (
        Queue_element** ,
        Queue_element** ,
        void*
    );

    int alloc_insert_into_queue
    (
        Queue_element** ,
        Queue_element** ,
        void*           ,
        long
    );

    int alloc_insert_at_end_of_queue
    (
        Queue_element** ,
        Queue_element** ,
        void*           ,
        long
    );

#endif


int insert_into_ordered_queue
(
    Queue_element** ,
    Queue_element** ,
    void*           ,
    int             (*)(const void* , const void*),
    int
);

Queue_element* remove_first_selected_element
(
    Queue_element** ,
    Queue_element** ,
    int             (*)(const void* , const void*),
    void*
);

Queue_element* remove_first_element(Queue_element**, Queue_element**);

Queue_element* remove_last_element(Queue_element**, Queue_element**);

int cat_queue_copy
(
    Queue_element** ,
    Queue_element** ,
    Queue_element*  ,
    long
);

int cat_str_queue_copy
(
    Queue_element** ,
    Queue_element** ,
    Queue_element*
);

int copy_queue
(
    Queue_element** ,
    Queue_element** ,
    Queue_element*  ,
    long
);

int copy_str_queue(Queue_element**, Queue_element**, Queue_element*);

int free_selected_elements
(
    Queue_element** ,
    Queue_element** ,
    int             (*)(void* , void*),
    void*           ,
    void            (*free_fn)(void*)
);

void* search_queue
(
    Queue_element* ,
    int            (*)(const void*, const void*),
    void*
);

Queue_element* remove_elements_less_than_key
(
    Queue_element** ,
    Queue_element** ,
    int             (*)(const void* , const void*),
    const void*
);

int is_queue_ordered(Queue_element* head_ptr, int (*cmp_fn) (const void *, const void *));

int apply_to_queue(Queue_element*, void (*)(void*));

int apply_and_free
(
    Queue_element* head_ptr,
    void           (*apply_fn)(void*),
    void           (*free_element_fn)(void*)
);

int count_queue_elements(Queue_element*);
int print_str_queue(Queue_element*);

int print_queue(Queue_element* head_ptr, int (*print_fn) (const void*));


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

