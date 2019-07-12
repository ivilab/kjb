/*
 * $Id: fish.c 21491 2017-07-20 13:19:02Z kobus $
 */

#include <l/l_sys_io.h>
#include <sample/sample_misc.h>

int main(int argc, char** argv)
{
    int histo[10] = {0,0,0,0,0,0,0,0,0,0};
    int j;
    double lambda = 4;
    
    for (j = 0; j < 1000; ++j) 
    {
        int x_poisson = poisson_rand(lambda);
        if (x_poisson < 0) return EXIT_FAILURE;
        if (x_poisson > 9) continue; /* rare but possible */
        histo[ x_poisson ] += 1;
    }

    /* Poisson(lambda=4) has pmf with peaks at 3 and 4.
     * So the mass at 3 should exceed the mass at 2, 5.
     * Ditto for the mass at 4.
     *
     * To repeat:  THE CRITERION BELOW IS CUSTOMIZED FOR LAMBDA OF FOUR.
     */
    if (    histo[3] < histo[2]
        ||  histo[3] < histo[5]
        ||  histo[4] < histo[2]
        ||  histo[4] < histo[5]
       )
    {
        kjb_fprintf(stderr, "Bad Poisson histogram (for lambda=%f):", lambda);
        for (j = 0; j < 10; ++j)
        {
            kjb_fprintf(stderr, " %d", histo[j]);
        }
        kjb_fprintf(stderr, "\n");
        return EXIT_BUG;
    }

    return EXIT_SUCCESS;
}
