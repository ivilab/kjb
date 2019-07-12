
/* $Id: malloc_overun.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h"

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int    i;
    unsigned char* uchar_mem;
    int*   int_mem;


    kjb_l_set("debug_level", "1"); 

    for (i=0; i<12; i++)
    {
        dbi(i); 
        int_mem = (int*)KJB_CALLOC(10, sizeof(int));
        dbi(int_mem[ 9 ] ); 
        dbi(int_mem[ 10 ] ); 
        dbi(int_mem[ 11 ] ); 
        int_mem[ i ] = -999;
        kjb_free(int_mem);
    }

    for (i=0; i<13; i++)
    {
        dbi(i); 
        uchar_mem = BYTE_MALLOC(10);
        dbc(uchar_mem[ 9 ] ); 
        dbc(uchar_mem[ 10 ] ); 
        dbc(uchar_mem[ 11 ] ); 
        uchar_mem[ i ] = '0';
        kjb_free(uchar_mem);
    }

    return EXIT_SUCCESS; 
}



