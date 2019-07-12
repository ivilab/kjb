
/* $Id: dbu.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h" 


int main(int argc, char *argv[])
{
    unsigned long  ui = 1;
    int i;

    for (i = 0; i < 40; i++)
    {
        dbi(ui);
        dbj(ui);
        dbu(ui);

        ui *= (unsigned int)2;
    }



    return EXIT_SUCCESS; 
}


