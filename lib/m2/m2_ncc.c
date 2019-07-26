
/* $Id: m2_ncc.c 9835 2011-06-29 22:11:53Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard & Quanfu Fan (author).
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

#include "m/m_incl.h"

#include "i/i_float.h"

#ifdef KJB_HAVE_FFTW
#    include "fftw3.h"
#endif

#include "m2/m2_ncc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define UPPER_LEFT_CENTER  0
#define LOWER_RIGHT_CENTER 1

/* -------------------------------------------------------------------------- */

#ifdef KJB_HAVE_FFTW
static int padding_matrix_to_fttw_vector
(
    fftw_complex** fftw_out,
    const Matrix*  mp,
    int            pad_num_rows,
    int            pad_num_cols
);

static int padding_image_channel_to_fttw_vector
(
    fftw_complex** fftw_out,
    const KJB_image*   mp,
    int            channel,
    int            pad_num_rows,
    int            pad_num_cols
);

static int flip_matrix
(
    Matrix**      out_mpp,
    const Matrix* mp
);

static int multiply_fftw
(
    fftw_complex** outp,
    fftw_complex*  f1,
    fftw_complex*  f2,
    int            nx,
    int            ny
);

static int build_sum_table
(
    Matrix**      out_mpp,
    const Matrix* mp
);

static int retrieve_fragment_sum
(
    Matrix* const mp,
    int     row_offset,
    int     col_offset,
    int     num_rows,
    int     num_cols,
    double* sum
);

static int build_square_sum_table
(
    Matrix**      out_mpp,
    const Matrix* mp
);

static int fourier_correlation_matrix
(
    Matrix**      out_mpp,
    const Matrix* in_mp,
    const Matrix* mask_mp
);

static int fourier_convolve_mat
(
    Matrix**      out_mpp,
    const Matrix* in_mp,
    const Matrix* mask_mp,
    int           cpos
);

static int fourier_convolve_img_channel
(
    KJB_image* in_mp,
    const Matrix* mask_mp,
    int           cpos
);

static int fftw_convolve(
        fftw_complex** out_complex,
        fftw_complex* in_complex,
        fftw_complex* mask_complex,
        int pad_num_rows,
        int pad_num_cols);
#endif

/* -------------------------------------------------------------------------- */

