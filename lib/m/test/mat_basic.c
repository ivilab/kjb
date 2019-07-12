
/* $Id: mat_basic.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"

/*
#define VERBOSE           1
*/


#define NUM_LOOPS       50
#define BASE_NUM_TRIES  100


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int num_rows;
    int num_cols; 
    int count;
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;
    Matrix* first_mp = NULL;
    Matrix* second_mp = NULL;
    Matrix* third_mp = NULL;
    Matrix* trans_mp = NULL;
    Vector* row_vp = NULL;
    Vector* col_vp = NULL;


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
                double  diff_1;
                double  diff_2;
                double  diff_3;
                int     i, j; 


                /*
                //   pso("%d %d %d\n", count, num_rows, num_cols);
                */   

                EPETE(get_random_matrix(&first_mp, num_rows, num_cols));
                EPETE(get_target_matrix(&second_mp, num_cols, num_rows));
                EPETE(get_target_matrix(&third_mp, num_cols, num_rows));
                EPETE(get_transpose(&trans_mp, first_mp));

#ifdef VERBOSE
                dbp("\n -------------------------------------------------- \n");
                db_mat(first_mp);
#endif 

                for (i=0; i<num_rows; i++)
                {
                    EPETE(get_matrix_row(&row_vp, first_mp, i));

#ifdef VERBOSE
                    db_rv(row_vp);
#endif 

                    EPETE(put_matrix_col(second_mp, row_vp, i));
                }

                for (j=0; j<num_cols; j++)
                {
                    EPETE(get_matrix_col(&col_vp, first_mp, j));

#ifdef VERBOSE
                    db_cv(col_vp);
#endif 

                    EPETE(put_matrix_row(third_mp, col_vp, j));
                }

#ifdef VERBOSE
                db_mat(trans_mp); 
                db_mat(second_mp);
                db_mat(third_mp);
#endif 

                diff_1 = max_abs_matrix_difference(trans_mp, second_mp);
                diff_2 = max_abs_matrix_difference(trans_mp, third_mp);

                EPETE(get_transpose(&trans_mp, trans_mp));
                diff_3 = max_abs_matrix_difference(trans_mp, first_mp);


                if ( ! IS_ZERO_DBL(diff_1))
                {
                    p_stderr("Problem with first test (%e != 0.0).\n", diff_1);
                    status = EXIT_BUG;
                }

                if ( ! IS_ZERO_DBL(diff_2) )
                {
                    p_stderr("Problem with second test (%e != 0.0).\n", diff_2);
                    status = EXIT_BUG;
                }

                if ( ! IS_ZERO_DBL(diff_3) )
                {
                    p_stderr("Problem with third test (%e != 0.0).\n", diff_3);
                    status = EXIT_BUG;
                }
            }
        }
    }

    free_vector(row_vp); 
    free_vector(col_vp); 
    free_matrix(first_mp);
    free_matrix(second_mp);
    free_matrix(third_mp); 
    free_matrix(trans_mp); 

    return status; 
}

