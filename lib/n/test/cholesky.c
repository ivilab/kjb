/* =========================================================================== *
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
   |  Author:  Jinyan Guan
 * =========================================================================== */

/* $Id: cholesky.c 21491 2017-07-20 13:19:02Z kobus $ */

#include <wrap_lapack/wrap_lapack.h>
#include <m/m_incl.h>
#include <n/n_cholesky.h>
#include <n/n_invert.h>

#define NUM_TRIES   100

int main(void)
{
    Matrix* A = NULL;
    Matrix* A_chol_lapack = NULL;
    Matrix* A_chol_native = NULL;
    double diff;
    int i;
    int status = EXIT_SUCCESS;
    static const double TOLERANCE = 1e-7;

    /* Kobus: 17-07-16
     * The logic behind calling kjb_rand() 100 times is not clear. A different
     * way to play with seeding?
    */
    for(i = 0; i < 100; ++i) 
        kjb_rand();

    for(i = 0; i < NUM_TRIES; ++i)
    {
        EPETE(get_random_matrix(&A, 50, 50));
        EPETE(multiply_by_transpose(&A, A, A));

        EPETE(set_cholesky_options("cholesky", "lapack"));
        EPETE(cholesky_decomposition(&A_chol_lapack, A));

        EPETE(set_cholesky_options("cholesky", "native"));
        EPETE(cholesky_decomposition(&A_chol_native, A));

        diff = max_abs_matrix_difference(A_chol_lapack, A_chol_native);

        if (diff > TOLERANCE)
        {
            p_stderr("Different results of laplack and native "
                    "cholesky_decomposition.\n");
            p_stderr(" %e != 0.0.\n", diff); 
            status = EXIT_BUG;
        }
    }
    free_matrix(A);
    free_matrix(A_chol_lapack);
    free_matrix(A_chol_native);

    return status;
}
