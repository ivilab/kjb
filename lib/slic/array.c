/* $Id: array.c 15688 2013-10-14 08:46:32Z predoehl $
 */
#include "l/l_incl.h"
#include "slic/array.h"

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
    Array            *array_ptr,
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
        pso(" (%d) ", i);
        print_fn(array_ptr->elements[i]);
        pso("\n");
    }
}
