
/* $Id: kjb_mkdir.c 21602 2017-07-31 20:36:24Z kobus $ */


#include "l/l_incl.h" 

int main(void)
{
    char dir[ MAX_FILE_NAME_SIZE ];
    


    while (BUFF_STDIN_GET_LINE("dir> ", dir) != EOF)
    {
        EPE(kjb_mkdir(dir));
    }

    return EXIT_SUCCESS; 
} 


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

