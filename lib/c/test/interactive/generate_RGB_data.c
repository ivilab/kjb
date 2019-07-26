
/* $Id: generate_RGB_data.c 21491 2017-07-20 13:19:02Z kobus $ */



/* =========================================================================== *
|                                                                              |
|  Copyright (c) 1994-1998, by Kobus Barnard (author).                         |
|                                                                              |
|  For use outside the SFU vision lab please contact the author(s).            |
|                                                                              |
* =========================================================================== */


#include "c/c_incl.h"


/* -------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    char     buff [ MAX_FILE_NAME_SIZE ];
    int      option;
    Spectra* reflect_sp                  = NULL;
    Spectra* illum_sp                    = NULL;
    int      result                      = NO_ERROR;
    Matrix*  rgb_mp                      = NULL;
    Matrix*  all_rgb_mp                  = NULL;
    Vector*  illum_vp                    = NULL;
    Vector*  reflect_vp                  = NULL;
    Vector*  product_vp                  = NULL;
    double   R, G, B;
    int      i, j;


    /* * Not ready to do anything in batch mode.  */
    if (! is_interactive()) 
    {
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    while (    (result == NO_ERROR)
            &&
               ((option = kjb_getopts(argc, argv, "-r:i:", NULL, buff, 
                                      sizeof(buff))) 
               != EOF)
          )
    {
        switch (option)
        {
            case ERROR :
                p_stderr("Aborting program due to argument error.\n");
                return EXIT_FAILURE;
            case 'r' :
                EPETE(read_reflectance_spectra(&reflect_sp, buff));
                break;
            case 'i' :
                EPETE(read_illuminant_spectra(&illum_sp, buff));
                break;
            default :
                break; 
        }
    }

    if (illum_sp == NULL)
    {
        p_stderr("No illum spectra file given on the command line.\n");
        return EXIT_FAILURE;
    }

    if (reflect_sp == NULL)
    {
        p_stderr("No reflectance spectra file given on the command line.\n");
        return EXIT_FAILURE;
    }

    EPETE(generate_RGB_data(&all_rgb_mp, illum_sp, USE_ALL_SPECTRA, 
                            reflect_sp, (Spectra*)NULL));

    for (i=0; i<illum_sp->spectra_mp->num_rows; i++)
    {
        EPETE(generate_RGB_data(&rgb_mp, illum_sp, i, reflect_sp, 
                                (Spectra*)NULL));
        EPETE(get_matrix_row(&illum_vp, illum_sp->spectra_mp, i));

        for (j=0; j<reflect_sp->spectra_mp->num_rows; j++)
        {
            int all_i = i * reflect_sp->spectra_mp->num_rows + j;

            EPETE(get_matrix_row(&reflect_vp, reflect_sp->spectra_mp, j)); 
            EPETE(multiply_vectors(&product_vp, reflect_vp, illum_vp));
            EPETE(get_RGB_from_spectrum(product_vp, illum_sp->offset, 
                                        illum_sp->step, &R, &G, &B));

            if (    (fabs((double)(rgb_mp->elements[ j ][ 0 ] - R)) > .000001)
                 || (fabs((double)(rgb_mp->elements[ j ][ 1 ] - G)) > .000001)
                 || (fabs((double)(rgb_mp->elements[ j ][ 2 ] - B)) > .000001)
               )
            {
                p_stderr("Difference found.\n");
                kjb_exit(EXIT_BUG);
            }


            if (    (fabs((double)(all_rgb_mp->elements[ all_i ][ 0 ] - R)) > .000001)
                 || (fabs((double)(all_rgb_mp->elements[ all_i ][ 1 ] - G)) > .000001)
                 || (fabs((double)(all_rgb_mp->elements[ all_i ][ 2 ] - B)) > .000001)
               )
            {
                p_stderr("Difference found (all).\n");
                kjb_exit(EXIT_BUG);
            }

        }
    }

    free_matrix(rgb_mp);
    free_matrix(all_rgb_mp);
    free_vector(illum_vp);
    free_vector(reflect_vp);
    free_vector(product_vp);
    free_spectra(illum_sp);
    free_spectra(reflect_sp);

    return EXIT_SUCCESS; 
}


