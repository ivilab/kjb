
/* $Id: canon.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h" 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int  done;
    int  res;


    done = FALSE;

    while (! done) 
    {
        res = term_getc();

        dbi(res); 

        if (res == 'n') 
        {
            dbi(term_set_raw_mode_with_no_echo()); 
        }
        else if (res == 'e')
        {
            dbi(term_set_raw_mode_with_echo()); 
        }
        else if (res == 'c')
        {
            dbi(term_set_cooked_mode()); 
        }
        else if (res == 'r')
        {
            p_stderr("Calling term_reset.\n"); 
            term_reset(); 
        }
        else if (res == EOF) 
        {
            done = TRUE; 
        }
    }

    return EXIT_SUCCESS; 
}

