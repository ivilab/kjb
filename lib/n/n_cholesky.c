
/* $Id: n_cholesky.c 17895 2014-10-24 23:38:49Z ksimek $ */

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

#include "n/n_cholesky.h"
#include "wrap_lapack/wrap_lapack.h"

#ifdef __cplusplus
extern "C" {
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* -------------------------------------------------------------------------- */

static Method_option fs_cholesky_methods[ ] =
{
    { "native",                 "native",    (int(*)())do_native_cholesky_decomposition},
    { "lapack",              "lapack", (int(*)())do_lapack_cholesky_decomposition_2}
};

static const int fs_num_cholesky_methods =
                                sizeof(fs_cholesky_methods) /
                                          sizeof(fs_cholesky_methods[ 0 ]);

static int         fs_cholesky_method                  = 0;
static const char* fs_cholesky_method_option_short_str =
                                                       "cholesky";
static const char* fs_cholesky_method_option_long_str =
                                                      "cholesky-method";

/* -------------------------------------------------------------------------- */

int set_cholesky_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result           = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option,
                           fs_cholesky_method_option_short_str)
          || match_pattern(lc_option,
                           fs_cholesky_method_option_long_str)
       )
    {
        if (value == NULL) return NO_ERROR;

        ERE(parse_method_option(fs_cholesky_methods,
                                fs_num_cholesky_methods,
                                fs_cholesky_method_option_long_str,
                                "cholesky decomposition method", value,
                                &fs_cholesky_method));
        result = NO_ERROR;
    }

    return result;
}

/* =============================================================================
 *                          cholesky_decomposition
 *
 * Performs Cholesky decomposition on a matrix, using a method hand-coded for 
 * the KJB library.
 *
 * This routine finds matrix L such that LL' = A. A must be positive definite.
 *
 * The first argument is the adress of the target matrix.  If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is. 
 *
 * If input and output matrices point to the same address, a temporary matrix 
 * will be allocated as a workspace, which will swapped with A before returning.
 *
 * Passing NULL as the source matrix will free L, set it to NULL and return 
 * NO_ERROR.
 *
 * The operation may be performed in one of several ways. The method used can be
 * set using the option "cholesky-method" which is normally exposed to
 * the user. If this options is "native" (the default), then we compute the
 * Cholesky decomposition using a native KJB implementation.  If this option 
 * is "lapack," Cholesky decomposition is perfomed by Lapack's DBOTRF subroutine.
 * The lapack version is slightly more forgiving on ill-conditioned matrices.
 *
 * Returns:
 *    Returns ERROR on failure, setting the the errors string accordingly, and 
 *    NO_ERROR otherwise.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: matrices, matrix decomposition
 *
 * -----------------------------------------------------------------------------
*/
int cholesky_decomposition(Matrix** target_mpp, const Matrix* input_mp)
{
    int result;
    if (    (fs_cholesky_method < 0)
         || (fs_cholesky_method >= fs_num_cholesky_methods)
       )
    {
        SET_CANT_HAPPEN_BUG();
        result = ERROR;
    }
    else if (    (input_mp == NULL)
              || (input_mp->num_cols < 1)
              || (input_mp->num_rows < 1)
              || (input_mp->num_rows != input_mp->num_cols)
            )
    {
        SET_ARGUMENT_BUG();
        result = ERROR;
    }
    else
    {
        int (*cholesky_fn) (Matrix**, const Matrix*)
        /* Kobus: 05/01/22: Be very pedantic here, to keep C++ happy.  */
            = (int (*) (Matrix**, const Matrix*)) fs_cholesky_methods[ fs_cholesky_method ].fn;

        result = cholesky_fn(target_mpp, input_mp);

        if (result == ERROR)
        {
            add_error("Error occured while inverting a %d by %d matrix.",
                      input_mp->num_rows, input_mp->num_cols);
        }
    }

    return result;
}

/* =============================================================================
 *                          do_native_cholesky_decomposition
 *
 * Performs Cholesky decomposition on a matrix, using a method hand-coded for 
 * the KJB library.
 *
 * This routine finds matrix L such that LL' = A. A must be positive definite.
 *
 * The first argument is the adress of the target matrix.  If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is. 
 *
 * If input and output matrices point to the same address, a temporary matrix 
 * will be allocated as a workspace, which will swapped with A before returning.
 *
 * Passing NULL as the source matrix will free L, set it to NULL and return 
 * NO_ERROR.
 *
 * Returns:
 *    Returns ERROR on failure, setting the the errors string accordingly, and 
 *    NO_ERROR otherwise.
 *
 * Note:
 *    This function sometimes returns an "not positive definite" error if the 
 *    matrix is too ill-conditioned.  The lapack version is slightly more 
 *    forgiving in this regard.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: matrices, matrix decomposition
 *
 * -----------------------------------------------------------------------------
*/
int do_native_cholesky_decomposition(Matrix** L, const Matrix* A)
{
    Matrix* Lp = NULL;
    double sum;
    Matrix** in_place_ptr = NULL;

    int i, j, k;

    if(A == NULL)
    {
        if(*L != NULL)
            free_matrix(*L);
        *L = NULL;
        return NO_ERROR;
    }
    if(A == *L)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(L != NULL && A == *L)
    {
        in_place_ptr = L;
        L = NULL;
    }

    ERE(get_zero_matrix(L, A->num_rows, A->num_cols));
    Lp = *L;

    for(i = 0; i < A->num_rows; i++)
    {
        for(j = 0; j <= i; j++)
        {
            if(i == j)
            {
                sum = 0;
                for(k = 0; k < i; k++)
                {
                    sum += Lp->elements[i][k] * Lp->elements[i][k];
                }

                if(A->elements[i][j] < sum)
                {
                    set_error("Cholesky decomposition: matrix is not positive definite.");
                    free_matrix(*L);
                    *L = NULL;
                    return ERROR;
                }
                Lp->elements[i][j] = sqrt(A->elements[i][j] - sum);
            }
            else
            {
                sum = 0;
                for(k = 0; k < j; k++)
                {
                    sum += Lp->elements[i][k] * Lp->elements[j][k];
                }
                Lp->elements[i][j] = (Lp->elements[j][j] != 0) ? (A->elements[i][j] - sum) / Lp->elements[j][j] : 0;
            }
        }
    }

    if(in_place_ptr)
    {
        free_matrix(*in_place_ptr);
        *in_place_ptr = Lp;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

