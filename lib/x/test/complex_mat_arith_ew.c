
/* $Id: complex_mat_arith_ew.c 21545 2017-07-23 21:57:31Z kobus $ */


#include "m/m_incl.h"
#include "x/x_incl.h"


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int num_rows = 50;
    int num_cols = 300; 
    Matrix* re1_mp = NULL; 
    Matrix* im1_mp = NULL; 
    Matrix* re2_mp = NULL; 
    Matrix* im2_mp = NULL; 
    Matrix* mult_re_mp = NULL;
    Matrix* mult_im_mp = NULL;
    Matrix* div_re_mp = NULL; 
    Matrix* div_im_mp = NULL; 
    Matrix* nr_mult_re_mp = NULL;
    Matrix* nr_mult_im_mp = NULL;
    Matrix* nr_div_re_mp = NULL; 
    Matrix* nr_div_im_mp = NULL; 
    int i,j, count;
    int max_count = 1000; 
    int test_factor = 1;
    Bool verification_failed = FALSE;


    kjb_init(); 

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
        kjb_set_verbose_level(2);  /* Also 2 for now */
    }

    for (count = 0; count < max_count; count++)
    {
        EPETE(get_random_matrix(&re1_mp, num_rows, num_cols));
        EPETE(get_random_matrix(&im1_mp, num_rows, num_cols));
        EPETE(get_random_matrix(&re2_mp, num_rows, num_cols));
        EPETE(get_random_matrix(&im2_mp, num_rows, num_cols));

        if (kjb_rand() < 0.7) 
        {
            if (kjb_rand() < 0.5)
            {
                free_matrix(re1_mp); re1_mp = NULL; 
            }
            else
            {
                free_matrix(im1_mp); im1_mp = NULL; 
            }
        }


        if (kjb_rand() < 0.7) 
        {
            if (kjb_rand() < 0.5)
            {
                free_matrix(re2_mp); re2_mp = NULL; 
            }
            else
            {
                free_matrix(im2_mp); im2_mp = NULL; 
            }
        }


        EPETE(get_target_matrix(&nr_mult_re_mp, num_rows, num_cols));
        EPETE(get_target_matrix(&nr_mult_im_mp, num_rows, num_cols));
        EPETE(get_target_matrix(&nr_div_re_mp, num_rows, num_cols));
        EPETE(get_target_matrix(&nr_div_im_mp, num_rows, num_cols));

        EPETE(complex_multiply_matrices_ew(&mult_re_mp, &mult_im_mp, 
                                           re1_mp, im1_mp, re2_mp, im2_mp));
            
        /*
        EPETE(write_matrix(mult_re_mp, NULL));
        pso("\n");
        EPETE(write_matrix(mult_im_mp, NULL));
        pso("\n");
        */

        for (i = 0; i < num_rows; i++)
        {
            for (j = 0; j < num_cols; j++)
            {
                KJB_complex a, b, c;

                a.r = (re1_mp == NULL) ? 0.0 : re1_mp->elements[ i ][ j ];
                a.i = (im1_mp == NULL) ? 0.0 : im1_mp->elements[ i ][ j ];
                b.r = (re2_mp == NULL) ? 0.0 : re2_mp->elements[ i ][ j ];
                b.i = (im2_mp == NULL) ? 0.0 : im2_mp->elements[ i ][ j ];

                c = multiply_complex(a, b); 

                nr_mult_re_mp->elements[ i ][ j ] = c.r;
                nr_mult_im_mp->elements[ i ][ j ] = c.i;

                /* pso("%.5f   %.5f\n", c.r, c.i); */
            }
        }

        /* pso("\n\n"); */
        
        EPETE(complex_divide_matrices_ew(&div_re_mp, &div_im_mp, 
                                         re1_mp, im1_mp, re2_mp, im2_mp));

        /*
        EPETE(write_matrix(div_re_mp, NULL));
        pso("\n");
        EPETE(write_matrix(div_im_mp, NULL));
        pso("\n");
        */

        for (i = 0; i < num_rows; i++)
        {
            for (j = 0; j < num_cols; j++)
            {
                KJB_complex a, b, c;

                a.r = (re1_mp == NULL) ? 0.0 : re1_mp->elements[ i ][ j ];
                a.i = (im1_mp == NULL) ? 0.0 : im1_mp->elements[ i ][ j ];
                b.r = (re2_mp == NULL) ? 0.0 : re2_mp->elements[ i ][ j ];
                b.i = (im2_mp == NULL) ? 0.0 : im2_mp->elements[ i ][ j ];

                c = divide_complex(a, b); 

                /* pso("%.5f   %.5f\n", c.r, c.i); */

                nr_div_re_mp->elements[ i ][ j ] = c.r;
                nr_div_im_mp->elements[ i ][ j ] = c.i;

            }
        }

        verify_matrix(mult_re_mp, &verification_failed);
        if (verification_failed) { kjb_exit(EXIT_BUG); }

        verify_matrix(nr_mult_re_mp, &verification_failed);
        if (verification_failed) { kjb_exit(EXIT_BUG); }

        verify_matrix(mult_im_mp, &verification_failed);
        if (verification_failed) { kjb_exit(EXIT_BUG); }

        verify_matrix(nr_mult_im_mp, &verification_failed);
        if (verification_failed) { kjb_exit(EXIT_BUG); }

        verify_matrix(div_re_mp, &verification_failed);
        if (verification_failed) { kjb_exit(EXIT_BUG); }

        verify_matrix(nr_div_re_mp, &verification_failed);
        if (verification_failed) { kjb_exit(EXIT_BUG); }

        verify_matrix(div_im_mp, &verification_failed);
        if (verification_failed) { kjb_exit(EXIT_BUG); }

        verify_matrix(nr_div_im_mp, &verification_failed);
        if (verification_failed) { kjb_exit(EXIT_BUG); }


        if (max_abs_matrix_difference(mult_re_mp, nr_mult_re_mp) > 100.0 * DBL_EPSILON)
        {
            dbe(max_abs_matrix_difference(mult_re_mp, nr_mult_re_mp));
            kjb_exit(EXIT_BUG); 
        }
        if (max_abs_matrix_difference(mult_im_mp, nr_mult_im_mp) > 100.0 * DBL_EPSILON)
        {
            dbe(max_abs_matrix_difference(mult_im_mp, nr_mult_im_mp));
            kjb_exit(EXIT_BUG); 
        }
        if (max_abs_matrix_difference(div_re_mp, nr_div_re_mp) > 100.0 * DBL_EPSILON)
        {
            dbe(max_abs_matrix_difference(div_re_mp, nr_div_re_mp));
            kjb_exit(EXIT_BUG); 
        }
        if (max_abs_matrix_difference(div_im_mp, nr_div_im_mp) > 100.0 * DBL_EPSILON)
        {
            dbe(max_abs_matrix_difference(div_im_mp, nr_div_im_mp));
            kjb_exit(EXIT_BUG); 
        }
    }

    free_matrix(re1_mp); 
    free_matrix(im1_mp);
    free_matrix(re2_mp);
    free_matrix(im2_mp);
    free_matrix(mult_re_mp);
    free_matrix(mult_im_mp);
    free_matrix(div_re_mp);
    free_matrix(div_im_mp);
    free_matrix(nr_mult_re_mp);
    free_matrix(nr_mult_im_mp);
    free_matrix(nr_div_re_mp);
    free_matrix(nr_div_im_mp);

    return EXIT_SUCCESS; 
}

