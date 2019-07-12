
/* $Id: int_mat_basic.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"

/*
#define VERBOSE           1
*/


#define NUM_LOOPS       50
#define BASE_NUM_TRIES  100


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int num_rows;
    int num_cols; 
    int count;
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;
    Matrix*   rand_mp = NULL; 
    Int_matrix* first_mp = NULL;
    Int_matrix* second_mp = NULL;
    Int_matrix* third_mp = NULL;
    Int_matrix* trans_mp = NULL;
    Int_vector* row_vp = NULL;
    Int_vector* col_vp = NULL;
    int result = EXIT_SUCCESS;


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
        for (num_rows=1; num_rows<NUM_LOOPS; num_rows++)
        {
            for (num_cols=1; num_cols<NUM_LOOPS; num_cols++)
            {
                int  diff_1;
                int  diff_2;
                int  diff_3;
                int  i, j; 


                /*
                //   pso("%d %d %d\n", count, num_rows, num_cols);
                */   

                EPETE(get_random_matrix(&rand_mp, num_rows, num_cols));
                EPETE(ow_subtract_scalar_from_matrix(rand_mp, 0.5)); 
                EPETE(ow_multiply_matrix_by_scalar(rand_mp, 2000.0)); 
                EPETE(copy_matrix_to_int_matrix(&first_mp, rand_mp));
                EPETE(get_target_int_matrix(&second_mp, num_cols, num_rows));
                EPETE(get_target_int_matrix(&third_mp, num_cols, num_rows));
                EPETE(get_int_transpose(&trans_mp, first_mp));

#ifdef VERBOSE
                dbp("\n -------------------------------------------------- \n");
                db_mat(first_mp);
#endif 

                for (i=0; i<num_rows; i++)
                {
                    EPETE(get_int_matrix_row(&row_vp, first_mp, i));

#ifdef VERBOSE
                    db_rv(row_vp);
#endif 

                    EPETE(put_int_matrix_col(second_mp, row_vp, i));
                }

                for (j=0; j<num_cols; j++)
                {
                    EPETE(get_int_matrix_col(&col_vp, first_mp, j));

#ifdef VERBOSE
                    db_cv(col_vp);
#endif 

                    EPETE(put_int_matrix_row(third_mp, col_vp, j));
                }

#ifdef VERBOSE
                db_mat(trans_mp); 
                db_mat(second_mp);
                db_mat(third_mp);
#endif 

                diff_1 = max_abs_int_matrix_difference(trans_mp, second_mp);
                diff_2 = max_abs_int_matrix_difference(trans_mp, third_mp);

                EPETE(get_int_transpose(&trans_mp, trans_mp));
                diff_3 = max_abs_int_matrix_difference(trans_mp, first_mp);


                if (diff_1 != 0)
                {
                    p_stderr("Problem with first test (%d != 0).\n", diff_1);
                    result = EXIT_BUG;
                }

                if (diff_2 != 0)
                {
                    p_stderr("Problem with second test (%d != 0).\n", diff_2);
                    result = EXIT_BUG;
                }

                if (diff_3 != 0)
                {
                    p_stderr("Problem with third test (%d != 0).\n", diff_3);
                    result = EXIT_BUG;
                }
            }
        }
    }

    free_matrix(rand_mp); 
    free_int_vector(row_vp); 
    free_int_vector(col_vp); 
    free_int_matrix(first_mp);
    free_int_matrix(second_mp);
    free_int_matrix(third_mp); 
    free_int_matrix(trans_mp); 

    return result; 
}

