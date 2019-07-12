
/* $Id: fill_3D_hull.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "h/h_incl.h" 

#define STEP 0.03
#define NUM_RANDOM_SAMPLES 500
#define HULL_FILL_RESOLUTION  100
#define TOLERANCE (4.0 / (double)HULL_FILL_RESOLUTION) 
#define BASE_NUM_TRIES   200

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int     i, k;
    double  x, y, z;
    Matrix* point_mp    = NULL;
    int     num_points;
    Vector* test_vp     = NULL;
    int     fill_inside;
    int     no_fill_inside;
    Hull*   fill_hp        = NULL;
    Hull*   no_fill_hp     = NULL;
    int     c1, c2;
    long    fill_cpu       = 0;
    long    no_fill_cpu    = 0;
    int     sum_c1         = 0;
    int     sum_c2         = 0;
    double    distance;
    char    hfr_str[ 100 ];
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


    EPETB(kjb_sprintf(hfr_str, sizeof(hfr_str), "%d", HULL_FILL_RESOLUTION));
    EPETB(set_hull_options("hfr", hfr_str)); 
    EPETB(set_random_options("seed", "0")); 
    EPETB(set_random_options("seed_2", "0")); 
    EPETB(set_debug_options("debug-level", "2")); 
    EPETB(set_heap_options("heap-checking", "f")); 
    EPETB(set_qhull_options("qhull-error-file", "qhull-error-log")); 

    EPETE(get_target_vector(&test_vp, 3)); 

    if (is_interactive())
    {
        EPETB(set_verbose_options("verbose", "1")); 
    }

    for (i=0; i<num_tries; i++)
    {
        c1 = c2 = 0;

        num_points = kjb_rint(30.0 + 500.0 * kjb_rand());

        verbose_pso(1, "Test %d (%d points).\n", i+1, num_points);

        EPETE(get_random_matrix(&point_mp, num_points, 3));

        NPETE(no_fill_hp = find_convex_hull(point_mp, DEFAULT_HULL_OPTIONS));
        EPETE(copy_hull(&fill_hp, no_fill_hp)); 
        EPETE(fill_hull(fill_hp)); 

        verbose_pso(3, "Testing with coarse grid.\n");

        for (x = 0.0; x <= 1.0; x += STEP)
        {
            for (y = 0.0; y <= 1.0; y += STEP)
            {
                for (z = 0.0; z <= 1.0; z += STEP)
                {
                    test_vp->elements[ 0 ] = x;
                    test_vp->elements[ 1 ] = y;
                    test_vp->elements[ 2 ] = z;


                    init_cpu_time(); 
                    EPETE(no_fill_inside = is_point_inside_hull(no_fill_hp, test_vp));
                    no_fill_cpu += get_cpu_time(); 

                    init_cpu_time(); 
                    EPETE(fill_inside = is_point_inside_hull(fill_hp, test_vp));
                    fill_cpu += get_cpu_time(); 

                    if ((no_fill_inside == TRUE) && (fill_inside == FALSE))
                    {
                        EPETE(get_distance_to_hull(no_fill_hp, test_vp, &distance));

                        if ((distance > DBL_EPSILON) || (distance < -TOLERANCE))
                        {
                            p_stderr("A grid point inside by %e (TOL = %e) is outside when filled hull is used!\n",
                                     -distance, TOLERANCE);
                            c1++; 
                        }
                    }

                    if ((no_fill_inside == FALSE) && (fill_inside == TRUE))
                    {
                        EPETE(get_distance_to_hull(no_fill_hp, test_vp, &distance));

                        if ((distance < -DBL_EPSILON) || (distance > TOLERANCE))
                        {
                            p_stderr("A grid point outside by %e (TOL = %e) is inside when filled hull is used!\n",
                                     distance, TOLERANCE);
                            c2++; 
                        }
                    }
                }
            }
        }

        verbose_pso(3, "Testing with random samples.\n"); 

        for (k=0; k<NUM_RANDOM_SAMPLES; k++)
        {
            test_vp->elements[ 0 ] = kjb_rand();
            test_vp->elements[ 1 ] = kjb_rand();
            test_vp->elements[ 2 ] = kjb_rand();

            init_cpu_time(); 
            EPETE(no_fill_inside = is_point_inside_hull(no_fill_hp, test_vp));
            no_fill_cpu += get_cpu_time(); 

            init_cpu_time(); 
            EPETE(fill_inside = is_point_inside_hull(fill_hp, test_vp));
            fill_cpu += get_cpu_time(); 

            if ((no_fill_inside == TRUE) && (fill_inside == FALSE))
            {
                EPETE(get_distance_to_hull(no_fill_hp, test_vp, &distance));

                if ((distance > DBL_EPSILON) || (distance < -TOLERANCE))
                {
                    p_stderr("A random point inside by %e (TOL = %e) is outside when filled hull is used!\n",
                         -distance, TOLERANCE);
                    c1++; 
                }
            }

            if ((no_fill_inside == FALSE) && (fill_inside == TRUE))
            {
                EPETE(get_distance_to_hull(no_fill_hp, test_vp, &distance));

                if ((distance < -DBL_EPSILON) || (distance > TOLERANCE))
                {
                    p_stderr("A random point outside by %e (TOL = %e) is inside when filled hull is used!\n",
                         distance, TOLERANCE);
                    c2++; 
                }
            }
        }
        
        sum_c1 += c1; 
        sum_c2 += c2; 

        kjb_flush(); 

        free_hull(no_fill_hp);
    }

    if (sum_c1 > 0)
    {
        p_stderr("\n%d points outside by at least %e are inside using fill.\n",
             sum_c1, TOLERANCE);
        status = EXIT_BUG;
    }

    if (sum_c2 > 0)
    {
        p_stderr("%d points inside by at least %e are outside using fill.\n",
             sum_c2, TOLERANCE);
        status = EXIT_BUG;
    }

    if (is_interactive())
    {
        pso("No fill CPU time is %ld seconds.\n", no_fill_cpu / 1000);
        pso("Fill    CPU time is %ld seconds.\n", fill_cpu / 1000);
    }

    free_hull(fill_hp);
    free_vector(test_vp);
    free_matrix(point_mp); 
        
    return status;
}

