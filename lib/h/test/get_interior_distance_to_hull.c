
/* $Id: get_interior_distance_to_hull.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "h/h_incl.h" 

#define STEP_2D 0.01
#define STEP_3D 0.05
#define BASE_NUM_TRIES  500

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int    i;
    double x, y, z;
    Hull*  hp;
    Vector* test_vp         = NULL;
    Matrix* point_mp        = NULL;
    int     num_points;
    double distance; 
    double root_2 = sqrt(2.0);
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


    EPETB(set_heap_options("heap-checking", "f")); 
    EPETB(set_qhull_options("qhull-error-file", "qhull-error-log")); 

    if (is_interactive())
    {
        EPETB(set_verbose_options("verbose", "1")); 
    }

    EPETE(get_target_vector(&test_vp, 2)); 
    EPETE(get_target_matrix(&point_mp, 3, 2));

    point_mp->elements[ 0 ][ 0 ] = 0.0;
    point_mp->elements[ 0 ][ 1 ] = 0.0;
    point_mp->elements[ 1 ][ 0 ] = 0.0;
    point_mp->elements[ 1 ][ 1 ] = 1.0;
    point_mp->elements[ 2 ][ 0 ] = 1.0;
    point_mp->elements[ 2 ][ 1 ] = 0.0;

    test_vp->elements[ 0 ] =  0.5*root_2 / (1.0 + root_2);
    test_vp->elements[ 1 ] =  0.5*root_2 / (1.0 + root_2);
    dbf(test_vp->elements[ 0 ]);

    NPETE(hp = find_convex_hull(point_mp, DEFAULT_HULL_OPTIONS));
    EPETE(get_interior_distance_to_hull(hp, test_vp, &distance));
    dbf(distance); 
    free_hull(hp);

    for (i=0; i<num_tries; i++)
    {
        verbose_pso(1, "2D Test %d.\n", i+1);

        num_points = 5 + 100.0 * kjb_rand();
        EPETE(get_random_matrix(&point_mp, num_points, 2));
        NPETE(hp = find_convex_hull(point_mp, DEFAULT_HULL_OPTIONS));

        for (x = 0.0; x <= 1.0; x += STEP_2D)
        {
            for (y = 0.0; y <= 1.0; y += STEP_2D)
            {
                int inside_hull;

                test_vp->elements[ 0 ] = x;
                test_vp->elements[ 1 ] = y;

                EPETE(inside_hull = is_point_inside_hull(hp, test_vp));
                EPETE(get_interior_distance_to_hull(hp, test_vp, &distance));

                if (inside_hull && (distance < 0.0))
                {
                    p_stderr("Point is inside hull, but distance is %e.\n", 
                             distance);
                    status = EXIT_FAILURE;
                }
                else if ( !inside_hull && (distance > 0.0))
                {
                    p_stderr("Point is outside hull, but distance is %e.\n", 
                             distance);
                    status = EXIT_FAILURE;
                }
            }
        }
        free_hull(hp);
    }

    EPETE(get_target_vector(&test_vp, 3)); 

    for (i=0; i<num_tries; i++)
    {
        verbose_pso(1, "3D Test %d.\n", i+1);

        num_points = 5 + 100.0 * kjb_rand();
        EPETE(get_random_matrix(&point_mp, num_points, 3));
        NPETE(hp = find_convex_hull(point_mp, DEFAULT_HULL_OPTIONS));

        for (x = 0.0; x <= 1.0; x += STEP_3D)
        {
            for (y = 0.0; y <= 1.0; y += STEP_3D)
            {
                for (z = 0.0; z <= 1.0; z += STEP_3D)
                {
                    int inside_hull;

                    test_vp->elements[ 0 ] = x;
                    test_vp->elements[ 1 ] = y;
                    test_vp->elements[ 2 ] = z;

                    EPETE(inside_hull = is_point_inside_hull(hp, test_vp));
                    EPETE(get_interior_distance_to_hull(hp, test_vp, &distance));

                    if (inside_hull && (distance < 0.0))
                    {
                        p_stderr("Point is inside hull, but distance is %e.\n", 
                                 distance);
                        status = EXIT_BUG;
                    }
                    else if ( !inside_hull && (distance > 0.0))
                    {
                        p_stderr("Point is outside hull, but distance is %e.\n", 
                                 distance);
                        status = EXIT_BUG;
                    }
                }
            }
        }
        free_hull(hp);
    }

    free_vector(test_vp);
    free_matrix(point_mp);
        
    return status;
}

