
/* $Id: wrap_slatec.c 4727 2009-11-16 20:53:54Z kobus $ */

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



/*
// This file implements wrappers to the SLATEC library. Code should go into
// this file if and only if it is involved with SLATEC. Thus this file forces
// loading of SLATEC. Other modules may implicitly force the loading of this
// file (and thus SLATEC), but then they should not contain modules which
// may be used independently of SLATEC.
//
// The code that follows is a bit ugly with #ifdef's because we want code that
// calls these routines to compile and link even if the SLATEC library is not
// available. Since we cannot know (at the library level) if the routine(s)
// used from this module are essential for the higher level code, it is better
// if the higher level code can pretend that the routine is available. Also, we
// do not want to force the higher level code to do the #ifdef'ing.
*/

#include "m/m_gen.h"        /*  Only safe if first #include in a ".c" file  */
#include "wrap_slatec/wrap_slatec.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*
// Un-comment to force NO SLATEC
//
#undef KJB_HAVE_SLATEC
*/

#ifdef KJB_HAVE_SLATEC
/* -----------------------------------------------------------------------------
|                            KJB_HAVE_SLATEC
|                                  ||
|                                 \||/
|                                  \/
*/
#include "slatec.h"

/*
#define NO_DBOCLS_EQUALITY_SLOP
*/
#define DBOCLS_EQUALITY_SLOP    (10000.0 * DBL_EPSILON)

#define MAX_CONSTRAINT_TOLERANCE  0.0001

/*
|                                  /\
|                                 /||\
|                                  ||
|                             KJB_HAVE_SLATEC
----------------------------------------------------------------------------- */
#else
/* -----------------------------------------------------------------------------
|                              no SLATEC
|                                  ||
|                                 \||/
|                                  \/
*/
    static void set_dont_have_slatec_error(void);
/*
|                                  /\
|                                 /||\
|                                  ||
|                              no SLATEC
----------------------------------------------------------------------------- */
#endif


/* -------------------------------------------------------------------------- */


#ifdef KJB_HAVE_SLATEC
/* -----------------------------------------------------------------------------
|                             KJB_HAVE_SLATEC
|                                  ||
|                                 \||/
|                                  \/
*/

