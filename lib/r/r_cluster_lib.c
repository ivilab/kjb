
/* $Id: r_cluster_lib.c 21596 2017-07-30 23:33:36Z kobus $ */

/* =========================================================================== *
|  
|  Copyright (c) by the UA computer vision group (representing affiliated
|  authors and institutions).  
|  
|  Authors of this code include:
|      Prasad Gabbur
|      Kobus Barnard
| 
|  This code is under development and is currently restricted to use within the
|  UA computer vision group except when granted on a case by case basis.
|  (Granting of permission is implicit if a vision group member explicitly
|  provides access to the code, perhaps by E-mailing a tar file). Permission for
|  further distribution of the code is not granted.  Permission for use is
|  further subject to the following general conditions for distribution of UA
|  computer vision code.
| 
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
| 
|  For other use contact kobus AT cs DOT arizona DOT edu.
| 
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or
|  fitness for any particular task. Nonetheless, we are interested in hearing
|  about problems that you encounter. 
|
*  ========================================================================== */


#include "m/m_incl.h"      /*  Only safe if first #include in a ".c" file  */
#include "p/p_plot.h"
#include "r/r_cluster_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define MIN_ALLOWABLE_DATA_DIFF  (1.0e-20)
#define MIN_ALLOWABLE_DATA_VAR   (DBL_MIN * 1000.0)

/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */

static double fs_cluster_random_stabilizing_factor_1 = 0.1;
static double fs_cluster_random_stabilizing_factor_2 = 0.1;
static int    fs_perturb_data_by_range = FALSE;

/* -------------------------------------------------------------------------- */

static int ow_perturb_composite_cluster_data_by_range
(
    Matrix_vector*    composite_feature_mvp,
    double            perturbation,
    int               num_features,
    int               num_disabled_features,
    const Int_vector* disable_perturb_vp
);

static int ow_perturb_composite_cluster_data_by_value
(
    Matrix_vector*    composite_feature_mvp,
    double            perturbation,
    int               num_features,
    int               num_disabled_features,
    const Int_vector* disable_perturb_vp
);

static int ow_perturb_unary_cluster_data_by_range
(
    Matrix*           unary_feature_mp,
    double            perturbation,
    int               num_disabled_features,
    const Int_vector* disable_perturb_vp
);

static int ow_perturb_unary_cluster_data_by_value
(
    Matrix*           unary_feature_mp,
    double            perturbation,
    int               num_disabled_features,
    const Int_vector* disable_perturb_vp
);

/* -------------------------------------------------------------------------- */

