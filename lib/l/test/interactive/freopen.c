
/* $Id: freopen.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h" 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    char  line[ 30 ];
    FILE* fp1;
    char  name[ 200 ];


    term_get_line("r> ",line, 30); 
    NPETE(fp1 = kjb_fopen(line,"r")); 

    BUFF_GET_FD_NAME(fileno(stdout), name); 
    dbs(name); 
    BUFF_GET_FD_NAME(fileno(fp1), name); 
    dbs(name); 

    term_get_line("w> ",line, 30); 
    NPETE(kjb_freopen(line,"w", stdout)); 

    term_get_line("r> ",line, 30); 
    NPETE(kjb_freopen(line,"r", fp1)); 

    pso("HELLO WORLD"); 

    BUFF_GET_FD_NAME(fileno(stdout), name); 
    dbs(name); 

    BUFF_GET_FD_NAME(fileno(fp1), name); 
    dbs(name); 

    return EXIT_SUCCESS; 
}

