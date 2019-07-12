
/* $Id: m_missing.c 20654 2016-05-05 23:13:43Z kobus $ */

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

#include "m/m_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "m/m_missing.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static Queue_element* fs_respect_missing_values_stack_head = NULL;

#ifdef TRACK_MEMORY_ALLOCATION
    static int        fs_first_respect_missing_values_stack_use = TRUE;
#endif

/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION
    static void free_respect_missing_values_stack(void);
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            respect_missing_values
 *
 * Returns the current setting for respecting missing values
 *
 * This routine returns the current current setting for respecting missing
 * values. If a routine wishes to cooperate with calling routines to perform
 * missing value computations when requested, then this routine should be called
 * to request the current state. The state is change with
 * enable_respect_missing_values(), disable_respect_missing_values() or
 * restore_respect_missing_values().
 *
 * Note:
 *    For performance, this is currently implemented as a macro which simply
 *    accesses a global variable.
 *
 * Returns:
 *    This routine returns either TRUE or FALSE
 *
 * Index: missing values
 *
 * -----------------------------------------------------------------------------
 */

#ifdef __C2MAN__
int respect_missing_values(void)
{
    IMPORT int kjb_respect_missing_values;

    return kjb_respect_missing_values;
}
#else
/* It is a macro! */
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            enable_respect_missing_values
 *
 * Requests cooperating routines to respect missing values
 *
 * This routine requests cooperating routines to respect missing values
 * (designated by DBL_MISSING). Some routines will check with this module if the
 * current state is to respect missing values, and they will do alternate
 * versions of calculations if that is the case. Whether this is the case will
 * be explained in the documentation for the particular routines.
 *
 * Notes:
 *     The facility operates as a stack. It is disabled by default. This pushes
 *     TRUE onto the stack.
 *
 *     The facility is not exported to the user in general, as this is too
 *     dangerous. However, it makes sense to have user controled options which
 *     selectively imply calls to this module.
 *
 * Returns:
 *    This routine returns NO_ERROR on success, and ERROR if there are
 *    unexpected problems.
 *
 * Index: missing values
 *
 * -----------------------------------------------------------------------------
 */

