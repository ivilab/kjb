
/* $Id: m_vec_arith.c 21522 2017-07-22 15:14:27Z kobus $ */

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
#include "m/m_vec_arith.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef TEST
#ifdef PROGRAMMER_IS_kobus
#define REPORT_MISSING_VALUE_ARITHMETIC
#endif
#endif

/*
#define SLOW_VECTOR_DIVIDE
*/

/* -------------------------------------------------------------------------- */

/*
 * =============================================================================
 *                            multiply_vector_by_scalar
 *
 * Multiplies a vector by a scalar
 *
 * This routine multiplies the vector in "source_vp" by the scalar in "scalar",
 * and puts the result in the vector pointed to by "target_vpp". That vector
 * is created or resized as necessary.
 *
 * If we are respecting missing values, then the sqrt() of a missing value
 * (DBL_MISSING) is DBL_MISSING.
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
 *    ow_multiply_vector_by_scalar
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int multiply_vector_by_scalar
(
    Vector**      target_vpp,
    const Vector* source_vp,
    double        scalar
)
{
    Vector* target_vp;
    int     i;
    int     length;
    double*   source_pos;
    double*   target_pos;


    ERE(get_target_vector(target_vpp, source_vp->length));
    target_vp = *target_vpp;

    source_pos = source_vp->elements;
    target_pos = target_vp->elements;

    length = source_vp->length;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        if (IS_MISSING_DBL(scalar))
        {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
            dbm("Missing value arithmetic.");
#endif
            return get_initialized_vector(target_vpp, length, DBL_MISSING);
        }

        for (i=0; i<length; i++)
        {
            if (IS_MISSING_DBL(*source_pos))
            {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                dbm("Missing value arithmetic.");
#endif
                *target_pos = DBL_MISSING;
            }
            else
            {
                (*target_pos) = (*source_pos) * scalar;
            }

            source_pos++;
            target_pos++;
        }
    }
    else
    {
        for (i=0; i<length; i++)
        {
            (*target_pos) = (*source_pos) * scalar;

            source_pos++;
            target_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            ow_multiply_vector_by_scalar
 *
 * Multiplies a vector by a scalar
 *
 * This routine multiplies the vector in "source_vp" by the scalar in "scalar",
 * overwriting the contents of source_vp.
 *
 * Returns :
 *    NO_ERROR on success, and ERROR on failure with the error message being
 *    set. Currently, there is no way to fail (gracefully, that is).
 *
 * Related:
 *    multiply_vector_by_scalar
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_multiply_vector_by_scalar(Vector* source_vp, double scalar)
{
    int     i;
    int     length;
    double*   source_pos;


    source_pos = source_vp->elements;
    length = source_vp->length;

#ifdef HOW_IT_WAS
    for (i=0; i<length; i++)
    {
        *source_pos++ *= scalar;
    }
#else
#endif
    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        if (IS_MISSING_DBL(scalar))
        {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
            dbm("Missing value arithmetic.");
#endif
            return ow_set_vector(source_vp, DBL_MISSING);
        }

        for (i=0; i<length; i++)
        {
            if (IS_MISSING_DBL(*source_pos))
            {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                dbm("Missing value arithmetic.");
#endif
                *source_pos = DBL_MISSING;
            }
            else
            {
                *source_pos *= scalar;
            }
            source_pos++;
        }
    }
    else
    {
        for (i=0; i<length; i++)
        {
            *source_pos *= scalar;
            source_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            divide_vector_by_scalar
 *
 * Divides a vector by a scalar
 *
 * This routine divides the vector in "source_vp" by the scalar in "scalar",
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
 *    set. The routine will fail if the scalar is too close to zero, or if there
 *    is a problem allocating memory.
 *
 * Related:
 *    ow_divide_vector_by_scalar
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int divide_vector_by_scalar
(
    Vector**      target_vpp,
    const Vector* source_vp,
    double        scalar
)
{
#ifdef SLOW_VECTOR_DIVIDE  /* For regession testing. */
    Vector* target_vp;
    int     i;
    int     length;
    double*   source_pos;
    double*   target_pos;


    if (respect_missing_values())
    {
        set_bug("The regression version does not support respecting missing values.");
        return ERROR;
    }

#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
    if (IS_ZERO_DBL(scalar))
    {
        SET_DIVIDE_BY_ZERO_ERROR();
        NOTE_ERROR(); 
        return ERROR;
    }
#else
    /*
    // Exact compare is OK because overflow is handled separately.
    */
    if (scalar == 0.0)
    {
        SET_DIVIDE_BY_ZERO_ERROR();
        NOTE_ERROR(); 
        return ERROR;
    }
#endif

#ifdef CHECK_VECTOR_INITIALZATION_ON_FREE
    /* Prepare for divide by zero. */
    ERE(get_zero_vector(target_vpp, source_vp->length));
#else
    ERE(get_target_vector(target_vpp, source_vp->length));
