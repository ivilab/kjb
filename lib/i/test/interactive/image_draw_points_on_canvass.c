
/* $Id: image_draw_point.c 4723 2009-11-16 18:57:09Z kobus $ */


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
    int read_res, i, j;
    char line_in[ 200 ];
    float r, g, b;

    kjb_init();   /* Best to do this if using KJB library. */

    if (! is_interactive()) 
    {
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }

    EPETE(get_initialized_image_2(&ip, 600, 800, 255, 255, 255));
     
    /*
    EPETE(kjb_display_image(ip, NULL));
    */
    /*
    EPETE(create_image_display());
    */
    
    while (TRUE)
    {
        EPETE(read_res = BUFF_STDIN_GET_LINE("i: ", line_in));
        if (read_res == EOF) break; 
        EPETE(ss1i(line_in, &i));

        EPETE(read_res = BUFF_STDIN_GET_LINE("j: ", line_in));
        if (read_res == EOF) break; 
        EPETE(ss1i(line_in, &j));

        r = 255.0 * kjb_rand();
        g = 255.0 * kjb_rand();
        b = 255.0 * kjb_rand();

        EPETE(image_draw_point_2(ip, i,j, 11, r,g,b));

        EPETE(kjb_display_image(ip, NULL));
    }

    kjb_free_image(ip);
    
    return EXIT_SUCCESS; 
} 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

