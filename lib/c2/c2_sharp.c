
/* $Id: c2_sharp.c 21596 2017-07-30 23:33:36Z kobus $ */

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

#include "c2/c2_gen.h"        /* Only safe as first include in a ".c" file. */
#include "c2/c2_sharp.h"
#include "c/c_sensor.h"
#include "n/n_fit.h"
#include "n/n_invert.h"
#include "n/n_diagonalize.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*
// These are static to reduce the amount of allocation needed in routines which
// are called many times.
*/

static Matrix* fs_T_transpose_T_mp    = NULL;
static Matrix* fs_ATD_inv_T_mp        = NULL;
static Matrix* fs_inv_T_mp            = NULL;
static Matrix* fs_sharp_A_mp          = NULL;
static Matrix* fs_sharp_B_mp          = NULL;
static Vector* fs_sharp_A_row_sums_vp = NULL;
static Vector* fs_sharp_B_row_sums_vp = NULL;
static double fs_sharp_mip_db_positivity_lambda    = 10.0;
static double fs_sharp_mip_db_normalization_lambda = 10.0;
static double fs_sharp_mip_db_max_error_change     = 0.0001;
static double fs_sharp_mip_db_positivity_offset    = 0.01;
static int fs_sharp_mip_db_environment_version = 1;

/* -------------------------------------------------------------------------- */

static int get_mip_db_sharp_error_gradient
(
    Matrix**       grad_T_mpp,
    Matrix*        T_mp,
    Matrix*        T_plus_epsilon_mp,
    double         error,
    Matrix_vector* A_mvp,
    Matrix*        B_mp,
    Matrix_vector* B_mvp
);

static int revise_mip_db_sharp_transform
(
    Matrix*        T_mp,
    Matrix*        scratch_T_mp,
    Matrix*        grad_T_mp,
    Matrix_vector* A_mvp,
    Matrix*        B_mp,
    Matrix_vector* B_mvp,
    double         error,
    double*        new_error_ptr
);

static int mip_db_sharp_error
(
    Matrix*        T_mp,
    Matrix_vector* A_mvp,
    Matrix*        B_mp,
    Matrix_vector* B_mvp,
    double*        error_ptr,
    double*        mapping_error_ptr,
    double*        positivity_error_ptr,
    double*        normalization_error_ptr
);

static int perfect_sharpen_guts
(
    Matrix**      sharpen_post_map_mpp,
    const Matrix* sensor_mp,
    const Matrix* illum_mp,
    const Matrix* illum_svd_basis_mp,
    const Vector* illum_diag_vp,
    const Matrix* reflect_mp,
    const Matrix* reflect_svd_basis_mp,
    const Vector* reflect_diag_vp
);

static int perfect_sharpen_from_bases
(
    Matrix**      sharpen_post_map_mpp,
    const Matrix* sensor_mp,
    const Matrix* illum_basis_mp,
    const Matrix* reflect_basis_mp
);

static int verbose_output_fit_info
(
    int           verbose_cutoff,
    const Matrix* sensor_mp,
    const Matrix* illum_mp,
    const Matrix* illum_basis_mp,
    const Matrix* reflect_mp,
    const Matrix* reflect_basis_mp
);

static int verbose_output_singular_values
(
    int           verbose_cutoff,
    int           count,
    const Vector* diag_vp,
    const char*   title
);

static int generate_sensor_responses
(
    Matrix**      rgb_mpp,
    const Matrix* illum_mp,
    const Matrix* reflect_mp,
    const Matrix* sensor_mp
);

#ifdef TRACK_MEMORY_ALLOCATION
    static void free_allocated_static_data(void);
    static void prepare_memory_cleanup(void);
#endif


/* -------------------------------------------------------------------------- */

int set_sharpening_transform_options
(
    const char* option,
    const char* value
)
{
    char   lc_option[ 100 ];
    double   temp_double;
    int    result           = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "sharp-mip-db-positivity-lambda"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("sharp-mip-db-positivity-lambda= %.2f\n",
                    fs_sharp_mip_db_positivity_lambda));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Sharp MIP DB positivity lambda is %.2f.\n",
                    fs_sharp_mip_db_positivity_lambda));
        }
        else
        {
            ERE(ss1d(value, &temp_double));
            fs_sharp_mip_db_positivity_lambda = temp_double;
            fs_sharp_mip_db_environment_version++;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "sharp-mip-db-normalization-lambda"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("sharp-mip-db-normalization-lambda= %.2f\n",
                    fs_sharp_mip_db_normalization_lambda));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Sharp MIP DB normalization lambda is %.2f.\n",
                    fs_sharp_mip_db_normalization_lambda));
        }
        else
        {
            ERE(ss1d(value, &temp_double));
            fs_sharp_mip_db_normalization_lambda = temp_double;
            fs_sharp_mip_db_environment_version++;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "sharp-mip-db-positivity-offset"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("sharp-mip-db-positivity-offset= %.2e\n",
                    fs_sharp_mip_db_positivity_offset));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Sharp MIP DB positivity offset is %.2e.\n",
                    fs_sharp_mip_db_positivity_offset));
        }
        else
        {
            ERE(ss1d(value, &temp_double));
            fs_sharp_mip_db_positivity_offset = temp_double;
            fs_sharp_mip_db_environment_version++;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "sharp-mip-db-max-error-change"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("sharp-mip-db-max-error-change= %.2e\n",
                    fs_sharp_mip_db_max_error_change));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Sharp MIP DB max error change is %.2e.\n",
                    fs_sharp_mip_db_max_error_change));
        }
        else
        {
            ERE(ss1d(value, &temp_double));
            fs_sharp_mip_db_max_error_change = temp_double;
            fs_sharp_mip_db_environment_version++;
        }

        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int sharpen_spectra
