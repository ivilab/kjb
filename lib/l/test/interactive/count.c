
/* $Id: count.c 4723 2009-11-16 18:57:09Z kobus $ */



#include "l/l_incl.h" 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int  res;
    char line[ 1000 ];



    EPE(res = count_real_lines(stdin)); 
    dbi(res); 

    EPE(res = count_tokens_in_file(stdin));
    dbi(res); 

    EPE(res = gen_count_tokens_in_file(stdin, " ,"));
    dbi(res); 

    EPE(BUFF_FGET_LINE(stdin, line)); 

    EPE(res = count_real_lines(stdin)); 
    dbi(res); 

    EPE(res = count_tokens_in_file(stdin));
    dbi(res); 

    EPE(res = gen_count_tokens_in_file(stdin, " ,"));
    dbi(res); 

    return EXIT_SUCCESS; 
}

