
/* $Id: kjb_sort.c 15999 2013-11-13 23:49:08Z kobus $ */



#include "l/l_incl.h" 
#include "m/m_incl.h" 

#define MAX_COUNT 100000

typedef struct Thing_one 
{
    int a;
    int b;
    char *str;
    int c;
} 
Thing_one;

typedef struct Thing_two 
{
    char *str;
    int a;
    int b;
    int c;
}
Thing_two;


static int thing_one_cmp_field_three(const void *p1, const void *p2)
{

    return kjb_strcmp(((const Thing_one*)p1)->str, 
                      ((const Thing_one*)p2)->str); 
}


static int thing_two_cmp_field_one(const void *p1, const void *p2)
{

    return kjb_strcmp(((const Thing_two*)p1)->str,
                      ((const Thing_two*)p2)->str); 
}

/* Copied from m_vector for quick hack. */

static int ascend_compare_indexed_vector_elements
(
    const void* first_ptr,
    const void* second_ptr
)
{
    const Indexed_vector_element *elem1_ptr =(const Indexed_vector_element*)first_ptr;
    const Indexed_vector_element *elem2_ptr=(const Indexed_vector_element*)second_ptr;


    if (elem2_ptr->element < elem1_ptr->element)
    {
        return FIRST_ITEM_GREATER;
    }
    else if (elem2_ptr->element > elem1_ptr->element)
    {
        return SECOND_ITEM_GREATER;
    }
    else
    {
        return EQUAL_ITEMS;
    }
}

/* Copied from m_vector for quick hack. */

