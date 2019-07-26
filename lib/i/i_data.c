
/* $Id: i_data.c 5831 2010-05-02 21:52:24Z ksimek $ */

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

#include "i/i_gen.h"     /* Only safe as first include in a ".c" file. */
#include "i/i_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                              get_image_pixels
 *
 * Puts all valid pixels into an Nx3 array.
 *
 * This routine puts all valid image pixels into an Nx3 array (one row for each
 * pixel). If the target matrix *mpp is NULL, then it is created. If it is
 * the wrong size, it is resized. Finally, if it is the right size, then the
 * storage is recycled as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images
 *
 * -----------------------------------------------------------------------------
*/

int get_image_pixels(const KJB_image* ip, Matrix** mpp)
{
    int i, j;
    int count;
    int num_rows, num_cols;
    long    max_num_pixels;
    Pixel*  in_pos;
    Matrix* mp;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    max_num_pixels = num_rows * num_cols;

    ERE(get_target_matrix(mpp, max_num_pixels, 3));
    mp = *mpp;

    count = 0;

    for (i=0; i<num_rows; i++)
    {
        in_pos  = ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            if ( ! in_pos->extra.invalid.pixel )
            {
                mp->elements[ count ][ 0 ] = in_pos->r;
                mp->elements[ count ][ 1 ] = in_pos->g;
                mp->elements[ count ][ 2 ] = in_pos->b;

                ASSERT(mp->elements[ count ][ 0 ] >= 0.0);
                ASSERT(mp->elements[ count ][ 1 ] >= 0.0);
                ASSERT(mp->elements[ count ][ 2 ] >= 0.0);

                count++;
            }

            in_pos++;
        }
    }

    mp->num_rows = count;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              get_registered_image_pixels
 *
 * Puts registered valid pixels from 2 images into arrays
 *
 * This selects all pixels locations which have valid pixels in both the input
 * images and puts the pixels for each of the input images into the
 * corresponding output matrices. The two matrices thus will have the same
 * number of rows. If either of the target matrices *mp1_ptr or *mp2_ptr are
 * NULL, then they are created. If either are the wrong size, then they are
 * resized. Finally, if either are the right size, then the storage is recycled
 * as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images
 *
 * -----------------------------------------------------------------------------
*/


int get_registered_image_pixels
(
    const KJB_image* ip1,
    const KJB_image* ip2,
    Matrix**         mp1_ptr,
    Matrix**         mp2_ptr,
    int**            invalid_registered_ptr
)
{
    int i, j;
    int count;
    int num_rows, num_cols;
    long   max_num_pixels;
    Pixel* in_pos1;
    Pixel* in_pos2;


    ERE(check_same_size_image(ip1, ip2));

    num_rows = ip1->num_rows;
    num_cols = ip1->num_cols;

    max_num_pixels = num_rows * num_cols;

    ERE(get_target_matrix(mp1_ptr, max_num_pixels, 3));
    ERE(get_target_matrix(mp2_ptr, max_num_pixels, 3));
    NRE(*invalid_registered_ptr = INT_MALLOC(max_num_pixels));

    count = 0;

    for (i=0; i<num_rows; i++)
    {
        in_pos1  = ip1->pixels[ i ];
        in_pos2  = ip2->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            if ( ! in_pos1->extra.invalid.pixel )
            {
                (*mp1_ptr)->elements[ count ][ 0 ] = in_pos1->r;
                (*mp1_ptr)->elements[ count ][ 1 ] = in_pos1->g;
                (*mp1_ptr)->elements[ count ][ 2 ] = in_pos1->b;

                (*mp2_ptr)->elements[ count ][ 0 ] = in_pos2->r;
                (*mp2_ptr)->elements[ count ][ 1 ] = in_pos2->g;
                (*mp2_ptr)->elements[ count ][ 2 ] = in_pos2->b;

                if ( ! in_pos2->extra.invalid.pixel )
                {
                    (*invalid_registered_ptr)[ count ] = FALSE;
                }
                else
                {
                    (*invalid_registered_ptr)[ count ] = TRUE;
                }

                count++;
            }

            in_pos1++;
            in_pos2++;
        }
    }

    (*mp1_ptr)->num_rows = count;
    (*mp2_ptr)->num_rows = count;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

