
/* $Id: wrap_lapack.c 21596 2017-07-30 23:33:36Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 1994-2008 by members of University of Arizona Computer Vision|
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


#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "wrap_lapack/wrap_lapack_gen.h"  
#include "wrap_lapack/wrap_lapack.h"

#ifdef KJB_HAVE_LAPACK

#define HAVE_LAPACK_ARGS

#ifdef HPUX
#    define dgesvd_ dgesvd
#    define dgetri_ dgetri
#    define dgetrf_ dgetrf
#    define ddot_ ddot
#endif

#ifdef MAC_OSX 
#    ifndef __C2MAN__
#        define __ACCELERATE__
#        include <Accelerate/Accelerate.h>
#    endif 

#    ifdef __CLAPACK_H
#        ifndef LAPACK_USES_C_CONVENTIONS
#            define LAPACK_USES_C_CONVENTIONS
#        endif 

#        define lapack_integer __CLPK_integer
#    else 
#        define lapack_integer int
#    endif 
#
#    /* This should, at a minimum, be available in include_after. */
#    include "lapack.h"
#else 
#    ifdef LAPACK_IS_ACML
#        include "acml.h"
#    else
#        ifdef LAPACK_IS_MKL
#            ifndef LAPACK_USES_C_CONVENTIONS
#                define LAPACK_USES_C_CONVENTIONS
#            endif 

#            include "mkl_lapack.h"
#        else 
#            include "lapack.h"
#        endif 
#    endif 

#    define lapack_integer int
#endif 

#else 

#ifdef __C2MAN__
#    define HAVE_LAPACK_ARGS
#endif 

#endif

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifndef KJB_HAVE_LAPACK
/* -----------------------------------------------------------------------------
|                              no LAPACK
|                                  ||
|                                 \||/
|                                  \/
*/
static void set_dont_have_lapack_error(void)
{
    set_error("Operation failed because the program was built without ");
    cat_error("the LAPACK interface.");
    add_error("(Appropriate re-compiling is needed to fix this.)");
}
/*
|                                  /\
|                                 /||\
|                                  ||
|                              no LAPACK
----------------------------------------------------------------------------- */
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int have_lapack(void)
{
#ifdef KJB_HAVE_LAPACK
    return TRUE;
#else
    return FALSE;
#endif
}


/* 
 * called from lapack_solve_triangular and lapack_solver_upper_triangular.  
 * passing 'U' (resp. 'L') to upper_lower assumes A_mp is upper (lower) triangular.
 */
#ifdef HAVE_LAPACK_ARGS
static int lapack_solve_triangular_general(
        const Matrix* A_mp,
        const Matrix* B_mp,
        Matrix** X_mpp, 
        char upper_lower)
