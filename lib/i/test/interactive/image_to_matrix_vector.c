
/* $Id: image_to_matrix_vector.c 21491 2017-07-20 13:19:02Z kobus $ */


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

 
/* -------------------------------------------------------------------------- */

int main(int argc, char** argv)
{
    KJB_image* ip = NULL;
    KJB_image* out_ip = NULL;
    Matrix_vector* mvp = NULL;

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

    EPETE(image_to_matrix_vector(ip, &mvp));

    EPETE(matrix_vector_to_image(mvp, &out_ip));

    EPETE(kjb_display_image(out_ip, NULL));

    prompt_to_continue();

    kjb_free_image(ip);
    kjb_free_image(out_ip);
    free_matrix_vector(mvp);

    return EXIT_SUCCESS; 
} 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

