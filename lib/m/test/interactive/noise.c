
/* $Id: noise.c 9312 2011-04-13 06:49:57Z kobus $ */


#include "sample/sample_incl.h" 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int i; 

   
    for (i=0; i<100000; i++) 
    {
        pso("%f\n", (double)gauss_rand()); 
    }

    return EXIT_SUCCESS; 
}

