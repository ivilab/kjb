
/* $Id: complex_mat_mult.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"
#include "x/x_incl.h"


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int     num_rows, num_cols;
    int     count;
    Matrix* target_re_mp = NULL;
    Matrix* target_im_mp = NULL;
    int     status       = EXIT_SUCCESS;
    Matrix* first_re_mp  = NULL;
    Matrix* first_im_mp  = NULL;
    Matrix* second_re_mp = NULL;
    Matrix* second_im_mp = NULL;
    int     max_count    = 1000;
    int     test_factor  = 1;
    int     num_loops_for_mult_test = 10;

    kjb_init(); 

    kjb_set_debug_level(2); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
        max_count *= test_factor; 
    }

    if (is_interactive())
    {
        kjb_set_verbose_level(2);
    }
    else 
    {
        kjb_set_verbose_level(0); 
        num_loops_for_mult_test = 50;
    }

    for (count=0; count<test_factor; count++)
    {
        for (num_rows=1; num_rows<num_loops_for_mult_test; num_rows++)
        {
            for (num_cols=1; num_cols<num_loops_for_mult_test; num_cols++)
            {

                EPETE(get_random_matrix(&first_re_mp, num_rows, num_cols)); 
                EPETE(get_random_matrix(&first_im_mp, num_rows, num_cols)); 
                EPETE(get_random_matrix(&second_re_mp, num_cols + 1, num_rows));
                EPETE(get_random_matrix(&second_im_mp, num_cols + 1, num_rows));

                verbose_pso(1, "\n ------------------------- \n\n %d %d %d \n\n",
                            count, num_rows, num_cols);

                if (complex_multiply_matrices(&target_re_mp, &target_im_mp, first_re_mp, first_im_mp, second_re_mp, second_im_mp) != ERROR)
                {
                    set_bug("Dimension check passed when it shouldn't.\n");
                    return EXIT_BUG;
                }
                else if (kjb_get_verbose_level() > 0)
                {
                    kjb_print_error(); 
                }

                EPETE(get_random_matrix(&second_re_mp, num_cols, num_rows));
                EPETE(get_random_matrix(&second_im_mp, num_cols, num_rows));
                
                if (kjb_rand() < 0.5)
                {
                    if (kjb_rand() < 0.66)
                    {
                        free_matrix(first_re_mp);
                        first_re_mp = NULL; 
                    }
                    else if (kjb_rand() < 0.5)
                    {
                        free_matrix(first_im_mp);
                        first_im_mp = NULL; 
                    }

                    if (kjb_rand() < 0.66)
                    {
                        free_matrix(second_re_mp);
                        second_re_mp = NULL; 
                    }
                    else if (kjb_rand() < 0.5)
                    {
                        free_matrix(second_im_mp);
                        second_im_mp = NULL; 
                    }
                }

                verbose_pso(1, "\n ------------------------- \n\n %d %d %d \n\n",
                    count, num_rows, num_cols);

                if (is_interactive())
                {
                    db_mat(first_re_mp);
                    db_mat(first_im_mp);
                    db_mat(second_re_mp);
                    db_mat(second_im_mp);
                }

                EPETE(complex_multiply_matrices(&target_re_mp, &target_im_mp,
                                                first_re_mp, first_im_mp,
                                                second_re_mp, second_im_mp));

                if (is_interactive())
                {
                    db_mat(target_re_mp);
                    db_mat(target_im_mp);
                }

            }
        }
    }

    free_matrix(first_re_mp);
    free_matrix(first_im_mp);
    free_matrix(second_re_mp);
    free_matrix(second_im_mp);
    free_matrix(target_re_mp);
    free_matrix(target_im_mp);

    return status; 
}

