
/* $Id: fpu.c 4723 2009-11-16 18:57:09Z kobus $ */


/* =========================================================================== *
|                                                                              |
|  Copyright (c) 1994-1998, by Kobus Barnard (author).                         |
|                                                                              |
|  For use outside the SFU vision lab please contact the author(s).            |
|                                                                              |
* =========================================================================== */

#include "m/m_incl.h" 

#ifdef LINUX_X86
#    include "fpu_control.h"
#endif 

/* For declared function pointers. Generally harmless. */
#ifdef __cplusplus     
using namespace kjb_c;
extern "C" {
#endif 

/* -------------------------------------------------------------------------- */

static int process_option(const char* option, const char* value);

/* -------------------------------------------------------------------------- */

static const char* default_options[ ] = 
{
    "exchange-usr-and-net=t", "disable-dir-open=t", 
    "gamma=on", "ciof=off", "cvof=off", "linearization=off", 
    "default-display-program=s2_display++,title",
    "debug=2", "verbose=5", 
    NULL
} ;
  
/* -------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
    const char** cur_option;
    double x = 1.0;
    double y = 0.0;
    double a = -1.0; 
    double z, n;


    kjb_init();

    set_atn_trap(confirm_exit_sig_fn, DONT_RESTART_AFTER_SIGNAL); 

#ifdef HELP_FILE   /* Normally not defined, and we just go with the default. */
    set_help_file(HELP_FILE);
#endif

    /*
    // EPETE(set_monochrome_plot());
    */

    cur_option = default_options;

    while (*cur_option != NULL)
    {
        if (process_option_string(*cur_option, process_option) == ERROR) 
        {
            insert_error("Processing of hard coded default option %q failed.",
                         *cur_option);
            kjb_print_error();
        }
        cur_option++;
    }

#ifdef DEF_OUT
    __setfpucw(0x1372);
#endif 

    dbw();

    n = log(a); 

    dbe(n); 

    z = x / y; 

    dbe(z); 

    kjb_cleanup(); /* Not needed on most platforms, but doing it twice is OK. */
    
    return EXIT_SUCCESS; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#define MAX_NUM_SET_FN  100

static int process_option(const char *option, const char *value)
{ 
    int  result = NOT_FOUND;
    int  temp_result; 
    int  (*set_fn_list[ MAX_NUM_SET_FN ]) (const char* opt, const char* val);

    set_fn_list[ 0 ] = kjb_l_set;
    set_fn_list[ 1 ] = kjb_m_set;

    ERE(temp_result = call_set_fn(1, set_fn_list, "Low Level Options",
                                      option, value));
    if (temp_result == NO_ERROR) result = NO_ERROR; 

    if (result == NOT_FOUND) 
    {
        set_error("%q is an invalid option.", option);
        return ERROR; 
    }

    return NO_ERROR; 
}
    
/* -------------------------------------------------------------------------- */

#ifdef __cplusplus 
}
#endif 



