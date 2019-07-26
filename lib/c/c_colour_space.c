
/* $Id: c_colour_space.c 4819 2009-11-20 18:22:08Z kobus $ */

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

#include "c/c_gen.h"     /* Only safe as first include in a ".c" file. */

#include "c/c_colour_space.h"

#ifdef __cplusplus
extern "C" {
#endif
 
/* -------------------------------------------------------------------------- */

#ifndef DATA_DIR
#    define DATA_DIR "data"
#endif

#ifndef MONITOR_DIR
#    define MONITOR_DIR "monitor"
#endif

#ifndef DEFAULT_RGB_TO_XYZ_MATRIX_FILE
#    define DEFAULT_RGB_TO_XYZ_MATRIX_FILE "rgb_to_xyz.mat"
#endif

/* -------------------------------------------------------------------------- */

static char    fs_rgb_to_xyz_file[ MAX_FILE_NAME_SIZE ] = { '\0' };
static Matrix* fs_rgb_to_xyz_mp    = NULL;

/* -------------------------------------------------------------------------- */

static int set_rgb_to_xyz_matrix(void);
static int read_rgb_to_xyz_matrix_from_file(const char* file_name);
static int set_builtin_rgb_to_xyz_matrix(void);

#ifdef TRACK_MEMORY_ALLOCATION
    static void free_allocated_static_data(void);
    static void prepare_memory_cleanup(void);
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                          set_image_colour_space_options
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int set_colour_space_options(const char *option, const char *value)
{
    char lc_option[ 100 ];
    int  result = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "rgb-to-xyz-file"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("Under construction\n"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(set_rgb_to_xyz_matrix());
            ERE(pso("Image RGB are converted to XYZ using matrix in %s.\n",
                     fs_rgb_to_xyz_file));
        }
        else
        {
            return read_rgb_to_xyz_matrix_from_file(value);
        }

        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/*
 * =============================================================================
 *                                  
 *
 *
 *
 *
 * -----------------------------------------------------------------------------
 */


int print_colour_space_options(void)
{

    ERE(set_rgb_to_xyz_matrix());

    ERE(pso("Image RGB are converted to XYZ using matrix in %s.\n",
             fs_rgb_to_xyz_file));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                       get_rgb_to_xyz_matrix
 *
 *
 *
 *
 * -----------------------------------------------------------------------------
 */


int get_rgb_to_xyz_matrix(Matrix **mpp)
{

    ERE(set_rgb_to_xyz_matrix());

    return copy_matrix(mpp, fs_rgb_to_xyz_mp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      convert_matrix_rgb_to_lab
 *
 * Converts matrix RGB to L*a*b.
 *
 * This routine converts matrix RGB to L*a*b. This conversion uses an RGB to XYZ
 * matrix. If KJB library options are being made available to the user, then
 * this matrix can be set using the option "rgb-to-xyz-file".  If no conversion
 * file has been set, than a default one is used. If no default is available,
 * then sRGB is used. The contents of the third argument, white_rgb_vp, is used
 * as the RGB of the white point. If it is NULL, then RGB=(255,255,255) is used.
 *
 * The output is put into *out_mpp. If *out_mpp is NULL, then it is
 * created. If it is the wrong size, then it is resized. If it is the right
 * size, the storage is recycled as is.
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure, with an error message being set.
 *
 * Index: images, RGB colour, L*a*b colour, colour spaces
 *
 * -----------------------------------------------------------------------------
*/

int convert_matrix_rgb_to_lab
(
    Matrix**      out_mpp,
    const Matrix* in_mp,
    const Vector* white_rgb_vp
)
{
    Vector* reciprocal_of_white_xyz_vp = NULL;
    int     num_rows, i;


    ERE(get_reciprocal_of_white_xyz(&reciprocal_of_white_xyz_vp,
                                    white_rgb_vp));

    ERE(convert_matrix_rgb_to_xyz(out_mpp, in_mp));

    /*
    // Make matrix of ( X/Xn, Y/Yn, Z/Zn ).
    */
    ERE(ow_multiply_matrix_by_row_vector_ew(*out_mpp,
                                          reciprocal_of_white_xyz_vp));

    free_vector(reciprocal_of_white_xyz_vp);

    num_rows = (*out_mpp)->num_rows;

    for (i=0; i<num_rows; i++)
    {
        double f_x = LAB_F((*out_mpp)->elements[ i ][ 0 ]);
        double f_y = LAB_F((*out_mpp)->elements[ i ][ 1 ]);
        double f_z = LAB_F((*out_mpp)->elements[ i ][ 2 ]);

        (*out_mpp)->elements[ i ][ 0 ] = 116.0 * f_y - 16.0;
        (*out_mpp)->elements[ i ][ 1 ] = 500 * (f_x - f_y);
        (*out_mpp)->elements[ i ][ 2 ] = 200 * (f_y - f_z);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        convert_matrix_rgb_to_xyz
 *
 * Converts matrix RGB to XYZ
 *
 * This routine converts the matrix RGB to XYZ based on an RGB to XYZ matrix.
 * If KJB library options are being made available to the user, then
 * this matrix can be set using the option "rgb-to-xyz-file".  If no conversion
 * file has been set, than a default one is used. If no default is available,
 * then sRGB is used.
 *
 * The output is put into *out_mpp. If *out_mpp is NULL, then it is
 * created. If it is the wrong size, then it is resized. If it is the right
 * size, the storage is recycled.
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure, with an error message being set.
 *
 * Index: images, RGB colour, XYZ colour, colour spaces
 *
 * -----------------------------------------------------------------------------
*/

int convert_matrix_rgb_to_xyz(Matrix **out_mpp, const Matrix *in_mp)
{

    ERE(set_rgb_to_xyz_matrix());

    ERE(multiply_by_transpose(out_mpp, in_mp, fs_rgb_to_xyz_mp));

    if (min_matrix_element(*out_mpp) < -DBL_EPSILON)
    {
        verbose_pso(30, "Conversion from RGB to XYZ required truncation ");
        verbose_pso(30, "at zero.\n");
    }

    return ow_min_thresh_matrix(*out_mpp, 0.0);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             convert_vector_rgb_to_lab
 *
 * Converts vector RGB to L*a*b.
 *
 * This routine converts vector RGB to L*a*b. This conversion uses an RGB to XYZ
 * vector. If KJB library options are being made available to the user, then
 * this vector can be set using the option "rgb-to-xyz-file".  If no conversion
 * file has been set, than a default one is used. If no default is available,
 * then sRGB is used. The contents of the third argument, white_rgb_vp, is used
 * as the RGB of the white point. If it is NULL, then RGB=(255,255,255) is used.
 *
 * The output is put into *out_vpp. If *out_vpp is NULL, then it is
 * created. If it is the wrong size, then it is resized. If it is the right
 * size, the storage is recycled as is.
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure, with an error message being set.
 *
 * Index: images, RGB colour, L*a*b colour, colour spaces
 *
 * -----------------------------------------------------------------------------
*/

int convert_vector_rgb_to_lab
(
    Vector**      out_vpp,
    const Vector* in_vp,
    const Vector* white_rgb_vp
)
{
    Vector* reciprocal_of_white_xyz_vp = NULL;
    double  f_x, f_y, f_z;


    ERE(get_reciprocal_of_white_xyz(&reciprocal_of_white_xyz_vp,
                                    white_rgb_vp));

    ERE(convert_vector_rgb_to_xyz(out_vpp, in_vp));

    /*
    // Make vector of ( X/Xn, Y/Yn, Z/Zn ).
    */
    ERE(ow_multiply_vectors(*out_vpp, reciprocal_of_white_xyz_vp));

    free_vector(reciprocal_of_white_xyz_vp);

    f_x = LAB_F((*out_vpp)->elements[ 0 ]);
    f_y = LAB_F((*out_vpp)->elements[ 1 ]);
    f_z = LAB_F((*out_vpp)->elements[ 2 ]);

    (*out_vpp)->elements[ 0 ] = 116.0 * f_y - 16.0;
    (*out_vpp)->elements[ 1 ] = 500 * (f_x - f_y);
    (*out_vpp)->elements[ 2 ] = 200 * (f_y - f_z);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_reciprocal_of_white_xyz
(
    Vector**      reciprocal_of_white_xyz_vpp,
    const Vector* white_rgb_vp
)
{
    Vector* standard_white_rgb_vp = NULL;
    Vector* white_xyz_vp = NULL;


    if (white_rgb_vp == NULL)
    {
        ERE(get_initialized_vector(&standard_white_rgb_vp, 3, 255.0));
        white_rgb_vp = standard_white_rgb_vp;
    }

    ERE(convert_vector_rgb_to_xyz(&white_xyz_vp, white_rgb_vp));

    ERE(invert_vector(reciprocal_of_white_xyz_vpp, white_xyz_vp));

    free_vector(white_xyz_vp);
    free_vector(standard_white_rgb_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        convert_vector_rgb_to_xyz
 *
 * Converts vector RGB to XYZ
 *
 * This routine converts the vector RGB to XYZ based on an RGB to XYZ vector.
 * If KJB library options are being made available to the user, then
 * this vector can be set using the option "rgb-to-xyz-file".  If no conversion
 * file has been set, than a default one is used. If no default is available,
 * then sRGB is used.
 *
 * The output is put into *out_vpp. If *out_vpp is NULL, then it is
 * created. If it is the wrong size, then it is resized. If it is the right
 * size, the storage is recycled.
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure, with an error message being set.
 *
 * Index: images, RGB colour, XYZ colour, colour spaces
 *
 * -----------------------------------------------------------------------------
*/

int convert_vector_rgb_to_xyz(Vector **out_vpp, const Vector *in_vp)
{

    ERE(set_rgb_to_xyz_matrix());

    ERE(multiply_matrix_and_vector(out_vpp, fs_rgb_to_xyz_mp, in_vp));

    if (min_vector_element(*out_vpp) < -DBL_EPSILON)
    {
        verbose_pso(30, "Conversion from RGB to XYZ required truncation ");
        verbose_pso(30, "at zero.\n");
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int set_rgb_to_xyz_matrix(void)
{

    if (fs_rgb_to_xyz_mp == NULL)
    {
        read_rgb_to_xyz_matrix_from_file(DEFAULT_RGB_TO_XYZ_MATRIX_FILE);
    }

    if (fs_rgb_to_xyz_mp == NULL)
    {
        ERE(set_builtin_rgb_to_xyz_matrix());
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int read_rgb_to_xyz_matrix_from_file(const char *file_name)
{
    char data_dir[ MAX_FILE_NAME_SIZE ];


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    data_dir[ 0 ] = '\0';

    BUFF_CAT(data_dir, DATA_DIR);
    BUFF_CAT(data_dir, DIR_STR);

    BUFF_CAT(data_dir, MONITOR_DIR);
    BUFF_CAT(data_dir, DIR_STR);

    return read_matrix_from_config_file(&fs_rgb_to_xyz_mp, (const char*)NULL,
                                        data_dir, file_name,
                                        "rgb to xyz matrix",
                                        fs_rgb_to_xyz_file,
                                        sizeof(fs_rgb_to_xyz_file));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int set_builtin_rgb_to_xyz_matrix(void)
{

#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    ERE(get_target_matrix(&fs_rgb_to_xyz_mp, 3, 3));

    /*
    // Use sRGB for builtin.
    */
    fs_rgb_to_xyz_mp->elements[ 0 ][ 0 ] = 0.4124;
    fs_rgb_to_xyz_mp->elements[ 0 ][ 1 ] = 0.3576;
    fs_rgb_to_xyz_mp->elements[ 0 ][ 2 ] = 0.1805;
    fs_rgb_to_xyz_mp->elements[ 1 ][ 0 ] = 0.2126;
    fs_rgb_to_xyz_mp->elements[ 1 ][ 1 ] = 0.7152;
    fs_rgb_to_xyz_mp->elements[ 1 ][ 2 ] = 0.0722;
    fs_rgb_to_xyz_mp->elements[ 2 ][ 0 ] = 0.0193;
    fs_rgb_to_xyz_mp->elements[ 2 ][ 1 ] = 0.1192;
    fs_rgb_to_xyz_mp->elements[ 2 ][ 2 ] = 0.9505;

    ERE(ow_divide_matrix_by_scalar(fs_rgb_to_xyz_mp, 255.0));

    BUFF_CPY(fs_rgb_to_xyz_file, "<builtin (sRGB)>");

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static void prepare_memory_cleanup(void)
{
    static int first_time = TRUE;


    if (first_time)
    {
        add_cleanup_function(free_allocated_static_data);
        first_time = FALSE;
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static void free_allocated_static_data(void)
{

    free_matrix(fs_rgb_to_xyz_mp);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif


