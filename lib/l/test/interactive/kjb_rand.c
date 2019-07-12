
/* $Id: kjb_rand.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h" 

int main(int argc, char **argv)
{
    int i;


    for (i=0; i<5; i++) 
    {
        kjb_seed_rand(99999999, 27); 
        pso("R1 %f    ", kjb_rand());
        pso("R2 %f\n", kjb_rand_2());
    }

    pso("\n");

    for (i=0; i<5; i++) 
    {
        kjb_seed_rand_2(99999999); 
        pso("R1 %f    ", kjb_rand());
        pso("R2 %f\n", kjb_rand_2());
    }

    return EXIT_SUCCESS; 
}

