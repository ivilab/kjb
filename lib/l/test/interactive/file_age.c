
/* $Id: file_age.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h" 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    time_t file_age; 


    EPETE(get_file_age(argv[ 1 ], &file_age)); 

    pso("%lu\n",(unsigned long)file_age); 

    return EXIT_SUCCESS; 
}