int set_cluster_lib_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result         = NOT_FOUND;
    double temp_double_value;
    int    temp_boolean_value;



    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "cluster-perturb-data-by-range")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Cluster data %s perturbed relative to feature range.\n",
                    fs_perturb_data_by_range ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-perturb-data-by-range = %s\n",
                    fs_perturb_data_by_range ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_perturb_data_by_range = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "cluster-random-stabilizing-factor-1")
          || match_pattern(lc_option, "cluster-random-factor-1")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("EM random stabilizing factor 1 is %.3e.\n",
                    fs_cluster_random_stabilizing_factor_1));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-random-stabilizing-factor-1 = %.3e\n",
                    fs_cluster_random_stabilizing_factor_1));
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_cluster_random_stabilizing_factor_1 = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "cluster-random-stabilizing-factor-2")
          || match_pattern(lc_option, "cluster-random-factor-2")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("EM random stabilizing factor 2 is %.3e.\n",
                    fs_cluster_random_stabilizing_factor_2));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-random-stabilizing-factor-2 = %.3e\n",
                    fs_cluster_random_stabilizing_factor_2));
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_cluster_random_stabilizing_factor_2 = temp_double_value;
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_cluster_random_matrix_1(Matrix** mpp, int num_rows, int num_cols)
{

    ERE(get_random_matrix(mpp, num_rows, num_cols));
    ERE(ow_add_scalar_to_matrix(*mpp, fs_cluster_random_stabilizing_factor_1));
    ERE(ow_scale_matrix_rows_by_sums(*mpp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_cluster_random_matrix_2(Matrix** mpp, int num_rows, int num_cols)
{

    ERE(get_random_matrix(mpp, num_rows, num_cols));
    ERE(ow_add_scalar_to_matrix(*mpp, fs_cluster_random_stabilizing_factor_2));
    ERE(ow_scale_matrix_cols_by_sums(*mpp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_cluster_random_matrix(Matrix** mpp, int num_rows, int num_cols)
{
    double   factor = kjb_rand();

    ERE(get_random_matrix(mpp, num_rows, num_cols));
    ERE(ow_multiply_matrix_by_scalar(*mpp, 1.0 / ABS_OF(factor - 0.5)));
    ERE(ow_add_scalar_to_matrix(*mpp, 1.0));
    ERE(ow_scale_matrix_rows_by_sums(*mpp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int perturb_composite_cluster_data
(
    Matrix_vector**      perturbed_composite_feature_mvpp,
    const Matrix_vector* composite_feature_mvp,
    double               perturbation,
    const Int_vector*    disable_perturb_vp
)
{

    ERE(copy_matrix_vector(perturbed_composite_feature_mvpp,
                           composite_feature_mvp));

    return ow_perturb_composite_cluster_data(*perturbed_composite_feature_mvpp,
                                             perturbation,
                                             disable_perturb_vp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_perturb_composite_cluster_data
(
    Matrix_vector* composite_feature_mvp,
    double         perturbation,
    const Int_vector* disable_perturb_vp
)
{
    int     num_features = NOT_SET;
    int     num_points;
    int     i;
    int num_enabled_features = 0;
    int num_disabled_features = 0;


    if (composite_feature_mvp == NULL) return NO_ERROR;

    num_points = composite_feature_mvp->length;

    for (i = 0; i<num_points; i++)
    {
        if (composite_feature_mvp->elements[ i ] != NULL)
        {
            if (KJB_IS_SET(num_features))
            {
                if (num_features != composite_feature_mvp->elements[ i ]->num_cols)
                {
                    set_error("Inconsistent number of continuous features spotted in data.");
                    return ERROR;
                }
            }
            else
            {
                num_features = composite_feature_mvp->elements[ i ]->num_cols;
            }
        }
    }

    if (num_features == NOT_SET)
    {
        /* No features, so we cant perturb them. */
        return NO_ERROR;
    }

    for (i = 0; i<num_features; i++)
    {
        if (    (disable_perturb_vp == NULL)
             || (disable_perturb_vp->elements[ i ] == 0)
           )
        {
            num_enabled_features++;
        }
        else
        {
            num_disabled_features++;
        }
    }

    if (num_enabled_features == 0)
    {
        /* No features, so we cant perturb them. */
        return NO_ERROR;
    }

    if (fs_perturb_data_by_range)
    {
        return ow_perturb_composite_cluster_data_by_range(composite_feature_mvp,
                                                          perturbation,
                                                          num_features,
                                                          num_disabled_features,
                                                          disable_perturb_vp);
    }
    else
    {
        return ow_perturb_composite_cluster_data_by_value(composite_feature_mvp,
                                                          perturbation,
                                                          num_features,
                                                          num_disabled_features,
                                                          disable_perturb_vp);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int ow_perturb_composite_cluster_data_by_range
(
    Matrix_vector* composite_feature_mvp,
    double         perturbation,
    int               num_features,
    int               num_disabled_features,
    const Int_vector* disable_perturb_vp
)
{
    Vector* data_min_vp = NULL;
    Vector* data_max_vp = NULL;
    Vector* data_max_min_diff_vp = NULL;
    int     num_points;
    int     i, j, k;
    int     result;


    num_points = composite_feature_mvp->length;

    ERE(get_initialized_vector(&data_min_vp, num_features,
                               DBL_HALF_MOST_POSITIVE));

    result = get_initialized_vector(&data_max_vp, num_features,
                                    DBL_HALF_MOST_NEGATIVE);

    for (i = 0; i<num_points; i++)
    {
        Matrix* feature_mp = composite_feature_mvp->elements[ i ];
        int num_feature_sources;

        if (result == ERROR) break;

        if (feature_mp == NULL) continue;

        num_feature_sources = feature_mp->num_rows;

        for (j=0; j<num_feature_sources; j++)
        {
            for (k = 0; k < num_features; k++)
            {
                double t = feature_mp->elements[ j ][ k ];

                if ((respect_missing_values()) && (IS_MISSING_DBL(t)))
                {
                    /*EMPTY*/
                    ; /* Do nothing */
                }
                else if (t < data_min_vp->elements[ k ])
                {
                    data_min_vp->elements[ k ] = t;
                }
                else if (t > data_max_vp->elements[ k ])
                {
                    data_max_vp->elements[ k ] = t;
                }
            }
        }
    }

    if (result != ERROR)
    {
        result = subtract_vectors(&data_max_min_diff_vp, data_max_vp,
                                  data_min_vp);
    }

    if (result != ERROR)
    {
        for (k = 0; k < num_features; k++)
        {
            if (data_max_min_diff_vp->elements[ k ] < 0.0001)
            {
                dbi(k);
                dbe(data_max_min_diff_vp->elements[ k ]);
            }
        }
    }

    for (i = 0; i<num_points; i++)
    {
        Matrix* feature_mp = composite_feature_mvp->elements[ i ];
        int num_feature_sources;

        if (result == ERROR) break;

        if (feature_mp == NULL) continue;

        num_feature_sources = feature_mp->num_rows;

        for (k = 0; k < num_features; k++)
        {
            if (    (num_disabled_features == 0)
                 || (disable_perturb_vp->elements[ k ] == 0)
               )
            {
                double f = data_max_min_diff_vp->elements[ k ];

                f *= perturbation;
                f *= 2.0;    /* We use random factors in [ -0.5, 0.5 ] */

                for (j=0; j<num_feature_sources; j++)
                {
                    if (   (respect_missing_values())
                        && (IS_MISSING_DBL(feature_mp->elements[ j ][ k ]))
                       )
                    {
                        feature_mp->elements[ j ][ k ] = DBL_MISSING;
                    }
                    else
                    {
                        feature_mp->elements[ j ][ k ] += f * (kjb_rand() - 0.5);
                    }
                }
            }
        }
    }

    free_vector(data_max_min_diff_vp);
    free_vector(data_min_vp);
    free_vector(data_max_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int ow_perturb_composite_cluster_data_by_value
(
    Matrix_vector*    composite_feature_mvp,
    double            perturbation,
    int               num_features,
    int               num_disabled_features,
    const Int_vector* disable_perturb_vp
)
{
    int     num_points;
    int     i, j, k;


    num_points = composite_feature_mvp->length;

    for (i = 0; i<num_points; i++)
    {
        Matrix* feature_mp = composite_feature_mvp->elements[ i ];
        int num_feature_sources;

        if (feature_mp == NULL) continue;

        num_feature_sources = feature_mp->num_rows;

        for (k = 0; k < num_features; k++)
        {
            if (    (num_disabled_features == 0)
                 || (disable_perturb_vp->elements[ k ] == 0)
               )
            {
                for (j=0; j<num_feature_sources; j++)
                {
                    double f = feature_mp->elements[ j ][ k ];

                    if (   (respect_missing_values())
                        && (IS_MISSING_DBL(f))
                       )
                    {
                        feature_mp->elements[ j ][ k ] = DBL_MISSING;
                    }
                    else
                    {
                        f *= perturbation;
                        f *= 2.0;    /* We use random factors in [ -0.5, 0.5 ] */

                        feature_mp->elements[ j ][ k ] += f * (kjb_rand() - 0.5);
                    }
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int perturb_unary_cluster_data
(
    Matrix**      perturbed_unary_feature_mpp,
    const Matrix* unary_feature_mp,
    double        perturbation,
    const Int_vector* disable_perturb_vp
)
{


    ERE(copy_matrix(perturbed_unary_feature_mpp, unary_feature_mp));

    if (unary_feature_mp == NULL) return NO_ERROR;

    return ow_perturb_unary_cluster_data(*perturbed_unary_feature_mpp,
                                         perturbation, disable_perturb_vp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_perturb_unary_cluster_data
(
    Matrix*           unary_feature_mp,
    double            perturbation,
    const Int_vector* disable_perturb_vp
)
{
    int num_features;
    int num_points;
    int     i;
    int num_disabled_features = 0;
    int num_enabled_features = 0;


    num_features = unary_feature_mp->num_cols;
    num_points = unary_feature_mp->num_rows;

    for (i = 0; i<num_features; i++)
    {
        if (    (disable_perturb_vp != NULL)
             && (disable_perturb_vp->elements[ i ] != 0)
           )
        {
            num_disabled_features++;
        }
        else
        {
            num_enabled_features++;
        }
    }

    if (num_enabled_features == 0)
    {
        /* No features, so we cant perturb them. */
        return NO_ERROR;
    }

    if (fs_perturb_data_by_range)
    {
        return ow_perturb_unary_cluster_data_by_range(unary_feature_mp,
                                                      perturbation,
                                                      num_disabled_features,
                                                      disable_perturb_vp);
    }
    else
    {
        return ow_perturb_unary_cluster_data_by_value(unary_feature_mp,
                                                      perturbation,
                                                      num_disabled_features,
                                                      disable_perturb_vp);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int ow_perturb_unary_cluster_data_by_range
(
    Matrix* unary_feature_mp,
    double  perturbation,
    int               num_disabled_features,
    const Int_vector* disable_perturb_vp
)
{
    Vector* rand_vp = NULL;
    Vector* data_min_vp = NULL;
    Vector* data_max_vp = NULL;
    Vector* data_max_min_diff_vp = NULL;
    int num_features;
    int num_points;
    int     i, k;
    int     result;
    int first_small_var_feature = TRUE;


    num_features = unary_feature_mp->num_cols;
    num_points = unary_feature_mp->num_rows;

    result = get_min_matrix_col_elements(&data_min_vp, unary_feature_mp);

    if (result != ERROR)
    {
        result = get_max_matrix_col_elements(&data_max_vp, unary_feature_mp);
    }

    if (result != ERROR)
    {
        result = subtract_vectors(&data_max_min_diff_vp, data_max_vp,
                                  data_min_vp);
    }

    for (k = 0; k < num_features; k++)
    {
        if (    (respect_missing_values())
             && (IS_MISSING_DBL(data_max_min_diff_vp->elements[ k ]))
           )
        {
            /*EMPTY*/
            ; /* Do nothing. */

            UNTESTED_CODE();
        }
        else if (data_max_min_diff_vp->elements[ k ] < MIN_ALLOWABLE_DATA_DIFF)
        {
            if (first_small_var_feature)
            {
                warn_pso("Data with at least one feature with very small variance spotted.\n");
                warn_pso("Perturbing them with small absolute instead of relative amount.\n");
                first_small_var_feature = FALSE;
            }

            data_max_min_diff_vp->elements[ k ] = MIN_ALLOWABLE_DATA_DIFF;
        }
    }

    for (i = 0; i < num_points; i++)
    {
        if (result == ERROR) break;

        result = get_random_vector(&rand_vp, num_features);

        if (result != ERROR)
        {
            result = ow_subtract_scalar_from_vector(rand_vp, 0.5);
        }
        if (result != ERROR)
        {
            /* Extra factor of 2 because we use random factors in [-0.5, 0.5] */
            result = ow_multiply_vector_by_scalar(rand_vp,
                                                  2.0 * perturbation);
        }
        if (result != ERROR)
        {
            result = ow_multiply_vectors(rand_vp, data_max_min_diff_vp);
        }

        if ((result != ERROR) && (num_disabled_features > 0))
        {
            int ii;

            for (ii = 0; ii<num_features; ii++)
            {
                if (disable_perturb_vp->elements[ ii ] != 0)
                {
                    rand_vp->elements[ ii ] = 0.0;
                }
            }
        }

        if (result != ERROR)
        {
            result = ow_add_vector_to_matrix_row(unary_feature_mp,
                                                 rand_vp, i);
        }
    }

    free_vector(rand_vp);
    free_vector(data_max_min_diff_vp);
    free_vector(data_min_vp);
    free_vector(data_max_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int ow_perturb_unary_cluster_data_by_value
(
    Matrix*           unary_feature_mp,
    double            perturbation,
    int               num_disabled_features,
    const Int_vector* disable_perturb_vp
)
{
    Vector* rand_vp = NULL;
    int num_features;
    int num_points;
    int     i;
    int     result = NO_ERROR;


    num_features = unary_feature_mp->num_cols;
    num_points = unary_feature_mp->num_rows;

    for (i = 0; i < num_points; i++)
    {
        if (result == ERROR) break;

        result = get_random_vector(&rand_vp, num_features);

        if (result != ERROR)
        {
            result = ow_subtract_scalar_from_vector(rand_vp, 0.5);
        }
        if (result != ERROR)
        {
            /* Extra factor of 2 because we use random factors in [-0.5, 0.5] */
            result = ow_multiply_vector_by_scalar(rand_vp,
                                                  2.0 * perturbation);
        }

        if ((result != ERROR) && (num_disabled_features > 0))
        {
            int ii;

            for (ii = 0; ii<num_features; ii++)
            {
                if (disable_perturb_vp->elements[ ii ] != 0)
                {
                    rand_vp->elements[ ii ] = 0.0;
                }
            }
        }

        if (result != ERROR)
        {
            result = ow_add_scalar_to_vector(rand_vp, 1.0);
        }

        if (result != ERROR)
        {
            result = ow_multiply_matrix_row_by_vector(unary_feature_mp,
                                                      rand_vp, i);
        }
    }

    free_vector(rand_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_normalize_composite_cluster_data
(
    Matrix_vector* composite_feature_mvp,
    Matrix_vector* held_out_composite_feature_mvp,
    Vector**       mean_vpp,
    Vector**       var_vpp,
    double         norm_stdev,
    const Int_vector* disable_normalize_vp
)
{
    Vector* mean_vp      = NULL;
    Vector* var_vp     = NULL;
    int     num_features = NOT_SET;
    int     num_points = (composite_feature_mvp == NULL) ? 0 : composite_feature_mvp->length;
    int     num_held_out_points = (held_out_composite_feature_mvp == NULL) ? 0 : held_out_composite_feature_mvp->length;
    int     i;
    int     result;
    int     count        = 0;
    Matrix* composite_feature_mp = NULL;
    Matrix* held_out_composite_feature_mp = NULL;
    int num_enabled_features = 0;


    for (i = 0; i<num_points; i++)
    {
        if (composite_feature_mvp->elements[ i ] != NULL)
        {
            if (KJB_IS_SET(num_features))
            {
                if (num_features != composite_feature_mvp->elements[ i ]->num_cols)
                {
                    set_error("Inconsistent number of continuous features spotted in data.");
                    return ERROR;
                }
            }
            else
            {
                num_features = composite_feature_mvp->elements[ i ]->num_cols;
            }
        }
    }

    for (i = 0; i<num_held_out_points; i++)
    {
        if (held_out_composite_feature_mvp->elements[ i ] != NULL)
        {
            if (KJB_IS_SET(num_features))
            {
                if (num_features != held_out_composite_feature_mvp->elements[ i ]->num_cols)
                {
                    set_error("Inconsistent number of continuous features spotted in data.");
                    return ERROR;
                }
            }
            else
            {
                num_features = held_out_composite_feature_mvp->elements[ i ]->num_cols;
            }
        }
    }

    if (num_features == NOT_SET)
    {
        /* No features, so we can't nomalized them. */
        return NO_ERROR;
    }

    for (i = 0; i<num_features; i++)
    {
        if (    (disable_normalize_vp == NULL)
             || (disable_normalize_vp->elements[ i ] == 0)
           )
        {
            num_enabled_features++;
        }
    }

    if (num_enabled_features == 0)
    {
        /* No features, so we can't nomalized them. */
        return NO_ERROR;
    }

    result = get_matrix_from_matrix_vector(&composite_feature_mp,
                                           composite_feature_mvp);

    if (result != ERROR)
    {
        result = get_matrix_from_matrix_vector(&held_out_composite_feature_mp,
                                               held_out_composite_feature_mvp);
    }

    if (result != ERROR)
    {
        result = ow_normalize_cluster_data(composite_feature_mp,
                                           held_out_composite_feature_mp,
                                           &mean_vp, &var_vp, norm_stdev,
                                           disable_normalize_vp);
    }

    for (i = 0; i<num_points; i++)
    {
        Matrix* feature_mp          = composite_feature_mvp->elements[ i ];
        int     num_feature_sources;

        if (result == ERROR) break;
        if (feature_mp == NULL) continue;

        num_feature_sources = feature_mp->num_rows;

        result = ow_copy_matrix_block(feature_mp, 0, 0,
                                      composite_feature_mp, count, 0,
                                      num_feature_sources, num_features);
        count += num_feature_sources;
    }

    count = 0;

    for (i = 0; i<num_held_out_points; i++)
    {
        Matrix* feature_mp = held_out_composite_feature_mvp->elements[ i ];
        int num_feature_sources;

        if (result == ERROR) break;

        if (feature_mp == NULL) continue;

        num_feature_sources = feature_mp->num_rows;

        result = ow_copy_matrix_block(feature_mp, 0, 0,
                                      held_out_composite_feature_mp, count, 0,
                                      num_feature_sources, num_features);
        count += num_feature_sources;
    }

    if ((result != ERROR) && (mean_vpp != NULL))
    {
        result = copy_vector(mean_vpp, mean_vp);
    }

    if ((result != ERROR) && (var_vpp != NULL))
    {
        result = copy_vector(var_vpp, var_vp);
    }

    free_vector(var_vp);
    free_vector(mean_vp);
    free_matrix(composite_feature_mp);
    free_matrix(held_out_composite_feature_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * We assume that the called routines respect missing values if needed.
*/
int ow_normalize_cluster_data
(
    Matrix*  feature_mp,
    Matrix*  held_out_feature_mp,
    Vector** mean_vpp,
    Vector** var_vpp,
    double   norm_stdev,
    const Int_vector* disable_normalize_vp
)
{
    Int_vector* non_missing_count_vp = NULL;
    Vector* sum_sqr_vp   = NULL;
    Vector* mean_vp      = NULL;
    Vector* stdev_vp     = NULL;
    int     num_features = NOT_SET;
    int     num_points;
    int     num_held_out_points = 0;
    int     i, j;
    int     result;
    Vector* data_min_vp = NULL;
    Vector* data_max_vp = NULL;
    Vector* data_max_min_diff_vp = NULL;
    int num_enabled_features = 0;
    int num_disabled_features = 0;


    if (feature_mp == NULL) return NO_ERROR;

    num_points = feature_mp->num_rows;
    num_features = feature_mp->num_cols;

#ifdef TEST
    if (disable_normalize_vp != NULL)
    {
        ASSERT(num_features == disable_normalize_vp->length);
    }
#endif

    for (i = 0; i<num_features; i++)
    {
        if (    (disable_normalize_vp == NULL)
             || (disable_normalize_vp->elements[ i ] == 0)
           )
        {
            num_enabled_features++;
        }
        else
        {
            num_disabled_features++;
        }
    }

    if ((num_points <= 0) || (num_enabled_features <= 0))
    {
        /* No points/features, so we can't nomalized them. */
        return NO_ERROR;
    }

    if (num_points < 2)
    {
        set_error("Insufficient data for normalization.");
        result = ERROR;
    }

    if (held_out_feature_mp != NULL)
    {
        num_held_out_points = held_out_feature_mp->num_rows;

        if (num_features != held_out_feature_mp->num_cols)
        {
            set_error("Inconsistent number of continuous features spotted in data.");
            return ERROR;
        }
    }

    result = average_matrix_rows(&mean_vp, feature_mp);

    if ((result != ERROR) && (num_disabled_features > 0))
    {
        for (i = 0; i<num_features; i++)
        {
            if (disable_normalize_vp->elements[ i ])
            {
                mean_vp->elements[ i ] = 0.0;
            }
        }
    }

    if (result != ERROR)
    {
        result = ow_subtract_row_vector_from_matrix(feature_mp, mean_vp);
    }

    if ((result != ERROR) && (num_held_out_points > 0))
    {
        result = ow_subtract_row_vector_from_matrix(held_out_feature_mp,
                                                    mean_vp);
    }

    if ((result != ERROR) && (mean_vpp != NULL))
    {
        result = copy_vector(mean_vpp, mean_vp);
    }

    if (result != ERROR)
    {
         result = get_zero_vector(&sum_sqr_vp, num_features);
    }

    if (result != ERROR)
    {
         result = get_zero_int_vector(&non_missing_count_vp,
                                      num_features);
    }

    for (i = 0; i<num_points; i++)
    {
        if (result == ERROR) break;

        for (j = 0; j < num_features; j++)
        {
            double t = feature_mp->elements[ i ][ j ];

            if (IS_NOT_MISSING_DBL(t))
            {
                sum_sqr_vp->elements[ j ] += t * t;
                (non_missing_count_vp->elements[ j ])++;
            }
        }
    }

    if (result != ERROR)
    {
        result = get_target_vector(&stdev_vp, num_features);
    }

    if (result != ERROR)
    {
        for (j = 0; j < num_features; j++)
        {
            if (non_missing_count_vp->elements[ j ] > 1)
            {
                stdev_vp->elements[ j ] = sum_sqr_vp->elements[ j ] /
                                 (non_missing_count_vp->elements[ j ] - 1.0);

            }
            else
            {
                stdev_vp->elements[ j ] = DBL_MISSING;
            }
        }
    }

    if ((result != ERROR) && (num_disabled_features > 0))
    {
        for (i = 0; i<num_features; i++)
        {
            if (disable_normalize_vp->elements[ i ] != 0)
            {
                stdev_vp->elements[ i ] = norm_stdev * norm_stdev;
            }
        }
    }

    if (result != ERROR)
    {
        for (i = 0; i<num_features; i++)
        {
            /* stdev_vp is currently variance. */
            if (    (IS_NOT_MISSING_DBL(stdev_vp->elements[ i ]))
                 && (stdev_vp->elements[ i ] <= MIN_ALLOWABLE_DATA_VAR)
               )
            {
                warn_pso("Data being normalized has a column (%d) with small or negative variance (%.4e).\n",
                         i, stdev_vp->elements[ i ]);
                warn_pso("This is less than the current cutoff (%.4e).\n",
                         MIN_ALLOWABLE_DATA_VAR);
                warn_pso("This is likley due to a particular feature having (nearly) the same value (not useful).\n");
                warn_pso("Skipping normalization of column %d.\n\n", i);

                stdev_vp->elements[ i ] = norm_stdev * norm_stdev;
            }
        }
    }

    if ((result != ERROR) && (var_vpp != NULL))
    {
        /* stdev_vp has not been square rooted yet. */
        result = copy_vector(var_vpp, stdev_vp);
    }

    if (result != ERROR)
    {
        result = ow_sqrt_vector(stdev_vp);

        if (result == ERROR)
        {
            add_error("Likely due to normalizing data with little variance.");
        }
    }

    if (result != ERROR)
    {
        result = ow_divide_matrix_by_row_vector(feature_mp, stdev_vp);

        if (result == ERROR)
        {
            add_error("Likely due to normalizing data with little variance.");
        }
    }

    if (result != ERROR)
    {
        result = ow_multiply_matrix_by_scalar(feature_mp, norm_stdev);
    }

    if (num_held_out_points > 0)
    {
        if (result != ERROR)
        {
            result = ow_divide_matrix_by_row_vector(held_out_feature_mp,
                                                    stdev_vp);
        }

        if (result != ERROR)
        {
            result = ow_multiply_matrix_by_scalar(held_out_feature_mp,
                                                  norm_stdev);
        }
    }

    free_vector(stdev_vp);
    free_vector(mean_vp);
    free_vector(sum_sqr_vp);

    free_vector(data_min_vp);
    free_vector(data_max_vp);
    free_vector(data_max_min_diff_vp);

    free_int_vector(non_missing_count_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_un_normalize_cluster_data
(
    Matrix*       model_mean_mp,
    Matrix*       model_var_mp,
    const Vector* mean_vp,
    const Vector* var_vp,
    double        norm_stdev
)
{
    Vector* stdev_vp     = NULL;

    ERE(sqrt_vector(&stdev_vp, var_vp));

    if (model_var_mp != NULL)
    {
        /*
        // Un-normalize variances
        */
        ERE(ow_multiply_matrix_by_row_vector_ew(model_var_mp,
                                                var_vp));
        ERE(ow_divide_matrix_by_scalar(model_var_mp,
                                       norm_stdev * norm_stdev));
    }

    if (model_mean_mp != NULL)
    {
        /*
        // Un-normalize means
        */
        ERE(ow_multiply_matrix_by_row_vector_ew(model_mean_mp,
                                                stdev_vp));
        ERE(ow_divide_matrix_by_scalar(model_mean_mp, norm_stdev));
        ERE(ow_add_row_vector_to_matrix(model_mean_mp, mean_vp));
    }

    free_vector(stdev_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int plot_log_likelihood
(
    const Vector* log_likelihood_sum_vp,
    const Vector* counts_vp,
    const Vector* min_log_likelihood_vp,
    const Vector* max_log_likelihood_vp,
    const char*   output_dir,
    const char*   title_str
)
{
    int     max_num_clusters             = log_likelihood_sum_vp->length;
    int     i;
    int     count                        = 0;
    char    plot_file_path[ 1000 ];
    char    plot_file_name[ 1000 ];
    char    title[ 1000 ];
    int     plot_id = NOT_SET;
    Vector* log_likelihood_vp            = NULL;
    Vector* actual_min_log_likelihood_vp = NULL;
    Vector* actual_max_log_likelihood_vp = NULL;
    Vector* log_num_cluster_vp           = NULL;
    int     result                       = NO_ERROR;


    for (i=0; i < max_num_clusters; i++)
    {
        if (counts_vp->elements[ i ] > 0.5) count++;
    }

    if (    (get_target_vector(&log_likelihood_vp, count) == ERROR)
         || (get_target_vector(&actual_min_log_likelihood_vp, count) == ERROR)
         || (get_target_vector(&actual_max_log_likelihood_vp, count) == ERROR)
         || (get_target_vector(&log_num_cluster_vp, count) == ERROR)
       )
    {
        result = ERROR;
    }

    if (result != ERROR)
    {
        count = 0;

        for (i=0; i < max_num_clusters; i++)
        {
            if (counts_vp->elements[ i ] > 0.5)
            {
                log_likelihood_vp->elements[ count ] =
                        log_likelihood_sum_vp->elements[ i ] /
                                                       counts_vp->elements[ i ];
                log_num_cluster_vp->elements[ count ] = log((double)i);

                actual_min_log_likelihood_vp->elements[ count ] =
                                          min_log_likelihood_vp->elements[ i ];
                actual_max_log_likelihood_vp->elements[ count ] =
                                          max_log_likelihood_vp->elements[ i ];

                count++;
            }
        }

        plot_id = plot_open();

        if (plot_id == ERROR) result = ERROR;
    }

    if (result != ERROR)
    {
        BUFF_CPY(title,
                 "MDL adjusted log likelihood vs log number of clusters");

        if (title_str != NULL)
        {
            BUFF_CAT(title, " for ");
            BUFF_CAT(title, title_str);
        }

        EPE(plot_set_title(plot_id, title, 0 , 0));
        EPE(plot_set_x_legend(plot_id, "Log number of clusters"));
        EPE(plot_set_y_legend(plot_id, "Adjusted log likelihood"));

        EPE(plot_curve(plot_id, log_num_cluster_vp, log_likelihood_vp,
                       "Average adjusted log likelihood"));
        EPE(plot_curve(plot_id, log_num_cluster_vp,
                       actual_min_log_likelihood_vp,
                       "Min adjusted log likelihood"));
        EPE(plot_curve(plot_id, log_num_cluster_vp,
                       actual_max_log_likelihood_vp,
                       "Max adjusted log likelihood"));
        EPE(plot_update(plot_id));

        if (output_dir != NULL)
        {
            BUFF_CPY(plot_file_path, output_dir);
            BUFF_CAT(plot_file_path, DIR_STR);
        }
        else
        {
            plot_file_path[ 0 ] = '\0';
        }

        if (title_str != NULL)
        {
            BUFF_CPY(plot_file_name, title_str);
            BUFF_CAT(plot_file_name, ".ps");
            char_for_char_translate(plot_file_name, ' ', '_');
        }
        else
        {
            BUFF_CPY(plot_file_name, "plot.ps");
        }

        BUFF_CAT(plot_file_path, plot_file_name);

        EPE(save_plot(CURRENT_PLOT, plot_file_path));
    }

    free_vector(log_num_cluster_vp);
    free_vector(log_likelihood_vp);
    free_vector(actual_max_log_likelihood_vp);
    free_vector(actual_min_log_likelihood_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

