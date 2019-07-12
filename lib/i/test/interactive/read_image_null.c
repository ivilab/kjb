
/* $Id: read_image_null.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "i/i_incl.h"

/*
 * Program to verify a sensible error mesasure if we try to open a file with a
 * null name. 
*/

int main(int argc, char** argv)
{
    KJB_image* ip = NULL;

    kjb_init();   /* Best to do this if using KJB library. */

    /*
     * This does not do anything in batch mode. 
    */
    if (! is_interactive()) 
    {
        return EXIT_SUCCESS;
    }
    

    EPETE(kjb_read_image_2(&ip, ""));


    kjb_cleanup(); /* Almost never needed, but doing it twice is OK. */

    return EXIT_SUCCESS; 
} 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

