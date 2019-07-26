
/* $Id: m_vec_norm.c 20654 2016-05-05 23:13:43Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author). (Lindsay Martin
|  contributed to the documentation of this code).
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

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                              normalize_vector
 *
 * Normalizes the elements of a vector.
 *
 * This routine normalizes an input vector and places the result into the
 * specified target vector. The normalization method must be supplied as an
 * argument.
 *
 * "target_vpp" is a double pointer to the Vector object to receive the
 * normalized output.
 *
 * "vp" points to the source Vector object to normalize.
 *
 * "normalization_method" indicates which normalization to perform. Currently,
 * the following values are supported:
 *
 *    DONT_NORMALIZE             - Do nothing. "target_vpp" is untouched.
 *    NORMALIZE_BY_MAGNITUDE     - Vector magnitude normalized to 1.
 *    NORMALIZE_BY_MEAN          - Vector mean value normailized to 1.
 *    NORMALIZE_BY_SUM           - Sum of elements normalized to 1.
 *    NORMALIZE_BY_MAX_ABS_VALUE - Maximum absolute value is normalized to 1.
 *
 * If "vp" is NULL, then "*target_vpp" becomes NULL also (and its contents
 * are freed if it has any), and NO_ERROR is returned.
 *
 * All normalization methods divide each element of the vector by the specified
 * quantity. Errors will be triggered if the normalization quantity is close to
 * zero.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     ow_normalize_vector, normalize_vector_2, ow_normalize_vector_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int normalize_vector
(
    Vector** target_vpp  /* Double pointer to output Vector. */,
    const Vector* vp   /* Pointer to input Vector.         */,
    Norm_method normalization_method   /* Method to use.              */
)
{


    return normalize_vector_2(target_vpp, vp, normalization_method,
                              (double*)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            scale_vector_by_sum
 *
 * Scales a vector so that the sum of elements is 1
 *
 * Scales the the elements of the input vector "vp" so that the sum of
 * it's elements equal 1. The scaled result is placed in "target_vpp",
 * and the original vector is untouched.
 *
 * The sum must be positive, and not too close to zero, otherwise ERROR is
 * returned, and an appropriate error message is set.
 *
 * If "vp" is NULL, then "*target_vpp" becomes NULL also (and its contents
 * are freed if it has any), and NO_ERROR is returned.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     ow_scale_vector_by_sum_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int scale_vector_by_sum
(
    Vector** target_vpp /* Double pointer to output Vector. */,
    const Vector* vp /* Pointer to input Vector.         */
)
{
    return scale_vector_by_sum_2(target_vpp, vp, (double*)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         scale_vector_by_mean
 *
 * Scales a vector such that it's mean value is 1
 *
 * Scales the contents of the input vector "vp" such that the vector mean
 * is 1. Places the result in "target_vpp", leaving the input vector
 * untouched.
 *
 * The mean must be positive, and not too close to zero, otherwise ERROR is
 * returned, and an appropriate error message is set.
 *
 * If "vp" is NULL, then "*target_vpp" becomes NULL also (and its contents
 * are freed if it has any), and NO_ERROR is returned.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     ow_scale_vector_by_mean_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int scale_vector_by_mean
(
   Vector** target_vpp   /* Double pointer to output Vector. */,
   const Vector* vp /* Pointer to input Vector.         */
)
{
    return scale_vector_by_mean_2(target_vpp, vp, (double*)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         scale_vector_by_max_abs
 *
 * Scales a vector such that it's maximum absolute value is 1
 *
 * Scales the contents of the input vector "vp" such that the maximum of
 * the absolute value of all elements is 1. Places the result in
 * "target_vpp", leaving the input vector untouched.
 *
 * If "vp" is NULL, then "*target_vpp" becomes NULL also (and its contents
 * are freed if it has any), and NO_ERROR is returned.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     ow_scale_vector_by_max_abs_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int scale_vector_by_max_abs
(
    Vector** target_vpp /* Double pointer to output Vector. */,
    const Vector* vp /* Pointer to input Vector.         */
)
{
    return scale_vector_by_max_abs_2(target_vpp, vp, (double*)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             scale_vector_by_max
 *
 * Scales a vector such that it's largest element is 1
 *
 * Scales the contents of the input vector "vp" such that the vector maximum
 * is 1. Places the result in "target_vpp", leaving the input vector
 * untouched.
 *
 * The maximum element must be positive, and not too close to zero, otherwise
 * ERROR is returned, and an appropriate error message is set.
 *
 * If "vp" is NULL, then "*target_vpp" becomes NULL also (and its contents
 * are freed if it has any), and NO_ERROR is returned.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     ow_scale_vector_by_max_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int scale_vector_by_max
(
    Vector** target_vpp  /* Double pointer to output Vector. */,
    const Vector* vp /* Pointer to input Vector.         */
)
{
    return scale_vector_by_max_2(target_vpp, vp, (double*)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      scale_vector_by_magnitude
 *
 * Scales a vector such that it's magnitude is 1
 *
 * Scales the contents of the input vector "vp" such that the vector
 * magnitude is 1. Places the result in "target_vpp", leaving the
 * input vector untouched.
 *
 * If "vp" is NULL, then "*target_vpp" becomes NULL also (and its contents
 * are freed if it has any), and NO_ERROR is returned.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     ow_scale_vector_by_magnitude_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int scale_vector_by_magnitude
(
    Vector** target_vpp  /* Double pointer to output Vector. */,
    const Vector* vp
)
{
    return scale_vector_by_magnitude_2(target_vpp, vp, (double*)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             ow_normalize_vector
 *
 * Normalizes the elements of a vector, overwriting contents.
 *
 * This routine normalizes an input vector, overwriting the vector's exisiting
 * contents. The normalization method must be supplied as an argument.
 *
 * "vp" points to the source Vector object to normalize. The initial contents
 * of this vector will be overwritten with the normalized values.
 *
 * "normalization_method" indicates which normalization to perform. Currently,
 * the following values are supported:
 *
 *    DONT_NORMALIZE             - Do nothing. "vp" is untouched.
 *    NORMALIZE_BY_MAGNITUDE     - Vector magnitude normalized to 1.
 *    NORMALIZE_BY_MEAN          - Vector mean value normailized to 1.
 *    NORMALIZE_BY_SUM           - Sum of elements normalized to 1.
 *    NORMALIZE_BY_MAX_ABS_VALUE - Maximum absolute value is normalized to 1.
 *
 * If "vp" is NULL, then this routine simply returns NO_ERROR.
 *
 * All normalization methods divide each element of the vector by the specified
 * quantity. Errors will be triggered if the normalization quantity is close to
 * zero.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     normalize_vector, normalize_vector_2, ow_normalize_vector_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/

int ow_normalize_vector
(
    Vector* vp /* Pointer to input Vector. Contents overwritten. */,
    Norm_method normalization_method
)
{
    return ow_normalize_vector_2(vp, normalization_method, (double*)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        silent_ow_scale_vector_by_sum
 *
 * Scales a vector so that the sum of elements is 1
 *
 * This routine is like safe_ow_scale_vector_by_sum(), except that no warning
 * message is printed. 
 *
 * If "vp" is NULL, then this routine simply returns NO_ERROR.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/

int silent_ow_scale_vector_by_sum(Vector* vp)
{

    if (vp == NULL)
    {
        return NO_ERROR;
    }

     if (ow_scale_vector_by_sum(vp) == ERROR)
     {
         ERE(ow_set_vector(vp, 1.0 / vp->length));
     }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        safe_ow_scale_vector_by_sum
 *
 * Scales a vector so that the sum of elements is 1
 *
 * This routine is like ow_scale_vector_by_sum(3), except that if the scale were
 * to lead to divide by zero, then a warning message is printed, and the vector
 * is set to uniform values.
 *
 * If "vp" is NULL, then this routine simply returns NO_ERROR.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
#ifdef TEST

int debug_safe_ow_scale_vector_by_sum
(
    Vector*     vp,
    const char* file,
    int         line
)
{

    if (vp == NULL)
    {
        return NO_ERROR;
    }

    if (ow_scale_vector_by_sum(vp) == ERROR)
    {
        warn_pso("Scale of vector failed. Making it uniform.\n");
        warn_pso("(Called from line %d of %s)\n\n", line, file);
        ERE(ow_set_vector(vp, 1.0 / vp->length));
    }

    return NO_ERROR;
}

#else

int safe_ow_scale_vector_by_sum(Vector* vp)
{

    if (vp == NULL)
    {
        return NO_ERROR;
    }

     if (ow_scale_vector_by_sum(vp) == ERROR)
     {
         warn_pso("Scale of vector failed. Making it uniform.\n");
         ERE(ow_set_vector(vp, 1.0 / vp->length));
     }

    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     safe_ow_scale_vector_by_sum_2
 *
 * Scales a vector so that the sum of elements is 1
 *
 * This routine is like ow_scale_vector_by_sum_2(3), except that if the scale
 * were to lead to divide by zero, then a warning message is printed, and the
 * vector is set to uniform values.
 *
 * If "vp" is NULL, then this routine simply returns NO_ERROR.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int safe_ow_scale_vector_by_sum_2(Vector* vp, double* scale_factor_ptr)
{

    if (vp == NULL)
    {
        if (scale_factor_ptr != NULL)
        {
            *scale_factor_ptr = 0.0;
        }
        return NO_ERROR;
    }

    if (ow_scale_vector_by_sum_2(vp, scale_factor_ptr) == ERROR)
    {
        warn_pso("Scale of vector failed. Making it uniform.\n");
        ERE(ow_set_vector(vp, 1.0 / vp->length));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            ow_scale_vector_by_sum
 *
 * Scales a vector so that the sum of elements is 1
 *
 * Scales the the elements of the input vector "vp" so that the sum of it's
 * elements equal 1. The scaled result is placed back in "vp", and it's original
 * contents are overwritten.
 *
 * If "vp" is NULL, then this routine simply returns NO_ERROR.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int ow_scale_vector_by_sum
(
    Vector* vp /* Pointer to input Vector. Contents overwritten. */
)
{

    return ow_scale_vector_by_sum_2(vp, (double*)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        ow_scale_vector_by_mean
 *
 * Scales a vector such that it's mean value is 1
 *
 * Scales the contents of the input vector "vp" such that the vector mean
 * is 1. Places the result back in "vp", overwriting it's original contents.
 *
 * If "vp" is NULL, then this routine simply returns NO_ERROR.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     scale_vector_by_mean_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int ow_scale_vector_by_mean
(
    Vector* vp   /* Pointer to input Vector. Contents overwritten. */
)
{
    return ow_scale_vector_by_mean_2(vp, (double*)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         ow_scale_vector_by_max_abs
 *
 * Scales a vector such that it's maximum absolute value is 1
 *
 * Scales the contents of the input vector "vp" such that the maximum of
 * the absolute value of all elements is 1. Places the result back in
 * "vp", overwriting the original contents.
 *
 * If "vp" is NULL, then this routine simply returns NO_ERROR.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     scale_vector_by_max_abs_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int ow_scale_vector_by_max_abs
(
    Vector* vp      /* Pointer to input Vector. Contents overwritten. */
)
{
    return ow_scale_vector_by_max_abs_2(vp, (double*)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           ow_scale_vector_by_max_2
 *
 * Scales a vector such that it's largest element is 1
 *
 * Scales the contents of the input vector "vp" such that the vector maximum
 * is 1. Places the result in "vp", overwriting it's original contents.
 *
 * The argument scale_factor_ptr can be used to return the scale factor used to
 * normalize the vector. Set it to NULL if you are not interested.
 *
 * If "vp" is NULL, then this routine simply returns NO_ERROR.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     scale_vector_by_max_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int ow_scale_vector_by_max
(
    Vector* vp   /* Pointer to input Vector. Contents overwritten. */
)
{
    return ow_scale_vector_by_max_2(vp, (double*)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      ow_scale_vector_by_magnitude
 *
 * Scales a vector such that it's magnitude is 1
 *
 * Scales the contents of the input vector "vp" such that the vector
 * magnitude is 1. Places the result in "vp", overwriting its original
 * contents.
 *
 * If "vp" is NULL, then this routine simply returns NO_ERROR.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     scale_vector_by_magnitude_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/

int ow_scale_vector_by_magnitude
(
    Vector* vp    /* Pointer to input Vector. Contents overwritten. */
)
{
    return ow_scale_vector_by_magnitude_2(vp, (double*)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              normalize_vector_2
 *
 * Normalizes the elements of a vector.
 *
 * This routine normalizes an input vector and places the result into the
 * specified target vector. The normalization method must be supplied as an
 * argument.
 *
 * "target_vpp" is a double pointer to the Vector object to receive the
 * normalized output.
 *
 * "vp" points to the source Vector object to normalize.
 *
 * "normalization_method" indicates which normalization to perform. Currently,
 * the following values are supported:
 *
 *    DONT_NORMALIZE             - Do nothing. "target_vpp" is untouched.
 *    NORMALIZE_BY_MAGNITUDE     - Vector magnitude normalized to 1.
 *    NORMALIZE_BY_MEAN          - Vector mean value normailized to 1.
 *    NORMALIZE_BY_SUM           - Sum of elements normalized to 1.
 *    NORMALIZE_BY_MAX_ABS_VALUE - Maximum absolute value is normalized to 1.
 *
 * The argument scale_factor_ptr can be used to return the scale factor used to
 * normalize the vector. Set it to NULL if you are not interested.
 *
 * If "vp" is NULL, then "*target_vpp" becomes NULL also (and its contents
 * are freed if it has any), and NO_ERROR is returned.
 *
 * All normalization methods divide each element of the vector by the specified
 * quantity. Errors will be triggered if the normalization quantity is close to
 * zero.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     ow_normalize_vector_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int normalize_vector_2
(
    Vector** target_vpp  /* Double pointer to output Vector. */,
    const Vector* vp   /* Pointer to input Vector.         */,
    Norm_method normalization_method   /* Method to use.              */,
    double* scale_factor_ptr   /* Optional return of scaling */
)
{


    if (vp == NULL)
    {
        UNTESTED_CODE();
        free_vector(*target_vpp);
        *target_vpp = NULL;
    }

    if (normalization_method == DONT_NORMALIZE)
    {
        return NO_ERROR;
    }
    else if (normalization_method == NORMALIZE_BY_MAGNITUDE)
    {
        return scale_vector_by_magnitude_2(target_vpp, vp, scale_factor_ptr);
    }
    else if (normalization_method == NORMALIZE_BY_MEAN)
    {
        return scale_vector_by_mean_2(target_vpp, vp, scale_factor_ptr);
    }
    else if (normalization_method == NORMALIZE_BY_SUM)
    {
        return scale_vector_by_sum_2(target_vpp, vp, scale_factor_ptr);
    }
    else if (normalization_method == NORMALIZE_BY_MAX_ABS_VALUE)
    {
        return scale_vector_by_max_abs_2(target_vpp, vp, scale_factor_ptr);
    }
    else
    {
        set_bug("Invalid normalization method passed to ow_normalize_vector_2.");
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            scale_vector_by_sum_2
 *
 * Scales a vector so that the sum of elements is 1
 *
 * Scales the the elements of the input vector "vp" so that the sum of
 * it's elements equal 1. The scaled result is placed in "target_vpp",
 * and the original vector is untouched.
 *
 * The argument scale_factor_ptr can be used to return the scale factor used to
 * normalize the vector. Set it to NULL if you are not interested.
 *
 * The sum must be positive, and not too close to zero, otherwise ERROR is
 * returned, and an appropriate error message is set.
 *
 * If "vp" is NULL, then "*target_vpp" becomes NULL also (and its contents
 * are freed if it has any), and NO_ERROR is returned.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     ow_scale_vector_by_sum_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int scale_vector_by_sum_2
(
    Vector** target_vpp /* Double pointer to output Vector. */,
    const Vector* vp /* Pointer to input Vector.         */,
    double* scale_factor_ptr /* Optional return of scaling */
)
{
    double sum;


    if (vp == NULL)
    {
        UNTESTED_CODE();
        free_vector(*target_vpp);
        *target_vpp = NULL;
        return NO_ERROR;
    }

    sum = sum_vector_elements(vp);

    if (sum < 0.0)
    {
        set_error("Scaling vector by sum failed.");
        add_error("Sum must be positive, but it is %.2e.", sum);
        return ERROR;
    }
    else if (sum < MIN_ABS_NORMALIZATION_VALUE)
    {
        set_error("Sum is too close to zero to scale vector.");
        add_error("    [ | %e | > %e ].", sum, MIN_ABS_NORMALIZATION_VALUE);
        return ERROR;
    }

    ERE(divide_vector_by_scalar(target_vpp, vp, sum));

    if (scale_factor_ptr != NULL)
    {
        UNTESTED_CODE();
        *scale_factor_ptr = sum;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         scale_vector_by_mean_2
 *
 * Scales a vector such that it's mean value is 1
 *
 * Scales the contents of the input vector "vp" such that the vector mean
 * is 1. Places the result in "target_vpp", leaving the input vector
 * untouched.
 *
 * The argument scale_factor_ptr can be used to return the scale factor used to
 * normalize the vector. Set it to NULL if you are not interested.
 *
 * The mean must be positive, and not too close to zero, otherwise ERROR is
 * returned, and an appropriate error message is set.
 *
 * If "vp" is NULL, then "*target_vpp" becomes NULL also (and its contents
 * are freed if it has any), and NO_ERROR is returned.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     ow_scale_vector_by_mean_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int scale_vector_by_mean_2
(
   Vector** target_vpp   /* Double pointer to output Vector. */,
   const Vector* vp /* Pointer to input Vector.         */,
   double* scale_factor_ptr /* Optional return of scaling */
)
{
    double sum;
    double mean;


    if (vp == NULL)
    {
        UNTESTED_CODE();
        free_vector(*target_vpp);
        *target_vpp = NULL;
        return NO_ERROR;
    }

    sum = sum_vector_elements(vp);

    mean = sum / ((double) vp->length);

    if (mean < 0.0)
    {
        set_error("Scaling vector by mean failed.");
        add_error("Mean must be positive, but it is %.2e.", mean);
        return ERROR;
    }
    else if (mean < MIN_ABS_NORMALIZATION_VALUE)
    {
        set_error("Mean is too close to zero to scale vector.");
        add_error("    [ | %e | > %e ].", mean, MIN_ABS_NORMALIZATION_VALUE);
        return ERROR;
    }

    ERE(divide_vector_by_scalar(target_vpp, vp, mean));

    if (scale_factor_ptr != NULL)
    {
        UNTESTED_CODE();
        *scale_factor_ptr = mean;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         scale_vector_by_max_abs_2
 *
 * Scales a vector such that it's maximum absolute value is 1
 *
 * Scales the contents of the input vector "vp" such that the maximum of
 * the absolute value of all elements is 1. Places the result in
 * "target_vpp", leaving the input vector untouched.
 *
 * The argument scale_factor_ptr can be used to return the scale factor used to
 * normalize the vector. Set it to NULL if you are not interested.
 *
 * If "vp" is NULL, then "*target_vpp" becomes NULL also (and its contents
 * are freed if it has any), and NO_ERROR is returned.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     ow_scale_vector_by_max_abs_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int scale_vector_by_max_abs_2
(
    Vector** target_vpp /* Double pointer to output Vector. */,
    const Vector* vp /* Pointer to input Vector.         */,
    double* scale_factor_ptr /* Optional return of scaling */
)
{
    double max;


    if (vp == NULL)
    {
        UNTESTED_CODE();
        free_vector(*target_vpp);
        *target_vpp = NULL;
        return NO_ERROR;
    }

    max = max_abs_vector_element(vp);

    if (max < MIN_ABS_NORMALIZATION_VALUE)
    {
        set_error("Max abs is too close to zero to scale vector.");
        add_error("    [ | %e | > %e ].", max, MIN_ABS_NORMALIZATION_VALUE);
        return ERROR;
    }

    ERE(divide_vector_by_scalar(target_vpp, vp, max));

    if (scale_factor_ptr != NULL)
    {
        UNTESTED_CODE();
        *scale_factor_ptr = max;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             scale_vector_by_max_2
 *
 * Scales a vector such that it's largest element is 1
 *
 * Scales the contents of the input vector "vp" such that the vector maximum
 * is 1. Places the result in "target_vpp", leaving the input vector
 * untouched.
 *
 * The argument scale_factor_ptr can be used to return the scale factor used to
 * normalize the vector. Set it to NULL if you are not interested.
 *
 * The maximum element must be positive, and not too close to zero, otherwise
 * ERROR is returned, and an appropriate error message is set.
 *
 * If "vp" is NULL, then "*target_vpp" becomes NULL also (and its contents
 * are freed if it has any), and NO_ERROR is returned.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     ow_scale_vector_by_max_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int scale_vector_by_max_2
(
    Vector** target_vpp  /* Double pointer to output Vector. */,
    const Vector* vp /* Pointer to input Vector.         */,
    double* scale_factor_ptr  /* Optional return of scaling */
)
{
    double max;


    if (vp == NULL)
    {
        UNTESTED_CODE();
        free_vector(*target_vpp);
        *target_vpp = NULL;
        return NO_ERROR;
    }

    max = max_vector_element(vp);

    if (max < 0.0)
    {
        set_error("Scaling vector by max failed.");
        add_error("Max must be positive, but it is %.2e.", max);
        return ERROR;
    }
    else if (max < MIN_ABS_NORMALIZATION_VALUE)
    {
        set_error("Max is too close to zero to scale vector.");
        add_error("    [ | %e | > %e ].", max, MIN_ABS_NORMALIZATION_VALUE);
        return ERROR;
    }

    ERE(divide_vector_by_scalar(target_vpp, vp, max));

    if (scale_factor_ptr != NULL)
    {
        UNTESTED_CODE();
        *scale_factor_ptr = max;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      scale_vector_by_magnitude_2
 *
 * Scales a vector such that it's magnitude is 1
 *
 * Scales the contents of the input vector "vp" such that the vector
 * magnitude is 1. Places the result in "target_vpp", leaving the
 * input vector untouched.
 *
 * The argument scale_factor_ptr can be used to return the scale factor used to
 * normalize the vector. Set it to NULL if you are not interested.
 *
 * If "vp" is NULL, then "*target_vpp" becomes NULL also (and its contents
 * are freed if it has any), and NO_ERROR is returned.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     ow_scale_vector_by_magnitude_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int scale_vector_by_magnitude_2
(
    Vector** target_vpp  /* Double pointer to output Vector. */,
    const Vector* vp /* Pointer to input Vector.        */,
    double* scale_factor_ptr  /* Optional return of scaling */
)
{
    double norm;


    if (vp == NULL)
    {
        UNTESTED_CODE();
        free_vector(*target_vpp);
        *target_vpp = NULL;
        return NO_ERROR;
    }

    norm = vector_magnitude(vp);

    if (norm < MIN_ABS_NORMALIZATION_VALUE)
    {
        set_error("Norm is too close to zero to scale vector.");
        add_error("    [ | %e | > %e ].", norm, MIN_ABS_NORMALIZATION_VALUE);
        return ERROR;
    }

    ERE(divide_vector_by_scalar(target_vpp, vp, norm));

    if (scale_factor_ptr != NULL)
    {
        UNTESTED_CODE();
        *scale_factor_ptr = norm;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                             ow_normalize_vector_2
 *
 * Normalizes the elements of a vector, overwriting contents.
 *
 * This routine normalizes an input vector, overwriting the vector's exisiting
 * contents. The normalization method must be supplied as an argument.
 *
 * "vp" points to the source Vector object to normalize. The initial contents
 * of this vector will be overwritten with the normalized values.
 *
 * "normalization_method" indicates which normalization to perform. Currently,
 * the following values are supported:
 *
 *    DONT_NORMALIZE             - Do nothing. "vp" is untouched.
 *    NORMALIZE_BY_MAGNITUDE     - Vector magnitude normalized to 1.
 *    NORMALIZE_BY_MEAN          - Vector mean value normailized to 1.
 *    NORMALIZE_BY_SUM           - Sum of elements normalized to 1.
 *    NORMALIZE_BY_MAX_ABS_VALUE - Maximum absolute value is normalized to 1.
 *
 * The argument scale_factor_ptr can be used to return the scale factor used to
 * normalize the vector. Set it to NULL if you are not interested.
 *
 * If "vp" is NULL, then this routine simply returns NO_ERROR.
 *
 * All normalization methods divide each element of the vector by the specified
 * quantity. Errors will be triggered if the normalization quantity is close to
 * zero.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     normalize_vector_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/

int ow_normalize_vector_2
(
    Vector* vp /* Pointer to input Vector. Contents overwritten. */,
    Norm_method normalization_method,
    double* scale_factor_ptr   /* Optional return of scaling */
)
{


    if (vp == NULL)
    {
        UNTESTED_CODE();
        if (scale_factor_ptr != NULL)
        {
            *scale_factor_ptr = 0.0;
        }
        return NO_ERROR;
    }

    if (normalization_method == DONT_NORMALIZE)
    {
        return NO_ERROR;
    }
    else if (normalization_method == NORMALIZE_BY_MAGNITUDE)
    {
        return ow_scale_vector_by_magnitude_2(vp, scale_factor_ptr);
    }
    else if (normalization_method == NORMALIZE_BY_MEAN)
    {
        return ow_scale_vector_by_mean_2(vp, scale_factor_ptr);
    }
    else if (normalization_method == NORMALIZE_BY_SUM)
    {
        return ow_scale_vector_by_sum_2(vp, scale_factor_ptr);
    }
    else if (normalization_method == NORMALIZE_BY_MAX_ABS_VALUE)
    {
        return ow_scale_vector_by_max_abs_2(vp, scale_factor_ptr);
    }
    else
    {
        set_bug("Invalid normalization method passed to ow_normalize_vector_2.");
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            ow_scale_vector_by_sum_2
 *
 * Scales a vector so that the sum of elements is 1
 *
 * Scales the the elements of the input vector "vp" so that the sum of it's
 * elements equal 1. The scaled result is placed back in "vp", and it's original
 * contents are overwritten.
 *
 * If "vp" is NULL, then this routine simply returns NO_ERROR.
 *
 * The argument scale_factor_ptr can be used to return the scale factor used to
 * normalize the vector. Set it to NULL if you are not interested.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     scale_vector_by_sum_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int ow_scale_vector_by_sum_2
(
    Vector* vp/* Pointer to input Vector. Contents overwritten. */,
    double* scale_factor_ptr /* Optional return of scaling */
)
{
    double sum;


    if (vp == NULL)
    {
        UNTESTED_CODE();
        if (scale_factor_ptr != NULL)
        {
            *scale_factor_ptr = 0.0;
        }
        return NO_ERROR;
    }

    sum = sum_vector_elements(vp);

    if (scale_factor_ptr != NULL)
    {
        UNTESTED_CODE();
        *scale_factor_ptr = sum;
    }

    if (fabs(sum) < MIN_ABS_NORMALIZATION_VALUE)
    {
        set_error("Sum is too close to zero to scale vector.");
        add_error("    [ | %e | < %e ].", sum, MIN_ABS_NORMALIZATION_VALUE);
        return ERROR;
    }

    ERE(ow_divide_vector_by_scalar(vp, sum));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        ow_scale_vector_by_mean_2
 *
 * Scales a vector such that it's mean value is 1
 *
 * Scales the contents of the input vector "vp" such that the vector mean
 * is 1. Places the result back in "vp", overwriting it's original contents.
 *
 * If "vp" is NULL, then this routine simply returns NO_ERROR.
 *
 * The argument scale_factor_ptr can be used to return the scale factor used to
 * normalize the vector. Set it to NULL if you are not interested.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     scale_vector_by_mean_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int ow_scale_vector_by_mean_2
(
    Vector* vp   /* Pointer to input Vector. Contents overwritten. */,
    double* scale_factor_ptr /* Optional return of scaling */
)
{
    double sum;
    double mean;


    if (vp == NULL)
    {
        UNTESTED_CODE();
        return NO_ERROR;
    }

    sum = sum_vector_elements(vp);

    mean = sum / ((double) vp->length);

    if (scale_factor_ptr != NULL)
    {
        UNTESTED_CODE();
        *scale_factor_ptr = mean;
    }

    if (fabs(mean) < MIN_ABS_NORMALIZATION_VALUE)
    {
        set_error("Mean is too close to zero to scale vector.");
        add_error("    [ | %e | > %e ].", mean, MIN_ABS_NORMALIZATION_VALUE);
        return ERROR;
    }

    ERE(ow_divide_vector_by_scalar(vp, mean));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         ow_scale_vector_by_max_abs_2
 *
 * Scales a vector such that it's maximum absolute value is 1
 *
 * Scales the contents of the input vector "vp" such that the maximum of
 * the absolute value of all elements is 1. Places the result back in
 * "vp", overwriting the original contents.
 *
 * The argument scale_factor_ptr can be used to return the scale factor used to
 * normalize the vector. Set it to NULL if you are not interested.
 *
 * If "vp" is NULL, then this routine simply returns NO_ERROR.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     scale_vector_by_max_abs_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int ow_scale_vector_by_max_abs_2
(
    Vector* vp             /* Pointer to input Vector. Contents overwritten. */,
    double* scale_factor_ptr /* Optional return of scaling */
)
{
    double max;


    if (vp == NULL)
    {
        UNTESTED_CODE();
        return NO_ERROR;
    }

    max = max_abs_vector_element(vp);

    if (scale_factor_ptr != NULL)
    {
        *scale_factor_ptr = max;
    }

    if (max < MIN_ABS_NORMALIZATION_VALUE)
    {
        set_error("Max abs is too close to zero to scale vector.");
        add_error("    [ | %e | > %e ].", max, MIN_ABS_NORMALIZATION_VALUE);
        return ERROR;
    }

    ERE(ow_divide_vector_by_scalar(vp, max));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           ow_scale_vector_by_max_2
 *
 * Scales a vector such that it's largest element is 1
 *
 * Scales the contents of the input vector "vp" such that the vector maximum
 * is 1. Places the result in "vp", overwriting it's original contents.
 *
 * The argument scale_factor_ptr can be used to return the scale factor used to
 * normalize the vector. Set it to NULL if you are not interested.
 *
 * If "vp" is NULL, then this routine simply returns NO_ERROR.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     scale_vector_by_max_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/
int ow_scale_vector_by_max_2
(
    Vector* vp            /* Pointer to input Vector. Contents overwritten. */,
    double* scale_factor_ptr  /* Optional return of scaling */
)
{
    double max;


    if (vp == NULL)
    {
        UNTESTED_CODE();
        return NO_ERROR;
    }

    max = max_vector_element(vp);

    if (scale_factor_ptr != NULL)
    {
        UNTESTED_CODE();
        *scale_factor_ptr = max;
    }

    if (fabs(max) < MIN_ABS_NORMALIZATION_VALUE)
    {
        set_error("Max is too close to zero to scale vector.");
        add_error("    [ | %e | > %e ].", max, MIN_ABS_NORMALIZATION_VALUE);
        return ERROR;
    }

    ERE(ow_divide_vector_by_scalar(vp, max));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      ow_scale_vector_by_magnitude_2
 *
 * Scales a vector such that it's magnitude is 1
 *
 * Scales the contents of the input vector "vp" such that the vector
 * magnitude is 1. Places the result in "vp", overwriting its original
 * contents.
 *
 * The argument scale_factor_ptr can be used to return the scale factor used to
 * normalize the vector. Set it to NULL if you are not interested.
 *
 * If "vp" is NULL, then this routine simply returns NO_ERROR.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin and Kobus Barnard
 *
 * Related:
 *     scale_vector_by_magnitude_2
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/

int ow_scale_vector_by_magnitude_2
(
    Vector* vp            /* Pointer to input Vector. Contents overwritten. */,
    double* scale_factor_ptr  /* Optional return of scaling */
)
{
    double norm;


    if (vp == NULL)
    {
        UNTESTED_CODE();
        return NO_ERROR;
    }

    norm = vector_magnitude(vp);

    if (scale_factor_ptr != NULL)
    {
        UNTESTED_CODE();
        *scale_factor_ptr = norm;
    }

    if (norm < MIN_ABS_NORMALIZATION_VALUE)
    {
        set_error("Norm is too close to zero to scale vector.");
        add_error("    [ | %e | > %e ].", norm, MIN_ABS_NORMALIZATION_VALUE);
        return ERROR;
    }

    ERE(ow_divide_vector_by_scalar(vp, norm));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               vector_magnitude
 *
 * Calculates the magnitude of a vector
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

double vector_magnitude(const Vector* vp)
{
    int     i;
    double  sum = 0.0;
    int     len = vp->length;
    double* pos = vp->elements;


    for (i = 0; i<len; i++)
    {
        sum += (*pos) * (*pos);
        pos++;
    }

    return sqrt(sum);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           max_abs_vector_element
 *
 * Returns the vector element with the maximum absolute value
 *
 * This routine returns the element with the maximum absolute value contained
 * in the vector specified by "vp".
 *
 * Returns:
 *     Maximum absolute value of all elements.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     max_vector_element, min_vector_element
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/
double max_abs_vector_element(const Vector* vp)
{
    int i;
    double max, abs_val;
    double* element_ptr;


    if (vp->length == 0)
    {
        SET_CANT_HAPPEN_BUG();
        return 0.0;
    }

    element_ptr = vp->elements;

    max = fabs(*element_ptr);
    element_ptr++;

    for (i = 1; i<vp->length; i++)
    {
       abs_val = fabs(*element_ptr);

       if (abs_val > max)
       {
           max = abs_val;
       }
       element_ptr++;
   }

    return max;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               min_vector_element
 *
 * Returns the smallest element of a vector
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
 *     max_vector_element
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/
double min_vector_element(const Vector* vp)
{
    int i;
    double min;
    double* element_ptr;


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
 *                               max_vector_element
 *
 * Returns the largest element of a vector
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
 *     min_vector_element
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/
double max_vector_element(const Vector* vp)
{
    int i;
    double max;
    double* element_ptr;


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

/* =============================================================================
 *                               get_min_vector_element
 *
 * Finds the smallest element of a vector
 *
 * This routine finds the smallest element in the vector specified by "vp", and
 * puts it into *min_ptr. The index of the element is returned. If you are only
 * interested in the index, then min_ptr can be set to NULL.
 *
 * Returns:
 *     The index of the smallest vector element, or ERROR if there are problems.
 *
 * Related:
 *     get_max_vector_element, min_vector_element, max_vector_element
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_min_vector_element
(
    const Vector* vp  /* Pointer to input Vector.   */,
    double* min_ptr  /* Pointer to min to be found */
)
{
    int   i;
    double  min;
    double* element_ptr;
    int   min_index;


    if (vp->length == 0)
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    element_ptr = vp->elements;

    min = *element_ptr;
    min_index = 0;

    element_ptr++;

    for (i = 1; i<vp->length; i++)
    {
       if (*element_ptr < min)
       {
           min = *element_ptr;
           min_index = i;
       }
       element_ptr++;
   }

    if (min_ptr != NULL) *min_ptr = min;

    return min_index;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               get_max_vector_element
 *
 * Finds the largest element of a vector
 *
 * This routine finds the largest element in the vector specified by "vp", and
 * puts it into *min_ptr. The index of the element is returned.  If you are only
 * interested in the index, then min_ptr can be set to NULL.
 *
 * Returns:
 *     The index of the largest vector element, or ERROR if there are problems.
 *
 * Related:
 *     get_min_vector_element, min_vector_element, max_vector_element
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_max_vector_element
(
    const Vector* vp  /* Pointer to input Vector.   */,
    double* max_ptr   /* Pointer to max to be found */
)
{
    int   i;
    double  max;
    double* element_ptr;
    int   max_index;


    if (vp->length == 0)
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
 *                            max_thresh_vector
 *
 * Limits vector elements to a maximum value
 *
 * This routine sets all elements greater than the specified threshold to the
 * threshold value. Elements in the vector specified by the pointer to a Vector
 * object "vp" are compared to the threshold value "max". All elements greater
 * than "max" are set equal to "max". The result is placed into the vector
 * specified by "target_vpp", leaving the contents of "vp" unchanged.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     ow_max_thresh_vector, min_thresh_vector
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/

int max_thresh_vector
(
    Vector** target_vpp  /* Double pointer to output Vector. */,
    const Vector* source_vp /* Pointer to input Vector.         */,
    double max   /* Maximum threshold value.         */
)
{
    Vector* target_vp;
    int     i;
    int     length;


    ERE(get_target_vector(target_vpp, source_vp->length));
    target_vp = *target_vpp;

    length = source_vp->length;

    for (i=0; i<length; i++)
    {
        if ((source_vp->elements)[i] > max)
        {
            (target_vp->elements)[i] = max;
        }
        else
        {
            (target_vp->elements)[i] = (source_vp->elements)[i];
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            min_thresh_vector
 *
 * Limits vector elements to a minimum value
 *
 * This routine sets all elements less than the specified threshold to the
 * threshold value. Elements in the vector specified by the pointer to a Vector
 * object "vp" are compared to the threshold value "min". All elements less
 * than "min" are set equal to "min". The result is placed into the vector
 * specified by "target_vpp", leaving the contents of "vp" unchanged.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     ow_min_thresh_vector, max_thresh_vector
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/

int min_thresh_vector
(
    Vector** target_vpp /* Double pointer to output Vector. */,
    const Vector* source_vp /* Pointer to input Vector.         */,
    double min_val  /* Minimum threshold value.         */
)
{
    Vector* target_vp;
    int     i;
    int     length;


    ERE(get_target_vector(target_vpp, source_vp->length));
    target_vp = *target_vpp;

    length = source_vp->length;

    for (i=0; i<length; i++)
    {
        if ((source_vp->elements)[i] < min_val)
        {
            (target_vp->elements)[i] = min_val;
        }
        else
        {
            (target_vp->elements)[i] = (source_vp->elements)[i];
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            ow_max_thresh_vector
 *
 * Limits vector elements to a maximum value
 *
 * This routine sets all elements greater than the specified threshold to the
 * threshold value. Elements in the vector specified by the pointer to a Vector
 * object "vp" are compared to the threshold value "max". All elements greater
 * than "max" are set equal to "max". The result is placed back into the vector
 * specified by "vp", overwriting the original contents.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     max_thresh_vector, ow_min_thresh_vector
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/

int ow_max_thresh_vector
(
    Vector* source_vp  /* Pointer to input Vector. Contents overwritten. */,
    double max         /* Maximum threshold value.                       */
)
{
    int i;
    int length;


    length = source_vp->length;

    for (i=0; i<length; i++)
    {
        if ((source_vp->elements)[i] > max)
        {
            (source_vp->elements)[i] = max;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            ow_min_thresh_vector
 *
 * Limits vector elements to a minimum value
 *
 * This routine sets all elements less than the specified threshold to the
 * threshold value. Elements in the vector specified by the pointer to a Vector
 * object "vp" are compared to the threshold value "min". All elements less
 * than "min" are set equal to "min". The result is placed back into the vector
 * specified by "vp", overwriting the original contents.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     min_thresh_vector, ow_max_thresh_vector
 *
 * Index: vectors, vector normalization
 *
 * -----------------------------------------------------------------------------
*/

int ow_min_thresh_vector
(
    Vector* source_vp /* Pointer to input Vector. Contents overwitten. */,
    double min_val    /* Minimum threshold value.                      */
)
{
    int     i;
    int     length;


    length = source_vp->length;

    for (i=0; i<length; i++)
    {
        if ((source_vp->elements)[i] < min_val)
        {
            (source_vp->elements)[i] = min_val;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              ow_normalize_log_prob_vp
 *
 * Normalizes a vector of log probs.
 *
 * This routine replaces a vector of log probabilities so that the sum in
 * non-log space is one. It is done in a way that reduces problems due to the
 * fact that the quantities actually represented in the input might be very
 * small.  The output is also log quantities.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Related:
 *     normalize_vector, ow_normalize_vector, normalize_vector_2,
 *     ow_normalize_vector_2
 *
 * Index: vectors, vector normalization, probability
 *
 * -----------------------------------------------------------------------------
*/

int ow_normalize_log_prob_vp(Vector* log_vp)
{
    int       i;
    const int len = log_vp->length;
    double    max = DBL_MOST_NEGATIVE;
    double    sum = 0.0;


    for (i = 0; i < len; i++)
    {
        ASSERT_IS_NUMBER_DBL(log_vp->elements[ i ]); 
        ASSERT_IS_FINITE_DBL(log_vp->elements[ i ]); 
        max = MAX_OF(max, log_vp->elements[ i ]);
    }

    for (i = 0; i < len; i++)
    {
        double temp = exp(log_vp->elements[ i ] - max);

        sum += temp;
        log_vp->elements[ i ] = temp;
    }

    for (i = 0; i < len; i++)
    {
        double temp = log_vp->elements[ i ];

        log_vp->elements[ i ] = SAFE_LOG(temp/sum);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                      ow_exp_scale_by_sum_log_vector
 *
 * Scales the elements of a vector in log space
 *
 * This routine scales a vector of log quantities by the sum of the non log
 * quantities, replacing the vector with non log quantities. This is a standard
 * normalization in probablistic calculations. The log of the sum is returned
 * (which typically maps onto a log likelihood of a mixture marginalization).
 *
 * Returns:
 *    log(sum(ew_exp(vp))   [ "ew" means element-wise ].
 *
 * Index: matrices, matrix arithmetic, probability
 *
 * -----------------------------------------------------------------------------
 */

double ow_exp_scale_by_sum_log_vector(Vector* log_vp)
{
    int length = log_vp->length;
    int j;
    double    max = DBL_MOST_NEGATIVE;
    double    sum = 0.0;
    double log_sum;


    for (j = 0; j < length; j++)
    {
        max = MAX_OF(max, log_vp->elements[ j ]);
    }

    for (j = 0; j < length; j++)
    {
        double temp = exp(log_vp->elements[ j ] - max);
        sum += temp;
    }
    
    log_sum = log(sum);

    for (j = 0; j < length; j++)
    {
        log_vp->elements[ j ] = exp(log_vp->elements[ j ]  - max - log_sum);
    }

#ifdef TEST
    sum = 0.0;

    for (j = 0; j < length; j++)
    {
        sum += log_vp->elements[ j ];
    }

    ASSERT(ABS_OF(sum - 1.0) < 0.00001);
#endif

    return log_sum + max;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

