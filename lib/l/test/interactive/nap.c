
/* $Id: nap.c 20773 2016-08-03 17:37:22Z kobus $ */


#include "l/l_incl.h" 

/*ARGSUSED*/
int  main(void)
{

    /*CONSTCOND*/
    while ( TRUE )
    {
        int i;

        pso("Two seconds of dots.\n"); 
        for (i=0; i< 20; i++)
        {
            nap(100); 
            pso(".");
        }
        pso("\n\n");

        pso("Two seconds of silence.\n"); 
        sleep(2);
        
        pso("Two seconds of dots.\n"); 
        for (i=0; i< 10; i++)
        {
            nap(200); 
            pso(".");
        }
        pso("\n\n");

        pso("Two seconds of silence.\n"); 
        sleep(2);
        
        pso("Two seconds of dots.\n"); 
        for (i=0; i< 4; i++)
        {
            nap(500); 
            pso(".");
        }
        pso("\n\n");

        pso("Two seconds of silence.\n"); 
        sleep(2);
        
    }

    return EXIT_SUCCESS; 
}

