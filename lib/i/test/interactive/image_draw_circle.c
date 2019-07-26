
/* $Id$ */


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

    kjb_init();   /* Best to do this if using KJB library. */

    if (! is_interactive()) 
    {
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    EPE(create_image_display());

    check_num_args(argc, 1, 1,  "image_draw_point [ file_name ]");
    EPETE(kjb_read_image(&ip, argv[ 1 ]));

    EPETE(image_draw_circle(ip, 100, 40, 70, 1, 200, 0, 0));

    EPETE(kjb_display_image(ip, NULL));

    prompt_to_continue();

    kjb_free_image(ip);
    
    return EXIT_SUCCESS; 
} 

