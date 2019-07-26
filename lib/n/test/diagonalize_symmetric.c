
/* $Id: diagonalize_symmetric.c 21491 2017-07-20 13:19:02Z kobus $ */

 
/*
 * This program tests the general eigen-solver in the case of symmetric
 * matrices.
*/

#include "n/n_incl.h"

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
#    define MAX_SIZE      200
#endif 

#define FIRST_ROW      1
#define FIRST_COL      1
#define BASE_NUM_TRIES   100


/*ARGSUSED*/
int main(int argc, char *argv[])
{
    int status = EXIT_SUCCESS;
    int     num_rows;
    int     num_cols;
    int     count;
    Vector* diag_vp            = NULL;
    Vector* non_sym_diag_vp    = NULL;
    Matrix* first_mp           = NULL;
    Matrix* sym_mp           = NULL;
    Matrix* t1_mp         = NULL;
    Matrix* t2_mp         = NULL;
    Matrix* ident_mp           = NULL;
    Matrix* e_mp = NULL; 
    Matrix* non_sym_e_mp = NULL; 
    Matrix* prod_mp            = NULL;
    double  diff;
    int num_elements; 
    long sym_cpu_time; 
    long non_sym_cpu_time; 
    double relative_cpu_time_diff_sum = 0.0; 
    int    diagonalization_count = 0; 
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;


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

        EPETE(get_random_matrix(&first_mp, num_rows, num_cols)); 

        EPETE(multiply_by_own_transpose(&sym_mp, first_mp));

        if (! is_symmetric_matrix(sym_mp))
        {
            set_error("Presumed symmetric matrix is not symmetric"); 
            kjb_print_error();
            status = EXIT_BUG;
        }

#ifdef VERBOSE
        db_mat(sym_mp);
#endif 
#ifdef EXTRA_VERBOSE
        dbi(first_mp->num_rows);
        dbi(first_mp->num_cols);
        dbi(sym_mp->num_rows);
        dbi(sym_mp->num_cols);
#endif 

        ASSERT(sym_mp->num_rows == MIN_OF(first_mp->num_rows,
                                          first_mp->num_cols)); 
        ASSERT(sym_mp->num_rows == sym_mp->num_cols); 

        num_elements = sym_mp->num_rows * sym_mp->num_cols;

        EPETE(get_identity_matrix(&ident_mp, sym_mp->num_cols)); 

#ifdef VERBOSE
        EPETE(diagonalize_symmetric(sym_mp, NULL, &diag_vp));
        db_rv(diag_vp);
#endif 

        init_cpu_time(); 
        EPETE(diagonalize_symmetric(sym_mp, &e_mp, &diag_vp));
        sym_cpu_time = get_cpu_time(); 


        init_cpu_time(); 
        EPETE(diagonalize(sym_mp, &non_sym_e_mp, &non_sym_diag_vp));
        non_sym_cpu_time = get_cpu_time(); 

        if (non_sym_cpu_time + sym_cpu_time > 0)
        {
            relative_cpu_time_diff_sum += (2.0 * (non_sym_cpu_time - sym_cpu_time)) / ((double)(non_sym_cpu_time + sym_cpu_time));
            diagonalization_count++; 
        }

        diff = max_abs_vector_difference(diag_vp, non_sym_diag_vp) / num_rows; 

        verbose_pso(1, "Diff is %.4e\n", diff);;

        if (diff > 1000.0 * DBL_EPSILON)
        {
            p_stderr("Test 0 (size %d): Difference %.3e not sufficiently close to zero.",
                     sym_mp->num_rows, diff);
            status = EXIT_BUG;
        }

#ifdef VERBOSE
        db_rv(diag_vp);
        db_mat(e_mp);
#endif 

        EPETE(multiply_by_own_transpose(&prod_mp, e_mp));

#ifdef EXTRA_VERBOSE
        db_mat(prod_mp);
#endif 
        diff = max_abs_matrix_difference(prod_mp, ident_mp) / num_elements; 

        verbose_pso(1, "Diff is %.4e\n", diff);;

        if (diff > 1000.0 * DBL_EPSILON)
        {
            p_stderr("Test 1 (size %d): Difference %.3e not sufficiently close to zero.",
                     sym_mp->num_rows, diff);
            status = EXIT_BUG;
        }

        EPETE(multiply_with_transpose(&prod_mp, e_mp, e_mp));

#ifdef EXTRA_VERBOSE
        db_mat(prod_mp);
#endif 
        diff = max_abs_matrix_difference(prod_mp, ident_mp) / num_elements; 

        verbose_pso(1, "Diff is %.4e\n", diff);;

        if (diff > 1000.0 * DBL_EPSILON)
        {
            p_stderr("Test 2 (size %d): Difference %.3e not sufficiently close to zero.",
                     sym_mp->num_rows, diff);
            status = EXIT_BUG;
        }

        EPETE(multiply_by_transpose(&prod_mp, e_mp, e_mp));

#ifdef EXTRA_VERBOSE
        db_mat(prod_mp);
#endif 
        diff = max_abs_matrix_difference(prod_mp, ident_mp) / num_elements; 

        verbose_pso(1, "Diff is %.4e\n", diff);;

        if (diff > 1000.0 * DBL_EPSILON)
        {
            p_stderr("Test 3 (size %d): Difference %.3e not sufficiently close to zero.",
                     sym_mp->num_rows, diff);
            status = EXIT_BUG;
        }

        EPETE(multiply_with_transpose(&t1_mp, sym_mp, e_mp));

        EPETE(multiply_matrix_by_row_vector_ew(&t2_mp, e_mp, diag_vp));

#ifdef EXTRA_VERBOSE
        db_mat(t2_mp); 
#endif 

        diff = max_abs_matrix_difference(t1_mp, t2_mp) / num_elements;

        verbose_pso(1, "Diff is %.4e\n", diff);;

        if (diff > 1000.0 * DBL_EPSILON)
        {
            p_stderr("Test 4 (size %d): Difference %.3e not sufficiently close to zero.",
                     sym_mp->num_rows, diff);
            status = EXIT_BUG;
        }
    }

    verbose_pso(1, "Relative time saved by symmetric routine is %.3f%% (ignore line for regression testing).\n",
                100.0 * relative_cpu_time_diff_sum / (double)diagonalization_count); 

    free_matrix(e_mp);
    free_matrix(non_sym_e_mp);
    free_matrix(t1_mp);
    free_matrix(t2_mp);
    free_vector(diag_vp);
    free_vector(non_sym_diag_vp);
    free_matrix(sym_mp); 
    free_matrix(first_mp); 
    free_matrix(ident_mp); 
    free_matrix(prod_mp);

    return status;
}

