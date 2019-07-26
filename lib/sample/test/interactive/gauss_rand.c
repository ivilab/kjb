
/* $Id: gauss_rand.c 4723 2009-11-16 18:57:09Z kobus $ */


#define NUM_TO_TRY (10000000)

#include "m/m_incl.h" 
#include "p/p_incl.h" 
#include "sample/sample_incl.h"

/*ARGSUSED*/
int main(int argc, char **argv)
{
    Vector* vp = NULL; 
    int plot_id, plot_id_2; 

    init_cpu_time();
    EPETE(get_lookup_gauss_random_vector(&vp, NUM_TO_TRY));
    p_stderr("Lookup CPU time: %ld milli-seconds\n", get_cpu_time()); 

    EPETE(plot_id = plot_open()); 
    EPETE(plot_histogram(plot_id, vp, 100, DBL_NOT_SET, NULL));


    init_cpu_time(); 
    EPETE(get_gauss_random_vector(&vp, NUM_TO_TRY));
    p_stderr("Box-Muller CPU time: %ld milli-seconds\n", get_cpu_time()); 

    EPETE(plot_id_2 = plot_open()); 
    EPETE(plot_histogram(plot_id_2, vp, 100, DBL_NOT_SET, NULL));


    prompt_to_continue(); 

    free_vector(vp);
    plot_close(plot_id); 

    return EXIT_SUCCESS; 
}

