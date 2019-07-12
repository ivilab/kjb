
/* $Id: a5-noise.c 21491 2017-07-20 13:19:02Z kobus $ */


/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003, by members of University of Arizona Computer Vision     |
|  group (the authors) including                                               ||
|        Kobus Barnard.                                                        |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */

#include "i/i_incl.h" 
#include "wrap_fftw/wrap_fftw.h" 
#include "x/x_incl.h" 

/* -------------------------------------------------------------------------- */


int main(int argc, char** argv)
{
    int num_rows, num_cols; 
    Matrix* mp = NULL;
    Matrix* noise_mp = NULL;
    Matrix* noise_freq_mp = NULL;
    Matrix* freq_mp = NULL;
    KJB_image* ip = NULL;
    KJB_image* noise_ip = NULL;
    KJB_image* noise_freq_ip = NULL;
    KJB_image* freq_ip = NULL;
    Matrix* re_mp = NULL;
    Matrix* im_mp = NULL;
    Matrix* input_with_noise_mp = NULL; 
    KJB_image* input_with_noise_ip = NULL; 
    Matrix* denoised_mp = NULL; 
    KJB_image* denoised_ip = NULL; 
    int i,j;
    int max_d;


    kjb_init();   /* Best to do this if using KJB library. */

    if (! is_interactive()) 
    {
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    create_image_display();  /* Best to do this before displaying images. */
        
    set_fftw_style(FFTW_MATLAB_STYLE);


    EPETE(get_random_matrix(&mp, 6, 6));
    EPETE(get_matrix_dft(&re_mp, &im_mp, mp, NULL));
    write_matrix(mp, NULL);
    pso("\n");
    write_matrix(re_mp, NULL); 
    pso("\n");
    write_matrix(im_mp, NULL); 
    pso("\n");
    pso("------------------------\n");
    pso("\n");
    EPETE(get_matrix_inverse_dft(&re_mp, &im_mp, mp, NULL));
    write_matrix(re_mp, NULL); 
    pso("\n");
    write_matrix(im_mp, NULL); 
    pso("\n");
    pso("\n");

    set_fftw_style(FFTW_NORMALIZE_STYLE);

    EPETE(kjb_read_image(&ip, "input.tiff"));
    EPETE(kjb_display_image(ip, "in"));
    EPETE(ow_make_black_and_white_image(ip));
    EPETE(bw_image_to_matrix(ip, &mp));

    EPETE(get_matrix_dft(&re_mp, &im_mp, mp, NULL));
    EPETE(complex_get_magnitude_matrix_ew(&freq_mp, re_mp, im_mp));
    EPETE(matrix_to_bw_image(freq_mp, &freq_ip));
    EPETE(kjb_display_image(freq_ip, "image freq"));

    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    EPETE(get_random_matrix(&noise_mp, num_rows, num_cols));
    EPETE(ow_multiply_matrix_by_scalar(noise_mp, 255.0));
    EPETE(matrix_to_bw_image(noise_mp, &noise_ip));
    EPETE(kjb_display_image(noise_ip, "noise"));
    EPETE(kjb_write_image(noise_ip, "noise.tiff")); 

    EPETE(get_matrix_dft(&re_mp, &im_mp, noise_mp, NULL));
    EPETE(complex_get_magnitude_matrix_ew(&noise_freq_mp, re_mp, im_mp));
    EPETE(matrix_to_bw_image(noise_freq_mp, &noise_freq_ip));

    EPETE(kjb_display_image(noise_freq_ip, "noise freq"));

    EPETE(ow_multiply_matrix_by_scalar(mp, 0.75));
    EPETE(ow_multiply_matrix_by_scalar(noise_mp, 0.25));
    EPETE(add_matrices(&input_with_noise_mp, mp, noise_mp));
    EPETE(matrix_to_bw_image(input_with_noise_mp, &input_with_noise_ip));

    EPETE(kjb_display_image(input_with_noise_ip, "input with noise"));

    EPETE(kjb_write_image(input_with_noise_ip, "input_with_noise.tiff"));


    /*
     * Pretend we are a new program, so read what we wrote, rather than access
     * the version in memory.
     */
    EPETE(kjb_read_image(&input_with_noise_ip, "input_with_noise.tiff"));
    EPETE(bw_image_to_matrix(input_with_noise_ip, &input_with_noise_mp)); 

    EPETE(get_matrix_dft(&re_mp, &im_mp, input_with_noise_mp, NULL));

    num_rows = input_with_noise_mp->num_rows;
    num_cols = input_with_noise_mp->num_cols;

    max_d = (num_rows*num_rows + num_cols*num_cols) / 4;

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            int d = (num_rows/2 - i)*(num_rows/2 - i) + (num_cols/2 - j)*(num_cols/2 - j);

            if (d < 0.8 * max_d) 
            {
                re_mp->elements[ i ][ j ] = 0.0;
                im_mp->elements[ i ][ j ] = 0.0;
            }

        }
    }

    EPETE(get_matrix_inverse_dft(&denoised_mp, NULL, re_mp, im_mp)); 
    EPETE(matrix_to_bw_image(denoised_mp, &denoised_ip));
    EPETE(kjb_display_image(denoised_ip, "without noise"));


    prompt_to_continue();

    free_matrix(noise_freq_mp);
    free_matrix(noise_mp);
    free_matrix(freq_mp);
    free_matrix(mp);
    free_matrix(re_mp);
    free_matrix(im_mp);
    free_matrix(input_with_noise_mp);
    free_matrix(denoised_mp);

    kjb_free_image(ip);
    kjb_free_image(freq_ip);
    kjb_free_image(noise_freq_ip);
    kjb_free_image(noise_ip);
    kjb_free_image(input_with_noise_ip);
    kjb_free_image(denoised_ip);

    kjb_cleanup(); /* Almost never needed, but doing it twice is OK. */

    return EXIT_SUCCESS; 
} 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

