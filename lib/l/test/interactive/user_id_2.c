
/* $Id: user_id_2.c 4723 2009-11-16 18:57:09Z kobus $ */



#include "l/l_incl.h" 


/*ARGSUSED*/
int main(int argc, char **argv)
{
    char user_id[ 100 ];

    extern int kjb_debug_level;

    kjb_debug_level = 2; 

    EPETE(get_user_id(user_id, sizeof(user_id)));

    kjb_fputs(stdout,"User id is: ");
    toggle_high_light(stdout); 
    kjb_fputs(stdout,user_id);
    toggle_high_light(stdout); 
    kjb_fputs(stdout,"\n");

    return EXIT_SUCCESS; 
}
