
/* $Id: term2.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h" 

Queue_element *string_queue_head, *string_queue_tail; 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    char line[ 30 ]; 
    int  res; 


    term_set_raw_mode_with_no_echo();

    set_atn_trap(raw_mode_with_no_echo_sig_fn, DONT_RESTART_AFTER_SIGNAL); 
    kjb_signal(SIGTERM, reset_term_before_default_sig_fn);
    kjb_signal(SIGQUIT, reset_term_before_default_sig_fn);
    kjb_signal(SIGTTIN, reset_term_before_default_sig_fn);
    kjb_signal(SIGTTOU, reset_term_before_default_sig_fn);
    kjb_signal(SIGXCPU, reset_term_before_default_sig_fn);
    kjb_signal(SIGXFSZ, reset_term_before_default_sig_fn);

    kjb_l_set("debug_level", "1"); 

    while ((res=term_get_line("abc>",line,30)) != EOF)
    {
        EPETE(res);

        dbs(line); 
        dbi(res); 
    }

    term_reset();

    return EXIT_SUCCESS; 
}

