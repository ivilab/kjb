/* =============================================================================
 * The functions in this file are meant to offer a number of ways to homogenize,
 * vectors. All functions add error messages as appropriate, but
 * leave error-handling to the caller of the function. Vectors with a z-value of
 * zero will cause errors.
 *
 * Function Summary:
 *    ow_homogenize_vector - changes the values of the vector directly,
 *        leaving z = 1.0. Returns ERROR or NO_ERROR as appropriate.
 *    homogenize_vector - places the homogenized values into a given Vector.
 *    Returns ERROR or NO_ERROR as appropriate.
 *
 * Author: Andrew Emmott (aemmott)
 *
 * -------------------------------------------------------------------------- */

/*Includes*/
#include <g/g_homogeneous_point.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 *                                 ow_homogenize_vector
 *
 * Normalizes the values in the given Vector.
 *
 * Changes the values of the vector directly, leaving z = 1.0. Returns ERROR or
 * NO_ERROR as appropriate.
 *
 * Returns:
 *   ERROR if the z-value = 0.0.
 *   NO_ERROR otherwise.
 *
 * Related:
 *    homogenize_vector
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: homogenize, normalization, homogeneous
 * -------------------------------------------------------------------------- */
int ow_homogenize_vector
(
    Vector *vector_to_homogenize /*The Vector to be changed.*/
)
{
    if(vector_to_homogenize == NULL)
    {
        G_HOMOGENEOUS_POINT_NULL_ERROR;
        return ERROR;
    }
    ERE(ow_homogenize_vector_at_index(vector_to_homogenize,vector_to_homogenize->length-1));
    return NO_ERROR;
}

int ow_homogenize_vector_at_index
(
    Vector *vector_to_homogenize, /*The Vector to be changed.*/
    int index
)
{

    /*Error Checking*/
    /*Bad or null vector.*/
    if (vector_to_homogenize == NULL || index < 0 || index >= vector_to_homogenize->length)
    {
        G_HOMOGENEOUS_POINT_NULL_ERROR;
        return ERROR;
    }

    /*z-value of 0*/
    if (fabs(vector_to_homogenize->elements[index]) == 0.0)
    {
        G_HOMOGENEOUS_POINT_Z_ERROR;
        return ERROR;
    }
    /*End Error Checking*/

    /*Divide all other elements by the z-value.*/
    ERE(ow_multiply_vector_by_scalar(vector_to_homogenize,1.0/vector_to_homogenize->elements[index]));

    return NO_ERROR;
}

/* =============================================================================
 *                                 homogenize_vector
 *
 * Normalizes the values in a vector and places them in a new vector. 
 *
 * Takes a Vector pointer that contains the values to homogenize, and a Vector**
 * that is used to allocate a new Vector of the same size. The homogenized values
 * are placed inside the new Vector. If the Vector** points to an existing
 * Vector, it will be completely overwritten, including its size if necessary.
 * Returns ERROR or NO_ERROR as appropriate.
 *
 * Returns:
 *   ERROR if the z-value = 0.0.
 *   ERROR if the call to get_target_vector fails.
 *   NO_ERROR otherwise.
 *
 * Related:
 *    ow_homogenize_vector
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: homogenize, normalization, homogeneous
 * -------------------------------------------------------------------------- */
int homogenize_vector
(
    Vector **homogenized_vector_address, /*The Vector pointer address to set with the homogenized values.*/
    const Vector *vector_to_homogenize /*The Vector whose values to homogenize.*/
)
{
    /*Error Checking*/
    /*Bad or null Vector.*/
    if (vector_to_homogenize == NULL)
    {
        G_HOMOGENEOUS_POINT_NULL_ERROR;
        return ERROR;
    }
    ERE(homogenize_vector_at_index(homogenized_vector_address,vector_to_homogenize,vector_to_homogenize->length-1));
    return NO_ERROR;
}

int homogenize_vector_at_index
(
    Vector **homogenized_vector_address, /*The Vector pointer address to set with the homogenized values.*/
    const Vector *vector_to_homogenize, /*The Vector whose values to homogenize.*/
    int index
)
{

    /*Error Checking*/
    /*Bad or null Vector.*/
    if (vector_to_homogenize == NULL || homogenized_vector_address == NULL || index < 0 || index >= vector_to_homogenize->length)
    {
        G_HOMOGENEOUS_POINT_NULL_ERROR;
        return ERROR;
    }

    if (fabs(vector_to_homogenize->elements[index]) == 0.0)
    {
        G_HOMOGENEOUS_POINT_Z_ERROR;
        return ERROR;
    }

    /*End Error Checking*/

    /*Insert the new values, divided by the z-value.*/
    ERE(multiply_vector_by_scalar(homogenized_vector_address,vector_to_homogenize,1.0/vector_to_homogenize->elements[index]));

    return NO_ERROR;
}

#ifdef __cplusplus
}
#endif

