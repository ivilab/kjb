
/* $Id: mat_arith_dot.c 21555 2017-07-23 23:48:57Z kobus $ */


#include "m/m_incl.h"

#define NUM_LOOPS_FOR_MULT_TEST    20
#define BASE_NUM_TRIES              5

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int num_rows;
    int num_cols; 
    int count;
    int status = EXIT_SUCCESS; 
    Matrix* mp = NULL;
    Matrix* result_1_mp = NULL;
    Matrix* result_2_mp = NULL;
    Matrix* result_3_mp = NULL;
    Matrix* result_4_mp = NULL;
    Vector* v1_vp = NULL;
    Vector* v2_vp = NULL;
    int  num_tries = BASE_NUM_TRIES;
    int num_loops_for_mult_test = NUM_LOOPS_FOR_MULT_TEST;
    int  test_factor = 1;


    kjb_init(); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (test_factor == 0)
    {
        num_tries = 1;
        num_loops_for_mult_test = 10;
    }
    else
    {
        num_tries *= test_factor;
        num_loops_for_mult_test *= test_factor;
    }

    for (count=0; count<num_tries; count++)
    {
        for (num_rows=1; num_rows<num_loops_for_mult_test; num_rows++)
        {
            for (num_cols=1; num_cols<num_loops_for_mult_test; num_cols++)
            {
                double  diff_1;
                double  diff_2;
                double  diff_3;
                double  diff_4;
                int     i, j; 
                int     out_dim = num_rows; 
                double dot_prod; 


                EPETE(get_random_matrix(&mp, num_rows, num_cols)); 
                EPETE(get_zero_matrix(&result_1_mp, out_dim, out_dim));
                EPETE(get_zero_matrix(&result_2_mp, out_dim, out_dim));

                for (i = 0; i < out_dim; i++)
                {
                    for (j = 0; j < out_dim; j++)
                    {
                        EPETE(get_matrix_row(&v1_vp, mp, i));
                        EPETE(get_matrix_row(&v2_vp, mp, j));
                        EPETE(get_dot_product(v1_vp, v2_vp, &dot_prod));
                        result_1_mp->elements[ i ][ j ] = dot_prod;
                    }
                }

                for (i = 0; i < out_dim; i++)
                {
                    for (j = 0; j < out_dim; j++)
                    {
                        EPETE(get_dot_product_of_matrix_rows(mp, i, j, &dot_prod));
                        result_2_mp->elements[ i ][ j ] = dot_prod;
                    }
                }

                EPETE(multiply_by_transpose(&result_3_mp, mp, mp));

                EPETE(multiply_by_own_transpose(&result_4_mp, mp));

                diff_1 = max_abs_matrix_difference(result_1_mp, result_2_mp) / out_dim;
                diff_2 = max_abs_matrix_difference(result_2_mp, result_3_mp) / out_dim;
                diff_3 = max_abs_matrix_difference(result_3_mp, result_4_mp) / out_dim;
                diff_4 = max_abs_matrix_difference(result_4_mp, result_1_mp) / out_dim;

                /*
                dbe(diff_1);
                dbe(diff_2);
                dbe(diff_3);
                dbe(diff_4);
                */


                if ((ABS_OF(diff_1)) > 10.0 * DBL_EPSILON)
                {
                    p_stderr("(%d %d)  %e != 0.0.\n", num_rows, num_cols, 
                             diff_1); 
                    p_stderr("Problem with test 1.\n");
                    status = EXIT_BUG;
                }

                if ((ABS_OF(diff_2) ) > 10.0 * DBL_EPSILON)
                {
                    p_stderr("(%d %d)  %e != 0.0.\n", num_rows, num_cols, 
                             diff_2); 
                    p_stderr("Problem test 2.\n");
                    status = EXIT_BUG;
                }

                if ((ABS_OF(diff_3) ) > 10.0 * DBL_EPSILON)
                {
                    p_stderr("(%d %d)  %e != 0.0.\n", num_rows, num_cols, 
                             diff_3); 
                    p_stderr("Problem test 3.\n");
                    status = EXIT_BUG;
                }

                if ((ABS_OF(diff_4) ) > 10.0 * DBL_EPSILON)
                {
                    p_stderr("(%d %d)  %e != 0.0.\n", num_rows, num_cols, 
                             diff_4); 
                    p_stderr("Problem with test 4.\n");
                    status = EXIT_BUG;
                }

            }
        }
    }

    free_matrix(mp);
    free_matrix(result_1_mp);
    free_matrix(result_2_mp);
    free_matrix(result_3_mp);
    free_matrix(result_4_mp);
    free_vector(v1_vp); 
    free_vector(v2_vp); 

    return status; 
}