int do_dlsei_quadratic
(
    Vector**      result_vpp,
    const Matrix* mp,
    const Vector* target_vp,
    const Matrix* le_constraint_mp,
    const Vector* le_constraint_col_vp,
    const Matrix* eq_constraint_mp,
    const Vector* eq_constraint_col_vp,
    const Vector* lb_vp,
    const Vector* ub_vp
)
{
    const Vector* result_vp = NULL;
    Matrix* bound_constraint_mp   = NULL;
    int     le_m;
    int     i;
    int     j;
    int     count;
    int     num_bound_constraints;
    int     result = NO_ERROR;

    /* Variable names as given in SLATEC docs. */
    int   N;
    double* WS;
    int*  IP;
    int   dim_IP;
    int   dim_WS;
    int   MDW;
    int   ME;
    int   MA;
    int   MG;
    int   MODE;
    double* W;
    double* X;
    double  RNORML;
    double  RNORME;
    double  PRGOPT[ 1 ];


    PRGOPT[ 0 ] = 1.0;

    N = NOT_SET;

    if (mp != NULL)
    {
        N = mp->num_cols;
        MA = mp->num_rows;
    }
    else
    {
        MA = 0;
    }

    if (le_constraint_mp != NULL)
    {
        if (N == NOT_SET)
        {
            N = le_constraint_mp->num_cols;
        }
        else
        {
            if (N != le_constraint_mp->num_cols)
            {
                SET_ARGUMENT_BUG();
                return ERROR;
            }
        }

        le_m = le_constraint_mp->num_rows;

        if (le_constraint_col_vp == NULL)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }

        if (le_m != le_constraint_col_vp->length)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }
    else
    {
        le_m = 0;
    }

    if (eq_constraint_mp != NULL)
    {
        if (N == NOT_SET)
        {
            N = eq_constraint_mp->num_cols;
        }
        else
        {
            if (N != eq_constraint_mp->num_cols)
            {
                SET_ARGUMENT_BUG();
                return ERROR;
            }
        }

        ME = eq_constraint_mp->num_rows;

        if (eq_constraint_col_vp == NULL)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }

        if (ME != eq_constraint_col_vp->length)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }
    else
    {
        ME = 0;
    }

    if (N == NOT_SET)
    {
        set_bug("All matrices passed to do_dlsie_quadratic are NULL.");
        return ERROR;
    }

    ERE(get_target_vector(result_vpp, N));
    result_vp = *result_vpp;


    num_bound_constraints = 0;

    if (lb_vp != NULL)
    {
        for (i=0; i<N; i++)
        {
            if ((lb_vp->elements)[i] > DBL_HALF_MOST_NEGATIVE / 3.0)
            {
                num_bound_constraints++;
            }
        }
    }

    if (ub_vp != NULL)
    {
        for (i=0; i<N; i++)
        {
            if ((ub_vp->elements)[i] < DBL_HALF_MOST_POSITIVE / 3.0)
            {
                num_bound_constraints++;
            }
        }
    }

    if (num_bound_constraints > 0)
    {
        int bound_inequality_count = 0;

        NRE(bound_constraint_mp = create_zero_matrix(num_bound_constraints,
                                                     N + 1));

        if (lb_vp != NULL)
        {
            for (i=0; i<N; i++)
            {
                if ((lb_vp->elements)[i]> DBL_HALF_MOST_NEGATIVE/3.0)
                {
                    (bound_constraint_mp->elements)[bound_inequality_count][i] =
                                                                       1.0;
                    (bound_constraint_mp->elements)[bound_inequality_count][N] =
                                                      (lb_vp->elements)[i];
                    bound_inequality_count++;
                }
            }
        }

        if (ub_vp != NULL)
        {
            for (i=0; i<N; i++)
            {
                if ((ub_vp->elements)[i]< DBL_HALF_MOST_POSITIVE/3.0)
                {
                    (bound_constraint_mp->elements)[bound_inequality_count][i] =
                                                                      -1.0;
                    (bound_constraint_mp->elements)[bound_inequality_count][N] =
                                                      -(ub_vp->elements)[i];
                    bound_inequality_count++;
                }
            }
        }
    }

    MG = le_m + num_bound_constraints;
    MDW = MA + ME + MG;

    NRE(X = DBL_MALLOC(N));
    dim_WS = 2*(ME+N)+(MAX_OF(MA+MG,N))+(MG+2)*(N+7);
    NRE(WS = DBL_MALLOC(dim_WS));
    dim_IP = MG+2*N+2;
    NRE(IP = INT_MALLOC(dim_IP));
    IP[ 0 ] = dim_WS;
    IP[ 1 ] = dim_IP;

    NRE(W = DBL_MALLOC( MDW * (N + 1)));

    for (i=0; i<MDW * (N+1); i++)
    {
        W[i] = DBL_NOT_SET;
    }

    /* F77 style arrays */

    count = 0;

    for (i=0; i<ME; i++)
    {
        for (j=0; j<N; j++)
        {
            W[ j*MDW + count ] = (eq_constraint_mp->elements)[i][j];
        }

        W[ N*MDW + count ] = (eq_constraint_col_vp->elements)[i];
        count++;
    }

    for (i=0; i<MA; i++)
    {
        for (j=0; j<N; j++)
        {
            W[ j*MDW + count ] = (mp->elements)[i][j];
        }
        W[ N*MDW + count ] = (target_vp->elements)[i];
        count++;
    }

    for (i=0; i<le_m; i++)
    {
        for (j=0; j<N; j++)
        {
            W[ j*MDW + count ] = -(le_constraint_mp->elements)[i][j];
        }

        W[ N*MDW + count ] = -(le_constraint_col_vp->elements)[i];
        count++;
    }

    for (i=0; i<num_bound_constraints; i++)
    {
        for (j=0; j<N+1; j++)
        {
            W[ j*MDW + count] = (bound_constraint_mp->elements)[i][j];
        }
        count++;
    }

    dlsei_(W,&MDW, &ME, &MA, &MG, &N, PRGOPT, X, &RNORME, &RNORML, &MODE, WS,
           IP);

    if (MODE == 0)
    {
        Vector* est_vp  = NULL;
        Vector* diff_vp = NULL;
        double  rnorm;


        for (i=0; i<result_vp->length; i++)
        {
            (result_vp->elements)[i] = X[i];
        }

        if (multiply_matrix_and_vector(&est_vp, mp, result_vp) == ERROR)
        {
            result = ERROR;
        }
        else if (subtract_vectors(&diff_vp, est_vp, target_vp) == ERROR)
        {
            result = ERROR;
        }
        else
        {
            rnorm = vector_magnitude(diff_vp);

            if (    (rnorm > 1000 * DBL_EPSILON)
                 && (fabs(rnorm - RNORML) / rnorm > 0.01)
               )
            {
                set_high_light(stdout);
                pso("\n");
                rep_print(stdout, '-', 80);
                pso("Warning:\n\n");
                pso("Externally computed residual norm differs ");
                pso("from dlsei() RNORML by more than 1%%.\n");
                pso("Computed residual norm is %e.\n", rnorm);
                pso("dlsei() RNORML is %e.\n", RNORML);
                rep_print(stdout, '-', 80);
                pso("\n\n");
                unset_high_light(stdout);
            }

            if (check_quadratic_constraints(result_vp, le_constraint_mp,
                                            le_constraint_col_vp,
                                            lb_vp, ub_vp) == ERROR)
            {
                add_error("Currently this is only treated as a WARNING"); 
                kjb_print_error(); 
            }
        }

        free_vector(est_vp);
        free_vector(diff_vp);
    }
    else if (MODE == 1)
    {
        set_error("Equality constraints are contradictory.");
        result = ERROR;
    }
    else if (MODE == 2)
    {
        set_error("Inequality constraints are contradictory.");
        result = ERROR;
    }
    else if (MODE == 3)
    {
        set_error(
             "Both equality and inequality constraints are contradictory.");
        result = ERROR;
    }
    else
    {
        set_bug("Unexpected MODE returned from dlsei.");
        result = ERROR;
    }

    free_matrix(bound_constraint_mp);

    kjb_free(W);
    kjb_free(X);
    kjb_free(WS);
    kjb_free(IP);

    return result;
}
/*
|                                  /\
|                                 /||\
|                                  ||
|                             KJB_HAVE_SLATEC
----------------------------------------------------------------------------- */
#else
/* -----------------------------------------------------------------------------
|                              no SLATEC
|                                  ||
|                                 \||/
|                                  \/
*/
/*ARGSUSED*/
int do_dlsei_quadratic
(
    Vector**      __attribute__((unused)) dummy_result_vpp,
    const Matrix* __attribute__((unused)) dummy_mp,
    const Vector* __attribute__((unused)) dummy_target_vp,
    const Matrix* __attribute__((unused)) dummy_le_constraint_mp,
    const Vector* __attribute__((unused)) dummy_le_constraint_col_vp,
    const Matrix* __attribute__((unused)) dummy_eq_constraint_mp,
    const Vector* __attribute__((unused)) dummy_eq_constraint_col_vp,
    const Vector* __attribute__((unused)) dummy_lb_vp,
    const Vector* __attribute__((unused)) dummy_ub_vp
)
{


    set_dont_have_slatec_error();
    return ERROR;
}
/*
|                                  /\
|                                 /||\
|                                  ||
|                              no SLATEC
----------------------------------------------------------------------------- */
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef KJB_HAVE_SLATEC
/* -----------------------------------------------------------------------------
|                             KJB_HAVE_SLATEC
|                                  ||
|                                 \||/
|                                  \/
*/

