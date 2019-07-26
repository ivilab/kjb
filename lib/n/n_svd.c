
/* $Id: n_svd.c 10388 2011-09-18 08:48:20Z kobus $ */

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

#include "n/n_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "wrap_lapack/wrap_lapack.h"
#include "n/n_svd.h"

#ifdef KJB_HAVE_NUMERICAL_RECIPES
#include "nr/nr_svd.h"
#endif

/* For declared function pointers. Generally harmless. */
#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int initialize_svd_method(void);

/* -------------------------------------------------------------------------- */

static Method_option fs_svd_methods[ ] =
{
#ifdef KJB_HAVE_NUMERICAL_RECIPES
    { "numerical-recipes", "nr",  (int(*)())do_numerical_recipes_svd},
#endif
    { "lapack",            "lp",  (int(*)())do_lapack_svd}
};

static const int fs_num_svd_methods = sizeof(fs_svd_methods) /
                                                      sizeof(fs_svd_methods[ 0 ]);
static int         fs_svd_method                  = NEVER_SET;
static const char* fs_svd_method_option_short_str = "svd";
static const char* fs_svd_method_option_long_str  = "svd-method";

static double fs_max_matrix_condition_number = 1.0 / (1000.0 * DBL_EPSILON);


#ifdef HOW_IT_WAS_2010_06_27
/*
 * Kobus: This might have been a way to do this, but no other similar module
 *        follows this convention. So we just use what was set_svd_options_2()
 *        as set_svd_options(). 
*/

/* -------------------------------------------------------------------------- */

static int set_svd_options_2(const char* option, const char* value);

/* -------------------------------------------------------------------------- */

