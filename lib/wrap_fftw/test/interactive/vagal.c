
/* $Id: vagal.c 21491 2017-07-20 13:19:02Z kobus $ */


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
 * An ill fated attempt to understand the analysis of RSA by looking at the
 * spectrum of signals whose period (e.g. heart rate) is as modulated by another
 * (breathing). As it turns out, the data that they do the Fourier transform of
 * is the period of the heart rate, which is more direct and makes more sense.
*/

#include "i/i_incl.h" 
#include "wrap_fftw/wrap_fftw.h" 
#include "x/x_incl.h" 

/* -------------------------------------------------------------------------- */

#define RESOLUTION             16
#define HEART_PER_BREATH       5
#define BREATHS                2

int main(int argc, char** argv)
{
    int length = RESOLUTION * HEART_PER_BREATH * BREATHS;
    Vector* vp = NULL;
    Vector* im_dft_vp = NULL;
    Vector* re_dft_vp = NULL;
    Vector* mag_dft_vp = NULL;
    int i, j, k; 
    int count = 0;
    int plot_id; 
    int fft_plot_id; 
    double val;
    double mean; 
    double factor;
    double stretch;


    kjb_init();   /* Best to do this if using KJB library. */

    /*
     * Not ready to do anything in batch mode. 
    */
    if (! is_interactive())
    {
        return EXIT_SUCCESS;
    }

    plot_id = plot_open();
    fft_plot_id = plot_open();

    EPETE(get_zero_vector(&vp, length));

    for (stretch = 0.0; stretch < 0.25; stretch += 0.1)
    {
        for (count = 0; count < length; count++) 
        {
            int beat_offset = count % (RESOLUTION * HEART_PER_BREATH);

            factor = 1.0 + stretch * sin(2.0 * M_PI * beat_offset / (RESOLUTION * HEART_PER_BREATH));

            val = sin(2.0 * M_PI* factor * beat_offset / (RESOLUTION)); 
            vp->elements[ count ] = val; 
        }

        mean = average_vector_elements(vp);
        ow_subtract_scalar_from_vector(vp, mean); 

        plot_vector(plot_id, vp, 0, 1, NULL);

        EPETE(get_vector_dft(&re_dft_vp, &im_dft_vp, vp, NULL)); 

        EPETE(complex_get_vector_element_magnitudes(&mag_dft_vp, 
                                                    re_dft_vp,
                                                    im_dft_vp));
        EPETE(get_vector_dft(&re_dft_vp, &im_dft_vp, vp, NULL)); 

        EPETE(complex_get_vector_element_magnitudes(&mag_dft_vp, 
                                                    re_dft_vp,
                                                    im_dft_vp));
        mag_dft_vp->length = length / 2;
        plot_vector(fft_plot_id, mag_dft_vp, 0, 1, NULL);
        mag_dft_vp->length = length;
    }


    prompt_to_continue();
    
    free_vector(vp);
    free_vector(im_dft_vp);
    free_vector(re_dft_vp);
    free_vector(mag_dft_vp);

    kjb_cleanup(); /* Almost never needed, but doing it twice is OK. */

    return EXIT_SUCCESS; 
} 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

