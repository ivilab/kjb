
/* $Id: intersect_2D_hulls.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "h/h_incl.h" 

/*
// Remember: With bad luck, the approximate hulls can have a point arbitrarily
// far away from the correct hull, by having a very small angle at one of the
// vertices.
*/
#define STEP 0.05
#define NUM_RANDOM_SAMPLES 1000
#define HULL_INTERSECTION_RESOLUTION  100
#define TOLERANCE (4.0 / (double)HULL_INTERSECTION_RESOLUTION) 
#define BASE_NUM_TRIES   1000

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int    v, i, j, k;
    double x, y;
    Queue_element* hull_list_head = NULL;
    Queue_element* cur_elem;
    Hull*          hp;
    Matrix*        point_mp  = NULL;
    int            original_intersection_result;
    int            approximation_intersection_result;
    int            dual_intersection_result;
    int            num_hulls;
    int            num_points;
    Vector*        test_vp         = NULL;
    int            inside;
    int            original_inside;
    int            approximation_inside;
    int            dual_inside;
    Hull*          original_hp      = NULL;
    Hull*          approximation_hp = NULL;
    Hull*          dual_hp          = NULL;
    int            c1, c2, c3, c4, c5, c6, c7, c8, c9;
    Vector*        min_vp = NULL;
    Vector*        max_vp = NULL;
    long           original_cpu      = 0;
    long           approximation_cpu = 0;
    long           dual_cpu          = 0;
    int            sum_c1            = 0;
    int            sum_c2            = 0;
    int            sum_c3            = 0;
    int            sum_c4            = 0;
    int            sum_c5            = 0;
    int            sum_c6            = 0;
    int            sum_c7            = 0;
    int            sum_c8            = 0;
    int            sum_c9            = 0;
    int            interior_point_count;
    double           distance;
    char           hir_str[ 100 ]; 
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


    EPETB(kjb_sprintf(hir_str, sizeof(hir_str), "%d", 
                      HULL_INTERSECTION_RESOLUTION));
    EPETB(set_random_options("seed", "0")); 
    EPETB(set_random_options("seed_2", "0")); 
    EPETB(set_debug_options("debug-level", "2")); 
    EPETB(set_hull_options("hir", hir_str)); 
    EPETB(set_heap_options("heap-checking", "f")); 
    EPETB(set_qhull_options("qhull-error-file", "qhull-error-log")); 

    if (is_interactive())
    {
        EPETB(set_verbose_options("verbose", "1")); 
    }

    EPETE(get_target_vector(&test_vp, 2)); 

    for (i=0; i<num_tries; i++)
    {
        c1 = c2 = c3 = c4 = c5 = c6 = c7 = c8 = c9 = interior_point_count = 0;

        num_hulls = 50.0 * kjb_rand() + 1;

        verbose_pso(1, "Test %d with %d hulls.\n", i+1, num_hulls);

        for (j=0; j<num_hulls; j++)
        {
            num_points = 10 + 100.0 * kjb_rand();

            EPETE(get_random_matrix(&point_mp, num_points, 2));

            NPETE(hp = find_convex_hull(point_mp, DEFAULT_HULL_OPTIONS));

            EPETE(insert_into_queue(&hull_list_head, (Queue_element**)NULL, 
                                    hp)); 
        }

        verbose_pso(2, "Doing original intersection.\n"); 
        EPETE(set_hull_options("hull-intersection-method", "original")); 
        init_cpu_time(); 
        EPE(original_intersection_result = intersect_hulls(hull_list_head, 
                                                           DEFAULT_HULL_OPTIONS,
                                                           &original_hp));
        original_cpu += get_cpu_time(); 

        if (    (original_intersection_result == NO_ERROR)
             && (original_hp == NULL)
           )
        {
            p_stderr("Original result is NO_ERROR, but result is NULL!.\n");
            original_intersection_result = ERROR;
            status = EXIT_BUG;
        }
            
        verbose_pso(2, "Doing approximation intersection.\n"); 
        EPETE(set_hull_options("hull-intersection-method", "approximation")); 
        init_cpu_time(); 
        EPE(approximation_intersection_result = intersect_hulls(hull_list_head, 
                                                          DEFAULT_HULL_OPTIONS,
                                                          &approximation_hp));
        approximation_cpu += get_cpu_time(); 

        if (    (approximation_intersection_result == NO_ERROR)
             && (approximation_hp == NULL)
           )
        {
            p_stderr("Approximation result is NO_ERROR, but result is NULL!.\n");
            approximation_intersection_result = ERROR;
            status = EXIT_BUG;
        }
            
        verbose_pso(2, "Doing dual intersection.\n"); 
        EPETE(set_hull_options("hull-intersection-method", "dual")); 
        init_cpu_time(); 
        EPE(dual_intersection_result = intersect_hulls(hull_list_head, 
                                                       DEFAULT_HULL_OPTIONS, 
                                                       &dual_hp));
        dual_cpu += get_cpu_time(); 

        if (    (dual_intersection_result == NO_ERROR)
             && (dual_hp == NULL)
           )
        {
            p_stderr("Dual result is NO_ERROR, but result is NULL!.\n");
            dual_intersection_result = ERROR;
            status = EXIT_BUG;
        }

        verbose_pso(2, "Testing intersection with dual hull vertices.\n"); 

        for (v=0; v<dual_hp->vertex_mp->num_rows; v++)
        {
            EPETE(get_matrix_row(&test_vp, dual_hp->vertex_mp, v));

            inside = TRUE; 
            cur_elem = hull_list_head; 

            while (cur_elem != NULL)
            {
                int is_inside;

                EPETE(is_inside = is_point_inside_hull((const Hull*)cur_elem->contents, test_vp));

                if ( ! is_inside)
                {
                    inside = FALSE; 
                    break; 
                }
                cur_elem = cur_elem->next;
            }

            if (inside) interior_point_count++;

            if (dual_intersection_result == NO_ERROR)
            {
                EPETE(dual_inside = is_point_inside_hull(dual_hp, test_vp));
            }
            else
            {
                dual_inside = FALSE;
            }
                
            if (original_intersection_result == NO_ERROR)
            {
                EPETE(original_inside = is_point_inside_hull(original_hp, test_vp));
            }
            else
            {
                original_inside = FALSE;
            }

            if (approximation_intersection_result == NO_ERROR)
            {
                EPETE(approximation_inside = is_point_inside_hull(approximation_hp,
                                                            test_vp));
            }
            else
            {
                approximation_inside = FALSE;
            }

            if ((inside == TRUE) && (original_intersection_result == NO_SOLUTION))
            {
                p_stderr("Interior point found but original intersection was empty!\n");
                status = EXIT_BUG;
            }
            else if ((inside == TRUE) && (original_inside == FALSE))
            {
                EPETE(get_distance_to_hull(original_hp, test_vp, &distance));
                if ((distance < -DBL_EPSILON) || (distance > TOLERANCE))
                {
                    p_stderr("A point is incorrectly outside original by %e!\n",
                         distance);
                    c1++; 
                    status = EXIT_BUG;
                }
            }

            if ((inside == FALSE) && (original_inside == TRUE))
            {
                EPETE(get_distance_to_hull(original_hp, test_vp, &distance));
                if ((distance > DBL_EPSILON) || (distance < -TOLERANCE))
                {
                    p_stderr("A point is incorrectly inside original by %e!\n",
                         -distance);
                    c2++; 
                    status = EXIT_BUG;
                }
            }

            if ((inside == TRUE) && (approximation_intersection_result == NO_SOLUTION))
            {
                p_stderr("Interior point found but approximation intersection was empty!\n");
            }
            else if ((inside == TRUE) && (approximation_inside == FALSE))
            {
                EPETE(get_distance_to_hull(approximation_hp, test_vp,
                                           &distance));
                if ((distance < -DBL_EPSILON) || (distance > TOLERANCE))
                {
                    p_stderr("A point is incorrectly outside approximation by %e!\n",
                         distance);
                    c3++; 
                    status = EXIT_BUG;
                }
            }

            if ((inside == FALSE) && (approximation_inside == TRUE))
            {
                EPETE(get_distance_to_hull(approximation_hp, test_vp,
                                           &distance));
                if ((distance > DBL_EPSILON) || (distance < -TOLERANCE))
                {
                    p_stderr("A point is incorrectly inside approximation by %e!\n",
                         -distance);
                    c4++; 
                    status = EXIT_BUG;
                }
            }

            if ((inside == TRUE) && (dual_intersection_result == NO_SOLUTION))
            {
                p_stderr("Interior point found but dual intersection was empty!\n");
            }
            else if ((inside == TRUE) && (dual_inside == FALSE))
            {
                EPETE(get_distance_to_hull(dual_hp, test_vp, &distance));

                if ((distance < -DBL_EPSILON) || (distance > TOLERANCE))
                {
                    p_stderr("A point is incorrectly outside dual by %e!\n",
                         distance);
                    c5++;
                    status = EXIT_BUG;
                }
            }

            if ((inside == FALSE) && (dual_inside == TRUE))
            {
                EPETE(get_distance_to_hull(dual_hp, test_vp, &distance));

                if ((distance > DBL_EPSILON) || (distance < -TOLERANCE))
                {
                    p_stderr("A point is incorrectly inside dual by %e!\n",
                         -distance);
                    c6++; 
                    status = EXIT_BUG;
                }
            }

            if (approximation_inside != original_inside)
            {
                c7++;
            }

            if (dual_inside != original_inside)
            {
                c8++;
            }

            if (dual_inside != approximation_inside)
            {
                c9++;
            }
        }

        verbose_pso(2, "Testing intersection with coarse grid.\n"); 

        for (x = 0.0; x <= 1.0; x += STEP)
        {
            for (y = 0.0; y <= 1.0; y += STEP)
            {
                test_vp->elements[ 0 ] = x;
                test_vp->elements[ 1 ] = y;

                inside = TRUE; 
                cur_elem = hull_list_head; 

                while (cur_elem != NULL)
                {
                    int is_inside;

                    EPETE(is_inside = is_point_inside_hull((const Hull*)cur_elem->contents, 
                                                     test_vp));

                    if ( ! is_inside)
                    {
                        inside = FALSE; 
                        break; 
                    }
                    cur_elem = cur_elem->next;
                }

                if (inside) interior_point_count++;

                if (dual_intersection_result == NO_ERROR)
                {
                    EPETE(dual_inside = is_point_inside_hull(dual_hp, test_vp));
                }
                else
                {
                    dual_inside = FALSE;
                }
                    
                if (original_intersection_result == NO_ERROR)
                {
                    EPETE(original_inside = is_point_inside_hull(original_hp,
                                                           test_vp));
                }
                else
                {
                    original_inside = FALSE;
                }

                if (approximation_intersection_result == NO_ERROR)
                {
                    EPETE(approximation_inside = is_point_inside_hull(
                                                            approximation_hp,
                                                            test_vp));
                }
                else
                {
                    approximation_inside = FALSE;
                }

                if ((inside == TRUE) && (original_intersection_result == NO_SOLUTION))
                {
                    p_stderr("Interior point found but original intersection was empty!\n");
                    status = EXIT_BUG;
                }
                else if ((inside == TRUE) && (original_inside == FALSE))
                {
                    EPETE(get_distance_to_hull(original_hp, test_vp,
                                               &distance));

                    if ((distance < -DBL_EPSILON) || (distance > TOLERANCE))
                    {
                        p_stderr("A point is incorrectly outside original by %e!\n",
                             distance);
                        c1++; 
                        status = EXIT_BUG;
                    }
                }

                if ((inside == FALSE) && (original_inside == TRUE))
                {
                    EPETE(get_distance_to_hull(original_hp, test_vp, 
                                               &distance));
                    if ((distance > DBL_EPSILON) || (distance < -TOLERANCE))
                    {
                        p_stderr("A point is incorrectly inside original by %e!\n",
                             -distance);
                        c2++; 
                        status = EXIT_BUG;
                    }
                }

                if ((inside == TRUE) && (approximation_intersection_result == NO_SOLUTION))
                {
                    p_stderr("Interior point found but approximation intersection was empty!\n");
                }
                else if ((inside == TRUE) && (approximation_inside == FALSE))
                {
                    EPETE(get_distance_to_hull(approximation_hp, test_vp,
                                               &distance));
                    if ((distance < -DBL_EPSILON) || (distance > TOLERANCE))
                    {
                        p_stderr("A point is incorrectly outside approximation by %e!\n",
                             distance);
                        c3++; 
                        status = EXIT_BUG;
                    }
                }

                if ((inside == FALSE) && (approximation_inside == TRUE))
                {
                    EPETE(get_distance_to_hull(approximation_hp, test_vp,
                                               &distance));
                    if ((distance > DBL_EPSILON) || (distance < -TOLERANCE))
                    {
                        p_stderr("A point is incorrectly inside approximation by %e!\n",
                             -distance);
                        c4++; 
                        status = EXIT_BUG;
                    }
                }

                if ((inside == TRUE) && (dual_intersection_result == NO_SOLUTION))
                {
                    p_stderr("Interior point found but dual intersection was empty!\n");
                }
                else if ((inside == TRUE) && (dual_inside == FALSE))
                {
                    EPETE(get_distance_to_hull(dual_hp, test_vp, &distance));

                    if ((distance < -DBL_EPSILON) || (distance > TOLERANCE))
                    {
                        p_stderr("A point is incorrectly outside dual by %e!\n",
                             distance);
                        c5++;
                        status = EXIT_BUG;
                    }
                }

                if ((inside == FALSE) && (dual_inside == TRUE))
                {
                    EPETE(get_distance_to_hull(dual_hp, test_vp, &distance));

                    if ((distance > DBL_EPSILON) || (distance < -TOLERANCE))
                    {
                        p_stderr("A point is incorrectly inside dual by %e!\n",
                             -distance);
                        c6++; 
                        status = EXIT_BUG;
                    }
                }

                if (approximation_inside != original_inside)
                {
                    c7++;
                }

                if (dual_inside != original_inside)
                {
                    c8++;
                }

                if (dual_inside != approximation_inside)
                {
                    c9++;
                }
            }
        }

        verbose_pso(2, "Testing intersection with random samples.\n"); 

        for (k=0; k<NUM_RANDOM_SAMPLES; k++)
        {
            test_vp->elements[ 0 ] = kjb_rand();
            test_vp->elements[ 1 ] = kjb_rand();

            inside = TRUE; 
            cur_elem = hull_list_head; 

            while (cur_elem != NULL)
            {
                int is_inside;

                EPETE(is_inside = is_point_inside_hull((const Hull*)cur_elem->contents, test_vp));

                if ( ! is_inside)
                {
                    inside = FALSE; 
                    break; 
                }
                cur_elem = cur_elem->next;
            }

            if (inside) interior_point_count++;

            if (dual_intersection_result == NO_ERROR)
            {
                EPETE(dual_inside = is_point_inside_hull(dual_hp, test_vp));
            }
            else
            {
                dual_inside = FALSE;
            }
                
            if (original_intersection_result == NO_ERROR)
            {
                EPETE(original_inside = is_point_inside_hull(original_hp, test_vp));
            }
            else
            {
                original_inside = FALSE;
            }

            if (approximation_intersection_result == NO_ERROR)
            {
                EPETE(approximation_inside = is_point_inside_hull(approximation_hp,
                                                            test_vp));
            }
            else
            {
                approximation_inside = FALSE;
            }

            if ((inside == TRUE) && (original_intersection_result == NO_SOLUTION))
            {
                p_stderr("Interior point found but original intersection was empty!\n");
                status = EXIT_BUG;
            }
            else if ((inside == TRUE) && (original_inside == FALSE))
            {
                EPETE(get_distance_to_hull(original_hp, test_vp, &distance));
                if ((distance < -DBL_EPSILON) || (distance > TOLERANCE))
                {
                    p_stderr("A point is incorrectly outside original by %e!\n",
                         distance);
                    c1++; 
                    status = EXIT_BUG;
                }
            }

            if ((inside == FALSE) && (original_inside == TRUE))
            {
                EPETE(get_distance_to_hull(original_hp, test_vp, &distance));
                if ((distance > DBL_EPSILON) || (distance < -TOLERANCE))
                {
                    p_stderr("A point is incorrectly inside original by %e!\n",
                         -distance);
                    c2++; 
                    status = EXIT_BUG;
                }
            }

            if ((inside == TRUE) && (approximation_intersection_result == NO_SOLUTION))
            {
                p_stderr("Interior point found but approximation intersection was empty!\n");
            }
            else if ((inside == TRUE) && (approximation_inside == FALSE))
            {
                EPETE(get_distance_to_hull(approximation_hp, test_vp,
                                           &distance));
                if ((distance < -DBL_EPSILON) || (distance > TOLERANCE))
                {
                    p_stderr("A point is incorrectly outside approximation by %e!\n",
                         distance);
                    c3++; 
                    status = EXIT_BUG;
                }
            }

            if ((inside == FALSE) && (approximation_inside == TRUE))
            {
                EPETE(get_distance_to_hull(approximation_hp, test_vp,
                                           &distance));
                if ((distance > DBL_EPSILON) || (distance < -TOLERANCE))
                {
                    p_stderr("A point is incorrectly inside approximation by %e!\n",
                         -distance);
                    c4++; 
                    status = EXIT_BUG;
                }
            }

            if ((inside == TRUE) && (dual_intersection_result == NO_SOLUTION))
            {
                p_stderr("Interior point found but dual intersection was empty!\n");
            }
            else if ((inside == TRUE) && (dual_inside == FALSE))
            {
                EPETE(get_distance_to_hull(dual_hp, test_vp, &distance));

                if ((distance < -DBL_EPSILON) || (distance > TOLERANCE))
                {
                    p_stderr("A point is incorrectly outside dual by %e!\n",
                         distance);
                    c5++; 
                    status = EXIT_BUG;
                }
            }

            if ((inside == FALSE) && (dual_inside == TRUE))
            {
                EPETE(get_distance_to_hull(dual_hp, test_vp, &distance));

                if ((distance > DBL_EPSILON) || (distance < -TOLERANCE))
                {
                    p_stderr("A point is incorrectly inside dual by %e!\n",
                         -distance);
                    c6++; 
                }
            }

            if (approximation_inside != original_inside)
            {
                c7++;
            }

            if (dual_inside != original_inside)
            {
                c8++;
            }

            if (dual_inside != approximation_inside)
            {
                c9++;
            }
        }
        
        verbose_pso(1, "%d interior points.\n", interior_point_count);

        if (c1 > 15)
        {
            verbose_pso(1, "%d points for which original is incorrectly outside by at least %3e!\n",
                 c1, TOLERANCE);
        }

        if (c2 > 15)
        {
            verbose_pso(1, "%d points for which original is incorrectly inside by at least %3e!\n",
                 c2, TOLERANCE);
        }

        if (c3 > 15)
        {
            verbose_pso(1, "%d points for which approximation is incorrectly outside by at least %3e!\n",
                 c3, TOLERANCE);
        }

        if (c4 > 15)
        {
            verbose_pso(1, "%d points for which approximation is incorrectly inside by at least %3e!\n",
                 c4, TOLERANCE);
        }

        if (c5 > 15)
        {
            verbose_pso(1, "%d points for which dual is incorrectly outside by at least %3e!\n",
                 c5, TOLERANCE);
        }

        if (c6 > 15)
        {
            verbose_pso(1, "%d points for which dual is incorrectly inside by at least %3e!\n",
                 c6, TOLERANCE);
        }

        if (c7 > 15)
        { 
            verbose_pso(1, "%d points where approximation and original disagree.\n", c7);
        }

        if (c8 > 15)
        {
            verbose_pso(1, "%d points where dual and original disagree.\n", c8);
        }

        if (c9 > 15)
        {
            verbose_pso(1, "%d points where dual and approximation disagree.\n", c9);
        }
        
        free_queue(&hull_list_head, (Queue_element**)NULL, 
                   (void(*)(void*))free_hull); 

        sum_c1 += c1; 
        sum_c2 += c2; 
        sum_c3 += c3; 
        sum_c4 += c4; 
        sum_c5 += c5; 
        sum_c6 += c6; 
        sum_c7 += c7; 
        sum_c8 += c8; 
        sum_c9 += c9; 
    }

    verbose_pso(1, "\n%d points for which original is incorrectly outside by at least %.3e.\n",
                sum_c1, TOLERANCE);
    verbose_pso(1, "%d points for which original is incorrectly inside by at least %.3e.\n",
                sum_c2, TOLERANCE);
    verbose_pso(1, "%d points for which approximation is incorrectly outside by at least %.3e.\n",
                sum_c3, TOLERANCE);
    verbose_pso(1, "%d points for which approximation is incorrectly inside by at least %.3e.\n",
                sum_c4, TOLERANCE);
    verbose_pso(1, "%d points for which dual is incorrectly outside by at least %.3e.\n",
                sum_c5, TOLERANCE);
    verbose_pso(1, "%d points for which dual is incorrectly inside by at least %.3e.\n",
                sum_c6, TOLERANCE);

    verbose_pso(1, "\n%d points where approximation and original disagree.\n", sum_c7);
    verbose_pso(1, "%d points where dual and original disagree.\n", sum_c8);
    verbose_pso(1, "%d points where dual and approximation disagree.\n", sum_c9);

    verbose_pso(1, "\nOriginal    CPU time is %ld milli-seconds.\n", original_cpu);
    verbose_pso(1, "Approximation CPU time is %ld milli-seconds.\n", approximation_cpu);
    verbose_pso(1, "Dual          CPU time is %ld milli-seconds.\n", dual_cpu);

    free_hull(dual_hp);
    free_hull(original_hp); 
    free_hull(approximation_hp); 
    free_vector(test_vp);
    free_vector(max_vp);
    free_vector(min_vp);
    free_matrix(point_mp); 
        
    return status;
}

