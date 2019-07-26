
/* $Id: i_matrix.c 15377 2013-09-19 21:23:28Z predoehl $ */

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
#include "i/i_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                             matrix_vector_to_image
 *
 * Converts a matrix vector to an image
 *
 * This routine converts a matrix vector to an image. The length of the matrix
 * vector must either be 1 (black and white) or 3 (RGB color).  The result is
 * put into (*out_ipp) which is created if it is NULL, and resized if it is the
 * wrong size.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images, matrices
 *
 * -----------------------------------------------------------------------------
*/

int matrix_vector_to_image(const Matrix_vector* mvp, KJB_image** out_ipp)
{
    Matrix* mp_list[ 3 ];


    mp_list[ 0 ] = mvp->elements[ 0 ];

    if (mvp->length == 1)
    {
        mp_list[ 1 ] = mvp->elements[ 0 ];
        mp_list[ 2 ] = mvp->elements[ 0 ];
    }
    else if (mvp->length == 3)
    {
        mp_list[ 1 ] = mvp->elements[ 1 ];
        mp_list[ 2 ] = mvp->elements[ 2 ];
    }
    else
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(rgb_matrix_array_to_image(mp_list, out_ipp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_to_matrix_vector
 *
 * Converts an image to a matrix vector of length 3.
 *
 * This routine converts an image to a matrix vector of length 3. If any the
 * matrices vector is NULL, then they it is created. If it is not of length 3,
 * it is resized. Similarly, if any of the matrices in the matrix vector are
 * NULL, they are created. Also, if any are the wrong size, they are resized.
 * Otherwise, the storage is recycled as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images, matrices
 *
 * -----------------------------------------------------------------------------
*/

int image_to_matrix_vector(const KJB_image* ip, Matrix_vector** mvpp)
{
    int      i, j, num_rows, num_cols;
    Pixel*   ip_row_pos;
    double*  r_pos;
    double*  g_pos;
    double*  b_pos;

    ERE(get_target_matrix_vector(mvpp, 3));

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i=0; i<3; i++)
    {
        ERE(get_target_matrix(&((*mvpp)->elements[ i ]), num_rows, num_cols));
    }

    for (i=0; i<num_rows; i++)
    {
        ip_row_pos = ip->pixels[ i ];

        r_pos = (*mvpp)->elements[ 0 ]->elements[ i ];
        g_pos = (*mvpp)->elements[ 1 ]->elements[ i ];
        b_pos = (*mvpp)->elements[ 2 ]->elements[ i ];

        for (j=0; j<num_cols; j++)
        {
            *r_pos++ = ip_row_pos->r;
            *g_pos++ = ip_row_pos->g;
            *b_pos++ = ip_row_pos->b;

            ip_row_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             rgb_matrix_array_to_image
 *
 * Converts an array of 3 matrices to an image
 *
 * This routine makes an image from an array of 3 matrices, one each for R, G,
 * and B. The result is put into (*out_ipp) which is created if it is NULL,
 * and resized if it is the wrong size.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images, matrices
 *
 * -----------------------------------------------------------------------------
*/

int rgb_matrix_array_to_image(Matrix* mp_list[ 3 ], KJB_image** out_ipp)
{
    KJB_image* out_ip;
    int        num_rows, num_cols, i, j;
    Pixel*     out_ip_row_pos;
    double*    r_pos;
    double*    g_pos;
    double*    b_pos;


    num_rows = mp_list[ 0 ]->num_rows;
    num_cols = mp_list[ 0 ]->num_cols;

    if (    (num_rows < 1)
         || (num_cols < 1)
         || (mp_list[ 1 ]->num_rows != num_rows)
         || (mp_list[ 2 ]->num_rows != num_rows)
         || (mp_list[ 1 ]->num_cols != num_cols)
         || (mp_list[ 2 ]->num_cols != num_cols)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_rows; i++)
    {
        out_ip_row_pos = out_ip->pixels[ i ];

        r_pos = mp_list[ 0 ]->elements[ i ];
        g_pos = mp_list[ 1 ]->elements[ i ];
        b_pos = mp_list[ 2 ]->elements[ i ];

        for (j=0; j<num_cols; j++)
        {
            out_ip_row_pos->r = *r_pos++;
            out_ip_row_pos->g = *g_pos++;
            out_ip_row_pos->b = *b_pos++;

            out_ip_row_pos->extra.invalid.r     = VALID_PIXEL;
            out_ip_row_pos->extra.invalid.g     = VALID_PIXEL;
            out_ip_row_pos->extra.invalid.b     = VALID_PIXEL;
            out_ip_row_pos->extra.invalid.pixel = VALID_PIXEL;

            out_ip_row_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_to_rgb_matrix_array
 *
 * Converts an image to an array of 3 matrices
 *
 * This routine converts an image to an array of 3 matrices, one each for R, G,
 * and B. If any of the matrices are NULL, then they are created. Also, if any
 * are the wrong size, they are resized. Otherwise, the storage is recycled as
 * is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images, matrices
 *
 * -----------------------------------------------------------------------------
*/

int image_to_rgb_matrix_array(const KJB_image* ip, Matrix* mp_list[ 3 ])
{
    Matrix** mp_list_pos;
    int      i, j, num_rows, num_cols;
    Pixel*   ip_row_pos;
    double*  r_pos;
    double*  g_pos;
    double*  b_pos;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    ASSERT(num_rows > 0);
    ASSERT(num_cols > 0);

    mp_list_pos = mp_list;

    /*
    // FIX ( Slight possibility of memory leak on failure.
    */
    for (i=0; i<3; i++)
    {
        ERE(get_target_matrix(mp_list_pos, num_rows, num_cols));
        mp_list_pos++;
    }

    for (i=0; i<num_rows; i++)
    {
        ip_row_pos = ip->pixels[ i ];

        r_pos = mp_list[ 0 ]->elements[ i ];
        g_pos = mp_list[ 1 ]->elements[ i ];
        b_pos = mp_list[ 2 ]->elements[ i ];

        for (j=0; j<num_cols; j++)
        {
            *r_pos++ = ip_row_pos->r;
            *g_pos++ = ip_row_pos->g;
            *b_pos++ = ip_row_pos->b;

            ip_row_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        matrix_to_bw_image
 *
 * Converts a matrix to a black and white image
 *
 * This routine converts a matrix to a black and white image.
 * For best results on typical hardware or image formats, the input matrix
 * should contain values in the range 0 to 255.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images, matrices
 *
 * -----------------------------------------------------------------------------
*/

int matrix_to_bw_image
(
    const Matrix* mp,
    KJB_image**   out_ipp
)
{
#define MODULAR_IMPLEMENTATION

#ifdef MODULAR_IMPLEMENTATION

    return rgb_matrices_to_image(mp, mp, mp, out_ipp);

#else
    /* Faster, but do we care? */

    KJB_image* out_ip;
    int        num_rows, num_cols, i, j;
    Pixel*     out_ip_row_pos;
    double*    pos;


    UNTESTED_CODE();

    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    for (i=0; i < num_rows; i++)
    {
        out_ip_row_pos = out_ip->pixels[ i ];

        pos = mp->elements[ i ];

        for (j=0; j<num_cols; j++)
        {
            out_ip_row_pos->r = *pos;
            out_ip_row_pos->g = *pos;
            out_ip_row_pos->b = *pos;

            out_ip_row_pos->extra.invalid.r     = VALID_PIXEL;
            out_ip_row_pos->extra.invalid.g     = VALID_PIXEL;
            out_ip_row_pos->extra.invalid.b     = VALID_PIXEL;
            out_ip_row_pos->extra.invalid.pixel = VALID_PIXEL;

            pos++;
            out_ip_row_pos++;
        }
    }

    return NO_ERROR;

#endif

}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      matrix_to_max_contrast_8bit_bw_image
 *
 * Convert matrix to an image, linearly scaling image from black to white
 *
 * This routine accepts a matrix and turns it into a grayscale image, with
 * the lowest-value matrix entry generating a pixel with R=G=B=0, and the
 * highest-value matrix entry generating a pixel with R=G=B=255, scaling all
 * other values linearly within that range.  This assumes the matrix is not
 * constant-valued.
 *
 * If the input matrix is a constant, the output image will have constant
 * value R=G=B=128, since that seems reasonable (albeit hacky).
 * I think this also might occur if the input contains NaN -- not sure.
 *
 * Author: Andrew Predoehl
 *
 * Documentor: Andrew Predoehl
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images, matrices
 *
 * -----------------------------------------------------------------------------
*/

int matrix_to_max_contrast_8bit_bw_image
(
    const Matrix* mp,
    KJB_image**   out_ipp
)
{
    int iii, jjj, rc;
    double hi, lo, *row;
    Matrix *m_scaled = NULL;
    double scale = 0, offset = 128.0;

    ERE(get_max_matrix_element(mp, &hi, NULL, NULL));
    ERE(get_min_matrix_element(mp, &lo, NULL, NULL));

    if (hi > lo)
    {
        /* common case (violated by constant matrix or degenerate values). */
        scale = 255.0 / (hi - lo);
        offset = 0;
    }

    /* Do the matrix copy after the above steps, because it allocates. */
    ERE(copy_matrix(&m_scaled, mp));

    for (iii = 0; iii < mp -> num_rows; ++iii)
    {
        row = m_scaled -> elements[ iii ];
        for (jjj = 0; jjj < mp -> num_cols; ++jjj, ++row)
        {
            *row = (*row - lo) * scale + offset;
        }
    }

    rc = matrix_to_bw_image(m_scaled, out_ipp);
    free_matrix(m_scaled);
    return rc;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             rgb_matrices_to_image
 *
 * Converts 3 matrices to an image
 *
 * This routine converts 3 matrices, one each for R, G, and B, to an image. All
 * matrices must be of the same size. They cannot be null.
 * The matrix values are expected to lie in the customary range for pixel
 * channels.  For example, if you plan to use the image with 8-bit RGB channel
 * values, then your input matrices must likewise lie in the 0-255 value
 * range.  
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images, matrices
 *
 * -----------------------------------------------------------------------------
*/

int rgb_matrices_to_image
(
    const Matrix* r_mp,
    const Matrix* g_mp,
    const Matrix* b_mp,
    KJB_image**   out_ipp
)
{
    KJB_image* out_ip;
    int        num_rows, num_cols, i, j;
    Pixel*     out_ip_row_pos;
    double*    r_pos;
    double*    g_pos;
    double*    b_pos;


    num_rows = r_mp->num_rows;
    num_cols = r_mp->num_cols;

    if (    (num_rows < 0)
         || (num_cols < 0)
         || (g_mp->num_rows != num_rows)
         || (g_mp->num_rows != num_rows)
         || (b_mp->num_cols != num_cols)
         || (b_mp->num_cols != num_cols)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    for (i=0; i < num_rows; i++)
    {
        out_ip_row_pos = out_ip->pixels[ i ];

        r_pos = r_mp->elements[ i ];
        g_pos = g_mp->elements[ i ];
        b_pos = b_mp->elements[ i ];

        for (j=0; j<num_cols; j++)
        {
            out_ip_row_pos->r = *r_pos++;
            out_ip_row_pos->g = *g_pos++;
            out_ip_row_pos->b = *b_pos++;

            out_ip_row_pos->extra.invalid.r     = VALID_PIXEL;
            out_ip_row_pos->extra.invalid.g     = VALID_PIXEL;
            out_ip_row_pos->extra.invalid.b     = VALID_PIXEL;
            out_ip_row_pos->extra.invalid.pixel = VALID_PIXEL;

            out_ip_row_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  image_to_matrix
 *
 * Converts an image to a matrix
 *
 * This routine converts an image to a single matrix. The image is converted to
 * black and white by simple averaging of the pixels. This is not the standard
 * method which de-emphasizes blue. Further, the possiblity that the image is
 * not linear is ignored. Likely some for these considerations will be
 * implemenated as options at some point, but will require moving this routine
 * to i_color.c so that the options can be shared with other routines that
 * convert color images to black and white. 
 *
 * If the matrix pointed to is NULL, then it is created.  If any are the wrong
 * size, they is is resized.  Otherwise, the storage is recycled as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images, matrices
 *
 * -----------------------------------------------------------------------------
*/

int image_to_matrix
(
    const KJB_image* ip,
    Matrix**         mpp
)
{
    return image_to_matrix_2 ( ip, 1./3, 1./3, 1./3, mpp);
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  image_to_matrix_2
 *
 * Converts an image to a matrix
 *
 * This routine converts an image to a single matrix. The image is converted to
 * black and white by weighted averaging of the pixels.  
 *
 * If the matrix pointed to is NULL, then it is created.  If any are the wrong
 * size, they is is resized.  Otherwise, the storage is recycled as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images, matrices
 *
 * -----------------------------------------------------------------------------
*/

int image_to_matrix_2
(
    const KJB_image* ip,
    double           r_weight,
    double           g_weight,
    double           b_weight,
    Matrix**         mpp
)
{
    int           num_rows, num_cols, i, j;
    double*       out_pos;
    Pixel*        in_pos;
    const Matrix* mp;

    double sum = r_weight + g_weight + b_weight;
    r_weight /= sum;
    g_weight /= sum;
    b_weight /= sum;


    if (ip == NULL)
    {
        if (mpp != NULL)
        {
            free_matrix(*mpp);
            *mpp = NULL; 
        }

        return NO_ERROR;
    }

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    ERE(get_target_matrix(mpp, num_rows, num_cols));
    mp = *mpp;

    for (i=0; i<num_rows; i++)
    {
        in_pos = ip->pixels[ i ];
        out_pos = mp->elements[ i ]; 

        for (j=0; j<num_cols; j++)
        {
            if (in_pos->extra.invalid.pixel != VALID_PIXEL)
            {
                (*out_pos = DBL_MISSING);
            }
            else
            {
                *out_pos = (r_weight * in_pos->r + g_weight * in_pos->g + b_weight * in_pos->b);
            }

            in_pos++;
            out_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         bw_image_to_matrix
 *
 * Converts an black and white image to a matrix
 *
 * This routine converts an image to a single matrix. The image is assumed to be
 * black and white (i.e., all channels are equal).  If the matrix pointed to is
 * NULL, then it is created.  If any are the wrong size, they is is resized.
 * Otherwise, the storage is recycled as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images, matrices
 *
 * -----------------------------------------------------------------------------
*/

int bw_image_to_matrix
(
    const KJB_image* ip,
    Matrix**         mpp
)
{

    return image_to_rgb_matrices(ip, mpp, NULL, NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_to_rgb_matrices
 *
 * Converts an image to 3 matrices
 *
 * This routine converts an image to 3 matrices, one each for R, G, and B. If
 * any of the pointers to the 3 matrices are null, that channel is not returned.
 * If any of the matrices pointed to are NULL, then they are created.  If any
 * are the wrong size, they are resized. Otherwise, the storage is recycled as
 * is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images, matrices
 *
 * -----------------------------------------------------------------------------
*/

int image_to_rgb_matrices
(
    const KJB_image* ip,
    Matrix**         r_mpp,
    Matrix**         g_mpp,
    Matrix**         b_mpp
)
{
    Pixel*  ip_row_pos;
    double* r_pos;
    double* g_pos;
    double* b_pos;
    int     i, j, num_rows, num_cols;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    ASSERT(num_rows > 0);
    ASSERT(num_cols > 0);


    if (r_mpp != NULL)
    {
        ERE(get_target_matrix(r_mpp, num_rows, num_cols));
    }

    if (g_mpp != NULL)
    {
        ERE(get_target_matrix(g_mpp, num_rows, num_cols));
    }

    if (b_mpp != NULL)
    {
        ERE(get_target_matrix(b_mpp, num_rows, num_cols));
    }

    if ((r_mpp != NULL) && (g_mpp != NULL) && (b_mpp != NULL))
    {
        for (i=0; i<num_rows; i++)
        {
            ip_row_pos = ip->pixels[ i ];

            r_pos = (*r_mpp)->elements[ i ];
            g_pos = (*g_mpp)->elements[ i ];
            b_pos = (*b_mpp)->elements[ i ];

            for (j=0; j<num_cols; j++)
            {
                *r_pos++ = ip_row_pos->r;
                *g_pos++ = ip_row_pos->g;
                *b_pos++ = ip_row_pos->b;

                ip_row_pos++;
            }
        }
    }
    else
    {
        if (r_mpp != NULL)
        {
            for (i=0; i<num_rows; i++)
            {
                ip_row_pos = ip->pixels[ i ];
                r_pos = (*r_mpp)->elements[ i ];

                for (j=0; j<num_cols; j++)
                {
                    *r_pos++ = ip_row_pos->r;
                    ip_row_pos++;
                }
            }
        }

        if (g_mpp != NULL)
        {
            for (i=0; i<num_rows; i++)
            {
                ip_row_pos = ip->pixels[ i ];
                g_pos = (*g_mpp)->elements[ i ];

                for (j=0; j<num_cols; j++)
                {
                    *g_pos++ = ip_row_pos->g;
                    ip_row_pos++;
                }
            }
        }

        if (b_mpp != NULL)
        {
            for (i=0; i<num_rows; i++)
            {
                ip_row_pos = ip->pixels[ i ];
                b_pos = (*b_mpp)->elements[ i ];

                for (j=0; j<num_cols; j++)
                {
                    *b_pos++ = ip_row_pos->b;
                    ip_row_pos++;
                }
            }
        }
    }

    return NO_ERROR;


}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_rgb_matrices(Matrix* r_mp, Matrix* g_mp, Matrix* b_mp)
{


    free_matrix(r_mp);
    free_matrix(g_mp);
    free_matrix(b_mp);

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_rgb_matrix_array(Matrix* mp_list[ 3 ])
{


    free_matrix(mp_list[ 0 ]);
    free_matrix(mp_list[ 1 ]);
    free_matrix(mp_list[ 2 ]);

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

