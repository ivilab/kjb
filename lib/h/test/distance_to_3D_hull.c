
/* $Id: distance_to_3D_hull.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "h/h_incl.h" 

/*
// Remember: With bad luck, the approximate hulls can have a point arbitrarily
// far away from the correct hull, by having a very small angle at one of the
// veritices.
*/
#define SANITY_CHECKS

#define STEP 0.05
#define BASE_NUM_TRIES   1000


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int     i;
    double  x, y, z;
    Hull*   hp;
    Vector* grid_vp      = NULL;
    Vector* test_vp      = NULL;
    Matrix* point_mp     = NULL;
    int     num_points;
    double    d, distance;
    double    new_distance;
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

    EPETE(set_hull_options("hir", "1000")); 
    EPETE(set_heap_options("heap-checking", "f")); 
    EPETE(set_debug_options("debug-level", "2")); 
    EPETE(set_qhull_options("qhull-error-file", "qhull-error-log")); 

#ifdef SANITY_CHECKS
    EPETE(get_target_matrix(&point_mp, 8, 3));
    point_mp->elements[ 0 ][ 0 ] = 0.0;
    point_mp->elements[ 0 ][ 1 ] = 0.0;
    point_mp->elements[ 0 ][ 2 ] = 0.0;
    point_mp->elements[ 1 ][ 0 ] = 1.0;
    point_mp->elements[ 1 ][ 1 ] = 0.0;
    point_mp->elements[ 1 ][ 2 ] = 0.0;
    point_mp->elements[ 2 ][ 0 ] = 1.0;
    point_mp->elements[ 2 ][ 1 ] = 1.0;
    point_mp->elements[ 2 ][ 2 ] = 0.0;
    point_mp->elements[ 3 ][ 0 ] = 0.0;
    point_mp->elements[ 3 ][ 1 ] = 1.0;
    point_mp->elements[ 3 ][ 2 ] = 0.0;
    point_mp->elements[ 4 ][ 0 ] = 0.0;
    point_mp->elements[ 4 ][ 1 ] = 0.0;
    point_mp->elements[ 4 ][ 2 ] = 1.0;
    point_mp->elements[ 5 ][ 0 ] = 1.0;
    point_mp->elements[ 5 ][ 1 ] = 0.0;
    point_mp->elements[ 5 ][ 2 ] = 1.0;
    point_mp->elements[ 6 ][ 0 ] = 1.0;
    point_mp->elements[ 6 ][ 1 ] = 1.0;
    point_mp->elements[ 6 ][ 2 ] = 1.0;
    point_mp->elements[ 7 ][ 0 ] = 0.0;
    point_mp->elements[ 7 ][ 1 ] = 1.0;
    point_mp->elements[ 7 ][ 2 ] = 1.0;

    NPETE(hp = find_convex_hull(point_mp, DEFAULT_HULL_OPTIONS));

    EPETE(get_target_vector(&test_vp, 3));

    test_vp->elements[ 0 ] = 0.5;
    test_vp->elements[ 1 ] = 0.5;
    test_vp->elements[ 2 ] = 0.5;

    EPETE(get_distance_to_hull(hp, test_vp, &new_distance));
    pso("Interior test distance is %.5e\n", new_distance); 

    test_vp->elements[ 0 ] = 0.9;
    test_vp->elements[ 1 ] = 1.7;
    test_vp->elements[ 2 ] = 0.9;

    EPETE(get_distance_to_hull(hp, test_vp, &new_distance));
    pso("Facet test distance is %.5e\n", new_distance); 

    test_vp->elements[ 0 ] = 0.9;
    test_vp->elements[ 1 ] = 1.3;
    test_vp->elements[ 2 ] = 1.4;

    EPETE(get_distance_to_hull(hp, test_vp, &new_distance));
    pso("Edge test distance is %.5e\n", new_distance); 


    test_vp->elements[ 0 ] = 1.5;
    test_vp->elements[ 1 ] = 1.6;
    test_vp->elements[ 2 ] = 1.7;

    EPETE(get_distance_to_hull(hp, test_vp, &new_distance));
    pso("Vertex test distance is %.5e\n", new_distance); 

    free_hull(hp); 
#endif

    EPETE(get_target_vector(&grid_vp, 3)); 

    for (i=0; i<num_tries; i++)
    {
        if (is_interactive())
        {
            pso("Test %d.\n", i+1);
        }

        num_points = 5 + 10000.0 * kjb_rand();
        EPETE(get_random_matrix(&point_mp, num_points, 3));
        NPETE(hp = find_convex_hull(point_mp, DEFAULT_HULL_OPTIONS));
        EPETE(get_random_vector(&test_vp, 3));
        EPETE(ow_multiply_vector_by_scalar(test_vp, 2.0));

        if (is_point_inside_hull(hp, test_vp))
        {
            get_interior_distance_to_hull(hp, test_vp, &distance);
            distance = -distance;
        }
        else 
        {
            distance = DBL_MAX;

            for (x = 0.0; x <= 1.0; x += STEP)
            {
                for (y = 0.0; y <= 1.0; y += STEP)
                {
                for (z = 0.0; z <= 1.0; z += STEP)
                    {
                        int inside_hull;

                        grid_vp->elements[ 0 ] = x;
                        grid_vp->elements[ 1 ] = y;
                        grid_vp->elements[ 2 ] = z;

                        EPETE(inside_hull = is_point_inside_hull(hp, grid_vp));

                        if (inside_hull)
                        {
                            d = vector_distance(grid_vp, test_vp);

                            if (d < distance) distance = d;
                        }
                    }
                }
            }

        }

        EPETE(get_distance_to_hull(hp, test_vp, &new_distance));

        if (ABS_OF(distance - new_distance) > 2.0 * STEP) 
        {
            IMPORT int kjb_debug_level;

            dbf(distance);
            dbf(new_distance);
            db_rv(test_vp);
            db_mat(point_mp); 

            kjb_debug_level = 10;
            EPETE(get_distance_to_hull(hp, test_vp, &new_distance));
            kjb_debug_level = 2;

            status = EXIT_BUG; 
        }

        free_hull(hp);

        kjb_flush();
    }

    free_vector(grid_vp);
    free_vector(test_vp);
    free_matrix(point_mp); 
        
    return status; 
}

