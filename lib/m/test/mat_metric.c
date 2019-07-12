
/* $Id: mat_metric.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"


/*
#define VERBOSE 1
*/


#define NUM_LOOPS        30
#define BASE_NUM_TRIES   10


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int     num_rows, num_cols;
    int     count;
    Vector* rel_diff_vp      = NULL;
    Vector* test_diff_vp     = NULL;
    Matrix* ave_mp           = NULL;
    Matrix* diff_mp          = NULL;
    Matrix* test_rel_diff_mp = NULL;
    Matrix* prod_mp          = NULL;
    Matrix* div_mp           = NULL;
    Vector* sub_vp           = NULL;
    Vector* test_rel_diff_vp = NULL;
    Vector* row_norms_vp     = NULL;
    int     num_tries        = BASE_NUM_TRIES;
    int     test_factor      = 1;
    Matrix* first_mp         = NULL;
    Matrix* second_mp        = NULL;


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

    if (is_interactive())
    {
        kjb_set_verbose_level(2);
        kjb_set_debug_level(2);
    }
    else
    {
        kjb_set_verbose_level(0);
        kjb_set_debug_level(0);
    }

    for (count=0; count<num_tries; count++)
    {
        for (num_rows=1; num_rows<NUM_LOOPS; num_rows++)
        {
            for (num_cols=1; num_cols<NUM_LOOPS; num_cols++)
            {
                double diff;
                double diff_2;
                double test_diff;
                double test_diff_2;
                double temp;
                double test_1_result;
                double test_2_result;
                double test_3_result;
                double test_4_result;
                double test_5_result;
                double test_6_result;
                double test_overall_rel_error;
                double overall_rel_error;


                EPETE(get_random_matrix(&first_mp, num_rows, num_cols)); 
                EPETE(get_random_matrix(&second_mp, num_rows, num_cols)); 

                verbose_pso(1, "\n ------------------------- \n\n %d %d %d \n\n",
                           count, num_rows, num_cols);

                diff = max_abs_matrix_difference(first_mp, second_mp);

                EPETE(subtract_matrices(&diff_mp, first_mp, second_mp));
                test_diff = max_abs_matrix_element(diff_mp); 

                test_1_result = ABS_OF(diff - test_diff);
                verbose_pso(2, "Test 1: %.4e\n", test_1_result);

                if (test_1_result > DBL_EPSILON)
                {
                    p_stderr("Num_rows : %d\nNum_cols %d.\n", num_rows, 
                             num_cols);
                    p_stderr("%e > %e.\n", test_1_result, DBL_EPSILON); 
                    p_stderr("Problem with max_abs_matrix_difference!");
                    status = EXIT_BUG;
                }

                diff_2 = rms_matrix_difference(first_mp, second_mp);

                EPETE(multiply_matrices_ew(&prod_mp, diff_mp, diff_mp));

                temp = sum_matrix_elements(prod_mp); 
                temp /= num_rows * num_cols; 
                test_diff_2 = sqrt(temp); 
                
                test_2_result = ABS_OF(diff_2 - test_diff_2);
                test_2_result /= (10.0 * num_rows * num_cols); 
                verbose_pso(2, "Test 2: %.4e\n", test_2_result);

                if (test_2_result > DBL_EPSILON)
                {
                    p_stderr("Num_rows : %d\nNum_cols %d.\n", num_rows, 
                             num_cols);
                    p_stderr("%e > %e.\n", test_2_result, DBL_EPSILON); 
                    p_stderr("Problem  with rms_matrix_difference!\n");
                    status = EXIT_BUG;
                }

                EPETE(get_rms_relative_row_error(&rel_diff_vp,
                                                 first_mp, 
                                                 second_mp, 
                                                 &overall_rel_error));

                EPETE(add_matrices(&ave_mp, first_mp, second_mp));
                EPETE(ow_divide_matrix_by_scalar(ave_mp, 2.0));

                EPETE(divide_matrices_ew(&test_rel_diff_mp, diff_mp, ave_mp));

                ERE(get_matrix_row_norms(&test_rel_diff_vp, test_rel_diff_mp));
                ERE(ow_divide_vector_by_scalar(test_rel_diff_vp, sqrt((double)num_cols)));

                test_3_result = max_abs_vector_difference(rel_diff_vp, test_rel_diff_vp); 
                test_3_result /= (10.0 * num_cols); 
                verbose_pso(2, "Test 3: %.4e\n", test_3_result);

                if (test_3_result > DBL_EPSILON)
                {
                    p_stderr("Num_rows : %d\nNum_cols %d\n", num_rows, 
                             num_cols);
                    p_stderr("%e > %e\n", test_3_result, DBL_EPSILON); 
                    p_stderr("Problem with vector part of get_rms_relative_row_error!");
                    status = EXIT_BUG;
                }

                test_overall_rel_error = vector_magnitude(test_rel_diff_vp);
                test_overall_rel_error /= sqrt((double)test_rel_diff_vp->length); 

                test_4_result = ABS_OF(test_overall_rel_error - overall_rel_error);
                test_4_result /= (10.0 * num_rows * num_cols); 
                verbose_pso(2, "Test 4: %.4e\n", test_4_result);
                
                if (test_4_result > DBL_EPSILON)
                {
                    p_stderr("Num_rows : %d\nNum_cols %d\n", num_rows, 
                             num_cols);
                    p_stderr("%e < %e\n", test_4_result, DBL_EPSILON); 
                    p_stderr("Problem  with overall part of get_rms_relative_row_error!");
                    status = EXIT_BUG;
                }

                EPETE(get_rms_relative_col_error(&rel_diff_vp,
                                                 first_mp, 
                                                 second_mp, 
                                                 &overall_rel_error));

                EPETE(add_matrices(&ave_mp, first_mp, second_mp));
                EPETE(ow_divide_matrix_by_scalar(ave_mp, 2.0));

                EPETE(divide_matrices_ew(&test_rel_diff_mp, diff_mp, ave_mp));

                ERE(get_matrix_col_norms(&test_rel_diff_vp, test_rel_diff_mp));
                ERE(ow_divide_vector_by_scalar(test_rel_diff_vp, sqrt((double)num_rows)));

                test_5_result = max_abs_vector_difference(rel_diff_vp, test_rel_diff_vp); 
                test_5_result /= (10.0 * num_rows); 
                verbose_pso(2, "Test 5: %.4e\n", test_3_result);

                if (test_5_result > DBL_EPSILON)
                {
                    p_stderr("Num_rows : %d\nNum_cols %d\n", num_rows, 
                             num_cols);
                    p_stderr("%e > %e\n", test_5_result, DBL_EPSILON); 
                    p_stderr("Problem with vector part of get_rms_relative_col_error!");
                    status = EXIT_BUG;
                }

                test_overall_rel_error = vector_magnitude(test_rel_diff_vp);
                test_overall_rel_error /= sqrt((double)test_rel_diff_vp->length); 

                test_6_result = ABS_OF(test_overall_rel_error - overall_rel_error);
                test_6_result /= (10.0 * num_rows * num_cols); 
                verbose_pso(2, "Test 6: %.4e\n", test_6_result);
                
                if (test_6_result > DBL_EPSILON)
                {
                    p_stderr("Num_rows : %d\nNum_cols %d\n", num_rows, 
                             num_cols);
                    p_stderr("%e < %e\n", test_6_result, DBL_EPSILON); 
                    p_stderr("Problem  with overall part of get_rms_relative_row_error!");
                    status = EXIT_BUG;
                }

            }
        }
    }

    free_matrix(first_mp);
    free_matrix(second_mp);
    free_vector(rel_diff_vp); 
    free_matrix(ave_mp); 
    free_matrix(diff_mp);
    free_matrix(test_rel_diff_mp);
    free_matrix(div_mp);
    free_matrix(prod_mp);
    free_vector(test_diff_vp); 
    free_vector(sub_vp);
    free_vector(test_rel_diff_vp);
    free_vector(row_norms_vp);

    return status; 
}

