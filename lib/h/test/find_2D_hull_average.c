
/* $Id: find_2D_hull_average.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "h/h_incl.h" 

#define STEP 0.02
#define NUM_RANDOM_SAMPLES 1000
#define BASE_NUM_TRIES   1000

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int    i;
    double x, y;
    Hull*  hp;
    Vector* ave_vp      = NULL;
    Vector* grid_ave_vp     = NULL;
    Vector* test_vp         = NULL;
    Matrix* point_mp        = NULL;
    int     num_points;
    double    grid_mass;
    double    mass;
    double    cx, cy;
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;


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

    EPETB(set_hull_options("hir", "100")); 
    EPETB(set_heap_options("heap-checking", "f")); 
    EPETB(set_qhull_options("qhull-error-file", "qhull-error-log")); 

    if (is_interactive())
    {
        EPETB(set_verbose_options("verbose", "1")); 
    }

    EPETE(get_target_vector(&test_vp, 2)); 
    EPETE(get_target_vector(&grid_ave_vp, 2)); 

    for (i=0; i<num_tries; i++)
    {
        verbose_pso(1, "Test %d.\n", i+1);

        num_points = 5 + 100.0 * kjb_rand();
        EPETE(get_random_matrix(&point_mp, num_points, 2));
        NPETE(hp = find_convex_hull(point_mp, DEFAULT_HULL_OPTIONS));

        EPE(get_hull_CM_and_volume(hp, &ave_vp, &mass));

        verbose_pso(2, "Finding average and mass with grid.\n"); 

        grid_mass = cx = cy = 0.0; 

        for (x = 0.0; x <= 1.0; x += STEP)
        {
            for (y = 0.0; y <= 1.0; y += STEP)
            {
                int inside_hull;

                test_vp->elements[ 0 ] = x;
                test_vp->elements[ 1 ] = y;

                EPETE(inside_hull = is_point_inside_hull(hp, test_vp));

                if (inside_hull)
                {
                    cx += x;
                    cy += y;
                    grid_mass++; 
                }
            }
        }

        grid_ave_vp->elements[ 0 ] = cx / grid_mass;
        grid_ave_vp->elements[ 1 ] = cy / grid_mass; 
        grid_mass *= STEP;
        grid_mass *= STEP;

        if (is_interactive())
        {
            db_rv(ave_vp);
            db_rv(grid_ave_vp);
            dbf(grid_mass);
            dbf(mass); 
        }

        if (ABS_OF(grid_mass - mass) > 4.0 * STEP)
        {
            p_stderr("Assertion that mass and grid_mass should be close failed.\n");
            status = EXIT_BUG;
        }

        if (vector_distance(ave_vp, grid_ave_vp) > STEP)
        {
            p_stderr("Assertion that CM and grid_CM should be close failed.\n");
            status = EXIT_BUG;
        }

        free_hull(hp);
    }

    free_vector(test_vp);
    free_vector(ave_vp);
    free_vector(grid_ave_vp);
    free_matrix(point_mp); 
        
    return status; 
}

