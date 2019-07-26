
/* $Id: diagonalize.c 21662 2017-08-05 17:00:07Z kobus $ */


/* Suspicious code --- calling diagonalize() multiple times on the mac (yet to
 * study on linux) with the same input leads to slightly different answers?
 *
 * This might be due to multi-threated stochastic optimization? 
 *
 * Study with NUM_IDENTICAL_COMPUTATIONS greater than 1. Currently we call small
 * numbers in the answers 0, which allows regression testing to succeeed. 
*/

/*
 * This program tests the general eigen-solver in the case of symmetric
 * matrices (ONLY).
*/

#include "n/n_incl.h"

#ifdef TEST_REPRODUCIBLE
#    define NUM_IDENTICAL_COMPUTATIONS 5
#    define MAKE_REPRODUCIBLE(X)  
#else 
#    define NUM_IDENTICAL_COMPUTATIONS 1
#    define MAKE_REPRODUCIBLE(X)  {X = (((X>-1.0e-14)&&(X<1.0e-14)) ? 0.0 : X);}
#endif 

/*
#define VERBOSE 1 
#define EXTRA_VERBOSE 1 
*/

#ifdef EXTRA_VERBOSE
#ifndef VERBOSE
#    define VERBOSE
#endif 
#endif 

#ifdef EXTRA_VERBOSE
#    define MAX_SIZE      5
#else 
#    define MAX_SIZE      100
#endif 

#define FIRST_ROW      1
#define FIRST_COL      1
#define BASE_NUM_TRIES   1000

