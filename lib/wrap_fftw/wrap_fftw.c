
/* $Id: wrap_fftw.c 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003-2008 by members of University of Arizona Computer Vision|
|  group (the authors) including                                               |
|        Kobus Barnard.                                                        |
|                                                                              |
|  (Copyright only applies to the wrapping code, not the wrapped code).        |
|                                                                              |
|  Personal and educational use of this code is granted, provided that this    |
|  header is kept intact, and that the authorship is not misrepresented, that  |
|  its use is acknowledged in publications, and relevant papers are cited.      |
|                                                                              |
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).         |
|                                                                              |
|  Please note that the code in this file has not necessarily been adequately  |
|  tested. Naturally, there is no guarantee of performance, support, or fitness|
|  for any particular task. Nonetheless, I am interested in hearing about      |
|  problems that you encounter.                                                |
|                                                                              |
* =========================================================================== */

/*
 * This file is for wrappers for the fftw3 library. It needs to be included
 * specifically because it implies construction of that library when we do a
 * "make depend" which we only want to do when it is needed.
 */

#include "m/m_gen.h"
#include "wrap_fftw/wrap_fftw.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


/*
 * IMPORTANT: Each exported routine needs to have a version for both with and
 * withtout FFTW.
*/

#ifdef KJB_HAVE_FFTW

/* -----------------------------------------------------------------------------
|                                 FFTW
|                                  ||
|                                 \||/
|                                  \/
*/

#include "fftw3.h"

static Wrap_fftw_style fs_fftw_style = FFTW_DEFAULT_STYLE;

/* -------------------------------------------------------------------------- */

static int get_matrix_dft_helper
(
    Matrix**      output_re_mpp,
    Matrix**      output_im_mpp,
    const Matrix* input_re_mp,
    const Matrix* input_im_mp,
    int   direction
);

static int get_vector_dft_helper
(
    Vector**      output_re_vpp,
    Vector**      output_im_vpp,
    const Vector* input_re_vp,
    const Vector* input_im_vp,
    int   direction
);

/* -------------------------------------------------------------------------- */

