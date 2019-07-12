
/* $Id: matrix.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"

/*
#define VERBOSE 1
*/


#define MAX_NUM_ROWS   500
#define MAX_NUM_COLS   500
#define BASE_NUM_TRIES   10

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int num_rows;
    int num_cols;
    int count;
    Matrix* first_mp = NULL;
    Matrix* third_mp = NULL;
    Matrix* second_mp = NULL;
    Matrix* fourth_mp = NULL;
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;
    Bool  verify_failure = FALSE;

    kjb_init(); 

    /* Test corner cases: 0 x 0 matrix, n x 0 matrix, or 0 x n */
    EPETE(get_target_matrix(&first_mp, 0, 0));
    dbx(first_mp->elements);
    ow_set_matrix(first_mp, -1.0); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (test_factor == 0)
    {
        num_tries = 1;
    }
    else
    {
        num_tries *= test_factor;
    }

    kjb_set_debug_level(2);

    for (count=0; count<num_tries; count++)
    {
        double  diff_1;
        double  diff_2;
        int     i, j; 

        num_rows = 1 + MAX_NUM_ROWS * kjb_rand();
        num_cols = 1 + MAX_NUM_ROWS * kjb_rand();


#ifdef VERBOSE
        p_stderr("%d %d %d\n", count, num_rows, num_cols);
#endif 

        EPETE(get_zero_matrix(&first_mp, num_rows, num_cols)); 
        verify_matrix(first_mp, &verify_failure); 
        if (verify_failure) { kjb_exit(EXIT_BUG); }

        for (i=0; i<first_mp->num_rows; i++)
        {
            for (j=0; j<first_mp->num_cols; j++)
            {
                ASSERT_IS_EQUAL_DBL(MAT_VAL(first_mp, i , j), 0.0); 
            }
        }

        EPETE(get_target_matrix(&second_mp, num_rows, num_cols)); 

        for (i=0; i<second_mp->num_rows; i++)
        {
            for (j=0; j<second_mp->num_cols; j++)
            {
                second_mp->elements[ i ][ j ] = 1.0; 
            }
        }

        verify_matrix(second_mp, &verify_failure); 
        if (verify_failure) { kjb_exit(EXIT_BUG); }

        EPETE(multiply_matrix_by_scalar(&third_mp, second_mp, 
                                        (double)count)); 
        verify_matrix(third_mp, &verify_failure); 
        if (verify_failure) { kjb_exit(EXIT_BUG); }

        EPETE(get_target_matrix(&fourth_mp, num_rows, num_cols)); 

        EPETE(ow_set_matrix(fourth_mp, (double)count));
        verify_matrix(fourth_mp, &verify_failure); 
        if (verify_failure) { kjb_exit(EXIT_BUG); }

#ifdef REALLY_VERBOSE 
        db_mat(first_mp);
        db_mat(second_mp);
        db_mat(third_mp);
        db_mat(fourth_mp);
#endif 

        diff_1 = max_abs_matrix_element(first_mp);

        diff_2 = max_abs_matrix_difference(third_mp, fourth_mp);

        if ( ! IS_ZERO_DBL(diff_1))
        {
            p_stderr("%e != 0.0.\n", diff_1); 
            p_stderr("Problem with test one.\n");
            status = EXIT_BUG;
        }

        if ( ! IS_ZERO_DBL(diff_2) )
        {
            p_stderr("%e != 0.0.\n", diff_2); 
            p_stderr("Problem with thest two.\n");
            status = EXIT_BUG;
        }

    }
    
    free_matrix(second_mp); 
    free_matrix(fourth_mp); 
    free_matrix(first_mp); 
    free_matrix(third_mp); 

    return status; 
}

