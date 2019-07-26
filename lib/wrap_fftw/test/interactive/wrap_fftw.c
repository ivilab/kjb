
/* $Id: wrap_fftw.c 21491 2017-07-20 13:19:02Z kobus $ */


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

/* -------------------------------------------------------------------------- */

int main(int argc, char** argv)
{
    int num_rows, num_cols; 
    Matrix* mp = NULL;
    Matrix* r_mp = NULL;
    Matrix* g_mp = NULL;
    Matrix* b_mp = NULL;
    Matrix* out_mp = NULL;
    Matrix* out_out_mp = NULL;
    KJB_image* ip = NULL;
    KJB_image* out_ip = NULL;
    Matrix* image_dct_mp = NULL;
    Matrix* mask_dct_mp = NULL;
    Matrix* prod_dct_mp = NULL;
    Matrix* inv_prod_dct_mp = NULL;
    Matrix* mask_mp = NULL;
    int i, j; 

    kjb_init();   /* Best to do this if using KJB library. */
        
    if (! is_interactive()) 
    {
        p_stderr("This test program either needs to be adjusted for batch testing or moved.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }

    EPETE(get_random_matrix(&mp, 7, 7));

    EPETE(get_matrix_dct(&out_mp, mp));

    EPETE(get_matrix_inverse_dct(&out_out_mp, out_mp));

    write_matrix(mp, NULL);
    pso("\n");
    write_matrix(out_mp, NULL);
    pso("\n");
    write_matrix(out_out_mp, NULL);

    EPETE(kjb_read_image(&ip, "input.tiff"));
    num_rows = ip->num_rows; 
    num_cols = ip->num_cols; 

    EPETE(ow_make_black_and_white_image(ip));

    EPETE(image_to_rgb_matrices(ip, &r_mp, &g_mp, &b_mp));

    EPETE(get_zero_matrix(&mask_mp, num_rows, num_cols));

    /*
    for (i = 0; i < 20; i++)
    {
        for (j = 0; j < 20; j++)
        {
            mask_mp->elements[ i ][ j ] = 1;
        }
    }
    */
    mask_mp->elements[ 1 ][ 0 ] = 20;
    mask_mp->elements[ 1 ][ 1 ] = 20;
    mask_mp->elements[ 1 ][ 2 ] = 20;
    mask_mp->elements[ 1 ][ 3 ] = 20;
    mask_mp->elements[ 1 ][ 4 ] = 20;
    mask_mp->elements[ 1 ][ 5 ] = 20;
    mask_mp->elements[ 1 ][ 6 ] = 20;
    mask_mp->elements[ 1 ][ 7 ] = 20;
    mask_mp->elements[ 1 ][ 8 ] = 20;
    mask_mp->elements[ 1 ][ 9 ] = 20;

    mask_mp->elements[ 1 ][ 10 ] = 20;
    mask_mp->elements[ 1 ][ 11 ] = 20;
    mask_mp->elements[ 1 ][ 12 ] = 20;
    mask_mp->elements[ 1 ][ 13 ] = 20;
    mask_mp->elements[ 1 ][ 14 ] = 20;
    mask_mp->elements[ 1 ][ 15 ] = 20;
    mask_mp->elements[ 1 ][ 16 ] = 20;
    mask_mp->elements[ 1 ][ 17 ] = 20;
    mask_mp->elements[ 1 ][ 18 ] = 20;
    mask_mp->elements[ 1 ][ 19 ] = 20;


    EPETE(get_matrix_dct(&image_dct_mp, r_mp));
    EPETE(get_matrix_dct(&mask_dct_mp, mask_mp));

    EPETE(multiply_matrices_ew(&prod_dct_mp, image_dct_mp, mask_dct_mp)); 

    EPETE(get_matrix_inverse_dct(&inv_prod_dct_mp,prod_dct_mp)); 

    EPETE(rgb_matrices_to_image(inv_prod_dct_mp, inv_prod_dct_mp, inv_prod_dct_mp, 
                                &out_ip));

    EPETE(kjb_display_image(ip, NULL));

    EPETE(kjb_display_image(out_ip, NULL));

    prompt_to_continue();
    
    kjb_cleanup(); /* Almost never needed, but doing it twice is OK. */

    return EXIT_SUCCESS; 
} 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