int do_dbocls_quadratic
(
    Vector**      result_vpp,
    const Matrix* mp,
    const Vector* target_vp,
    const Matrix* le_constraint_mp,
    const Vector* le_constraint_col_vp,
    const Matrix* eq_constraint_mp,
    const Vector* eq_constraint_col_vp,
    const Vector* lb_vp,
    const Vector* ub_vp
)
{
    const Vector* result_vp = NULL;
    Matrix* constraint_mp   = NULL;
    Vector* input_lb_vp;
    Vector* input_ub_vp;
    int     eq_n;
    int     eq_m;
    int     le_n;
    int     le_m;
    int     n, m, i, j;
    int     result          = NO_ERROR;
    /* Variable names as given in SLATEC docs. */
    int   I;
    double* RW;
    int*  IW;
    int   MDW;
    int   MCON;
    int   MROWS;
    int   NCOLS;
    int   MODE;
    double* W;
    double* X;
    double  RNORMC;
    double  RNORM;
    double* BL;
    double* BU;
    int   IOPT[ 17 + 9   ];   /* 17 is base, 9 is extra for OPTION 2. */
    int*  IND;


    if (mp->num_cols <= 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (le_constraint_mp == NULL)
    {
        le_n = le_m = 0;
    }
    else
    {
        le_m = le_constraint_mp->num_rows;
        le_n = le_constraint_mp->num_cols;

        if ((le_constraint_col_vp == NULL) ||
            (le_constraint_col_vp->length != le_m))
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    if (eq_constraint_mp == NULL)
    {
        eq_n = eq_m = 0;
    }
    else
    {
        eq_m = eq_constraint_mp->num_rows;
        eq_n = eq_constraint_mp->num_cols;

        if ((eq_constraint_col_vp == NULL) ||
            (eq_constraint_col_vp->length != eq_m))
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    if ((le_n != 0) && (eq_n != 0))
    {
        if (le_n != eq_n)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
        n = le_n;
    }
    else if (le_n != 0)
    {
        n = le_n;
    }
    else if (eq_n != 0)
    {
        n = eq_n;
    }
    else
    {
        n = mp->num_cols;
    }

    if (n != mp->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_vector(result_vpp, n));
    result_vp = *result_vpp;

    m = le_m + eq_m;

    if (m > 0)
    {
        NRE(constraint_mp = create_zero_matrix(m, n));
    }

    I = 0;

    for (i=0; i<le_m; i++)
    {
        for (j=0; j<n; j++)
        {
            (constraint_mp->elements)[I][j]= (le_constraint_mp->elements)[i][j];
        }
        I++;
    }

    for (i=0; i<eq_m; i++)
    {
        for (j=0; j<n; j++)
        {
            (constraint_mp->elements)[I][j]= (eq_constraint_mp->elements)[i][j];
        }
        I++;
    }

    NRE(input_lb_vp = create_vector(n + eq_m + le_m));
    NRE(input_ub_vp = create_vector(n + eq_m + le_m));

    for (j=0; j<n; j++)
    {
        if (lb_vp != NULL)
        {
            (input_lb_vp->elements)[j] = (lb_vp->elements)[j];
        }
        else
        {
            (input_lb_vp->elements)[j] = DBL_MOST_NEGATIVE;
        }

        if (ub_vp != NULL)
        {
            (input_ub_vp->elements)[j] = (ub_vp->elements)[j];
        }
        else
        {
            (input_ub_vp->elements)[j] = DBL_MOST_POSITIVE;
        }
    }

    /* Constraints */
    for (j=0; j<le_m; j++)
    {
        (input_lb_vp->elements)[n + j] = DBL_MOST_NEGATIVE;

        (input_ub_vp->elements)[n + j]=(le_constraint_col_vp->elements)[j];
    }

    for (j=0; j<eq_m; j++)
    {
#ifdef NO_DBOCLS_EQUALITY_SLOP
        (input_lb_vp->elements)[n + le_m + j] =
                                              eq_constraint_col_vp->elements[j];

        (input_ub_vp->elements)[n + le_m + j] =
                                              eq_constraint_col_vp->elements[j];
#else
#ifdef DBOCLS_EQUALITY_SLOP
        (input_lb_vp->elements)[n + le_m + j] =
                              SUB_RELATIVE_DBL(eq_constraint_col_vp->elements[j],
                                           DBOCLS_EQUALITY_SLOP);

        (input_ub_vp->elements)[n + le_m + j] =
                              ADD_RELATIVE_DBL(eq_constraint_col_vp->elements[j],
                                           DBOCLS_EQUALITY_SLOP);
#else
        (input_lb_vp->elements)[n + le_m + j] =
                              SUB_EPSILON_DBL(eq_constraint_col_vp->elements[j]);

        (input_ub_vp->elements)[n + le_m + j] =
                              ADD_DBL_EPSILON(eq_constraint_col_vp->elements[j]);
#endif
#endif
    }

    MROWS = mp->num_rows;
    NCOLS = n;
    MCON = le_m + eq_m;
    MDW = MCON + MAX_OF(MROWS, NCOLS);

    NRE(X = DBL_MALLOC(2*(NCOLS+MCON)+2));   /* No extra due to options */
    NRE(RW = DBL_MALLOC(6*NCOLS+5*MCON));
    NRE(IW = INT_MALLOC(2*(NCOLS+MCON)));

    NRE(IND = INT_MALLOC(MCON + NCOLS));

    for (i=0; i<MCON + NCOLS; i++)
    {
        if (    ((input_lb_vp->elements)[i] > DBL_HALF_MOST_NEGATIVE / 3.0)
             && ((input_ub_vp->elements)[i] < DBL_HALF_MOST_POSITIVE / 3.0)
           )
        {
            IND[ i ] = 3;
        }
        else if ((input_lb_vp->elements)[i] > DBL_HALF_MOST_NEGATIVE / 3.0)
        {
            IND[ i ] = 1;
        }
        else if ((input_ub_vp->elements)[i] < DBL_HALF_MOST_POSITIVE / 3.0)
        {
            IND[ i ] = 2;
        }
        else
        {
            IND[ i ] = 4;
        }
    }

    NRE(BU = DBL_MALLOC(input_ub_vp->length));
    NRE(BL = DBL_MALLOC(input_lb_vp->length));

    ASSERT(input_ub_vp->length == input_lb_vp->length);

    for (i=0; i<input_ub_vp->length; i++)
    {
        BU[i] = (input_ub_vp->elements)[i];
        BL[i] = (input_lb_vp->elements)[i];
    }

    NRE(W = DBL_MALLOC( MDW * (NCOLS + MCON + 1) ));

    for (i=0; i<MDW * (NCOLS + MCON + 1); i++)
    {
        W[i] = 0.0;
    }

    /* F77 style arrays */

    for (i=0; i<mp->num_rows; i++)
    {
        for (j=0; j<mp->num_cols; j++)
        {
            W[ j*MDW + i + MCON  ] = (mp->elements)[i][j];
        }

        W[ NCOLS*MDW + i + MCON ] = (target_vp->elements)[i];
    }

    /*
    // If constraint_mp is NULL, then m is zero.
    */
    for (i=0; i<m; i++)
    {
        for (j=0; j<n; j++)
        {
            W[ j*MDW + i ] = (constraint_mp->elements)[i][j];
        }
    }

    /*
    // The first option, flagged by the 2, says to check the array sizes. The
    // sizes follow for the next 7 values.
    */
    IOPT[ 0 ] = 2;
    IOPT[ 1 ] = MDW;
    IOPT[ 2 ] = NCOLS + MCON + 1 ;
    IOPT[ 3 ] = input_lb_vp->length;
    IOPT[ 4 ] = 2*(NCOLS+MCON)+2;
    IOPT[ 5 ] = 6*NCOLS+5*MCON;
    IOPT[ 6 ] = 2*(NCOLS+MCON);
    IOPT[ 7 ] = 17 + 9;
    IOPT[ 8 ] = 99;

/* ========================================================================== */

    dbocls_(W, &MDW, &MCON, &MROWS, &NCOLS, BL, BU,
            IND, IOPT, X, &RNORMC, &RNORM, &MODE, RW, IW);

    if (MODE == -22)
    {
        set_high_light(stdout);
        pso("\n");
        rep_print(stdout, '-', 80);
        pso("Warning:\n\n");
        pso("Quadratic programming problem reached max number of ");
        pso("allowed iterations.\n");
        pso("Solution may not be accurate.");
        rep_print(stdout, '-', 80);
        pso("\n\n");
        unset_high_light(stdout);

        result = NO_ERROR;
    }
    else if (MODE < 0)
    {
        set_error("Routine dbocls() failed (MODE=%ld).", (long)MODE);
        result = ERROR;
    }

    if (result != ERROR)
    {
        Vector* est_vp  = NULL;
        Vector* diff_vp = NULL;
        double  rnorm;


        for (i=0; i<result_vp->length; i++)
        {
            (result_vp->elements)[ i ] = X[i];
        }

        if (multiply_matrix_and_vector(&est_vp, mp, result_vp) == ERROR)
        {
            result = ERROR;
        }
        else if (subtract_vectors(&diff_vp, est_vp, target_vp) == ERROR)
        {
            result = ERROR;
        }
        else
        {
            rnorm = vector_magnitude(diff_vp);

            if (    (rnorm > 1000 * DBL_EPSILON)
                 && (fabs(rnorm - RNORM) / rnorm > 0.01)
               )
            {
                set_high_light(stdout);
                pso("\n");
                rep_print(stdout, '-', 80);
                pso("Warning:\n\n");
                pso("Externally computed residual norm differs ");
                pso("from dbocls() RNORM by more than 1%%.\n");
                pso("Computed residual norm is %e.\n", rnorm);
                pso("dbocls() RNORM is %e.\n", RNORM);
                rep_print(stdout, '-', 80);
                pso("\n\n");
                unset_high_light(stdout);
            }

            if (check_quadratic_constraints(result_vp, le_constraint_mp,
                                            le_constraint_col_vp,
                                            lb_vp, ub_vp) == ERROR)
            {
                add_error("Currently this is only treated as a WARNING"); 
                kjb_print_error(); 
            }
        }

        free_vector(est_vp);
        free_vector(diff_vp);
    }

    kjb_free(W);
    kjb_free(BL);
    kjb_free(BU);
    kjb_free(IND);
    kjb_free(X);
    kjb_free(RW);
    kjb_free(IW);

    free_vector(input_lb_vp);
    free_vector(input_ub_vp);
    free_matrix(constraint_mp);

    return result;

}
/*
|                                  /\
|                                 /||\
|                                  ||
|                             KJB_HAVE_SLATEC
----------------------------------------------------------------------------- */
#else
/* -----------------------------------------------------------------------------
|                              no SLATEC
|                                  ||
|                                 \||/
|                                  \/
*/
/*ARGSUSED*/
int do_dbocls_quadratic
(
    Vector**      __attribute__((unused)) dummy_result_vpp,
    const Matrix* __attribute__((unused)) dummy_mp,
    const Vector* __attribute__((unused)) dummy_target_vp,
    const Matrix* __attribute__((unused)) dummy_le_constraint_mp,
    const Vector* __attribute__((unused)) dummy_le_constraint_col_vp,
    const Matrix* __attribute__((unused)) dummy_eq_constraint_mp,
    const Vector* __attribute__((unused)) dummy_eq_constraint_col_vp,
    const Vector* __attribute__((unused)) dummy_lb_vp,
    const Vector* __attribute__((unused)) dummy_ub_vp
)
{


    set_dont_have_slatec_error();
    return ERROR;
}
/*
|                                  /\
|                                 /||\
|                                  ||
|                              no SLATEC
----------------------------------------------------------------------------- */
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef KJB_HAVE_SLATEC
/* -----------------------------------------------------------------------------
|                             KJB_HAVE_SLATEC
|                                  ||
|                                 \||/
|                                  \/
*/
int slatec_i1mach(int* machine_constant_ptr, int machine_constant_id)
{
    int machine_constant_id_arg = machine_constant_id;

    if ((machine_constant_id < 1) || (machine_constant_id > 16))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }
    *machine_constant_ptr = (int)i1mach_(&machine_constant_id_arg);
    return NO_ERROR;
}
/*
|                                  /\
|                                 /||\
|                                  ||
|                             KJB_HAVE_SLATEC
----------------------------------------------------------------------------- */
#else
/* -----------------------------------------------------------------------------
|                              no SLATEC
|                                  ||
|                                 \||/
|                                  \/
*/
int slatec_i1mach
(
    int* __attribute__((unused)) dummy_machine_constant_ptr,
    int  __attribute__((unused)) dummy_machine_constant_id
)
{
    set_dont_have_slatec_error();
    return ERROR;
}
/*
|                                  /\
|                                 /||\
|                                  ||
|                              no SLATEC
----------------------------------------------------------------------------- */
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef KJB_HAVE_SLATEC
/* -----------------------------------------------------------------------------
|                             KJB_HAVE_SLATEC
|                                  ||
|                                 \||/
|                                  \/
*/
int slatec_d1mach(double* machine_constant_ptr, int machine_constant_id)
{
    int machine_constant_id_arg = machine_constant_id;

    if ((machine_constant_id < 1) || (machine_constant_id > 5))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }
    *machine_constant_ptr = d1mach_(&machine_constant_id_arg);
    return NO_ERROR;
}
/*
|                                  /\
|                                 /||\
|                                  ||
|                             KJB_HAVE_SLATEC
----------------------------------------------------------------------------- */
#else
/* -----------------------------------------------------------------------------
|                              no SLATEC
|                                  ||
|                                 \||/
|                                  \/
*/
int slatec_d1mach
(
    double* __attribute__((unused)) dummy_machine_constant_ptr,
    int     __attribute__((unused)) dummy_machine_constant_id
)
{
    set_dont_have_slatec_error();
    return ERROR;
}
/*
|                                  /\
|                                 /||\
|                                  ||
|                              no SLATEC
----------------------------------------------------------------------------- */
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifndef KJB_HAVE_SLATEC
/* -----------------------------------------------------------------------------
|                              no SLATEC
|                                  ||
|                                 \||/
|                                  \/
*/
static void set_dont_have_slatec_error(void)
{
    set_error("Operation failed because the program was built without ");
    cat_error("the SLATEC interface.");
    add_error("(Appropriate re-compiling is needed to fix this.)");
}
/*
|                                  /\
|                                 /||\
|                                  ||
|                              no SLATEC
----------------------------------------------------------------------------- */
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif


