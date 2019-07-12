
/* $Id: x_vector.c 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 2003-2008 by Kobus Barnard (author).
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

#include "x/x_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "x/x_vector.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                 complex_get_vector_element_magnitudes
 *
 * Computes magnitudes of the elements of a complex vector
 *
 * This routine computes the magnitude of the elements of a complex vector
 * represented as a pair of vectors. All non-null vectors must be the same
 * size. If one of a pair of vectors is NULL, then it is treated as zero. If
 * both vectors of a pair are NULL, then the result is two NULL vectors.
 *
 * The first argument is a pointer to target a vector. If it is null, then a
 * vector of the appropriate size is created. If it is of the wrong size, it is
 * resized. Finally, if they are the right size, then the storage is recycled,
 * as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: vectors, vector arithmetic, complex numbers
 *
 * -----------------------------------------------------------------------------
 */

int complex_get_vector_element_magnitudes
(
    Vector**      out_mpp,
    const Vector* in_re_mp,
    const Vector* in_im_mp
)
{
    int     length, i;
    double* re_ptr;
    double* im_ptr;
    double* out_ptr;
    double  re, im;

    if (in_im_mp == NULL)
    {
        return copy_vector(out_mpp, in_re_mp);
    }
    else if (in_re_mp == NULL)
    {
        return copy_vector(out_mpp, in_im_mp);
    }

    length = in_re_mp->length;

    if (in_im_mp->length != length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_vector(out_mpp, length));

    out_ptr = (*out_mpp)->elements;
    re_ptr = in_re_mp->elements;
    im_ptr = in_im_mp->elements;

    for (i = 0; i < length; i++)
    {
        re = *re_ptr;
        im = *im_ptr;

        *out_ptr = sqrt(re*re + im*im);

        ASSERT_IS_NUMBER_DBL(*out_ptr);
        ASSERT_IS_FINITE_DBL(*out_ptr);

        out_ptr++;
        re_ptr++;
        im_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