(
    Spectra**      sharpend_sp_ptr,
    const Spectra* sp,
    const Matrix*  sharpen_post_map_mp
)
{

    ERE(copy_spectra(sharpend_sp_ptr, sp));

    return multiply_with_transpose(&((*sharpend_sp_ptr)->spectra_mp),
                                   sharpen_post_map_mp, sp->spectra_mp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             fix_sharpen_map
 *
 * Adjusts sharpening map so that is is more diagonal
 *
 * The routine adjusts a sharpening map so that it is more diagonal. Since a
 * sharpening transform is normally from an eigen-value decomposition, the
 * columns may be swapped and multiplied by any scalar. However, since the
 * columns are already normalized we only reverse the sign of chosen columns.
 * The basic idea is to put the element with largest absolute value on the
 * diagonal, and switch the sine of the column to make that element positive.
 * The same is then done with the remaining two columns, but now one of the rows
 * (the one used by the first step) cannot be used.
 *
 * Returns:
 *     NO_ERRO on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: sensors, sharpening
 *
 * -----------------------------------------------------------------------------
*/


int fix_sharpen_map(Matrix** new_post_map_mpp, const Matrix* post_map_mp)
{
    Vector* temp_vp = NULL;
    Matrix* post_map_copy_mp = NULL;
    Matrix* new_post_map_mp;
    int     i, i_max;
    int     j, j_max;
    double    max;
    int     count;


    if ((post_map_mp->num_rows != 3) || (post_map_mp->num_cols != 3))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_matrix(new_post_map_mpp, 3, 3));


    new_post_map_mp = *new_post_map_mpp;

    for (i=0; i<3; i++)
    {
        for (j=0; j<3; j++)
        {
            new_post_map_mp->elements[ i ][ j ] = DBL_MOST_NEGATIVE;
        }
    }

    ERE(copy_matrix(&post_map_copy_mp, post_map_mp));

    for (count=0; count<3; count++)
    {
        max = 0.0;
        i_max = NOT_SET;
        j_max = NOT_SET;

        /*
        // First find the largest, non marked element and get it's indices.
        // Elements are are marked by setting them to a large negative number.
        */

        for (i=0; i<3; i++)
        {
            for (j=0; j<3; j++)
            {
                /*
                // Find column with largest element (third condition) subject to
                // the column has not already been processed (first condition),
                // and that we would not been trying to move the column under
                // consideration to a column which has already been used (second
                // condition).  This condition is critical when the transform
                // cannot be made to look very diagonal.
                */
                if (    (IS_FINITE_DBL(post_map_copy_mp->elements[ i ][ j ]))
                     && ( ! IS_FINITE_DBL(new_post_map_mp->elements[ i ][ i ]))
                     && (ABS_OF(post_map_copy_mp->elements[ i ][ j ]) > max)
                   )
                {
                    max = ABS_OF(post_map_copy_mp->elements[ i ][ j ]);

                    i_max = i;
                    j_max = j;
                }
            }
        }

        ASSERT(i_max >= 0);
        ASSERT(j_max >= 0);

        /*
        // Put the column thus found so that the largest element is on the
        // diagonal.
        */

        ERE(get_matrix_col(&temp_vp, post_map_copy_mp, j_max));

        if ((temp_vp->elements)[ i_max ] < 0.0)
        {
            ERE(ow_multiply_vector_by_scalar(temp_vp, -1.0));
        }

        ERE(put_matrix_col(new_post_map_mp, temp_vp, i_max));

        for (i=0; i<3; i++)
        {
            (post_map_copy_mp->elements)[i][j_max] = DBL_MOST_NEGATIVE;
        }
    }

    free_vector(temp_vp);
    free_matrix(post_map_copy_mp);

    for (i=0; i<3; i++)
    {
        for (j=0; j<3; j++)
        {
            if ( ! IS_FINITE_DBL(new_post_map_mp->elements[ i ][ j ]))
            {
                set_error("BUG in code to fix sharpening matrix.");
                return ERROR;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_db_sharp_transform
(
    Matrix**      T_mpp,
    const Matrix* data_mp,
    const Matrix* canon_data_mp
)
{
    int result;
    Matrix* best_post_map_mp = NULL;
    Matrix* T_mp = NULL;
    Vector* D_vp = NULL;


    result = get_best_post_map(&best_post_map_mp, data_mp, canon_data_mp);

    if (result != ERROR)
    {
        result = diagonalize(best_post_map_mp, &T_mp, &D_vp);
    }

    if (result != ERROR)
    {
        result = fix_sharpen_map(T_mpp, T_mp);
    }

    free_vector(D_vp);
    free_matrix(T_mp);
    free_matrix(best_post_map_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#define MIP_DB_SHARP_MAX_ITS          500
#define MIP_DB_SHARP_BACKOFF_MAX_ITS  100
#define MIP_DB_SHARP_MAX_RELATIVE_ERROR_DIFF  0.0001
#define MIP_DB_SHARP_EPSILON         0.00001

int get_mip_db_sharp_transform
(
    Matrix**       T_mpp,
    const Spectra* reflect_spectra_sp,
    const Spectra* illum_spectra_sp,
    const Spectra* canon_spectra_sp,
    const Matrix*  initial_T_mp
)
{
    const int report_positivity_verbose_level = 1;
    int            num_illum    = NOT_SET;
    int            i;
    Matrix*        T_mp         = NULL;
    Matrix*        scratch_T_mp = NULL;
    Matrix_vector* A_mvp        = NULL;
    Matrix_vector* B_mvp        = NULL;
    Matrix*        B_mp         = NULL;
    Matrix*        grad_T_mp    = NULL;
    double         error        = DBL_NOT_SET;
    double         new_error    = DBL_NOT_SET;
    int            result       = NO_ERROR;


    ERE(get_target_matrix(&scratch_T_mp, 3, 3));

    if (initial_T_mp == NULL)
    {
        ERE(get_identity_matrix(&T_mp, 3));
    }
    else
    {
        ERE(copy_matrix(&T_mp, initial_T_mp));
    }

    /*
    // Big HACK put in place to do mip-db sharpining on image data. Ideally
    // this would be a different sharpening method, or an option.
    */
    if (    (get_path_type("sharp_A_mvp") == PATH_IS_REGULAR_FILE)
         && (get_path_type("sharp_B_mvp") == PATH_IS_REGULAR_FILE)
       )
    {
        verbose_pso(1, "Sharp data matrices are overiding use of spectra.\n");

        if (    (read_matrix_vector(&A_mvp, "sharp_A_mvp") == ERROR)
             || (read_matrix_vector(&B_mvp, "sharp_B_mvp") == ERROR)
           )
        {
            result = ERROR;
        }
        else
        {
            /*
            // FIX
            //
            // Should really be a test for equal sizes of the component matrices
            // also.
            */
            if ( ! matrix_vectors_are_comparable(A_mvp, B_mvp))
            {
                set_error("Sharp data matrix vectors are not comparable.");
                result = ERROR;
            }
            else
            {
                num_illum = A_mvp->length;

                for (i=0; i<num_illum; i++)
                {
                    if (result == ERROR) break;

                    if (result != ERROR)
                    {
                        result = ow_scale_matrix_by_max(A_mvp->elements[i]);
                    }

                    if (result != ERROR)
                    {
                        result = ow_multiply_matrix_by_scalar(A_mvp->elements[i],
                                                              255.0);
                    }

                    if (result != ERROR)
                    {
                        result = ow_scale_matrix_by_max(B_mvp->elements[i]);
                    }

                    if (result != ERROR)
                    {
                        result = ow_multiply_matrix_by_scalar(B_mvp->elements[i],
                                                              255.0);
                    }
                }
            }
        }
    }
    else
    {
        num_illum = illum_spectra_sp->spectra_mp->num_rows;

        if (    ((get_target_matrix_vector(&A_mvp, num_illum)) == ERROR)
             || (generate_RGB_data(&B_mp, canon_spectra_sp, 0,
                                   reflect_spectra_sp,
                                   (Spectra*)NULL)
                 == ERROR)
           )
        {
            result = ERROR;
        }

        if (result != ERROR)
        {
            for (i=0; i<num_illum; i++)
            {
                if (generate_RGB_data(&(A_mvp->elements[ i ]),
                                      illum_spectra_sp, i,
                                      reflect_spectra_sp, (Spectra*)NULL)
                    == ERROR)
                {
                    result = ERROR;
                    break;
                }
            }
        }
    }

    if (result != ERROR)
    {
        ASSERT(num_illum >= 0);

        if (num_illum == 0)
        {
            set_error("Zero illuminants for gradient descent sharpening.");
            result = ERROR;
        }
    }

    if (result != ERROR)
    {
        result = mip_db_sharp_error(T_mp, A_mvp, B_mp, B_mvp, &error,
                                    (double*)NULL, (double*)NULL,
                                    (double*)NULL);
    }

    for (i=0; i<MIP_DB_SHARP_MAX_ITS; i++)
    {
        IMPORT volatile Bool io_atn_flag;
        IMPORT volatile Bool halt_all_output;

        if (io_atn_flag)
        {
            set_error("Processing interrupted.");
            halt_all_output = FALSE;
            result = ERROR;
        }

        if (result == ERROR) break;

        if (i % 10 == 0)
        {
            double normalization_error;
            double positivity_error;
            double mapping_error;

            result = mip_db_sharp_error(T_mp, A_mvp, B_mp, B_mvp, NULL,
                                        &mapping_error, &positivity_error,
                                        &normalization_error);

            if (result != ERROR)
            {
                dbi(i);
                db_mat(T_mp);
                dbe(mapping_error);
                dbe(positivity_error);
                dbe(normalization_error);
                dbe(error);
            }
        }

        if (result != ERROR)
        {
            result = get_mip_db_sharp_error_gradient(&grad_T_mp, T_mp,
                                                     scratch_T_mp, error,
                                                     A_mvp, B_mp, B_mvp);
        }

        if (result != ERROR)
        {
            result = revise_mip_db_sharp_transform(T_mp, scratch_T_mp,
                                                   grad_T_mp, A_mvp,
                                                   B_mp, B_mvp,
                                                   error, &new_error);
        }

        if (i % 10 == 0)
        {
            double new_normalization_error;
            double new_positivity_error;
            double new_mapping_error;

            result = mip_db_sharp_error(T_mp, A_mvp, B_mp, B_mvp, NULL,
                                        &new_mapping_error,
                                        &new_positivity_error,
                                        &new_normalization_error);

            if (result != ERROR)
            {
                db_mat(T_mp);
                db_mat(grad_T_mp);
                dbe(new_mapping_error);
                dbe(new_positivity_error);
                dbe(new_normalization_error);
                dbe(new_error);
                dbe(ABS_OF(new_error - error));
            }
        }

        if (result != ERROR)
        {
            if (ABS_OF((new_error - error) / new_error) < fs_sharp_mip_db_max_error_change) break;
        }

        error = new_error;
    }

    if (     (result != ERROR)
         &&  ( ! (ABS_OF((new_error - error) / new_error) < fs_sharp_mip_db_max_error_change))
       )
    {
        warn_pso("Convergence not reached for MIP DB sharp transform.\n");
        warn_pso("After %d iterations, the relative error difference is %.3e.\n",
                 i, ABS_OF((new_error - error) / new_error));
        warn_pso("The desired error difference is %.3e.\n",
                 MIP_DB_SHARP_MAX_RELATIVE_ERROR_DIFF);
    }

    if (    (result != ERROR)
         && (kjb_get_verbose_level() >= report_positivity_verbose_level)
       )
    {
        int            j, k;
        double         temp;
        double         max_positivity_error  = 0.0;
        double         sum_positivity_error = 0.0;
        int            negative_count         = 0;
        int            num_RGB = num_illum * fs_sharp_A_mp->num_rows;

        for (i=0; i<num_illum; i++)
        {
            ERE(multiply_matrices(&fs_sharp_A_mp, A_mvp->elements[ i ],T_mp));

            for (j=0; j<fs_sharp_A_mp->num_rows; j++)
            {
                for (k=0; k<3; k++)
                {
                    if ((temp = fs_sharp_A_mp->elements[ j ][ k ]) < 0.0)
                    {
                        double p_temp = ABS_OF(temp);

                        sum_positivity_error += p_temp;
                        negative_count++;

                        if (p_temp > max_positivity_error)
                        {
                            max_positivity_error = p_temp;
                        }
                    }
                }
            }
        }

        verbose_pso(report_positivity_verbose_level,
                    "MIP sharp transform gives %d negative RGB components.\n",
                    negative_count);

        if (negative_count > 0)
        {
            verbose_pso(report_positivity_verbose_level,
                        "The largest magnitude is %.2e.\n",
                        max_positivity_error);

            verbose_pso(report_positivity_verbose_level,
                      "The average magnitude over %d negative ones is %.2e.\n",
                      negative_count,
                      sum_positivity_error /(double)negative_count);

            verbose_pso(report_positivity_verbose_level,
                        "The average magnitude over all %d RGB is %.2e.\n",
                        num_RGB, sum_positivity_error / (double)num_RGB);
        }
    }

    if (result != ERROR)
    {
        ASSERT(KJB_IS_SET(num_illum));
        result = copy_matrix(T_mpp, T_mp);
    }

    free_matrix_vector(A_mvp);
    free_matrix_vector(B_mvp);
    free_matrix(B_mp);
    free_matrix(T_mp);
    free_matrix(grad_T_mp);
    free_matrix(scratch_T_mp);

    if (result == ERROR)
    {
        return ERROR;
    }
    else
    {
        return fs_sharp_mip_db_environment_version;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int sharp_db_mip_env_is_up_to_date(int version)
{
    return (version == fs_sharp_mip_db_environment_version);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_mip_db_sharp_error_gradient
(
    Matrix**       grad_T_mpp,
    Matrix*        T_mp,
    Matrix*        T_plus_epsilon_mp,
    double         error,
    Matrix_vector* A_mvp,
    Matrix*        B_mp,
    Matrix_vector* B_mvp
)
{
    int    m, n;
    double new_error;
    int    result    = NO_ERROR;


    ERE(get_target_matrix(grad_T_mpp, 3, 3));

    for (m=0; m<3; m++)
    {
        if (result == ERROR) break;

        for (n=0; n<3; n++)
        {
            if (result == ERROR) break;

            if (copy_matrix(&T_plus_epsilon_mp, T_mp) == ERROR)
            {
                result = ERROR;
            }

            if (result != ERROR)
            {
                T_plus_epsilon_mp->elements[ m ][ n ] += MIP_DB_SHARP_EPSILON;

                result = mip_db_sharp_error(T_plus_epsilon_mp,
                                            A_mvp, B_mp, B_mvp, &new_error,
                                            (double*)NULL, (double*)NULL,
                                            (double*)NULL);
            }

            if (result != ERROR)
            {
                (*grad_T_mpp)->elements[ m ][ n ] = (new_error - error) /
                                                           MIP_DB_SHARP_EPSILON;
            }
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int revise_mip_db_sharp_transform
(
    Matrix*        T_mp,
    Matrix*        scratch_T_mp,
    Matrix*        grad_T_mp,
    Matrix_vector* A_mvp,
    Matrix*        B_mp,
    Matrix_vector* B_mvp,
    double         error,
    double*        new_error_ptr
)
{
    double new_error;
    double delta = -.001;
    int result = NO_ERROR;
    int i;


    for (i= 0; i<MIP_DB_SHARP_BACKOFF_MAX_ITS; i++)
    {
        IMPORT volatile Bool io_atn_flag;
        IMPORT volatile Bool halt_all_output;

        if (io_atn_flag)
        {
            set_error("Processing interrupted.");
            halt_all_output = FALSE;
            return ERROR;
        }

        ERE(copy_matrix(&scratch_T_mp, grad_T_mp));

        ERE(ow_multiply_matrix_by_scalar(scratch_T_mp, delta));

        ERE(ow_add_matrices(scratch_T_mp, T_mp));

        ERE(mip_db_sharp_error(scratch_T_mp, A_mvp, B_mp, B_mvp, &new_error,
                               (double*)NULL, (double*)NULL, (double*)NULL));

        if (ABS_OF(new_error) < ABS_OF(error)) break;

        delta /= 2.0;
    }

    if (ABS_OF(new_error) > ABS_OF(error))
    {
        set_error("Convergence not reached for MIP DB sharp transform.");
        result = ERROR;
    }
    else
    {
        ERE(copy_matrix(&T_mp, scratch_T_mp));
        *new_error_ptr = new_error;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int mip_db_sharp_error
(
    Matrix*        T_mp,
    Matrix_vector* A_mvp,
    Matrix*        B_mp,
    Matrix_vector* B_mvp,
    double*        error_ptr,
    double*        mapping_error_ptr,
    double*        positivity_error_ptr,
    double*        normalization_error_ptr
)
{
    static double   diag_elements[ 3 ];
    static Vector diag_vector        = { 3, 3, diag_elements };
    Vector*       diag_vp            = &diag_vector;
    int           num_illum          = A_mvp->length;
    int           num_surfaces;
    int           i, j, k;
    double        mapping_error = 0.0;
    double        normalization_error;
    double        positivity_error = 0.0;
    double        temp;
    double trace = 0.0;
    int total_num_rgb = 0;


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    ERE(multiply_by_transpose(&fs_T_transpose_T_mp, T_mp, T_mp));

    /* Use the trace for now. */
    for (k=0; k<3; k++)
    {
        trace += fs_T_transpose_T_mp->elements[ k ][ k ];
    }

    normalization_error = (3.0 - trace) * (3.0 - trace);

    ERE(get_matrix_inverse(&fs_inv_T_mp, T_mp));

    if (B_mvp == NULL)
    {
        ERE(multiply_matrices(&fs_sharp_B_mp, B_mp, T_mp));
        ERE(sum_matrix_rows(&fs_sharp_B_row_sums_vp, fs_sharp_B_mp));
    }

    for (i=0; i<num_illum; i++)
    {
        IMPORT volatile Bool io_atn_flag;
        IMPORT volatile Bool halt_all_output;

        if (io_atn_flag)
        {
            set_error("Processing interrupted.");
            halt_all_output = FALSE;
            return ERROR;
        }

        if (B_mvp != NULL)
        {
            B_mp = B_mvp->elements[ i ];

            ERE(multiply_matrices(&fs_sharp_B_mp, B_mp, T_mp));
            ERE(sum_matrix_rows(&fs_sharp_B_row_sums_vp, fs_sharp_B_mp));
        }

        ERE(multiply_matrices(&fs_sharp_A_mp, A_mvp->elements[ i ], T_mp));

        num_surfaces = B_mp->num_rows;

        /*
        // We want to penalize the optimization for negative sharpened
        // componants. If we add an error for every negative sharpened A for
        // every illuminant, then we will have done this nicely.
        */
        for (j=0; j<fs_sharp_A_mp->num_rows; j++)
        {
            for (k=0; k<3; k++)
            {
                temp = fs_sharp_A_mp->elements[ j ][ k ];
                temp -= fs_sharp_mip_db_positivity_offset;

                if (temp < 0.0)
                {
                    positivity_error += (temp * temp);
                }
            }
        }

        ERE(sum_matrix_rows(&fs_sharp_A_row_sums_vp, fs_sharp_A_mp));
        ERE(divide_vectors(&diag_vp, fs_sharp_B_row_sums_vp, fs_sharp_A_row_sums_vp));
        ERE(ow_multiply_matrix_by_row_vector_ew(fs_sharp_A_mp, diag_vp));
        ERE(multiply_matrices(&fs_ATD_inv_T_mp, fs_sharp_A_mp, fs_inv_T_mp));

        ERE(ow_subtract_matrices(fs_ATD_inv_T_mp, B_mp));

        /*
        // Normalize so we are treating each illuminant the same. The factor of
        // 1995 compensates for the fact that the first set of experiments were
        // run on data of this size, and I didn't want to re-tune the
        // parameters.
        */
        mapping_error += 1995 * frobenius_matrix_norm(fs_ATD_inv_T_mp) / num_surfaces;

        total_num_rgb += num_surfaces;
    }

    mapping_error /= num_illum;

    /*
    // We could normalize on a per-illuminant basis, but this is not how I
    // started doing it, and I don't think it would make a big different. It is
    // not clear whether we would want to do this anyway.
    */
    positivity_error /= total_num_rgb;

    if (error_ptr != NULL)
    {
        *error_ptr = mapping_error;
        *error_ptr += positivity_error * fs_sharp_mip_db_positivity_lambda;
        *error_ptr += normalization_error * fs_sharp_mip_db_normalization_lambda;
    }

    if (mapping_error_ptr != NULL)
    {
        *mapping_error_ptr = mapping_error;
    }

    if (positivity_error_ptr != NULL)
    {
        *positivity_error_ptr = positivity_error;
    }

    if (normalization_error_ptr != NULL)
    {
        *normalization_error_ptr = normalization_error;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DO_BIMODAL_ANALYSIS
int get_two_mode_perfect_sharp_transform
(
    Matrix**       sharpen_post_map_mpp,
    const Spectra* sensor_sp,
    const Spectra* illum_sp,
    const Spectra* reflect_sp
)
{
    Spectra* norm_illum_sp    = NULL;
    Matrix* reflect_basis_mp = NULL;
    Matrix* illum_basis_mp   = NULL;
    Vector* reflect_diag_vp  = NULL;
    Vector* illum_diag_vp    = NULL;


    ERE(normalize_illum_spectra(&norm_illum_sp, illum_sp, sensor_sp, 255.0));

    ERE(get_two_mode_basis_for_rows(sensor_sp->spectra_mp,
                                    2, norm_illum_sp->spectra_mp,
                                    &illum_basis_mp,
                                    3, reflect_sp->spectra_mp,
                                    &reflect_basis_mp));

    ERE(perfect_sharpen_guts(sharpen_post_map_mpp, sensor_sp->spectra_mp,
                             norm_illum_sp->spectra_mp, illum_basis_mp,
                             illum_diag_vp, reflect_sp->spectra_mp,
                             reflect_basis_mp, reflect_diag_vp));

    free_spectra(norm_illum_sp);
    free_vector(illum_diag_vp);
    free_vector(reflect_diag_vp);
    free_matrix(illum_basis_mp);
    free_matrix(reflect_basis_mp);

    return NO_ERROR;
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_canon_perfect_sharp_transform
(
    Matrix**       sharpen_post_map_mpp,
    const Spectra* sensor_sp,
    const Spectra* canon_sp,
    const Spectra* illum_sp,
    const Spectra* reflect_sp
)
{
    Matrix*  reflect_svd_basis_mp    = NULL;
    Matrix*  illum_svd_basis_mp      = NULL;
    Vector*  reflect_diag_vp         = NULL;
    Vector*  illum_diag_vp           = NULL;
    Vector*  canon_illum_vp          = NULL;
    int      i;
    Vector*  temp_row_vp             = NULL;
    Matrix*  temp_illum_svd_basis_mp = NULL;
    Matrix*  new_illum_mp            = NULL;
    Vector*  proj_vp                 = NULL;
    Spectra* norm_illum_sp    = NULL;
    Matrix*  norm_illum_mp;
    Matrix*  sensor_mp;
    Matrix*  reflect_mp;
    Matrix*  canon_mp;


    ERE(normalize_illum_spectra(&norm_illum_sp, illum_sp, sensor_sp, 255.0));
    canon_mp  = canon_sp->spectra_mp;
    norm_illum_mp = norm_illum_sp->spectra_mp;
    sensor_mp = sensor_sp->spectra_mp;
    reflect_mp = reflect_sp->spectra_mp;

    ERE(get_matrix_row(&canon_illum_vp, canon_mp, 0));
    ERE(ow_scale_vector_by_magnitude(canon_illum_vp));

    ERE(copy_matrix(&new_illum_mp, norm_illum_mp));

    for (i=0; i<new_illum_mp->num_rows; i++)
    {
        double dot_product;

        ERE(get_matrix_row(&temp_row_vp, new_illum_mp, i));
        ERE(copy_vector(&proj_vp, canon_illum_vp));

        ERE(get_dot_product(proj_vp, temp_row_vp, &dot_product));

        /* Relying on having normalized canon_illum_vp above */
        ERE(ow_multiply_vector_by_scalar(proj_vp, dot_product));
        ERE(ow_subtract_vectors(temp_row_vp, proj_vp));

        ERE(put_matrix_row(new_illum_mp, temp_row_vp, i));
    }

    free_vector(proj_vp);

    ERE(get_svd_basis_for_rows(new_illum_mp, &temp_illum_svd_basis_mp,
                               &illum_diag_vp));

    ERE(get_target_matrix(&illum_svd_basis_mp,
                          temp_illum_svd_basis_mp->num_rows + 1,
                          temp_illum_svd_basis_mp->num_cols));

    ERE(put_matrix_row(illum_svd_basis_mp, canon_illum_vp, 0));

    for (i=0; i<new_illum_mp->num_rows - 1; i++)
    {
        ERE(get_matrix_row(&temp_row_vp, temp_illum_svd_basis_mp, i));
        ERE(put_matrix_row(illum_svd_basis_mp, temp_row_vp, i + 1));
    }

    free_matrix(new_illum_mp);
    free_matrix(temp_illum_svd_basis_mp);
    free_vector(temp_row_vp);
    free_vector(canon_illum_vp);

    ERE(get_svd_basis_for_rows(reflect_mp, &reflect_svd_basis_mp,
                               &reflect_diag_vp));

    ERE(perfect_sharpen_guts(sharpen_post_map_mpp, sensor_mp,
                             norm_illum_mp, illum_svd_basis_mp,
                             illum_diag_vp, reflect_mp,
                             reflect_svd_basis_mp, reflect_diag_vp));


    free_spectra(norm_illum_sp);
    free_vector(illum_diag_vp);
    free_vector(reflect_diag_vp);
    free_matrix(illum_svd_basis_mp);
    free_matrix(reflect_svd_basis_mp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_standard_perfect_sharp_transform
(
    Matrix**       sharpen_post_map_mpp,
    const Spectra* sensor_sp,
    const Spectra* illum_sp,
    const Spectra* reflect_sp
)
{
    Matrix*  reflect_svd_basis_mp = NULL;
    Matrix*  illum_svd_basis_mp   = NULL;
    Vector*  reflect_diag_vp      = NULL;
    Vector*  illum_diag_vp        = NULL;
    Spectra* norm_illum_sp    = NULL;
    Matrix*  norm_illum_mp;
    Matrix*  sensor_mp;
    Matrix*  reflect_mp;


    ERE(normalize_illum_spectra(&norm_illum_sp, illum_sp, sensor_sp, 255.0));
    norm_illum_mp = norm_illum_sp->spectra_mp;
    sensor_mp = sensor_sp->spectra_mp;
    reflect_mp  = reflect_sp->spectra_mp;

    ERE(get_svd_basis_for_rows(norm_illum_mp, &illum_svd_basis_mp,
                               &illum_diag_vp));

    ERE(get_svd_basis_for_rows(reflect_mp, &reflect_svd_basis_mp,
                               &reflect_diag_vp));

    ERE(perfect_sharpen_guts(sharpen_post_map_mpp, sensor_mp,
                             norm_illum_mp, illum_svd_basis_mp,
                             illum_diag_vp, reflect_mp,
                             reflect_svd_basis_mp, reflect_diag_vp));

    free_spectra(norm_illum_sp);
    free_vector(illum_diag_vp);
    free_vector(reflect_diag_vp);
    free_matrix(illum_svd_basis_mp);
    free_matrix(reflect_svd_basis_mp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/
static int perfect_sharpen_guts
(
    Matrix**      sharpen_post_map_mpp,
    const Matrix* sensor_mp,
    const Matrix* illum_mp,
    const Matrix* illum_svd_basis_mp,
    const Vector* illum_diag_vp,
    const Matrix* reflect_mp,
    const Matrix* reflect_svd_basis_mp,
    const Vector* reflect_diag_vp
)
{
    const int verbose_level_for_fits  = 20;
    const int max_num_basis_functions = 10;


    ERE(verbose_output_singular_values(verbose_level_for_fits,
                                       max_num_basis_functions,
                                       illum_diag_vp,
                                       "Illuminant spectra "));

    ERE(verbose_output_singular_values(verbose_level_for_fits,
                                       max_num_basis_functions,
                                       reflect_diag_vp,
                                       "Reflectance spectra "));

    ERE(verbose_output_fit_info(verbose_level_for_fits, sensor_mp,
                                illum_mp, illum_svd_basis_mp,
                                reflect_mp, reflect_svd_basis_mp));

    ERE(perfect_sharpen_from_bases(sharpen_post_map_mpp, sensor_mp,
                                   illum_svd_basis_mp, reflect_svd_basis_mp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int perfect_sharpen_from_bases
(
    Matrix**      sharpen_post_map_mpp,
    const Matrix* sensor_mp,
    const Matrix* illum_basis_mp,
    const Matrix* reflect_basis_mp
)
{
    Matrix* canon_light_mp      = NULL;
    Matrix* light_mp            = NULL;
    Matrix* inv_canon_light_mp  = NULL;
    Vector* reflect_vp          = NULL;
    Vector* prod_vp             = NULL;
    Matrix* M_mp                = NULL;
    Matrix* new_T_inv_mp        = NULL;
    Matrix* T_inv_mp            = NULL;
    Matrix* T_mp                = NULL;
    Vector* D_vp = NULL;
    int     j;
    Vector* rgb_vp = NULL;
    Vector*  first_illum_vp = NULL;
    Vector*  second_illum_vp = NULL;
    int result = NO_ERROR;


    if (    (get_zero_matrix(&canon_light_mp, 3, 3) == ERROR)
         || (get_zero_matrix(&light_mp, 3, 3) == ERROR)
         || (get_matrix_row(&first_illum_vp, illum_basis_mp, 0) == ERROR)
         || (get_matrix_row(&second_illum_vp, illum_basis_mp, 1) == ERROR)
       )
    {
        result = ERROR;
        goto cleanup;
    }

    for (j=0; j<3; j++)
    {
        if (    (get_matrix_row(&reflect_vp, reflect_basis_mp, j) == ERROR)
             || (multiply_vectors(&prod_vp, reflect_vp, first_illum_vp)== ERROR)
             || (multiply_matrix_and_vector(&rgb_vp,
                                            sensor_mp, prod_vp) == ERROR)
           )
        {
            result = ERROR;
            goto cleanup;
        }

        (canon_light_mp->elements)[ 0 ][ j ] = (rgb_vp->elements)[ 0 ];
        (canon_light_mp->elements)[ 1 ][ j ] = (rgb_vp->elements)[ 1 ];
        (canon_light_mp->elements)[ 2 ][ j ] = (rgb_vp->elements)[ 2 ];
    }

    ERE(get_matrix_inverse(&inv_canon_light_mp, canon_light_mp));


    for (j=0; j<3; j++)
    {
        if (    (get_matrix_row(&reflect_vp, reflect_basis_mp, j) == ERROR)
             || (multiply_vectors(&prod_vp,
                                  reflect_vp, second_illum_vp)== ERROR)
             || (multiply_matrix_and_vector(&rgb_vp,
                                            sensor_mp, prod_vp) == ERROR)
           )
        {
            result = ERROR;
            goto cleanup;
        }

        (light_mp->elements)[ 0 ][ j ] = (rgb_vp->elements)[ 0 ];
        (light_mp->elements)[ 1 ][ j ] = (rgb_vp->elements)[ 1 ];
        (light_mp->elements)[ 2 ][ j ] = (rgb_vp->elements)[ 2 ];
    }

    ERE(multiply_matrices(&M_mp, light_mp, inv_canon_light_mp));
    ERE(diagonalize(M_mp, &T_inv_mp, &D_vp));
    ERE(get_matrix_inverse(&T_mp, T_inv_mp));
    ERE(fix_sharpen_map(&new_T_inv_mp, T_inv_mp));
    ERE(get_matrix_inverse(&T_mp, new_T_inv_mp));
    free_matrix(new_T_inv_mp);

    ERE(get_transpose(sharpen_post_map_mpp, T_mp));

cleanup :
    free_matrix(canon_light_mp);
    free_vector(first_illum_vp);
    free_vector(second_illum_vp);
    free_vector(rgb_vp);
    free_matrix(T_inv_mp);
    free_vector(D_vp);
    free_vector(prod_vp);
    free_vector(reflect_vp);
    free_matrix(inv_canon_light_mp);
    free_matrix(light_mp);
    free_matrix(M_mp);
    free_matrix(T_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int verbose_output_fit_info
(
    int           verbose_cutoff,
    const Matrix* sensor_mp,
    const Matrix* illum_mp,
    const Matrix* illum_basis_mp,
    const Matrix* reflect_mp,
    const Matrix* reflect_basis_mp
)
{
    double    illum_basis_error;
    double    reflect_basis_error;
    Matrix* illum_fit_mp        = NULL;
    Matrix* reflect_fit_mp      = NULL;
    Matrix* rgb_mp              = NULL;
    Matrix* est_rgb_mp          = NULL;
    Vector* channel_error_vp    = NULL;
    Vector* norm_vp             = NULL;
    Matrix* est_mp              = NULL;
    int     i;
    int     j;
    int     plot_id;
    double    total_error;
    Matrix* truncated_illum_basis_mp = NULL;
    Matrix* truncated_reflect_basis_mp = NULL;


    if (kjb_get_verbose_level() < verbose_cutoff) return NO_ERROR;

    ERE(plot_id = plot_open());
    ERE(plot_set_title(plot_id, "Illuminants", 0, 0));
    ERE(plot_multi_matrix_rows(plot_id, illum_mp, 380.0, 4.0, NULL));

    ERE(plot_id = plot_open());
    ERE(plot_set_title(plot_id, "Reflectances", 0, 0));
    ERE(plot_multi_matrix_rows(plot_id, reflect_mp, 380.0 , 4.0, NULL));

    ERE(copy_matrix(&truncated_illum_basis_mp, illum_basis_mp));
    truncated_illum_basis_mp->num_rows = MIN_OF(5, illum_basis_mp->num_rows) ;

    ERE(plot_id = plot_open());
    ERE(plot_set_title(plot_id, "Illuminant principle components", 0, 0));

    ERE(plot_matrix_rows(plot_id, truncated_illum_basis_mp, 380.0, 4.0, NULL,
                         (const int*)NULL));


    ERE(copy_matrix(&truncated_reflect_basis_mp, reflect_basis_mp));
    truncated_reflect_basis_mp->num_rows = MIN_OF(5,
                                                  reflect_basis_mp->num_rows);

    ERE(plot_id = plot_open());
    ERE(plot_set_title(plot_id, "Reflectance principle components", 0, 0));

    ERE(plot_matrix_rows(plot_id, truncated_reflect_basis_mp, 380.0, 4.0, NULL,
                         (const int*)NULL));

    for (i=0; i < MIN_OF(10, illum_basis_mp->num_rows); i++)
    {
        get_row_fits(&est_mp, illum_mp, i+1, illum_basis_mp,
                     &illum_basis_error);
        verbose_pso(verbose_cutoff,
                    "Illum Fit Error for %d PC vectors: %.2f%%\n",
                    i+1, illum_basis_error * 100.0);
    }
    verbose_pso(5, "\n");

    for (i=0; i < MIN_OF(10, reflect_basis_mp->num_rows); i++)
    {
        get_row_fits(&est_mp, reflect_mp, i+1, reflect_basis_mp,
                     &reflect_basis_error);
        verbose_pso(verbose_cutoff,
                    "Surface Fit Error for %d PC vectors: %.2f%%\n",
                    i+1, reflect_basis_error * 100.0);
    }
    verbose_pso(verbose_cutoff, "\n");

    ERE(generate_sensor_responses(&rgb_mp, illum_mp, reflect_mp, sensor_mp));

    for (i=2; i<= MIN_OF(5, illum_basis_mp->num_rows); i++)
    {
        for (j=2; j <= MIN_OF(5, reflect_basis_mp->num_rows); j++)
        {
            ERE(get_row_fits(&illum_fit_mp, illum_mp, i, illum_basis_mp,
                             &illum_basis_error));

            ERE(get_row_fits(&reflect_fit_mp, reflect_mp, j, reflect_basis_mp,
                             &reflect_basis_error));

            ERE(generate_sensor_responses(&est_rgb_mp, illum_fit_mp,
                                          reflect_fit_mp, sensor_mp));

            /*
            ERE(get_rms_relative_row_error(&channel_error_vp, rgb_mp,
                                           est_rgb_mp, &total_error));
            */

            ERE(get_rms_col_error(&channel_error_vp, rgb_mp,
                                  est_rgb_mp, &total_error));

            verbose_pso(verbose_cutoff, "Absolute error\n");

            verbose_pso(verbose_cutoff,
                        "    Num illum basis vectors : %d\n", i);

            verbose_pso(verbose_cutoff,
                        "    Num reflect basis vectors : %d\n", j);

            verbose_pso(verbose_cutoff, "        Total : %.3f\n", total_error);

            verbose_pso(verbose_cutoff, "        Red   : %.3f\n",
                        channel_error_vp->elements[ 0 ]);

            verbose_pso(verbose_cutoff, "        Green : %.3f\n",
                        channel_error_vp->elements[ 1 ]);

            verbose_pso(verbose_cutoff, "        Blue  : %.3f\n\n",
                        channel_error_vp->elements[ 2 ]);
        }
    }

    free_vector(channel_error_vp);
    free_vector(norm_vp);
    free_matrix(illum_fit_mp);
    free_matrix(reflect_fit_mp);
    free_matrix(rgb_mp);
    free_matrix(est_rgb_mp);
    free_matrix(est_mp);
    free_matrix(truncated_illum_basis_mp);
    free_matrix(truncated_reflect_basis_mp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int verbose_output_singular_values
(
    int           verbose_cutoff,
    int           count,
    const Vector* diag_vp,
    const char*   title
)
{
    int i;
    Vector *normalized_diag_vp = NULL;


    if (    (kjb_get_verbose_level() < verbose_cutoff)
         || (diag_vp == NULL)
       )
    {
        return NO_ERROR;
    }

    ERE(copy_vector(&normalized_diag_vp, diag_vp));
    ERE(ow_scale_vector_by_sum(normalized_diag_vp));

    for (i=0; i < MIN_OF(count, normalized_diag_vp->length); i++)
    {
        verbose_pso(verbose_cutoff, "%s singular value %d is %.2f\n", title, i,
                    (normalized_diag_vp->elements)[i]);
    }

    verbose_pso(verbose_cutoff, "\n");

    free_vector(normalized_diag_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int generate_sensor_responses
(
    Matrix**      rgb_mpp,
    const Matrix* illum_mp,
    const Matrix* reflect_mp,
    const Matrix* sensor_mp
)
{
    Matrix* product_mp = NULL;


    ERE(multiply_matrix_rows(&product_mp, illum_mp, reflect_mp));

    ERE(multiply_by_transpose(rgb_mpp, product_mp, sensor_mp));

    free_matrix(product_mp);

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
    free_matrix(fs_T_transpose_T_mp);
    free_matrix(fs_ATD_inv_T_mp);
    free_matrix(fs_inv_T_mp);
    free_matrix(fs_sharp_A_mp);
    free_matrix(fs_sharp_B_mp);
    free_vector(fs_sharp_A_row_sums_vp);
    free_vector(fs_sharp_B_row_sums_vp);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


#ifdef __cplusplus
}
#endif

