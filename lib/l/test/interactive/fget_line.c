
/* $Id: fget_line.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h" 

TRAP_FN_RETURN_TYPE test_atn_fn(int);

/*ARGSUSED*/
int main(int argc, char **argv)
{
    char line[ 30 ];
    int  res;


    kjb_l_set("debug_level", "1"); 

    set_atn_trap(test_atn_fn, DONT_RESTART_AFTER_SIGNAL); 

    set_no_blocking(fileno(stdin));

    while ((res=fget_line(stdin, line,30)) != EOF) 
    {
        EPETE(res);

        dbs(line); 
        dbi(res); 

        nap(3000);

        dbm("Nap Done.\n");
    }

    dbi(res);
    return EXIT_SUCCESS; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/
TRAP_FN_RETURN_TYPE test_atn_fn(int dummy_sig_arg)
{

    term_puts("Trapped.\n");

}