#else 
/*ARGSUSED*/
static int lapack_solve_triangular_general
(
    const Matrix* __attribute__((unused)) dummy_A_mp,
    const Matrix* __attribute__((unused)) dummy_B_mp,
    Matrix**      __attribute__((unused)) dummy_X_mpp,
    char     __attribute__((unused)) dummy_upper_lower
)
#endif 
{
#ifdef KJB_HAVE_LAPACK
    char UPLO[ 1 ]; /* L: lower triangular, U: upper triangular*/
    char TRANS[ 1 ]   = { 'N' }; /* no transpose */
    char DIAG[ 1 ]   = { 'N' }; /* not unit triangular */
    lapack_integer N;
    lapack_integer NRHS;
    lapack_integer LDA;
    lapack_integer LDB;
    lapack_integer INFO;
    int result = NO_ERROR;
    double*   A;
    double*   B;

#ifndef LAPACK_USES_C_CONVENTIONS
    const int      uplo_len = 1;
    const int      trans_len = 1;
    const int      diag_len = 1;
#endif 

    UPLO[0] = upper_lower;


    NRE(A = get_fortran_1D_dp_array_from_matrix(A_mp));
    NRE(B = get_fortran_1D_dp_array_from_matrix(B_mp));

    N = B_mp->num_rows;
    NRHS = B_mp->num_cols;

    LDA = N;
    LDB = N;
    
    if(A_mp->num_rows != A_mp->num_cols)
    {
        set_error("Unable to solve linear equation: A must be square");
        return ERROR;
    }

    if(A_mp->num_rows != N)
    {
        set_error("Unable to solve linear equation: Dimension mismatch");
        result = ERROR;
    }

    if(result != ERROR)
    {
        dtrtrs_(UPLO, TRANS, DIAG, &N, &NRHS, A, &LDA, B, &LDB, &INFO
#ifndef LAPACK_USES_C_CONVENTIONS
               , uplo_len, trans_len, diag_len
#endif
                );

        if(INFO < 0)
        {
            set_error("Problem with value %ld of LAPACK routine dtrtrs: illegal value.", -(long)INFO);
            result = ERROR;
        }
        else if(INFO > 0)
        {
            set_error("Linear solver failed: matrix is singular.");
            result = ERROR;
        }
    }

    if ((result != ERROR) && (X_mpp != NULL))
    {
        result = get_matrix_from_fortran_1D_dp_array(X_mpp, N, NRHS, B);
    }

    kjb_free(A);
    kjb_free(B);

    if (result == ERROR)
    {
        add_error("Problem occured using LAPACK routine DTRTRS for solving diagonal linear system.");
    }

    return result;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              KJB_HAVE_LAPACK
    ------------------------------------------------------------------------- */
#else
    /* -------------------------------------------------------------------------
    |                              no LAPACK
    |                                  ||
    |                                 \||/
    |                                  \/
    */
    set_dont_have_lapack_error();
    return ERROR;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              no LAPACK
    ------------------------------------------------------------------------- */
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/* =============================================================================
 *                             lapack_solve_triangular
 *
 * Uses lapack to solve the linear equation 
 * |    A * X = B 
 * where A is lower-triangular.  Avoids expensive inversion and/or factorization
 * required in the general problem.
 *
 * If this routine was not compiled with lapack available, then it returns
 * ERROR with an appropriate error being set.
 * 
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: solve, lapack
 *
 * -----------------------------------------------------------------------------
*/
int lapack_solve_triangular(
        const Matrix* A_mp,
        const Matrix* B_mp,
        Matrix** X_mpp)
{
    return lapack_solve_triangular_general(A_mp, B_mp, X_mpp, 'L');
}

/* =============================================================================
 *                             lapack_solve_upper_triangular
 *
 * Uses lapack to solve the linear equation 
 * |    A * X = B 
 * where A is upper-triangular.  Avoids expensive inversion and/or factorization
 * required in the general problem.
 *
 * If this routine was not compiled with lapack available, then it returns
 * ERROR with an appropriate error being set.
 * 
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: solve, lapack
 *
 * -----------------------------------------------------------------------------
*/
int lapack_solve_upper_triangular(
        const Matrix* A_mp,
        const Matrix* B_mp,
        Matrix** X_mpp)
{
    return lapack_solve_triangular_general(A_mp, B_mp, X_mpp, 'U');
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             lapack_solve_symmetric
 *
 * Uses lapack to solve the linear equation 
 * |    A * X = B 
 * where A is symmetric indefinite.  If A is symmetric positive definite,
 * use lapack_solve_symmetric_pd.   This should be much more numerically stable
 * and possibly more efficient than inverting A.
 *
 * If this routine was not compiled with lapack available, then it returns
 * ERROR with an appropriate error being set.
 * 
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: solve, lapack
 *
 * -----------------------------------------------------------------------------
*/
#ifdef HAVE_LAPACK_ARGS
int lapack_solve_symmetric(
        const Matrix* A_mp,
        const Matrix* B_mp,
        Matrix** X_mpp)
#else 
/*ARGSUSED*/
int lapack_solve_symmetric
(
    const Matrix* __attribute__((unused)) dummy_A_mp,
    const Matrix* __attribute__((unused)) dummy_B_mp,
    Matrix**      __attribute__((unused)) dummy_X_mpp_
)
#endif 
{
#ifdef KJB_HAVE_LAPACK
    char UPLO[ 1 ] = { 'L' }; /* L: lower triangle  is used*/
    lapack_integer N;
    lapack_integer NRHS;
    lapack_integer LDA;
    lapack_integer LDB;
    lapack_integer INFO;
    lapack_integer LWORK;
    lapack_integer*   IPIV_ptr = NULL;
    int result = NO_ERROR;
    double*   A = NULL;
    double*   B = NULL;
    double*   WORK = NULL;

#ifndef LAPACK_USES_C_CONVENTIONS
    const int      uplo_len = 1;
#endif 

    N = B_mp->num_rows;
    NRHS = B_mp->num_cols;

    LDA = N;
    LDB = N;

    NRE(A = get_fortran_1D_dp_array_from_matrix(A_mp));
    NRE(B = get_fortran_1D_dp_array_from_matrix(B_mp));
    NRE(IPIV_ptr = N_TYPE_MALLOC(lapack_integer, N));
    LWORK = N * 64; /* 64 is optimal block size */
    NRE(WORK = DBL_MALLOC(LWORK));
    
    if(A_mp->num_rows != A_mp->num_cols)
    {
        set_error("Unable to solve linear equation: A must be square");
        return ERROR;
    }

    if(A_mp->num_rows != N)
    {
        set_error("Unable to solve linear equation: Dimension mismatch");
        result = ERROR;
    }

    if(result != ERROR)
    {
        dsysv_(UPLO, &N, &NRHS, A, &LDA, IPIV_ptr, B, &LDB, WORK, &LWORK, &INFO
#ifndef LAPACK_USES_C_CONVENTIONS
               , uplo_len
#endif
                );

        if(INFO < 0)
        {
            set_error("Problem with value %ld of LAPACK routine dsysv: illegal value.", -(long)INFO);
            result = ERROR;
        }
        else if(INFO > 0)
        {
            set_error("Linear solver failed: matrix is singular.");
            result = ERROR;
        }
    }

    if ((result != ERROR) && (X_mpp != NULL))
    {
        result = get_matrix_from_fortran_1D_dp_array(X_mpp, N, NRHS, B);
    }

    kjb_free(A);
    kjb_free(B);
    kjb_free(WORK);
    kjb_free(IPIV_ptr);

    if (result == ERROR)
    {
        add_error("Problem occured using LAPACK routine DSYSV for solving symmetric linear system.");
    }

    return result;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              KJB_HAVE_LAPACK
    ------------------------------------------------------------------------- */
#else
    /* -------------------------------------------------------------------------
    |                              no LAPACK
    |                                  ||
    |                                 \||/
    |                                  \/
    */
    set_dont_have_lapack_error();
    return ERROR;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              no LAPACK
    ------------------------------------------------------------------------- */
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             lapack_solve_symmetric_pd
 *
 * Uses lapack to solve the linear equation 
 * |    A * X = B 
 * where A is symmetric and positive definite.  If A is symmetric but not positive
 * definite, use lapack_solve_symmetric.   This should be much more numerically stable
 * and possibly more efficient than inverting A.
 *
 * If this routine was not compiled with lapack available, then it returns
 * ERROR with an appropriate error being set.
 * 
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: solve, lapack
 *
 * -----------------------------------------------------------------------------
*/
#ifdef HAVE_LAPACK_ARGS
int lapack_solve_symmetric_pd(
        const Matrix* A_mp,
        const Matrix* B_mp,
        Matrix** X_mpp)
#else 
/*ARGSUSED*/
int lapack_solve_symmetric_pd
(
    const Matrix* __attribute__((unused)) dummy_A_mp,
    const Matrix* __attribute__((unused)) dummy_B_mp,
    Matrix**      __attribute__((unused)) dummy_X_mpp
)
#endif 
{
#ifdef KJB_HAVE_LAPACK
    char UPLO[ 1 ] = { 'L' }; /* L: lower triangle  is used*/
    lapack_integer N;
    lapack_integer NRHS;
    lapack_integer LDA;
    lapack_integer LDB;
    lapack_integer INFO;
    int result = NO_ERROR;
    double*   A = NULL;
    double*   B = NULL;

#ifndef LAPACK_USES_C_CONVENTIONS
    const int      uplo_len = 1;
#endif 

    N = B_mp->num_rows;
    NRHS = B_mp->num_cols;

    LDA = N;
    LDB = N;

    NRE(A = get_fortran_1D_dp_array_from_matrix(A_mp));
    NRE(B = get_fortran_1D_dp_array_from_matrix(B_mp));
    
    if(A_mp->num_rows != A_mp->num_cols)
    {
        set_error("Unable to solve linear equation: A must be square");
        return ERROR;
    }

    if(A_mp->num_rows != N)
    {
        set_error("Unable to solve linear equation: Dimension mismatch");
        result = ERROR;
    }

    if(result != ERROR)
    {
        dposv_(UPLO, &N, &NRHS, A, &LDA, B, &LDB, &INFO
#ifndef LAPACK_USES_C_CONVENTIONS
               , uplo_len
#endif
                );

        if(INFO < 0)
        {
            set_error("Problem with value %ld of LAPACK routine DPOSV: illegal value.", -(long)INFO);
            result = ERROR;
        }
        else if(INFO > 0)
        {
            set_error("Linear solver failed: matrix is singular.");
            result = ERROR;
        }
    }

    if ((result != ERROR) && (X_mpp != NULL))
    {
        result = get_matrix_from_fortran_1D_dp_array(X_mpp, N, NRHS, B);
    }

    kjb_free(A);
    kjb_free(B);

    if (result == ERROR)
    {
        add_error("Problem occured using LAPACK routine DPOSV for solving symmetric PD linear system.");
    }

    return result;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              KJB_HAVE_LAPACK
    ------------------------------------------------------------------------- */
#else
    /* -------------------------------------------------------------------------
    |                              no LAPACK
    |                                  ||
    |                                 \||/
    |                                  \/
    */
    set_dont_have_lapack_error();
    return ERROR;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              no LAPACK
    ------------------------------------------------------------------------- */
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             lapack_solve
 *
 * Uses lapack to solve the linear equation 
 * |    A * X = B 
 * where A is a general nonsingular square matrix.  This should be much more numerically stable
 * and possibly more efficient than inverting A.
 *
 * If this routine was not compiled with lapack available, then it returns
 * ERROR with an appropriate error being set.
 * 
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: solve, lapack
 *
 * -----------------------------------------------------------------------------
*/
#ifdef HAVE_LAPACK_ARGS
int lapack_solve
(
        const Matrix* A_mp,
        const Matrix* B_mp,
        Matrix** X_mpp)
#else 
/*ARGSUSED*/
int lapack_solve
(
    const Matrix* __attribute__((unused)) dummy_A_mp,
    const Matrix* __attribute__((unused)) dummy_B_mp,
    Matrix**      __attribute__((unused)) dummy_X_mpp)
#endif 
{
#ifdef KJB_HAVE_LAPACK
    lapack_integer N;
    lapack_integer NRHS;
    lapack_integer LDA;
    lapack_integer LDB;
    lapack_integer INFO;
    lapack_integer* IPIV_ptr;
    int result = NO_ERROR;
    double*   A = NULL;
    double*   B = NULL;

    N = B_mp->num_rows;
    NRHS = B_mp->num_cols;

    LDA = N;
    LDB = N;

    NRE(A = get_fortran_1D_dp_array_from_matrix(A_mp));
    NRE(B = get_fortran_1D_dp_array_from_matrix(B_mp));
    NRE(IPIV_ptr = N_TYPE_MALLOC(lapack_integer, N));

    if(A_mp->num_rows != A_mp->num_cols)
    {
        set_error("Unable to solve linear equation: A must be square");
        return ERROR;
    }

    if(A_mp->num_rows != N)
    {
        set_error("Unable to solve linear equation: Dimension mismatch");
        result = ERROR;
    }

    if(result != ERROR)
    {
        dgesv_(&N, &NRHS, A, &LDA, IPIV_ptr, B, &LDB, &INFO);

        if(INFO < 0)
        {
            set_error("Problem with value %ld of LAPACK routine DGESV: illegal value.", -(long)INFO);
            result = ERROR;
        }
        else if(INFO > 0)
        {
            set_error("Linear solver failed: matrix is singular.");
            result = ERROR;
        }
    }

    if ((result != ERROR) && (X_mpp != NULL))
    {
        result = get_matrix_from_fortran_1D_dp_array(X_mpp, N, NRHS, B);
    }

    kjb_free(A);
    kjb_free(B);
    kjb_free(IPIV_ptr);

    if (result == ERROR)
    {
        add_error("Problem occured using LAPACK routine DGESV for solving linear system.");
    }

    return result;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              KJB_HAVE_LAPACK
    ------------------------------------------------------------------------- */
#else
    /* -------------------------------------------------------------------------
    |                              no LAPACK
    |                                  ||
    |                                 \||/
    |                                  \/
    */
    set_dont_have_lapack_error();
    return ERROR;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              no LAPACK
    ------------------------------------------------------------------------- */
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             lapack_qr_decompose
 *
 * Uses lapack to do eigendecomposition.
 *
 * This routine is the wrapper for lapack eigendecomposition. 
 * Normally we don't call this routine directly, but through diagonalize(). 
 *
 * If this routine was not compiled with lapack available, then it returns
 * ERROR with an appropriate error being set.
 * 
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: matrix decomposition, lapack
 * -----------------------------------------------------------------------------
*/
#ifdef HAVE_LAPACK_ARGS
int lapack_diagonalize
(
     const Matrix* mp,
     Matrix** E_mpp,
     Vector** D_vpp
)
#else 
/*ARGSUSED*/
int lapack_diagonalize
(
    const Matrix* __attribute__((unused)) dummy_mp,
    Matrix**      __attribute__((unused)) dummy_E_mpp,
    Vector**      __attribute__((unused)) dummy_D_vpp
)
#endif 
{
#ifdef KJB_HAVE_LAPACK
    int       i, j;
    char JOBVL[ 1 ] = { 'N' };
    char JOBVR[ 1 ] = { 'V' };
#ifndef LAPACK_USES_C_CONVENTIONS
    const int  jobvl_len = 1;
    const int  jobvr_len = 1;
#endif 
    lapack_integer N;
    lapack_integer INFO;
    lapack_integer LDA;
    lapack_integer LWORK;
    lapack_integer LDVL;
    lapack_integer LDVR;
    double*   A;
    double*   WI;
    double*   VL;
    double*   VR;
    double*   WORK;
    Matrix*   E_mp       = NULL;
    Vector*   D_vp       = NULL;
    Vector*   E_col_vp   = NULL;
    double    global_max = DBL_NOT_SET;


#ifdef DEF_OUT_BECAUSE_IT_IS_FIXED
#ifdef LINUX_X86_64
    if (! is_symmetric_matrix(mp))
    {
        static int first_time = TRUE;

        if (first_time)
        {
            warn_pso("\n----------------------------------------------------------------------------------\n");
            warn_pso("Diagonalization of non-symmetric matrices not reliable on 64 bit machines.\n");
            warn_pso("----------------------------------------------------------------------------------\n\n");
            first_time = FALSE;
        }
    }
#endif
#endif

    NRE(A = get_fortran_1D_dp_array_from_matrix(mp));

    N = mp->num_rows;
    LDVR = LDA = N;
    LDVL = 1;

    ERE(get_target_vector(&D_vp, N));

    NRE(WI = DBL_MALLOC(N));
    NRE(VR = DBL_MALLOC(N * N));
    NRE(VL = DBL_MALLOC(1));

    LWORK = 20 * N;
    NRE(WORK = DBL_MALLOC(LWORK));

    /* lapack will barf if this isn't true, and if that happens, we don't get a stacktrace, so catch it here: */
    ASSERT(LDA >= 1); 

    dgeev_(JOBVL, JOBVR, &N, A, &LDA, D_vp->elements, WI, VL, &LDVL, VR, &LDVR, WORK, &LWORK, &INFO
#ifndef LAPACK_USES_C_CONVENTIONS
           , jobvl_len, jobvr_len
#endif 
          );

    if (INFO == 0)
    {
        ERE(get_matrix_from_fortran_1D_dp_array(&E_mp, N, N, VR));

        ERE(get_target_matrix(E_mpp, N, N));
        ERE(get_target_vector(D_vpp, N));

        for (i=0; i<N; i++)
        {
            int index_of_max = 0;
            double max = D_vp->elements[ 0 ];;

            for (j=1; j<N; j++)
            {
                if (D_vp->elements[ j ] > max)
                {
                    index_of_max = j;
                    max = (D_vp->elements)[ j ];

                    if (i == 0)
                    {
                        global_max = max;
                    }
                }
            }

            ERE(get_matrix_col(&E_col_vp, E_mp, index_of_max));
            ERE(put_matrix_col(*E_mpp, E_col_vp, i));

            ((*D_vpp)->elements)[ i ] = (D_vp->elements)[ index_of_max ];
            (D_vp->elements)[ index_of_max ] = DBL_MOST_NEGATIVE;
        }

        for (i=0; i<N; i++)
        {
            if (fabs(WI[ i ] / global_max) > 0.0001)
            {
                warn_pso("Eigenvalue with imaginary componant %e found.\n",
                         WI[ i ]);
                warn_pso("    (Perhaps significant compared with the maxiumum eigenvalue magnitude %e.\n",
                         global_max);
            }
        }
    }
    else
    {
        free_matrix(*E_mpp);
        *E_mpp = NULL;
        free_vector(*D_vpp);
        *D_vpp = NULL;
    }


    free_vector(E_col_vp);
    free_vector(D_vp);
    free_matrix(E_mp);

    kjb_free(A);
    kjb_free(WI);
    kjb_free(VR);
    kjb_free(VL);
    kjb_free(WORK);

    if (INFO == 0)
    {
        return NO_ERROR;
    }
    else
    {
        set_error("Unable to diagonalize matrix.");
        add_error("Lapack INFO code is %d.", INFO);
        return ERROR;
    }
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              KJB_HAVE_LAPACK
    ------------------------------------------------------------------------- */
#else
    /* -------------------------------------------------------------------------
    |                              no LAPACK
    |                                  ||
    |                                 \||/
    |                                  \/
    */
    set_dont_have_lapack_error();
    return ERROR;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              no LAPACK
    ------------------------------------------------------------------------- */
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef HAVE_LAPACK_ARGS
int lapack_diagonalize_2
(
    const Matrix* mp,
    Matrix**      E_re_mpp,
    Matrix**      E_im_mpp,
    Vector**      D_re_vpp,
    Vector**      D_im_vpp
)
#else
/*ARGSUSED*/
int lapack_diagonalize_2
(
    const Matrix* __attribute__((unused)) dummy_mp,
    Matrix**      __attribute__((unused)) dummy_E_re_mpp,
    Matrix**      __attribute__((unused)) dummy_E_im_mpp,
    Vector**      __attribute__((unused)) dummy_D_re_vpp,
    Vector**      __attribute__((unused)) dummy_D_im_vpp
)
#endif 
{
#ifdef KJB_HAVE_LAPACK
    int   i;
    int   j;
    lapack_integer   N;
    lapack_integer   INFO;
    lapack_integer   LDA;
    lapack_integer   LWORK;
    lapack_integer   LDVL;
    lapack_integer   LDVR;
    double* A;
    double* VL;
    double* VR;
    double* WORK;
    Vector* allocated_D_re_vp     = NULL;
    Vector* allocated_D_im_vp     = NULL;
    const Vector* D_re_vp;
    const Vector* D_im_vp;
    const Matrix* E_re_mp = NULL;
    const Matrix* E_im_mp = NULL;
    Matrix* vr_mp = NULL;
    char JOBVL[ 1 ] = { 'N' };
    char JOBVR[ 1 ];
#ifndef LAPACK_USES_C_CONVENTIONS
    const int  jobvl_len = 1;
    const int  jobvr_len = 1;
#endif 

#ifdef LINUX_X86_64
    /* FIXME */
    if (! is_symmetric_matrix(mp))
    {
        static int first_time = TRUE;

        if (first_time)
        {
            warn_pso("\n----------------------------------------------------------------------------------\n");
            warn_pso("Diagonalization of non-symmetric matrices routine not reliable on 64 bit machines.\n");
            warn_pso("----------------------------------------------------------------------------------\n\n");
            first_time = FALSE;
        }
    }
#endif

    JOBVR[ 0 ] = ((E_re_mpp != NULL) || (E_im_mpp != NULL)) ? 'V' : 'N';

    NRE(A = get_fortran_1D_dp_array_from_matrix(mp));

    LDA = N = mp->num_rows;

    LDVR = ((E_re_mpp != NULL) || (E_im_mpp != NULL)) ? N : 1;
    LDVL = 1;

    if (E_re_mpp != NULL)
    {
        ERE(get_zero_matrix(E_re_mpp, N, N));
        E_re_mp = *E_re_mpp;
    }

    if (E_im_mpp != NULL)
    {
        ERE(get_zero_matrix(E_im_mpp, N, N));
        E_im_mp = *E_im_mpp;
    }

    if (D_re_vpp != NULL)
    {
        ERE(get_zero_vector(D_re_vpp, N));
        D_re_vp = *D_re_vpp;
    }
    else
    {
        ERE(get_zero_vector(&allocated_D_re_vp, N));
        D_re_vp = allocated_D_re_vp;

    }

    if (D_im_vpp != NULL)
    {
        ERE(get_zero_vector(D_im_vpp, N));
        D_im_vp = *D_im_vpp;
    }
    else
    {
        ERE(get_zero_vector(&allocated_D_im_vp, N));
        D_im_vp = allocated_D_im_vp;

    }

    NRE(VR = DBL_MALLOC(LDVR * LDVR));
    NRE(VL = DBL_MALLOC(LDVL * LDVL));

    LWORK = 20 * N;   /* HACK, FIX */

    NRE(WORK = DBL_MALLOC(LWORK));

    dgeev_(JOBVL, JOBVR, &N, A, &LDA, D_re_vp->elements, D_im_vp->elements, VL,
           &LDVL, VR, &LDVR, WORK, &LWORK, &INFO
#ifndef LAPACK_USES_C_CONVENTIONS
           , jobvl_len, jobvr_len
#endif 
           );

    if (INFO == 0)
    {
        if ((E_re_mp != NULL) || (E_im_mp != NULL))
        {
            ERE(get_matrix_from_fortran_1D_dp_array(&vr_mp, N, N, VR));

            j = 0;

            while (j < N)
            {
                if (D_im_vp->elements[ j ] > 0.0)
                {
                    /* Complex, so we get a conjugate pair. */

                    for (i = 0; i < N; i++)
                    {
                        double re = vr_mp->elements[ i ][ j ];
                        double im = vr_mp->elements[ i ][ j + 1 ];

                        if (E_re_mp != NULL)
                        {
                             E_re_mp->elements[ i ][ j ] = re;
                             E_re_mp->elements[ i ][ j + 1 ] = re;
                        }

                        if (E_im_mp != NULL)
                        {
                             E_im_mp->elements[ i ][ j ] = im;
                             E_im_mp->elements[ i ][ j + 1 ] = -im;
                        }
                    }

                    j += 2;
                }
                else if (E_re_mp != NULL)
                {
                    /*
                     * No need to do imaginary part, becuase if E_im_mp is
                     * needed, it was initialized to 0.
                    */
                    for (i = 0; i < N; i++)
                    {
                        E_re_mp->elements[ i ][ j ] = vr_mp->elements[ i ][ j ];
                    }
                    j++;
                }
                else
                {
                    j++;
                }

            }
        }
    }

    free_matrix(vr_mp);

    kjb_free(A);
    kjb_free(VR);
    kjb_free(VL);
    kjb_free(WORK);


    if (INFO == 0)
    {
        return NO_ERROR;
    }
    else
    {
        set_error("Unable to diagonalize matrix.");
        add_error("Lapack INFO code is %d.", INFO);
        return ERROR;
    }
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              KJB_HAVE_LAPACK
    ------------------------------------------------------------------------- */
#else
    /*--------------------------------------------------------------------------
    |                              no LAPACK
    |                                  ||
    |                                 \||/
    |                                  \/
    */
    set_dont_have_lapack_error();
    return ERROR;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              no LAPACK
    ------------------------------------------------------------------------- */
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             lapack_qr_decompose
 *
 * Uses lapack to do QR decomposition.
 *
 * This routine is the wrapper for lapack QR decomposition. Normally we don't call this
 * routine directly, but through qr_decompose(). 
 *
 * If this routine was not compiled with lapack available, then it returns
 * ERROR with an appropriate error being set.
 * 
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: matrix decomposition, lapack
 *
 * Author: Kyle Simek
 * -----------------------------------------------------------------------------
*/
#ifdef HAVE_LAPACK_ARGS
int lapack_qr_decompose
(
    const Matrix* mp,
    Matrix** Q_mpp,
    Matrix** R_mpp
)
#else
/*ARGSUSED*/
int lapack_qr_decompose
(
    const Matrix* __attribute__((unused)) dummy_mp,
    Matrix**      __attribute__((unused)) dummy_Q_mpp,
    Matrix**      __attribute__((unused)) dummy_R_mpp
)
#endif 
{
#ifdef KJB_HAVE_LAPACK

    int            i, j;
    lapack_integer M;
    lapack_integer N;
    lapack_integer INFO;
    lapack_integer LDA;
    lapack_integer LWORK;
    double*        A;
    double*        TAU;
    double*        WORK;
    int            NB;
    char           OPTS[ 1 ]   = { ' ' };
    char           DGEQRF[ 10 ] = "DGEQRF";
    lapack_integer ONE = 1;
    lapack_integer MINUS_ONE = -1;
    int            min_m_n;

    M = mp->num_rows;
    N = mp->num_cols;
    min_m_n = MIN(M,N);

    /*  VARIABLE SETUP */
    NB = ilaenv_(&ONE, DGEQRF, OPTS, &M, &N, &MINUS_ONE, &MINUS_ONE
#if !defined(MAC_OS_X_VERSION_MIN_REQUIRED) || MAC_OS_X_VERSION_MIN_REQUIRED < 1070
#ifndef LAPACK_USES_C_CONVENTIONS
            , strlen(DGEQRF), strlen(OPTS)
#endif 
#endif
            );


    NRE(A = get_fortran_1D_dp_array_from_matrix(mp));

    LDA = M;

    NRE(TAU = DBL_MALLOC(min_m_n));

    LWORK = (NB * N);
    NRE(WORK = DBL_MALLOC(LWORK));

    /*  CALL  */
    dgeqrf_(&M, &N, A, &LDA, TAU, WORK, &LWORK, &INFO);

    /*  POST_PROCESSNG / ERROR HANDLOING*/

    if(INFO == 0)
    {
        /*  UNPACK Q and R from A, TAU */
        Matrix* A_mp = NULL;
        Matrix* H    = NULL;
        Vector* V    = NULL;
        Matrix* temp_mp = NULL;


        ERE(get_matrix_from_fortran_1D_dp_array(&A_mp, M, N, A));
        

        if(Q_mpp != NULL)
        {
            ERE(get_identity_matrix(Q_mpp, M));
            for(i = 0; i < min_m_n; i++)
            {
                double tau   = TAU[i];

                ERE(get_target_vector(&V, M));

                for(j = 0; j <= i-1; j++)
                {
                    V->elements[j] = 0;
                }
                V->elements[i] = 1;
                for(j = i+1; j < M; j++)
                {
                    V->elements[j] = A_mp->elements[j][i];
                    A_mp->elements[j][i] = 0.0;
                }

                ERE(get_vector_outer_product(&temp_mp, V, V));
                ERE(ow_multiply_matrix_by_scalar(temp_mp, tau));

                ERE(get_identity_matrix(&H, M));
                ERE(subtract_matrices(&H, H, temp_mp));

                /* H = I - tau * v * v' */
                /*  where v(1:i-1) = 0 and v(i) = 1; 
                 *  v(i+1:m) is stored on exit in A(i+1:m,i) 
                 *  (using lapack's 1-indexing scheme). */

                ERE(multiply_matrices(Q_mpp, *Q_mpp, H));
            }

            free_matrix(H);
            H = NULL;
            free_vector(V);
            V = NULL;
            free_matrix(temp_mp);
            temp_mp = NULL;
        }
        else
        {
            /* Zero out bottom triangle of A (ignore Q values) */
            for(i = 0; i < min_m_n; i++)
            {
                for(j = 0; j < i; j++)
                {
                    A_mp->elements[i][j] = 0.0;
                }
            }

        }

        if(R_mpp != NULL)
        {
            copy_matrix_block(R_mpp, A_mp, 0, 0, min_m_n, N);


            if(M > N)
            {
                const Matrix* two_m[2];

                get_zero_matrix(&temp_mp, M - min_m_n, N);

                two_m[0] = *R_mpp;
                two_m[1] = temp_mp;
                ERE(concat_matrices_vertically(R_mpp, 2, two_m));

                free_matrix(temp_mp);
                temp_mp = NULL;
            }
        }

        free_matrix(A_mp);
        A_mp = NULL;
    }
    else
    {
        /* FREE OUTPUT VECTORS */
        free_matrix(*Q_mpp);
        *Q_mpp = NULL;
        free_matrix(*R_mpp);
        *R_mpp = NULL;
    }

    /*  FREE ALL TEMPS */
    kjb_free(A);
    kjb_free(TAU);
    kjb_free(WORK);



    if(INFO == 0)
    {
        return NO_ERROR;
    }
    else
    {
        set_error("Unable to QR decomposition matrix.");
        add_error("Lapack INFO code is %d.", INFO);
        return ERROR;
    }
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              KJB_HAVE_LAPACK
    ------------------------------------------------------------------------- */
#else
    /* -------------------------------------------------------------------------
    |                              no LAPACK
    |                                  ||
    |                                 \||/
    |                                  \/
    */
    set_dont_have_lapack_error();
    return ERROR;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              no LAPACK
    ------------------------------------------------------------------------- */
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef HAVE_LAPACK_ARGS
int lapack_diagonalize_symmetric(const Matrix* mp, Matrix** E_mpp, Vector** D_vpp)
#else
/*ARGSUSED*/
int lapack_diagonalize_symmetric
(
    const Matrix* __attribute__((unused)) dummy_mp,
    Matrix**      __attribute__((unused)) dummy_E_mpp,
    Vector**      __attribute__((unused)) dummy_D_vpp
)
#endif 
{
#ifdef KJB_HAVE_LAPACK
    int            i, j;
    lapack_integer N;
    lapack_integer INFO;
    lapack_integer LDA;
    lapack_integer LWORK;
    double*        A;
    double*        WORK;
    Matrix*        E_mp        = NULL;
    Vector*        D_vp        = NULL;
    Vector*        E_col_vp    = NULL;
    int            NB;
    char           UPLO[ 1 ]   = { 'U' };
    char           JOBZ[ 1 ];
#ifndef LAPACK_USES_C_CONVENTIONS
    const int      uplo_len = 1;
    const int      jobz_len = 1;
#endif 
    char           DSYTRD[ 10 ];
    lapack_integer ONE         = 1;
    lapack_integer MINUS_ONE   = -1;


    JOBZ[ 0 ] = (E_mpp == NULL) ? 'N' : 'V';

    BUFF_CPY(DSYTRD, "DSYTRD");

    N = mp->num_rows;

#ifdef TEST
    {
        double temp_max = max_abs_matrix_element(mp);
        double max_diff = DBL_MOST_NEGATIVE;
        double diff;
        double thresh = 1000.0 * DBL_EPSILON;

        for (i = 0; i < N; i++)
        {
            for (j = 0; j < N; j++)
            {
                diff = ABS_OF(mp->elements[ i ][ j ] - mp->elements[ j ][ i ]);

                if (diff > max_diff)
                {
                    max_diff = diff;
                }
            }
        }


        if (max_diff > thresh * temp_max)
        {
            warn_pso("Matrix passed to routine diagonalize_symmetric() is not symmetric.\n");
            warn_pso("The maximum absolute matrix element is %e.\n", temp_max);
            warn_pso("The maximum absolute difference of opposite elements is %e.\n",
                     max_diff);
            warn_pso("The ratio of these exceeds the threshold of %e.\n",
                     thresh);
        }
    }
#endif

    NB = ilaenv_(&ONE, DSYTRD, UPLO, &N, &MINUS_ONE, &MINUS_ONE, &MINUS_ONE
#if !defined(MAC_OS_X_VERSION_MIN_REQUIRED) || MAC_OS_X_VERSION_MIN_REQUIRED < 1070
#ifndef LAPACK_USES_C_CONVENTIONS
            , strlen(DSYTRD), strlen(UPLO)
#endif 
#endif
            );

    NRE(A = get_fortran_1D_dp_array_from_matrix(mp));

    LDA = N;

    ERE(get_target_vector(&D_vp, N));

    LWORK = (NB + 2) * N;
    NRE(WORK = DBL_MALLOC(LWORK));

    dsyev_(JOBZ, UPLO, &N, A, &LDA, D_vp->elements, WORK, &LWORK, &INFO
#ifndef LAPACK_USES_C_CONVENTIONS
           , jobz_len, uplo_len 
#endif 
           );

    if (INFO == 0)
    {
        if (E_mpp != NULL)
        {
            ERE(get_matrix_from_fortran_1D_dp_array(&E_mp, N, N, A));
            ERE(get_target_matrix(E_mpp, N, N));
        }

        ERE(get_target_vector(D_vpp, N));

        for (i=0; i<N; i++)
        {
            int    index_of_max = 0;
            double max = D_vp->elements[ 0 ];;

            for (j=1; j<N; j++)
            {
                if (D_vp->elements[ j ] > max)
                {
                    index_of_max = j;
                    max = (D_vp->elements)[ j ];
                }
            }

            if (E_mpp != NULL)
            {
                ERE(get_matrix_col(&E_col_vp, E_mp, index_of_max));
                ERE(put_matrix_col(*E_mpp, E_col_vp, i));
            }

            ((*D_vpp)->elements)[ i ] = (D_vp->elements)[ index_of_max ];
            (D_vp->elements)[ index_of_max ] = DBL_MOST_NEGATIVE;
        }
    }
    else
    {
        free_matrix(*E_mpp);
        *E_mpp = NULL;
        free_vector(*D_vpp);
        *D_vpp = NULL;
    }


    free_vector(E_col_vp);
    free_vector(D_vp);
    free_matrix(E_mp);

    kjb_free(A);
    kjb_free(WORK);

    if (INFO == 0)
    {
        return NO_ERROR;
    }
    else
    {
        set_error("Unable to diagonalize matrix.");
        add_error("Lapack INFO code is %d.", INFO);
        return ERROR;
    }
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              KJB_HAVE_LAPACK
    ------------------------------------------------------------------------- */
#else
    /* -------------------------------------------------------------------------
    |                              no LAPACK
    |                                  ||
    |                                 \||/
    |                                  \/
    */
    set_dont_have_lapack_error();
    return ERROR;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              no LAPACK
    ------------------------------------------------------------------------- */
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             do_lapack_svd
 *
 * Uses lapack to do SVD
 *
 * This routine is the wrapper for lapack SVD. Normally we don't call this
 * routine directly, but through do_svd(). 
 *
 * If this routine was not compiled with lapack available, then it returns
 * ERROR with an appropriate error being set.
 * 
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: SVD, lapack
 *
 * -----------------------------------------------------------------------------
*/

#ifdef HAVE_LAPACK_ARGS
int do_lapack_svd
(
    const Matrix* a_mp,
    Matrix**      u_mpp,
    Vector**      d_vpp,
    Matrix**      vt_mpp
)
#else
int do_lapack_svd
(
    const Matrix* __attribute__((unused)) dummy_a_mp,
    Matrix**      __attribute__((unused)) dummy_u_mpp,
    Vector**      __attribute__((unused)) dummy_d_vpp,
    Matrix**      __attribute__((unused)) dummy_vt_mpp
)
#endif 
{
#ifdef KJB_HAVE_LAPACK
    /* -------------------------------------------------------------------------
    |                              KJB_HAVE_LAPACK
    |                                  ||
    |                                 \||/
    |                                  \/
    */
    lapack_integer M, N;
    lapack_integer INFO;
    lapack_integer LDA;
    lapack_integer LWORK;
    lapack_integer LDU;
    lapack_integer LDVT;
    double*        A;
    double*        S;
    double*        U;
    double*        VT;
    double*        WORK;
    int            result     = NO_ERROR;
    char           JOBU[ 1 ]  = { 'S' };
    char           JOBVT[ 1 ] = { 'A' };
#ifndef LAPACK_USES_C_CONVENTIONS
    const int  jobu_len = 1;
    const int  jobvt_len = 1;
#endif 
    Vector*        allocated_d_vp = NULL;



    M = a_mp->num_rows;
    N = a_mp->num_cols;
    LDA = M;
    LDVT = N;
    LDU = M;
    LWORK = MAX_OF(3*MIN_OF(M,N) + MAX_OF(M,N), 5 * MIN_OF(M,N) - 4);
    /* Docs say LWORK should be bigger for good performance, but they don't say
    // how much bigger! Lets make it 3 times bigger.
    */
    LWORK *= 3;


    /*
     * Get rid of extra allocation (May 22, 2004).
     *
    S = DBL_MALLOC(MIN_OF(M, N));
    */
    if (d_vpp == NULL)
    {
        ERE(get_target_vector(&allocated_d_vp, MIN_OF(M,N)));
        S = allocated_d_vp->elements;
    }
    else
    {
        ERE(get_target_vector(d_vpp, MIN_OF(M,N)));
        S = (*d_vpp)->elements;
    }

    A = get_fortran_1D_dp_array_from_matrix(a_mp);
    U = DBL_MALLOC(M * MIN_OF(M,N));
    VT = DBL_MALLOC(N * N);
    WORK = DBL_MALLOC(LWORK);

    if ((A == NULL) || (U == NULL) || (VT == NULL) || (WORK == NULL))
    {
        result = ERROR;
    }

    if (result != ERROR)
    {
        dgesvd_(JOBU, JOBVT, &M, &N, A, &LDA, S, U, &LDU, VT,
                &LDVT, WORK, &LWORK, &INFO
#ifndef LAPACK_USES_C_CONVENTIONS
                , jobu_len, jobvt_len 
#endif 
                );

        if (INFO < 0)
        {
            set_bug("Problem with parameter %ld of LAPACK routine dgesvd.",
                    -(long)INFO);
            result = ERROR;
        }
        else if (INFO > 0)
        {
            set_error("SVD computation did not converge.");
            result = ERROR;
        }
    }

    if ((result != ERROR) && (u_mpp != NULL))
    {
        result = get_matrix_from_fortran_1D_dp_array(u_mpp, M, MIN_OF(M, N), U);
    }

    if ((result != ERROR) && (vt_mpp != NULL))
    {
        result = get_matrix_from_fortran_1D_dp_array(vt_mpp, N, N, VT);
    }

    kjb_free(A);
    kjb_free(VT);
    kjb_free(U);
    kjb_free(WORK);

    free_vector(allocated_d_vp);

    if (result == ERROR)
    {
        add_error("Problem occured using LAPACK routine DGESVD for SVD.");
    }

    return result;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              KJB_HAVE_LAPACK
    ------------------------------------------------------------------------- */
#else
    /* -------------------------------------------------------------------------
    |                              no LAPACK
    |                                  ||
    |                                 \||/
    |                                  \/
    */
    /*ARGSUSED*/
    set_dont_have_lapack_error();
    return ERROR;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              no LAPACK
    ------------------------------------------------------------------------- */
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             do_lapack_matrix_inversion
 *
 * Uses lapack to invert matrices.
 *
 * This routine is the wrapper for lapack matrix inversion. Normally we don't
 * call this routine directly, but through get_matrix_inverse(). 
 *
 * If this routine was not compiled with lapack available, then it returns
 * ERROR with an appropriate error being set.
 * 
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: matrix inversion, lapack
 *
 * -----------------------------------------------------------------------------
*/
int do_lapack_matrix_inversion
(
    Matrix**      target_mpp,
    const Matrix* input_mp
)
{
    return do_lapack_matrix_inversion_2(target_mpp, input_mp, NULL, NULL);
}

/* =============================================================================
 *                             do_lapack_matrix_inversion_2
 *
 * Uses lapack to invert matrices and compute determinant.  
 * Because inversion requires LU decomposition, computing the determinant
 * at the same time is very cheap.  
 *
 * To avoid numeric overflow, the log of the absolute value of the determinant 
 * is returned in log_det. The sign of the determinant (1, -1) is stored in 
 * det_sign. Setting either to NULL will skip the determinant computation.
 *
 *
 * If this routine was not compiled with lapack available, then it returns
 * ERROR with an appropriate error being set.
 * 
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: matrix inversion, lapack
 *
 * -----------------------------------------------------------------------------
*/
#ifdef HAVE_LAPACK_ARGS
int do_lapack_matrix_inversion_2
(
    Matrix**      target_mpp,
    const Matrix* input_mp,
    double* log_det,
    int* det_sign
)
#else
/*ARGSUSED*/
int do_lapack_matrix_inversion_2
(
    Matrix**      __attribute__((unused)) dummy_target_mpp,
    const Matrix* __attribute__((unused)) dummy_input_mp,
    double* __attribute__((unused)) dummy_det,
    int* __attribute__((unused)) dummy_det_sign
)
#endif 
{
#ifdef KJB_HAVE_LAPACK
    /* -------------------------------------------------------------------------
    |                              KJB_HAVE_LAPACK
    |                                  ||
    |                                 \||/
    |                                  \/
    */
    char            OPTS[ 1 ]   = { '\0' };
    char            DGETRI[ 7 ] = { 'D', 'G', 'E', 'T', 'R', 'I', '\0' };
    lapack_integer  ONE         = 1;
    lapack_integer  MINUS_ONE   = -1;
    lapack_integer  N;
    lapack_integer  INFO;
    lapack_integer  LDA;
    lapack_integer  LWORK;
    lapack_integer* IPIV_ptr    = NULL;
    double*         WORK        = NULL;
    int             result      = NO_ERROR;
    int             num_rows;
    int             num_cols;
    double*         A_ptr;
    int             NB;
    int             i;


    if (input_mp == NULL)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return NO_ERROR;
    }

    num_rows = input_mp->num_rows;
    num_cols = input_mp->num_cols;

    if ((num_rows < 1) || (num_rows != num_cols))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    verify_matrix(input_mp, NULL); 

    LDA = N = num_rows;

    NRE(IPIV_ptr = N_TYPE_MALLOC(lapack_integer, N));

    result = copy_matrix(target_mpp, input_mp);

    if (result == ERROR) { NOTE_ERROR(); goto cleanup; }

    /*
     * We just let Fortran invert the transpose, because inv(A')=inv(A)'.
    */

    A_ptr = (*target_mpp)->elements[ 0 ];

    dgetrf_(&N, &N, A_ptr, &LDA, IPIV_ptr, &INFO);

    /* compute log determinant and sign */
    if(log_det != NULL && det_sign != NULL)
    {
        double** elements = (*target_mpp)->elements;
        *log_det = 0;
        *det_sign = 1;
        for(i = 0; i < num_rows; ++i)
        {
            /* note: fortran indices start at 1 */
            if(IPIV_ptr[i] != (i+1)) 
                *det_sign = -*det_sign;

            if(elements[i][i] < 0)
            {
                *det_sign = -*det_sign;
                *log_det += log(-elements[i][i]);
            }
            else
                *log_det += log(elements[i][i]);
        }
    }


    if (INFO < 0)
    {
        SET_CANT_HAPPEN_BUG();
        NOTE_ERROR();
        result = ERROR;
        return ERROR;
    }
    else if (INFO > 0)
    {
        set_error("Matrix to be inverted is singular.");
        NOTE_ERROR();
        result = ERROR;
        goto cleanup;
    }

    NB = ilaenv_(&ONE, DGETRI, OPTS, &N, &MINUS_ONE, &MINUS_ONE, &MINUS_ONE
#if !defined(MAC_OS_X_VERSION_MIN_REQUIRED) || MAC_OS_X_VERSION_MIN_REQUIRED < 1070
#ifndef LAPACK_USES_C_CONVENTIONS
            , strlen(DGETRI), strlen(OPTS)
#endif 
#endif
            );

    LWORK = NB * N;  /* Should be at least N*NB */
    WORK = DBL_MALLOC(LWORK);

    if (WORK == NULL)
    {
        NOTE_ERROR();
        result = ERROR;
        goto cleanup;
    }

    dgetri_(&N, A_ptr, &LDA, IPIV_ptr, WORK, &LWORK, &INFO);

cleanup:

    kjb_free(IPIV_ptr);
    kjb_free(WORK);

    if (result == ERROR)
    {
        add_error("Problem occured using LAPACK routines DGETRI for matrix inversion.");
    }

    return result;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              KJB_HAVE_LAPACK
    ------------------------------------------------------------------------- */
#else
    /* -------------------------------------------------------------------------
    |                              no LAPACK
    |                                  ||
    |                                 \||/
    |                                  \/
    */
    set_dont_have_lapack_error();
    return ERROR;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              no LAPACK
    ------------------------------------------------------------------------- */
#endif
}

/* =============================================================================
 *                            do_lapack_cholesky_decomposition 
 *
 * Uses lapack to perform Cholesky decomposition.  Operation is performed 
 * in-place, and no temporary storage is allocated in this function.
 *
 * Passing NULL in input_mp does nothing, and returns NO_ERROR.
 *
 * If this routine was not compiled with lapack available, then it returns
 * ERROR with an appropriate error being set.
 * 
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: matrix decomposition, lapack
 *
 * -----------------------------------------------------------------------------
*/
#ifdef HAVE_LAPACK_ARGS
int do_lapack_cholesky_decomposition
(
    Matrix* input_mp
)
#else
/*ARGSUSED*/
int do_lapack_cholesky_decomposition
(
    Matrix* __attribute__((unused)) dummy_input_mp
)
#endif 
{
#ifdef KJB_HAVE_LAPACK
    /* -------------------------------------------------------------------------
    |                              KJB_HAVE_LAPACK
    |                                  ||
    |                                 \||/
    |                                  \/
    */
    char            UPLO[1] = {'U'}; 
    lapack_integer  N;
    lapack_integer  LDA;
    lapack_integer  INFO;
    int             result      = NO_ERROR;
    int             num_rows;
    int             num_cols;
    double*         A_ptr;
    int i,j;


    if (input_mp == NULL)
    {
        return NO_ERROR;
    }

    num_rows = input_mp->num_rows;
    num_cols = input_mp->num_cols;

    if ((num_rows < 1) || (num_rows != num_cols))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    verify_matrix(input_mp, NULL); 

    LDA = N = num_rows;

    A_ptr = input_mp->elements[ 0 ];

#ifdef LAPACK_IS_ACML
    dpotrf_(UPLO, &N, A_ptr, &LDA, &INFO, (int)sizeof(UPLO));
#else 
    dpotrf_(UPLO, &N, A_ptr, &LDA, &INFO);
#endif 

    if (INFO < 0)
    {
        SET_CANT_HAPPEN_BUG();
        NOTE_ERROR();
        result = ERROR;
        return ERROR;
    }
    else if (INFO > 0)
    {
        set_error("Is not positive definite.");
        NOTE_ERROR();
        result = ERROR;
        goto cleanup;
    }

    for(i = 0; i < N; ++i)
    {
        for(j = i+1; j < N; ++j)
        {
            input_mp->elements[i][j] = 0.0;
        }

    }

cleanup:
    if (result == ERROR)
    {
        add_error("Problem occured using LAPACK routine DPOTRF for matrix inversion.");
    }

    return result;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              KJB_HAVE_LAPACK
    ------------------------------------------------------------------------- */
#else
    /* -------------------------------------------------------------------------
    |                              no LAPACK
    |                                  ||
    |                                 \||/
    |                                  \/
    */
    set_dont_have_lapack_error();
    return ERROR;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              no LAPACK
    ------------------------------------------------------------------------- */
#endif
}

/* =============================================================================
 *                            do_lapack_cholesky_decomposition_2
 *
 * Uses lapack to perform Cholesky decomposition.  If input and output
 * matrices point to the same address, operation is performed 
 * in-place, and no temporary storage is allocated in this function.  Otherwise
 * the source matrix is copied to the target matrix (reallocating it if 
 * necessary), then then the single-parameter version is called on the target
 * matrix.
 *
 * Passing NULL in input_mp does nothing, and returns NO_ERROR.
 *
 * If this routine was not compiled with lapack available, then it returns
 * ERROR with an appropriate error being set.
 * 
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: matrix decomposition, lapack
 *
 * -----------------------------------------------------------------------------
*/
int do_lapack_cholesky_decomposition_2(Matrix** target_mpp, const Matrix* input_mp)
{
    /* no-op if *L == A */
    ERE(copy_matrix(target_mpp, input_mp));
    return do_lapack_cholesky_decomposition(*target_mpp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef CURRENTLY_DOES_NOT_ALWAYS_WORK

/* =============================================================================
 *                             do_lapack_dot_product
 *
 * Uses lapack to compute dot products
 *
 * This routine is the wrapper for a lapack routine to compute dot products. It
 * is currently experimental. Generally, get_dot_product() does the job just
 *
 * If this routine was not compiled with lapack available, then it returns
 * ERROR with an appropriate error being set.
 * 
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: dot product, lapack
 *
 * -----------------------------------------------------------------------------
*/

#ifdef HAVE_LAPACK_ARGS
int do_lapack_dot_product
(
    const Vector* vp1,
    const Vector* vp2,
    double *dp_ptr
)
#else
/*ARGSUSED*/
int do_lapack_dot_product
(
    const Vector* __attribute__((unused)) vp1,
    const Vector* __attribute__((unused)) vp2,
    double* __attribute__((unused)) dp_ptr;
)
#endif 
{
#ifdef KJB_HAVE_LAPACK
    /* -----------------------------------------------------------------------------
    |                              KJB_HAVE_LAPACK
    |                                  ||
    |                                 \||/
    |                                  \/
    */
    double* pos1;
    double* pos2;
    int len;
    int one = 1;


    ERE(check_same_vector_lengths(vp1, vp2, "get_dot_product"));

    len = vp1->length;
    pos1 = vp1->elements;
    pos2 = vp2->elements;


    *dp_ptr = ddot_(&len, pos1, &one, pos2, &one);

    return NO_ERROR;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              KJB_HAVE_LAPACK
    ------------------------------------------------------------------------ */
#else
    /* -------------------------------------------------------------------------
    |                              no LAPACK
    |                                  ||
    |                                 \||/
    |                                  \/
    */
    set_dont_have_lapack_error();
    return ERROR;
    /*
    |                                  /\
    |                                 /||\
    |                                  ||
    |                              no LAPACK
    ------------------------------------------------------------------------- */
#endif
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

