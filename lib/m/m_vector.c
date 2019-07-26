
/* $Id: m_vector.c 22174 2018-07-01 21:49:18Z kobus $ */

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
#include "m/m_vector.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int ascend_compare_indexed_vector_elements
(
    const void* first_ptr,
    const void* second_ptr
);

static int descend_compare_indexed_vector_elements
(
    const void* first_ptr,
    const void* second_ptr
);

static int ascend_compare_vector_elements
(
    const void* first_ptr,
    const void* second_ptr
);

#ifndef EXPOSE_CREATE_ROUTINES
/*
 * We want to migrate all create routines to statics, but many are still used in
 * legacy code. As they get purged, we put them here. 
*/
static Vector_vector* create_vector_vector(int length);
static V_v_v* create_v3(int length);
static V_v_v_v* create_v4(int length);
#endif 

/* -------------------------------------------------------------------------- */

#ifdef TEST
double debug_vec_val
(
 const Vector* vp,
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
 *                          get_unity_vector
 *
 * Gets a vector of all ones.
 *
 * This routine is similar to get_target_vector(3), except that the vector
 * elements are set to one.
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_unity_vector(Vector** vpp, int len)
{


    return get_initialized_vector(vpp, len, 1.0);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_zero_vector
 *
 * Gets zero vector
 *
 * This routine is similar to get_target_vector(3), except that the vector
 * elements are set to zero.
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_zero_vector(Vector** vpp, int len)
{


    ERE(get_target_vector(vpp, len));

    return ow_zero_vector(*vpp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_initialized_vector
 *
 * Gets initialzed vector
 *
 * This routine is similar to get_target_vector(3), except that the elements of
 * the vector are set to the specitied value.
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_initialized_vector(Vector** vpp, int len, double initial_value)
{


    ERE(get_target_vector(vpp, len));
    ERE(ow_set_vector(*vpp, initial_value));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                get_target_vector
 *
 * Gets target vector
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of vectors. If *target_vpp is NULL, then this routine
 * creates the vector. If it is not null, and it is the right size, then this
 * routine does nothing. If it is the wrong size, then it is resized.
 *
 * In order to be memory efficient, we free before resizing. If the resizing
 * fails, then the original contents of the *target_vpp will be lost.  However,
 * (*target_vpp)->elements will be set to NULL, so (*target_vpp) can be safely
 * sent to free_vector().  Note that this is in fact the convention throughout
 * the KJB library--if destruction on failure is a problem (usually when
 * *target_vpp is global)--then work on a copy!
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION
int debug_get_target_vector
(
    Vector**    target_vpp,
    int         length,
    const char* file_name,
    int         line_number
)
{
    Vector* vp = *target_vpp;


    if (length < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (vp == NULL)
    {
        NRE(vp = debug_create_vector(length, file_name, line_number));
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
            NRE(vp->elements = DBL_DEBUG_MALLOC(length, file_name,
                                                 line_number));
        }
        else
        {
            vp->elements = NULL;
        }

        vp->length   = length;
        vp->max_length = length;
    }

    return NO_ERROR;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int get_target_vector(Vector** target_vpp, int length)
{
    Vector* vp = *target_vpp;


    if (length < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (vp == NULL)
    {
        NRE(vp = create_vector(length));
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
            NRE(vp->elements = DBL_MALLOC(length));
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

#ifdef DOCUMENT_CREATE_VERSIONS  /* Create confuses. Should likely be static. */

/* =============================================================================
 *                             create_vector
 *
 * Creates a vector
 *
 * This routine creates a vector and returns a pointer to it.
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_create_vector, which is the version available in the developement
 * library. In developement code memory is tracked so that memory leaks can be
 * found more easily. Furthermore, all memory freed is checked that it was
 * allocated by an L library routine.
 *
 * The routine free_vector should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    One success it returns a pointer to the vector.
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

#endif

#ifdef TRACK_MEMORY_ALLOCATION

Vector* debug_create_vector
(
    int         len,
    const char* file_name,
    int         line_number
)
{
    Vector* vp;


    if (len < 0)
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    NRN(vp = DEBUG_TYPE_MALLOC(Vector, file_name, line_number));

    if (len > 0)
    {
        /*
        //  Use debug_kjb_malloc as opposed to macros to pass down file_name
        //  and line_number.
        */

        vp->elements = DBL_DEBUG_MALLOC(len, file_name, line_number);

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

Vector* create_vector(int len)
{
    Vector* vp;


    if (len < 0)
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    NRN(vp = TYPE_MALLOC(Vector));

    if (len > 0)
    {
        vp->elements = N_TYPE_MALLOC(double, len);

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
 *                             free_vector
 *
 * Frees the space associated with a vector.
 *
 * This routine frees the storage associated with a vector.  If the argument is
 * NULL, then this routine returns safely without doing anything.
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

void free_vector(Vector* vp)
{

    if (vp != NULL)
    {
        if (vp->elements != NULL)
        {
#ifdef CHECK_VECTOR_INITIALZATION_ON_FREE
            /*
             * We must have a valid pointer to check the initialization,
             * otherwise problems such as double free can look like unitialized
             * memory.
            */
            if (kjb_debug_level >= 10)
            {
                check_initialization(vp->elements, vp->length, sizeof(double));
            }
#endif
            kjb_free(vp->elements);
        }
    }

    kjb_free(vp);

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Vector* create_zero_vector(int length)
{
    Vector* vp;


    NRN(vp = create_vector(length));

    if (ow_zero_vector(vp) == ERROR)
    {
        free_vector(vp);
        return NULL;
    }

    return vp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_zero_vector(Vector* vp)
{


    return ow_set_vector(vp, 0.0);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_set_vector(Vector* vp, double set_val)
{
    int i;
    int len;
    double* vec_pos;


    if (vp != NULL)
    {
        len = vp->length;
        vec_pos = vp->elements;

        for (i=0; i<len; i++)
        {
            *vec_pos = set_val;
            vec_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DOCUMENT_CREATE_VERSIONS  /* Create confuses. Should likely be static. */

/* =============================================================================
 *                                create_random_vector
 *
 *
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

#endif

Vector* create_random_vector(int length)
{
    Vector* vp;
    double*   elem_ptr;
    int    i;


    NRN(vp = create_vector(length));

    elem_ptr = vp->elements;

    for (i=0; i<length; i++)
    {
        *elem_ptr++ = kjb_rand();
    }

    return vp;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DOCUMENT_CREATE_VERSIONS  /* Create confuses. Should likely be static. */

/* =============================================================================
 *                                create_random_vector_2
 *
 *
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

#endif

Vector* create_random_vector_2(int length)
{
    Vector* vp;
    double*   elem_ptr;
    int    i;


    NRN(vp = create_vector(length));

    elem_ptr = vp->elements;

    for (i=0; i<length; i++)
    {
        *elem_ptr++ = kjb_rand_2();
    }

    return vp;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            get_random_vector
 *
 * Gets a uniform random vector
 *
 * This routine gets a matrix of the specified length, and fills it with uniform
 * random values between 0.0 and 1.0. The routine kjb_rand() is used.
 *
 * The first argument is the adress of the target vector. If the target vector
 * itself is null, then a vector  of the appropriate size is created. If the
 * target vector is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: vectors, random
 *
 * -----------------------------------------------------------------------------
*/

int get_random_vector(Vector** target_vpp, int length)
{
    Vector* vp;
    double*   elem_ptr;
    int    i;


    ERE(get_target_vector(target_vpp, length));
    vp = *target_vpp;

    elem_ptr = vp->elements;

    for (i=0; i<length; i++)
    {
        *elem_ptr++ = kjb_rand();
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_random_vector_2
 *
 * Gets a uniform random vector
 *
 * This routine is exaclty like get_random_vector(), exept that the alternative
 * random stream (e.g. kjb_rand_2()) is used.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: vectors, random
 *
 * -----------------------------------------------------------------------------
*/

int get_random_vector_2(Vector** target_vpp, int length)
{
    Vector* vp;
    double*   elem_ptr;
    int    i;


    ERE(get_target_vector(target_vpp, length));
    vp = *target_vpp;

    elem_ptr = vp->elements;

    for (i=0; i<length; i++)
    {
        *elem_ptr++ = kjb_rand_2();
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            get_random_unit_vector
 *
 * Gets a uniform random vector
 *
 * this routine gets a random unit-vector of the specified length, uniformly
 * distributed on the unit-sphere.  This routine uses rejection sampling to get
 * truly uniform samples.  The rejection rate is just below 50% for  a 3-vector, 
 * 69% for a 4-vector, 83% for a 5-vector, 92% for a 6-vector, and approaches 
 * 100% asymtotically above 6 dimensions.  Thus, this isn't recommended for
 * dimensions above 4, and a warning will display in this case.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: vectors, random
 *
 * -----------------------------------------------------------------------------
*/

int get_random_unit_vector(Vector** target_vpp, int length)
{
    {
        static int warned = FALSE;

        if(length > 3 && !warned)
        {
            warned = TRUE;
            TEST_PSE(("WARNING: Generating random unit vectors of dimension > 4 not recommended!  This could take a while...  Line %d of file %s.\n", __LINE__, __FILE__));
            TEST_PSE(("    (This message is only printed once.)\n"));
        }
    }

    ERE(get_random_vector(target_vpp, length));


    /* make sure vector lies inside unit-sphere */
    while(vector_magnitude(*target_vpp) > 1.0)
    {
        ERE(get_random_vector(target_vpp, length));
    }

    /* "push out" vector to lie on surface of unit-sphere */
    ERE(normalize_vector(target_vpp, *target_vpp, NORMALIZE_BY_MAGNITUDE));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ascend_sort_vector(Vector* vp)
{
#ifdef REALLY_TEST
    int i;
    int len = vp->length;
#endif 

    ERE(kjb_sort(vp->elements, vp->length, sizeof(double),
                 ascend_compare_vector_elements,
                 USE_CURRENT_ATN_HANDLING));

#ifdef REALLY_TEST
    for (i = 0; i < len; i++)
    {
        if (i > 0)
        {
            if (vp->elements[ i - 1] > vp->elements[ i ])
            {
                SET_SORT_BUG();
                return ERROR;
            }
        }
    }
#endif 

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               vp_get_indexed_vector
 *
 * Gets an indexed version of a vector
 *
 * This routine puts an indexed version of the vector init_vp, into *vpp,
 * allocating memory if needed. If init_vp is NULL, then *vpp will be freed and
 * become NULL also.
 *
 * The elements structure member of the Indexed_vector_element structure become
 * the same as those in init_vp, and the index structure member become the index
 * of the init_vp (i.e., 0, 1, 2, 3).
 *
 * Index: indexed vectors, vectors
 *
 * -----------------------------------------------------------------------------
*/

int vp_get_indexed_vector(Indexed_vector** vpp, const Vector* init_vp)
{
    Indexed_vector_element* vp_pos;
    int                     length;
    int                     i;


    if (init_vp == NULL)
    {
        free_indexed_vector(*vpp);
        *vpp = NULL;
        return NO_ERROR;
    }

    length = init_vp->length;

    ERE(get_target_indexed_vector(vpp, length));

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
 *                               get_zero_indexed_vector
 *
 * Gets an indexed vector set to zeros
 *
 * This routine sets up an indexed vector of the specified length, and sets the
 * element member of the Indexed_vector_element structure to zeros, and the
 * index structure member to 0, 1, 2, 3, ... ,  length-1.
 *
 * Index: indexed vectors, vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_zero_indexed_vector(Indexed_vector** vpp, int length)
{
    Indexed_vector_element* vp_pos;
    int                     i;


    ERE(get_target_indexed_vector(vpp, length));

    vp_pos = (*vpp)->elements;

    for (i=0; i<length; i++)
    {
        vp_pos->element = 0.0;
        vp_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         get_random_indexed_vector
 *
 * Gets an indexed vector set to random numbers
 *
 * This routine sets up an indexed vector of the specified length, and sets the
 * element member of the Indexed_vector_element structure to random number
 * between 0 and 1, and the index structure member to 0, 1, 2, 3, ... ,
 * length-1.
 *
 * Index: indexed vectors, vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_random_indexed_vector(Indexed_vector** vpp, int length)
{
    Indexed_vector_element* vp_pos;
    int                     i;


    ERE(get_target_indexed_vector(vpp, length));

    vp_pos = (*vpp)->elements;

    for (i=0; i<length; i++)
    {
        vp_pos->element = kjb_rand();
        vp_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               get_target_indexed_vector
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

int get_target_indexed_vector(Indexed_vector** vpp, int length)
{
    Indexed_vector*         vp     = *vpp;
    Indexed_vector_element* vp_pos;
    int                     i;


    if (length < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (vp == NULL)
    {
        NRE(vp = create_indexed_vector(length));
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
            vp->elements = N_TYPE_MALLOC(Indexed_vector_element, length);
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

Indexed_vector* create_indexed_vector(int length)
{
    Indexed_vector* result_ptr;
    int i;


    NRN(result_ptr = TYPE_MALLOC(Indexed_vector));

    result_ptr->length = length;
    result_ptr->elements = N_TYPE_MALLOC(Indexed_vector_element, length);

    if (result_ptr->elements == NULL)
    {
        kjb_free(result_ptr);
        return NULL;
    }

    for (i = 0; i < length; i++)
    {
        result_ptr->elements[ i ].index = i;
    }

    return result_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Indexed_vector* create_zero_indexed_vector(int length)
{
    Indexed_vector* result_ptr;
    int i;


    NRN(result_ptr = TYPE_MALLOC(Indexed_vector));

    result_ptr->length = length;
    result_ptr->elements = N_TYPE_MALLOC(Indexed_vector_element, length);

    if (result_ptr->elements == NULL)
    {
        kjb_free(result_ptr);
        return NULL;
    }

    for (i = 0; i < length; i++)
    {
        result_ptr->elements[ i ].index = i;
        result_ptr->elements[ i ].element = 0.0;
    }

    return result_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Indexed_vector* vp_create_indexed_vector(const Vector* vp)
{
    Indexed_vector* result_ptr;
    int length, i;


    NRN(result_ptr = TYPE_MALLOC(Indexed_vector));

    length = vp->length;

    result_ptr->length = length;
    result_ptr->elements = N_TYPE_MALLOC(Indexed_vector_element, length);

    if (result_ptr->elements == NULL)
    {
        kjb_free(result_ptr);
        return NULL;
    }

    for (i=0; i<length; i++)
    {
        result_ptr->elements[i].index = i;
        result_ptr->elements[i].element = vp->elements[ i ];
    }

    return result_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            free_indexed_vector
 *
 * Frees an indexed vector
 *
 * This routine frees storage associated with an indexed vector. As with all KJB
 * library free routines, a NULL argument is OK. 
 *
 * Index: indexed vectors, vectors
 *
 * -----------------------------------------------------------------------------
*/

void free_indexed_vector(Indexed_vector* vp)
{

    if (vp != NULL)
    {
        kjb_free(vp->elements);
        kjb_free(vp);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               ascend_sort_indexed_vector
 *
 * Sorts an indexed vector
 *
 * This routine sorts an indexed vector so that the element index structure
 * members are in ascending order. The index structure members are moved in the
 * sorting operation but are not changed. Thus they keep track of where the
 * elements came from (which is why this function is useful).
 *
 * Index: indexed vectors, vectors
 *
 * -----------------------------------------------------------------------------
*/

int ascend_sort_indexed_vector(Indexed_vector* vp)
{
#ifdef REALLY_TEST
    int i, j;
    int len = vp->length;
    int unique_indicies = TRUE; 
#endif 


#ifdef REALLY_TEST
    for (i = 0; i < len; i++)
    {
        for (j = 0; j < len; j++)
        {
            if (j == i) continue;

            if (vp->elements[ i ].index == vp->elements[ j ].index)
            {
                unique_indicies = FALSE; 
                break;
            }
        }

    }

    if (! unique_indicies)
    {
        verbose_pso(2, "Sorting an indexed vector with non-unique indicies.\n"); 
    }
#endif 

    ERE(kjb_sort(vp->elements, vp->length, sizeof(Indexed_vector_element),
                 ascend_compare_indexed_vector_elements,
                 USE_CURRENT_ATN_HANDLING));

#ifdef REALLY_TEST
    for (i = 0; i < len; i++)
    {
        if (i > 0)
        {
            if (vp->elements[ i - 1].element > vp->elements[ i ].element)
            {
                SET_SORT_BUG();
                return ERROR;
            }
        }

        if (unique_indicies)
        {
            for (j = 0; j < len; j++)
            {
                if (j == i) continue;

                if (vp->elements[ i ].index == vp->elements[ j ].index)
                {
                    SET_SORT_BUG();
                    return ERROR;
                }
            }
        }
    }
#endif 

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

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

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              descend_sort_indexed_vector
 *
 * Sorts an indexed vector
 *
 * This routine sorts an indexed vector so that the element index structure
 * members are in descending order. The index structure members are moved in the
 * sorting operation but are not changed. Thus they keep track of where the
 * elements came from (which is why this function is useful).
 *
 * Index: indexed vectors, vectors
 *
 * -----------------------------------------------------------------------------
*/

int descend_sort_indexed_vector(Indexed_vector* vp)
{
#ifdef REALLY_TEST
    int i, j;
    int len = vp->length;
    int unique_indicies = TRUE; 
#endif 


#ifdef REALLY_TEST
    for (i = 0; i < len; i++)
    {
        for (j = 0; j < len; j++)
        {
            if (j == i) continue;

            if (vp->elements[ i ].index == vp->elements[ j ].index)
            {
                unique_indicies = FALSE; 
                break;
            }
        }

    }

    if (! unique_indicies)
    {
        verbose_pso(2, "Sorting an indexed vector with non-unique indicies.\n"); 
    }
#endif 

    ERE(kjb_sort(vp->elements, vp->length, sizeof(Indexed_vector_element),
                 descend_compare_indexed_vector_elements,
                 USE_CURRENT_ATN_HANDLING));

#ifdef REALLY_TEST
    for (i = 0; i < len; i++)
    {
        if (i > 0)
        {
            if (vp->elements[ i - 1].element < vp->elements[ i ].element)
            {
                SET_SORT_BUG();
                return ERROR;
            }
        }

        if (unique_indicies)
        {
            for (j = 0; j < len; j++)
            {
                if (j == i) continue;

                if (vp->elements[ i ].index == vp->elements[ j ].index)
                {
                    SET_SORT_BUG();
                    return ERROR;
                }
            }
        }
    }
#endif 

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

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

static int ascend_compare_vector_elements
(
    const void* first_ptr,
    const void* second_ptr
)
{
    const double first_elem = *((const double*)first_ptr);
    const double second_elem = *((const double*)second_ptr);

    if (first_elem > second_elem)
    {
        return FIRST_ITEM_GREATER;
    }
    else if (second_elem > first_elem)
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
 *                             get_target_vector_vector
 *
 * Gets a target vector vector
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of vector arrays. If *target_vvpp is
 * NULL, then this routine creates the vector vector. If it is not null, and it
 * is the right size, then this routine does nothing. If it is the wrong size,
 * then it is resized.
 *
 * The routine free_vector_vector should be used to dispose of the storage once
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

int get_target_vector_vector(Vector_vector** target_vvpp, int count)
{
    int i;

    if(target_vvpp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(*target_vvpp == NULL)
    {
        NRE(*target_vvpp = create_vector_vector(count));
        return NO_ERROR;
    }

    if((*target_vvpp)->length == count)
    {
        return NO_ERROR;
    }

    for(i = 0; i < (*target_vvpp)->length; i++)
    {
        free_vector((*target_vvpp)->elements[i]);
    }
    NRE((*target_vvpp)->elements = N_TYPE_REALLOC((*target_vvpp)->elements, Vector*, count));
    (*target_vvpp)->length = count;
    for(i = 0; i < count; i++)
    {
        (*target_vvpp)->elements[i] = NULL;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef EXPOSE_CREATE_ROUTINES 
/* =============================================================================
 *                             create_vector_vector
 *
 * Creates a vector vector
 *
 * This routine creates a vector vector of size "length" which is a type for a
 * vector of vectors.  All vector pointers are set to NULL.
 *
 * The routine free_vector_vector should be used to dispose of the storage once
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

Vector_vector* create_vector_vector(int length)
#else
static Vector_vector* create_vector_vector(int length)
#endif 
{
    Vector_vector* vvp;
    int           i;


    NRN(vvp = TYPE_MALLOC(Vector_vector));
    vvp->length = length;

    if (length > 0)
    {
        NRN(vvp->elements = N_TYPE_MALLOC(Vector*, length));
    }
    else
    {
        vvp->elements = NULL;
    }

    for (i=0; i<length; i++)
    {
        vvp->elements[ i ] = NULL;
    }

    return vvp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_vector_vector
 *
 * Frees the storage in a vector vector
 *
 * This routine frees the storate in a vector array.
 *
 * Index: memory allocation, arrays, vectors
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * -----------------------------------------------------------------------------
*/

void free_vector_vector(Vector_vector* vvp)
{
    int count, i;
    Vector** vp_pos;


    if (vvp == NULL) return;

    vp_pos = vvp->elements;
    count = vvp->length;

    for (i=0; i<count; i++)
    {
        free_vector(*vp_pos);
        vp_pos++;
    }

    kjb_free(vvp->elements);
    kjb_free(vvp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             allocate_2D_vp_array
 *
 * Creates a 2D array of vector pointers
 *
 * This routine returns a pointer P to a two-dimensional array of vector
 * pointers. P can be de-referenced by P[row][col] to obtain the storage of a
 * Vector*. P[0] points to a contiguous area of memory which contains the
 * entire array. P[1] is a short-cut pointer to the first element of the second
 * row, and so on. Thus the array can be accessed sequentually starting at
 * P[0], or explicitly by row and column. (Note that this is not the common way
 * of setting up a two-dimensional array--see below).
 *
 * This routine sets all the pointers to NULL.
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_2D_vp_array, which is the version available in the
 * development library. In development code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_2D_vp_array_and_vectors should be used to dispose of the
 * storage once it is no longer needed.
 *
 * Warning:
 *    The structure of the returned array is somewhat different than a more
 *    common form of a two-dimensional array in "C" where each row is allocated
 *    separately. Here the storage area is contiguous. This allows for certain
 *    operations to be done quickly, but note the following IMPORTANT point:
 *    num_rows cannot be swapped by simply swapping row pointers!
 *
 * Note:
 *    Naming convention--this routine is called "allocate_2D_vp_array" rather
 *    than "create_2D_vp_array" or "create_vector_matrix" because we are not
 *    creating an abstract data type, but rather just allocating raw storage.
 *    For many puposes this routine is a hack waiting for a
 *    get_target_vector_matrix routine to be written, but it has uses outside of
 *    this as well.
 *
 * Index: memory allocation, arrays, vectors
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

Vector*** debug_allocate_2D_vp_array
(
    int         num_rows,
    int         num_cols,
    const char* file_name,
    int         line_number
)
{
    Vector***   array;
    Vector***   array_pos;
    Vector**    col_ptr;
    int         i, j;
    Malloc_size num_bytes;


    if ((num_rows <= 0) || (num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    num_bytes = num_rows * sizeof(Vector**);
    NRN(array = (Vector ***)debug_kjb_malloc(num_bytes, file_name, line_number));

    num_bytes = num_rows * num_cols * sizeof(Vector*);
    col_ptr = (Vector**)debug_kjb_malloc(num_bytes, file_name, line_number);

    if (col_ptr == NULL)
    {
        kjb_free(array);
        return NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    for(i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            array[ i ][ j ] = NULL;
        }
    }

    return array;
}
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

Vector*** allocate_2D_vp_array(int num_rows, int num_cols)
{
    Vector*** array;
    Vector*** array_pos;
    Vector**  col_ptr;
    int       i, j;


    if ((num_rows <= 0) || (num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(Vector**, num_rows));

    col_ptr = N_TYPE_MALLOC(Vector*, num_rows * num_cols);

    if (col_ptr == NULL)
    {
        kjb_free(array);
        return NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    for(i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            array[ i ][ j ] = NULL;
        }
    }

    return array;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          free_2D_vp_array_and_vectors
 *
 * Frees a raw 2D array of vectors
 *
 * This routine frees the storage obtained from allocate_2D_vp_array and the
 * vectors pointed to by the pointers. If the argument is NULL, then this
 * routine returns safely without doing anything.
 *
 * Index: memory allocation, arrays, vectors
 *
 * -----------------------------------------------------------------------------
*/

void free_2D_vp_array_and_vectors
(
    Vector*** array,
    int       num_rows,
    int       num_cols
)
{
    int i, j;


    if (array == NULL) return;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            free_vector(array[ i ][ j ]);
        }
    }

    kjb_free(*array);
    kjb_free(array);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_target_v3
 *
 * Gets a target vector vector vector
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of vector vector vectors. If *target_vvvpp is NULL, then
 * this routine creates the vector vector vector. If it is not null, and it is
 * the right size, then this routine does nothing. If it is the wrong size,
 * then it is resized.
 *
 * The routine free_v3 should be used to dispose of the storage once
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

int get_target_v3(V_v_v** target_vvvpp, int length)
{

    free_v3(*target_vvvpp);
    NRE(*target_vvvpp = create_v3(length));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef EXPOSE_CREATE_ROUTINES  

/* =============================================================================
 *                             create_v3
 *
 * Creates a vector vector vector
 *
 * This routine creates a vector vector vector of size "length". All vector
 * vector pointers are set to NULL.
 *
 * The routine free_v3 should be used to dispose of the storage once
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

V_v_v* create_v3(int length)
#else 
static V_v_v* create_v3(int length)
#endif
{
    V_v_v* vvvp;
    int    i;


    NRN(vvvp = TYPE_MALLOC(V_v_v));
    vvvp->length = length;
    NRN(vvvp->elements = N_TYPE_MALLOC(Vector_vector*, length));

    for (i=0; i<length; i++)
    {
        vvvp->elements[ i ] = NULL;
    }

    return vvvp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_v3
 *
 * Frees the storage in a vector vector vector
 *
 * This routine frees the storate in a vector vector vector.
 *
 * Index: memory allocation, arrays, vectors
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * -----------------------------------------------------------------------------
*/

void free_v3(V_v_v* vvvp)
{
    int             count;
    int             i;
    Vector_vector** vvp_pos;


    if (vvvp == NULL) return;

    vvp_pos = vvvp->elements;
    count = vvvp->length;

    for (i=0; i<count; i++)
    {
        free_vector_vector(*vvp_pos);
        vvp_pos++;
    }

    kjb_free(vvvp->elements);
    kjb_free(vvvp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_target_v4
 *
 * Gets a target vector vector vector
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of vector vector vectors. If *target_vvvvpp is NULL, then
 * this routine creates the vector vector vector. If it is not null, and it is
 * the right size, then this routine does nothing. If it is the wrong size,
 * then it is resized.
 *
 * The routine free_v4 should be used to dispose of the storage once
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

int get_target_v4(V_v_v_v** target_vvvvpp, int length)
{

    free_v4(*target_vvvvpp);
    NRE(*target_vvvvpp = create_v4(length));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef EXPOSE_CREATE_ROUTINES 

/* =============================================================================
 *                             create_v4
 *
 * Creates a vector vector vector vector
 *
 * This routine creates a vector vector vector vector of size "length". All
 * vector vector vector pointers are set to NULL.
 *
 * The routine free_v4 should be used to dispose of the storage once
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

V_v_v_v* create_v4(int length)
#else 
static V_v_v_v* create_v4(int length)
#endif
{
    V_v_v_v* vvvvp;
    int    i;


    NRN(vvvvp = TYPE_MALLOC(V_v_v_v));
    vvvvp->length = length;
    NRN(vvvvp->elements = N_TYPE_MALLOC(V_v_v*, length));

    for (i=0; i<length; i++)
    {
        vvvvp->elements[ i ] = NULL;
    }

    return vvvvp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_v4
 *
 * Frees the storage in a vector vector vector
 *
 * This routine frees the storate in a vector vector.
 *
 * Index: memory allocation, arrays, vectors
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * -----------------------------------------------------------------------------
*/

void free_v4(V_v_v_v* vvvvp)
{
    int     count;
    int     i;
    V_v_v** vvvp_pos;


    if (vvvvp == NULL) return;

    vvvp_pos = vvvvp->elements;
    count = vvvvp->length;

    for (i=0; i<count; i++)
    {
        free_v3(*vvvp_pos);
        vvvp_pos++;
    }

    kjb_free(vvvvp->elements);
    kjb_free(vvvvp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

