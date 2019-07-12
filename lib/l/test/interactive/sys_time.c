
/* $Id: sys_time.c 21602 2017-07-31 20:36:24Z kobus $ */


#include "l/l_incl.h"

/*ARGSUSED*/
int  main(int argc, char **argv)
{
    int  i, j;
    int  real_time;
    int  cpu_time;
    char time_string[ 200 ];
    char  line[ 3000 ];
    int   res;


    get_time(time_string, 200); 
    dbs(time_string); 

    term_puts("\nEnter time formats (see strftime(3)), or EOF\n\n");
    while ((res=BUFF_STDIN_GET_LINE("fmt> ",line)) != EOF) 
    {
        EPETE(res);
        BUFF_GET_TIME_2(time_string, line);
        pso(time_string);
        pso("\n");
    }

    
    init_cpu_time();
    init_real_time(); 

    for (i=0; i< 10000000; i++)
    {
        j = i + i;
    }

    real_time = get_real_time(); 
    dbi(real_time); 
    display_real_time();

    cpu_time = get_cpu_time();
    dbi(cpu_time); 
    display_cpu_time();

    p_stderr("The following should be 2*(10000000-1): %d.\n\n", j); 

    p_stderr("The following should take about 5 seconds.\n");

    for (i=0; i<100; i++)
    {
        nap(50); 
    }
    p_stderr("Done.\n\n"); 

    p_stderr("The following should also take 5 seconds.\n");

    nap(5000); 

    p_stderr("Done.\n\n"); 

    p_stderr("The following should also take 5 seconds.\n");

    set_time_alarm(5); 
    pause();
    unset_time_alarm();  

    p_stderr("Done.\n\n"); 

    return EXIT_SUCCESS; 
}

