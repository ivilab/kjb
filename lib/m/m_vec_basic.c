
/* $Id: m_vec_basic.c 21522 2017-07-22 15:14:27Z kobus $ */

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
#include "m/m_vec_basic.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                             split_v4
 *
 * Splits a target vector vector vector vector
 *
 * This routine splits a vector vector vector vector with the
 * creation/over-writing semantics used in the KJB library in the case of vector
 * vector vectors according to a list provided in index_list_vp.  If
 * *target_1_vvvvpp or *target_2_vvvvpp is NULL, then this routine creates the
 * vector vector. If they are not null, and are the right size, then the storage
 * is recycled. If they are the wrong size, then they are resized.
 *
 * The routine free_v4 should be used to dispose of the storage once it is no
 * longer needed.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to two arrays.
 *
 * Author:
 *    Ranjini Swaminathan
 *
 * Index: memory allocation, arrays, vectors
 *
 * -----------------------------------------------------------------------------
 */

int split_v4
(
    V_v_v_v**         target_1_vvvvpp,
    V_v_v_v**         target_2_vvvvpp,
    const V_v_v_v*    source_vvvvp,
    const Int_vector* index_list_vp
)
{
    int length;
    int index_list_length;
    int i;
    int source_index = 0;
    int target_index_1 = 0;
    int target_index_2 = 0;

    if (source_vvvvp == NULL)
    {
        free_v4(*target_1_vvvvpp);
        *target_1_vvvvpp = NULL;

        free_v4(*target_2_vvvvpp);
        *target_2_vvvvpp = NULL;
        return NO_ERROR;
    }

    length = source_vvvvp->length;
    index_list_length = index_list_vp->length;

    ERE(get_target_v4(target_1_vvvvpp, index_list_length));
    ERE(get_target_v4(target_2_vvvvpp, length - index_list_length));

    for (i = 0; i < length; i++)
    {
        if(i == index_list_vp->elements[ source_index ])
        {
            ERE(copy_v3(&((*target_1_vvvvpp)->elements[ target_index_1 ]),
                        source_vvvvp->elements[ i ]));
            target_index_1++;
            source_index++;
        }
        else
        {
            ERE(copy_v3(&((*target_2_vvvvpp)->elements[ target_index_2 ]),
                        source_vvvvp->elements[ i ]));
            target_index_2++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             split_vector_vector
 *
 * Splits a vector vector into two target vector vectors
 *
 * This routine splits a vector vector with the creation/over-writing semantics
 * used in the KJB library in the case of vector arrays. If *target_1_vvpp or
 * *target_2_vvpp is NULL, then this routine creates the vector vector.
 * If they  not null, and are the right size, then the storage is recycled.
 * If the wrong size, then they are resized.
 *
 * The routine free_vector_vector should be used to dispose of the storage once
 * the targets are no longer needed
 *
 * Index: memory allocation, arrays, vectors
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the two target arrays.
 *
 * Author:
 *    Ranjini Swaminathan
 *
 * -----------------------------------------------------------------------------
 */

int split_vector_vector
(
    Vector_vector**      target_1_vvpp,
    Vector_vector**      target_2_vvpp,
    const Vector_vector* source_vvp,
    const Int_vector* index_list_vp
)
{
    int length;
    int index_list_length;
    int i;
    int source_index = 0;
    int source_index_2 = 0;

    UNTESTED_CODE();
    if (source_vvp == NULL)
    {
        free_vector_vector(*target_1_vvpp);
        free_vector_vector(*target_2_vvpp);
        *target_1_vvpp = NULL;
        *target_2_vvpp = NULL;
        return NO_ERROR;
    }

    length = source_vvp->length;
    index_list_length = index_list_vp->length;

    if(index_list_length >= length)
    {

    }

    ERE(get_target_vector_vector(target_1_vvpp, index_list_length));
    ERE(get_target_vector_vector(target_2_vvpp, length - index_list_length));

    for (i = 0; i < length; i++)
    {
        /*
        // Ranjini : [ ASSUMPTION ] index list is sorted
         */
        if((source_index < index_list_vp->length) && (i == index_list_vp->elements[source_index]))
        {
            ERE(copy_vector(&((*target_1_vvpp)->elements[ source_index ]),
                            source_vvp->elements[ i ]));
            source_index++;
        }
        else
        {
            ERE(copy_vector(&((*target_1_vvpp)->elements[ source_index_2 ]),
                            source_vvp->elements[ i ]));
            source_index_2++;
        }

    }

    return NO_ERROR;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           split_vector
 *
 * Splits a vector
 *
 * This routine splits the vector pointed to by source_vp to the vectors pointed
 * to by *target_1_vp and *target_2_vp. If either of *target_1_vp or *target_2_vp
 * is NULL, then it is created. If it is the wrong size, it is resized.
 * The vector pointed to by *target_1_vp contains elements from source_vp that are
 * indexed by the vector pointed to by index_list_vp and *target_2_vp points to
 * those elements that are not on the list.
 *
 * Returns :
 *    On success, this routine NO_ERROR, and failure, it returns NULL, and sets
 *    an error message. Currently this routine can fail if storage
 *    allocation fails or if source_vp cannot be indexed by one or more entries
 *    in the index list .
 *
 * Author:
 *    Ranjini Swaminathan
 *
 * Index : vectors
 *
 * -----------------------------------------------------------------------------
 */

int split_vector
(
    Vector** target_1_vpp,
    Vector** target_2_vpp,
    const Vector* source_vp,
    const Int_vector* index_list_vp
)
{

    int   i;
    int   length;
    int index_list_length;
    int source_index = 0;
    int target_index_1 = 0;
    int target_index_2 = 0;

    UNTESTED_CODE();
    if (source_vp == NULL)
    {
        free_vector(*target_1_vpp);
        free_vector(*target_2_vpp);
        *target_1_vpp = NULL;
        *target_2_vpp = NULL;
        return NO_ERROR;
    }

    index_list_length = index_list_vp->length;
    length = source_vp->length;

    ERE(get_target_vector(target_1_vpp, index_list_length));
    ERE(get_target_vector(target_2_vpp, length - index_list_length));



    for (i=0; i<length; i++)
    {
        if((source_index < index_list_vp->length) && (i == index_list_vp->elements[source_index]))
        {
            (*target_1_vpp)->elements[target_index_1] = source_vp->elements[i];
            source_index++;
            target_index_1++;
        }
        else
        {
            (*target_2_vpp)->elements[target_index_2] = source_vp->elements[i];
            target_index_2++;
        }
    }


    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             copy_v4
 *
 * Copies a target vector vector vector vector
 *
 * This routine copies a vector vector vector vector with the
 * creation/over-writing semantics used in the KJB library in the case of
 * vector vector vectors. If *target_vvvvpp is NULL, then this routine creates
 * the vector vector. If it is not null, and it is the right size, then the
 * storage is recycled. If it is the wrong size, then it is resized.
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

int copy_v4(V_v_v_v** target_vvvvpp, const V_v_v_v* source_vvvvp)
{
    int length;
    int i;

    if (source_vvvvp == NULL)
    {
        free_v4(*target_vvvvpp);
        *target_vvvvpp = NULL;
        return NO_ERROR;
    }

    length = source_vvvvp->length;

    ERE(get_target_v4(target_vvvvpp, length));

    for (i = 0; i < length; i++)
    {
        ERE(copy_v3(&((*target_vvvvpp)->elements[ i ]),
                    source_vvvvp->elements[ i ]));
    }

    return NO_ERROR;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             copy_v3
 *
 * Copies a target vector vector vector
 *
 * This routine copies a vector vector vector with the creation/over-writing
 * semantics used in the KJB library in the case of vector vector vectors. If
 * *target_vvvpp is NULL, then this routine creates the vector vector. If it is
 * not null, and it is the right size, then the storage is recycled. If it is
 * the wrong size, then it is resized.
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

int copy_v3(V_v_v** target_vvvpp, const V_v_v* source_vvvp)
{
    int length;
    int i;

    if (source_vvvp == NULL)
    {
        free_v3(*target_vvvpp);
        *target_vvvpp = NULL;
        return NO_ERROR;
    }

    length = source_vvvp->length;

    ERE(get_target_v3(target_vvvpp, length));

    for (i = 0; i < length; i++)
    {
        ERE(copy_vector_vector(&((*target_vvvpp)->elements[ i ]),
                               source_vvvp->elements[ i ]));
    }

    return NO_ERROR;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           copy_indexed_vector
 *
 * Copies an indexed vector
 *
 * This routine copies the indexed vector pointed to by source_vp to the vector
 * pointer to by *target_vp. If *target_vp is NULL, then it is created. If it
 * is the wrong size, it is resized.
 *
 * Returns :
 *    On success, this routine returns a pointer to a newly created vector
 *    which is a copy of the input vector. On failure, it returns NULL, and sets
 *    an error message. Currently this routine can only fail if storage
 *    allocation fails.
 *
 * Index : vectors
 *
 * -----------------------------------------------------------------------------
 */

int copy_indexed_vector
(
    Indexed_vector**      target_vpp,
    const Indexed_vector* source_vp
)
{
    int   i;
    int   length;
    Indexed_vector_element* target_pos;
    Indexed_vector_element* source_pos;


    if (source_vp == NULL)
    {
        free_indexed_vector(*target_vpp);
        *target_vpp = NULL;
        return NO_ERROR;
    }

    length = source_vp->length;

    ERE(get_target_indexed_vector(target_vpp, length));

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
 *                           create_vector_copy
 *
 * Creates a copy of a vector
 *
 * This routine creates a copy of the input vector, and returns a pointer to it.
 *
 * Returns :
 *    On success, this routine returns a pointer to a newly created vector
 *    which is a copy of the input vector. On failure, it returns NULL, and sets
 *    an error message. Currently this routine can only fail if storage
 *    allocation fails.
 *
 * Index : vectors
 *
 * -----------------------------------------------------------------------------
 */

Vector* create_vector_copy(const Vector* source_vp)
{
    Vector* target_vp = NULL;


    ERN(copy_vector(&target_vp, source_vp));
    return target_vp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             select_from_vector_vector
 *
 * Copies a selected vectors from a vector vector
 *
 * This routine copies selected vectors from a  vector vector with the
 * creation/over-writing semantics used in the KJB library in the case of vector
 * arrays. If *target_vvpp is NULL, then this routine creates the vector vector.
 * If it is not null, and it is the right size, then the storage is recycled. If
 * it is the wrong size, then it is resized.
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

int select_from_vector_vector
(
    Vector_vector**      target_vvpp,
    const Vector_vector* source_vvp,
    const Int_vector*    enable_vp
)
{
    int target_length = 0;
    int source_length;
    int i;


    UNTESTED_CODE(); 

    if (enable_vp == NULL)
    {
        return copy_vector_vector(target_vvpp, source_vvp);
    }

    if (source_vvp == NULL)
    {
        free_vector_vector(*target_vvpp);
        *target_vvpp = NULL;
        return NO_ERROR;
    }

    source_length = source_vvp->length;

    if (enable_vp->length != source_length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for (i = 0; i < source_length; i++)
    {
        if (enable_vp->elements[ i ])
        {
            target_length++; 
        }
    }

    ERE(get_target_vector_vector(target_vvpp, target_length));

    target_length = 0; 

    for (i = 0; i < source_length; i++)
    {
        if (enable_vp->elements[ i ])
        {
            ERE(copy_vector(&((*target_vvpp)->elements[ target_length ]),
                            source_vvp->elements[ i ]));
            target_length++;
        }
    }

    return NO_ERROR;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             copy_vector_vector
 *
 * Copies a target vector vector
 *
 * This routine copies a vector vector with the creation/over-writing semantics
 * used in the KJB library in the case of vector arrays. If *target_vvpp is
 * NULL, then this routine creates the vector vector. If it is not null, and it
 * is the right size, then the storage is recycled. If it is the wrong size,
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

int copy_vector_vector
(
    Vector_vector**      target_vvpp,
    const Vector_vector* source_vvp
)
{
    int length;
    int i;

    if (source_vvp == NULL)
    {
        free_vector_vector(*target_vvpp);
        *target_vvpp = NULL;
        return NO_ERROR;
    }

    length = source_vvp->length;

    ERE(get_target_vector_vector(target_vvpp, length));


    for (i = 0; i < length; i++)
    {
        ERE(copy_vector(&((*target_vvpp)->elements[ i ]),
                        source_vvp->elements[ i ]));
    }

    return NO_ERROR;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int copy_vector_vector_block
(
    Vector_vector**      target_vvpp,
    const Vector_vector* source_vvp,
    int start_index,
    int length
)
{
    int i;

    if (source_vvp == NULL)
    {
        free_vector_vector(*target_vvpp);
        *target_vvpp = NULL;
        return NO_ERROR;
    }

    if (start_index + length > source_vvp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_vector_vector(target_vvpp, length));

    for (i = 0; i < length; i++)
    {
        ERE(copy_vector(&((*target_vvpp)->elements[ i ]),
                        source_vvp->elements[ start_index + i ]));
    }

    return NO_ERROR;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           copy_vector_segment
 *
 * Copies part of a vector
 *
 * This routine copies part of the vector pointed to by source_vp to the vector
 * pointer to by *target_vp.  If *target_vp is NULL, then it is created.  If it
 * is the wrong size, it is resized.  The copying begins at start_index, and
 * goes on for the specified length.
 *
 * Returns :
 *    On success, this routine returns a pointer to a newly created vector
 *    which is a copy of the input vector.  On failure, it returns NULL, and
 *    sets an error message.  This routine can fail if storage allocation
 *    fails, if start_index is negative, if length is negative,
 *    or if the quantity (start_index + length) exceeds the length of the
 *    source_vp vector.
 *
 * Index : vectors
 *
 * -----------------------------------------------------------------------------
 */

int copy_vector_segment(Vector** target_vpp, const Vector* source_vp,
                        int start_index, int length)
{
    int   i;
    double* target_pos;
    double* source_pos;


    if (source_vp == NULL)
    {
        free_vector(*target_vpp);
        *target_vpp = NULL;
        return NO_ERROR;
    }

    if (        start_index < 0
            ||  length < 0
            ||  start_index + length > source_vp->length )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_vector(target_vpp, length));

    source_pos = source_vp->elements + start_index;
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
 *                           copy_vector
 *
 * Copies a vector
 *
 * This routine copies the vector pointed to by source_vp to the vector pointer
 * to by *target_vp. If *target_vp is NULL, then it is created. If it is the
 * wrong size, it is resized.
 *
 * Returns :
 *    On success, this routine NO_ERROR, and failure, it returns NULL, and sets
 *    an error message. Currently this routine can only fail if storage
 *    allocation fails.
 *
 * Index : vectors
 *
 * -----------------------------------------------------------------------------
 */

#ifdef TRACK_MEMORY_ALLOCATION

int debug_copy_vector
(
    Vector**      target_vpp,
    const Vector* source_vp,
    const char*   file_name,
    int           line_number
)
{
    IMPORT int kjb_use_memcpy;
    int   i;
    int   length;
    double* target_pos;
    double* source_pos;


    if (source_vp == NULL)
    {
        free_vector(*target_vpp);
        *target_vpp = NULL;
        return NO_ERROR;
    }

    length = source_vp->length;

    ERE(debug_get_target_vector(target_vpp, length, file_name, line_number));

    source_pos = source_vp->elements;
    target_pos = (*target_vpp)->elements;

    if (kjb_use_memcpy)
    {
        (void)memcpy(target_pos,  source_pos,
                     ((size_t)length) * sizeof(double));
    }
    else
    {
        for (i=0; i<length; i++)
        {
            *target_pos++ = *source_pos++;
        }
    }

    return NO_ERROR;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int copy_vector(Vector** target_vpp, const Vector* source_vp)
{
    IMPORT int kjb_use_memcpy;
    int   i;
    int   length;
    double* target_pos;
    double* source_pos;


    if (source_vp == NULL)
    {
        free_vector(*target_vpp);
        *target_vpp = NULL;
        return NO_ERROR;
    }

    length = source_vp->length;

    ERE(get_target_vector(target_vpp, length));

    source_pos = source_vp->elements;
    target_pos = (*target_vpp)->elements;

    if (kjb_use_memcpy)
    {
        (void)memcpy(target_pos,  source_pos,
                     ((size_t)length) * sizeof(double));
    }
    else
    {
        for (i=0; i<length; i++)
        {
            *target_pos++ = *source_pos++;
        }
    }

    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_copy_vector
 *
 * Copies a vector to an existing vector
 *
 * This routine copies the vector pointed to by source_vp to the vector pointer
 * to by target_vp, offset by "offset".
 *
 * Returns :
 *    NO_ERROR on success and ERROR on failure. This routine can only fail if
 *    the vector to be copied is too long, and this is treated as a bug.
 *
 * Index : vectors
 *
 * -----------------------------------------------------------------------------
 */

int ow_copy_vector
(
    Vector*       target_vp,
    int           offset,
    const Vector* source_vp
)
{
    int   i;
    int   length;
    double* target_pos;
    double* source_pos;

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

int concat_vectors(Vector** vpp, int num_vectors, const Vector** vectors)
{
    int i, length = 0;
    double* target_pos;

    UNTESTED_CODE();

    for (i = 0; i < num_vectors; i++)
    {
        if (vectors[ i ] != NULL)
        {
            length += vectors[ i ]->length;
        }
    }

    ERE(get_target_vector(vpp, length));
    target_pos = (*vpp)->elements;

    for (i = 0; i < num_vectors; i++)
    {
        if (vectors[ i ] != NULL)
        {
            const Vector* source_vp     = vectors[ i ];
            int           source_length = source_vp->length;
            double*         source_pos    = source_vp->elements;
            int           j;

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

int concat_vector_vectors
(
    Vector_vector**       vvpp,
    int                   num_vectors,
    const Vector_vector** vector_vectors
)
{
    int i, length = 0;
    Vector** target_pos;

    UNTESTED_CODE();

    for (i = 0; i < num_vectors; i++)
    {
        if (vector_vectors[ i ] != NULL)
        {
            length += vector_vectors[ i ]->length;
        }
    }

    ERE(get_target_vector_vector(vvpp, length));
    target_pos = (*vvpp)->elements;

    for (i = 0; i < num_vectors; i++)
    {
        const Vector_vector* source_vvp = vector_vectors[ i ];

        if (source_vvp != NULL)
        {
            int                  source_length = source_vvp->length;
            Vector**             source_pos    = source_vvp->elements;
            int                  j;

            for (j = 0; j < source_length; j++)
            {
                ERE(copy_vector(target_pos, *source_pos));
                target_pos++;
                source_pos++;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST
void debug_verify_vector
(
    const Vector* vp,
    Bool*         failed_ptr, 
    const char*   file_name,
    int           line_number
)
{
    int i, length;

    if (failed_ptr != NULL) { *failed_ptr = FALSE; }

    if (vp == NULL) return; 

    length = vp->length;

    for (i = 0; i < length; i++)
    {
        if (    (isnand(vp->elements[ i ]))
             || ( ! isfinited(vp->elements[ i ]))
           )
        {
            warn_pso("Vector verifcation failed.\n");
            warn_pso("Element %d is not ", i);

            if (isnand(vp->elements[ i ]))
            {
                warn_pso("a number.\n");
            }
            else
            {
                warn_pso("finite.\n");
            }

            warn_pso("Bits are %x, %x\n",
                     *((int*)&(vp->elements[ i ])),
                     *(1 + ((int*)&(vp->elements[ i ]))));
            warn_pso("Setting it to zero\n");
            vp->elements[ i ] = 0.0;
            warn_pso("Verification called from line %d of %s.\n",
                     line_number, file_name);

            if (failed_ptr != NULL) { *failed_ptr = TRUE; }
        }
    }
}

#endif
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             flatten_vector_vector
 *
 * Creates Vector from Vector_vector
 *
 * This routine concatenates vectors in source_vvp and puts them in target_vpp. 
 * If *target_vpp is NULL, it will be created; otherwise it will be resized or 
 * reused.
 *
 * Index: arrays, vectors
 *
 * Returns:
 *    On error, this routine ERROR, with an error message being set.
 *    On success it returns NO_ERROR.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
*/

int flatten_vector_vector(Vector** target_vpp, const Vector_vector* source_vvp)
{
    int size;
    int i;

    if(source_vvp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    size = 0;
    for(i = 0; i < source_vvp->length; i++)
    {
        size += (source_vvp->elements[i] != NULL) ? source_vvp->elements[i]->length : 0;
    }

    get_target_vector(target_vpp, size);
    size = 0;
    for(i = 0; i < source_vvp->length; i++)
    {
        if(source_vvp->elements[i] != NULL)
        {
            ow_copy_vector(*target_vpp, size, source_vvp->elements[i]);
            size += source_vvp->elements[i]->length;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             vector_vector_from_vector
 *
 * Creates Vector_vector from a Vector
 *
 * This routine separates source_vp into vectors of length size which it subsequently
 * puts in target_vvpp. source_vp->length must be a multiple of size.
 *
 * If *target_vpp is NULL, it will be created; otherwise it will be resized or 
 * reused.
 *
 * Index: arrays, vectors
 *
 * Returns:
 *    On error, this routine ERROR, with an error message being set.
 *    On success it returns NO_ERROR.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
*/

int vector_vector_from_vector(Vector_vector** target_vvpp, const Vector* source_vp, int size)
{
    int i = 0;
    int n;

    if(source_vp->length % size != 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }
    n = source_vp->length / size;

    get_target_vector_vector(target_vvpp, n);
    for(i = 0; i < n; i++)
    {
        copy_vector_segment(&(*target_vvpp)->elements[i], source_vp, i * size, size);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             vector_vector_from_matrix
 *
 * Creates Vector_vector from a Matrix
 *
 * This routine separates source_mp into vectors which it subsequently
 * puts in target_vvpp.
 *
 * If *target_vpp is NULL, it will be created; otherwise it will be resized or 
 * reused.
 *
 * Index: arrays, vectors, matrices
 *
 * Returns:
 *    On error, this routine ERROR, with an error message being set.
 *    On success it returns NO_ERROR.
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * -----------------------------------------------------------------------------
*/

int vector_vector_from_matrix
(
    Vector_vector** target_vvpp, /*Vector_vector to fill*/
    const Matrix* source_mp /*The Matrix to copy*/
)
{
    int i;

    ERE(get_target_vector_vector(target_vvpp,source_mp->num_rows));
    
    for (i=0; i<source_mp->num_rows; i++)
    {
        ERE(get_matrix_row(&((*target_vvpp)->elements[i]),source_mp,i));
    }

    return NO_ERROR;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */



/* =============================================================================
 *                             get_vector_vector_transpose
 *
 * Computes the "transpose" of a Vector_vector
 *
 * This routine computes the "transpose" of a the Vector_vector source_vvp, when
 * thought of as a matrix. For example, (*target_vvpp)->elements[0] will have
 * source_vvp->length elements, which will be
 *
 *      source_vvp->elements[0]->elements[0]
 *      source_vvp->elements[1]->elements[0]
 *      ...
 *      source_vvp->elements[N]->elements[0]
 *
 * where N = source_vvp->length. Clearly, all the vectors of source_vvp must have
 * the same length.
 *
 * If *target_vpp is NULL, it will be created; otherwise it will be resized or 
 * reused.
 *
 * Index: arrays, vectors
 *
 * Returns:
 *    On error, this routine ERROR, with an error message being set.
 *    On success it returns NO_ERROR.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
*/

int get_vector_vector_transpose(Vector_vector** target_vvpp, const Vector_vector* source_vvp)
{
    int n, k;
    int i, j;

    if(source_vvp == NULL || target_vvpp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    n = source_vvp->length;
    k = source_vvp->elements[0]->length;

    for(i = 1; i < n; i++)
    {
        if(source_vvp->elements[i]->length != k)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    get_target_vector_vector(target_vvpp, k);

    for(i = 0; i < k; i++)
    {
        get_target_vector(&(*target_vvpp)->elements[i], n);
        for(j = 0; j < n; j++)
        {
            (*target_vvpp)->elements[i]->elements[j] = source_vvp->elements[j]->elements[i];
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             is_element_in_vector
 *
 * Figures out if element is in vector.
 *
 * This routine searches the vector vp for element elem; if elem is there,
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

int is_element_in_vector(int* index, const Vector* vp, double elem)
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
            if(index != NULL)
            {
                *index = i;
            }
            return TRUE;
        }
    }

    if(index != NULL)
    {
        *index = -1;
    }

    return FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       is_vector_vector_consistent
 *
 * Checks if matrix vector is 'consistent'
 *
 * This routine checks whether all matrices in a matrix vector are the same size.
 *
 * Index: arrays, matrices
 *
 * Returns:
 *    TRUE if the all the matrices in mvp are the same size, FALSE otherwise.
 *
 * -----------------------------------------------------------------------------
*/

int is_vector_vector_consistent(const Vector_vector* vvp)
{
    int i;

    if(vvp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for(i = 1; i < vvp->length; i++)
    {
        if(vvp->elements[i]->length != vvp->elements[0]->length)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