/*
 * =============================================================================
 *                              set_fftw_style
 *
 * Sets the style of transforms wrapped from the fftw library
 *
 * This routine sets the style of transforms from the fftw library. The default
 * behaviour of the wrapped fftw routines is simply to pass back the transform
 * computed by the fftw library. However, it is sometimes more convenient to
 * normalize the transforms, or match what Matlab does. This function sets an
 * internal variable to choose among these. The argument is one of
 * FFTW_DEFAULT_STYLE, FFTW_NORMALIZE_STYLE, or FFTW_MATLAB_STYLE.
 *
 * References:
 *     http://www.fftw.org/
 *
 * Index: fourier transforms, vectors, vector arithmetic, matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

void set_fftw_style(Wrap_fftw_style fftw_style)
{
    fs_fftw_style = fftw_style;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              get_matrix_dct
 *
 * Computes the discrete cosine transform (DCT) of a matrix
 *
 * This routine computes the discrete cosine transform (DCT) of a matrix.
 *
 * The first argument is the adress of the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Note:
 *     This routine simply wraps the appropriate fftw library functions for
 *     convenient use with our our data structures. The fftw library can also be
 *     used directly in conjunction with the kjb_library by including fftw3.h
 *     (the makefile builder should know where to find everthing).  This can be
 *     preferable in some cases where one wants to exploit the tuning mechanism
 *     from that library, and cut down on overhead. However, for ocasional use,
 *     the savings won't be very great.
 *
 * Warning:
 *     The default behaviour of this routine is to inherits fftw's normalization
 *     strategy which is to return unnormalized transforms. This means that
 *     taking the transform, and then inverting, requires division by
 *     N=num_cols*num_rows. This behaviour can be changed using the
 *     set_fftw_style() function. Currently the style setting of
 *     FFTW_MATLAB_STYLE is not valid with DCT because it has not yet been
 *     implemented (on the TODO list).
 *
 * References:
 *     http://www.fftw.org/
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: fourier transforms, vectors, vector arithmetic, matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int get_matrix_dct
(
    Matrix**      output_mpp,
    const Matrix* input_mp
)
{
    int num_rows = input_mp->num_rows;
    int num_cols = input_mp->num_cols;
    fftw_plan p;

    if (fs_fftw_style == FFTW_MATLAB_STYLE)
    {
        set_error("DCT with Matlab style not yet implemented.");
        return ERROR;
    }

    ERE(get_zero_matrix(output_mpp, num_rows, num_cols));

    p = fftw_plan_r2r_2d(num_rows, num_cols,
                         input_mp->elements[ 0 ],
                         (*output_mpp)->elements[ 0 ],
                         FFTW_REDFT10, FFTW_REDFT10,
                         FFTW_ESTIMATE);

    fftw_execute(p);

    if (fs_fftw_style == FFTW_NORMALIZE_STYLE)
    {
        ERE(ow_divide_matrix_by_scalar((*output_mpp),
                                       2.0 * sqrt((double)(num_rows * num_cols))));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              get_matrix_inverse_dct
 *
 * Computes the inverse discrete cosine transform (DCT) of a matrix
 *
 * This routine computes the inverse discrete cosine transform (DCT) of a
 * matrix.
 *
 * The first argument is the adress of the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Note:
 *     This routine simply wraps the appropriate fftw library functions for
 *     convenient use with our our data structures. The fftw library can also be
 *     used directly in conjunction with the kjb_library by including fftw3.h
 *     (the makefile builder should know where to find everthing).  This can be
 *     preferable in some cases where one wants to exploit the tuning mechanism
 *     from that library, and cut down on overhead. However, for ocasional use,
 *     the savings won't be very great.
 *
 * Warning:
 *     The default behaviour of this routine is to inherits fftw's normalization
 *     strategy which is to return unnormalized transforms. This means that
 *     taking the transform, and then inverting, requires division by
 *     N=num_cols*num_rows. This behaviour can be changed using the
 *     set_fftw_style() function. Currently the style setting of
 *     FFTW_MATLAB_STYLE is not valid with DCT because it has not yet been
 *     implemented (on the TODO list).
 *
 * References:
 *     http://www.fftw.org/
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: fourier transforms, vectors, vector arithmetic, matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int get_matrix_inverse_dct
(
    Matrix**      output_mpp,
    const Matrix* input_mp
)
{
    int num_rows = input_mp->num_rows;
    int num_cols = input_mp->num_cols;
    fftw_plan p;

    if (fs_fftw_style == FFTW_MATLAB_STYLE)
    {
        set_error("Inverse DCT with Matlab style not yet implemented.");
        return ERROR;
    }

    ERE(get_zero_matrix(output_mpp, num_rows, num_cols));

    p = fftw_plan_r2r_2d(num_rows, num_cols,
                         input_mp->elements[ 0 ],
                         (*output_mpp)->elements[ 0 ],
                         FFTW_REDFT01, 
                         FFTW_REDFT01,
                         FFTW_ESTIMATE);

    fftw_execute(p);

    if (fs_fftw_style == FFTW_NORMALIZE_STYLE)
    {
        ERE(ow_divide_matrix_by_scalar((*output_mpp),
                                       2.0 * sqrt((double)(num_rows * num_cols))));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              get_matrix_dft
 *
 * Computes the discrete Fourier transform (DFT) of a matrix
 *
 * This routine computes the discrete Fourier transform (DFT) of a paire of
 * matrices representing a complex matrix. The matrix representing the imaginary
 * part can be NULL.
 *
 * The first two argument are pointers to matrices for the real and imaginary
 * parts of the result. If they are null, then matrices of the appropriate sizes
 * are created. If they are the wrong size, they are resized. Finally, if they
 * are the right size, then the storage is recycled, as is.
 *
 * Note:
 *     This routine simply wraps the appropriate fftw library functions for
 *     convenient use with our our data structures. The fftw library can also be
 *     used directly in conjunction with the kjb_library by including fftw3.h
 *     (the makefile builder should know where to find everthing).  This can be
 *     preferable in some cases where one wants to exploit the tuning mechanism
 *     from that library, and cut down on overhead. However, for ocasional use,
 *     the savings won't be very great.
 *
 * Warning:
 *     The default behaviour of this routine is to inherits fftw's normalization
 *     strategy which is to return unnormalized transforms. This means that
 *     taking the transform, and then inverting, requires division by
 *     N=num_cols*num_rows. This behaviour can be changed using the
 *     set_fftw_style() function.
 *
 * References:
 *     http://www.fftw.org/
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: fourier transforms, vectors, vector arithmetic, matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int get_matrix_dft
(
    Matrix**      output_re_mpp,
    Matrix**      output_im_mpp,
    const Matrix* input_re_mp,
    const Matrix* input_im_mp
)
{
    ERE(get_matrix_dft_helper(output_re_mpp, output_im_mpp, input_re_mp,
                              input_im_mp, FFTW_FORWARD));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              get_matrix_inverse_dft
 *
 * Computes the inverse discrete Fourier transform (DFT) of a matrix
 *
 * This routine computes the inverse discrete Fourier transform (DFT) of a paire
 * of matrices representing a complex matrix. The matrix representing the
 * imaginary part can be NULL.
 *
 * The first two argument are pointers to matrices for the real and imaginary
 * parts of the result. If they are null, then matrices of the appropriate sizes
 * are created. If they are the wrong size, they are resized. Finally, if they
 * are the right size, then the storage is recycled, as is.
 *
 * Note:
 *     This routine simply wraps the appropriate fftw library functions for
 *     convenient use with our our data structures. The fftw library can also
 *     be used directly in conjunction with the kjb_library by including
 *     fftw3.h (the makefile builder should know where to find everthing).
 *     This can be preferable in some cases where one wants to exploit the
 *     tuning mechanism from that library, and cut down on overhead. However,
 *     for ocasional use, the savings won't be very great.
 *
 * Warning:
 *     The default behaviour of this routine is to inherits fftw's normalization
 *     strategy which is to return unnormalized transforms. This means that
 *     taking the transform, and then inverting, requires division by
 *     N=num_cols*num_rows. This behaviour can be changed using the
 *     set_fftw_style() function.
 *
 * Options:
 *     Proposed options (not yet implemented): fftw_default, fftw_matlab,
 *     fftw_normalized
 *
 * References:
 *     http://www.fftw.org/
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: fourier transforms, vectors, vector arithmetic, matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int get_matrix_inverse_dft
(
    Matrix**      output_re_mpp,
    Matrix**      output_im_mpp,
    const Matrix* input_re_mp,
    const Matrix* input_im_mp
)
{
    ERE(get_matrix_dft_helper(output_re_mpp, output_im_mpp, input_re_mp,
                              input_im_mp, FFTW_BACKWARD));

    if (    (fs_fftw_style == FFTW_MATLAB_STYLE)
         && (output_re_mpp != NULL)
         && (*output_re_mpp != NULL)
       )
    {
        int num_rows = (*output_re_mpp)->num_rows;
        int num_cols = (*output_re_mpp)->num_cols;

        ERE(ow_divide_matrix_by_scalar((*output_re_mpp),
                                       (double)(num_rows * num_cols)));
    }

    if (    (fs_fftw_style == FFTW_MATLAB_STYLE)
         && (output_im_mpp != NULL)
         && (*output_im_mpp != NULL)
       )
    {
        int num_rows = (*output_im_mpp)->num_rows;
        int num_cols = (*output_im_mpp)->num_cols;

        ERE(ow_divide_matrix_by_scalar((*output_im_mpp),
                                       (double)(num_rows * num_cols)));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_matrix_dft_helper
(
    Matrix**      output_re_mpp,
    Matrix**      output_im_mpp,
    const Matrix* input_re_mp,
    const Matrix* input_im_mp,
    int   direction
)
{
    int num_rows = input_re_mp->num_rows;
    int num_cols = input_re_mp->num_cols;
    int n = num_rows * num_cols;
    int i, j;
    fftw_complex* in;
    fftw_complex* out;
    fftw_plan p;
    double* out_re_pos;
    double* out_im_pos;
    fftw_complex *in_pos;
    fftw_complex *out_pos;
    int count = 0;

    NRE(in = N_TYPE_MALLOC(fftw_complex, n));
    NRE(out = N_TYPE_MALLOC(fftw_complex, n));

    in_pos = in;
    out_pos = out;


    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            in_pos[ count ][ 0 ] = input_re_mp->elements[ i ][ j ];

            if (input_im_mp != NULL)
            {
                in_pos[ count ][ 1 ] = input_im_mp->elements[ i ][ j ];
            }
            else
            {
                in_pos[ count ][ 1 ] = 0.0;
            }

            count++;
        }
    }

    p = fftw_plan_dft_2d(num_rows, num_cols,
                         in, out, direction, FFTW_ESTIMATE);

    fftw_execute(p);

    ERE(get_zero_matrix(output_re_mpp, num_rows, num_cols));
    out_re_pos = (*output_re_mpp)->elements[ 0 ];

    for (i = 0; i < n; i++)
    {
        *out_re_pos = out_pos[ i ][ 0 ];
        out_re_pos++;
    }

    if (fs_fftw_style == FFTW_NORMALIZE_STYLE)
    {
        ERE(ow_divide_matrix_by_scalar((*output_re_mpp),
                                       sqrt((double)(num_rows * num_cols))));
    }

    if (output_im_mpp != NULL)
    {
        ERE(get_zero_matrix(output_im_mpp, num_rows, num_cols));
        out_im_pos = (*output_im_mpp)->elements[ 0 ];

        for (i = 0; i < n; i++)
        {
            *out_im_pos = out_pos[ i ][ 1 ];
            out_im_pos++;
        }

        if (fs_fftw_style == FFTW_NORMALIZE_STYLE)
        {
            ERE(ow_divide_matrix_by_scalar((*output_im_mpp),
                                           sqrt((double)(num_rows * num_cols))));
        }
    }

    kjb_free(out);
    kjb_free(in);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              get_vector_dft
 *
 * Computes the discrete Fourier transform (DFT) of a vector
 *
 * This routine computes the discrete Fourier transform (DFT) of a paire of
 * matrices representing a complex vector. The vector representing the imaginary
 * part can be NULL.
 *
 * The first two argument are pointers to matrices for the real and imaginary
 * parts of the result. If they are null, then matrices of the appropriate sizes
 * are created. If they are the wrong size, they are resized. Finally, if they
 * are the right size, then the storage is recycled, as is.
 *
 * Note:
 *     This routine simply wraps the appropriate fftw library functions for
 *     convenient use with our our data structures. The fftw library can also
 *     be used directly in conjunction with the kjb_library by including
 *     fftw3.h (the makefile builder should know where to find everthing).
 *     This can be preferable in some cases where one wants to exploit the
 *     tuning mechanism from that library, and cut down on overhead. However,
 *     for ocasional use, the savings won't be very great.
 *
 * Warning:
 *     The default behaviour of this routine is to inherits fftw's normalization strategy which is to return
 *     unnormalized transforms. This means that taking the transform, and then
 *     inverting, requires division by N=num_cols*num_rows. This behaviour can
 *     be changed using the set_fftw_style() function.
 *
 * References:
 *     http://www.fftw.org/
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: fourier transforms, vectors, vector arithmetic, matrices, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int get_vector_dft
(
    Vector**      output_re_vpp,
    Vector**      output_im_vpp,
    const Vector* input_re_vp,
    const Vector* input_im_vp
)
{
    ERE(get_vector_dft_helper(output_re_vpp, output_im_vpp, input_re_vp,
                              input_im_vp, FFTW_FORWARD));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_vector_dft_helper
(
    Vector**      output_re_vpp,
    Vector**      output_im_vpp,
    const Vector* input_re_vp,
    const Vector* input_im_vp,
    int   direction
)
{
    int length = input_re_vp->length;
    int i;
    fftw_complex* in;
    fftw_complex* out;
    fftw_plan p;
    double* out_re_pos;
    double* out_im_pos;
    fftw_complex *in_pos;
    fftw_complex *out_pos;
    int count = 0;

    NRE(in = N_TYPE_MALLOC(fftw_complex, length));
    NRE(out = N_TYPE_MALLOC(fftw_complex, length));

    in_pos = in;
    out_pos = out;

    for (i = 0; i < length; i++)
    {
        in_pos[ count ][ 0 ] = input_re_vp->elements[ i ];

        if (input_im_vp != NULL)
        {
            in_pos[ count ][ 1 ] = input_im_vp->elements[ i ];
        }
        else
        {
            in_pos[ count ][ 1 ] = 0.0;
        }

        count++;
    }

    p = fftw_plan_dft_1d(length, in, out, direction, FFTW_ESTIMATE);

    fftw_execute(p);

    ERE(get_zero_vector(output_re_vpp, length));
    out_re_pos = (*output_re_vpp)->elements;

    for (i = 0; i < length; i++)
    {
        *out_re_pos = out_pos[ i ][ 0 ];
        out_re_pos++;
    }

    if (fs_fftw_style == FFTW_NORMALIZE_STYLE)
    {
        ERE(ow_divide_vector_by_scalar((*output_re_vpp),
                                       sqrt((double)length)));
    }

    if (output_im_vpp != NULL)
    {
        ERE(get_zero_vector(output_im_vpp, length));
        out_im_pos = (*output_im_vpp)->elements;

        for (i = 0; i < length; i++)
        {
            *out_im_pos = out_pos[ i ][ 1 ];
            out_im_pos++;
        }

        if (fs_fftw_style == FFTW_NORMALIZE_STYLE)
        {
            ERE(ow_divide_vector_by_scalar((*output_im_vpp),
                                           sqrt((double)length )));
        }
    }

    kjb_free(out);
    kjb_free(in);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#else

/* -----------------------------------------------------------------------------
|                               no FFTW
|                                  ||
|                                 \||/
|                                  \/
*/

