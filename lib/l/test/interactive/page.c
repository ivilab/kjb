
/* $Id: page.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h"

/*ARGSUSED*/
int  main(int argc, char **argv)
{
    IMPORT int volatile kjb_tty_cols;
    unsigned int i;
    int          j;
    char*        str;
    char         line[ 200 ];


    /* Force tty open so that kjb_tty_cols is valid */
    term_puts("\n"); 

    for (i=0; i< sizeof(line)-ROOM_FOR_NULL;  i++) 
    {
        line[i] = '0' + i % 10;
    }

    line[ sizeof(line)- ROOM_FOR_NULL ] = '\0';

    line[ kjb_tty_cols - 1] = '*';
    line[ kjb_tty_cols ] = '\n';
    line[ kjb_tty_cols  + 1] = '\0';


    for (i=0; i<150; i++)
    {
        pso(line);
    }


    line[ kjb_tty_cols - 2] = '*';
    line[ kjb_tty_cols - 1] = '\n';
    line[ kjb_tty_cols] = '\0';


    for (i=0; i<150; i++)
    {
        pso(line);
    }


    line[ kjb_tty_cols - 3] = '*';
    line[ kjb_tty_cols - 2] = '\n';
    line[ kjb_tty_cols - 1] = '\0';


    for (i=0; i<150; i++)
    {
        pso(line);
    }


    NPETE(str=STR_MALLOC(100));

    for (i=0; i<10; i++) {
        for (j=0; j<100; j++) {
            sprintf(str, "<------%2d------->", j);
            kjb_fprintf(stderr, str);
        }
    }


    for (i=0; i<3000; i++) {
        strcpy(str, "123456789ABCD");
        kjb_fprintf(stderr, str);
    }

    kjb_free(str); 

    return EXIT_SUCCESS;
} 

