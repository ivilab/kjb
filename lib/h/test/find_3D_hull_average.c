
/* $Id: find_3D_hull_average.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "h/h_incl.h" 

#define RESOLUTION 30.0

#define BASE_NUM_TRIES  200

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int    i;
    double x, y, z;
    Hull*  hp;
    Vector* old_ave_vp      = NULL;
    Vector* ave_vp      = NULL;
    Vector* test_vp      = NULL;
    Matrix* point_mp      = NULL;
    int num_points; 
    double    grid_mass;
    double    mass, old_mass;
    double    cx, cy, cz;
    Vector* grid_ave_vp = NULL; 
    double    x_min, x_max, y_min, y_max, z_min, z_max;
    double    x_step, y_step, z_step; 
    Vector* min_vp = NULL;
    Vector* max_vp = NULL;
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

    EPETB(set_hull_options("hir", "200")); 
    EPETB(set_hull_options("har", "200")); 
    EPETB(set_heap_options("heap-checking", "f")); 
    EPETB(set_qhull_options("qhull-error-file", "qhull-error-log")); 

    if (is_interactive())
    {
        EPETB(set_verbose_options("verbose", "1")); 
    }

    EPETE(get_target_vector(&test_vp, 3)); 
    EPETE(get_target_vector(&grid_ave_vp, 3)); 

    for (i=0; i<num_tries; i++)
    {
        verbose_pso(1, "Test %d.\n", i+1);

        num_points = 5 + 100.0 * kjb_rand();
        EPETE(get_random_matrix(&point_mp, num_points, 3));

        NPETE(hp = find_convex_hull(point_mp, DEFAULT_HULL_OPTIONS));

        EPE(approximate_hull_average(hp, &old_ave_vp, &old_mass));

        EPE(get_hull_CM_and_volume(hp, &ave_vp, &mass));

        /* pso("Finding average and mass with grid.\n");  */

        ERE(get_min_matrix_col_elements(&min_vp, hp->vertex_mp));
        ERE(get_max_matrix_col_elements(&max_vp, hp->vertex_mp));

        x_min = min_vp->elements[ 0 ]; 
        y_min = min_vp->elements[ 1 ]; 
        z_min = min_vp->elements[ 2 ]; 

        x_max = ADD_DBL_EPSILON(max_vp->elements[ 0 ]); 
        y_max = ADD_DBL_EPSILON(max_vp->elements[ 1 ]); 
        z_max = ADD_DBL_EPSILON(max_vp->elements[ 2 ]); 

        x_step = (x_max - x_min) / RESOLUTION; 
        y_step = (y_max - y_min) / RESOLUTION; 
        z_step = (z_max - z_min) / RESOLUTION; 

        grid_mass = cx = cy = cz = 0.0; 

        for (x = x_min; x <= x_max; x += x_step)
        {
            for (y = y_min; y <= y_max; y += y_step)
            {
                for (z = z_min; z <= z_max; z += z_step)
                {
                    int inside_hull;

                    test_vp->elements[ 0 ] = x;
                    test_vp->elements[ 1 ] = y;
                    test_vp->elements[ 2 ] = z;

                    EPETE(inside_hull = is_point_inside_hull(hp, test_vp));

                    if (inside_hull)
                    {
                        cx += x;
                        cy += y;
                        cz += z;
                        grid_mass++; 
                    }
                }
            }
        }

        grid_ave_vp->elements[ 0 ] = cx / grid_mass;
        grid_ave_vp->elements[ 1 ] = cy / grid_mass; 
        grid_ave_vp->elements[ 2 ] = cz / grid_mass; 
        grid_mass *= x_step;
        grid_mass *= y_step;
        grid_mass *= z_step;

        if (is_interactive())
        {
            db_rv(ave_vp);
            db_rv(old_ave_vp);
            db_rv(grid_ave_vp);
            dbf(mass); 
            dbf(old_mass); 
            dbf(grid_mass);
        }

        if (ABS_OF(grid_mass - mass) > 6.0 / RESOLUTION)
        {
            p_stderr("Assertion that mass and grid_mass should be close failed.\n");
            status = EXIT_BUG;
        }

        if (vector_distance(ave_vp, grid_ave_vp) > RESOLUTION)
        {
            p_stderr("Assertion that CM and grid CM should be close failed.\n");
            status = EXIT_BUG;
        }


        free_hull(hp);
    }

    free_vector(test_vp);
    free_vector(grid_ave_vp);
    free_vector(ave_vp);
    free_vector(old_ave_vp);
    free_matrix(point_mp); 
        
    return status;
}

