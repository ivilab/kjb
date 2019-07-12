
/* $Id: term_get_line.c 21602 2017-07-31 20:36:24Z kobus $ */


#include "l/l_incl.h" 


/*ARGSUSED*/
int main(int argc, char **argv)
{
    char line[ 3000 ]; 
    int  res; 


    while ((res=BUFF_STDIN_GET_LINE("abc>",line)) != EOF)
    {
        EPETE(res);
        kjb_fprintf(stderr, "%s\n", line);
    }

    return EXIT_SUCCESS; 
}

