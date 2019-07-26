
/* $Id: find_3D_max_coord_prod.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "h2/h2_incl.h" 

#define BASE_NUM_TRIES   5000

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int     i;
    Hull*   hp           = NULL;
    Matrix* point_mp     = NULL;
    int     num_points;
    double  max_prod;
    double  est_max_prod;
    int     resolution   = 200;
    char    resolution_str[ 1000 ];
    int     pass_count  = 0;
    int     fail_count  = 0;
    int     num_tries   = BASE_NUM_TRIES;
    int     test_factor = 1;


    kjb_init(); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (test_factor <= 0)
    {
        num_tries = 1;
    }
    else
    {
        num_tries *= test_factor;
    } 

    if (is_interactive())
    {
        EPETB(set_verbose_options("verbose", "1")); 
    }

    kjb_l_set("page", "off"); 

    EPETE(kjb_sprintf(resolution_str, sizeof(resolution_str),"%d", resolution)); 
    EPETE(set_hull_max_coord_prod_options("hull-max-coord-product-resolution", 
                                          resolution_str)); 
    /*
    EPETE(set_qhull_options("qhull-error-file", "qhull-error-log")); 
    */

    for (i=0; i<num_tries; i++)
    {
        verbose_pso(1, "Test %d ... ", i+1);

        num_points = kjb_rint(5 + 100.0 * kjb_rand());
        EPETE(get_random_matrix(&point_mp, num_points, 3));

        EPETE(get_convex_hull(&hp, point_mp, DEFAULT_HULL_OPTIONS));

        EPETE(find_hull_max_coord_product(hp, NULL, &max_prod));
        EPETE(estimate_hull_max_coord_product(hp, NULL, &est_max_prod));

        /* 
        // Allowable error is to be a function of resolution.
        // Ideally, it should be less than 3*delta.
        */
        if (ABS_OF(max_prod - est_max_prod) > 3.0 / resolution)
        {
            p_stderr("failed (%.3e).\n", ABS_OF(max_prod - est_max_prod)); 
            dbe(ABS_OF(max_prod - est_max_prod)); 
            fail_count++; 
        }
        else
        {
            verbose_pso(1, "passed (%.3e).\n", ABS_OF(max_prod - est_max_prod)); 
            pass_count++; 
        }
    }

    free_matrix(point_mp); 
    free_hull(hp);

    verbose_pso(1, "\n\n%d passed and %d failed.\n", pass_count, fail_count);

    if (fail_count > 0)
    {
        p_stderr("\n\n%d tests failed.\n", fail_count);
        return EXIT_BUG;
    }
    else
    {
        return EXIT_SUCCESS; 
    }
}

