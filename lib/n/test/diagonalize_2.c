
/* $Id: diagonalize_2.c 21491 2017-07-20 13:19:02Z kobus $ */


/*
 * This program tests the general eigen-solver.
*/

#include "n/n_incl.h"
#include "x/x_incl.h"


#ifdef EXTRA_EXTRA_VERBOSE
#ifndef EXTRA_VERBOSE
#    define EXTRA_VERBOSE
#endif 
#endif 

#ifdef EXTRA_VERBOSE
#    define MAX_SIZE      5
#else 
#    define MAX_SIZE      200
#endif 

#define FIRST_ROW      1
#define FIRST_COL      1
#define BASE_NUM_TRIES 100


/*ARGSUSED*/
int main(int argc, char *argv[])
{
    int status = EXIT_SUCCESS;
    int     num_rows;
    int     num_cols;
    int     count;
    Vector* re_diag_vp            = NULL;
    Vector* im_diag_vp            = NULL;
    Matrix* D_re_mp = NULL; 
    Matrix* D_im_mp = NULL; 
    Matrix* input_mp           = NULL;
    Matrix* t1_re_mp         = NULL;
    Matrix* t1_im_mp         = NULL;
    Matrix* t2_re_mp         = NULL;
    Matrix* t2_im_mp         = NULL;
    Matrix* ident_mp           = NULL;
    Matrix* e_im_mp = NULL; 
    Matrix* e_re_mp = NULL; 
    Matrix* transpose_e_im_mp = NULL; 
    Matrix* transpose_e_re_mp = NULL; 
    Matrix* prod_re_mp            = NULL;
    Matrix* prod_im_mp            = NULL;
    double  diff;
    int num_elements; 
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

        num_cols = num_rows;

        verbose_pso(1, "Try %d with num_rows = %d and num_cols = %d.\n",
                    count + 1, num_rows, num_cols);

        EPETE(get_random_matrix(&input_mp, num_rows, num_cols)); 

#ifdef EXTRA_VERBOSE
        db_mat(input_mp);
#endif 

        ASSERT(input_mp->num_rows == input_mp->num_cols); 

        num_elements = input_mp->num_rows * input_mp->num_cols;

        EPETE(get_identity_matrix(&ident_mp, input_mp->num_cols)); 

        EPETE(diagonalize_2(input_mp, &e_re_mp, &e_im_mp, &re_diag_vp, &im_diag_vp));

#ifdef EXTRA_VERBOSE
        db_rv(re_diag_vp);
        db_rv(im_diag_vp);
        db_mat(e_re_mp);
        db_mat(e_im_mp);
#endif 


        EPETE(get_diagonal_matrix(&D_re_mp, re_diag_vp));
        EPETE(get_diagonal_matrix(&D_im_mp, im_diag_vp));

        EPETE(complex_multiply_matrices(&t1_re_mp, &t1_im_mp, input_mp, NULL, e_re_mp, e_im_mp));
        EPETE(complex_multiply_matrices(&t2_re_mp, &t2_im_mp, e_re_mp, e_im_mp, D_re_mp, D_im_mp));

#ifdef EXTRA_EXTRA_VERBOSE
        db_mat(t1_re_mp); 
        db_mat(t1_im_mp); 
        db_mat(t2_re_mp); 
        db_mat(t2_im_mp); 
#endif 

        diff = max_abs_matrix_difference(t1_re_mp, t2_re_mp) / num_elements;

        verbose_pso(1, "Test 2 diff is %.4e\n", diff);;

        if (diff > 1000.0 * DBL_EPSILON)
        {
            p_stderr("\nTest 1 (size %d): Difference %.3e not sufficiently close to zero.\n\n",
                     input_mp->num_rows, diff);
            db_mat(t1_re_mp);
            db_mat(t2_re_mp);
            status = EXIT_BUG;
        }

        diff = max_abs_matrix_difference(t1_im_mp, t2_im_mp) / num_elements;

        verbose_pso(1, "Test 2 diff is %.4e\n", diff);;

        if (diff > 1000.0 * DBL_EPSILON)
        {
            p_stderr("\nTest 2 (size %d): Difference %.3e not sufficiently close to zero.\n\n",
                     input_mp->num_rows, diff);
            db_mat(t1_im_mp);
            db_mat(t2_im_mp);
            status = EXIT_BUG;
        }

    }

    free_matrix(e_re_mp);
    free_matrix(e_im_mp);
    free_matrix(transpose_e_re_mp);
    free_matrix(transpose_e_im_mp);
    free_matrix(t1_re_mp);
    free_matrix(t1_im_mp);
    free_matrix(t2_re_mp);
    free_matrix(t2_im_mp);
    free_vector(re_diag_vp);
    free_vector(im_diag_vp);
    free_matrix(input_mp); 
    free_matrix(ident_mp); 
    free_matrix(prod_re_mp);
    free_matrix(prod_im_mp);
    free_matrix(D_re_mp);
    free_matrix(D_im_mp);

    return status;
}

