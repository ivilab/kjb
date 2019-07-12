
/* $Id: l_queue.c 15908 2013-10-26 20:23:11Z kobus $ */

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

#include "l/l_gen.h"     /* Only safe as first include in a ".c" file. */
#include "l/l_string.h"
#include "l/l_queue.h"


/*
    Procedures for queue manipulation. Some are for random queues,
    others pertain to ones ordered by a user supplied function.
*/

/*
// Warning : The code for backward links (IE the "previous" field) has not been
// systematically tested yet.
*/

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                              free_queue
 *
 * Frees a queue
 *
 * This routine is used to dispose of the queue pointed to by *head_ptr_ptr. The
 * tail pointer is optional for queues. If there is one, then it should be
 * passed to this routine; otherwise tail_ptr_ptr should be NULL. The queue
 * elements are freed using the function (*free_contents_fn). If
 * free_contents_fn is NULL, then kjb_free is used. Once the queue is freed,
 * *head_ptr_ptr, (and *tail_ptr_ptr if applicable) are set to NULL
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/
void free_queue(Queue_element** head_ptr_ptr, Queue_element** tail_ptr_ptr,
                void (*free_contents_fn) (void * ))
{
    Queue_element* current;
    Queue_element* previous;


    if (head_ptr_ptr == NULL) return;

    if (free_contents_fn == NULL) free_contents_fn = kjb_free;

    current = *head_ptr_ptr;

    while (current != NULL)
    {
        previous = current;
        current = current->next;
        (*free_contents_fn)(previous->contents);
        kjb_free(previous);
    }

    *head_ptr_ptr = NULL;

    if (tail_ptr_ptr != NULL) *tail_ptr_ptr = NULL;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              free_queue_element
 *
 *
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

void free_queue_element(Queue_element* element_ptr,
                        void (*free_contents_fn) (void * ))
{


    if (element_ptr == NULL) return;

    (*free_contents_fn)(element_ptr->contents);
    kjb_free(element_ptr);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            alloc_insert_into_queue
 *
 *
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

int debug_alloc_insert_into_queue(Queue_element** head_ptr_ptr,
                                  Queue_element** tail_ptr_ptr, void* data_ptr,
                                  long int data_size, const char* file_name,
                                  int line_number)
{
    char* contents_ptr;


    NRE(contents_ptr = DEBUG_STR_MALLOC(data_size, file_name, line_number));

    kjb_memcpy(contents_ptr, data_ptr, data_size);
    debug_insert_into_queue(head_ptr_ptr, tail_ptr_ptr, (void*)contents_ptr,
                            file_name, line_number);

    return NO_ERROR;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int alloc_insert_into_queue(Queue_element** head_ptr_ptr,
                            Queue_element** tail_ptr_ptr, void* data_ptr,
                            long data_size)
{
    char* contents_ptr;


    NRE(contents_ptr = STR_MALLOC(data_size));

    kjb_memcpy(contents_ptr, data_ptr, data_size);
    insert_into_queue(head_ptr_ptr, tail_ptr_ptr, (void*)contents_ptr);

    return NO_ERROR;
}

#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                    alloc_insert_at_end_of_queue
 *
 *
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

int debug_alloc_insert_at_end_of_queue(Queue_element** head_ptr_ptr,
                                       Queue_element** tail_ptr_ptr,
                                       void* data_ptr, long int data_size,
                                       const char* file_name, int line_number)
{
    char* contents_ptr;


    if (    (data_size <= 0)
         || (head_ptr_ptr == NULL)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    NRE(contents_ptr = DEBUG_STR_MALLOC(data_size, file_name, line_number));
    kjb_memcpy(contents_ptr, data_ptr, data_size);

    ERE(debug_insert_at_end_of_queue(head_ptr_ptr, tail_ptr_ptr,
                                     (void*) contents_ptr, file_name,
                                     line_number));

    return NO_ERROR;
}


        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int alloc_insert_at_end_of_queue(Queue_element** head_ptr_ptr,
                                 Queue_element** tail_ptr_ptr,
                                 void* data_ptr, long int data_size)
{
    char* contents_ptr;


    if (    (data_size <= 0)
         || (head_ptr_ptr == NULL)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    NRE(contents_ptr = STR_MALLOC(data_size));
    kjb_memcpy(contents_ptr, data_ptr, data_size);

    ERE(insert_at_end_of_queue(head_ptr_ptr, tail_ptr_ptr,
                               (void*) contents_ptr));

    return NO_ERROR;
}


#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       insert_into_queue
 *
 * Inserts an element at the front of the queue.
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

int debug_insert_into_queue(Queue_element** head_ptr_ptr,
                            Queue_element** tail_ptr_ptr,
                            void* contents_ptr,
                            const char* file_name, int line_number)
{
    Queue_element* save_head_ptr;


    save_head_ptr = *head_ptr_ptr;
    NRE(*head_ptr_ptr = DEBUG_TYPE_MALLOC(Queue_element, file_name,
                                          line_number));
    (*head_ptr_ptr)->contents = contents_ptr;
    (*head_ptr_ptr)->next = save_head_ptr;
    (*head_ptr_ptr)->previous = NULL;

    if (save_head_ptr != NULL)
    {
        /* Simple growth */
        save_head_ptr->previous = *head_ptr_ptr;
    }
    else if (tail_ptr_ptr != NULL)
    {
        /* Head comming in (save_head_ptr) is NULL, so we are initializng. */
        *tail_ptr_ptr = *head_ptr_ptr;
    }

    return NO_ERROR;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int insert_into_queue(Queue_element** head_ptr_ptr,
                      Queue_element** tail_ptr_ptr,
                      void* contents_ptr)
{
    Queue_element* save_head_ptr;


    save_head_ptr = *head_ptr_ptr;
    NRE(*head_ptr_ptr = TYPE_MALLOC(Queue_element));

    (*head_ptr_ptr)->contents = contents_ptr;
    (*head_ptr_ptr)->next = save_head_ptr;
    (*head_ptr_ptr)->previous = NULL;

    if (save_head_ptr != NULL)
    {
        save_head_ptr->previous = *head_ptr_ptr;
    }
    else if (tail_ptr_ptr != NULL)
    {
        *tail_ptr_ptr = *head_ptr_ptr;
    }

    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          insert_at_end_of_queue
 *
 *
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

int debug_insert_at_end_of_queue(Queue_element** head_ptr_ptr,
                                 Queue_element** tail_ptr_ptr,
                                 void* contents_ptr,
                                 const char* file_name, int line_number)
{
    Queue_element* local_tail_ptr;
    Queue_element* prev_elem, *cur_elem;


    NRE(local_tail_ptr = DEBUG_TYPE_MALLOC(Queue_element, file_name,
                                           line_number));
    local_tail_ptr->contents = contents_ptr;
    local_tail_ptr->next = NULL;
    local_tail_ptr->previous = NULL;

    if (tail_ptr_ptr == NULL)
    {
        if (*head_ptr_ptr == NULL)
        {
             *head_ptr_ptr = local_tail_ptr;
        }
        else
        {
            cur_elem = *head_ptr_ptr;

            while (cur_elem != NULL)
            {
                prev_elem = cur_elem;
                cur_elem = cur_elem->next;
            }
            prev_elem->next = local_tail_ptr;
            local_tail_ptr->previous = prev_elem;
        }
    }
    else if (*tail_ptr_ptr == NULL)
    {
        if (*head_ptr_ptr == NULL)
        {
             *head_ptr_ptr = local_tail_ptr;
             *tail_ptr_ptr = local_tail_ptr;
         }
        else
        {
            set_bug(
               "Tail is NULL, but head is not NULL in insert_at_end_of_queue.");
            return ERROR;
        }
    }
    else
    {
         (*tail_ptr_ptr)->next = local_tail_ptr;
         local_tail_ptr->previous = *tail_ptr_ptr;
         *tail_ptr_ptr = local_tail_ptr;
     }

    return NO_ERROR;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int insert_at_end_of_queue(Queue_element** head_ptr_ptr,
                           Queue_element** tail_ptr_ptr,
                           void* contents_ptr)
{
    Queue_element* local_tail_ptr;
    Queue_element* prev_elem = NULL;
    Queue_element* cur_elem;


    NRE(local_tail_ptr = TYPE_MALLOC(Queue_element));
    local_tail_ptr->contents = contents_ptr;
    local_tail_ptr->next = NULL;
    local_tail_ptr->previous = NULL;

    if (tail_ptr_ptr == NULL)
    {
        if (*head_ptr_ptr == NULL)
        {
             *head_ptr_ptr = local_tail_ptr;
        }
        else
        {
            cur_elem = *head_ptr_ptr;

            while (cur_elem != NULL)
            {
                prev_elem = cur_elem;
                cur_elem = cur_elem->next;
            }
            prev_elem->next = local_tail_ptr;
            local_tail_ptr->previous = prev_elem;
        }
    }
    else if (*tail_ptr_ptr == NULL)
    {
        if (*head_ptr_ptr == NULL)
        {
             *head_ptr_ptr = local_tail_ptr;
             *tail_ptr_ptr = local_tail_ptr;
         }
        else
        {
            set_bug(
               "Tail is NULL, but head is not NULL in insert_at_end_of_queue.");
            return ERROR;
        }
    }
    else
    {
         (*tail_ptr_ptr)->next = local_tail_ptr;
         local_tail_ptr->previous = *tail_ptr_ptr;
         *tail_ptr_ptr = local_tail_ptr;
     }

    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          insert_into_ordered_queue
 *
 *
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

int insert_into_ordered_queue(Queue_element** head_ptr_ptr,
                              Queue_element** tail_ptr_ptr,
                              void* contents_ptr,
                              int (*key_comp_fn) (const void *, const void *),
                              int skip_duplicates)
{
    Queue_element* save_head_ptr, *new_element_ptr;
    Queue_element* current, *previous;
    int compare_res;


    if (*head_ptr_ptr == NULL)
    {
        NRE(*head_ptr_ptr = TYPE_MALLOC(Queue_element));
        (*head_ptr_ptr)->next = NULL;
        (*head_ptr_ptr)->previous = NULL;
        (*head_ptr_ptr)->contents = contents_ptr;

        if (tail_ptr_ptr != NULL) *tail_ptr_ptr = *head_ptr_ptr;

        return NO_ERROR;
    }

    compare_res = (*key_comp_fn)((*head_ptr_ptr)->contents, contents_ptr);

    if ((compare_res == 0) && (skip_duplicates))
    {
        return NOT_INSERTED;
    }

    if (compare_res > 0)
    {
        save_head_ptr = *head_ptr_ptr;
        NRE((*head_ptr_ptr) = TYPE_MALLOC(Queue_element));
        (*head_ptr_ptr)->contents = contents_ptr;

        (*head_ptr_ptr)->next = save_head_ptr;
        (*head_ptr_ptr)->previous = NULL;
        save_head_ptr->previous = *head_ptr_ptr;
        return NO_ERROR;
    }

    previous = *head_ptr_ptr;
    current = (*head_ptr_ptr)->next;

    while (current != NULL )
    {
        compare_res = (*key_comp_fn)(current->contents, contents_ptr);

        if ((compare_res == 0) && (skip_duplicates))
        {
            return NOT_INSERTED;
        }

        if (compare_res >= 0)
        {
            NRE(new_element_ptr = TYPE_MALLOC(Queue_element));
            new_element_ptr->contents = contents_ptr;

            previous->next = new_element_ptr;
            new_element_ptr->previous = previous;

            new_element_ptr->next = current;
            current->previous = new_element_ptr;

            return NO_ERROR;
        }
        previous = current;
        current = current->next;
    }

    NRE(new_element_ptr = TYPE_MALLOC(Queue_element));
    previous->next = new_element_ptr;
    new_element_ptr->previous = previous;

    new_element_ptr->next = NULL;
    new_element_ptr->contents = contents_ptr;

    if (tail_ptr_ptr != NULL) *tail_ptr_ptr = new_element_ptr;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            remove_first_element
 *
 *
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

Queue_element* remove_first_element(Queue_element** head_ptr_ptr,
                                    Queue_element** tail_ptr_ptr)
{
      Queue_element* element_to_be_removed;


      if (*head_ptr_ptr == NULL) return NULL;

      element_to_be_removed = *head_ptr_ptr;
      *head_ptr_ptr = (*head_ptr_ptr)->next;

      if ((*head_ptr_ptr) != NULL)
      {
          (*head_ptr_ptr)->previous = NULL;
      }
      else if (tail_ptr_ptr != NULL)
      {
          *tail_ptr_ptr = NULL;
      }

      return element_to_be_removed;
  }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         remove_last_element
 *
 *
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

Queue_element* remove_last_element(Queue_element** head_ptr_ptr,
                                   Queue_element** tail_ptr_ptr)
{
     Queue_element* cur_elem;
     Queue_element* prev_elem;


     UNTESTED_CODE();

     if ( *head_ptr_ptr == NULL)
     {
         return NULL;
     }
     else if ( ((*head_ptr_ptr)->next) == NULL)
     {
         cur_elem = *head_ptr_ptr;
         *head_ptr_ptr = NULL;

         if (tail_ptr_ptr != NULL)
         {
             *tail_ptr_ptr = NULL;
         }
         return cur_elem;
     }
     else
     {
         prev_elem = *head_ptr_ptr;
         cur_elem = prev_elem->next;

         while ((cur_elem->next) != NULL)
         {
             prev_elem = cur_elem;
             cur_elem = cur_elem->next;
         }
         prev_elem->next = NULL;

         if (tail_ptr_ptr != NULL)
         {
             *tail_ptr_ptr = prev_elem;
         }
         return cur_elem;
     }
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int cat_queue_copy(Queue_element** head_ptr_ptr, Queue_element** tail_ptr_ptr,
                   Queue_element* queue_to_cat_ptr, long int size_of_contents)
{
    Queue_element* copy_head;
    Queue_element* copy_tail;
    Queue_element* previous = NULL;
    Queue_element* current;


    /*
    // It may be a while before this gets tested in the context of backward
    // links.
    */
    UNTESTED_CODE();

    ERE(copy_queue(&copy_head, &copy_tail, queue_to_cat_ptr,
                   size_of_contents));

    if (*head_ptr_ptr == NULL)
    {
        *head_ptr_ptr = copy_head;

        if (tail_ptr_ptr != NULL)
        {
            *tail_ptr_ptr = copy_tail;
        }
    }
    else
    {
        if (tail_ptr_ptr != NULL)
        {
            (*tail_ptr_ptr)->next = copy_head;
            copy_head->previous = *tail_ptr_ptr;

            (*tail_ptr_ptr) = copy_tail;
        }
        else
        {
            current = *head_ptr_ptr;

            while ( current != NULL)
            {
                previous = current;
                current = current->next;
            }
            previous->next = copy_head;
            copy_head->previous = previous;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int copy_queue(Queue_element** head_ptr_ptr, Queue_element** tail_ptr_ptr,
               Queue_element* queue_to_copy, long int size_of_contents)
{
    void* contents;


    /*
    // Lint spotted what looks like it must be a bug. I fixed it, but have
    // not tested it since.
    */
    UNTESTED_CODE();

    if ((size_of_contents <= 0) || (head_ptr_ptr == NULL))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    *head_ptr_ptr = NULL;

    if (tail_ptr_ptr != NULL)
    {
        *tail_ptr_ptr = NULL;
    }

    if (queue_to_copy != NULL)
    {
        Queue_element* cur_source_elem = queue_to_copy;


        while ( cur_source_elem != NULL)
        {
            NRE(contents = (void*)KJB_MALLOC(size_of_contents));

            kjb_memcpy(contents, (cur_source_elem->contents),
                       size_of_contents);

            ERE(insert_at_end_of_queue(head_ptr_ptr, tail_ptr_ptr, contents));

            cur_source_elem = cur_source_elem->next;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int cat_str_queue_copy(Queue_element** head_ptr_ptr,
                       Queue_element** tail_ptr_ptr,
                       Queue_element* queue_to_cat_ptr)
{
    Queue_element*  copy_head;
    Queue_element*  copy_tail;
    Queue_element*  previous = NULL;
    Queue_element*  current;


    /*
    // It may be a while before this gets tested in the context of backward
    // links.
    */
    UNTESTED_CODE();

    ERE(copy_str_queue(&copy_head, &copy_tail, queue_to_cat_ptr));

    if ( *head_ptr_ptr == NULL)
    {
        *head_ptr_ptr = copy_head;

        if (tail_ptr_ptr != NULL)
        {
            *tail_ptr_ptr = copy_tail;
        }
    }
    else
    {
        if (tail_ptr_ptr != NULL)
        {
            (*tail_ptr_ptr)->next = copy_head;
            copy_head->previous = *tail_ptr_ptr;

            (*tail_ptr_ptr) = copy_tail;
        }
        else
        {
            current = *head_ptr_ptr;

            while ( current != NULL)
            {
                previous = current;
                current = current->next;
            }
            previous->next = copy_head;
            copy_head->previous = previous;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int copy_str_queue(Queue_element** head_ptr_ptr, Queue_element** tail_ptr_ptr,
                   Queue_element* queue_to_copy)
{
    void*           contents;
    Queue_element*  cur_source_elem;


    /*
    // It may be a while before this gets tested in the context of backward
    // links.
    */
    UNTESTED_CODE();

    *head_ptr_ptr = NULL;

    if (tail_ptr_ptr != NULL)
    {
        *tail_ptr_ptr = NULL;
    }

    if (queue_to_copy != NULL)
    {
        cur_source_elem = queue_to_copy;

        while ( cur_source_elem != NULL)
        {
            NRE(contents=(void*)kjb_strdup(
                                          (char*)(cur_source_elem->contents)));

            ERE(insert_at_end_of_queue(head_ptr_ptr, tail_ptr_ptr,
                                       contents));

            cur_source_elem = cur_source_elem->next;
        }
     }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int free_selected_elements(Queue_element** head_ptr_ptr,
                           Queue_element** tail_ptr_ptr,
                           int (*key_comp_fn) (void * , void * ),
                           void* key_ptr,
                           void (*free_contents_fn) (void * ))
{
    Queue_element*  element_to_be_removed;
    Queue_element*  current;
    Queue_element*  previous;


    /*
    // It may be a while before this gets tested in the context of backward
    // links.
    */
    UNTESTED_CODE();

    while (    (*head_ptr_ptr != NULL)
            && ((*key_comp_fn)((*head_ptr_ptr)->contents, key_ptr)
                  == SELECT_ELEMENT
               )
          )
    {
        element_to_be_removed = *head_ptr_ptr;
        *head_ptr_ptr = (*head_ptr_ptr)->next;
        (*head_ptr_ptr)->previous = NULL;
        free_queue_element(element_to_be_removed, free_contents_fn);
    }

    if (*head_ptr_ptr == NULL)
    {
        if (tail_ptr_ptr != NULL) *tail_ptr_ptr = NULL;

        return NO_ERROR;
    }

    previous = *head_ptr_ptr;
    current = (*head_ptr_ptr)->next;

    while (current != NULL)
    {
        if ((*key_comp_fn)(current->contents, key_ptr) == SELECT_ELEMENT)
        {
            element_to_be_removed = current;
            previous->next = current->next;
            current = current->next;

            if (current != NULL)
            {
                current->previous = previous;
            }
            else if (tail_ptr_ptr != NULL)
            {
                *tail_ptr_ptr = previous;
            }

            free_queue_element(element_to_be_removed, free_contents_fn);
        }
        else
        {
            previous = current;
            current = current->next;
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               search_queue
 *
 *
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

void* search_queue (Queue_element* head_ptr,
                    int (*key_comp_fn) (const void *, const void *),
                    void* key_ptr)
{
    Queue_element* current;


    current = head_ptr;

    while (current != NULL )
    {
        if ( (*key_comp_fn)(current->contents, key_ptr) == SELECT_ELEMENT)
        {
            return current->contents;
        }
        current = current->next;
    }
    return NULL;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            remove_first_selected_element
 *
 *
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

Queue_element* remove_first_selected_element(Queue_element** head_ptr_ptr,
                                             Queue_element** tail_ptr_ptr,
                                             int (*key_comp_fn) (const void *, const void *),
                                             void* key_ptr)
{
    Queue_element*  element_to_be_removed;
    Queue_element*  current;
    Queue_element*  previous;


    /*
    // It may be a while before this gets tested in the context of backward
    // links.
    */
    UNTESTED_CODE();

    if (*head_ptr_ptr == NULL) return NULL;

    if ((*key_comp_fn)((*head_ptr_ptr)->contents, key_ptr) == SELECT_ELEMENT)
    {
        element_to_be_removed = *head_ptr_ptr;
        *head_ptr_ptr = (*head_ptr_ptr)->next;

        if (*head_ptr_ptr != NULL)
        {
            (*head_ptr_ptr)->previous = NULL;
        }
        else if (tail_ptr_ptr != NULL)
        {
            *tail_ptr_ptr = NULL;
        }

        return element_to_be_removed;
    }

    previous = *head_ptr_ptr;
    current = (*head_ptr_ptr)->next;

    while (current != NULL )
    {
        if ( (*key_comp_fn)(current->contents, key_ptr) == SELECT_ELEMENT)
        {
            element_to_be_removed = current;
            current = current->next;

            previous->next = current;

            if (current != NULL)
            {
                current->previous = previous;
            }
            else if (tail_ptr_ptr != NULL)
            {
                *tail_ptr_ptr = NULL;
            }

            return element_to_be_removed;
        }

        previous = current;
        current = current->next;
    }
    return NULL;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       remove_elements_less_than_key
 *
 *
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

/* This only works on ordered queues. */

Queue_element* remove_elements_less_than_key(Queue_element** head_ptr_ptr,
                                             Queue_element** tail_ptr_ptr,
                                             int (*select_fn) (const void *,
                                                               const void *),
                                             const void* select_key_ptr)
{
    IMPORT int kjb_debug_level;
    Queue_element*  removed_queue_head;
    Queue_element*  current;
    Queue_element*  previous;


    /*
    // It may be a while before this gets tested in the context of backward
    // links.
    UNTESTED_CODE();
    */

    if ((kjb_debug_level > 0) && (! is_queue_ordered(*head_ptr_ptr, select_fn)))
    {
        set_bug("Attempt to remove elements less than key from a queue that is not ordered.");
        return NULL;
    }

    if (*head_ptr_ptr == NULL) return NULL;

    if ( (*select_fn)((*head_ptr_ptr)->contents, select_key_ptr) >= 0)
    {
        return NULL;
    }

    previous = *head_ptr_ptr;
    current = (*head_ptr_ptr)->next;

    while (    (current != NULL )
            && ((*select_fn)(current->contents, select_key_ptr) < 0)
          )
    {
        previous = current;
        current = current->next;
    }

    previous->next = NULL;

    removed_queue_head = *head_ptr_ptr;

    *head_ptr_ptr = current;

    if (*head_ptr_ptr == NULL) 
    {
         if (tail_ptr_ptr != NULL)
         {
             *tail_ptr_ptr = NULL;
         }
    }
    else 
    {
        (*head_ptr_ptr)->previous = NULL;
    }
        
    return removed_queue_head;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       is_queue_ordered
 *
 * Checks that a queue is in order.
 *
 * If the queue is in order, this function returns TRUE, otherwise FALSE. 
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

/* This only works on ordered queues. */

int is_queue_ordered(Queue_element* head_ptr, int (*cmp_fn) (const void *, const void *))
{
    Queue_element*  current;
    Queue_element*  previous;


    if (head_ptr == NULL) return TRUE;

    previous = head_ptr;
    current = head_ptr->next;

    while (current != NULL ) 
    {
        if ((*cmp_fn)(current->contents, previous->contents) < 0)
        {
            return FALSE;
        }
        previous = current;
        current = current->next;
    }

    return TRUE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             apply_to_queue
 *
 *
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

int apply_to_queue(Queue_element* head_ptr, void (*apply_fn) (void * ))
{
    Queue_element* current;

    current = head_ptr;

    while (current != NULL)
    {
        (*apply_fn) ( current->contents );
        current = current->next;
    }
     return NO_ERROR;
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 apply_and_free
 *
 *
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

int apply_and_free(Queue_element* head_ptr, void (*apply_fn) (void * ), void (*free_contents_fn) (void * ))
{
    Queue_element* current;
    Queue_element* previous;


    current = head_ptr;

    while (current != NULL)
    {
        (*apply_fn)(current->contents );
        previous = current;
        current = current->next;
        free_queue_element(previous, free_contents_fn);
    }

    return NO_ERROR;
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             count_queue_elements
 *
 *
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

int count_queue_elements(Queue_element* head_ptr)
{
    Queue_element* cur_elem;
    int count;


    count = 0;
    cur_elem = head_ptr;

    while (cur_elem != NULL)
    {
        count++;
        cur_elem = cur_elem->next;
    }
    return count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            print_str_queue
 *
 *
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

int print_str_queue(Queue_element* head_ptr)
{
    Queue_element* cur_elem;


    cur_elem = head_ptr;

    pso("----------  str_queue  ----------\n\n");

    while (cur_elem != NULL)
    {
        kjb_fputs(stdout, (char*)cur_elem->contents);
        pso("\n");
        cur_elem = cur_elem->next;
    }

    pso("\n---------------------------------\n");

    fflush(stdout);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            print_queue
 *
 *
 *
 * Index: queues, linked lists, stacks
 *
 * -----------------------------------------------------------------------------
*/

int print_queue(Queue_element* head_ptr, int (*print_fn) (const void *))
{
    Queue_element* cur_elem;


    cur_elem = head_ptr;

    pso("----------  queue  ----------\n\n");

    while (cur_elem != NULL)
    {
        ERE(print_fn(cur_elem->contents));
        cur_elem = cur_elem->next;
    }

    pso("\n-----------------------------\n");

    fflush(stdout);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

