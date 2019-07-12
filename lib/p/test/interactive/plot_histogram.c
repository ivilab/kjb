/* $Id: plot_histogram.c 21491 2017-07-20 13:19:02Z kobus $ */

#include "p/p_incl.h"
/* 
 * Generate a vector of random numbers and plot the histogram. 
 */

/*ARGSUSED*/
int main(int argc, char **argv)
{
    Vector *scale_vp = NULL;
    int plot_id; 
    int num_elements = 100;
    int num_bins = 5;
    int result;

    /*
     * Most plot test will not be very interesting in batch mode! 
     */
    if (! is_interactive()) 
    {
        p_stderr("This test program either needs to be adjusted for batch testing or moved.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }

    /*verbose_pso(5, "Generate a random vector.\n");*/
    result = get_random_vector( &scale_vp, num_elements );
    if (result == ERROR)
    {
        /*pso("Unable to create a random vector.");*/
        add_error("Unable to create a random vector.");
        free_vector(scale_vp);
        return ERROR;
    }

    /*verbose_pso(5, " Plot the histogram.\n");*/
    plot_id = plot_open();
    if (plot_id == ERROR)
    {
        /*pso("Unable to open a plot.");*/
        add_error("Unable to open a plot.");
        free_vector(scale_vp);
        return ERROR;
    }  

    /* Plots a histogram for the vector of data, which is passed as the second parameter. 
       The number of bins is supplied as the third parameter. 
       If the fourth parameter (sigma) is positive, it is used to smooth the histograms. 
       If name is not NULL (last parameter), then it is used to label the histogram. 
     */
    result = plot_histogram(plot_id, scale_vp, num_bins, 0, " Histogram of random numbers.");
    if (result == ERROR)
    {
        /*pso("An error occured creating the plot.");*/
        add_error("An error occured creating the plot.");
        free_vector(scale_vp);
        return ERROR;
    }

    prompt_to_continue();

    plot_close(plot_id);

    free_vector(scale_vp);

    return EXIT_SUCCESS; 
}
