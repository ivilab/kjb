
/* $Id: mat_arith_mult.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"


#define NUM_LOOPS_FOR_MULT_TEST   30
#define BASE_NUM_TRIES            10


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int num_rows;
    int num_cols; 
    int count;
    Matrix* new_target_mp = NULL; 
    Matrix* new_target_two_mp = NULL; 
    Matrix* new_target_three_mp = NULL; 
    Matrix* target_mp = NULL; 
    Vector* col_vp = NULL;
    Vector* row_vp = NULL; 
    Vector* new_col_vp = NULL;
    Vector* new_row_vp = NULL; 
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;
    Matrix* first_mp = NULL;
    Matrix* second_mp = NULL;
    Matrix* transpose_first_mp = NULL;
    Matrix* transpose_second_mp = NULL;


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
        for (num_rows=1; num_rows<NUM_LOOPS_FOR_MULT_TEST; num_rows++)
        {
            for (num_cols=1; num_cols<NUM_LOOPS_FOR_MULT_TEST; num_cols++)
            {
                double  diff_1;
                double  diff_2;
                double  diff_3;
                double  diff_4;
                double  diff_5;
                double  diff_6;
                int     i; 


                EPETE(get_random_matrix(&first_mp, num_rows, num_cols)); 
                EPETE(get_random_matrix(&second_mp, num_cols + 1, num_rows));

#ifdef VERBOSE
                pso("\n ------------------------- \n\n %d %d %d \n\n",
                    count, num_rows, num_cols);
                db_mat(first_mp);
                db_mat(second_mp);
#endif 

                if (multiply_matrices(&target_mp, first_mp, second_mp) != ERROR)
                {
                    p_stderr("Dimension check passed when it shouldn't.\n\n");
                    status = EXIT_BUG;
                }
#ifdef VERBOSE
                else 
                {
                    kjb_print_error(); 
                }
#endif 

                EPETE(get_random_matrix(&second_mp, num_cols, num_rows)); 
                EPETE(multiply_matrices(&target_mp, first_mp, second_mp));

#ifdef VERBOSE
                db_mat(first_mp);
                db_mat(second_mp);
                db_mat(target_mp);
#endif 

                EPETE(get_target_matrix(&new_target_mp, num_rows, num_rows)); 

                for (i=0; i<num_rows; i++)
                {
                    EPETE(get_matrix_row(&row_vp, first_mp, i));

                    if (count % 2 == 0)
                    {
                        /* 
                         * Test that it is OK to send multiplication routines
                         * the same vector for input and output. 
                        */
                        EPETE(multiply_vector_and_matrix(&row_vp, row_vp, 
                                                         second_mp));
                        EPETE(put_matrix_row(new_target_mp, row_vp, i));
                    }
                    else
                    {
                        EPETE(multiply_vector_and_matrix(&new_row_vp, row_vp, 
                                                         second_mp));
                        EPETE(put_matrix_row(new_target_mp, new_row_vp, i));
                    }
                }

#ifdef VERBOSE
                db_mat(new_target_mp);
#endif 

                diff_1 = max_abs_matrix_difference(new_target_mp, target_mp);
                diff_1 /= (num_rows + num_cols); 



                EPETE(get_target_matrix(&new_target_mp, num_rows, num_rows)); 

                for (i=0; i<num_rows; i++)
                {
                    EPETE(get_matrix_col(&col_vp, second_mp, i));

                    if (count % 2 == 0)
                    {
                        /* 
                         * Test that it is OK to send multiplication routines
                         * the same vector for input and output. 
                        */
                        EPETE(multiply_matrix_and_vector(&col_vp, first_mp, 
                                                         col_vp));
                        EPETE(put_matrix_col(new_target_mp, col_vp, i));
                    }
                    else
                    {
                        EPETE(multiply_matrix_and_vector(&new_col_vp, 
                                                         first_mp, col_vp));
                        EPETE(put_matrix_col(new_target_mp, new_col_vp, i));
                    }

                }

#ifdef VERBOSE
                db_mat(new_target_mp);
