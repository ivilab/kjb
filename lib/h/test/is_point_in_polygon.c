
/* $Id: is_point_in_polygon.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "g/g_incl.h" 
#include "h/h_incl.h" 

/*
// NOTE: So far stress testing is limited to 3D, and points that are inside the
// polygon. To do points inside and out, we can generate point on the plane, and
// then apply find_hull to the points, and chekc with is_point_inside_hull. Checking
// for 2D is a bit trickier BECAUSE Qhull does NOT return the hull points in
// cyclic order.
*/
#define NUM_SUB_TESTS  50
#define BASE_NUM_TRIES  1000

/*ARGSUSED*/
int main(int argc, char *argv[])
{
    int     status      = EXIT_SUCCESS;
    int     i, j;
    Vector* test_vp     = NULL;
    Vector* a_vp        = NULL;
    int     inside;
    Hull*   hp          = NULL;
    Hull*   facet_hp    = NULL;
    Matrix* point_mp    = NULL;
    int     num_points;
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
        /*
         * This fails regression testing with test_factor greater than 1. It may
         * be the case that the method of generating the data does not give
         * coplanar or in-sequence points. Also, there is no concept of
         * tolerance, so a test point near the boundary might cause failure.
        */
        num_tries *= test_factor;
    } 

    EPETB(set_heap_options("heap-checking", "f")); 

    if (is_interactive())
    {
        EPETB(set_verbose_options("verbose", "1")); 
    }


    EPETE(get_target_matrix(&point_mp, 4, 2));
    point_mp->elements[ 0 ][ 0 ] = 0.0;
    point_mp->elements[ 0 ][ 1 ] = 0.0;
    point_mp->elements[ 1 ][ 0 ] = 1.0;
    point_mp->elements[ 1 ][ 1 ] = 0.0;
    point_mp->elements[ 2 ][ 0 ] = 1.0;
    point_mp->elements[ 2 ][ 1 ] = 1.0;
    point_mp->elements[ 3 ][ 0 ] = 0.0;
    point_mp->elements[ 3 ][ 1 ] = 1.0;

    EPETE(get_target_vector(&test_vp, 2));
    test_vp->elements[ 0 ] = 0.5;
    test_vp->elements[ 1 ] = 0.5;
    EPETE(inside = is_point_in_polygon(point_mp, test_vp)); 
    if (!inside) 
    {
        p_stderr("Simple test 1 failed.\n"); 
        status = EXIT_BUG;
    }

    EPETE(get_target_vector(&test_vp, 2));
    test_vp->elements[ 0 ] = 0.5;
    test_vp->elements[ 1 ] = 1.0;
    EPETE(inside = is_point_in_polygon(point_mp, test_vp)); 
    if (!inside) 
    {
        p_stderr("Simple test 2 failed.\n"); 
        status = EXIT_BUG;
    }

    EPETE(get_target_vector(&test_vp, 2));
    test_vp->elements[ 0 ] = 0.0;
    test_vp->elements[ 1 ] = 0.0;
    EPETE(inside = is_point_in_polygon(point_mp, test_vp)); 
    if (!inside) 
    {
        p_stderr("Simple test 2 failed.\n"); 
        status = EXIT_BUG;
    }

    EPETE(get_target_vector(&test_vp, 2));
    test_vp->elements[ 0 ] = 0.5;
    test_vp->elements[ 1 ] = 1.5;
    EPETE(inside = is_point_in_polygon(point_mp, test_vp)); 
    if (inside) 
    {
        p_stderr("Simple test 3 failed.\n"); 
        status = EXIT_BUG;
    }

    EPETE(get_target_vector(&test_vp, 2));
    test_vp->elements[ 0 ] = 1.5;
    test_vp->elements[ 1 ] = 0.5;
    EPETE(inside = is_point_in_polygon(point_mp, test_vp)); 
    if (inside) 
    {
        p_stderr("Simple test 4 failed.\n"); 
        status = EXIT_BUG;
    }

    for (i=0; i<num_tries; i++)
    {
        verbose_pso(1, "3D Test %d.\n", i+1);

        num_points = 5 + 50.0 * kjb_rand();
        EPETE(get_random_matrix(&point_mp, num_points, 3));
        EPETE(get_convex_hull(&hp, point_mp, DEFAULT_HULL_OPTIONS));

        for (j=0; j<NUM_SUB_TESTS; j++)
        {
            int k;

            for (k=0; k<hp->num_facets; k++)
            {
                const Matrix* vertex_mp = hp->facets->elements[k];

                EPETE(get_random_vector(&a_vp, vertex_mp->num_rows));
                EPETE(ow_normalize_vector(a_vp, NORMALIZE_BY_SUM));
                EPETE(multiply_vector_and_matrix(&test_vp, a_vp, vertex_mp));

                EPETE(inside = is_point_in_polygon(vertex_mp, test_vp));

                if (!inside)
                {
                    p_stderr("Convex combination of vertices is NOT inside.\n");
                    
                    if (is_interactive())
                    {
                        db_mat(vertex_mp);
                        db_rv(test_vp); 
                    }

                    status = EXIT_BUG;
                }
            }
        }

    }

    free_vector(test_vp);
    free_vector(a_vp);
    free_matrix(point_mp); 
    free_hull(hp);
    free_hull(facet_hp);
        
    return status;
}

