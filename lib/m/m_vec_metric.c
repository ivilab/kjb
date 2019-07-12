
/* $Id: m_vec_metric.c 21596 2017-07-30 23:33:36Z kobus $ */

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
#include "m/m_vec_norm.h"
#include "m/m_vec_metric.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                           max_abs_vector_difference
 *
 * Returns the max absolute difference between two vectors
 *
 * This routine calculates the maximum of the absolute value of the difference
 * of corrresponding elements of two vectors.
 *
 * Returns:
 *    The distance between the two matrices, or a negative number if there is an
 *    error (not very likely--currently impossible). Note that sending in
 *    vectors of difference lengths is regarded as a bug. (See set_bug(3)).
 *    If both arguments equal NULL then the difference returned is (still) zero.
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

double max_abs_vector_difference
(
    const Vector* first_vp,
    const Vector* second_vp
)
{
    int   i;
    int   length;
    double* first_pos;
    double* second_pos;
    double  max_diff   = 0.0;


    /*
     * Return is a bit awkward due to function semantics, so we opt for calling
     * dimension problems a bug. The ERROR return gets cast into a double which
     * is fine as this routine should return a non-negative value.
    */
    ESBRE(check_same_vector_lengths(first_vp, second_vp,
                                    "max_abs_vector_difference"));

    length = first_vp->length;
    first_pos = first_vp->elements;
    second_pos = second_vp->elements;

    for (i=0; i<length; i++)
    {
        double diff;


        diff = ABS_OF(*first_pos - *second_pos);

        if (diff > max_diff) max_diff = diff;

        first_pos++;
        second_pos++;
    }

    return max_diff;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              get_vector_angle_in_degrees
 *
 * Computes the angle between two vectors in degrees
 *
 * This routine computes the angle between two vectors in degrees and puts the
 * result in *result_ptr.
 *
 * Returns :
 *    NO_ERROR on success, and ERROR on failure. This routine will fail if
 *    either of the vectors is too close to zero, or the vectors are of un-equal
 *    length. It should be noted that this error is currently treated as bug
 *    (see kjb_bug(3)).
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
 */

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

int get_vector_angle_in_degrees
(
    const Vector* vp1,
    const Vector* vp2,
    double*       result_ptr
)
{
    double rad_vector_angle;


    ERE(get_vector_angle_in_radians(vp1, vp2, &rad_vector_angle));

    *result_ptr = (180.0 * rad_vector_angle) / M_PI;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              get_vector_angle_in_radians
 *
 * Computes the angle between two vectors in radians
 *
 * This routine computes the angle between two vectors in radians and puts the
 * result in *result_ptr.
 *
 * Returns :
 *    NO_ERROR on success, and ERROR on failure. This routine will fail if
 *    either of the vectors is too close to zero, or the vectors are of un-equal
 *    length. It should be noted that this error is currently treated as bug
 *    (see kjb_bug(3)).
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
 */

int get_vector_angle_in_radians
(
    const Vector* vp1,
    const Vector* vp2,
    double*       result_ptr
)
{
    double mag1;
    double mag2;
    double mag_product;
    double dot_product;
    double temp;


    verify_vector(vp1, NULL);
    verify_vector(vp2, NULL);

    ERE(check_same_vector_lengths(vp1, vp2,
                                  "get_vector_angle_in_radians"));

    mag1 = vector_magnitude(vp1);
    mag2 = vector_magnitude(vp2);
    mag_product = mag1 * mag2;

    /* p_stderr("KJB mag: %.20e\n", mag_product); */

#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
    if ( ! IS_POSITIVE_DBL(mag_product) )
    {
        set_error("Vector magnitude product too close to zero to find angle.");
        return ERROR;
    }
#else 
#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
    if (mag_product <= 0.0)
    {
        set_error("Vector magnitude product too close to zero to find angle.");
        return ERROR;
    }
#endif 
#endif

    ERE(get_dot_product(vp1, vp2, &dot_product));

    temp = dot_product / mag_product;

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
    if (    (temp > DBL_HALF_MOST_POSITIVE)
         || (temp < DBL_HALF_MOST_NEGATIVE)
       )
    {
        set_error("Vector magnitude product too close to zero to find angle.");
        return ERROR;
    }
#endif
    /*
    // If |temp| is slightly more than 1.0, we will be treating it as 1.0.
    // However, if it more than slightly more than 1.0, it is an error.
    */
    if (ABS_OF(temp) > 1.00001)
    {
        set_error("Absolute value of normalized dot product exceeds one.");
        add_error("Dot product was %e.", dot_product);
        add_error("Magnitude of first vector was %e.", mag1);
        add_error("Magnitude of second vector was %e.", mag2);
        add_error("Magnitude product was %e.", mag_product);
        add_error("Normalized dot product was %e.", temp);
        add_error("Attempt to find angle between vectors failed.");
        return ERROR;
    }

    /*
     * Having touble with the optmizer. Perhaps this is giving it trouble.

       temp = (temp > 1.0) ? 1.0 : (temp < -1.0 ? -1.0 : temp);
    */

    if (temp > 1.0) 
    {
        if (IS_GREATER_DBL(temp, 1.0))
        {
            dbp("Suspicious value of normalized dot product.");
            dbe(temp); 
        }

        temp = 1.0;
    }
    else if (temp < -1.0)
    {
        if (IS_LESSER_DBL(temp, -1.0))
        {
            dbp("Suspicious value of normalized dot product.");
            dbe(temp); 
        }
        temp = -1.0;
    }

    /*
                p_stderr("KJB final arg to acos: %.20e\n", temp);  
                */

    *result_ptr = acos(temp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           get_dot_product
 *
 * Computes the dot product of two vectors
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

int get_dot_product
(
    const Vector* vp1,
    const Vector* vp2,
    double*       result_ptr
)
{
    int   i;
    int   len;
    double  sum = 0.0;
    double* pos1;
    double* pos2;


    if ((vp1 != NULL) || (vp2 != NULL))
    {
        ERE(check_same_vector_lengths(vp1, vp2, "get_dot_product"));

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
 *                           vector_distance
 *
 * Returns the distance between two points
 *
 * This routine calculates the distance between two points represented by
 * vectors. In other words, it computes the norm of the difference of the
 * vectors.
 *
 * Returns:
 *    The distance between the two points, or a negative number if there is an
 *    error (not very likely--currently impossible). Note that sending in
 *    vectors of difference lengths is regarded as a bug. (See set_bug(3)).
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

double vector_distance(const Vector* vp1, const Vector* vp2)
{


    return sqrt(vector_distance_sqrd(vp1, vp2));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           vector_distance_sqrd
 *
 * Returns the distance squared between two points
 *
 * This routine calculates the distance squared between two points represented
 * by vectors. In other words, it computes the norm squared of the difference
 * of the vectors.
 *
 * Returns:
 *    The distance squared between the two points, or a negative number if
 *    there is an error (not very likely--currently impossible). Note that
 *    sending in vectors of difference lengths is regarded as a bug. (See
 *    set_bug(3)).
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

double vector_distance_sqrd(const Vector* vp1, const Vector* vp2)
{
    double* pos1;
    double* pos2;
    double  temp;
    double  sum_sqr;
    int   i;
    int   len;


    /*
     * Return is a bit awkward due to function semantics, so we opt for calling
     * dimension problems a bug. The ERROR return gets cast into a double which
     * is fine as this routine should return a non-negative value.
    */
    ESBRE(check_same_vector_lengths(vp1, vp2, "vector_distance_sqrd"));

    sum_sqr = 0.0;

    pos1 = vp1->elements;
    pos2 = vp2->elements;

    len = vp1->length;

    for (i=0; i<len; i++)
    {
        temp = (*pos1) - (*pos2);

        sum_sqr += (temp * temp);

        pos1++;
        pos2++;
    }

    return sum_sqr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int first_vector_is_less
(
    const Vector* vp1,
    const Vector* vp2,
    double        relative_tolerance,
    double        absolute_tolerance
)
{
    return first_vector_is_less_2(vp1, vp2, relative_tolerance, absolute_tolerance, NULL, NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int first_vector_is_less_2
(
    const Vector* vp1,
    const Vector* vp2,
    double        relative_tolerance,
    double        absolute_tolerance,
    int*          index_for_biggest_difference_ptr,
    double*       biggest_difference_ptr

)
{
    int i;
    double relative_max;
    double absolute_max;
    double max;
    double difference;
    double biggest_difference = 0.0;
    int    index_for_biggest_difference = NOT_SET;



    if (vp1->length != vp2->length)
    {
        SET_ARGUMENT_BUG();
        return FALSE;
    }

    for (i=0; i<vp1->length; i++)
    {
        relative_max = ADD_RELATIVE_DBL(vp2->elements[ i ],
                                        relative_tolerance);

        absolute_max = vp2->elements[ i ] + absolute_tolerance;

        max = MAX_OF(relative_max, absolute_max);

        if (vp1->elements[ i ] > max)
        {
            difference = vp1->elements[ i ] - max;       

            if (difference > biggest_difference) 
            {
                biggest_difference = difference;
                index_for_biggest_difference = i;
            }
        }
    }

    if (KJB_IS_SET(index_for_biggest_difference))
    {
        if (biggest_difference_ptr != NULL)
        {
            *biggest_difference_ptr = biggest_difference;
        }

        if (index_for_biggest_difference_ptr != NULL)
        {
            *index_for_biggest_difference_ptr = index_for_biggest_difference;
        }
        return FALSE;
    }

    return TRUE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

double rms_absolute_error_between_vectors
(
    const Vector* vp1,
    const Vector* vp2
)
{
    return vector_distance(vp1, vp2) / sqrt((double)vp1->length);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

double normalized_rms_error_between_vectors
(
    const Vector* vp1,
    const Vector* vp2
)
{
    double rms_error = vector_distance(vp1, vp2);
    double v1_mag = vector_magnitude(vp1);
    double v2_mag = vector_magnitude(vp2);

    if ((rms_error < 0.0) || (v1_mag < 0.0) || (v2_mag < 0.0))
    {
        return DBL_NOT_SET;
    }
    else
    {
        double max_mag = MAX_OF(v1_mag, v2_mag);

        if (max_mag > 0.0)
        {
            return rms_error / max_mag;
        }
        else
        {
            return 0.0;
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

double rms_relative_error_between_vectors
(
    const Vector* vp1,
    const Vector* vp2
)
{
    Vector* diff_vp = NULL;
    double result;
    int i;
    int length = vp1->length;


    if (subtract_vectors(&diff_vp, vp1, vp2) == ERROR)
    {
        free_vector(diff_vp);
        return DBL_NOT_SET;
    }

    for (i = 0; i < length; i++)
    {
        diff_vp->elements[ i ] /= MAX_OF(ABS_OF(vp1->elements[ i ]), ABS_OF(vp2->elements[ i ]));
    }

    result = vector_magnitude(diff_vp) / sqrt((double)vp1->length);

    free_vector(diff_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

