
/* $Id: kjb_err2.c 21602 2017-07-31 20:36:24Z kobus $ */



#include "l/l_incl.h" 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    char buff[ 1000 ]; 
    int* y = NULL;
    /*LINTED*/
    int  x;


    set_default_abort_trap();

    p_stderr("Test bug handling by entering something like \"Foo %%Z\"\n");
    p_stderr("Test core dump handling by entering something starting with \"*\"\n");

    while (BUFF_STDIN_GET_LINE("input> ", buff) != EOF)
    {
        if (buff[ 0 ] == '*') 
        {
            x = *y;
        }

        p_stderr("  You entered ");
        EPE(p_stderr(buff));
        p_stderr("\n");
    }


    return EXIT_SUCCESS;
}

