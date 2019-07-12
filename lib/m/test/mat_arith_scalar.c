
/* $Id: mat_arith_scalar.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"

/*
#define VERBOSE 1
*/

#define NUM_LOOPS       50
#define BASE_NUM_TRIES  20

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int num_rows;
    int num_cols;
    int count;
    Matrix* first_mp = NULL;
    Matrix* second_mp = NULL;
    Matrix* diff_mp = NULL;
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
        if (IS_EVEN(count))
        {
            enable_respect_missing_values();
        }
        else 
        {
            disable_respect_missing_values();
        }

#ifdef VERBOSE
        dbi(respect_missing_values());
#endif 

        for (num_rows=1; num_rows<NUM_LOOPS; num_rows++)
        {
            for (num_cols=1; num_cols<NUM_LOOPS; num_cols++)
            {
                double  diff_1;
                double  diff_2;
                double  diff_3;
                double  diff_4;
                double  diff_5;
                double  scalar = count + 1; 

#ifdef VERBOSE
                pso("\n --------------------- \n");
                pso("%.1f %d %d\n\n", scalar, num_rows, num_cols);
#endif 

                EPETE(get_random_matrix(&first_mp, num_rows, num_cols)); 

                first_mp->elements[ num_rows / 2 ][ num_cols / 2 ] = DBL_MISSING; 

#ifdef VERBOSE
                db_mat(first_mp);
#endif 


                EPETE(add_scalar_to_matrix(&second_mp, first_mp, scalar));

#ifdef VERBOSE
                db_mat(second_mp);
#endif 

                EPETE(ow_subtract_scalar_from_matrix(second_mp, scalar));

#ifdef VERBOSE
                db_mat(second_mp);
#endif 

                diff_1 = max_abs_matrix_difference(second_mp, first_mp);


                if (diff_1 > 100.0 * DBL_EPSILON)
                {
                    p_stderr("%e != 0.0.\n", diff_1); 
                    p_stderr("Problem with test one\n");
                    status = EXIT_BUG;
                }

                EPETE(subtract_scalar_from_matrix(&second_mp, first_mp, scalar));

#ifdef VERBOSE
                db_mat(second_mp);
#endif 

                EPETE(ow_add_scalar_to_matrix(second_mp, scalar));

#ifdef VERBOSE
                db_mat(second_mp);
#endif 

                diff_2 = max_abs_matrix_difference(second_mp, first_mp);

                if (diff_2 > 100.0 * DBL_EPSILON)
                {
                    p_stderr("%e != 0.0.\n", diff_2); 
                    p_stderr("Problem with test two\n");
                    status = EXIT_BUG;
                }

                
                EPETE(multiply_matrix_by_scalar(&second_mp, first_mp, scalar));

#ifdef VERBOSE
                db_mat(second_mp);
#endif 

                EPETE(ow_divide_matrix_by_scalar(second_mp, scalar));

#ifdef VERBOSE
                db_mat(second_mp);
#endif 

                diff_3 = max_abs_matrix_difference(second_mp, first_mp);

                if (diff_3 > 100.0 * DBL_EPSILON)
                {
                    p_stderr("%e != 0.0.\n", diff_3); 
                    p_stderr("Problem with test three\n");
                    status = EXIT_BUG;
                }

                EPETE(divide_matrix_by_scalar(&second_mp, first_mp, scalar));

#ifdef VERBOSE
                db_mat(second_mp);
#endif 

                EPETE(ow_multiply_matrix_by_scalar(second_mp, scalar));

#ifdef VERBOSE
                db_mat(second_mp);
#endif 

                diff_4 = max_abs_matrix_difference(second_mp, first_mp);

                if (diff_4 > 100.0 * DBL_EPSILON)
                {
                    p_stderr("%e != 0.0.\n", diff_4); 
                    p_stderr("Problem with test four\n");
                    status = EXIT_BUG;
                }
 
                EPETE(invert_matrix_elements(&second_mp, first_mp));

#ifdef VERBOSE
                db_mat(second_mp);
#endif 

                EPETE(ow_invert_matrix_elements(second_mp));

#ifdef VERBOSE
                db_mat(second_mp);
#endif 

                EPETE(subtract_matrices(&diff_mp, second_mp, first_mp));

                EPETE(ow_divide_matrices_ew(diff_mp, first_mp)); 

                diff_5 = max_abs_matrix_element(diff_mp);

                if (diff_5 > 100.0 * DBL_EPSILON)
                {
                    p_stderr("%e != 0.0.\n", diff_5); 
                    p_stderr("Problem with test five\n");
                    status = EXIT_BUG;
                }
            }
        }
    }
    
#ifdef VERBOSE
    for (count=0; count<num_tries; count++)
    {
        dbi(respect_missing_values());
        restore_respect_missing_values();
    }
#endif 

    free_matrix(first_mp); 
    free_matrix(second_mp); 
    free_matrix(diff_mp); 

    return status; 
}

