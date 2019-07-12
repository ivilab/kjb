
/* $Id: convolve.c 21491 2017-07-20 13:19:02Z kobus $ */


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

/*
//  Most programs need at least the "m" library. 
*/

#include "i/i_incl.h" 


static int old_gauss_convolve_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    double           sigma 
);

/* -------------------------------------------------------------------------- */

int main(int argc, char** argv)
{
    KJB_image* ip = NULL;
    KJB_image* out_ip = NULL;

    kjb_init();   /* Best to do this if using KJB library. */

    if (! is_interactive()) 
    {
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    create_image_display();

    check_num_args(argc, 1, 1,  "image_draw_point [ file_name ]");
    EPETE(kjb_read_image(&ip, argv[ 1 ]));

    p_stderr("ONE\n"); 
    EPETE(old_gauss_convolve_image(&out_ip, ip, 5.0));
    p_stderr("ONE\n\n\n"); 
    EPETE(kjb_display_image(out_ip, NULL));

    p_stderr("TWO\n"); 
    EPETE(gauss_convolve_image(&out_ip, ip, 5.0));
    p_stderr("TWO\n"); 
    EPETE(kjb_display_image(out_ip, NULL));


    prompt_to_continue();

    kjb_free_image(ip);
    kjb_free_image(out_ip);


    kjb_cleanup(); /* Almost never needed, but doing it twice is OK. */

    return EXIT_SUCCESS; 
} 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int old_gauss_convolve_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    double           sigma 
)
{
    KJB_image* out_ip;
    int        num_rows, num_cols, i, j, mi, mj, m, n;
    int        mask_width;
    int        mask_size;
    Matrix*    mask_mp    = NULL;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols)); 
    out_ip = *out_ipp; 

    mask_width = (int)(1.0 + 2.0 * sigma); 
    mask_size  = 1 + 2 * mask_width; 

    ERE(get_2D_gaussian_mask(&mask_mp, mask_size, sigma)); 

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            double r_sum      = 0.0;
            double g_sum      = 0.0;
            double b_sum      = 0.0;
            double weight_sum = 0.0;
            double weight; 

            for (mi = 0; mi < mask_size; mi++)
            {
                for (mj = 0; mj < mask_size; mj++)
                {
                    m = i + mi - mask_width;
                    n = j + mj - mask_width;

                    if (   (m >= 0) && (m < num_rows)
                        && (n >= 0) && (n < num_cols))
                    {
                        weight = mask_mp->elements[mi][mj];

                        r_sum += in_ip->pixels[m][n].r * weight; 
                        g_sum += in_ip->pixels[m][n].g * weight; 
                        b_sum += in_ip->pixels[m][n].b * weight; 

#ifdef OBSOLETE_NORMALIZE_CONVOLUTIONS 
                        weight_sum += weight;
#endif 
                    }
                }
            }
#ifdef OBSOLETE_NORMALIZE_CONVOLUTIONS 
            r_sum /= weight_sum; 
            g_sum /= weight_sum; 
            b_sum /= weight_sum; 
#endif 
            out_ip->pixels[i][j].r = r_sum; 
            out_ip->pixels[i][j].g = g_sum; 
            out_ip->pixels[i][j].b = b_sum; 

            out_ip->pixels[i][j].extra.invalid = in_ip->pixels[i][j].extra.invalid; 
        }
    }

    free_matrix(mask_mp);  

    return NO_ERROR;
}

