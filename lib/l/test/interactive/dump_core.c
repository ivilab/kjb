
/* $Id: dump_core.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h" 


int main(int argc, char **argv)
{
    int* ptr = NULL;
    int  i;


    i = *ptr; 

    return EXIT_SUCCESS; 
}