#endif

    target_vp = *target_vpp;

    source_pos = source_vp->elements;
    target_pos = target_vp->elements;

    length = source_vp->length;

    for (i=0; i<length; i++)
    {
        (*target_pos) = (*source_pos) / scalar;

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
        if (    (*target_pos > DBL_MOST_POSITIVE)
             || (*target_pos < DBL_MOST_NEGATIVE)
           )
        {
            SET_OVERFLOW_ERROR();
            return ERROR;
        }
#endif
        source_pos++;
        target_pos++;
    }

    return NO_ERROR;

#else

    if (ABS_OF(scalar) < 10.0 * DBL_MIN)
    {
        SET_DIVIDE_BY_ZERO_ERROR();
        NOTE_ERROR(); 
        return ERROR;
    }
    else if (respect_missing_values() && (IS_MISSING_DBL(scalar)))
    {
        int length = source_vp->length;

        UNTESTED_CODE();

#ifdef REPORT_MISSING_VALUE_ARITHMETIC
        dbm("Missing value arithmetic.");
#endif
        return get_initialized_vector(target_vpp, length, DBL_MISSING);
    }
    else
    {
        return multiply_vector_by_scalar(target_vpp, source_vp, 1.0 / scalar);
    }

#endif

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_divide_vector_by_scalar
 *
 * Divides a vector by a scalar
 *
 * This routine divides the vector in "source_vp" by the scalar in "scalar",
 * overwriting the contents of source_vp.
 *
 * Returns :
 *    NO_ERROR on success, and ERROR on failure with the error message being
 *    set. The routine will fail if the scalar is too close to zero.
 *
 * Related:
 *    divide_vector_by_scalar
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_divide_vector_by_scalar(Vector* source_vp, double scalar)
{
#ifdef SLOW_VECTOR_DIVIDE  /* For regession testing. */
    int     i;
    int     length;
    double*   source_pos;


#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
    if (IS_ZERO_DBL(scalar))
    {
        SET_DIVIDE_BY_ZERO_ERROR();
        NOTE_ERROR(); 
        return ERROR;
    }
#else
    /*
    // Exact compare is OK because overflow is handled separately.
    */
    if (scalar == 0.0)
    {
        SET_DIVIDE_BY_ZERO_ERROR();
        NOTE_ERROR(); 
        return ERROR;
    }
#endif

    if (respect_missing_values())
    {
        set_bug("The regressing version does not support respecting missing values.");
        return ERROR;
    }

    source_pos = source_vp->elements;
    length = source_vp->length;

    for (i=0; i<length; i++)
    {
        *source_pos /= scalar;

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
        if (    (*source_pos > DBL_MOST_POSITIVE)
             || (*source_pos < DBL_MOST_NEGATIVE)
           )
        {
            SET_OVERFLOW_ERROR();
            return ERROR;
        }
#endif
        source_pos++;
    }

    return NO_ERROR;

#else

    if (ABS_OF(scalar) < 10.0 * DBL_MIN)
    {
        SET_DIVIDE_BY_ZERO_ERROR();
        NOTE_ERROR(); 
        return ERROR;
    }
    else if (respect_missing_values() && (IS_MISSING_DBL(scalar)))
    {
        UNTESTED_CODE();

#ifdef REPORT_MISSING_VALUE_ARITHMETIC
        dbm("Missing value arithmetic.");
#endif
        return ow_set_vector(source_vp, DBL_MISSING);
    }
    else
    {
        return ow_multiply_vector_by_scalar(source_vp, 1.0 / scalar);
    }

#endif

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            add_scalar_to_vector
 *
 * Adds a scalar to a vector
 *
 * This routine adds the the scalar in "scalar" to the vector in "source_vp",
 * and puts the result in the vector pointed to by "target_vpp". That vector
 * is created or resized as necessary.
 *
 * If we are respecting missing values, then the sqrt() of a missing value
 * (DBL_MISSING) is DBL_MISSING.
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
 *    ow_add_scalar_to_vector
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int add_scalar_to_vector
(
    Vector**      target_vpp,
    const Vector* source_vp,
    double        scalar
)
{
    Vector* target_vp;
    int     i;
    int     length;
    double*   source_pos;
    double*   target_pos;


    ERE(get_target_vector(target_vpp, source_vp->length));
    target_vp = *target_vpp;

    source_pos = source_vp->elements;
    target_pos = target_vp->elements;

    length = source_vp->length;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        if (IS_MISSING_DBL(scalar))
        {
            UNTESTED_CODE();

#ifdef REPORT_MISSING_VALUE_ARITHMETIC
            dbm("Missing value arithmetic.");
#endif
            return get_initialized_vector(target_vpp, length, DBL_MISSING);
        }

        for (i=0; i<length; i++)
        {
            if (IS_MISSING_DBL(*source_pos))
            {
                UNTESTED_CODE();

#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                dbm("Missing value arithmetic.");
#endif
                *target_pos = DBL_MISSING;
            }
            else
            {
                (*target_pos) = (*source_pos) + scalar;
            }

            source_pos++;
            target_pos++;
        }
    }
    else
    {
        for (i=0; i<length; i++)
        {
            (*target_pos) = (*source_pos) + scalar;

            source_pos++;
            target_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            ow_add_scalar_to_vector
 *
 * Adds a scalar to a vector
 *
 * This routine adds  the scalar in "scalar" to the vector in "source_vp",
 * overwriting the contents of source_vp.
 *
 * If we are respecting missing values, then the sqrt() of a missing value
 * (DBL_MISSING) is DBL_MISSING.
 *
 * Returns :
 *    NO_ERROR on success, and ERROR on failure with the error message being
 *    set. Currently, there is no way to fail (gracefully, that is).
 *
 * Related:
 *    add_scalar_to_vector
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_scalar_to_vector(Vector* source_vp, double scalar)
{
    int     i;
    int     length;
    double*   source_pos;


    source_pos = source_vp->elements;
    length = source_vp->length;

#ifdef HOW_IT_WAS
    for (i=0; i<length; i++)
    {
        *source_pos++ += scalar;
    }
#else
#endif

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        if (IS_MISSING_DBL(scalar))
        {
            UNTESTED_CODE(); /* Since rework for missing. */

#ifdef REPORT_MISSING_VALUE_ARITHMETIC
            dbm("Missing value arithmetic.");
#endif
            return ow_set_vector(source_vp, DBL_MISSING);
        }

        for (i=0; i<length; i++)
        {
            if (IS_MISSING_DBL(*source_pos))
            {
                UNTESTED_CODE(); /* Since rework for missing. */

#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                dbm("Missing value arithmetic.");
#endif
                *source_pos = DBL_MISSING;
            }
            else
            {
                *source_pos += scalar;
            }
            source_pos++;
        }
    }
    else
    {
        for (i=0; i<length; i++)
        {
            *source_pos += scalar;
            source_pos++;
        }
    }


    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            subtract_scalar_from_vector
 *
 * Subtracs a scalar from a vector
 *
 * This routine subtracts the the scalar in "scalar" from the vector in
 * "source_vp", and puts the result in the vector pointed to by "target_vpp".
 * That vector is created or resized as necessary.
 *
 * If we are respecting missing values, then the sqrt() of a missing value
 * (DBL_MISSING) is DBL_MISSING.
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
 *    ow_subtract_scalar_from_vector
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int subtract_scalar_from_vector
(
    Vector**      target_vpp,
    const Vector* source_vp,
    double        scalar
)
{
    Vector* target_vp;
    int     i;
    int     length;
    double*   source_pos;
    double*   target_pos;


    ERE(get_target_vector(target_vpp, source_vp->length));
    target_vp = *target_vpp;

    source_pos = source_vp->elements;
    target_pos = target_vp->elements;

    length = source_vp->length;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        if (IS_MISSING_DBL(scalar))
        {
            UNTESTED_CODE();

#ifdef REPORT_MISSING_VALUE_ARITHMETIC
            dbm("Missing value arithmetic.");
#endif
            return get_initialized_vector(target_vpp, length, DBL_MISSING);
        }

        for (i=0; i<length; i++)
        {
            if (IS_MISSING_DBL(*source_pos))
            {
                UNTESTED_CODE();

#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                dbm("Missing value arithmetic.");
#endif
                *target_pos = DBL_MISSING;
            }
            else
            {
                (*target_pos) = (*source_pos) - scalar;
            }

            source_pos++;
            target_pos++;
        }
    }
    else
    {
        for (i=0; i<length; i++)
        {
            (*target_pos) = (*source_pos) - scalar;

            source_pos++;
            target_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            ow_subtract_scalar_from_vector
 *
 * Subtracs a scalar from a vector
 *
 * This routine subtracts the scalar in "scalar" from the vector in "source_vp",
 * overwriting the contents of source_vp.
 *
 * If we are respecting missing values, then the sqrt() of a missing value
 * (DBL_MISSING) is DBL_MISSING.
 *
 * Returns :
 *    NO_ERROR on success, and ERROR on failure with the error message being
 *    set. Currently, there is no way to fail (gracefully, that is).
 *
 * Related:
 *    subtract_scalar_from_vector
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_subtract_scalar_from_vector(Vector* source_vp, double scalar)
{
    int     i;
    int     length;
    double*   source_pos;


    source_pos = source_vp->elements;
    length = source_vp->length;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        if (IS_MISSING_DBL(scalar))
        {
            UNTESTED_CODE();

#ifdef REPORT_MISSING_VALUE_ARITHMETIC
            dbm("Missing value arithmetic.");
#endif
            return ow_set_vector(source_vp, DBL_MISSING);
        }

        for (i=0; i<length; i++)
        {
            if (IS_MISSING_DBL(*source_pos))
            {
                UNTESTED_CODE();

#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                dbm("Missing value arithmetic.");
#endif
                *source_pos = DBL_MISSING;
            }
            else
            {
                *source_pos -= scalar;
            }
            source_pos++;
        }
    }
    else
    {
        for (i=0; i<length; i++)
        {
            *source_pos -= scalar;
            source_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              add_vectors
 *
 * Performs element-wise addition of two vectors.
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

int add_vectors
(
    Vector**      target_vpp,
    const Vector* first_vp,
    const Vector* second_vp
)
{
    double* target_elem_ptr;
    double* first_elem_ptr;
    double* second_elem_ptr;
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

    ERE(get_target_vector(target_vpp, length));
    target_elem_ptr = (*target_vpp)->elements;
    first_elem_ptr = first_vp->elements;

    for (block=0; block<factor; block++)
    {
        second_elem_ptr = second_vp->elements;

        /*
         * Duplicate code for speed.
        */
        if (respect_missing_values())
        {
            for (i=0; i<second_length; i++)
            {
                if (    (IS_MISSING_DBL(*first_elem_ptr))
                     || (IS_MISSING_DBL(*second_elem_ptr))
                   )
                {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                    dbm("MISSING VALUE ARITHMETIC.");
#endif
                    *target_elem_ptr = DBL_MISSING;
                }
                else
                {
                    *target_elem_ptr = (*first_elem_ptr) + (*second_elem_ptr);
                }

                target_elem_ptr++;
                first_elem_ptr++;
                second_elem_ptr++;
            }
        }
        else
        {
            for (i=0; i<second_length; i++)
            {
                *target_elem_ptr = (*first_elem_ptr) + (*second_elem_ptr);

                target_elem_ptr++;
                first_elem_ptr++;
                second_elem_ptr++;
            }
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              subtract_vectors
 *
 * Performs element-wise subtraction of two vectors.
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

int subtract_vectors
(
    Vector**      target_vpp,
    const Vector* first_vp,
    const Vector* second_vp
)
{
    double* target_elem_ptr;
    double* first_elem_ptr;
    double* second_elem_ptr;
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

    ERE(get_target_vector(target_vpp, length));
    target_elem_ptr = (*target_vpp)->elements;
    first_elem_ptr = first_vp->elements;

    for (block=0; block<factor; block++)
    {
        second_elem_ptr = second_vp->elements;

        /*
         * Duplicate code for speed.
        */
        if (respect_missing_values())
        {
            for (i=0; i<second_length; i++)
            {
                if (    (IS_MISSING_DBL(*first_elem_ptr))
                     || (IS_MISSING_DBL(*second_elem_ptr))
                   )
                {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                    dbm("Missing value arithmetic.");
#endif
                    *target_elem_ptr = DBL_MISSING;
                }
                else
                {
                    *target_elem_ptr = (*first_elem_ptr) - (*second_elem_ptr);
                }

                target_elem_ptr++;
                first_elem_ptr++;
                second_elem_ptr++;
            }
        }
        else
        {
            for (i=0; i<second_length; i++)
            {
                *target_elem_ptr = (*first_elem_ptr) - (*second_elem_ptr);

                target_elem_ptr++;
                first_elem_ptr++;
                second_elem_ptr++;
            }
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              multiply_vectors
 *
 * Performs element-wise multiplication of two vectors.
 *
 * This routine performs element-wise multiplication of two vectors. The second
 * vector must fit into the first an integral number of times.The second
 * vector is added to each block in the first vector.
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

int multiply_vectors
(
    Vector**      target_vpp,
    const Vector* first_vp,
    const Vector* second_vp
)
{
    double* target_elem_ptr;
    double* first_elem_ptr;
    double* second_elem_ptr;
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
        set_bug("A vector of length %d can't multiply one of length %d.",
                second_length, length);
        return ERROR;
    }

    ERE(get_target_vector(target_vpp, length));
    target_elem_ptr = (*target_vpp)->elements;
    first_elem_ptr = first_vp->elements;

    for (block=0; block<factor; block++)
    {
        second_elem_ptr = second_vp->elements;

        /*
         * Duplicate code for speed.
        */
        if (respect_missing_values())
        {
            for (i=0; i<second_length; i++)
            {
                if (    (IS_MISSING_DBL(*first_elem_ptr))
                     || (IS_MISSING_DBL(*second_elem_ptr))
                   )
                {
                    UNTESTED_CODE();

#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                    dbm("Missing value arithmetic.");
#endif
                    *target_elem_ptr = DBL_MISSING;
                }
                else
                {
                    *target_elem_ptr = (*first_elem_ptr) * (*second_elem_ptr);
                }

                target_elem_ptr++;
                first_elem_ptr++;
                second_elem_ptr++;
            }
        }
        else
        {
            for (i=0; i<second_length; i++)
            {
                *target_elem_ptr = (*first_elem_ptr) * (*second_elem_ptr);

                target_elem_ptr++;
                first_elem_ptr++;
                second_elem_ptr++;
            }
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              divide_vectors
 *
 * Performs element-wise division of two vectors.
 *
 * This routine performs element-wise division of two vectors. The second
 * vector must fit into the first an integral number of times.The second
 * vector is added to each block in the first vector.
 *
 * The first argument is the adress of the target vector. If the target vector
 * itself is null, then a vector of the appropriate size is created. If the
 * target vector is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine will fail if any of the matrix elements are too close
 *     to zero, or if memory allocation fails. The routine will also fail due to
 *     dimension mismatch, but this is currently treated as a bug (see
 *     set_bug()).
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int divide_vectors
(
    Vector**      target_vpp,
    const Vector* first_vp,
    const Vector* second_vp
)
{
    double* target_elem_ptr;
    double* first_elem_ptr;
    double* second_elem_ptr;
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
        set_bug("A vector of length %d can't divide one of length %d.",
                second_length, length);
        return ERROR;
    }


#ifdef CHECK_VECTOR_INITIALZATION_ON_FREE
    /* Prepare for divide by zero. */
    ERE(get_zero_vector(target_vpp, length));
#else
    ERE(get_target_vector(target_vpp, length));
#endif

    target_elem_ptr = (*target_vpp)->elements;
    first_elem_ptr = first_vp->elements;

    for (block=0; block<factor; block++)
    {
        second_elem_ptr = second_vp->elements;

        for (i=0; i<second_length; i++)
        {
#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
            if (IS_ZERO_DBL(*second_elem_ptr))
            {
                SET_DIVIDE_BY_ZERO_ERROR();
                NOTE_ERROR(); 
                return ERROR;
            }
#else
            /*
            // Exact compare is OK because overflow is handled separately.
            */
            if (*second_elem_ptr == 0.0)
            {
                SET_DIVIDE_BY_ZERO_ERROR();
                NOTE_ERROR(); 
                return ERROR;
            }
#endif

            if (    (respect_missing_values() )
                 && (    (IS_MISSING_DBL(*first_elem_ptr))
                      || (IS_MISSING_DBL(*second_elem_ptr))
                    )
               )
            {
                UNTESTED_CODE();
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                dbm("Missing value arithmetic.");
                dbe(*first_elem_ptr);
                dbe(*second_elem_ptr); 
#endif
                *target_elem_ptr = DBL_MISSING;
            }
            else
            {
                *target_elem_ptr = (*first_elem_ptr) / (*second_elem_ptr);
            }

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
            if (    (*target_elem_ptr > DBL_MOST_POSITIVE)
                 || (*target_elem_ptr < DBL_MOST_NEGATIVE)
               )
            {
                SET_OVERFLOW_ERROR();
                return ERROR;
            }
#endif
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
 *                   ow_add_scalar_times_vector_to_vector
 *
 * Adds a scalar times the second vector to the first.
 *
 * This routine adds a scalar times the second vector to the first. The second
 * vector must fit into the first an integral number of times. The scalar times
 * the second vector is added to each block in the first vector.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_scalar_times_vector_to_vector
(
    Vector*       first_vp,
    const Vector* second_vp,
    double        scalar
)
{
    double* first_elem_ptr;
    double* second_elem_ptr;
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
            *first_elem_ptr += scalar * (*second_elem_ptr);

            first_elem_ptr++;
            second_elem_ptr++;
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_add_vectors
 *
 * Adds the second vector to the first.
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

int ow_add_vectors(Vector* first_vp, const Vector* second_vp)
{
    double* first_elem_ptr;
    double* second_elem_ptr;
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
 *                              ow_add_vector_times_scalar
 *
 * Adds a scalar times the second vector to the first.
 *
 * This routine adds scalar times the second vector to the first. The second
 * vector must fit into the first an integral number of times. The second vector
 * is added to each block in the first vector.
 *
 * Note: 
 *     While it looks very specific, this operation is surprisingly common, and
 *     thus it makes sense to do both the multiply and add in the same
 *     loop.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_vector_times_scalar(Vector* first_vp, const Vector* second_vp, double scalar)
{
    double* first_elem_ptr;
    double* second_elem_ptr;
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
            *first_elem_ptr += (scalar * (*second_elem_ptr));

            first_elem_ptr++;
            second_elem_ptr++;
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_subtract_vectors
 *
 * Subtracts the second vector from the first.
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

int ow_subtract_vectors(Vector* first_vp, const Vector* second_vp)
{
    double* first_elem_ptr;
    double* second_elem_ptr;
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

/*
 * =============================================================================
 *                           ow_subtract_vector_times_scalar
 *
 * Subtracts a scalar times the second vector from the first.
 *
 * This routine subtracts a scalar times the second vector from the first. The
 * second vector must fit into the first an integral number of times. The second
 * vector is added to each block in the first vector.
 *
 * Note: 
 *     While it looks very specific, this operation is surprisingly common, and
 *     thus it makes sense to do both the multiply and subtract in the same
 *     loop.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_subtract_vector_times_scalar(Vector* first_vp, const Vector* second_vp, double scalar)
{
    double* first_elem_ptr;
    double* second_elem_ptr;
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
            *first_elem_ptr -= (scalar * (*second_elem_ptr));

            first_elem_ptr++;
            second_elem_ptr++;
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_multiply_vectors
 *
 * Multiplies the first vector by the second.
 *
 * This routine multiplies the first vector by the second. The second vector
 * must fit into the first an integral number of times. The second vector is
 * multiplies each block in the first vector elementwise.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Note: 
 *     This should probably be called ow_multiply_vectors_ew(). 
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_multiply_vectors(Vector* first_vp, const Vector* second_vp)
{
    double* first_elem_ptr;
    double* second_elem_ptr;
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
        set_bug("A vector of length %d can't multiply one of length %d.",
                length, second_length);
        return ERROR;
    }

    first_elem_ptr = first_vp->elements;

    for (block=0; block<factor; block++)
    {
        second_elem_ptr = second_vp->elements;

        for (i=0; i<second_length; i++)
        {
            *first_elem_ptr *= (*second_elem_ptr);

            first_elem_ptr++;
            second_elem_ptr++;
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_divide_vectors
 *
 * Divides the first vector by the second.
 *
 * This routine divides the first vector by the second. The second vector
 * must fit into the first an integral number of times. The second vector is
 * added to each block in the first vector.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine will fail if any of the matrix elements are too close
 *     to zero.  This routine will fail if there is a dimension mismatch, but
 *     this is currently treated as a bug (see set_bug()).
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_divide_vectors(Vector* first_vp, const Vector* second_vp)
{
    double* first_elem_ptr;
    double* second_elem_ptr;
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
        set_bug("A vector of length %d can't divide one of length %d.",
                second_length, length);
        return ERROR;
    }

    first_elem_ptr = first_vp->elements;

    for (block=0; block<factor; block++)
    {
        second_elem_ptr = second_vp->elements;

        for (i=0; i<second_length; i++)
        {
#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
            if (IS_ZERO_DBL(*second_elem_ptr))
            {
                SET_DIVIDE_BY_ZERO_ERROR();
                NOTE_ERROR(); 
                return ERROR;
            }
#else
            /*
            // Exact compare is OK because overflow is handled separately.
            */
            if (*second_elem_ptr == 0.0)
            {
                SET_DIVIDE_BY_ZERO_ERROR();
                NOTE_ERROR(); 
                return ERROR;
            }
#endif

            *first_elem_ptr /= (*second_elem_ptr);

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
            if (    (*first_elem_ptr > DBL_MOST_POSITIVE)
                 || (*first_elem_ptr < DBL_MOST_NEGATIVE)
               )
            {
                SET_OVERFLOW_ERROR();
                return ERROR;
            }
#endif
            first_elem_ptr++;
            second_elem_ptr++;
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              invert_vector
 *
 * Calculates the element-wise reciprocal of a vector.
 *
 * This routine calculates the element-wise reciprocal of a vector.
 *
 * The first argument is the adress of the target vector. If the target vector
 * itself is null, then a vector of the appropriate size is created. If the
 * target vector is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine will fail if any of the matrix elements are too close
 *     to zero, or if memory allocation fails.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int invert_vector(Vector** target_vpp, const Vector* source_vp)
{
    Vector* target_vp;
    int i;

    verify_vector(source_vp, NULL);

#ifdef CHECK_VECTOR_INITIALZATION_ON_FREE
    /* Prepare for divide by zero. */
    ERE(get_zero_vector(target_vpp, source_vp->length));
#else
    ERE(get_target_vector(target_vpp, source_vp->length));
#endif

    target_vp = *target_vpp;

    for (i=0; i<source_vp->length; i++)
    {
        double val = (source_vp->elements)[i];

#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
        if ( IS_ZERO_DBL(val)
        {
            SET_DIVIDE_BY_ZERO_ERROR();
            add_error("Error occured with vector element %d.", i); 
            NOTE_ERROR(); 
            return ERROR;
        }
#else
        /*
        // Exact compare is OK because overflow is handled separately.
        */
        if (val  == 0.0)
        {
            SET_DIVIDE_BY_ZERO_ERROR();
            add_error("Error occured with vector element %d.", i); 
            NOTE_ERROR(); 
            return ERROR;
        }
#endif
        if ((respect_missing_values()) && (IS_MISSING_DBL(val)))
        {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
            dbm("Missing value arithmetic.");
#endif
            target_vp->elements[i] = DBL_MISSING;
        }
        else
        {
            target_vp->elements[i] = 1.0 / source_vp->elements[i];
        }

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
        if (    (target_vp->elements[i] > DBL_MOST_POSITIVE)
             || (target_vp->elements[i] < DBL_MOST_NEGATIVE)
           )
        {
            SET_OVERFLOW_ERROR();
            return ERROR;
        }
#endif
    }

   return NO_ERROR;
  }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_invert_vector
 *
 * Calculates the element-wise reciprocal of a vector.
 *
 * This routine calculates the element-wise reciprocal of a vector, overwiting
 * the input vector with the result.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. THis routine will fail if any of the element of the input vector is
 *     too close to zero.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_invert_vector(Vector* vp)
{
    int i;

    verify_vector(vp, NULL);

    for (i=0; i<vp->length; i++)
    {
#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
        if ( IS_ZERO_DBL((vp->elements)[i] ))
        {
            SET_DIVIDE_BY_ZERO_ERROR();
            add_error("Error occured with vector element %d.", i); 
            NOTE_ERROR(); 
            return ERROR;
        }
#else
        /*
        // Exact compare is OK because overflow is handled separately.
        */
        if ( (vp->elements)[i]  == 0.0)
        {
            SET_DIVIDE_BY_ZERO_ERROR();
            add_error("Error occured with vector element %d.", i); 
            NOTE_ERROR(); 
            return ERROR;
        }
#endif

        vp->elements[ i ] = 1.0 / vp->elements[ i ];

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
        if (    (vp->elements[ i ] > DBL_MOST_POSITIVE)
             || (vp->elements[ i ] < DBL_MOST_NEGATIVE)
           )
        {
            SET_OVERFLOW_ERROR();
            return ERROR;
        }
#endif
    }

   return NO_ERROR;
  }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              sqrt_vector
 *
 * Calculates the element-wise sqrt of a vector.
 *
 * This routine calculates the element-wise square root of a vector with the
 * KJB library creation semantics.
 *
 * If we are respecting missing values, then the sqrt() of a missing value
 * (DBL_MISSING) is DBL_MISSING.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine will fail if any of the element of the input vector is
 *     negative.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int sqrt_vector(Vector** target_vpp, const Vector* vp)
{

    verify_vector(vp, NULL);

    ERE(copy_vector(target_vpp, vp));
    ERE(ow_sqrt_vector(*target_vpp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_sqrt_vector
 *
 * Calculates the element-wise sqrt of a vector.
 *
 * This routine calculates the element-wise square root of a vector, overwiting
 * the input vector with the result.
 *
 * If we are respecting missing values, then the sqrt() of a missing value
 * (DBL_MISSING) is DBL_MISSING.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. THis routine will fail if any of the element of the input vector is
 *     negative.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_sqrt_vector(Vector* vp)
{
    int i;

    verify_vector(vp, NULL);

    for (i=0; i<vp->length; i++)
    {
        double val = vp->elements[i];

        if (IS_ZERO_DBL(val))
        {
            vp->elements[i] = 0.0;
        }
        else if (    (IS_MISSING_DBL(val))
                  && (respect_missing_values())
                )
        {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
             dbm("Missing value arithmetic.");
#endif
             vp->elements[ i ] = DBL_MISSING;
        }
        else if (val < 0.0)
        {
            set_error("Attempt to take the square root of ");
            cat_error("a negative vector element.");
            add_error("Element %d of a vector of length %d is %.3e.\n",
                      i + 1, vp->length, vp->elements[ i ]);
            return ERROR;
        }
        else
        {
            vp->elements[ i ] = sqrt(val);
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              log_vector
 *
 * Calculates the element-wise log of a vector.
 *
 * This routine calculates the element-wise log of a vector with the KJB library
 * creation semantics. If an element is zero, then LOG_ZERO is used.  LOG_ZERO
 * is suffiently negative number that exp(LOG_ZERO) is zero.  If an element is
 * negative, then this routine fails.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine will fail if any of the element of the input vector is
 *     negative.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */
int log_vector(Vector** target_vpp, const Vector* vp)
{

    return log_vector_2(target_vpp, vp, LOG_ZERO);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              log_vector_2
 *
 * Calculates the element-wise log of a vector.
 *
 * This routine calculates the element-wise log of a vector with the KJB library
 * creation semantics. If an element is zero, then the value of the parameter
 * log_zero is used for the log. If an element is negative, then this routine
 * fails.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine will fail if any of the element of the input vector is
 *     negative.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int log_vector_2(Vector** target_vpp, const Vector* vp, double log_zero)
{

    verify_vector(vp, NULL);

    ERE(copy_vector(target_vpp, vp));
    ERE(ow_log_vector_2(*target_vpp, log_zero));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_log_vector
 *
 * Calculates the element-wise log of a vector.
 *
 * This routine calculates the element-wise log of a vector, overwiting the
 * input vector with the result.  If an element is zero, then LOG_ZERO is used.
 * LOG_ZERO is suffiently negative number that exp(LOG_ZERO) is zero.  If an
 * element is negative, then this routine fails.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. THis routine will fail if any of the element of the input vector are
 *     negative
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_log_vector(Vector* vp)
{
    return ow_log_vector_2(vp, LOG_ZERO);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_log_vector_2
 *
 * Calculates the element-wise log of a vector.
 *
 * This routine calculates the element-wise log of a vector, overwiting the
 * input vector with the result.  If an element is zero, then the value of the
 * parameter log_zero is used for the log. If an element is negative, then this
 * routine fails.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. THis routine will fail if any of the element of the input vector are
 *     negative
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_log_vector_2(Vector* vp, double log_zero)
{
    int i;

    verify_vector(vp, NULL);

    for (i=0; i<vp->length; i++)
    {
        if (vp->elements[ i ] < 0.0)
        {
            set_error("Attempt to take the log of ");
            cat_error("a negative vector element.");
            add_error("Element %d of a vector of length %d is %.3e.\n",
                      i + 1, vp->length, vp->elements[ i ]);
            return ERROR;
        }
        else if (vp->elements[i] == 0.0)
        {
            vp->elements[i] = log_zero;
        }
        else
        {
            vp->elements[ i ] = log(vp->elements[ i ]);
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              exp_vector
 *
 * Calculates the element-wise exp of a vector.
 *
 * This routine calculates the element-wise exp() of a vector with the KJB
 * library creation semantics.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int exp_vector(Vector** target_vpp, const Vector* vp)
{

    verify_vector(vp, NULL);

    ERE(copy_vector(target_vpp, vp));
    ERE(ow_exp_vector(*target_vpp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_exp_vector
 *
 * Calculates the element-wise exp of a vector.
 *
 * This routine calculates the element-wise exp() of a vector, overwiting
 * the input vector with the result.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_exp_vector(Vector* vp)
{
    int i;

    verify_vector(vp, NULL);

    for (i=0; i<vp->length; i++)
    {
        vp->elements[ i ] = exp(vp->elements[ i ]);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              get_abs_of_vector
 *
 * Calculates the absolute values of vector componants.
 *
 * This routine calculates the absolute values of vector componants.
 *
 * The first argument is the adress of the target vector. If the target vector
 * itself is null, then a vector of the appropriate size is created. If the
 * target vector is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine will fail if any of the matrix elements are too close
 *     to zero, or if memory allocation fails.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int get_abs_of_vector(Vector** target_vpp, const Vector* source_vp)
{
    Vector* target_vp;
    int i;


    ERE(get_target_vector(target_vpp, source_vp->length));
    target_vp = *target_vpp;

    for (i=0; i<source_vp->length; i++)
    {
        target_vp->elements[i] = ABS_OF(source_vp->elements[i]);
    }

   return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_get_abs_of_vector
 *
 * Calculates the element-wise absolute value of a vector.
 *
 * This routine calculates the element-wise absolute value of a vector,
 * overwiting the input vector with the result.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. THis routine will fail if any of the element of the input vector is
 *     too close to zero.
 *
 * Index: vectors, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_get_abs_of_vector(Vector* vp)
{
    int i;


    for (i=0; i<vp->length; i++)
    {
        vp->elements[ i ] = ABS_OF(vp->elements[ i ]);
    }

   return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  average_vector_elements
 *
 * Returns the average of the elements of a vector
 *
 * This routine calculates the average of the elements of a vector. NULL
 * vectors, or vectors with zero length, have average 0. (This may change as
 * more sophisticated handling of missing values is implemented). 
 *
 * Returns :
 *     The sum of the elements of a vector.
 *
 * Index : vectors
 *
 * -----------------------------------------------------------------------------
 */

double average_vector_elements(const Vector* vp)
{

    if ((vp == NULL) || (vp->length <= 0))
    {
        return 0.0;
    }

    return sum_vector_elements(vp) / vp->length;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             average_vector_squared_elements
 *
 * Returns the average of the elements squared of a vector
 *
 * This routine calculates the average of the elements squared of a vector. NULL
 * vectors, or vectors with zero length, have average 0. (This may change as
 * more sophisticated handling of missing values is implemented). 
 *
 * Returns :
 *     The sum of the elements of a vector.
 *
 * Index : vectors
 *
 * -----------------------------------------------------------------------------
 */

double average_vector_squared_elements(const Vector* vp)
{

    
    if ((vp == NULL) || (vp->length <= 0))
    {
        return 0.0;
    }

    return sum_vector_squared_elements(vp) / vp->length;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  sum_vector_elements
 *
 * Returns the sum of the elements of a vector
 *
 * This routine calculates the sum of the elements of a vector.
 *
 * Returns :
 *     The sum of the elements of a vector.
 *
 * Index : vectors
 *
 * -----------------------------------------------------------------------------
 */

double sum_vector_elements(const Vector* vp)
{
    int   i;
    double  sum = 0.0;
    double* element_ptr;


    element_ptr = vp->elements;

    for (i = 0; i<vp->length; i++)
    {
       sum += (*element_ptr);
       element_ptr++;
    }

    return sum;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             sum_vector_squared_elements
 *
 * Returns the sum of the elements squared of a vector
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

double sum_vector_squared_elements(const Vector* vp)
{
    int   i;
    double  sum = 0.0;
    double* element_ptr;


    element_ptr = vp->elements;

    for (i = 0; i<vp->length; i++)
    {
       sum += ((*element_ptr) * (*element_ptr));
       element_ptr++;
    }

    return sum;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              multiply_vector_elements
 *
 * Returns the product of the elements of a vector
 *
 * This routine calculates the product of the elements of a vector.
 *
 * Returns :
 *     The product of the elements of a vector.
 *
 * Index : vectors
 *
 * -----------------------------------------------------------------------------
 */

double multiply_vector_elements(const Vector* vp)
{
    int   i;
    double  product;
    double* element_ptr;


    product = 1.0;

    element_ptr = vp->elements;

    for (i = 0; i<vp->length; i++)
    {
       product *= (*element_ptr);
       element_ptr++;
    }

    return product;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

