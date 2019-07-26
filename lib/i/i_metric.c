
/* $Id: i_metric.c 8780 2011-02-27 23:42:02Z predoehl $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
* =========================================================================== */

#include "i/i_gen.h"     /* Only safe as first include in a ".c" file. */
#include "i/i_metric.h"


#ifdef __cplusplus
extern "C" {
#endif


/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                          compute_rms_image_difference
 *
 * Computes the RMS difference between two images
 *
 * This routine computes the RMS difference between two images. Only valid
 * pixel pairs are used. The result is returned in *rms_diff_ptr.
 *
 * Returns :
 *    NO_ERROR on success and ERROR on failure, with an error message being set.
 *    The routine will fail if the two input images are different dimensions, or
 *    there is no valid pixel pair.
 *
 * Index: images, image processing, image difference
 *
 * -----------------------------------------------------------------------------
*/

int compute_rms_image_difference
(
    const KJB_image* in1_ip,
    const KJB_image* in2_ip,
    double*          rms_diff_ptr
)
{
    Pixel*      in1_pos;
    Pixel*      in2_pos;
    int         i, j, num_rows, num_cols;
    long_double sum      = 0.0;
    int         count    = 0;


    ERE(check_same_size_image(in1_ip, in2_ip));

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in1_pos = in1_ip->pixels[ i ];
        in2_pos = in2_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            if ( ! (in1_pos->extra.invalid.pixel || in2_pos->extra.invalid.pixel))
            {
                double r_diff = in1_pos->r - in2_pos->r;
                double g_diff = in1_pos->g - in2_pos->g;
                double b_diff = in1_pos->b - in2_pos->b;
                double temp = r_diff * r_diff + g_diff * g_diff + b_diff * b_diff;

                sum += temp;
                count++;
            }

            in1_pos++;
            in2_pos++;
        }
    }

    if (count == 0)
    {
        set_error("No valid pixels available to compute RMS difference.");
        return ERROR;
    }
    else
    {
        *rms_diff_ptr = sqrt((double)(sum / (long_double)count));
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          compute_image_difference
 *
 * Computes the pixel-wise difference between two images
 *
 * This routine computes the pixel-difference between two images. The resulting
 * difference image is pointed to by diff_ip
 *
 * Returns :
 *    NO_ERROR on success and ERROR on failure. The routine will fail if the 
 *    two input images are different dimensions there is no valid pixel pair.
 *
 * Index: images, image processing, image difference
 *
 * -----------------------------------------------------------------------------
*/
int compute_image_difference
(
    const KJB_image* in1_ip,
    const KJB_image* in2_ip,
    KJB_image** diff_ipp
)
{

    int        i, j, num_rows, num_cols;

    ERE(check_same_size_image(in1_ip, in2_ip));

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;



    /*Allocated new image and set ptr to it*/


    *diff_ipp = NULL;
    if(get_target_image(diff_ipp, num_rows, num_cols) == ERROR)
    {
    set_error("Difference image could not be created.\n");
    return ERROR;
    }


    for(i=0; i < num_rows; i++)
    {
    for(j=0; j < num_cols; j++)
    {
        (*diff_ipp)->pixels[i][j].r = in2_ip->pixels[i][j].r - in1_ip->pixels[i][j].r;
        (*diff_ipp)->pixels[i][j].g = in2_ip->pixels[i][j].g - in1_ip->pixels[i][j].g;
        (*diff_ipp)->pixels[i][j].b = in2_ip->pixels[i][j].b - in1_ip->pixels[i][j].b;
    }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


#ifdef __cplusplus
}
#endif

