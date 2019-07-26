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

#include "wrap_wordnet/wn_array.h"


#ifdef __cplusplus
extern "C" {
#endif

Array *create_array
(
    int length 
)
{
    int i;
    Array *array_ptr = NULL;

    array_ptr = (Array*)malloc(sizeof(Array));
    if(array_ptr == NULL) return NULL;

    array_ptr->elements = (void**)malloc(sizeof(void*)*length);
    if(array_ptr->elements == NULL)
    {
        free(array_ptr);
        return NULL;
    }
    
    for(i=0; i<length; i++)
    {
        array_ptr->elements[i] = NULL;
    }

    array_ptr->length = length;

    return array_ptr;
}

void free_array
(
    Array  *array_ptr,
    void   (*free_element_fn)(void*)
)
{
    int i;
    int length;

    if(array_ptr == NULL) return;

    length = array_ptr->length;
    if(array_ptr->elements != NULL)
    {
        if(free_element_fn != NULL)
        {
            for(i=0; i<length; i++)
            {
                free_element_fn(array_ptr->elements[i]);
            }
        }
        free(array_ptr->elements);
    }

    free(array_ptr);
    array_ptr = NULL;
}

void free_array_array
(
    Array  *array_array_ptr,
    void   (*free_array_fn)(void*)
)
{
    int i;
    int length;

    if(array_array_ptr == NULL) return;

    length = array_array_ptr->length;
    if(array_array_ptr->elements != NULL)
    {
        if(free_array_fn != NULL)
        {
            for(i=0; i<length; i++)
            {
                free_array_fn(array_array_ptr->elements[i]);
            }
        }
        free(array_array_ptr->elements);
    }

    free(array_array_ptr);
    array_array_ptr = NULL;
}

int search_array
(
    const Array      *array_ptr,
    int              (*key_comp_fn) (const void*, const void*),
    const void       *key_ptr
)
{
    int i;
    int length;
    
    if(array_ptr == NULL) return -1;
    
    length = array_ptr->length;
    for(i=0; i<length; i++)
    {
        if(key_comp_fn(array_ptr->elements[i], key_ptr) == 1)
        {
            return i;
        }
    }

    return -1;
}

int merge_arrays
(
    const Array *a_ptr,
    const Array *b_ptr,
    Array       **c_ptr_ptr
)
{
    Array *c_ptr = NULL;
    int a_len = 0;
    int b_len = 0;
    int len;
    int count;
    int i;

    if(a_ptr == NULL && b_ptr == NULL)
    {
        return 0;
    }
    
    if(c_ptr_ptr == NULL) return 0;

    if(a_ptr != NULL)
    {
        a_len = a_ptr->length;
    }
    
    if(b_ptr != NULL)
    {
        b_len = b_ptr->length;
    }

    len = a_len + b_len;
    c_ptr = create_array(len);
    if(c_ptr == NULL)
    {
        return ERROR;
    }
    *c_ptr_ptr = c_ptr;

    count = 0;
    for(i=0; i<a_len; i++)
    {
        c_ptr->elements[count++] = a_ptr->elements[i]; 
    }
    
    for(i=0; i<b_len; i++)
    {
        c_ptr->elements[count++] = b_ptr->elements[i]; 
    }

    return len;
}

void print_array
(
    const Array * array_ptr, 
    void (*print_fn) (const void *)
)
{
    int i;
    int length;

    if(array_ptr == NULL) return;
    
    length = array_ptr->length;
    for(i=0; i<length; i++)
    {
        printf(" (%d) ", i);
        print_fn(array_ptr->elements[i]);
        printf("\n");
    }
}


#ifdef __cplusplus
}
#endif