int set_svd_options(const char* option, const char* value)
{
    int result      = NOT_FOUND;
    int temp_result;
    int print_title = FALSE;


    if (    (is_pattern(option))
         && (value != NULL)
         && ((value[ 0 ] == '\0') || (value[ 0 ] == '?'))
       )
    {
        /* Dry run */

        ERE(temp_result = set_svd_options_2(option, (char*)NULL));
        if (temp_result == NO_ERROR) result = NO_ERROR;

        if (result == NOT_FOUND) return result;

        print_title = TRUE;

        ERE(pso("\n"));
        ERE(set_high_light(stdout));
        ERE(pso("SVD options"));
        ERE(unset_high_light(stdout));
        ERE(pso("\n"));
    }

    result = set_svd_options_2(option, value);

    if (print_title)
    {
        ERE(pso("\n"));
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int set_svd_options_2(const char* option, const char* value)
#else
int set_svd_options(const char* option, const char* value)
#endif 
{
    char lc_option[ 100 ];
    int  result           = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "max-matrix-condition-number")
       )
    {
        double temp_real_value;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("The maximimum allowed matrix condition number is %.3e.\n",
                    fs_max_matrix_condition_number));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("max-matrix-condition-number = %.4e\n",
                    fs_max_matrix_condition_number));
        }
        else
        {
            ERE(ss1snd(value, &temp_real_value));
            fs_max_matrix_condition_number = temp_real_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, fs_svd_method_option_short_str)
          || match_pattern(lc_option, fs_svd_method_option_long_str)
       )
    {
        if (value == NULL) return NO_ERROR;

        ERE(initialize_svd_method());

        ERE(parse_method_option(fs_svd_methods, fs_num_svd_methods,
                                fs_svd_method_option_long_str,
                                "SVD method", value, &fs_svd_method));
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int initialize_svd_method(void)
{
    if (fs_svd_method == NEVER_SET)
    {
#ifdef KJB_HAVE_NUMERICAL_RECIPES
        const char* default_method =
                                have_lapack() ? "lapack" : "numerical-recipes";
#else
        const char* default_method = "lapack";
#endif

        if (parse_method_option(fs_svd_methods, fs_num_svd_methods,
                                fs_svd_method_option_long_str,
                                "SVD method", default_method, &fs_svd_method)
            == ERROR)
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  do_svd
 *
 * Calculates the (reduced) SVD
 *
 * Given a matrix A, this routine computes A = U*D*VT, where D is diagonal,
 * and contains the singular values of A. The reduced SVD is computed, so,
 * letting m be the number of rows in A, n be the number of columns in A, and
 * letting r=min(m,n), then the dimensions of U are mxr, the dimensions of D are
 * rxn, and the dimensions of VT are nxn. (D is actually returned as a
 * vector of length r, and if D is needed as a matrix, the caller will have to
 * put the vector into a zero matrix of dimension rxn).
 *
 * The matrices U and VT are put into *u_mpp and *vt_mpp, respectively,
 * which are created or resized as needed. The diagonal matrix of singular
 * values is put into the vector pointed to by *d_vpp, which is also created
 * or resized as needed. Also, an estimate of the rank of A is put into
 * *rank_ptr. This is the number of singular values of A such that the ratio of
 * the first singular value to the singular values is less than the maximum
 * allowed condition number. The maximum allowed condition number is normally
 * exposed to the user through the option "max-matrix-condition-number". (The
 * condition number of matrix is the raio of the largest (first) singular value
 * to the smallest (last) singular value). If you are not interested in one or
 * more of U, D, VT, or rank, then the corresponding pointer argument may be set
 * to NULL.
 *
 * The method used to compute the svd is also user settable. The default method
 * is to use the LAPACK routine, if it is available. If the LAPACK library is
 * not available, then code from numerical recipes in C is used, provided that
 * it is available. Note that the terms of use for numerical recipes excludes
 * making those routines available in code exported to the world.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: SVD, singular value decomposition, matrix decomposition
 *
 * -----------------------------------------------------------------------------
*/

int do_svd
(
    const Matrix* a_mp,
    Matrix**      u_mpp,
    Vector**      d_vpp,
    Matrix**      v_trans_mpp,
    int*          rank_ptr
)
{
    int result = NO_ERROR;  /* Avoid warning message. */


    ERE(initialize_svd_method());

    if (fs_svd_method == NOT_SET)
    {
        set_error("No svd method is currently set.");
        result = ERROR;
    }
    else if ((fs_svd_method < 0) || (fs_svd_method >= fs_num_svd_methods))
    {
        SET_CANT_HAPPEN_BUG();
        result = ERROR;
    }
    else
    {
        Vector*  d_vp             = NULL;
        Vector** derived_d_vpp;
        int (*svd_fn)(const Matrix*, Matrix**, Vector**, Matrix**)
            /* Kobus: 05/01/22: Be very pedantic here, to keep C++ happy.  */
            = (int(*)(const Matrix*, Matrix**, Vector**, Matrix**))fs_svd_methods[ fs_svd_method ].fn;

        if ((d_vpp == NULL) && (rank_ptr != NULL))
        {
            derived_d_vpp = &d_vp;
        }
        else
        {
            derived_d_vpp = d_vpp;
        }

        result = (*svd_fn)(a_mp, u_mpp, derived_d_vpp, v_trans_mpp);

        if (result == ERROR)
        {
            add_error("Error occured while using SVD method: %s",
                      fs_svd_methods[ fs_svd_method ].long_name);
        }

        if ((result != ERROR) && (rank_ptr != NULL))
        {
            Vector* derived_d_vp = *derived_d_vpp;
            double  s1 = derived_d_vp->elements[ 0 ];
            int     i;

            for (i=1; i<derived_d_vp->length; i++)
            {
                double s2 = derived_d_vp->elements[ i ];

                if (s1 >= s2 * fs_max_matrix_condition_number) break;
            }

            *rank_ptr = i;
        }

        free_vector(d_vp);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            get_determinant_abs
 *
 * Calculates the absolute value of the determinant of a matrix
 *
 * This routine uses SVD to compute the absolute value of the determinant of a
 * matrix.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: SVD, singular value decomposition, matrix decomposition
 *
 * -----------------------------------------------------------------------------
*/

int get_determinant_abs(const Matrix* mp, double* det_ptr)
{
    Vector* vp = NULL;


    ERE(do_svd(mp, (Matrix**)NULL, &vp, (Matrix**)NULL, (int*)NULL));

    *det_ptr = multiply_vector_elements(vp);

    free_vector(vp);

    return NO_ERROR;
}

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