#ifdef KJB_HAVE_FFTW
static int fourier_convolve_mat
(
    Matrix**      out_mpp,
    const Matrix* in_mp,
    const Matrix* mask_mp,
    int           cpos
)
{
    fftw_complex* inverse_fftw    = NULL;
    fftw_complex* in_complex      = NULL;
    fftw_complex* mask_complex    = NULL;
    Matrix*       out_mp          = NULL;
    int           num_rows, num_cols;
    int           mask_rows;
    int           mask_cols;
    int           pad_num_rows;
    int           pad_num_cols;
    int           i, j, mi, mj;
    int           num;
    int           index;
    int           mask_row_offset;
    int           mask_col_offset;

    mask_rows = mask_mp->num_rows;
    mask_cols = mask_mp->num_cols;
    num_rows = in_mp->num_rows;
    num_cols = in_mp->num_cols;
    ASSERT(mask_rows <= num_rows);
    ASSERT(mask_cols <= num_cols);

    /*expand the mask matrix and feed it into a 'fftw_complex' data structure.*/
    pad_num_rows = mask_rows + num_rows - 1;
    pad_num_cols = mask_cols + num_cols - 1;
    num = pad_num_rows * pad_num_cols;
    EGC(padding_matrix_to_fttw_vector(&in_complex, in_mp, pad_num_rows,
                                      pad_num_cols));

    /* CUT HERE */
    EGC(padding_matrix_to_fttw_vector(&mask_complex, mask_mp, pad_num_rows,
                                      pad_num_cols));

    /* the big shebang */
    fftw_convolve(&inverse_fftw, in_complex, mask_complex, pad_num_rows, pad_num_cols);

    EGC(get_target_matrix(out_mpp, num_rows, num_cols));
    out_mp = *out_mpp;

    /* fill in the output matrix */
    if(cpos == UPPER_LEFT_CENTER)
    {
        mask_row_offset = mask_mp->num_rows / 2;
        mask_col_offset = mask_mp->num_cols / 2;
    }
    else
    {
        mask_row_offset = (mask_mp->num_rows - 1)  / 2;
        mask_col_offset = ( mask_mp->num_cols - 1) / 2;
    }

    /* CUT HERE  */
    for (i=0; i<num_rows; i++)
    {
        for(j=0; j<num_cols; j++)
        {
            mi = mask_row_offset + i;
            mj = mask_col_offset + j;
            index = mi * pad_num_cols + mj;
            out_mp->elements[i][j] = inverse_fftw[index][0]/num;
        }
    }

cleanup:
    fftw_free(in_complex);
    fftw_free(mask_complex);
    fftw_free(inverse_fftw);

    return NO_ERROR;
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef KJB_HAVE_FFTW
static int fourier_convolve_img_channel
(
    KJB_image* img_ip,
    const Matrix* mask_mp,
    int           cpos
)
{
    int err = NO_ERROR;
    fftw_complex* inverse_fftw    = NULL;
    fftw_complex* in_complex      = NULL;
    fftw_complex* mask_complex    = NULL;

    int           num_rows, num_cols;
    int           mask_rows;
    int           mask_cols;
    int           pad_num_rows;
    int           pad_num_cols;

    int           i, j, mi, mj;
    int           channel;
    int           index;
    int           mask_row_offset;
    int           mask_col_offset;

    int num;

    mask_rows = mask_mp->num_rows;
    mask_cols = mask_mp->num_cols;
    num_rows = img_ip->num_rows;
    num_cols = img_ip->num_cols;
    ASSERT(mask_rows <= num_rows);
    ASSERT(mask_cols <= num_cols);

    /*expand the mask matrix and feed it into a 'fftw_complex' data structure.*/
    pad_num_rows = mask_rows + num_rows - 1;
    pad_num_cols = mask_cols + num_cols - 1;
    num = pad_num_rows * pad_num_cols;
    EGC( err = padding_matrix_to_fttw_vector(&mask_complex, mask_mp, pad_num_rows,
              pad_num_cols));


    /* fill in the output matrix */
    if(cpos == UPPER_LEFT_CENTER)
    {
        mask_row_offset = mask_mp->num_rows / 2;
        mask_col_offset = mask_mp->num_cols / 2;
    }
    else
    {
        mask_row_offset = (mask_mp->num_rows - 1)  / 2;
        mask_col_offset = ( mask_mp->num_cols - 1) / 2;
    }

    /* the big shebang */
    for(channel = 0; channel < 3; channel++)
    {
        EGC( err = padding_image_channel_to_fttw_vector(&in_complex, img_ip, channel, pad_num_rows,
                pad_num_cols) );
        EGC( err = fftw_convolve(&inverse_fftw, in_complex, mask_complex, pad_num_rows, pad_num_cols) );


        for (i=0; i<num_rows; i++)
        {
            for(j=0; j<num_cols; j++)
            {
                float* pix_channel;

                mi = mask_row_offset + i;
                mj = mask_col_offset + j;
                index = mi * pad_num_cols + mj;

                switch(channel)
                {
                    case 0:
                        pix_channel = &img_ip->pixels[i][j].r;
                        break;
                    case 1:
                        pix_channel = &img_ip->pixels[i][j].g;
                        break;
                    case 2:
                        pix_channel = &img_ip->pixels[i][j].b;
                        break;
                    default:
                        set_error("Channel must be 0, 1, or 2");
                        err = ERROR;
                        goto cleanup;
                }

                *pix_channel = inverse_fftw[index][0]/num;
            }
        }
    }

cleanup:
    fftw_free(in_complex);
    fftw_free(mask_complex);
    fftw_free(inverse_fftw);

    return err;
}
#endif

#ifdef KJB_HAVE_FFTW
static int fftw_convolve(
        fftw_complex** out_complex,
        fftw_complex* in_complex,
        fftw_complex* mask_complex,
        int pad_num_rows,
        int pad_num_cols)
{
    fftw_complex* in_fftw         = NULL;
    fftw_complex* mask_fftw       = NULL;
    fftw_complex* product_fftw    = NULL;

    int num = pad_num_rows * pad_num_cols;
    fftw_plan     p;

    /* perform fourier transform */
    mask_fftw = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * num);
    p = fftw_plan_dft_2d(pad_num_rows, pad_num_cols, mask_complex, mask_fftw,
                         FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(p);

    fftw_destroy_plan(p);

    in_fftw = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * num);


    p = fftw_plan_dft_2d(pad_num_rows, pad_num_cols, in_complex, in_fftw,
                         FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(p); /* repeat as needed */

    fftw_destroy_plan(p);

    /* multiply the fourier-transformed results */
    multiply_fftw(&product_fftw, in_fftw, mask_fftw, pad_num_rows, pad_num_cols);

    /* invert the the product obtained above */
    *out_complex = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * num);
    p = fftw_plan_dft_2d(pad_num_rows, pad_num_cols, product_fftw, *out_complex,
                         FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_execute(p); /* repeat as needed */

    fftw_destroy_plan(p);

    fftw_free(in_fftw);
    fftw_free(mask_fftw);
    fftw_free(product_fftw);

    return NO_ERROR;
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            fourier_convolve_matrix
 *
 * Convolve matrix with an arbitrary mask by using fourier transform.
 *
 * This routine use the fourier transform method to convolve the matrix pointed
 * to by in_mp with the mask mask_mp, putting the result into *out_mpp. When the
 * size of the mask matrix is large, the routine is highly recommended when the
 * mask size is large.  Unlike the routine 'convolve_matrix',
 * 'fourier_convolve_matrix' deals with the boundary by padding 0, rather than
 * by wrapping around the 'in_matrix'.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: convolution
 *
 * Author: Quanfu Fan and Kobus Barnard
 *
 * Documentor: Quanfu Fan and Kobus Barnard
 *
 * -----------------------------------------------------------------------------
 */

#ifdef KJB_HAVE_FFTW
int fourier_convolve_matrix
(
    Matrix**      out_mpp,
    const Matrix* in_mp,
    const Matrix* mask_mp
)
{
    return fourier_convolve_mat(out_mpp, in_mp, mask_mp, UPPER_LEFT_CENTER);
}
#else
int fourier_convolve_matrix
(
    Matrix**      __attribute__((unused)) dummy_out_mpp,
    const Matrix* __attribute__((unused)) dummy_in_mp,
    const Matrix* __attribute__((unused)) dummy_mask_mp
)
{
    set_error("DFT facility not available.");
    add_error("Likely the program was compiled without fftw.");
    return ERROR;
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            fourier_convolve_image
 *
 * Convolve an image with an arbitrary mask by using fourier transform.
 *
 * This routine use the fourier transform method to convolve the image pointed
 * to by in_mp with the mask mask_mp, putting the result into *out_ipp. When the
 * size of the mask image is large, the routine is highly recommended when the
 * image size is large.  Unlike the routine 'convolve_image',
 * 'fourier_convolve_image' deals with the boundary by padding 0, rather than
 * by wrapping around the 'in_image'.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: convolution
 *
 * Author: Kyle Sime, Quanfu Fan and Kobus Barnard
 *
 * Documentor: Kyle Simek, Quanfu Fan and Kobus Barnard
 *
 * -----------------------------------------------------------------------------
 */

#ifdef KJB_HAVE_FFTW
int fourier_convolve_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    const Matrix* mask_mp
)
{
    int channel;

    ERE(kjb_copy_image(out_ipp, in_ip));

    /* convolve with each channel */
    /* TODO: do this loop inside of fourier_convolve_img, to avoid extra malloc/frees */
    ERE(fourier_convolve_img_channel(*out_ipp, mask_mp, UPPER_LEFT_CENTER));


    return NO_ERROR;
}
#else
int fourier_convolve_image
(
    KJB_image**      __attribute__((unused)) dummy_out_mpp,
    const KJB_image* __attribute__((unused)) dummy_in_mp,
    const Matrix* __attribute__((unused)) dummy_mask_mp
)
{
    set_error("DFT facility not available.");
    add_error("Likely the program was compiled without fftw.");
    return ERROR;
}
#endif
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef KJB_HAVE_FFTW
static int multiply_fftw
(
    fftw_complex** outp,
    fftw_complex*  f1,
    fftw_complex*  f2,
    int            nx,
    int            ny
)
{
    int num;
    int i;
    fftw_complex *out;

    num = nx * ny;

    if(*outp == NULL)
    {
        (*outp) = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * num);
    }
    out = *outp;

    for(i=0; i<num; i++)
    {
        out[i][0] = f1[i][0] * f2[i][0] - f1[i][1] * f2[i][1];
        out[i][1] = f1[i][1] * f2[i][0] + f1[i][0] * f2[i][1];
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int retrieve_fragment_sum
(
    Matrix* const mp,
    int     row_offset,
    int     col_offset,
    int     target_num_rows,
    int     target_num_cols,
    double* sum
)
{
    int end_row, end_col;

    ASSERT(row_offset < mp->num_rows);
    ASSERT(col_offset < mp->num_cols);

    end_row = row_offset + target_num_rows - 1;
    if(end_row >= mp->num_rows)
        end_row = mp->num_rows - 1;
    end_col = col_offset + target_num_cols - 1;
    if(end_col  >= mp->num_cols)
        end_col = mp->num_cols - 1;

    if(row_offset <= 0)
    {
        if(col_offset <= 0)
            *sum = mp->elements[end_row][end_col];
        else
            *sum = mp->elements[end_row][end_col] - mp->elements[end_row][col_offset-1];
    }
    else if(col_offset <= 0)
    {
        *sum = mp->elements[end_row][end_col] - mp->elements[row_offset-1][end_col];
    }
    else
    {
        *sum = mp->elements[end_row][end_col] - mp->elements[row_offset-1][end_col] -
            mp->elements[end_row][col_offset-1] + mp->elements[row_offset-1][col_offset-1];
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int build_sum_table(Matrix **out_mpp, const Matrix *mp)
{
    int i, j;
    int num_rows, num_cols;
    Matrix *out_mp;

    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    ERE(get_target_matrix(out_mpp, num_rows, num_cols));
    out_mp = *out_mpp;

    out_mp->elements[0][0] = mp->elements[0][0];
    for(i=1; i<num_rows; i++)
        out_mp->elements[i][0] = out_mp->elements[i-1][0] + mp->elements[i][0];
    for(i=1; i<num_cols; i++)
        out_mp->elements[0][i] = out_mp->elements[0][i-1] + mp->elements[0][i];

    for(i=1; i<num_rows; i++)
    {
        for(j=1; j<num_cols; j++)
        {
            out_mp->elements[i][j] = out_mp->elements[i][j-1] + out_mp->elements[i-1][j] -
                out_mp->elements[i-1][j-1] + mp->elements[i][j];
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int build_square_sum_table(Matrix **out_mpp, const Matrix *mp)
{
    int i, j;
    int num_rows, num_cols;
    Matrix *out_mp;

    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    ERE(get_target_matrix(out_mpp, num_rows, num_cols));
    out_mp = *out_mpp;

    out_mp->elements[0][0] = mp->elements[0][0] * mp->elements[0][0];
    for(i=1; i<num_rows; i++)
        out_mp->elements[i][0] = out_mp->elements[i-1][0] + mp->elements[i][0] * mp->elements[i][0];
    for(i=1; i<num_cols; i++)
        out_mp->elements[0][i] = out_mp->elements[0][i-1] + mp->elements[0][i] * mp->elements[0][i];

    for(i=1; i<num_rows; i++)
    {
        for(j=1; j<num_cols; j++)
        {
            out_mp->elements[i][j] = out_mp->elements[i][j-1] + out_mp->elements[i-1][j] -
                out_mp->elements[i-1][j-1] + mp->elements[i][j] * mp->elements[i][j];
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int padding_matrix_to_fttw_vector
(
    fftw_complex** fftw_out,
    const Matrix*  mp,
    int            pad_num_rows,
    int            pad_num_cols
)
{
    int i, j;
    int num_rows = mp->num_rows;
    int num_cols = mp->num_cols;
    fftw_complex *out;

    ASSERT(num_rows <= pad_num_rows);
    ASSERT(num_cols <= pad_num_cols);

    if(*fftw_out == NULL)
        *fftw_out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * pad_num_rows * pad_num_cols);
    out = *fftw_out;

    bzero(out, sizeof(fftw_complex) * pad_num_rows * pad_num_cols);

    for(i=0; i<num_rows; i++)
    {
        for(j=0; j<num_cols; j++)
        {
            out[i*pad_num_cols+j][0] = mp->elements[i][j];
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int padding_image_channel_to_fttw_vector
(
    fftw_complex** fftw_out,
    const KJB_image*   mp,
    int            channel,
    int            pad_num_rows,
    int            pad_num_cols
)
{
    int i, j;
    int num_rows = mp->num_rows;
    int num_cols = mp->num_cols;
    fftw_complex *out;

    ASSERT(num_rows <= pad_num_rows);
    ASSERT(num_cols <= pad_num_cols);

    if(*fftw_out == NULL)
        *fftw_out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * pad_num_rows * pad_num_cols);
    out = *fftw_out;

    bzero(out, sizeof(fftw_complex) * pad_num_rows * pad_num_cols);

    /* shouldn't this be centered; i.e. shouldn't there be a gutter on the top and left of the "out" matrix? */
    for(i=0; i<num_rows; i++)
    {
        for(j=0; j<num_cols; j++)
        {
            double color;
            switch(channel)
            {
                case 0:
                    color = mp->pixels[i][j].r;
                    break;
                case 1:
                    color = mp->pixels[i][j].g;
                    break;
                case 2:
                    color = mp->pixels[i][j].b;
                    break;
                default:
                    set_error("channel must be 0, 1, or 2.");
                    break;
            }

            out[i*pad_num_cols+j][0] = color;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int flip_matrix(Matrix **out_mpp, const Matrix *mp)
{
    Matrix *out_mp = NULL;
    int num_rows = mp->num_rows;
    int num_cols = mp->num_cols;
    int i, j;

    ERE(get_target_matrix(out_mpp, num_rows, num_cols));
    out_mp = *out_mpp;

    for(i=0; i<num_rows; i++)
    {
        for(j=0; j<num_cols; j++)
        {
            out_mp->elements[i][j] = mp->elements[num_rows-i-1][num_cols-j-1];
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int fourier_correlation_matrix(Matrix **out_mpp, const Matrix *in_mp,
                                      const Matrix *mask_mp)
{
    Matrix *flip_mp = NULL;

    ERE(flip_matrix(&flip_mp, mask_mp));

    fourier_convolve_mat(out_mpp, in_mp, flip_mp, LOWER_RIGHT_CENTER);

    free_matrix(flip_mp);

    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           fourier_ncc_template_matrix
 *
 * Computes the normalized cross correlation (ncc) of a template pointed to by
 * template_mp with the corresponding patch at each element of a matrix.
 *
 * This routine computes the normalized cross correlation of a template with a
 * matrix. The fourier transform and the running sum techniques are used to
 * speed up the routine. It is highly recommended when the the template size is
 * large. The result is put into *out_mpp.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: convolution
 *
 * Author: Quanfu Fan and Kobus Barnard
 *
 * Documentor: Quanfu Fan and Kobus Barnard
 *
 *
 * -----------------------------------------------------------------------------
 */

#ifdef KJB_HAVE_FFTW
int fourier_ncc_template_matrix
(
    Matrix**      out_mpp,
    const Matrix* in_mp,
    const Matrix* template_mp
)
{
    Matrix *sum_table = NULL;
    Matrix *square_table = NULL;
    Matrix *corr_mp = NULL;
    Matrix *out_mp = NULL;
    int num_rows, num_cols;
    int mask_rows, mask_cols, mask_row_offset, mask_col_offset;
    int i, j, num;
    double tsquare; /*square sum of the values of elements in the template matrix*/
    double tsum; /*sum of the values of elements in the template matrix*/
    double tsigma; /*sum of the squre of the diff between the value of an element and the mean*/
    double ssquare; /*square sum of the values of elements in a patch of in_mp*/
    double ssum; /*sum of the values of elements in a patch of in_mp*/
    double ssigma; /*sum of the squre of the diff between the value of an element and the mean*/

    mask_rows = template_mp->num_rows;
    mask_cols = template_mp->num_cols;
    mask_row_offset = template_mp->num_rows / 2  ;
    mask_col_offset = template_mp->num_cols/ 2;
    num_rows = in_mp->num_rows;
    num_cols = in_mp->num_cols;
    num = mask_rows * mask_cols;

    tsum = 0.0;
    tsquare = 0.0;
    for(i=0; i<mask_rows; i++)
    {
        for(j=0; j<mask_cols; j++)
        {
            tsum += template_mp->elements[i][j];
            tsquare += (template_mp->elements[i][j] * template_mp->elements[i][j]);
        }
    }
    tsigma = sqrt(tsquare - tsum*tsum/num);

    /* create the running sum and running squre sume tables for the lookup purpose*/
    ERE(build_sum_table(&sum_table, in_mp));
    ERE(build_square_sum_table(&square_table, in_mp));

    /* correlate two matrices with the fourier-based covolution*/
    ERE(fourier_correlation_matrix(&corr_mp, in_mp, template_mp));

    ERE(get_target_matrix(out_mpp, num_rows, num_cols));
    out_mp = *out_mpp;

    for(i=0; i<num_rows; i++)
    {
        for(j=0; j<num_cols; j++)
        {
            ERE(retrieve_fragment_sum(sum_table, i-mask_row_offset,
                                      j-mask_col_offset, mask_rows, mask_cols, &ssum));
            ERE(retrieve_fragment_sum(square_table, i-mask_row_offset,
                                      j-mask_col_offset, mask_rows, mask_cols, &ssquare));
            ssigma = sqrt(ssquare - ssum*ssum/num);
            if(fabs(tsigma) < NCC_EPSLON || fabs(ssigma) < NCC_EPSLON )
                out_mp->elements[i][j] = 0;
            else
            {
                out_mp->elements[i][j] = corr_mp->elements[i][j] - tsum*ssum/num;
                out_mp->elements[i][j] /= (tsigma * ssigma);
            }

        }
    }

    free_matrix(sum_table);
    free_matrix(square_table);
    free_matrix(corr_mp);

    return NO_ERROR;
}
#else
int fourier_ncc_template_matrix
(
    Matrix**      __attribute__((unused)) dummy_out_mpp,
    const Matrix* __attribute__((unused)) dummy_in_mp,
    const Matrix* __attribute__((unused)) dummy_template_mp
)
{
    set_error("DFT facility not available.");
    add_error("Likely the program was compiled without fftw.");

    return ERROR;
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          fourier_ncc_template_mvector
 *
 * Computes the normalized cross correlation (ncc) of a template matrix vector
 * pointed to by *template_mvp with the corresponding patch at each element of
 * a matrix vector.
 *
 * This routine computes the normalized cross correlation of a template matrix
 * vector with another matrix vector. It's desirable when we want to compute
 * the normalized correlation over RGB instead of only over the intensity.
 * The fourier transform and the running sum techniques are used
 * to speed up the routine. The result is put into *out_mpp.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: convolution
 *
 * Author: Quanfu Fan and Kobus Barnard
 *
 * Documentor: Quanfu Fan and Kobus Barnard
 *
 * -----------------------------------------------------------------------------
 */

#ifdef KJB_HAVE_FFTW
int fourier_ncc_template_mvector
(
    Matrix**             out_mpp,
    const Matrix_vector* in_mvp,
    const Matrix_vector* template_mvp
)
{
    Matrix_vector *sum_table = NULL;
    Matrix_vector *square_table = NULL;
    Matrix_vector *corr_mp = NULL;
    Matrix *out_mp = NULL;
    int num_rows, num_cols;
    int mask_rows, mask_cols, mask_row_offset, mask_col_offset;
    int i, j, k, num;
    double *tsquare, *tsum, tsigma;
    double *ssquare, *ssum, ssigma;
    double tot_tsum, tot_tsquare, tot_ssum, tot_ssquare, tot_corr;
    int length;

    length = in_mvp->length;
    ASSERT(length == template_mvp->length);
    for(i=0; i<length; i++)
    {
        ASSERT(template_mvp->elements[i]->num_rows <= in_mvp->elements[i]->num_rows);
        ASSERT(template_mvp->elements[i]->num_cols <= in_mvp->elements[i]->num_cols);
    }

    mask_rows = template_mvp->elements[0]->num_rows;
    mask_cols = template_mvp->elements[0]->num_cols;
    mask_row_offset = mask_rows / 2;
    mask_col_offset = mask_cols / 2;
    num_rows = in_mvp->elements[0]->num_rows;
    num_cols = in_mvp->elements[0]->num_cols;
    num = mask_rows * mask_cols *length;

    tsquare = (double*)malloc(sizeof(double)*length);
    tsum = (double*)malloc(sizeof(double)*length);
    ssquare = (double*)malloc(sizeof(double)*length);
    ssum = (double*)malloc(sizeof(double)*length);

    bzero(tsum, sizeof(double) * length);
    bzero(tsquare, sizeof(double) * length);

    for(k=0; k<length; k++)
    {
        for(i=0; i<mask_rows; i++)
        {
            for(j=0; j<mask_cols; j++)
            {
                tsum[k] += template_mvp->elements[k]->elements[i][j];
                tsquare[k] += (template_mvp->elements[k]->elements[i][j] *
                               template_mvp->elements[k]->elements[i][j]);
            }
        }
    }
    tot_tsquare = 0.0;
    tot_tsum = 0.0;
    for(k=0; k<length; k++)
    {
        tot_tsquare += tsquare[k];
        tot_tsum += tsum[k];
    }

    tsigma = sqrt(tot_tsquare - tot_tsum*tot_tsum/num);

    ERE(get_target_matrix_vector(&sum_table, length));
    ERE(get_target_matrix_vector(&square_table, length));
    for(k=0; k<length; k++)
    {
        ERE(build_sum_table(&(sum_table->elements[k]), in_mvp->elements[k]));
        ERE(build_square_sum_table(&(square_table->elements[k]), in_mvp->elements[k]));
    }

    ERE(get_target_matrix_vector(&corr_mp, length));
    for(k=0; k<length; k++)
        ERE(fourier_correlation_matrix(&(corr_mp->elements[k]), in_mvp->elements[k],
                                       template_mvp->elements[k]));

    ERE(get_target_matrix(out_mpp, num_rows, num_cols));
    out_mp = *out_mpp;

    for(i=0; i<num_rows; i++)
    {
        for(j=0; j<num_cols; j++)
        {
            for(k=0; k<length; k++)
            {
                ERE(retrieve_fragment_sum(sum_table->elements[k], i-mask_row_offset, j-mask_col_offset,
                                          mask_rows, mask_cols, ssum+k));
                ERE(retrieve_fragment_sum(square_table->elements[k], i-mask_row_offset, j-mask_col_offset,
                                          mask_rows, mask_cols, ssquare+k));
            }
            tot_ssquare = 0.0;
            tot_ssum = 0.0;
            tot_corr = 0.0;
            for(k=0; k<length; k++)
            {
                tot_ssquare += ssquare[k];
                tot_ssum += ssum[k];
                tot_corr += corr_mp->elements[k]->elements[i][j];
            }

            ssigma = sqrt(tot_ssquare - tot_ssum*tot_ssum/num);
            if(fabs(tsigma) < NCC_EPSLON || fabs(ssigma) < NCC_EPSLON )
                out_mp->elements[i][j] = 0;
            else
            {
                out_mp->elements[i][j] = tot_corr - tot_tsum*tot_ssum/num;
                out_mp->elements[i][j] /= (tsigma * ssigma);
            }

        }
    }

    free(tsquare);
    free(ssquare);
    free(tsum);
    free(ssum);
    free_matrix_vector(sum_table);
    free_matrix_vector(square_table);
    free_matrix_vector(corr_mp);

    return NO_ERROR;
}
#else
int fourier_ncc_template_mvector
(
    Matrix**             __attribute__((unused)) dummy_out_mpp,
    const Matrix_vector* __attribute__((unused)) dummy_in_mvp,
    const Matrix_vector* __attribute__((unused)) dummy_template_mvp
)
{
    set_error("DFT facility not available.");
    add_error("Likely the program was compiled without fftw.");

    return ERROR;
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                ncc_matrix
 *
 * Computes the normalized cross correlation (ncc) of two matrices
 *
 * This routine computes the normalized cross correlation of two matrices. The
 * two matrix must have the same size.  The result is put into *corr.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: convolution
 *
 * Author: Quanfu Fan and Kobus Barnard
 *
 * Documentor: Quanfu Fan and Kobus Barnard
 *
 * -----------------------------------------------------------------------------
 */

int ncc_matrix(double *corr, const Matrix *mp1, const Matrix *mp2)

{
    int num_rows;
    int num_cols;
    int num;
    int i, j;
    double sum1, sum2, square1, square2, cov, sigma1, sigma2;

    ASSERT(mp1->num_rows == mp2->num_rows);
    ASSERT(mp1->num_cols == mp2->num_cols);

    num_rows = mp1->num_rows;
    num_cols = mp1->num_cols;
    num = num_rows * num_cols;

    sum1 = 0.0;
    sum2 = 0.0;
    square1 = 0.0;
    square2 = 0.0;
    cov = 0.0;
    for(i=0; i<num_rows; i++)
    {
        for(j=0; j<num_cols; j++)
        {
            sum1 += mp1->elements[i][j];
            square1 += (mp1->elements[i][j] * mp1->elements[i][j]);
            sum2 += mp2->elements[i][j];
            square2 += (mp2->elements[i][j] * mp2->elements[i][j]);
            cov += mp1->elements[i][j] * mp2->elements[i][j];
        }
    }

    sigma1 = sqrt(square1 - sum1*sum1/num);
    sigma2 = sqrt(square2 - sum2*sum2/num);
    if( fabs(sigma1) < NCC_EPSLON || fabs(sigma2) < NCC_EPSLON)
    {
        *corr = 0.0;
    }
    else
    {
        *corr = cov - sum1*sum2/num;
        *corr /= (sigma1 * sigma2);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                ncc_mvector
 *
 * Computes the normalized cross correlation (ncc) of two matrix vectors.
 *
 * This routine computes the normalized cross correlation of two matrix vectors.
 * The two matrix vectors must have the same length and their corresponding
 * matrices should have the same size as well. The result is put into *corr.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: convolution
 *
 * Author: Quanfu Fan and Kobus Barnard
 *
 * Documentor: Quanfu Fan and Kobus Barnard
 *
 * -----------------------------------------------------------------------------
 */
int ncc_mvector
(
    double*              corr,
    const Matrix_vector* mvp1,
    const Matrix_vector* mvp2
)
{
    Matrix *mp1 = NULL, *mp2 = NULL;
    int num_rows;
    int num_cols;
    int i, j, k;
#ifdef NOT_USED   /* Kobus */
    double sum1, sum2, square1, square2, cov, sigma1, sigma2;
#endif
    int length;

    ASSERT(mvp1->length == mvp2->length);

    length = mvp1->length;
    for(i=0; i<length; i++)
    {
        ASSERT(mvp1->elements[i]->num_rows == mvp2->elements[i]->num_rows);
        ASSERT(mvp1->elements[i]->num_cols == mvp2->elements[i]->num_cols);
    }

    num_rows = mvp1->elements[0]->num_rows;
    num_cols = mvp1->elements[0]->num_cols;

    ERE(get_target_matrix(&mp1, num_rows * length, num_cols));
    ERE(get_target_matrix(&mp2, num_rows * length, num_cols));
    for(i=0; i<num_rows; i++)
    {
        for(j=0; j<num_cols; j++)
        {
            for(k=0; k<length; k++)
            {
                int index = i + k * num_rows;
                mp1->elements[index][j] = mvp1->elements[k]->elements[i][j];
                mp2->elements[index][j] = mvp2->elements[k]->elements[i][j];
            }
        }
    }

    ERE(ncc_matrix(corr, mp1, mp2));

    free_matrix(mp1);
    free_matrix(mp2);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

