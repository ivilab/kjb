
/* $Id: kjb_sprintf.c 20919 2016-10-31 22:09:08Z kobus $ */



#include "l/l_incl.h" 

int main(void)
{
    IMPORT int kjb_debug_level; 
    char buff[ 1000 ]; 


    /*
    kjb_debug_level = 1;
    */

    EPETE(kjb_sprintf(buff, sizeof(buff), "%%A"));
    dbs(buff);
    put_line(buff);
    pso("%%A\n");

    EPETE(kjb_sprintf(buff, sizeof(buff), "%ld", 85));
    dbs(buff);
    pso("%ld\n", 85);



    return EXIT_SUCCESS; 
}

