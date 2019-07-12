/**
 * This work is licensed under a Creative Commons
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 *
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 *
 * You are free:
 *
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 *
 * Under the following conditions:
 *
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 *
 *    Noncommercial. You may not use this work for commercial purposes.
 *
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 *
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 *
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 *
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */

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
| Authors:
|     Luca Del Pero
|
* =========================================================================== */

#include "wrap_wordnet/wn_linkedlist.h"


#ifdef __cplusplus
extern "C" {
#endif

#define SELECT_ELEMENT 1

Linkedlist *create_linkedlist
(
    void
)
{
    Linkedlist *list_ptr = NULL;

    list_ptr = (Linkedlist*)malloc(sizeof(Linkedlist));
    if(list_ptr == NULL)
    {
        return NULL;
    }

    list_ptr->head = NULL;
    list_ptr->tail = NULL;
    list_ptr->length = 0;

    return list_ptr;
}

void free_linkedlist
(
    Linkedlist *list_ptr,
    void      (*free_contents_fn)(void*)
)
{
    List_element* current;
    List_element* previous;

    if(list_ptr == NULL) return;

    /*if (free_contents_fn == NULL) free_contents_fn = free; */

    current = list_ptr->head; 

    while (current != NULL) 
    {
        previous = current; 
        current = current->next; 
        if(previous->contents != NULL)
        {
            if(free_contents_fn != NULL)
            {
                (*free_contents_fn)(previous->contents);
            }
            previous->contents = NULL;
        }

        free(previous); 
        previous = NULL;
    }

    free(list_ptr);
    list_ptr = NULL;
}

int insert_head_element
(
    Linkedlist   *list_ptr,
    List_element *element
)
{
    if(element == NULL)
    {
        return NO_ERROR;
    }

    if(list_ptr->head == NULL)
    {
        list_ptr->head = element;
        list_ptr->tail = element;
    }
    else
    {
        list_ptr->head->prev = element;
        element->next = list_ptr->head;
        list_ptr->head = element;
    }

    list_ptr->length++;

    return NO_ERROR;
}

int insert_head_element_with_contents
(
    Linkedlist *list_ptr,
    void       *contents
)
{
    List_element *element;
    int res;

    element = (List_element*)malloc(sizeof(List_element)); 
    if(element == NULL)
    {
        return ERROR;
    }

    element->contents = contents;
    element->next = NULL;
    element->prev = NULL;

    res = insert_head_element(list_ptr, element);

    return res;
}

int append_element
(
    Linkedlist   *list_ptr,
    List_element *element
)
{
    if(element == NULL)
    {
        return NO_ERROR;
    }
    
    if(list_ptr->tail == NULL)
    {
        list_ptr->head = element;
        list_ptr->tail = element;
    }
    else
    {
        list_ptr->tail->next = element;
        element->prev = list_ptr->tail;
        list_ptr->tail = element;
    }

    list_ptr->length++;

    return NO_ERROR;
}

int append_element_with_contents
(
    Linkedlist *list_ptr,
    void *contents
)
{
    List_element *element;
    int res;

    element = (List_element*)malloc(sizeof(List_element)); 
    if(element == NULL)
    {
        return ERROR;
    }

    element->contents = contents;
    element->next = NULL;
    element->prev = NULL;

    res = append_element(list_ptr, element);

    return res;
}

void *remove_list_head
(
    Linkedlist *list_ptr
)
{
    List_element *element = NULL;
    void *ptr = NULL;

    if(list_ptr->head == NULL)
    {
        return NULL;
    }

    element = list_ptr->head;
    list_ptr->head = element->next;
    if(list_ptr->head == NULL)
    {
        list_ptr->tail = NULL;
    }
    else 
    {
        list_ptr->head->prev = NULL;
    }
   
    ptr = element->contents;

    list_ptr->length--;

    free(element);
    element = NULL;

    return ptr;
}

void *remove_list_element
(
    Linkedlist          *list_ptr,
    const void          *contents
)
{
    List_element* current;
    void *ptr = NULL;

    current = list_ptr->head; 

    while (current != NULL ) 
    {
        if(current->contents == contents)
        {
            break;
        }
        current = current->next; 
    }

    if(current == NULL)
    {
        return NULL;
    }

    if(current == list_ptr->head)
    {
        list_ptr->head = current->next;
        if(list_ptr->head == NULL) /* become empty list */
        {
            list_ptr->tail = NULL;
        }
        else
        {
            list_ptr->head->prev = NULL;
        }
    }
    else if(current == list_ptr->tail)
    {
        list_ptr->tail = current->prev;
        if(list_ptr->tail == NULL) /* become empty list  should have been handled!*/
        {
            list_ptr->head = NULL;
        }
        else
        {
            list_ptr->tail->next = NULL;
        }
    }
    else
    {
        current->prev->next = current->next;
        current->next->prev = current->prev;
    }

    list_ptr->length--;

    ptr = current->contents;
    free(current);
    current = NULL;

    return ptr;
}

void *search_linkedlist
(
    const Linkedlist *list_ptr,
    int              (*key_comp_fn) (const void*, const void*),
    const void       *key_ptr
)
{
    List_element* current;

    current = list_ptr->head; 

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

/* dump the list element into an array*/
int from_list_to_array
(
    const Linkedlist *list_ptr,
    Array            **array_ptr_ptr
)
{
    int count;
    int length;
    List_element *current = NULL;
    Array *array_ptr = NULL;

    if(list_ptr == NULL) return ERROR;

    length = list_ptr->length;
    if(length == 0) return NO_ERROR;

    array_ptr = create_array(length);
   if(array_ptr == NULL)
   {
       return ERROR;
   }
   *array_ptr_ptr = array_ptr;

   count = 0;
   current = list_ptr->head;
   while(current != NULL)
   {
       array_ptr->elements[count++] = current->contents;
       current = current->next;
   }

   return NO_ERROR;
}

void print_linkedlist
(
    const Linkedlist * list_ptr, 
    void (*print_fn) (const void *)
)
{
    List_element* cur_elem;

    if(list_ptr == NULL) return;

    cur_elem = list_ptr->head; 

    printf("----------  Linked List: %d elements ----------\n\n", list_ptr->length); 

    while (cur_elem != NULL) 
    {
        print_fn(cur_elem->contents);
        cur_elem = cur_elem->next; 
    }

    printf("\n-----------------------------\n"); 

    fflush(stdout);
}

#ifdef __cplusplus
}
#endif
