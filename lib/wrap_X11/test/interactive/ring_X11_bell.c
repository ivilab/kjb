
/* $Id: ring_X11_bell.c 11388 2012-01-10 23:46:54Z kobus $ */


#include "l/l_incl.h" 
#include "wrap_X11/wrap_X11.h" 

int main(int argc, char **argv)
{
    int i;


    for (i=0; i<5; i++) 
    {
        EPETE(ring_X11_bell());
        sleep(1);
    }

    return EXIT_SUCCESS; 
}

