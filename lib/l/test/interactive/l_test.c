
/* $Id: l_test.c 4723 2009-11-16 18:57:09Z kobus $ */


/* =========================================================================== *
|                                                                              |
|  Copyright (c) 1994-1998, by Kobus Barnard (author).                         |
|                                                                              |
|  For use outside the SFU vision lab please contact the author(s).            |
|                                                                              |
* =========================================================================== */

#include "l/l_incl.h" 

/* For declared function pointers. Generally harmless. */
#ifdef __cplusplus     
extern "C" {
#endif 

/* -------------------------------------------------------------------------- */

static int process_option(const char* option, const char* value);

/* -------------------------------------------------------------------------- */

static const char* default_options[ ] = 
{
    "exchange-usr-and-net=t", "disable-dir-open=t", 
    "debug=2", "verbose=5", 
    NULL
} ;
  
/* -------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
    const char** cur_option;
    FILE*        option_fp;


    kjb_init();

    set_atn_trap(confirm_exit_sig_fn, DONT_RESTART_AFTER_SIGNAL); 

    /*
    EPETE(print_version()); 
    */

    /*
    // Not ablsolutely necessary, but more efficient to do it in advance. Also,
    // if we do not do it in advance, we can get bogus memory leak reports, as
    // then the implicit fork will occur when memory has been allocated, and we
    // don't neccessarily arrange for the free by the child process. 
    */
    EPETE(create_system_command_process()); 

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

    if ((option_fp = kjb_fopen("options", "r")) != NULL)
    {
        char line[ 1000 ];

        while (BUFF_GET_REAL_LINE(option_fp, line) != EOF)
        {
            EPE(process_option_string(line, process_option));
        }
        kjb_fclose(option_fp); 
    }

    dbi(sizeof(float));
    dbi(sizeof(double));

#ifdef TEST
    pso("\n-------------------------------------------------------------\n\n"); 
    pso("Process_is_alive\n\n"); 
    test_process_is_alive(); 

#endif 

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



