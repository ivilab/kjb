
/* $Id: 3d.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "p/p_incl.h" 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    Matrix* mp = NULL;
    int plot_id; 

    /*
     * Most plot test will not be very interesting in batch mode! 
    */
    if (! is_interactive()) 
    {
        p_stderr("This test program either needs to be adjusted for batch testing or moved.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    /*
    kjb_set_verbose_level(1000);
    */

    ERE(get_random_matrix(&mp, 10, 10));

    plot_id = plot_open3();
    ERE(plot_matrix_values_2(plot_id, mp, 0.0, 1.0, 0.0, 1.0));

    plot_id = plot_open3();
    ERE(plot_matrix_values(plot_id, mp));

    prompt_to_continue();

    plot_close(plot_id);

    free_matrix(mp);

    return EXIT_SUCCESS; 
}

