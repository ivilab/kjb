
/* $Id: high.c 4723 2009-11-16 18:57:09Z kobus $ */



#include "l/l_incl.h" 


/*ARGSUSED*/
main(int argc, char **argv)
{

    kjb_l_set("debug_level", "1"); 

    kjb_fputs(stdout,"not high-lighted.\n");
    kjb_fputs(stdout,"[1mhigh-lighted.[m\n");

    kjb_fputs(stdout,"not high-lighted.\n");
    toggle_high_light(stdout); 
    kjb_fputs(stdout,"high-lighted.\n");
    toggle_high_light(stdout); 

    kjb_fputs(stdout,"not high-lighted.\n");
    toggle_high_light(stdout); 
    kjb_fputs(stdout,"high-lighted.\n");
    toggle_high_light(stdout); 

    kjb_fputs(stdout,"not high-lighted.\n");
    toggle_high_light(stdout); 
    kjb_fputs(stdout,"high-lighted.\n");
    toggle_high_light(stdout); 

    return EXIT_SUCCESS; 
}
