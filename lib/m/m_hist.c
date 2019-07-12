
/* $Id: m_hist.c 20654 2016-05-05 23:13:43Z kobus $ */

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

#include "m/m_gen.h"     /* Only safe as first include in a ".c" file. */
#include "m/m_convolve.h"
#include "m/m_hist.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


static int fs_normalize_histograms = TRUE;


/* -------------------------------------------------------------------------- */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int set_normalized_histograms(int normalize_histograms)
{

    fs_normalize_histograms = normalize_histograms;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           get_1D_hist
 *
 * Computes a 1D histogram, optionally soft-binning
 *
 * This routine computes a 1D histogram, optionally soft-binning.
 *
 * Index: histograms
 *
 * -----------------------------------------------------------------------------
*/

#define ONE_D_SOFT_BIN_RESOLUTION   (10)

int get_1D_hist
(
    Vector**      hist_vpp,
    const Vector* vp,
    double        min,
    double        max,
    int           num_bins,
    double        sigma
)
{
    double  d_num_bins = num_bins;
    double  range = max - min;
    int i, j, bin;
    double d_bin;
    const Vector* hist_vp;


    ERE(get_zero_vector(hist_vpp, num_bins));
    hist_vp = *hist_vpp;

    if (sigma <= 0.0)
    {
        for (i = 0; i < vp->length; i++)
        {
            d_bin = d_num_bins * (vp->elements[ i ] - min) / range;

            /*
             * Stuff outside the edges is omitted. This is on purpose.  Perhaps
             * this behviour should be controlled with an option.  Note that we
             * have to be careful because (int)(-0.1) = 0.0.
            */
            if ((d_bin >= 0.0) && (ADD_DBL_EPSILON(d_bin) < d_num_bins))
            {
                bin = (int)d_bin;

                if (bin >= num_bins)
                {
                    bin = num_bins - 1;
                }

                (hist_vp->elements[ bin ])++;
            }
        }
    }
    else
    {
        Vector* gauss_vp = NULL;
        /* Two times sigma because a sigma of one is the bin size, but we
         * measure from the center.
        */
        int half_mask_width = MAX_OF(1, (int)(3 * ONE_D_SOFT_BIN_RESOLUTION * 2.0 * sigma));


        ERE(get_1D_gaussian_mask(&gauss_vp,
                                 3 + 2 * half_mask_width,
                                 ONE_D_SOFT_BIN_RESOLUTION * sigma));

        for (i = 0; i < vp->length; i++)
        {
            for (j = -half_mask_width; j <= half_mask_width; j++)
            {
                d_bin = d_num_bins * ((vp->elements[ i ] - min) / range) + ((double)j / (double)ONE_D_SOFT_BIN_RESOLUTION);

                /*
                 * Stuff outside the edges is omitted. This is on purpose.
                 * Perhaps this behviour should be controlled with an option.
                 * Note that we have to be careful because (int)(-0.1) = 0.0.
                */
                if (d_bin >= 0.0)
                {
                    bin = (int)d_bin;

                    if ((bin >= 0 ) && (bin < num_bins))
                    {
                        (hist_vp->elements[ bin ]) += gauss_vp->elements[ half_mask_width + j ];
                    }
                }
            }
        }

        free_vector(gauss_vp);
    }

    if (fs_normalize_histograms)
    {
        ERE(ow_scale_vector_by_sum(*hist_vpp));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