static void set_dont_have_fftw_error(void)
{
    set_error("Operation failed because the program was built without the ");
    add_error("fftw libraries readily available.");
    add_error("Appropriate installation, file manipulation and re-compiling ");
    add_error("is needed to fix this.)");
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void set_fftw_style(Wrap_fftw_style __attribute__((unused)) dummy_fftw_style)
{

    set_dont_have_fftw_error();
    return;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


int get_matrix_dct
(
    Matrix**      __attribute__((unused)) dummy_output_mpp,
    const Matrix* __attribute__((unused)) dummy_input_mp
)
{

    set_dont_have_fftw_error();
    return ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_matrix_inverse_dct
(
    Matrix**      __attribute__((unused)) dummy_output_mpp,
    const Matrix* __attribute__((unused)) dummy_input_mp
)
{

    set_dont_have_fftw_error();
    return ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_matrix_dft
(
    Matrix**      __attribute__((unused)) dummy_output_re_mpp,
    Matrix**      __attribute__((unused)) dummy_output_im_mpp,
    const Matrix* __attribute__((unused)) dummy_input_re_mp,
    const Matrix* __attribute__((unused)) dummy_input_im_mp
)
{

    set_dont_have_fftw_error();
    return ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_matrix_inverse_dft
(
    Matrix**      __attribute__((unused)) dummy_output_re_mpp,
    Matrix**      __attribute__((unused)) dummy_output_im_mpp,
    const Matrix* __attribute__((unused)) dummy_input_re_mp,
    const Matrix* __attribute__((unused)) dummy_input_im_mp
)
{

    set_dont_have_fftw_error();
    return ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_vector_dft
(
    Vector**      __attribute__((unused)) dummy_output_re_vpp,
    Vector**      __attribute__((unused)) dummy_output_im_vpp,
    const Vector* __attribute__((unused)) dummy_input_re_vp,
    const Vector* __attribute__((unused)) dummy_input_im_vp
)
{
    set_dont_have_fftw_error();
    return ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/*
|                                  /\
|                                 /||\
|                                  ||
|                               no FFTW
----------------------------------------------------------------------------- */

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