#endif 

                diff_2 = max_abs_matrix_difference(new_target_mp, target_mp);
                diff_2 /= (num_rows + num_cols); 


                EPETE(get_transpose(&transpose_second_mp, second_mp));

                EPETE(multiply_by_transpose(&new_target_two_mp, first_mp, 
                                            transpose_second_mp));

                diff_3 = max_abs_matrix_difference(new_target_mp, 
                                                   new_target_two_mp);

                diff_3 /= (num_rows + num_cols); 

                EPETE(get_transpose(&transpose_first_mp, first_mp));

                EPETE(multiply_with_transpose(&new_target_three_mp, 
                                              transpose_first_mp, second_mp));

                diff_4 = max_abs_matrix_difference(new_target_mp, 
                                                   new_target_three_mp);
                diff_4 /= (num_rows + num_cols); 


                if (count % 2 == 0)
                {
                    EPETE(multiply_matrices(&first_mp, first_mp, second_mp));
                    diff_5 = max_abs_matrix_difference(new_target_mp, first_mp);
                    diff_5 /= (num_rows + num_cols); 

                    /* 
                     * Test that it is OK to send multiplication routines
                     * the same vector for input and output. 
                    */
                    EPETE(multiply_with_transpose(&second_mp, transpose_first_mp, second_mp));
                    diff_6 = max_abs_matrix_difference(new_target_three_mp, second_mp);
                    diff_6 /= (num_rows + num_cols); 
                }
                else
                {
                    EPETE(multiply_matrices(&second_mp, first_mp, second_mp));
                    diff_5 = max_abs_matrix_difference(new_target_mp, second_mp);
                    diff_5 /= (num_rows + num_cols); 

                    /* 
                     * Test that it is OK to send multiplication routines
                     * the same vector for input and output. 
                    */
                    EPETE(multiply_by_transpose(&first_mp, first_mp , transpose_second_mp));
                    diff_6 = max_abs_matrix_difference(new_target_three_mp, first_mp);
                    diff_6 /= (num_rows + num_cols); 
                }


                if ((ABS_OF(diff_1)) > 10.0 * DBL_EPSILON)
                {
                    p_stderr("Problem with test 1.\n");
                    p_stderr("(%d %d)  %e != 0.0.\n", num_rows, num_cols, 
                             diff_1); 
                    status = EXIT_BUG;
                }

                if ((ABS_OF(diff_2) ) > 10.0 * DBL_EPSILON)
                {
                    p_stderr("Problem test 2.\n");
                    p_stderr("(%d %d)  %e != 0.0.\n", num_rows, num_cols, 
                             diff_2); 
                    status = EXIT_BUG;
                }

                if ((ABS_OF(diff_3) ) > 10.0 * DBL_EPSILON)
                {
                    p_stderr("Problem test 3.\n");
                    p_stderr("(%d %d)  %e != 0.0.\n", num_rows, num_cols, 
                             diff_3); 
                    status = EXIT_BUG;
                }

                if ((ABS_OF(diff_4) ) > 10.0 * DBL_EPSILON)
                {
                    p_stderr("Problem with test 4.\n");
                    p_stderr("(%d %d)  %e != 0.0.\n", num_rows, num_cols, 
                             diff_4); 
                    status = EXIT_BUG;
                }

                if ((ABS_OF(diff_5) ) > 10.0 * DBL_EPSILON)
                {
                    p_stderr("Problem with test 5.\n");
                    p_stderr("(%d %d)  %e != 0.0.\n", num_rows, num_cols, 
                             diff_5); 
                    status = EXIT_BUG;
                }

                if ((ABS_OF(diff_6) ) > 10.0 * DBL_EPSILON)
                {
                    p_stderr("Problem with test 6.\n");
                    p_stderr("(%d %d)  %e != 0.0.\n", num_rows, num_cols, 
                             diff_6); 
                    status = EXIT_BUG;
                }

            }
        }
    }

    free_matrix(first_mp);
    free_matrix(second_mp);
    free_matrix(transpose_first_mp);
    free_matrix(transpose_second_mp);
    free_matrix(target_mp);
    free_matrix(new_target_mp);
    free_matrix(new_target_two_mp);
    free_matrix(new_target_three_mp);
    free_vector(col_vp); 
    free_vector(new_col_vp); 
    free_vector(new_row_vp); 
    free_vector(row_vp); 

    return status; 
}

