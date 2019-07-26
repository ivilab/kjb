
/* $Id: mat_arith_ew.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"

#define NUM_LOOPS_FOR_EW_TEST     50
#define BASE_NUM_TRIES            20


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int num_rows;
    int num_cols; 
    int count;
    Matrix* a_mp = NULL; 
    Matrix* b_mp = NULL; 
    Matrix* c_mp = NULL; 
    Matrix* d_mp = NULL; 
    Matrix* first_mp = NULL;
    Matrix* clone_of_first_mp = NULL;
    Matrix* second_mp = NULL;
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;
    Matrix* ul_mp = NULL;
    Matrix* ur_mp = NULL;
    Matrix* ll_mp = NULL;
    Matrix* lr_mp = NULL;


    kjb_init(); 

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

    for (count=0; count<num_tries; count++)
    {
        for (num_rows=1; num_rows<NUM_LOOPS_FOR_EW_TEST; num_rows++)
        {
            for (num_cols=1; num_cols<NUM_LOOPS_FOR_EW_TEST; num_cols++)
            {
                double  diff;
                double  diff_2;
                double  diff_3;
                int     row_factor, col_factor; 
                int     result; 
                int nr_2 = 2*num_rows;
                int nc_2 = 2*num_cols;
                int i;

                EPETE(get_random_matrix(&first_mp, nr_2, nc_2)); 
                EPETE(copy_matrix(&clone_of_first_mp, first_mp));

                EPETE(get_random_matrix(&ul_mp, num_rows, num_cols)); 
                EPETE(get_random_matrix(&ur_mp, num_rows, num_cols)); 
                EPETE(get_random_matrix(&ll_mp, num_rows, num_cols)); 
                EPETE(get_random_matrix(&lr_mp, num_rows, num_cols)); 

                EPETE(get_initialized_matrix(&second_mp, nr_2, nc_2, DBL_NOT_SET)); 

                EPETE(ow_copy_matrix(second_mp, 0, 0, ul_mp));
                EPETE(ow_copy_matrix(second_mp, 0, num_cols, ur_mp));
                EPETE(ow_copy_matrix(second_mp, num_rows, 0, ll_mp));
                EPETE(ow_copy_matrix(second_mp, num_rows, num_cols, lr_mp));

                EPETE(ow_multiply_matrices_ew(first_mp, second_mp));

                EPETE(ow_multiply_matrices_ew_2(clone_of_first_mp, 0, 0, ul_mp)); 
                EPETE(ow_multiply_matrices_ew_2(clone_of_first_mp, 0, num_cols, ur_mp)); 
                EPETE(ow_multiply_matrices_ew_2(clone_of_first_mp, num_rows, 0, ll_mp)); 
                EPETE(ow_multiply_matrices_ew_2(clone_of_first_mp, num_rows, num_cols, lr_mp)); 

                diff = max_abs_matrix_difference(first_mp, clone_of_first_mp);

                if ( ! IS_ZERO_DBL(diff))
                {
                    p_stderr("Problem with ow_multiply_matrices_ew_2().\n");
                    dbe(diff);
                    status = EXIT_BUG;
                }

                for (i = 0; i < nr_2; i++)
                {
                    EPETE(ow_add_matrix_row_times_scalar(first_mp, i, second_mp, i, M_PI));
                }

                EPETE(ow_multiply_matrix_by_scalar(second_mp, M_PI));
                EPETE(ow_add_matrices(clone_of_first_mp, second_mp)); 
            
                diff = max_abs_matrix_difference(first_mp, clone_of_first_mp);

                if (ABS_OF(diff) > nr_2 * nr_2 * 10.0 * DBL_EPSILON)
                {
                    p_stderr("Problem with ow_add_matrix_row_times_scalar().\n");
                    dbe(diff);
                    status = EXIT_BUG;
                }


                /*
                //   pso("%d %d %d\n", count, num_rows, num_cols);
                */   

                EPETE(get_random_matrix(&first_mp, num_rows, num_cols)); 
                EPETE(get_initialized_matrix(&second_mp, count+2, count+3, 2.0)); 

                result = multiply_matrices_ew(&a_mp, first_mp, second_mp);

                row_factor = num_rows / (count+2);
                col_factor = num_cols / (count+3);

                if (    (num_rows == (row_factor * (count+2)))
                     && (num_cols == (col_factor * (count+3)))
                   )
                {
                    /*
                    //  db_mat(first_mp);
                    //  db_mat(a_mp);
                    */

                    EPETE(add_matrices(&b_mp, first_mp, first_mp));

                    /*
                    //  db_mat(b_mp); 
                    */

                    diff = max_abs_matrix_difference(a_mp, b_mp);

                    EPETE(subtract_matrices(&c_mp, b_mp, first_mp)); 

                    /*
                    //  db_mat(c_mp);
                    */

                    diff_2 = max_abs_matrix_difference(c_mp, first_mp);

                    EPETE(divide_matrices_ew(&d_mp, b_mp, second_mp));

                    /*
                    //  db_mat(d_mp); 
                    */

                    diff_3 = max_abs_matrix_difference(d_mp, first_mp); 

                    if ( ! IS_ZERO_DBL(diff))
                    {
                        p_stderr("Problem comparing add with mult times 2\n");
                        status = EXIT_BUG;
                    }

                    if ( ! IS_ZERO_DBL(diff_2) )
                    {
                        p_stderr("Problem with add then subtract being no-op\n");
                        status = EXIT_BUG;
                    }
                    if ( ! IS_ZERO_DBL(diff_3) )
                    {
                        p_stderr("Problem with times then divide being no-op\n");
                        status = EXIT_BUG;
                    }
                }
                else if (result != ERROR)
                {
                    p_stderr("Problem rejecting dimensions %d %d and %d %d.\n",
                            num_rows, num_cols, count+2, count+3); 
                    status = EXIT_BUG;
                }
                else
                {
                    result = divide_matrices_ew(&a_mp, first_mp, second_mp);
                    
                    if (result != ERROR)
                    {
                        p_stderr("Problem rejecting dimensions %d %d and %d %d in divide_matrices_ew().\n",
                                num_rows, num_cols, count+2, count+3); 
                        status = EXIT_BUG;
                    }
                        
                    result = subtract_matrices(&a_mp, first_mp, second_mp);
                    
                    if (result != ERROR)
                    {
                        p_stderr("Problem rejecting dimensions %d %d and %d %d in subtract_matrices().\n",
                                num_rows, num_cols, count+2, count+3); 
                        status = EXIT_BUG;
                    }
                        
                    result = add_matrices(&a_mp, first_mp, second_mp);
                    
                    if (result != ERROR)
                    {
                        p_stderr("Problem rejecting dimensions %d %d and %d %d add_matrices().\n",
                                num_rows, num_cols, count+2, count+3); 
                        status = EXIT_BUG;
                    }
                }
            }
        }
    }
    
    free_matrix(ul_mp); 
    free_matrix(ur_mp); 
    free_matrix(ll_mp); 
    free_matrix(lr_mp); 
    free_matrix(a_mp); 
    free_matrix(b_mp);
    free_matrix(c_mp);
    free_matrix(d_mp);
    free_matrix(first_mp);
    free_matrix(clone_of_first_mp); 
    free_matrix(second_mp);

    return status; 
}