static int descend_compare_indexed_vector_elements
(
    const void* first_ptr,
    const void* second_ptr
)
{
    const Indexed_vector_element *elem1_ptr= (const Indexed_vector_element*)first_ptr;
    const Indexed_vector_element *elem2_ptr=(const Indexed_vector_element*)second_ptr;


    if (elem2_ptr->element > elem1_ptr->element)
    {
        return FIRST_ITEM_GREATER;
    }
    else if (elem2_ptr->element < elem1_ptr->element)
    {
        return SECOND_ITEM_GREATER;
    }
    else
    {
        return EQUAL_ITEMS;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int main(int argc, char **argv)
{
    FILE*      fp;
    int        i;
    int        count;
    Thing_one* thing_one_ptr;
    Thing_one  thing_one;
    Thing_two* thing_two_ptr;
    Thing_two  thing_two;
    char       word[ 200 ];
    char       line[ 10000 ];
    char*      line_pos;
    int        sort_result;
    Indexed_vector* ivp = NULL;
    int is_sorted;

    kjb_disable_paging();

    for (i = 0; i < 100; i ++)
    {
        EPETE(get_random_indexed_vector(&ivp, 10000)); 

        is_sorted = check_sort(ivp->elements, ivp->length, 
                               sizeof(Indexed_vector_element),
                               ascend_compare_indexed_vector_elements);
        ASSERT(! is_sorted);

        is_sorted = check_sort(ivp->elements, ivp->length, 
                               sizeof(Indexed_vector_element),
                               descend_compare_indexed_vector_elements);
        ASSERT(! is_sorted);

        EPETE(ascend_sort_indexed_vector(ivp));

        is_sorted = check_sort(ivp->elements, ivp->length, 
                               sizeof(Indexed_vector_element),
                               ascend_compare_indexed_vector_elements);

        if (! is_sorted) 
        {
            p_stderr("Sort failed.\n");
            exit(EXIT_BUG);
        }

        EPETE(descend_sort_indexed_vector(ivp));

        is_sorted = check_sort(ivp->elements, ivp->length, 
                               sizeof(Indexed_vector_element),
                               descend_compare_indexed_vector_elements);

        if (! is_sorted) 
        {
            p_stderr("Sort failed.\n");
            exit(EXIT_BUG);
        }

    }

    free_indexed_vector(ivp); 
    
    if (argc <= 1) 
    {
        kjb_exit(EXIT_SUCCESS);
    }


#ifdef TEST
    /* We don't have a feee routine for thing one and thing two. */
    disable_heap_checking();
#endif 

    NPETE(fp = kjb_fopen(argv[ 1 ], "r"));

    NPETE(thing_one_ptr = N_TYPE_MALLOC(Thing_one, MAX_COUNT));

    count = 0;

    while (BUFF_FGET_LINE(fp, line) != EOF) 
    {
        line_pos = line;

        while (BUFF_GET_TOKEN_OK(&line_pos, word)) 
        {
            if (count > MAX_COUNT - 1) 
            {
                p_stderr("Max count exceeded");
                exit( EXIT_BUG );
            }

            NPETE(thing_one.str = kjb_strdup(word));
            thing_one.a = 10000 * kjb_rand();
            thing_one.b = 10000 * kjb_rand();
            thing_one.c = 10000 * kjb_rand();

            EPETE(insert_into_sorted_array(thing_one_ptr, &count, 
                                           sizeof(Thing_one), 
                                           thing_one_cmp_field_three, 
                                           &thing_one));
        }
    }


    pso("-------------------------------------------\n");
    pso("   Should be sorted by last (string) column.\n\n");

    for (i=0; i<count; i++) 
    {
        pso("%8d %8d %8d  %s\n", thing_one_ptr[ i ].a, thing_one_ptr[ i ].b,
            thing_one_ptr[ i ].c, thing_one_ptr[ i ].str);
    }

    for (i=1; i<count; i++)
    {
        if (thing_one_cmp_field_three(thing_one_ptr+i-1, thing_one_ptr + i) > 0)
        {
            p_stderr("Check on sort 1 failed.\n");
            kjb_exit(EXIT_BUG);
        }
    }

    EPETE(int_sort(thing_one_ptr, count, sizeof(Thing_one), 0, 
                   USE_CURRENT_ATN_HANDLING));

    pso("-------------------------------------------\n");
    pso("   Should be sorted by first column.\n\n");


    for (i=0; i<count; i++) 
    {
        pso("%8d %8d %8d  %s\n", thing_one_ptr[ i ].a, thing_one_ptr[ i ].b,
            thing_one_ptr[ i ].c, thing_one_ptr[ i ].str);
    }

    for (i=1; i<count; i++)
    {
        if (thing_one_ptr[i - 1].a > thing_one_ptr[ i ].a)
        {
            p_stderr("Check on sort 2 failed.\n");
            kjb_exit(EXIT_BUG);
        }
    }

    /* Offset calculation assumes no padding. Perhaps FIXME? */
    EPETE(int_sort(thing_one_ptr, count, sizeof(Thing_one), sizeof(int), 
                   USE_CURRENT_ATN_HANDLING));

    pso("-------------------------------------------\n");
    pso("   Should be sorted by second column.\n\n");

    for (i=0; i<count; i++) 
    {
        pso("%8d %8d %8d  %s\n", thing_one_ptr[ i ].a, thing_one_ptr[ i ].b,
            thing_one_ptr[ i ].c, thing_one_ptr[ i ].str);
    }

    for (i=1; i<count; i++)
    {
        if (thing_one_ptr[i - 1].b > thing_one_ptr[ i ].b)
        {
            p_stderr("Check on sort 3 failed.\n");
            kjb_exit(EXIT_BUG);
        }
    }


    EPETE(sort_result = kjb_sort(thing_one_ptr, count, sizeof(Thing_one), 
                                 thing_one_cmp_field_three,
                                 USE_CURRENT_ATN_HANDLING));

    if (is_interactive())
    {
        pso("Sort result is %d\n", sort_result);
    }


    pso("-------------------------------------------\n");
    pso("   Should be sorted by last (string) column.\n\n");

    for (i=0; i<count; i++) 
    {
        pso("%8d %8d %8d  %s\n", thing_one_ptr[ i ].a, thing_one_ptr[ i ].b,
            thing_one_ptr[ i ].c, thing_one_ptr[ i ].str);
    }

    for (i=1; i<count; i++)
    {
        if (thing_one_cmp_field_three(thing_one_ptr + i - 1,thing_one_ptr + i)> 0)
        {
            p_stderr("Check on sort 4 failed.\n");
            kjb_exit(EXIT_BUG);
        }
    }

    if (is_interactive())
    {
        pso("\n-------------------------------------------\n");

        while (BUFF_TERM_GET_LINE("Binary search: ", line) != EOF) 
        {
            thing_one.str = line;

            EPETE(i = binary_search(thing_one_ptr, count, sizeof(Thing_one), 
                                    thing_one_cmp_field_three, 
                                    (void*)&thing_one));

            if (i == NOT_FOUND) 
            {
                pso("Not found.\n");
            }
            else 
            {
                pso("%8d %8d %8d  %s\n", thing_one_ptr[ i ].a, 
                    thing_one_ptr[ i ].b,
                    thing_one_ptr[ i ].c, thing_one_ptr[ i ].str);
            }
        }

        pso("\n-------------------------------------------\n");

        while (BUFF_TERM_GET_LINE("Linear search: ", line) != EOF) 
        {
            thing_one.str = line; 

            EPETE(i = linear_search(thing_one_ptr, count, sizeof(Thing_one),
                                    thing_one_cmp_field_three, 
                                    (void*)&thing_one));

            if (i == NOT_FOUND) 
            {
                pso("Not found.\n");
            }
            else 
            {
                pso("%8d %8d %8d  %s\n", thing_one_ptr[ i ].a, 
                    thing_one_ptr[ i ].b,
                    thing_one_ptr[ i ].c, thing_one_ptr[ i ].str);
            }
        }

        pso("\n");
    }

    rewind(fp);

    count = 0;

    NPETE(thing_two_ptr = N_TYPE_MALLOC(Thing_two, MAX_COUNT));

    while (BUFF_FGET_LINE(fp, line) != EOF) 
    {
        line_pos = line;

        while (BUFF_GET_TOKEN_OK(&line_pos, word)) 
        {
            if (count > MAX_COUNT - 1) 
            {
                p_stderr("Max count exceeded");
                exit( EXIT_BUG );
            }

            NPETE(thing_two.str = kjb_strdup(word));
            thing_two.a = 10000 * kjb_rand();
            thing_two.b = 10000 * kjb_rand();
            thing_two.c = 10000 * kjb_rand();

            EPETE(insert_into_sorted_array(thing_two_ptr, &count, 
                                           sizeof(Thing_two), 
                                           thing_two_cmp_field_one, 
                                           &thing_two));
        }
    }

    pso("-------------------------------------------\n");
    pso("   Should be sorted by last column.\n\n");

    for (i=0; i<count; i++) 
    {
        pso("%8d %8d %8d  %s\n", thing_two_ptr[ i ].a, thing_two_ptr[ i ].b,
            thing_two_ptr[ i ].c, thing_two_ptr[ i ].str);
    }

    for (i=1; i<count; i++)
    {
        if (thing_two_cmp_field_one(thing_two_ptr + i - 1, thing_two_ptr + i) > 0)
        {
            p_stderr("Check on sort 5 failed.\n");
            kjb_exit(EXIT_BUG);
        }
    }

    /* Offset calculation assumes no padding. Perhaps FIXME? */
    EPETE(int_sort(thing_two_ptr, count, sizeof(Thing_two), sizeof(int*), 
                   USE_CURRENT_ATN_HANDLING));

    pso("-------------------------------------------\n");
    pso("   Should be sorted by first column.\n\n");

    for (i=0; i<count; i++) 
    {
        pso("%8d %8d %8d  %s\n", thing_two_ptr[ i ].a, thing_two_ptr[ i ].b,
            thing_two_ptr[ i ].c, thing_two_ptr[ i ].str);
    }

    for (i=1; i<count; i++)
    {
        if (thing_two_ptr[i - 1].a > thing_two_ptr[ i ].a)
        {
            p_stderr("Check on sort 6 failed.\n");
            kjb_exit(EXIT_BUG);
        }
    }

    /* Offset calculation assumes no padding. Perhaps FIXME? */
    EPETE(int_sort(thing_two_ptr, count, sizeof(Thing_two), 
                   sizeof(int*) + sizeof(int), 
                   USE_CURRENT_ATN_HANDLING));

    pso("-------------------------------------------\n");
    pso("   Should be sorted by second column.\n\n");


    for (i=0; i<count; i++) 
    {
        pso("%8d %8d %8d  %s\n", thing_two_ptr[ i ].a, thing_two_ptr[ i ].b,
            thing_two_ptr[ i ].c, thing_two_ptr[ i ].str);
    }

    for (i=1; i<count; i++)
    {
        if (thing_two_ptr[i - 1].b > thing_two_ptr[ i ].b)
        {
            p_stderr("Check on sort 7 failed.\n");
            kjb_exit(EXIT_BUG);
        }
    }

    EPETE(sort_result = kjb_sort(thing_two_ptr, count, sizeof(Thing_two), 
                                 thing_two_cmp_field_one, 
                                 USE_CURRENT_ATN_HANDLING));

    if (is_interactive())
    {
        pso("Sort result is %d\n", sort_result);
    }

    pso("-------------------------------------------\n");
    pso("   Should be sorted by last (string) column.\n\n");

    for (i=0; i<count; i++) 
    {
        pso("%8d %8d %8d  %s\n", thing_two_ptr[ i ].a, thing_two_ptr[ i ].b,
            thing_two_ptr[ i ].c, thing_two_ptr[ i ].str);
    }

    for (i=1; i<count; i++)
    {
        if (thing_two_cmp_field_one(thing_two_ptr + i - 1, thing_two_ptr + i) > 0)
        {
            p_stderr("Check on sort 8 failed.\n");
            kjb_exit(EXIT_BUG);
        }
    }

    if (is_interactive())
    {
        pso("\n-------------------------------------------\n");

        while (BUFF_TERM_GET_LINE("Binary search: ", line) != EOF) 
        {
            thing_two.str = line;

            EPETE(i = binary_search(thing_two_ptr, count, sizeof(Thing_two), 
                                    thing_two_cmp_field_one, 
                                    (void*)&thing_two));

            if (i == NOT_FOUND) 
            {
                pso("Not found.\n");
            }
            else 
            {
                pso("%8d %8d %8d  %s\n", thing_two_ptr[ i ].a, 
                    thing_two_ptr[ i ].b,
                    thing_two_ptr[ i ].c, thing_two_ptr[ i ].str);
            }
        }

        pso("\n-------------------------------------------\n");

        while (BUFF_TERM_GET_LINE("Linear search: ", line) != EOF) 
        {
            thing_two.str = line;

            EPETE(i = linear_search(thing_two_ptr, count, sizeof(Thing_two), 
                                    thing_two_cmp_field_one, 
                                    (void*)&thing_two));

            if (i == NOT_FOUND) 
            {
                pso("Not found.\n");
            }
            else 
            {
                pso("%8d %8d %8d  %s\n", thing_two_ptr[ i ].a, 
                    thing_two_ptr[ i ].b,
                    thing_two_ptr[ i ].c, thing_two_ptr[ i ].str);
            }
        }

        pso("\n");
    }

    kjb_free(thing_one_ptr);
    kjb_free(thing_two_ptr);
    kjb_fclose(fp);

    return EXIT_SUCCESS; 



}

