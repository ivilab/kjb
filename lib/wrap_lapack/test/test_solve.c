/* $Id: test_solve.c 21491 2017-07-20 13:19:02Z kobus $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2014 by Kobus Barnard (author)
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

/* vim: tabstop=4 shiftwidth=4 foldmethod=marker */


#include <wrap_lapack/wrap_lapack.h>
#include <m/m_incl.h>
#include <n/n_cholesky.h>
#include <n/n_invert.h>

#define NUM_TRIES   1000

int main()
{
    Matrix* A = NULL;
    Matrix* A_chol = NULL;
    Matrix* B = NULL;
    Matrix* A_inv = NULL;
    Matrix* X_ref = NULL;
    Matrix* X1 = NULL;
    Matrix* X2 = NULL;
    Matrix* X3 = NULL;
    Matrix* X4 = NULL;
    Matrix* tmp = NULL;
    double diff1, diff2, diff3, diff4;
    int i;
    int status = EXIT_SUCCESS;
    static const double TOLERANCE = 1e-7;

    /* Kobus: 17/07/04
     * I am not sure wy we have this. Fancy way to use something other than the
     * default random seed? 
    */
    for(i = 0; i < 100; ++i) 
        kjb_rand();

    for(i = 0; i < NUM_TRIES; ++i)
    {
        EPETE(get_random_matrix(&A, 50, 50));
        EPETE(get_random_matrix(&B, 50, 25));
        EPETE(multiply_by_transpose(&A, A, A));
        EPETE(get_matrix_inverse(&A_inv, A));
        EPETE(multiply_matrices(&X_ref, A_inv, B));

        EPETE(set_cholesky_options("cholesky", "lapack"));
        EPETE(cholesky_decomposition(&A_chol, A));
        EPETE(lapack_solve_triangular(A_chol, B, &X1));
        tmp = X1;
        EPETE(get_matrix_transpose(&A_chol, A_chol));
        EPETE(lapack_solve_upper_triangular(A_chol, tmp, &X1));

        EPETE(lapack_solve_symmetric(A, B, &X2));
        EPETE(lapack_solve_symmetric_pd(A, B, &X3));
        EPETE(lapack_solve(A, B, &X4));

        /* Kobus: 17/07/11
         * This program fails due to results outside tolerance. However, it is
         * not clear if the tolerance limit is reasonable, and/or whether the
         * tolerance should be relative. 
        */

        diff1 = max_abs_matrix_difference(X_ref, X1);
        diff2 = max_abs_matrix_difference(X_ref, X2);
        diff3 = max_abs_matrix_difference(X_ref, X3);
        diff4 = max_abs_matrix_difference(X_ref, X4);


        if (diff1 > TOLERANCE)
        {
            p_stderr("Problem with test 1.\n");
            p_stderr(" %e != 0.0.\n", diff1); 
            status = EXIT_BUG;
        }

        if (diff2 > TOLERANCE)
        {
            p_stderr("Problem with test 2.\n");
            p_stderr(" %e != 0.0.\n", diff2); 
            status = EXIT_BUG;
        }

        if (diff3 > TOLERANCE)
        {
            p_stderr("Problem with test 3.\n");
            p_stderr(" %e != 0.0.\n", diff3); 
            status = EXIT_BUG;
        }

        if (diff4 > TOLERANCE)
        {
            p_stderr("Problem with test 4.\n");
            p_stderr(" %e != 0.0.\n", diff4); 
            status = EXIT_BUG;
        }
    }
    free_matrix(A);
    free_matrix(A_chol);
    free_matrix(B);
    free_matrix(A_inv);
    free_matrix(X_ref);
    free_matrix(X1);
    free_matrix(X2);
    free_matrix(X3);
    free_matrix(X4);

    return status;
}
