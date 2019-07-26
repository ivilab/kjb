
/* $Id: l_int_vector.c 21520 2017-07-22 15:09:04Z kobus $ */

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

#include "l/l_gen.h"      /*  Only safe if first #include in a ".c" file  */

#include "l/l_io.h"
#include "l/l_parse.h"
#include "l/l_sys_scan.h"
#include "l/l_sys_rand.h"
#include "l/l_sort.h"
#include "l/l_int_vector.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define KJB_DATA_HEAD_SIZE  64
#define KJB_RAW_INT_VECTOR_STRING  "kjb raw int vector\n\n\f\n"
#define KJB_RAW_INT_VECTOR_VECTOR_STRING  "kjb raw int vector vector\n\n\f\n"

/* -------------------------------------------------------------------------- */

static int fp_read_ascii_int_vector_2(Int_vector** vpp, FILE* fp, int length);

static int ascend_compare_indexed_int_vector_elements
(
    const void* first_ptr,
    const void* second_ptr
);

static int descend_compare_indexed_int_vector_elements
(
    const void* first_ptr,
    const void* second_ptr
);

static Int_vector_vector* create_int_vector_vector(int count);
static Int_v_v_v*         create_int_v3           (int length);

/* -------------------------------------------------------------------------- */

#ifdef TEST
double debug_int_vec_val
(
 const Int_vector* vp,
 int           i,
 const char*   file_name,
 int           line_number
)
{
    UNTESTED_CODE();

    if ((i < 0) || (i >= vp->length))
    {

        set_bug("Bounds check error on line %d of file %s.",
                line_number, file_name);
        return 0.0;
    }
    else
    {
        return vp->elements[i];
    }
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_zero_int_vector
 *
 * Gets zero integer verctor
 *
 * This routine is similar to get_target_int_vector(), except that the integer
 * elements are set to zero.
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_zero_int_vector(Int_vector** vpp, int len)
{


    ERE(get_target_int_vector(vpp, len));

    return ow_zero_int_vector(*vpp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_unity_int_vector
 *
 * Gets integer verctor initialized to ones.
 *
 * This routine is similar to get_target_int_vector(), except that the integer
 * elements are set to one.
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_unity_int_vector(Int_vector** vpp, int len)
{


    ERE(get_target_int_vector(vpp, len));

    return ow_set_int_vector(*vpp, 1);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_initialized_int_vector
 *
 * Gets initialzed integer verctor
 *
 * This routine is similar to get_target_int_vector(), except that the elements
 * of the integer list are set to the specitied value.
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_initialized_int_vector(Int_vector** vpp, int len, int initial_value)
{



    ERE(get_target_int_vector(vpp, len));
    ERE(ow_set_int_vector(*vpp, initial_value));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef FREE_BEFORE_RESIZE

/* =============================================================================
 *                                get_target_int_vector
 *
 * Gets target integer list
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of integer list.. If *target_vpp is NULL, then this
 * routine creates the integer list. If it is not null, and it is the right
 * size, then this routine does nothing. If it is the wrong size, then it is
 * resized.
 *
 * In order to be memory efficient, we free before resizing. If the resizing
 * fails, then the original contents of the *target_vpp will be lost.
 * However, (*target_vpp)->elements will be set to NULL, so (*target_vpp)
 * can be safely sent to free_int_vector().
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
*/

#else

/* =============================================================================
 *                          get_target_int_vector
 *
 * Gets a target integer vector
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of integer list.. If *target_vpp is NULL, then this
 * routine creates the integer list. If it is not null, and it is the right
 * size, then this routine does nothing. If it is the wrong size, then it is
 * resized.
 *
 * If an actual resize is needed, then a new integer list of the required size
 * is first created. If the creation is successful, then the old integer list
 * is free'd.  The reason is that if the new allocation fails, a calling
 * application should still be able to use the old integer list. The alternate
 * is to free the old integer list first.  This is more memory efficient. A
 * more sophisticated alternative is to free the old integer list if it can be
 * deterimined that the subsequent allocation will succeed. Although such
 * approaches have merit, it is expected that resizing will occur infrequently
 * enough that it is not worth implementing them. Thus the simplest method
 * with good semantics under most conditions has been used.
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
*/

#endif


#ifdef TRACK_MEMORY_ALLOCATION
int debug_get_target_int_vector(Int_vector** target_vpp, int length,
                                const char* file_name, int line_number)
{
    Int_vector* vp = *target_vpp;


    if (length < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (vp == NULL)
    {
        NRE(vp = debug_create_int_vector(length, file_name, line_number));
        *target_vpp = vp;
    }
    else if (length == vp->length)
    {
        /*EMPTY*/
        ; /* Do nothing */
    }
    else if (length < vp->max_length)
    {
        vp->length = length;
    }
    else
    {
        kjb_free(vp->elements);

        if (length > 0)
        {
            NRE(vp->elements = DEBUG_INT_MALLOC(length, file_name,
                                                line_number));
        }
        else
        {
            vp->elements = NULL;
        }

        vp->length = length;
        vp->max_length = length;
    }

    return NO_ERROR;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int get_target_int_vector(Int_vector** target_vpp, int length)
{
    Int_vector* vp = *target_vpp;


    if (length < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (vp == NULL)
    {
        NRE(vp = create_int_vector(length));
        *target_vpp = vp;
    }
    else if (length <= vp->max_length)
    {
        vp->length = length;
    }
    else
    {
        kjb_free(vp->elements);

        if (length > 0)
        {
            NRE(vp->elements = INT_MALLOC(length));
        }
        else
        {
            vp->elements = NULL;
        }

        vp->length = length;
        vp->max_length = length;
    }

    return NO_ERROR;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           ra_get_target_int_vector
 *
 * Gets a target integer vector
 *
 * This routine is similar to get_target_int_vector(), except that reallocation
 * is used when appropriate, via kjb_realloc(). This means that an existing
 * vector argument size is increased without disturbing existing elements. Also
 * different than get_target_int_vector(), if the vector size is being
 * decreased, then the amount of storage is decreased. By contrast,
 * get_target_int_vector() shrinks a vector by simply changing the "length"
 * field. The behaviour of get_target_int_vector() is more efficient, but it can
 * lead to using more memory than needed. Since it is sometimes helfpul to be
 * able to trim the memory to the minimum needed, we implement this different
 * behavior as part of this function.
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION
int debug_ra_get_target_int_vector(Int_vector** target_vpp, int length,
                                   const char* file_name, int line_number)
{
    Int_vector* vp = *target_vpp;


    if (length < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if ((vp == NULL) || (length == 0))
    {
        return debug_get_target_int_vector(target_vpp, length,
                                           file_name, line_number);
    }
    else if (length == vp->length)
    {
        /*EMPTY*/
        ; /* Do nothing */
    }
    else
    {
        NRE(vp->elements = DEBUG_INT_REALLOC(vp->elements,
                                             length, file_name,
                                             line_number));
        vp->length = length;
        vp->max_length = length;
    }

    return NO_ERROR;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int ra_get_target_int_vector(Int_vector** target_vpp, int length)
{
    Int_vector* vp = *target_vpp;


    if (length < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if ((vp == NULL) || (length == 0))
    {
        return get_target_int_vector(target_vpp, length);
    }
    else if (length == vp->max_length)
    {
        /*EMPTY*/
        ; /* Do nothing */
    }
    else
    {
        NRE(vp->elements = INT_REALLOC(vp->elements, length));

        vp->length = length;
        vp->max_length = length;
    }

    return NO_ERROR;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * Create confuses. Should likely be static. But we need to purge a few uses
 * first.
*/
#ifdef DOCUMENT_CREATE_VERSIONS
/* =============================================================================
 *                             create_int_vector
 *
 * Creates an integer list
 *
 * This routine creates an integer list and returns a pointer to it.
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_create_int_vector, which is the version available in the developement
 * library. In developement code memory is tracked so that memory leaks can be
 * found more easily. Furthermore, all memory freed is checked that it was
 * allocated by an L library routine.
 *
 * The routine free_int_vector should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    One success it returns a pointer to the integer list.
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
*/
#endif

#ifdef TRACK_MEMORY_ALLOCATION

Int_vector* debug_create_int_vector(int len,
                                    const char* file_name, int line_number)
{
    Int_vector* vp;


    if (len < 0)
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    NRN(vp = DEBUG_TYPE_MALLOC(Int_vector, file_name, line_number));

    if (len > 0)
    {
        /*
        //  Use debug_kjb_malloc as opposed to macros to pass down file_name
        //  and line_number.
        */
        vp->elements = DEBUG_INT_MALLOC(len, file_name, line_number);

        if (vp->elements == NULL)
        {
            kjb_free(vp);
            return NULL;
        }
    }
    else
    {
        vp->elements = NULL;
    }

    vp->length = len;
    vp->max_length = len;

    return vp;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

Int_vector* create_int_vector(int len)
{
    Int_vector* vp;


    if (len < 0)
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    NRN(vp = TYPE_MALLOC(Int_vector));

    vp->elements = N_TYPE_MALLOC(int, len);

    if (len > 0)
    {
        if (vp->elements == NULL)
        {
           kjb_free(vp);
           return NULL;
        }
    }
    else
    {
        vp->elements = NULL;
    }

    vp->length = len;
    vp->max_length = len;

    return vp;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_int_vector
 *
 * Frees the space associated with a integer list
 *
 * This routine frees the storage associated with an integer vector.  If the
 * argument is NULL, then this routine returns safely without doing anything.
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
*/

void free_int_vector(Int_vector* vp)
{

    if (vp != NULL)
    {
        if (vp->elements != NULL)
        {
#if 0 /* was ifdef DISABLE */
#ifdef TRACK_MEMORY_ALLOCATION
            UNTESTED_CODE();

            check_initialization(vp->elements, vp->length, sizeof(int));
#endif
#endif
            kjb_free(vp->elements);
        }
    }

    kjb_free(vp);

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#if 0 /* was ifdef OBSOLETE */
Int_vector* create_zero_int_vector(int length)
{
    Int_vector* vp;


    UNTESTED_CODE();

    NRN(vp = create_int_vector(length));

    if (ow_zero_int_vector(vp) == ERROR)
    {
        free_int_vector(vp);
        return NULL;
    }

    return vp;
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_zero_int_vector(Int_vector* vp)
{


    return ow_set_int_vector(vp, 0);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_set_int_vector(Int_vector* vp, int set_val)
{
    int i;
    int len;
    int* vec_pos;


    if ((vp == NULL) || (vp->length < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    len = vp->length;
    vec_pos = vp->elements;

    for (i=0; i<len; i++)
    {
        *vec_pos = set_val;
        vec_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ascend_sort_int_vector(Int_vector* vp)
{


    UNTESTED_CODE();

    ERE(int_sort(vp->elements, vp->length, sizeof(int), 0,
                 USE_CURRENT_ATN_HANDLING));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ascend_sort_indexed_int_vector(Indexed_int_vector *vp)
{


    UNTESTED_CODE();

    ERE(kjb_sort(vp->elements, vp->length, sizeof(Indexed_int_vector_element),
                 ascend_compare_indexed_int_vector_elements,
                 USE_CURRENT_ATN_HANDLING));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int ascend_compare_indexed_int_vector_elements(const void* first_ptr,
                                                      const void* second_ptr)
{
    const Indexed_int_vector_element *elem1_ptr = (const Indexed_int_vector_element*)first_ptr;
    const Indexed_int_vector_element *elem2_ptr = (const Indexed_int_vector_element*)second_ptr;


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

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int descend_sort_indexed_int_vector(Indexed_int_vector *vp)
{


    UNTESTED_CODE();

    ERE(kjb_sort(vp->elements, vp->length, sizeof(Indexed_int_vector_element),
                 descend_compare_indexed_int_vector_elements,
                 USE_CURRENT_ATN_HANDLING));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int descend_compare_indexed_int_vector_elements(const void* first_ptr,
                                                       const void* second_ptr)
{
    const Indexed_int_vector_element *elem1_ptr = (const Indexed_int_vector_element*)first_ptr;
    const Indexed_int_vector_element *elem2_ptr = (const Indexed_int_vector_element*)second_ptr;


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

/* =============================================================================
 *                               vp_get_indexed_int_vector
 *
 * Gets an indexed version of an integer vector
 *
 * This routine puts an indexed version of the integer vector init_vp, into
 * *vpp, allocating memory if needed. If init_vp is NULL, then *vpp will be
 * freed and become NULL also.
 *
 * The elements structure member of the Indexed_int_vector_element structure
 * become the same as those in init_vp, and the index structure member become
 * the index of the init_vp (i.e., 0, 1, 2, 3).
 *
 * Index: indexed vectors, vectors
 *
 * -----------------------------------------------------------------------------
*/

int vp_get_indexed_int_vector
(
    Indexed_int_vector** vpp,
    const Int_vector*    init_vp
)
{
    Indexed_int_vector_element* vp_pos;
    int                         length;
    int                         i;


    if (init_vp == NULL)
    {
        free_indexed_int_vector(*vpp);
        *vpp = NULL;
        return NO_ERROR;
    }

    length = init_vp->length;

    ERE(get_target_indexed_int_vector(vpp, length));

    vp_pos = (*vpp)->elements;

    for (i=0; i<length; i++)
    {
        vp_pos->index = i;
        vp_pos->element = init_vp->elements[ i ];
        vp_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_zero_indexed_int_vector
 *
 * Gets an indexed vector set to zeros
 *
 * This routine sets up an indexed vector of the specified length, and sets the
 * element member of the Indexed_int_vector_element structure to zeros, and the
 * index structure member to 0, 1, 2, 3, ... ,  length-1.
 *
 * Index: indexed vectors, vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_zero_indexed_int_vector(Indexed_int_vector** vpp, int length)
{
    Indexed_int_vector_element* vp_pos;
    int                     i;


    ERE(get_target_indexed_int_vector(vpp, length));

    vp_pos = (*vpp)->elements;

    for (i=0; i<length; i++)
    {
        vp_pos->element = 0;
        vp_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               get_target_indexed_int_vector
 *
 * Gets an indexed vector
 *
 * This routine sets up an indexed vector of the specified length, and sets the
 * index structure member to 0, 1, 2, 3, ... , length-1. The element structure
 * members are left uninitialized.
 *
 * Index: indexed vectors, vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_target_indexed_int_vector(Indexed_int_vector** vpp, int length)
{
    Indexed_int_vector*         vp     = *vpp;
    Indexed_int_vector_element* vp_pos;
    int                     i;


    if (length < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (vp == NULL)
    {
        NRE(vp = create_indexed_int_vector(length));
        *vpp = vp;
    }
    else if (length == vp->length)
    {
        /*EMPTY*/
        ; /* Do nothing */
    }
    else if (length < vp->length)
    {
        vp->length = length;
    }
    else
    {
        kjb_free(vp->elements);

        if (length > 0)
        {
            vp->elements = N_TYPE_MALLOC(Indexed_int_vector_element, length);
        }
        else
        {
            vp->elements = NULL;
        }

        vp->length = length;
    }

    vp_pos = (*vpp)->elements;

    for (i=0; i<length; i++)
    {
        vp_pos->index = i;
        vp_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Indexed_int_vector* vp_create_indexed_int_vector(const Int_vector* vp)
{
    Indexed_int_vector* result_ptr;
    int len, i;


    UNTESTED_CODE();

    NRN(result_ptr = TYPE_MALLOC(Indexed_int_vector));

    len = vp->length;

    result_ptr->length = len;
    result_ptr->elements = N_TYPE_MALLOC(Indexed_int_vector_element, len);

    if (result_ptr->elements == NULL)
    {
        kjb_free(result_ptr);
        return NULL;
    }

    for (i=0; i<len; i++)
    {
        result_ptr->elements[i].index = i;
        result_ptr->elements[i].element = vp->elements[ i ];
    }

    return result_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Indexed_int_vector* create_indexed_int_vector(int len)
{
    Indexed_int_vector* result_ptr;
    int i;


    NRN(result_ptr = TYPE_MALLOC(Indexed_int_vector));

    result_ptr->length = len;
    result_ptr->elements = N_TYPE_MALLOC(Indexed_int_vector_element, len);

    if (result_ptr->elements == NULL)
    {
        kjb_free(result_ptr);
        return NULL;
    }

    for (i=0; i<len; i++)
    {
        result_ptr->elements[i].index = i;
    }

    return result_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_indexed_int_vector(Indexed_int_vector *vp)
{

    UNTESTED_CODE();

    if (vp != NULL)
    {
        kjb_free(vp->elements);
        kjb_free(vp);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              add_int_vectors
 *
 * Performs element-wise addition of two int vectors.
 *
 * This routine performs element-wise addition of two vectors. The second
 * vector must fit into the first an integral number of times.The second
 * vector is added to each block in the first vector.
 *
 * If we are respecting missing values, then the sqrt() of a missing value
 * (DBL_MISSING) is DBL_MISSING.
 *
 * The first argument is the adress of the target vector. If the target vector
 * itself is null, then a vector of the appropriate size is created. If the
 * target vector is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int add_int_vectors
(
    Int_vector**      target_vpp,
    const Int_vector* first_vp,
    const Int_vector* second_vp
)
{
    int* target_elem_ptr;
    int* first_elem_ptr;
    int* second_elem_ptr;
    int   i;
    int   block;
    int   length;
    int   second_length;
    int   factor;


    length = first_vp->length;

    second_length = second_vp->length;
    factor = length / second_length;

    if (length != factor * second_length)
    {
        set_bug("A vector of length %d can't be added to one of length %d.",
                second_length, length);
        return ERROR;
    }

    ERE(get_target_int_vector(target_vpp, length));
    target_elem_ptr = (*target_vpp)->elements;
    first_elem_ptr = first_vp->elements;

    for (block=0; block<factor; block++)
    {
        second_elem_ptr = second_vp->elements;

        for (i=0; i<second_length; i++)
        {
            *target_elem_ptr = (*first_elem_ptr) + (*second_elem_ptr);

            target_elem_ptr++;
            first_elem_ptr++;
            second_elem_ptr++;
        }

    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              subtract_int_vectors
 *
 * Performs element-wise subtraction of two int vectors.
 *
 * This routine performs element-wise subtraction of two vectors. The second
 * vector must fit into the first an integral number of times.The second
 * vector is added to each block in the first vector.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * The first argument is the adress of the target vector. If the target vector
 * itself is null, then a vector of the appropriate size is created. If the
 * target vector is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int subtract_int_vectors
(
    Int_vector**      target_vpp,
    const Int_vector* first_vp,
    const Int_vector* second_vp
)
{
    int* target_elem_ptr;
    int* first_elem_ptr;
    int* second_elem_ptr;
    int   i;
    int   block;
    int   length;
    int   second_length;
    int   factor;


    length = first_vp->length;

    second_length = second_vp->length;
    factor = length / second_length;

    if (length != factor * second_length)
    {
        set_bug(
            "A vector of length %d can't be subtracted from one of length %d.",
            second_length, length);
        return ERROR;
    }

    ERE(get_target_int_vector(target_vpp, length));
    target_elem_ptr = (*target_vpp)->elements;
    first_elem_ptr = first_vp->elements;

    for (block=0; block<factor; block++)
    {
        second_elem_ptr = second_vp->elements;

        for (i=0; i<second_length; i++)
        {
            *target_elem_ptr = (*first_elem_ptr) - (*second_elem_ptr);

            target_elem_ptr++;
            first_elem_ptr++;
            second_elem_ptr++;
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_add_int_vectors
 *
 * Adds the second int vector to the first.
 *
 * This routine adds the second vector to the first. The second vector must fit
 * into the first an integral number of times. The second vector is added to
 * each block in the first vector.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_int_vectors(Int_vector* first_vp, const Int_vector* second_vp)
{
    int* first_elem_ptr;
    int* second_elem_ptr;
    int   i;
    int   block;
    int   length;
    int   second_length;
    int   factor;


    length = first_vp->length;

    second_length = second_vp->length;
    factor = length / second_length;

    if (length != factor * second_length)
    {
        set_bug("A vector of length %d can't be added to one of length %d.",
                second_length, length);
        return ERROR;
    }

    first_elem_ptr = first_vp->elements;

    for (block=0; block<factor; block++)
    {
        second_elem_ptr = second_vp->elements;

        for (i=0; i<second_length; i++)
        {
            *first_elem_ptr += (*second_elem_ptr);

            first_elem_ptr++;
            second_elem_ptr++;
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_subtract_int_vectors
 *
 * Subtracts the second int vector from the first.
 *
 * This routine subtracts the second vector from the first. The second vector
 * must fit into the first an integral number of times. The second vector is
 * added to each block in the first vector.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_subtract_int_vectors(Int_vector* first_vp, const Int_vector* second_vp)
{
    int* first_elem_ptr;
    int* second_elem_ptr;
    int   i;
    int   block;
    int   length;
    int   second_length;
    int   factor;


    length = first_vp->length;

    second_length = second_vp->length;
    factor = length / second_length;

    if (length != factor * second_length)
    {
        set_bug(
            "A vector of length %d can't be subtracted from one of length %d.",
             second_length, length);
        return ERROR;
    }

    first_elem_ptr = first_vp->elements;

    for (block=0; block<factor; block++)
    {
        second_elem_ptr = second_vp->elements;

        for (i=0; i<second_length; i++)
        {
            *first_elem_ptr -= (*second_elem_ptr);

            first_elem_ptr++;
            second_elem_ptr++;
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_random_index_vector
 *
 * Sets a vector with random zeros and ones
 *
 * This routine creates a vector of length "length", which has a random
 * approximate fraction "fraction" of ones, with the other elements being zero.
 *
 * If *target_vpp is NULL, then this routine creates the integer vector.  If it
 * is not null, and it is the right size, then the storage is recycled.  If the
 * wrong size, then it is resized.
 *
 * The routine free_int_vector should be used to dispose of the storage once the
 * targets are no longer needed
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the two target arrays.
 *
 * Author:
 *    Kobus Barnard
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
 */

int get_random_index_vector
(
     Int_vector** target_vpp,
     int      length,
     double   fraction
)
{
    int i;


    if ((fraction < 0.0) || (fraction > 1.0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_zero_int_vector(target_vpp, length));

    for (i=0; i<length; i++)
    {
        if (kjb_rand() < fraction)
        {
            (*target_vpp)->elements[ i ] = 1;
        }
    }

    return NO_ERROR;
}

/* =============================================================================
 *                             split_int_vector_vector
 *
 * Splits an integer vector vector into two target vector vectors
 *
 * This routine splits an integer vector vector with the creation/over-writing
 * semantics used in the KJB library in the case of integer vector arrays. If
 * *target_1_vvpp or *target_2_vvpp is NULL, then this routine creates the
 * integer vector vector.  If they  are not null, and are the right size, then
 * the storage is recycled.  If the wrong size, then they are resized.
 *
 * The routine free_int_vector_vector should be used to dispose of the storage
 * once the targets are no longer needed
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the two target arrays.
 *
 * Author:
 *    Ranjini Swaminathan
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
 */

int split_int_vector_vector
(
    Int_vector_vector**      target_1_vvpp,
    Int_vector_vector**      target_2_vvpp,
    const Int_vector_vector* source_vvp,
    const Int_vector*        index_list_vp
)
{
    int length;
    int index_list_length;
    int i;

    int target_1_index = 0;
    int target_2_index = 0;

    if (source_vvp == NULL)
    {
        free_int_vector_vector(*target_1_vvpp);
        free_int_vector_vector(*target_2_vvpp);
        *target_1_vvpp = NULL;
        *target_2_vvpp = NULL;
        return NO_ERROR;
    }

    length = source_vvp->length;
    index_list_length = index_list_vp->length;

    ERE(get_target_int_vector_vector(target_1_vvpp, index_list_length));
    ERE(get_target_int_vector_vector(target_2_vvpp, length - index_list_length));

    for (i = 0; i < length; i++)
    {
        /*
        // Ranjini : [ ASSUMPTION ] index list is sorted
         */
        if(i == index_list_vp->elements[target_1_index])
        {
            ERE(copy_int_vector(&((*target_1_vvpp)->elements[ target_1_index ]),
                                source_vvp->elements[ i ]));
            target_1_index++;
        }
        else
        {
            ERE(copy_int_vector(&((*target_1_vvpp)->elements[ target_2_index ]),
                                source_vvp->elements[ i ]));
            target_2_index++;
        }

    }

    return NO_ERROR;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           split_int_vector
 *
 * Splits an integer vector
 *
 * This routine splits the integer vector pointed to by source_vp to the
 * integer vectors pointed to by *target_1_vp and *target_2_vp based on
 * a list of elements given by index_list_vp. If *target_1_vp or
 * target_2_vp is NULL, then it is created. If it is the wrong size, it is resized.
 *
 * Returns:
 *    On success, this routine returns a pointer to two newly created integer
 *    lists which are obtained by splitting  a copy of the input integer list.
 *    On failure, it returns NULL, and sets an error message. Currently this
 *    routine can only fail if storage allocation fails.
 *
 * Author:
 *    Ranjini Swaminathan
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
 */

int split_int_vector
(
    Int_vector** target_1_vpp,
    Int_vector** target_2_vpp,
    const Int_vector* source_vp,
    const Int_vector* index_list_vp
)
{
    int   i;
    int   length;
    int   index_list_length;
    int source_index = 0;
    int target_1_index = 0;
    int target_2_index = 0;

    UNTESTED_CODE();

    if (source_vp == NULL)
    {
        free_int_vector(*target_1_vpp);
        *target_1_vpp = NULL;
        free_int_vector(*target_2_vpp);
        *target_2_vpp = NULL;
        return NO_ERROR;
    }

    length = source_vp->length;
    index_list_length = index_list_vp->length;

    if(index_list_length >= length)
    {
        EPETE(copy_int_vector(target_1_vpp,source_vp));
        free_int_vector(*target_2_vpp);
        *target_2_vpp = NULL;
        return NO_ERROR;
    }
    ERE(get_target_int_vector(target_1_vpp, index_list_length));
    ERE(get_target_int_vector(target_2_vpp, length - index_list_length));

    for (i=0; i<length; i++)
    {
        if( i == index_list_vp->elements[source_index])
        {
            (*target_1_vpp)->elements[target_1_index] = source_vp->elements[i];
            source_index++;
            target_1_index++;
        }
        else
        {
            (*target_2_vpp)->elements[target_2_index] = source_vp->elements[i];
            target_2_index++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           copy_int_vector_vector
 *
 * Copies an integer vector vector
 *
 * This routine copies the integer vector pvector ointed to by source_vvp to
 * the integer vector pointed to by *target_vp. If *target_vp is NULL, then it
 * is created. If it is the wrong size, it is resized.
 *
 * Returns:
 *    On success, this routine returns a pointer to a newly created integer
 *    list which is a copy of the input integer list. On failure, it returns
 *    NULL, and sets an error message. Currently this routine can only fail if
 *    storage allocation fails.
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
 */

int copy_int_vector_vector
(
    Int_vector_vector** target_vvpp,
    const Int_vector_vector* source_vvp
)
{
    int   i;
    int   length;


    if (source_vvp == NULL)
    {
        free_int_vector_vector(*target_vvpp);
        *target_vvpp = NULL;
        return NO_ERROR;
    }

    length = source_vvp->length;

    ERE(get_target_int_vector_vector(target_vvpp, length));

    for (i=0; i<length; i++)
    {
        ERE(copy_int_vector(&((*target_vvpp)->elements[ i ]),
                            source_vvp->elements[ i ]));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#if 0 /* was ifdef OBSOLETE */
/*
 * =============================================================================
 *                           create_int_vector_copy
 *
 * Creates a copy of an integer vector
 *
 * This routine creates a copy of the input integer vector, and returns a
 * pointer to it.
 *
 * Returns:
 *    On success, this routine returns a pointer to a newly created integer
 *    list which is a copy of the input integer list. On failure, it returns
 *    NULL, and sets an error message. Currently this routine can only fail if
 *    storage allocation fails.
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
 */

Int_vector* create_int_vector_copy(Int_vector* source_vp)
{
    Int_vector* target_vp = NULL;


    UNTESTED_CODE();

    ERN(copy_int_vector(&target_vp, source_vp));

    return target_vp;
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           copy_int_vector
 *
 * Copies an integer vector
 *
 * This routine copies the integer vector pointed to by source_vp to the
 * integer vector pointed to by *target_vp. If *target_vpp is NULL, then it is
 * created. If it is the wrong size, it is resized.
 * If source_vp is NULL and *target_vpp is not, then *target_vpp is freed.
 *
 * Returns:
 *    On success, this routine returns NO_ERROR.
 *    On failure, this routine returns ERROR and sets an error message.
 *    Currently this routine can only fail if
 *    storage allocation fails.
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
 */

int copy_int_vector(Int_vector** target_vpp, const Int_vector* source_vp)
{
    int   i;
    int   length;
    int* target_pos;
    int* source_pos;

    if (source_vp == NULL)
    {
        free_int_vector(*target_vpp);
        *target_vpp = NULL;
        return NO_ERROR;
    }

    length = source_vp->length;

    ERE(get_target_int_vector(target_vpp, length));

    source_pos = source_vp->elements;
    target_pos = (*target_vpp)->elements;

    for (i=0; i<length; i++)
    {
        *target_pos++ = *source_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           copy_int_vector_section
 *
 * Copies an integer list
 *
 * This routine copies part of the integer vector pointed to by source_vp to
 * the integer vector pointed to by *target_vpp. "length" elements are copied
 * starting with "first_elem."  If *target_vpp is NULL, then it is created.
 * If it is the wrong size, it is resized.  If source_vp equals NULL and
 * *target_vpp does not, *target_vpp is freed.
 *
 * Returns:
 *    On success, this routine returns NO_ERROR.
 *    On failure, this routine returns ERROR and sets an error message.
 *    If the specified range goes outside the bounds of source_vp, set_bug()
 *    is also called.
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
 */

int copy_int_vector_section(
    Int_vector** target_vpp,
    const Int_vector* source_vp,
    int first_elem,
    int length
)
{
    int   i;
    int* target_pos;
    int* source_pos;


    if (source_vp == NULL)
    {
        free_int_vector(*target_vpp);
        *target_vpp = NULL;
        return NO_ERROR;
    }

    if (    (first_elem < 0)
         || (first_elem + length > source_vp->length)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_int_vector(target_vpp, length));

    source_pos = source_vp->elements + first_elem;
    target_pos = (*target_vpp)->elements;

    for (i=0; i<length; i++)
    {
        *target_pos++ = *source_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               read_int_vector
 *
 * Reads data from a file specified by name into an integer vector
 *
 * This routine reads data contained in a file specified by its name into an
 * integer vector. The length of the new vector is determined by the number of
 * data elements contained in the input file. The integer vector pointed to by
 * *vpp is created or resized as necessary.
 *
 * "file_name" points to a character array specifying the name of the file
 * to read the data from. If the "file_name" argument is NULL, or the first
 * character is null, input is expected from STDIN instead of a file.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Related:
 *     fp_read_int_vector, fp_read_int_vector
 *
 * Index: I/O,  integer vectors,  integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_int_vector(Int_vector** vpp, const char* file_name)
{
    FILE* fp;
    int   result;


    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdin;
    }
    else
    {
        NRE(fp = kjb_fopen(file_name, "r"));
    }

    result = fp_read_int_vector(vpp, fp);

    /*
    //  The file pointer based read routines can return EOF because they are
    //  used as building blocks. But for name based read routines, we expect at
    //  least one vector.
    */
    if (result == EOF)
    {
        set_error("Unable to read an integer vector file from %F.", fp);
        result = ERROR;
    }

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        (void)kjb_fclose(fp);  /* Ignore return--only reading. */
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           fp_read_int_vector
 *
 * Reads data from a file into an integer vector
 *
 * This routine reads data contained in a file specified by a pointer to FILE
 * into an integer vector that is created or resized if needed. The length of
 * the result vector is determined by the number of data elements contained in
 * the input file.
 *
 * "fp" points to a FILE that corresponds to the stream to read from.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: I/O,  integer vectors,  integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_int_vector(Int_vector** result_vpp, FILE* fp)
{
    int       result        = NOT_FOUND;
    Path_type path_type;
    long      save_file_pos;


    path_type = fp_get_path_type(fp);

    if (path_type == PATH_ERROR)
    {
        result = ERROR;
    }

    if (path_type != PATH_IS_REGULAR_FILE)
    {
        set_error("Input %F is not a regular file.", fp);
        add_error("Read of vector aborted.");
        result = ERROR;
    }

    ERE(save_file_pos = kjb_ftell(fp));

    if (result == NOT_FOUND)
    {
        result = fp_read_raw_int_vector(result_vpp, fp);

        if (result == NOT_FOUND)
        {
            ERE(kjb_fseek(fp, save_file_pos, SEEK_SET));
        }
    }

    if (result == NOT_FOUND)
    {
        result = fp_read_int_vector_with_header(result_vpp, fp);

        if (result == NOT_FOUND)
        {
            ERE(kjb_fseek(fp, save_file_pos, SEEK_SET));
        }
    }

    if (result == NOT_FOUND)
    {
        result = fp_read_ascii_int_vector(result_vpp, fp);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           fp_read_raw_int_vector
 *
 * Reads raw data from a file into an integer vector
 *
 * This routine reads data contained in a file specified by a pointer to FILE
 * into an integer vector that is created dynamically if needed. The length of the
 * result vector is determined by the number of data elements contained in the
 * input file.
 *
 * "fp" points to a FILE that corresponds to the stream to read from.
 *
 * Returns:
 *     NO_ERROR on success, NOT_FOUND if the file does not appear to contain a
 *     valid KJB library format raw integer vector, and ERROR on failure, with
 *     an error message being set.
 *
 * Index: I/O,  integer vectors,  integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_raw_int_vector(Int_vector** result_vpp, FILE* fp)
{
    int   length;
    long  bytes_used_so_far;
    int   byte_order;
    char  head_str[ KJB_DATA_HEAD_SIZE ];
    int   pad;
    off_t num_bytes;


    ERE(fp_get_byte_size(fp, &num_bytes));
    ERE(bytes_used_so_far = kjb_ftell(fp));

    num_bytes -= bytes_used_so_far;

    if (num_bytes < KJB_DATA_HEAD_SIZE) return NOT_FOUND;

    ERE(kjb_fread_exact(fp, head_str, sizeof(head_str)));
    head_str[ sizeof(head_str) - 1 ] = '\0';

    if ( ! STRCMP_EQ(head_str, KJB_RAW_INT_VECTOR_STRING))
    {
        return NOT_FOUND;
    }

    ERE(FIELD_READ(fp, byte_order));
    ERE(FIELD_READ(fp, pad));
    ERE(FIELD_READ(fp, length));
    ERE(FIELD_READ(fp, pad));

    ERE(fp_get_byte_size(fp, &num_bytes));
    ERE(bytes_used_so_far = kjb_ftell(fp));

    num_bytes -= bytes_used_so_far;

    if (length == 0)
    {
        /* Special case: The vector is mean to be null. */
        free_int_vector(*result_vpp);
        *result_vpp = NULL;
    }
    else if (length < 0)
    {
        set_error("Invalid data in vector file %F.");
        add_error("The length is negative ( %d ).", length);
        return ERROR;
    }
    else if (num_bytes < (off_t)(length * sizeof(int)))
    {
        set_error("Invalid data in vector file %F.");
        add_error("The number of bytes should be at least %d.",
                  bytes_used_so_far + length * sizeof(int));
        return ERROR;
    }
    else
    {
        ERE(get_target_int_vector(result_vpp, length));

        if (length > 0)
        {
            ERE(kjb_fread_exact(fp, (*result_vpp)->elements,
                                length * sizeof(int)));
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           fp_read_int_vector_with_header
 *
 * Reads ascii data from an input stream into an integer vector
 *
 * This routine reads data contained in a file specified by a pointer to FILE
 * into an integer vector. The length of the new vector is determined by the
 * number of data elements contained in the input file.  The vector *vpp is
 * created or resized as necessary.
 *
 * "vpp" is a pointer to a integer vector pointer.
 *
 * "fp" points to a FILE that corresponds to the stream to read from.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Related:
 *     read_int_vector, fp_read_int_vector
 *
 * Index: I/O,  integer vectors,  integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_int_vector_with_header(Int_vector** vpp, FILE* fp)
{
    int num_elements;
    int result       = fp_read_vector_length_header(fp, &num_elements);


    if (result == ERROR)
    {
        set_error("Error reading vector length header from %F.", fp);
        return ERROR;
    }
    else if (result == NOT_FOUND)
    {
        return NOT_FOUND;
    }
    else if (num_elements == 0)
    {
        free_int_vector(*vpp);
        *vpp = NULL;
        return NO_ERROR;
    }
    else
    {
        return fp_read_ascii_int_vector_2(vpp, fp, num_elements);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           fp_read_ascii_int_vector
 *
 * Reads ascii data from an input stream into a integer vector
 *
 * This routine reads data contained in a file specified by a pointer to FILE
 * into an integer vector. The length of the new vector is determined by the
 * number of data elements contained in the input file.  The vector *vpp is
 * created or resized as necessary.
 *
 * "vpp" is a pointer to a integer vector pointer.
 *
 * "fp" points to a FILE that corresponds to the stream to read from.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Related:
 *     read_int_vector, fp_read_int_vector
 *
 * Index: I/O,  integer vectors,  integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_ascii_int_vector(Int_vector** vpp, FILE* fp)
{
    int  num_elements;
    char line[ LARGE_IO_BUFF_SIZE ];
    long read_res;


    ERE(num_elements = gen_count_tokens_in_file(fp, " ,\t"));

    if (num_elements == 0)
    {
        set_error("Expecting at least one number in %F.", fp);
        return ERROR;
    }

    ERE(fp_read_ascii_int_vector_2(vpp, fp, num_elements));

    /*
     * Since we counted out the number of tokens that we expected, and stopped
     * reading when we got all of them, there can be junk left that we have to
     * swallow.
    */
    while (TRUE)
    {
        ERE(read_res = BUFF_GET_REAL_LINE(fp, line));

        if (read_res < 0) break;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int fp_read_ascii_int_vector_2(Int_vector** vpp, FILE* fp, int length)
{
    Int_vector* vp;
    char        line[ LARGE_IO_BUFF_SIZE ];
    char        element_buff[ 200 ];
    char*       line_pos;
    int         i;
    int         scan_res;
    long        read_res;


    ERE(get_target_int_vector(vpp, length));

    if (length == 0) return NO_ERROR;

    vp = *vpp;

    i = 0;

    while (i < length)
    {
        ERE(read_res = BUFF_GET_REAL_LINE(fp, line));

        if (read_res == EOF)
        {
            set_error("Unexpected end of file reading vector from %F.", fp);
            return ERROR;
        }

        line_pos = line;

        while (BUFF_GEN_GET_TOKEN_OK(&line_pos, element_buff, " ,\t"))
        {
            if (i < vp->length)
            {
                scan_res = ss1i(element_buff, (vp->elements)+i);

                if (scan_res == ERROR)
                {
                    insert_error("Error reading integer from %F.",
                                 fp);
                    return ERROR;
                }

                i++;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                       fp_read_vector_length_header
 *
 * Reads vector length header from file.
 *
 * This routine reads the vector length header from the file pointed to by the
 * argument "fp". If the header contains information about the vector length,
 * then the corresponding variables whose pointers are arguments are set.
 * Variables are not changed unless there is information in the header.
 *
 * The vector length header has the format:
 * |    #! length=<vector-length>
 * (The "#" is actually the comment char (user settable) and the "!" is acutally
 * the header char, also user settable).
 *
 * Returns:
 *    Either NO_ERROR if a vector length header was successfully read, or ERROR
 *    on a file error or if no header is present.
 *
 * Note:
 *    This routine can fail if the stream is a pipe. The argument fp must point
 *    to something that can be "seeked". Exaclty what that is can depend on the
 *    OS.
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

/*
 * This is a bit of a mess because it takes a very general strategy inherited
 * from fp_read_matrix_size_header(), where the strategy was already overkill.
 * However, there is lots of room for enhacements.
*/

int fp_read_vector_length_header(FILE* fp, int* length_ptr)
{
    IMPORT int kjb_comment_char;
    int            i;
    int            num_options;
    char**         option_list;
    char**         option_list_pos;
    char**         value_list;
    char**         value_list_pos;
    char           line[ 100 ];
    char*          line_pos;
    long           start_file_pos;   /* File position at function call */
    int            length = NOT_SET;
    int            result   = NO_ERROR;


    ERE(start_file_pos = kjb_ftell(fp));

    while (length == NOT_SET)
    {
        int read_result;

        read_result  = BUFF_FGET_LINE(fp, line);

        if (read_result == ERROR) result = ERROR;

        if (read_result < 0) break; /* Bail out of while (TRUE) */

        line_pos = line;

        trim_beg(&line_pos);

        if (*line_pos == '\0')
        {
            /*EMPTY*/
            ;  /* Do nothing. */
        }
        else if (*line_pos == kjb_comment_char)
        {
            line_pos++;

            trim_beg(&line_pos);

            if (*line_pos == '!')
            {
                line_pos++;

                num_options = parse_options(line_pos, &option_list,
                                            &value_list);
                option_list_pos = option_list;
                value_list_pos  = value_list;

                if (num_options == ERROR)
                {
                    result = ERROR;
                    break;
                }

                for (i=0; i<num_options; i++)
                {
                    /* Look for flag indicating the length */
                    if (IC_STRCMP_EQ(*option_list_pos,"length"))
                    {
                        if (ss1pi(*value_list_pos, &length) == ERROR)
                        {
                            result = ERROR;
                            break;
                        }
                    }

                } /* for (i=0; i<num_options; i++) */

                free_options(num_options, option_list, value_list);

                if (result == ERROR)
                {
                    break;
                }
            } /* if (*line_pos == '!') */
        } /* else if (*line_pos == kjb_comment_char) */
        else
        {
            result = NO_ERROR;
            break;
        }
    } /* end while ( TRUE ) - all "breaks" wind up here */

    if (result == ERROR)
    {
        /* Rewind file to starting position */
        (void)kjb_fseek(fp, start_file_pos, SEEK_SET);
        return ERROR;
    }
    else if (length == NOT_SET)
    {
        /* Rewind file to starting position */
        ERE(kjb_fseek(fp, start_file_pos, SEEK_SET));
        return NOT_FOUND;
    }
    else
    {
        *length_ptr = length;
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          write_col_int_vector
 *
 * Writes an integer vector as a column to a file specified by name
 *
 * This routine writes data contained in a integer vector to a file specified
 * by its name. The vector is output as a column of data.
 *
 * "file_name" points to a character array specifying the name of the file for
 * writing. If the "file_name" argument is NULL, or the first character is
 * null, output is directed to STDOUT.
 *
 * "vp" points to the integer vector whose contents are to be written.
 * If it is NULL, this is a NOP.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Related:
 *     fp_write_col_int_vector, write_row_int_vector
 *
 * Index: I/O,  integer vectors,  integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_col_int_vector(const Int_vector* vp, const char* file_name)
{
    FILE* fp;
    int   result;


    if (file_name == NULL)
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name))
        {
            return NO_ERROR;
        }

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    result = fp_write_col_int_vector(vp, fp);

    if (file_name != NULL)
    {
        Error_action save_error_action = get_error_action();

        if (result == ERROR)
        {
            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        }

        if (kjb_fclose(fp) == ERROR)
        {
            result = ERROR;
        }

        set_error_action(save_error_action);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          fp_write_col_int_vector
 *
 * Writes an integer vector as a column to an output stream
 *
 * This routine writes data contained in a integer vector to a file specified
 * by a pointer to FILE. The vector is output as a column of data.
 *
 * "fp" points to a FILE associated with the output stream.
 * "vp" points to the integer vector whose contents are to be written.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Related:
 *     write_col_int_vector, fp_write_row_int_vector
 *
 * Index: I/O,  integer vectors,  integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/


int fp_write_col_int_vector(const Int_vector* vp, FILE* fp)
{
    int   i;


    if (vp == NULL) return NO_ERROR;

    for (i=0; i<vp->length; i++)
    {
        ERE(kjb_fprintf(fp, "%d\n", vp->elements[i]));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          fp_write_col_int_vector_with_title
 *
 * Writes an integer vector as a column with a title
 *
 * This routine writes data in an integer vector with a title to the file
 * pointed to by "fp". The vector is output as a column of data. This routine
 * is largely designed for debugging, and is often called with the macro dbi_cv.
 *
 * "fp" points to a character array specifying the name of the file to write
 * data ti. If fp is NULL, output is directed to STDOUT.  "vp" points to the
 * integer vector whose contents are to be written.  "title" points to a
 * title string which is printed before the vector. "title" may be NULL, in
 * which case, nothing is printed. If the vector itself is NULL, then "NULL" is
 * printed instead of any data.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Related:
 *     fp_write_row_int_vector_with_title, db_icv, db_irv
 *
 * Index: I/O,  integer vectors,  integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_col_int_vector_with_title(const Int_vector* vp, FILE* fp,
                                       const char* title)
{
    int   i;


    if (vp == NULL)
    {
        if (title != NULL)
        {
            ERE(kjb_fprintf(fp, title));
            ERE(kjb_fprintf(fp, ": "));
        }
        ERE(kjb_fprintf(fp, "NULL\n"));

        return NO_ERROR;
     }

    if (title != NULL)
    {
        ERE(kjb_fprintf(fp, title));
        ERE(kjb_fprintf(fp, "\n"));
    }

    for (i=0; i<vp->length; i++)
    {
        ERE(kjb_fprintf(fp, "%d\n", vp->elements[i]));
    }

    ERE(kjb_fprintf(fp, "\n"));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       write_col_int_vector_with_header
 *
 * Writes an integer vector as a column to a file specified by name
 *
 * This routine writes data contained in a vector to a file specified by its
 * name. The vector is output as a column of data with a header.
 *
 * The vector length header has the format:
 * |    #! length=<vector-length>
 * (The "#" is actually the comment char (user settable) and the "!" is acutally
 * the header char, also user settable).
 *
 * "file_name" points to a character array specifying the name of the file to
 * read the data from. If the "file_name" argument is NULL, or the first
 * character is null, output is directed to STDOUT.
 *
 * "vp" points to the vector whose contents are to be written.
 * If it is NULL, then  vector of length zero is written which will be read
 * back as NULL by the corresponding read routine.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_col_int_vector_with_header(const Int_vector* vp, const char* file_name)
{
    FILE* fp;
    int   write_result;
    int   close_result = NO_ERROR;


    UNTESTED_CODE();

    if (file_name == NULL)
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) return NO_ERROR;

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    write_result = fp_write_col_int_vector_with_header(vp, fp);

    if (file_name != NULL)
    {
        if (write_result == ERROR)
        {
            push_error_action(FORCE_ADD_ERROR_ON_ERROR);
        }

        close_result = kjb_fclose(fp);

        if (write_result == ERROR)
        {
            pop_error_action();
        }
    }

    if ((write_result == ERROR) || (close_result == ERROR))
    {
        return ERROR;
    }
    else
    {
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        fp_write_col_int_vector_with_header
 *
 * Writes an integer vector as a column to an output stream with a header
 *
 * This routine writes data contained in a vector to a file specified by a
 * pointer to FILE. The vector is output as a column of data.
 *
 * The vector length header has the format:
 * |    #! length=<vector-length>
 * (The "#" is actually the comment char (user settable) and the "!" is acutally
 * the header char, also user settable).
 *
 * "fp" points to a FILE associated with the output stream.
 *
 * "vp" points to the vector whose contents are to be written.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/


int fp_write_col_int_vector_with_header(const Int_vector* vp, FILE* fp)
{

    UNTESTED_CODE();

    ERE(fp_write_vector_length_header(fp, (vp == NULL) ? 0 :  vp->length));
    ERE(fp_write_col_int_vector(vp, fp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          write_row_int_vector
 *
 * Writes an integer vector as a rowumn to a file specified by name
 *
 * This routine writes data contained in a integer vector to a file specified
 * by its name. The vector is output as a rowumn of data.
 *
 * "file_name" points to a character array specifying the name of the file for
 * writing. If the "file_name" argument is NULL, or the first character is
 * null, output is directed to STDOUT.
 *
 * "vp" points to the integer vector whose contents are to be written.
 * If it is NULL, this is a NOP.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Related:
 *     fp_write_row_int_vector, write_row_int_vector
 *
 * Index: I/O, integer vectors,  integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_row_int_vector(const Int_vector* vp, const char* file_name)
{
    FILE* fp;
    int   result;


    if (file_name == NULL)
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) return NO_ERROR;

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    result = fp_write_row_int_vector(vp, fp);

    if (file_name != NULL)
    {
        Error_action save_error_action = get_error_action();

        if (result == ERROR)
        {
            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        }

        if (kjb_fclose(fp) == ERROR)
        {
            result = ERROR;
        }

        set_error_action(save_error_action);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          fp_write_row_int_vector
 *
 * Writes an integer vector as a row to an output stream
 *
 * This routine writes data contained in a integer vector to a file specified
 * by a pointer to FILE. The vector is output as a rowumn of data.
 *
 * "fp" points to a FILE associated with the output stream.
 *
 * "vp" points to the integer vector whose contents are to be written.
 * If it is NULL, this is a NOP.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Related:
 *     write_row_int_vector, fp_write_row_int_vector
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/


int fp_write_row_int_vector(const Int_vector* vp, FILE* fp)
{
    int   i;


    if (vp == NULL) return NO_ERROR;

    for (i=0; i<vp->length; i++)
    {
        ERE(kjb_fprintf(fp, "%d ", vp->elements[i]));
    }

    ERE(kjb_fprintf(fp, "\n"));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          fp_write_row_int_vector_with_title
 *
 * Writes an integer vector as a row with a title
 *
 * This routine writes data in an integer vector with a title to the file
 * pointed to by "fp". The vector is output as a row of data. This routine
 * is largely designed for debugging, and is often called with the macro dbi_cv.
 *
 * "fp" points to a character array specifying the name of the file to write
 * data ti. If fp is NULL, output is directed to STDOUT.  "vp" points to the
 * integer vector whose contents are to be written.  "title" points to a
 * title string which is printed before the vector. "title" may be NULL, in
 * which case, nothing is printed. If the vector itself is NULL, then "NULL" is
 * printed instead of any data.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Related:
 *     fp_write_col_int_vector_with_title, db_icv, db_irv
 *
 * Index: I/O, integer vectors, integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_row_int_vector_with_title(const Int_vector* vp, FILE* fp,
                                       const char* title)
{
    int   i;


    if (vp == NULL)
    {
        if (title != NULL)
        {
            ERE(kjb_fprintf(fp, title));
            ERE(kjb_fprintf(fp, ": "));
        }
        ERE(kjb_fprintf(fp, "NULL\n"));

        return NO_ERROR;
     }

    if (title != NULL)
    {
        ERE(kjb_fprintf(fp, title));
        ERE(kjb_fprintf(fp, "\n"));
    }

    for (i=0; i<vp->length; i++)
    {
        if (i != 0)
        {
            ERE(kjb_fputs(fp, "  "));
        }

        ERE(kjb_fprintf(fp, "%d", vp->elements[i]));
    }

    ERE(kjb_fprintf(fp, "\n"));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                       fp_write_vector_length_header
 *
 * Writes vector length header to a file.
 *
 * This routine writes the vector length
 * to the file pointer "fp".
 *
 * The header has the format:
 *|    #! length=<vector-length>
 * (The "#" is actually the comment char (user settable) and the "!" is actually
 * the header char, also user settable).
 *
 * Returns:
 *    Either NO_ERROR on success, or ERROR, with an appropriate error message
 *    being set.
 *
 * Index: I/O, vectors, vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_vector_length_header(FILE* fp, int length)
{

    IMPORT int kjb_comment_char;
    IMPORT int kjb_header_char;
    int        result;


    UNTESTED_CODE();

    if (length < 0)
    {
        result = ERROR;
        set_error("Error writing vector size header to: \"%F\"", fp);
        add_error("Length (%d) is negative.");
    }
    else
    {
        result = kjb_fprintf(fp, "\n%c%c length=%d\n\n",
                             kjb_comment_char, kjb_header_char,
                             length);
        if (result > 0)
        {
            result = NO_ERROR;
        }
        else
        {
            result = ERROR;
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          write_raw_int_vector
 *
 * Writes a vector to a file specified by name as raw data
 *
 * This routine writes data contained in a integer vector to a file specified by
 * its name. The vector is output as raw data.
 *
 * "file_name" points to a character array specifying the name of the file
 * to read the data from. If the "file_name" argument is NULL, or the first
 * character is null, output is directed to STDOUT.
 *
 * "vp" points to the integer vector whose contents are to be written.
 * If it is NULL, then  vector of length zero is written which will be read
 * back as NULL by the corresponding read routine.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, integer vectors, integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_raw_int_vector(const Int_vector* vp, const char* file_name)
{
    FILE* fp;
    int   result;


    UNTESTED_CODE();

    if (file_name == NULL)
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) return NO_ERROR;

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    result = fp_write_raw_int_vector(vp, fp);

    if (file_name != NULL)
    {
        Error_action save_error_action = get_error_action();

        if (result == ERROR)
        {
            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        }

        if (kjb_fclose(fp) == ERROR)
        {
            result = ERROR;
        }

        set_error_action(save_error_action);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          fp_write_raw_int_vector
 *
 * Writes an integer vector to an output stream as raw data
 *
 * This routine writes an integer vector to a file specified by a pointer to
 * FILE. The vector is output as raw data.
 *
 * "fp" points to a FILE associated with the output stream.  "vp" points
 * to the integer vector to be written.
 *
 * Returns:
 *     NO_ERROR if successful, ERROR on failure.
 *
 * Index: I/O, integer vectors, integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_raw_int_vector(const Int_vector* vp, FILE* fp)
{
    int i;
    int         byte_order = 1;
    char        head_str[ KJB_DATA_HEAD_SIZE ];
    int         pad = 0;
    int         length = 0;


    UNTESTED_CODE();

    if (vp != NULL)
    {
        length = vp->length;
    }

    for (i = 0; i < KJB_DATA_HEAD_SIZE; i++)
    {
        head_str[ i ] = '\0';
    }

    BUFF_CPY(head_str, KJB_RAW_INT_VECTOR_STRING);

    ERE(kjb_fwrite(fp, head_str, sizeof(head_str)));

    ERE(FIELD_WRITE(fp, byte_order));
    ERE(FIELD_WRITE(fp, pad));
    ERE(FIELD_WRITE(fp, length));
    ERE(FIELD_WRITE(fp, pad));

    if (length > 0)
    {
        ERE(kjb_fwrite_2(fp, vp->elements, sizeof(int) * length, NULL));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_target_int_vector_vector
 *
 * Gets a target integer vector vector
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of integer vector arrays. If *target_vvpp is NULL, then
 * this routine creates the vector vector. If it is not null, and it is the
 * right size, then this routine does nothing. If it is the wrong size, then it
 * is resized.
 *
 * The routine free_int_vector_vector should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Index: memory allocation, arrays, vectors
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index : integer vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_target_int_vector_vector(Int_vector_vector **target_vvpp, int count)
{

    free_int_vector_vector(*target_vvpp);
    NRE(*target_vvpp = create_int_vector_vector(count));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Int_vector_vector* create_int_vector_vector(int count)
{
    Int_vector_vector* vvp;
    int           i;


    NRN(vvp = TYPE_MALLOC(Int_vector_vector));
    vvp->length = count;

    if (count > 0)
    {
        NRN(vvp->elements = N_TYPE_MALLOC(Int_vector*, count));
    }
    else
    {
        vvp->elements = NULL;
    }

    for (i=0; i<count; i++)
    {
        vvp->elements[ i ] = NULL;
    }

    return vvp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_int_vector_vector
 *
 * Frees the storage in an integer vector vector
 *
 * This routine frees the storate in a vector array.
 *
 * Index: memory allocation, arrays, vectors
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index : integer vectors
 *
 * -----------------------------------------------------------------------------
*/

void free_int_vector_vector(Int_vector_vector *vvp)
{
    int count, i;
    Int_vector** vp_pos;


    if (vvp == NULL) return;

    vp_pos = vvp->elements;
    count = vvp->length;

    for (i=0; i<count; i++)
    {
        free_int_vector(*vp_pos);
        vp_pos++;
    }

    kjb_free(vvp->elements);
    kjb_free(vvp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_target_int_v3
 *
 * Gets a target integer vector vector vector
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of integer vector vector vectors. If *target_vvvpp is
 * NULL, then this routine creates the vector vector vector. If it is not null,
 * and it is the right size, then this routine does nothing. If it is the wrong
 * size, then it is resized.
 *
 * The routine free_int_v3 should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Index: memory allocation, arrays, vectors
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * -----------------------------------------------------------------------------
*/

int get_target_int_v3(Int_v_v_v** target_vvvpp, int length)
{

    free_int_v3(*target_vvvpp);
    NRE(*target_vvvpp = create_int_v3(length));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Int_v_v_v* create_int_v3(int length)
{
    Int_v_v_v* vvvp;
    int    i;


    NRN(vvvp = TYPE_MALLOC(Int_v_v_v));
    vvvp->length = length;
    NRN(vvvp->elements = N_TYPE_MALLOC(Int_vector_vector*, length));

    for (i=0; i<length; i++)
    {
        vvvp->elements[ i ] = NULL;
    }

    return vvvp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_int_v3
 *
 * Frees the storage in an integeer vector vector vector
 *
 * This routine frees the storage in an integer vector ector vector.
 *
 * Index: memory allocation, arrays, vectors
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * -----------------------------------------------------------------------------
*/

void free_int_v3(Int_v_v_v* vvvp)
{
    int             count;
    int             i;
    Int_vector_vector** vvp_pos;


    if (vvvp == NULL) return;

    vvp_pos = vvvp->elements;
    count = vvvp->length;

    for (i=0; i<count; i++)
    {
        free_int_vector_vector(*vvp_pos);
        vvp_pos++;
    }

    kjb_free(vvvp->elements);
    kjb_free(vvvp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int concat_int_vectors(Int_vector** vpp, int num_vectors,
                       const Int_vector** vectors)
{
    int  i;
    int  length     = 0;
    int* target_pos;

    UNTESTED_CODE();

    for (i = 0; i < num_vectors; i++)
    {
        if (vectors[ i ] != NULL)
        {
            length += vectors[ i ]->length;
        }
    }

    ERE(get_target_int_vector(vpp, length));
    target_pos = (*vpp)->elements;

    for (i = 0; i < num_vectors; i++)
    {
        if (vectors[ i ] != NULL)
        {
            const Int_vector* source_vp     = vectors[ i ];
            int               source_length = source_vp->length;
            int*              source_pos    = source_vp->elements;
            int               j;

            for (j = 0; j < source_length; j++)
            {
                *target_pos = *source_pos;
                target_pos++;
                source_pos++;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                       int_vector_binary_search
 *
 * Finds an integer in an integer vector
 *
 * This routine finds an integer in the integer vector "vp" with a binary
 * search.
 *
 * Returns:
 *    On success, this routine returns an index into the array. If the element
 *    is not found, then it returns NOT_FOUND.
 *
 * Index : integer vectors, sorting/searching
 *
 * -----------------------------------------------------------------------------
 */

int int_vector_binary_search(const Int_vector* vp, int search_elem)
{
    int* array = vp->elements;
    int num_elements = vp->length;

    if (vp == NULL) return NOT_FOUND;

#ifdef TEST
    {
        int m1 = binary_search_int_array((const int*)array, num_elements,
                                         search_elem);
        int m2 = int_binary_search((const void*)array, num_elements,
                                   sizeof(int), (size_t)0, search_elem);
        ASSERT(m1 == m2);
    }
#endif

    return binary_search_int_array((const int*)array, num_elements,
                                   search_elem);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                       int_vector_linear_search
 *
 * Finds an integer in an integer vector
 *
 * This routine finds an integer in the integer vector "vp" with a linear
 * search.
 *
 * Returns:
 *    On success, this routine returns an index into the array. If the element
 *    is not found, then it returns NOT_FOUND.
 *
 * Index : integer vectors, sorting/searching
 *
 * -----------------------------------------------------------------------------
 */

int int_vector_linear_search(const Int_vector* vp, int search_elem)
{
    int* array = vp->elements;
    int num_elements = vp->length;

    if (vp == NULL) return NOT_FOUND;

    return linear_search_int_array((const int*)array, num_elements,
                                   search_elem);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                               sum_int_vector_elements
 *
 * Returns the sum of the elements of an integer vector
 *
 * This routine calculates the sum of the elements of an integer vector.
 *
 * Returns :
 *     The sum of the elements of a vector.
 *
 * Index : integer vectors
 *
 * -----------------------------------------------------------------------------
 */

int sum_int_vector_elements(const Int_vector* vp)
{
    int   i;
    int  sum;
    int* element_ptr;


    sum = 0;

    element_ptr = vp->elements;

    for (i = 0; i<vp->length; i++)
    {
       sum += (*element_ptr);
       element_ptr++;
    }

    return sum;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         read_int_vector_vector
 *
 * Reads an integer vector vector from file
 *
 * This routine reads an integer vector vector from a file.  If the file_name is
 * NULL or a null string, then stdin is assumed. If this is the case, and if the
 * source is not a file (i.e. a pipe), then this routine will fail.
 *
 * Currently only one format is implemented. Specifically we assume that the
 * file is a formatted ascii file, and the dimensions are deduced
 * from the number of rows and columns.
 *
 * The integer vector vector *result_vvpp is created or resized as necessary.
 *
 * "file_name" points to a character array specifying the name of the file
 * to read the data from. If the "file_name" argument is NULL or the
 * first character is null, input is retrieved from STDIN. If STDIN is
 * not a redirected file, an error message will be generated.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Index: I/O, integer vectors, integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_int_vector_vector
(
    Int_vector_vector** result_vvpp,
    const char*     file_name
)
{
    FILE*     fp;
    int       result;


    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdin;
    }
    else
    {
        NRE(fp = kjb_fopen(file_name, "r"));
    }

    result = fp_read_int_vector_vector(result_vvpp, fp);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        (void)kjb_fclose(fp);  /* Ignore return: Only reading. */
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       fp_read_int_vector_vector
 *
 * Reads an integer vector vector from data in a formated file
 *
 * This routine reads an integer vector vector from a file.  If the file_name is
 * NULL or a null string, then stdin is assumed. If this is the case, and if the
 * ource is not a file (i.e. a pipe), then this routine will fail.  If there are
 * multiple matrices in the file separated with soft EOFs, then the reading
 * continues to the next soft (or hard) EOF.
 *
 * Currently only one format is implemented. Specifically we assume that the
 * file is a formatted ascii file, and the dimensions are deduced
 * from the number of rows and columns.
 *
 * The integer vector vector *result_vvpp is created or resized as necessary.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Index: I/O, integer vectors, integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_int_vector_vector(Int_vector_vector** result_vvpp, FILE* fp)
{
    int       result        = NOT_FOUND;
    Path_type path_type;
    long      save_file_pos;


    path_type = fp_get_path_type(fp);

    if (path_type == PATH_ERROR)
    {
        result = ERROR;
    }

    if (path_type != PATH_IS_REGULAR_FILE)
    {
        set_error("Input %F is not a regular file.", fp);
        add_error("Read of vector vector aborted.");
        result = ERROR;
    }

    ERE(save_file_pos = kjb_ftell(fp));

    if (result == NOT_FOUND)
    {
        result = fp_read_raw_int_vector_vector(result_vvpp, fp);

        if (result == NOT_FOUND)
        {
            ERE(kjb_fseek(fp, save_file_pos, SEEK_SET));
        }
    }

    if (result == NOT_FOUND)
    {
        result = fp_read_formatted_int_vector_vector(result_vvpp, fp);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        fp_read_raw_int_vector_vector
 *
 * Reads a raw integer vector vector from a stream
 *
 * This routine reads a raw (binary format) integer vector vector from the
 * stream pointed to by fp.  string, then stdin is assumed. If this is the case,
 * and if the source is not a file (i.e. a pipe), then this routine will fail.
 *
 * The integer vector vector *result_vvpp is created or resized as necessary.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Index: I/O, integer vectors, integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_raw_int_vector_vector(Int_vector_vector** vvpp, FILE* fp)
{
    int     num_vectors;
    int     i;
    long    bytes_used_so_far;
    int     byte_order;
    char    head_str[ KJB_DATA_HEAD_SIZE ];
    int     pad;
    off_t   num_bytes;
    int     result = NO_ERROR;


    ERE(fp_get_byte_size(fp, &num_bytes));
    ERE(bytes_used_so_far = kjb_ftell(fp));

    num_bytes -= bytes_used_so_far;

    if (num_bytes < KJB_DATA_HEAD_SIZE) return NOT_FOUND;

    ERE(kjb_fread_exact(fp, head_str, sizeof(head_str)));
    head_str[ sizeof(head_str) - 1 ] = '\0';

    if ( ! STRCMP_EQ(head_str, KJB_RAW_INT_VECTOR_VECTOR_STRING))
    {
        return NOT_FOUND;
    }

    ERE(FIELD_READ(fp, byte_order));
    ERE(FIELD_READ(fp, pad));
    ERE(FIELD_READ(fp, num_vectors));
    ERE(FIELD_READ(fp, pad));

    if (num_vectors < 0)
    {
        set_error("Invalid data in vector vector file %F.");
        add_error("The number of vectors is negative ( %d ).", num_vectors);
        return ERROR;
    }

    ERE(get_target_int_vector_vector(vvpp, num_vectors));

    for (i = 0; i < num_vectors; i++)
    {
        if (fp_read_raw_int_vector(&((*vvpp)->elements[ i ]), fp) == ERROR)
        {
            add_error("Failed reading vector %d.\n", i + 1);
            result = ERROR;
            break;
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     fp_read_formatted_int_vector_vector
 *
 * Reads an integer vector vector from data in a formatted FILE
 *
 * This routine reads an integer vector vector from a file containing data
 * formatted into fixed rows and columns. The vector vector dimensions are
 * controlled by the formatting of the data in the file. The number of vectors
 * is the number of rows, and the number of elements in a given element is the
 * number of elements in the column.
 *
 * The integer vector vector *result_vvpp is created or resized as necessary.
 *
 * "fp" points to a FILE specifying the file to read the data from.
 *
 * Returns:
 *     Either NO_ERROR, or ERROR, with an appropriate error message being set.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, integer vectors, integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_formatted_int_vector_vector
(
    Int_vector_vector** result_vvpp,
    FILE*           fp
)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Int_vector_vector*      vvp;
    char                line[ LARGE_IO_BUFF_SIZE ];
    char                num_buff[ 10000 ];
    int                 length;
    int                 num_vectors;
    int                 i, j;
    char*               line_pos;
    int                 scan_res;



    ERE(num_vectors = count_real_lines(fp));

    if ((num_vectors == 0) || (num_vectors == EOF))
    {
        return EOF;
    }

    ERE(get_target_int_vector_vector(result_vvpp, num_vectors));
    vvp = *result_vvpp;

    for (i=0; i<num_vectors; i++)
    {
        ERE(BUFF_GET_REAL_LINE(fp, line));

        length = 0;
        line_pos = line;

        trim_beg(&line_pos);
        trim_end(line_pos);

        if (STRCMP_EQ(line_pos, "NULL"))
        {
            /* Leave this vector NULL. */
            continue;
        }

        while (BUFF_GEN_GET_TOKEN_OK(&line_pos, num_buff, " ,\t"))
        {
            length++;
        }

        ERE(get_target_int_vector(&(vvp->elements[ i ]), length));

        line_pos = line;

        for (j=0; j<length; j++)
        {
            if (! BUFF_GEN_GET_TOKEN_OK(&line_pos, num_buff, " ,\t"))
            {
                set_error("Missing data on row %d of %F.", (i+1), fp);
                return ERROR;
            }

            scan_res = ss1i(num_buff, &(vvp->elements[i]->elements[j]));

            if (scan_res == ERROR)
            {
                insert_error("Error reading floating point number from %F.",
                             fp);
                return ERROR;
            }
            else if (io_atn_flag)
            {
                halt_all_output = FALSE;
                set_error("Processing interrupted.");
                return ERROR;
            }
        }
    }

    /*
    // Suck up remaining lines to position file pointer after soft EOF, in the
    // case that the read stopped at a soft EOF.
    */

    /*CONSTCOND*/
    while (TRUE)
    {
        int read_res = BUFF_GET_REAL_LINE(fp, line);

        if (read_res == EOF) break;

        if (read_res < 0)
        {
            return read_res;
        }

        if (read_res > 0)
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           write_int_vector_vector
 *
 * Writes an integer vector vector to a file specified by name
 *
 * This routine outputs an integer vector vector to a file specified by the
 * input file name.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the vector vector contents to. If "file_name" is NULL
 * or equal to '\0', output is directed to STDOUT. Otherwise, the file
 * is created or the existing copy is overwritten.
 *
 * "vvp" is a pointer to the Int_vector_vector object whose contents are to be
 * written.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file close error.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, integer vectors, integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_int_vector_vector
(
    const Int_vector_vector* vvp,
    const char*              file_name
)
{
    FILE* fp;
    int   write_result;
    int   close_result = NO_ERROR;


    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) return NO_ERROR;

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    write_result = fp_write_int_vector_vector(vvp, fp);

    kjb_fflush(fp);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        Error_action save_error_action = get_error_action();

        if (write_result == ERROR)
        {
            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        }
        close_result = kjb_fclose(fp);

        set_error_action(save_error_action);
    }

    if ((write_result == ERROR) || (close_result == ERROR))
    {
        return ERROR;
    }
    else return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           fp_write_int_vector_vector
 *
 * Write an integer vector vector to a file specified by FILE pointer
 *
 * This routine outputs a Int_vector_vector to a file specified by a pointer to a
 * FILE.
 *
 * "fp" is a pointer to a FILE as returned by "kjb_fopen". It
 * is assumed that this file pointer is valid.
 *
 * "vvp" is a pointer to the Int_vector_vector object to output. The pointer is
 * assumed to be valid.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: I/O, integer vectors, integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_int_vector_vector(const Int_vector_vector* vvp, FILE* fp)
{
    int   i;


    for (i=0; i<vvp->length; i++)
    {
        Int_vector* vp = vvp->elements[ i ];

        if (vp == NULL)
        {
            ERE(kjb_fprintf(fp, "NULL\n"));
        }
        else
        {
            ERE(fp_write_row_int_vector(vp, fp));
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                    write_raw_int_vector_vector
 *
 * Writes a vector vector to a file specified by name a raw data
 *
 * This routine outputs an integer vector vector to a file specified by the
 * input file name as raw (binary) data.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the vector vector contents to. If "file_name" is NULL
 * or equal to '\0', output is directed to STDOUT. Otherwise, the file
 * is created or the existing copy is overwritten.
 *
 * "vvp" is a pointer to the integer vector vector to be written.
 *
 * The output format depends on the magnitude of the maximum value in the
 * vector. If the maximum is < 0.01 or greater than 10000, the data is
 * written in exponential format according to the format string
 * "%10.3e". Otherwise the vector vector elements are written in fixed format
 * according to "9.5f".
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file close error.
 *
 * Index: I/O, integer vectors, integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_raw_int_vector_vector
(
    const Int_vector_vector* vvp,
    const char*              file_name
)
{
    FILE* fp;
    int   write_raw_result;
    int   close_result = NO_ERROR;


    UNTESTED_CODE();

    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdout;
    }
    else
    {
        if (skip_because_no_overwrite(file_name)) return NO_ERROR;

        NRE(fp = kjb_fopen(file_name, "w"));
    }

    write_raw_result = fp_write_raw_int_vector_vector(vvp, fp);

    kjb_fflush(fp);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        Error_action save_error_action = get_error_action();

        if (write_raw_result == ERROR)
        {
            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        }
        close_result = kjb_fclose(fp);

        set_error_action(save_error_action);
    }

    if ((write_raw_result == ERROR) || (close_result == ERROR))
    {
        return ERROR;
    }
    else return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     fp_write_raw_int_vector_vector
 *
 * Write a vector vector to a file specified by FILE pointer as raw data
 *
 * This routine outputs an integer vector vector to a file specified by a
 * pointer to FILE as raw data.
 *
 * "fp" is a pointer to a FILE as returned by "kjb_fopen". It
 * is assumed that this file pointer is valid.
 *
 * "vvp" is a pointer to the integer vector vector to be written. The pointer is
 * assumed to be valid.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a failure.
 *
 * Index: I/O, integer vectors, integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_raw_int_vector_vector(const Int_vector_vector* vvp, FILE* fp)
{
    int num_vectors = vvp->length;
    int i;


    UNTESTED_CODE();

    ERE(fp_write_raw_int_vector_vector_header(num_vectors, fp));

    for (i=0; i<num_vectors; i++)
    {
        Int_vector* vp = vvp->elements[ i ];

        ERE(fp_write_raw_int_vector(vp, fp));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ============================================================================
 *                      fp_write_raw_int_vector_vector_header
 *
 * Writes the header for a raw vector vector to a stream
 *
 * This routine outputs the header for a raw vector vector to a stream pointed
 * to by fp. This routine exists so that vector vectors can be written
 * sequentially, one vector at a time, so that the entire vector vector does not
 * need to be in memory).
 *
 * Returns:
 *     NO_ERROR on success, or ERROR if there are problems.
 *
 * Index: I/O, integer vectors, integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_raw_int_vector_vector_header(int num_vectors, FILE* fp)
{
    int byte_order = 1;
    char head_str[ KJB_DATA_HEAD_SIZE ];
    int  pad = 0;
    int  i;


    UNTESTED_CODE();

    for (i = 0; i < KJB_DATA_HEAD_SIZE; i++)
    {
        head_str[ i ] = '\0';
    }

    BUFF_CPY(head_str, KJB_RAW_INT_VECTOR_VECTOR_STRING);

    ERE(kjb_fwrite(fp, head_str, sizeof(head_str)));

    ERE(FIELD_WRITE(fp, byte_order));
    ERE(FIELD_WRITE(fp, pad));
    ERE(FIELD_WRITE(fp, num_vectors));
    ERE(FIELD_WRITE(fp, pad));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int concat_int_vector_vectors
(
    Int_vector_vector**       vvpp,
    int                       num_vectors,
    const Int_vector_vector** int_vector_vectors
)
{
    int i, length = 0;
    Int_vector** target_pos;

    UNTESTED_CODE();

    for (i = 0; i < num_vectors; i++)
    {
        if (int_vector_vectors[ i ] != NULL)
        {
            length += int_vector_vectors[ i ]->length;
        }
    }

    ERE(get_target_int_vector_vector(vvpp, length));
    target_pos = (*vvpp)->elements;

    for (i = 0; i < num_vectors; i++)
    {
        const Int_vector_vector* source_vvp = int_vector_vectors[ i ];

        if (source_vvp != NULL)
        {
            int          source_length = source_vvp->length;
            Int_vector** source_pos    = source_vvp->elements;
            int          j;

            for (j = 0; j < source_length; j++)
            {
                ERE(copy_int_vector(target_pos, *source_pos));
                target_pos++;
                source_pos++;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ============================================================================
 *                                sget_int_vector
 *
 * Extracts an integer vector from a string
 *
 * This routine reads a blank or comma or tab separated list of integers from a
 * string into an integer vector.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR if there are problems.
 *
 * Index: I/O, integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int sget_int_vector(Int_vector** vpp, const char* line)
{
    const char* line_pos = line;
    const char* save_line_pos = line_pos;
    int    count;
    int    length = 0;
    int    result = NO_ERROR;
    char   buff[ 1000 ];


    while (BUFF_CONST_GEN_MQ_GET_TOKEN(&line_pos, buff, " ,\t"))
    {
        length++;
    }

    line_pos = save_line_pos;

    ERE(get_target_int_vector(vpp, length));

    count = 0;

    while (BUFF_CONST_GEN_MQ_GET_TOKEN(&line_pos, buff, " ,\t"))
    {
        if (ss1i(buff, &((*vpp)->elements[ count ])) == ERROR)
        {
            result = ERROR;
            break;
        }
        count++;
    }

    if (result == ERROR)
    {
        free_int_vector(*vpp);
        *vpp = NULL;
        return ERROR;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ============================================================================
 *                      sget_positive_int_vector
 *
 * Reads a positive intger vector from a string.
 *
 * This routine reads a blank or comma or tab separated list of integers from a
 * string into an integer vector. The values that are read must be positive
 * otherwise error is returned.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR if there are problems.
 *
 * Index: I/O, integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int sget_positive_int_vector(Int_vector** vpp, const char* line)
{
    const char* line_pos = line;
    const char* save_line_pos = line_pos;
    int    count;
    int    length = 0;
    int    result = NO_ERROR;
    char   buff[ 1000 ];


    while (BUFF_CONST_GEN_MQ_GET_TOKEN(&line_pos, buff, " ,"))
    {
        length++;
    }

    line_pos = save_line_pos;

    ERE(get_target_int_vector(vpp, length));

    count = 0;

    while (BUFF_CONST_GEN_MQ_GET_TOKEN(&line_pos, buff, " ,"))
    {
        if (ss1pi(buff, &((*vpp)->elements[ count ])) == ERROR)
        {
            result = ERROR;
            break;
        }
        else if ((*vpp)->elements[ count ] == 0)
        {
            set_error("A positive value is required.");
            return ERROR;
        }
        count++;
    }

    if (result == ERROR)
    {
        free_int_vector(*vpp);
        *vpp = NULL;
        return ERROR;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ============================================================================
 *                      sget_non_negative_int_vector
 *
 * Reads a positive intger vector from a string.
 *
 * This routine reads a blank or comma or tab separated list of integers from a
 * string into an integer vector. The values that are read must be non-negative
 * otherwise error is returned.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR if there are problems.
 *
 * Index: I/O, integer vector I/O
 *
 * -----------------------------------------------------------------------------
*/

int sget_non_negative_int_vector(Int_vector** vpp, const char* line)
{
    const char* line_pos = line;
    const char* save_line_pos = line_pos;
    int    count;
    int    length = 0;
    int    result = NO_ERROR;
    char   buff[ 1000 ];


    UNTESTED_CODE();

    while (BUFF_CONST_GEN_MQ_GET_TOKEN(&line_pos, buff, " ,\t"))
    {
        length++;
    }

    line_pos = save_line_pos;

    ERE(get_target_int_vector(vpp, length));

    count = 0;

    while (BUFF_CONST_GEN_MQ_GET_TOKEN(&line_pos, buff, " ,\t"))
    {
        if (ss1pi(buff, &((*vpp)->elements[ count ])) == ERROR)
        {
            result = ERROR;
            break;
        }
        count++;
    }

    if (result == ERROR)
    {
        free_int_vector(*vpp);
        *vpp = NULL;
        return ERROR;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_int_vector
(
    const char*       output_dir,
    const char*       file_name,
    const Int_vector* vp
)
{
    char out_file_name[ MAX_FILE_NAME_SIZE ];

    if (vp == NULL) return NO_ERROR;

    BUFF_CPY(out_file_name, output_dir);
    BUFF_CAT(out_file_name, DIR_STR);
    BUFF_CAT(out_file_name, file_name);

    return write_col_int_vector(vp, out_file_name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_raw_int_vector
(
    const char*       output_dir,
    const char*       file_name,
    const Int_vector* vp
)
{
    char out_file_name[ MAX_FILE_NAME_SIZE ];

    if (vp == NULL) return NO_ERROR;

    BUFF_CPY(out_file_name, output_dir);
    BUFF_CAT(out_file_name, DIR_STR);
    BUFF_CAT(out_file_name, file_name);

    return write_raw_int_vector(vp, out_file_name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_int_vector_vector
(
    const char*          output_dir,
    const char*          file_name,
    const Int_vector_vector* vvp
)
{
    char out_file_name[ MAX_FILE_NAME_SIZE ];

    if (vvp == NULL) return NO_ERROR;

    BUFF_CPY(out_file_name, output_dir);
    BUFF_CAT(out_file_name, DIR_STR);
    BUFF_CAT(out_file_name, file_name);

    return write_int_vector_vector(vvp, out_file_name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_copy_int_vector
 *
 * Copies a integer vector to an existing integer vector
 *
 * This routine copies the integer vector pointed to by source_vp to the integer
 * vector pointer to by target_vp, offset by "offset".
 *
 * Returns :
 *    NO_ERROR on success and ERROR on failure. This routine can only fail if
 *    the vector to be copied is too long, and this is treated as a bug.
 *
 * Author:
 *     Ernesto Brau
 *
 * Index : integer vectors
 *
 * -----------------------------------------------------------------------------
 */

int ow_copy_int_vector
(
    Int_vector*         target_vp,
    int                 offset,
    const Int_vector*   source_vp
)
{
    int   i;
    int   length;
    int* target_pos;
    int* source_pos;

    length = source_vp->length;

    if (length + offset > target_vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    target_pos = target_vp->elements + offset;
    source_pos = source_vp->elements;

    for (i=0; i<length; i++)
    {
       *target_pos = *source_pos;
        target_pos++;
        source_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               min_int_vector_element
 *
 * Returns the smallest element of an int vector
 *
 * This routine returns the smallest element in the vector specified by "vp".
 *
 * Returns:
 *     smallest vector element.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     max_int_vector_element
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/
int min_int_vector_element(const Int_vector* vp)
{
    int i;
    int min;
    int* element_ptr;


    if (vp->length == 0)
    {
        SET_CANT_HAPPEN_BUG();
        return 0.0;
    }

    element_ptr = vp->elements;

    min = *element_ptr;
    element_ptr++;

    for (i = 1; i<vp->length; i++)
    {
       if (*element_ptr < min)
       {
           min = *element_ptr;
       }
       element_ptr++;
   }

    return min;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               max_int_vector_element
 *
 * Returns the largest element of an int vector
 *
 * This routine returns the largest element in the vector specified by "vp".
 *
 * Returns:
 *     Largest vector element.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     min_int_vector_element
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/
int max_int_vector_element(const Int_vector* vp)
{
    int i;
    int max;
    int* element_ptr;


    if (vp->length == 0)
    {
        SET_CANT_HAPPEN_BUG();
        return 0.0;
    }

    element_ptr = vp->elements;

    max = *element_ptr;
    element_ptr++;

    for (i = 1; i<vp->length; i++)
    {
       if (*element_ptr > max)
       {
           max = *element_ptr;
       }
       element_ptr++;
   }

    return max;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               get_max_int_vector_element
 *
 * Finds the largest element of a vector
 *
 * This routine finds the largest element in the vector specified by "vp", and
 * puts it into *max_ptr. The index of the element is returned.  If you are only
 * interested in the index, then max_ptr can be set to NULL.
 * If vp equals NULL or if vp has zero length, this is treated as a bug, and
 * the return value is ERROR.
 *
 * Returns:
 *     The index of the largest vector element, or ERROR if there are problems.
 *
 * Related:
 *     get_min_int_vector_element, min_int_vector_element, max_int_vector_element
 *
 * Author:
 *     Ernesto Brau
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_max_int_vector_element(const Int_vector* vp, int* max_ptr)
{
    int i;
    int  max;
    int* element_ptr;
    int   max_index;

    if ( NULL == vp || vp->length == 0)
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    element_ptr = vp->elements;

    max = *element_ptr;
    max_index = 0;

    element_ptr++;

    for (i = 1; i<vp->length; i++)
    {
        if (*element_ptr > max)
        {
            max = *element_ptr;
            max_index = i;
        }
        element_ptr++;
    }

    if (max_ptr != NULL) *max_ptr = max;

    return max_index;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               get_min_int_vector_element
 *
 * Finds the smallest element of a vector
 *
 * This routine finds the smallest element in the vector specified by "vp", and
 * puts it into *min_ptr. The index of the element is returned.  If you are only
 * interested in the index, then min_ptr can be set to NULL.
 * If vp equals NULL or if vp has zero length, this is treated as a bug, and
 * the return value is ERROR.
 *
 * Returns:
 *     The index of the smallest vector element, or ERROR if there are problems.
 *
 * Related:
 *     min_int_vector_element,
 *     get_max_int_vector_element,
 *     max_int_vector_element
 *
 * Author:
 *     Andrew Predoehl
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_min_int_vector_element(const Int_vector* vp, int* min_ptr)
{
    int  i;
    int  min;
    int* element_ptr;
    int  min_index;

    if ( NULL == vp || 0 == vp->length )
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    element_ptr = vp->elements;

    min = *element_ptr;
    min_index = 0;

    element_ptr++;

    for ( i = 1; i < vp->length; ++i )
    {
        if (*element_ptr < min)
        {
            min = *element_ptr;
            min_index = i;
        }
        element_ptr++;
    }

    if ( min_ptr != NULL )
    {
        *min_ptr = min;
    }

    return min_index;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            multiply_int_vector_by_int_scalar
 *
 * Multiplies an integer vector by an integer scalar
 *
 * This routine multiplies the vector in "source_vp" by the scalar in "scalar",
 * and puts the result in the vector pointed to by "target_vpp". That vector
 * is created or resized as necessary.
 *
 * More specifically, the first argument is the adress of the target vector. If
 * the target vector itself is null, then a vector of the appropriate size is
 * created. If the target vector is the wrong size, it is resized. Finally, if
 * it is the right size, then the storage is recycled, as is.
 *
 * Returns :
 *    NO_ERROR on success, and ERROR on failure with the error message being
 *    set. Currently, failure can only be due to storage allocation error.
 *
 * Related:
 *    ow_multiply_int_vector_by_int_scalar
 *
 * Author:
 *     Ernesto Brau
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
 */

int multiply_int_vector_by_int_scalar
(
    Int_vector**      target_vpp,
    const Int_vector* source_vp,
    int               scalar
)
{
    Int_vector* target_vp;
    int    i;
    int    length;
    int*   source_pos;
    int*   target_pos;

    ERE(get_target_int_vector(target_vpp, source_vp->length));
    target_vp = *target_vpp;

    source_pos = source_vp->elements;
    target_pos = target_vp->elements;

    length = source_vp->length;
    for (i=0; i<length; i++)
    {
        (*target_pos) = (*source_pos) * scalar;

        source_pos++;
        target_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            ow_multiply_int_vector_by_int_scalar
 *
 * Multiplies a integer vector by an integer scalar
 *
 * This routine multiplies the vector in "source_vp" by the scalar in "scalar",
 * overwriting the contents of source_vp.
 *
 * Returns :
 *    NO_ERROR on success, and ERROR on failure with the error message being
 *    set. Currently, there is no way to fail (gracefully, that is).
 *
 * Related:
 *    multiply_int_vector_by_int_scalar
 *
 * Author:
 *     Ernesto Brau
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
 */

int ow_multiply_int_vector_by_int_scalar
(
    Int_vector* source_vp,
    int         scalar
)
{
    int     i;
    int     length;
    int*   source_pos;


    source_pos = source_vp->elements;
    length = source_vp->length;

    for (i=0; i<length; i++)
    {
        *source_pos *= scalar;
        source_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            add_int_scalar_to_int_vector
 *
 * Adds an integer scalar to a integer vector
 *
 * This routine adds the the scalar in "scalar" to the vector in "source_vp",
 * and puts the result in the vector pointed to by "target_vpp". That vector
 * is created or resized as necessary.
 *
 * More specifically, the first argument is the adress of the target vector. If
 * the target vector itself is null, then a vector of the appropriate size is
 * created. If the target vector is the wrong size, it is resized. Finally, if
 * it is the right size, then the storage is recycled, as is.
 *
 * Returns :
 *    NO_ERROR on success, and ERROR on failure with the error message being
 *    set. Currently, failure can only be due to storage allocation error.
 *
 * Related:
 *    ow_add_int_scalar_to_int_vector
 *
 * Author:
 *     Ernesto Brau
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
 */

int add_int_scalar_to_int_vector
(
    Int_vector**      target_vpp,
    const Int_vector* source_vp,
    int               scalar
)
{
    Int_vector* target_vp;
    int     i;
    int     length;
    int*   source_pos;
    int*   target_pos;


    ERE(get_target_int_vector(target_vpp, source_vp->length));
    target_vp = *target_vpp;

    source_pos = source_vp->elements;
    target_pos = target_vp->elements;

    length = source_vp->length;
    for (i=0; i<length; i++)
    {
        (*target_pos) = (*source_pos) + scalar;

        source_pos++;
        target_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            int_set_difference
 *
 * Computes the difference between two integer sets
 *
 * This routine computes the set differene between two sets of integers,
 * represented by the Int_vectors s1 and s2, and stores it in *diff. In other words,
 * it computes *diff = s1 \ s2. If s1 or s2 are NULL, they are treated as empty.
 * Also, if the result is an empty set, *diff is set to NULL.
 *
 * If diff is NULL, then a vector of the appropriate size is created. If diff
 * is the wrong size, it is resized. Finally, if it is the right size, then
 * the storage is recycled, as is.
 *
 * Returns :
 *    NO_ERROR on success, and ERROR on failure with the error message being
 *    set.
 *
 * Author:
 *     Ernesto Brau
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
 */

int int_set_difference
(
    Int_vector**      diff,
    const Int_vector* s1,
    const Int_vector* s2
)
{
    int count;
    Int_vector* keeps = NULL;
    int i, j;

    if(diff == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(s1 == NULL)
    {
        free_int_vector(*diff);
        *diff = NULL;
        return NO_ERROR;
    }

    if(s2 == NULL)
    {
        copy_int_vector(diff, s1);
        return NO_ERROR;
    }

    count = s1->length;
    ERE(get_initialized_int_vector(&keeps, s1->length, 1));
    for(i = 0; i < s1->length; i++)
    {
        for(j = 0; j < s2->length; j++)
        {
            if(s1->elements[i] == s2->elements[j])
            {
                keeps->elements[i] = 0;
                count--;
                break;
            }
        }
    }

    if(count == 0)
    {
        free_int_vector(*diff);
        *diff = NULL;
        free_int_vector(keeps);
        return NO_ERROR;
    }
    ERE(get_target_int_vector(diff, count));

    count = 0;
    for(i = 0; i < keeps->length; i++)
    {
        if(keeps->elements[i] == 1)
        {
            (*diff)->elements[count] = s1->elements[i];
            count++;
        }
    }

    free_int_vector(keeps);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             is_element_in_int_vector
 *
 * Figures out if element is in integer vector.
 *
 * This routine searches the integer vector vp for element elem; if elem is there,
 * it returns TRUE and sets *index to the first index where elem is in vp, unless
 * index is NULL. If elem is not there, it returns FALSE and sets *index to -1.
 *
 * Index: vectors
 *
 * Returns:
 *    TRUE (1) if elem is found and FALSE (0) otherwise. If vp is NULL, it returns
 *    ERROR.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_element_in_int_vector(int* index, const Int_vector* vp, int elem)
{
    int i;

    if(vp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for(i = 0; i < vp->length; i++)
    {
        if(vp->elements[i] == elem)
        {
            *index = i;
            return TRUE;
        }
    }

    *index = -1;
    return FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           get_int_dot_product
 *
 * Computes the dot product of two int vectors
 *
 * This routine computes the dot product two vectors and puts the result in
 * *result_ptr. If both vectors are NULL, then the result is zero.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure. Currenty, this outine will only
 *    fail if the vectors are of unequal length or if one of them is NULL but
 *    the other is not. If the routine fails, an appropriate error message being
 *    set.
 *
 * Note:
 *    Vectors of unequal lenths has been treated as a bug (see kjb_bug(3)) in
 *    some previous versions, and it is possible that we will go back to this
 *    behaviour in the future. Currently, we do not have a good convention for
 *    this, and putting one in place may change things. However, such a change
 *    should not affect code that checks for an error return.
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
 */

int get_int_dot_product
(
    const Int_vector* vp1,
    const Int_vector* vp2,
    long int*       result_ptr
)
{
    int   i;
    int   len;
    long int  sum = 0;
    int* pos1;
    int* pos2;


    if ((vp1 != NULL) || (vp2 != NULL))
    {
        ERE(check_same_int_vector_lengths(vp1, vp2, "get_dot_product"));

        len = vp1->length;
        pos1 = vp1->elements;
        pos2 = vp2->elements;

        for (i=0; i<len; i++)
        {
            sum += (*pos1) * (*pos2);
            pos1++;
            pos2++;
        }
    }

    *result_ptr = sum;

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               int_vector_magnitude
 *
 * Calculates the magnitude of an int vector
 *
 * This routine calculates the magnitude of the input vector specified by a
 * pointer to a Vector object. The vector magnitude is the square root of the
 * sum of squares of vector elements.
 *
 * Returns:
 *     Vector magnitude
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

double int_vector_magnitude(const Int_vector* vp)
{
    int     i;
    double  sum = 0.0;
    int     len = vp->length;
    int* pos = vp->elements;


    for (i = 0; i<len; i++)
    {
        sum += (*pos) * (*pos);
        pos++;
    }

    return sqrt(sum);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             sum_int_vector_squared_elements
 *
 * Returns the sum of the elements squared of an int vector
 *
 * This routine calculates the sum of the elements squared of a vector.
 *
 * Returns :
 *     The sum of the elements of a vector.
 *
 * Index : vectors
 *
 * -----------------------------------------------------------------------------
 */

long int sum_int_vector_squared_elements(const Int_vector* vp)
{
    int   i;
    long int  sum = 0;
    int* element_ptr;


    element_ptr = vp->elements;

    for (i = 0; i<vp->length; i++)
    {
       sum += ((*element_ptr) * (*element_ptr));
       element_ptr++;
    }

    return sum;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         check_same_int_vector_lengths
 *
 * Checks that two int vectors have the same length
 *
 * This routine checks that two vectors have the same length. If they do
 * not,then the an error message is set and ERROR is returned. The argument
 * context_str can be used to add more information; typically, it is the name of
 * the calling routine, but it could be some other string which works well as
 * the sequal to the message "failed in".  If context_str is NULL, then it is
 * not used.
 *
 * Macros:
 *     If different vector lengths are likely due to a bug, then you may want to
 *     wrap this routine in the macro ESBRE(), which prints the error, calls
 *     set_bug(), (the "SB"), and then returns ERROR. If different vector
 *     lengths are simply an error that the calling routine should deal with,
 *     then wrapping the call to this routine in ERE() saves typing.
 *
 * Returns:
 *    NO_ERROR if the matrices have the same dimensions, and ERROR otherwise.
 *    Depending on the bug handler in place, this routine may not return at all.
 *
 * Index: error handling, debugging
 *
 * -----------------------------------------------------------------------------
*/


int check_same_int_vector_lengths
(
    const Int_vector* first_vp,
    const Int_vector* second_vp,
    const char*   context_str
)
{
    int first_vector_length, second_vector_length;
    int result = NO_ERROR;


    if ((first_vp == NULL) && (second_vp == NULL))
    {
        return NO_ERROR;
    }
    else if (first_vp == NULL)
    {
        set_error("Null first vector but non-null second vector sent to dimension equality check.");
        result = ERROR;
    }
    else if (second_vp == NULL)
    {
        set_error("Null second vector but non-null first vector sent to dimension equality check.");
        result = ERROR;
    }

    if (result != ERROR)
    {
        first_vector_length = first_vp->length;
        second_vector_length = second_vp->length;

        if (first_vector_length != second_vector_length)
        {
            set_error("Mismatch in vector length (%d versus %d).",
                      first_vector_length, second_vector_length);
            result = ERROR;
        }
    }

    if (result == ERROR)
    {

        if (context_str != NULL)
        {
            add_error("Int Vector dimension equality check failed in %s.", context_str);
        }
        else
        {
            add_error("Int Vector dimension equality check failed.");
        }

    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ============================================================================
 *                        max_abs_int_vector_difference
 *
 * Returns the max absolute difference between two vectors
 *
 * This routine calculates the maximum of the absolute value of the elementwise
 * difference between two integer-valued vectors.  The result is zero even if
 * both vectors have zero length.
 *
 * Returns:
 *    The largest of the absolute values of differences between corresponding
 *    vector entries, or a negative number if there is an error or if the
 *    result is undefined.
 *    The two vectors must be the same size; if not, it is
 *    regarded as a bug not an error.  (See set_bug(3).)
 *
 * Index: error handling, debugging
 *
 * Author: Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * -----------------------------------------------------------------------------
 */

int max_abs_int_vector_difference
(
    const Int_vector* first_vp,
    const Int_vector* second_vp
)
{
    int i;
    const int *first_p, *second_p;
    int diff = 0;

    ESBRE(check_same_int_vector_lengths( first_vp, second_vp,
                                        "max_abs_int_vector_difference" ));

    first_p = first_vp -> elements;
    second_p = second_vp -> elements;

    for( i = 0; i < first_vp->length; ++i )
    {
        /* Do not put the right-hand expression for new_diff inside ABS_OF,
         * because ABS_OF is a macro and will evaluate the expression twice.
         * (As I rediscovered, painfully.)
         */
        int new_diff = *first_p++ - *second_p++;
        diff = MAX_OF( diff, ABS_OF( new_diff ) );
    }

    return diff;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


static int int_vec_is_perm_impl(
    const Int_vector* vp,
    int start_value,
    int* result,
    char* out_buf,
    size_t buf_size
)
{
    int i, size;
    Int_vector* aux = NULL;

    if (NULL == vp) /* the empty permutation */
    {
        if (result) *result = TRUE;
        return NO_ERROR;
    }

    size = vp -> length;
    ERE(get_initialized_int_vector(&aux, size, -1));

    if (result) *result = FALSE;
    for (i = 0; i < size; ++i)
    {
        int j = vp -> elements[i] - start_value;
        if (j < 0 || size <= j)
        {
            if (out_buf)
            {
                kjb_sprintf(out_buf, buf_size, "Input vector contains "
                            "out-of-range value %d.", vp -> elements[i]);
            }
            free_int_vector(aux);
            return NO_ERROR;
        }
        aux -> elements[j] = j;
    }

    for (i = 0; i < size; ++i)
    {
        if (aux -> elements[i] != i)
        {
            if (out_buf)
            {
                kjb_sprintf(out_buf, buf_size, "Input vector lacks value %d.",
                            i + start_value);
            }
            free_int_vector(aux);
            return NO_ERROR;
        }
    }

    if (result) *result = TRUE;
    free_int_vector(aux);
    return NO_ERROR;
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ============================================================================
 *                        int_vector_is_permutation
 *
 * Test if input contains a permutation
 *
 * This is a predicate that tests whether the contents of the Int_vector
 * constitute a permutation (possibly with an offset value).
 *
 * If the input pointer vp equals NULL or *vp contains zero elements, the
 * result is trivially true.  Otherwise, suppose vp -> size equals N.
 * Informally, this tests whether the vector is a "shuffling" of the integers
 * | start_value, start_value+1, . . ., start_value+N-1
 * without any repetitions or missing numbers.
 *
 * To be more formal, this tests whether the function p is a
 * permutation (a.k.a.  a bijection, a.k.a. a one-to-one mapping from D onto D)
 * where p and D are defined like so:
 *
 * | p : D --> D
 * | D = {0, 1, 2, . . ., N-1}
 * | p(i) = vp -> elements[i] - start_value
 *
 * Returns:
 *    This returns ERROR if we cannot allocate the auxiliary array this routine
 *    needs, or if output parameter 'result' equals NULL.  Otherwise this
 *    returns NO_ERROR and sets *result to TRUE or FALSE.  TRUE indicates that
 *    the input is a permutation (relative to start_value) as defined
 *    above.
 *
 * Related:
 *    get_string_why_int_vector_is_not_permutation
 *
 * Index: integer vectors
 *
 * Author: Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * -----------------------------------------------------------------------------
 */

int int_vector_is_permutation(
    const Int_vector* vp,
    int start_value,
    int* result
)
{
    NRE(result);
    return int_vec_is_perm_impl(vp, start_value, result, NULL, 0);
}




/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ============================================================================
 *                        get_string_why_int_vector_is_not_permutation
 *
 * Explain why input is not a permutation
 *
 * This is a helper function complementing int_vector_is_permutation().
 * Once you know an int vector is NOT a permutation, as defined for that
 * function, you can use this function to offer one reason why.  The output is
 * supplied as a C-style string written into the buffer provided by 'out_buf'
 * and 'buf_size.'  If the input is, in fact, a permutation, then the output
 * string is the empty string.  Lazy programmers can thus even use this
 * function as a roundabout way to avoid calling int_vector_is_permutation().
 *
 * Returns:
 *    This returns ERROR if we cannot allocate the auxiliary array this routine
 *    needs, if the output pointer out_buf equals NULL, or if buf_size is zero;
 *    also, an error string is generated.  Otherwise this returns NO_ERROR.
 *
 * Related:
 *    int_vector_is_permutation
 *
 * Index: integer vectors
 *
 * Author: Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * -----------------------------------------------------------------------------
 */

int get_string_why_int_vector_is_not_permutation(
    const Int_vector* vp,
    int start_value,
    char *out_buf,
    size_t buf_size
)
{
    NRE(out_buf);
    if (0 == buf_size)
    {
        set_error("Input string buffer has zero capacity -- cannot complete");
        return ERROR;
    }
    *out_buf = '\0'; /* if input is a permutation, output should be empty. */
    return int_vec_is_perm_impl(vp, start_value, NULL, out_buf, buf_size);
}


#ifdef __cplusplus
}
#endif

