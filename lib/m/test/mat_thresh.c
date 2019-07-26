
/* $Id: mat_thresh.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"

/*
#define VERBOSE 1
*/

#define NUM_LOOPS        20
#define BASE_NUM_TRIES   10


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int num_rows;
    int num_cols; 
    int count;
    Matrix* first_mp = NULL;
    Matrix* second_mp = NULL;
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;


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
                double diff;


#ifdef VERBOSE
                pso("\n-------------------------------------------------\n\n");
                pso("%d %d %d\n\n", count, num_rows, num_cols);
#endif 

                
                EPETE(get_random_matrix(&first_mp, num_rows, num_cols)); 

#ifdef VERBOSE
                db_mat(first_mp); 
#endif 

                EPETE(min_thresh_matrix(&second_mp, first_mp, 0.2)); 
                EPETE(ow_min_thresh_matrix(first_mp, 0.2)); 

#ifdef VERBOSE
                db_mat(second_mp); 
                db_mat(first_mp); 
#endif 

                if (min_matrix_element(first_mp) < 0.2)
                {
                    p_stderr("Min matrix element is less than threshold.\n");
                    status = EXIT_BUG;
                }

                if (min_matrix_element(second_mp) < 0.2)
                {
                    p_stderr("Min matrix element is less than threshold.\n");
                    status = EXIT_BUG;
                }

                diff = max_abs_matrix_difference(first_mp, second_mp); 

                if ( ! (IS_ZERO_DBL(diff)) )
                {
                    p_stderr("Min thresh and ow min thresh not the same.\n"); 
                    status = EXIT_BUG;
                }






                EPETE(get_random_matrix(&first_mp, num_rows, num_cols)); 

#ifdef VERBOSE
                db_mat(first_mp); 
#endif 

                EPETE(max_thresh_matrix(&second_mp, first_mp, 0.8)); 
                EPETE(ow_max_thresh_matrix(first_mp, 0.8)); 

#ifdef VERBOSE
                db_mat(second_mp); 
                db_mat(first_mp); 
#endif 

                if (max_matrix_element(first_mp) > 0.8)
                {
                    p_stderr("Max matrix element is less than threshold.\n");
                    status = EXIT_BUG;
                }

                if (max_matrix_element(second_mp) > 0.8)
                {
                    p_stderr("Max matrix element is less than threshold.\n");
                    status = EXIT_BUG;
                }

                diff = max_abs_matrix_difference(first_mp, second_mp); 

                if ( ! (IS_ZERO_DBL(diff)) )
                {
                    p_stderr("Max thresh and ow max thresh not the same.\n"); 
                    status = EXIT_BUG;
                }

            }
        }
    }
    
    free_matrix(first_mp); 
    free_matrix(second_mp); 


    return status; 
}