/*ARGSUSED*/
int main(int argc, char* argv[])
{
    int status = EXIT_SUCCESS;
    int     num_rows;
    int     num_cols;
    int     count;
    Vector* diag_vp            = NULL;
    Vector* save_diag_vp            = NULL;
    Matrix* first_mp           = NULL;
    Matrix* sym_mp           = NULL;
    Matrix* save_sym_mp           = NULL;
    Matrix* t1_mp         = NULL;
    Matrix* t2_mp         = NULL;
    Matrix* ident_mp           = NULL;
    Matrix* e_mp = NULL; 
    Matrix* save_e_mp = NULL; 
    Matrix* prod_mp            = NULL;
    double  diff;
    int num_elements; 
    int i, j;
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;
    int  again;


    kjb_init(); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (test_factor <= 0)
    {
        num_tries = 1;
    }
    else
    {
        num_tries *= test_factor;
    } 

    if (is_interactive())
    {
        EPETE(set_verbose_options("verbose", "1")); 
    }

    for (count=0; count<num_tries; count++)
    {
        num_rows = 1 + kjb_rint(MAX_SIZE * kjb_rand());
        num_cols = num_rows + kjb_rint(MAX_SIZE * kjb_rand());

        verbose_pso(1, "Try %d with num_rows = %d and num_cols = %d.\n",
                    count + 1, num_rows, num_cols);


        if (kjb_rand() < 0.5)
        {
            EPETE(get_random_matrix(&first_mp, num_rows, num_cols)); 
            EPETE(ow_subtract_scalar_from_matrix(first_mp, 0.5)); 
            EPETE(multiply_by_own_transpose(&sym_mp, first_mp));

            ASSERT(sym_mp->num_rows == MIN_OF(first_mp->num_rows,
                                              first_mp->num_cols)); 
        }
        else
        {
            EPETE(get_random_matrix(&sym_mp,
                                    MIN_OF(num_rows, num_cols), 
                                    MIN_OF(num_rows, num_cols)));
            EPETE(ow_subtract_scalar_from_matrix(sym_mp, 0.5)); 

            for (i = 0; i < MIN_OF(num_rows, num_cols); i++)
            {
                for (j = 0; j < MIN_OF(num_rows, num_cols); j++)
                {
                    if (j < i)
                    {
                        sym_mp->elements[ i ][ j ] = sym_mp->elements[ j ][ i ];
                    }
                }
            }
        }

#ifdef VERBOSE
        fp_write_matrix(sym_mp, stdout);
#endif 

        /* 
         * This is used to prove that we do not always get the same answer from
         * the lapack library. This is probably due to multi-threading? 
        */

        for (again = 0; again < NUM_IDENTICAL_COMPUTATIONS; again++)
        {
            double save_diff_A = DBL_NOT_SET;
            double save_diff_B = DBL_NOT_SET;
            double save_diff_C = DBL_NOT_SET;
            double save_diff_D = DBL_NOT_SET;

            ASSERT(sym_mp->num_rows == sym_mp->num_cols); 

            num_elements = sym_mp->num_rows * sym_mp->num_cols;

            EPETE(get_identity_matrix(&ident_mp, sym_mp->num_cols)); 

            if (! is_symmetric_matrix(sym_mp))
            {
                set_error("Presumed symmetric matrix is not symmetric"); 
                kjb_print_error();
                kjb_exit(EXIT_BUG); 
            }

            if (again > 0) 
            {
                diff = max_abs_matrix_difference(sym_mp, save_sym_mp);
                ASSERT(diff == 0.0);
            }
            EPETE(copy_matrix(&save_sym_mp, sym_mp)); 

            EPETE(diagonalize(sym_mp, &e_mp, &diag_vp));

            if (again > 0) 
            {
                diff = max_abs_vector_difference(diag_vp, save_diag_vp);
                ASSERT(diff == 0.0);
                diff = max_abs_matrix_difference(e_mp, save_e_mp);
                ASSERT(diff == 0.0);
            }
            EPETE(copy_vector(&save_diag_vp, diag_vp));
            EPETE(copy_matrix(&save_e_mp, e_mp));

#ifdef VERBOSE
            fp_write_row_vector(diag_vp, stdout);
            fp_write_matrix(e_mp, stdout);
#endif 

            EPETE(multiply_by_own_transpose(&prod_mp, e_mp));

#ifdef EXTRA_VERBOSE
            fp_write_matrix(prod_mp, stdout);
#endif 
            /* How accurate is diagonalization? */
            diff = max_abs_matrix_difference(prod_mp, ident_mp) / sqrt((double)num_elements); 

            if (diff > 1000.0 * DBL_EPSILON)
            {
                p_stderr("Test 1 (size %d): Difference %.3e not sufficiently close to zero.\n",
                         sym_mp->num_rows, diff);
                status = EXIT_BUG;
            }

            MAKE_REPRODUCIBLE(diff)
            verbose_pso(1, "A Diff is %.4e\n", diff);;

            if (again > 0) 
            {
                ASSERT(diff == save_diff_A);
            }
            save_diff_A = diff; 
            

            EPETE(multiply_with_transpose(&prod_mp, e_mp, e_mp));

#ifdef EXTRA_VERBOSE
            fp_write_matrix(prod_mp, stdout);
#endif 
            /* How accurate is diagonalization? */
            diff = max_abs_matrix_difference(prod_mp, ident_mp) / sqrt((double)num_elements); 

            if (diff > 1000.0 * DBL_EPSILON)
            {
                p_stderr("Test 2 (size %d): Difference %.3e not sufficiently close to zero.\n",
                         sym_mp->num_rows, diff);
                status = EXIT_BUG;
            }

            MAKE_REPRODUCIBLE(diff)
            verbose_pso(1, "B Diff is %.4e\n", diff);;

            if (again > 0) 
            {
                ASSERT(diff == save_diff_B);
            }
            save_diff_B = diff; 

            EPETE(multiply_by_transpose(&prod_mp, e_mp, e_mp));

#ifdef EXTRA_VERBOSE
            fp_write_matrix(prod_mp, stdout);
#endif 
            /* How accurate is diagonalization? */
            diff = max_abs_matrix_difference(prod_mp, ident_mp) / sqrt((double)num_elements); 

            if (diff > 1000.0 * DBL_EPSILON)
            {
                p_stderr("Test 3 (size %d): Difference %.3e not sufficiently close to zero.\n",
                         sym_mp->num_rows, diff);
                status = EXIT_BUG;
            }

            MAKE_REPRODUCIBLE(diff)
            verbose_pso(1, "C Diff is %.4e\n", diff);;

            if (again > 0) 
            {
                ASSERT(diff == save_diff_C);
            }
            save_diff_C = diff; 

            EPETE(multiply_with_transpose(&t1_mp, sym_mp, e_mp));

            EPETE(multiply_matrix_by_row_vector_ew(&t2_mp, e_mp, diag_vp));

#ifdef EXTRA_VERBOSE
            fp_write_matrix(t2_mp, stdout); 
#endif 

            /* How accurate is diagonalization? */
            diff = max_abs_matrix_difference(t1_mp, t2_mp) / sqrt((double)num_elements);

            if (diff > 1000.0 * DBL_EPSILON)
            {
                p_stderr("Test 4 (size %d): Difference %.3e not sufficiently close to zero.\n",
                         sym_mp->num_rows, diff);
                status = EXIT_BUG;
            }

            MAKE_REPRODUCIBLE(diff)
            verbose_pso(1, "D Diff is %.4e\n", diff);;

            if (again > 0) 
            {
                ASSERT(diff == save_diff_D);
            }
            save_diff_D = diff; 
        }
    }

    free_matrix(e_mp);
    free_matrix(save_e_mp);
    free_matrix(t1_mp);
    free_matrix(t2_mp);
    free_vector(diag_vp);
    free_vector(save_diag_vp);
    free_matrix(sym_mp); 
    free_matrix(save_sym_mp); 
    free_matrix(first_mp); 
    free_matrix(ident_mp); 
    free_matrix(prod_mp);

    return status;
}