int enable_respect_missing_values(void)
{
    IMPORT int kjb_respect_missing_values;
    int*       save_respect_missing_values_ptr;
    int        result;


    SKIP_HEAP_CHECK_2();
    save_respect_missing_values_ptr = INT_MALLOC(1);
    CONTINUE_HEAP_CHECK_2();
    NRE(save_respect_missing_values_ptr);
    *save_respect_missing_values_ptr = kjb_respect_missing_values;

#ifdef TRACK_MEMORY_ALLOCATION
    if (fs_first_respect_missing_values_stack_use)
    {
        fs_first_respect_missing_values_stack_use = FALSE;
        add_cleanup_function(free_respect_missing_values_stack);
    }
#endif

    SKIP_HEAP_CHECK_2();
    result = insert_into_queue(&fs_respect_missing_values_stack_head,
                               (Queue_element**)NULL,
                               (void*)save_respect_missing_values_ptr);
    CONTINUE_HEAP_CHECK_2();
    ERE(result);

    kjb_respect_missing_values = TRUE;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            disable_respect_missing_values
 *
 * Requests cooperating routines to ignore the concept of missing values
 *
 * This routine requests that cooperating routines do not treat missing values
 * (designated by DBL_MISSING) as special.  Some routines will check with this
 * module if the current state is to respect missing values, and they will do
 * alternate versions of calculations if that is the case. Whether this is the
 * case will be explained in the documentation for the particular routines.
 *
 * Notes:
 *     The facility operates as a stack. It is disabled by default. This pushes
 *     TRUE onto the stack.
 *
 *     The facility is not exported to the user in general, as this is too
 *     dangerous. However, it makes sense to have user controled options which
 *     selectively imply calls to this module.
 *
 * Returns:
 *    This routine returns NO_ERROR on success, and ERROR if there are
 *    unexpected problems.
 *
 * Index: missing values
 *
 * -----------------------------------------------------------------------------
 */

int disable_respect_missing_values(void)
{
    IMPORT int kjb_respect_missing_values;
    int*       save_respect_missing_values_ptr;
    int        result;


    SKIP_HEAP_CHECK_2();
    save_respect_missing_values_ptr = INT_MALLOC(1);
    CONTINUE_HEAP_CHECK_2();
    NRE(save_respect_missing_values_ptr);
    *save_respect_missing_values_ptr = kjb_respect_missing_values;

#ifdef TRACK_MEMORY_ALLOCATION
    if (fs_first_respect_missing_values_stack_use)
    {
        fs_first_respect_missing_values_stack_use = FALSE;
        add_cleanup_function(free_respect_missing_values_stack);
    }
#endif

    SKIP_HEAP_CHECK_2();
    result = insert_into_queue(&fs_respect_missing_values_stack_head,
                               (Queue_element**)NULL,
                               (void*)save_respect_missing_values_ptr);
    CONTINUE_HEAP_CHECK_2();
    ERE(result);

    kjb_respect_missing_values = FALSE;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            restore_respect_missing_values
 *
 * Resets respect_missing_values to previous behaviour.
 *
 * This routine resets respect_missing_values to whatever it was previously, as
 * set by either enable_respect_missing_values(), or
 * disable_respect_missing_values().  If the entire stack has been used up, then
 * this routine silently does nothing.
 *
 * Returns:
 *    This routine returns TRUE if we are now respect_missing_values, FALSE if we are not, and
 *    ERROR if there are unexpected problems.
 *
 * Index: missing values
 *
 * -----------------------------------------------------------------------------
 */

int restore_respect_missing_values(void)
{
    Queue_element* cur_elem;


    if (fs_respect_missing_values_stack_head == NULL)
    {
#ifdef TEST
        set_bug("Restore respect missing values on empty stack.");
#endif
        return NO_ERROR;
    }

    NRE(cur_elem = remove_first_element(&fs_respect_missing_values_stack_head,
                                        (Queue_element**)NULL));

    kjb_respect_missing_values = *((int*)(cur_elem->contents));

    SKIP_HEAP_CHECK_2();
    free_queue_element(cur_elem, kjb_free);
    CONTINUE_HEAP_CHECK_2();

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int index_matrix_vector_rows_with_missing
(
    Int_vector_vector**  missing_index_vvpp,
    const Matrix_vector* mvp
)
{
    if (mvp == NULL)
    {
        free_int_vector_vector(*missing_index_vvpp);
        *missing_index_vvpp = NULL;
    }
    else
    {
        int len = mvp->length;
        int i;

        ERE(get_target_int_vector_vector(missing_index_vvpp, len));

        for (i = 0; i < len; i++)
        {
            ERE(index_matrix_rows_with_missing(&((*missing_index_vvpp)->elements[ i ]),
                                               mvp->elements[ i ]));
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int index_matrix_rows_with_missing
(
    Int_vector**  missing_index_vpp,
    const Matrix* mp
)
{
    if (mp == NULL)
    {
        free_int_vector(*missing_index_vpp);
        *missing_index_vpp = NULL;
    }
    else
    {
        int num_rows = mp->num_rows;
        int num_cols = mp->num_cols;
        int i, j;

        ERE(get_initialized_int_vector(missing_index_vpp, num_rows, FALSE));

        if ( ! respect_missing_values()) return NO_ERROR;

        for (i = 0; i < num_rows; i++)
        {
            for (j = 0; j < num_cols; j++)
            {
                if (IS_MISSING_DBL(mp->elements[ i ][ j ]))
                {
                    (*missing_index_vpp)->elements[ i ] = TRUE;
                    break;
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION
/*
 * =============================================================================
 * STATIC                      free_respect_missing_values_stack
 * -----------------------------------------------------------------------------
 */

static void free_respect_missing_values_stack(void)
{
    free_queue(&fs_respect_missing_values_stack_head, (Queue_element**)NULL,
               kjb_free);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

