
/* $Id: g2_ellipse.c 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 1994-2008 by Kobus Barnard (author).                         |
|                                                                              |
|  For use outside the SFU vision lab please contact the author(s).            |
|                                                                              |
* =========================================================================== */

#include "m/m_gen.h"
#include "g2/g2_ellipse.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


/* =============================================================================
 *                          get_target_ellipse_list
 *
 * Gets target ellipse list
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of ellipse_lists. If *target_list_ptr_ptr is NULL, then
 * this routine creates the ellipse_list. If it is not null, and it is the
 * right size, then this routine does nothing. If it is the wrong size, then it
 * is resized.
 *
 * In order to be memory efficient, we free before resizing. If the resizing
 * fails, then the original contents of the *target_list_ptr_ptr will be lost.
 * However, (*target_list_ptr_ptr)->ellipses will be set to NULL, so
 * (*target_list_ptr_ptr) can be safely sent to free_ellipse_list().
 *
 * Index: images processing, ellipses
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION
int debug_get_target_ellipse_list
(
    Ellipse_list** target_list_ptr_ptr,
    int            num_ellipses,
    const char*    file_name,
    int            line_number
)
{
    Ellipse_list* list_ptr = *target_list_ptr_ptr;


    if (num_ellipses <= 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (list_ptr == NULL)
    {
        NRE(list_ptr = debug_create_ellipse_list(num_ellipses, file_name,
                                                 line_number));
        *target_list_ptr_ptr = list_ptr;
    }
    else if (num_ellipses == list_ptr->num_ellipses)
    {
        /*EMPTY*/
        ; /* Do nothing */
    }
    else if (num_ellipses < list_ptr->num_ellipses)
    {
        int i;

        for (i=num_ellipses; i<list_ptr->num_ellipses; i++)
        {
            free_ellipse(list_ptr->ellipses[ i ]);
        }
        list_ptr->num_ellipses = num_ellipses;
    }
    else
    {
        free_ellipse_list(list_ptr);
        NRE(list_ptr = debug_create_ellipse_list(num_ellipses, file_name,
                                                 line_number));
        *target_list_ptr_ptr = list_ptr;
    }

    return NO_ERROR;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int get_target_ellipse_list
(
    Ellipse_list** target_list_ptr_ptr,
    int            num_ellipses
)
{
    Ellipse_list* list_ptr = *target_list_ptr_ptr;


    if (num_ellipses <= 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (list_ptr == NULL)
    {
        NRE(list_ptr = create_ellipse_list(num_ellipses));
        *target_list_ptr_ptr = list_ptr;
    }
    else if (num_ellipses == list_ptr->num_ellipses)
    {
        /*EMPTY*/
        ; /* Do nothing */
    }
    else if (num_ellipses < list_ptr->num_ellipses)
    {
        int i;

        for (i=num_ellipses; i<list_ptr->num_ellipses; i++)
        {
            free_ellipse(list_ptr->ellipses[ i ]);
        }
        list_ptr->num_ellipses = num_ellipses;
    }
    else
    {
        free_ellipse_list(list_ptr);
        NRE(list_ptr = create_ellipse_list(num_ellipses));
        *target_list_ptr_ptr = list_ptr;
    }

    return NO_ERROR;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             create_ellipse_list
 *
 * Creates a ellipse_list
 *
 * This routine creates a ellipse_list and returns a pointer to it.
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_create_ellipse_list, which is the version available in the developement
 * library. In developement code memory is tracked so that memory leaks can be
 * found more easily. Furthermore, all memory freed is checked that it was
 * allocated by an L library routine.
 *
 * The routine free_ellipse_list should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    One success it returns a pointer to the ellipse_list.
 *
 * Acknowlegement:
 *    This routine grew out of one written by Ian Harder. However, it is
 *    important to note that this routine is somewhat different than any
 *    of Ian's routines.
 *
 * Index: ellipse_lists
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

Ellipse_list* debug_create_ellipse_list
(
    int         num_ellipses,
    const char* file_name,
    int         line_number
)
{
    Ellipse_list* list_ptr;
    int i;


    if (num_ellipses <= 0)
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    NRN(list_ptr = DEBUG_TYPE_MALLOC(Ellipse_list, file_name, line_number));

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    list_ptr->ellipses = DEBUG_N_TYPE_MALLOC(Ellipse*, num_ellipses, file_name,
                                             line_number);

   if (list_ptr->ellipses == NULL)
   {
       kjb_free(list_ptr);
       return NULL;
   }

   list_ptr->num_ellipses = num_ellipses;

   for (i=0; i<num_ellipses; i++)
   {
       list_ptr->ellipses[ i ] = NULL;
   }

   return list_ptr;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

Ellipse_list* create_ellipse_list(int num_ellipses)
{
    Ellipse_list *list_ptr;
    int i;


    if (num_ellipses <= 0)
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    NRN(list_ptr = TYPE_MALLOC(Ellipse_list));

    list_ptr->ellipses = N_TYPE_MALLOC(Ellipse*, num_ellipses);

    if (list_ptr->ellipses == NULL)
    {
       kjb_free(list_ptr);
       return NULL;
    }

    list_ptr->num_ellipses = num_ellipses;


    for (i=0; i<num_ellipses; i++)
    {
        list_ptr->ellipses[ i ] = NULL;
    }

    return list_ptr;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_ellipse_list
 *
 * Frees the space associated with a ellipse_list.
 *
 * This routine frees the storage associated with a ellipse_list obtained from
 * create_ellipse_list (perhaps indirectly). If the argument is NULL, then this
 * routine returns safely without doing anything.
 *
 * Acknowlegement:
 *    This routine grew out of one written by Ian Harder. However, it is
 *    important to note that this routine is somewhat different than any
 *    of Ian's routines.
 *
 * Index: ellipse_lists
 *
 * -----------------------------------------------------------------------------
*/

void free_ellipse_list(Ellipse_list* list_ptr)
{

    if (list_ptr != NULL)
    {
        if (list_ptr->ellipses != NULL)
        {
            int num_ellipses = list_ptr->num_ellipses;
            int i;

#ifdef DISABLE
#ifdef TRACK_MEMORY_ALLOCATION
            /*
             * We must have a valid pointer to check the initialization,
             * otherwise problems such as double free can look like unitialized
             * memory.
            */
            kjb_check_free(list_ptr);

            check_initialization(list_ptr->ellipses, list_ptr->num_ellipses,
                                 sizeof(Ellipse));
#endif
#endif
            for (i=0; i<num_ellipses; i++)
            {
                free_ellipse(list_ptr->ellipses[ i ]);
            }

            kjb_free(list_ptr->ellipses);
        }
    }

    kjb_free(list_ptr);

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_ellipse(Ellipse* ellipse_ptr)
{

    if (ellipse_ptr != NULL)
    {
        free_matrix(ellipse_ptr->rotation_mp);
        free_vector(ellipse_ptr->offset_vp);

        kjb_free(ellipse_ptr);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_multiply_ellipse_by_scalar(Ellipse* ellipse_ptr, double factor)
{

    if (ellipse_ptr != NULL)
    {
        (ellipse_ptr->max_x) *= factor;
        (ellipse_ptr->max_y) *= factor;
        (ellipse_ptr->a) /= (factor * factor);
        (ellipse_ptr->b) /= (factor * factor);
        ERE(ow_multiply_vector_by_scalar(ellipse_ptr->offset_vp, factor));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


#ifdef __cplusplus
}
#endif

