
/* $Id: dual_intersect_2D_hulls.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "h/h_incl.h" 

#define STEP 0.02
#define NUM_RANDOM_SAMPLES 10000
#define BASE_NUM_TRIES     200

#define DISTANCE_TO_INTERSECTION_TOL  1e-5

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int    i, j, k, v;
    double x, y;
    Queue_element* hull_list_head = NULL;
    Queue_element* cur_elem;
    Hull*          hp;
    Matrix*        point_mp  = NULL;
    int            dual_intersection_result;
    int            num_hulls;
    int            num_points;
    Vector*        test_vp         = NULL;
    int            inside;
    int            dual_inside;
    Hull*          dual_hp         = NULL;
    Vector*        min_vp          = NULL;
    Vector*        max_vp          = NULL;
    int            sum_error_count = 0;
    int            vertex_error_count;
    int            grid_error_count;
    int            random_error_count;
    double max_distance_from_intersection;
    double distance_from_intersection;
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

    EPETE(set_random_options("seed", "0")); 
    EPETE(set_random_options("seed_2", "0")); 
    EPETE(set_debug_options("debug-level", "2")); 
    EPETE(set_hull_options("hir", "300")); 
    EPETE(set_heap_options("heap-checking", "f")); 
    EPETE(set_qhull_options("qhull-error-file", "qhull-error-log")); 
    EPETE(get_target_vector(&test_vp, 2)); 

    if (is_interactive())
    {
        EPETE(set_verbose_options("verbose", "1")); 
    }

    EPETE(set_random_options("seed", "505229549:116")); 

    for (i=0; i<num_tries; i++) 
    {
        /* EPETE(set_random_options("seed_2", "0"));  */

        num_hulls = 50.0 * kjb_rand() + 1;

        verbose_pso(1, "Test %d with %d hulls.\n", i+1, num_hulls); 

        for (j=0; j<num_hulls; j++)
        {
            num_points = 7 + 1000.0 * kjb_rand();

            EPETE(get_random_matrix(&point_mp, num_points, 2));

            NPETE(hp = find_convex_hull(point_mp, DEFAULT_HULL_OPTIONS));

            EPETE(insert_into_queue(&hull_list_head, (Queue_element**)NULL, 
                                    hp)); 
        }

        EPETE(set_hull_options("hull-intersection-method", "dual")); 
        init_cpu_time(); 
        dual_intersection_result = intersect_hulls(hull_list_head, 
                                                   DEFAULT_HULL_OPTIONS, 
                                                   &dual_hp);

        if (    (dual_intersection_result == NO_ERROR)
             && (dual_hp == NULL)
           )
        {
            p_stderr("Dual result is NO_ERROR, but result is NULL!.\n");
            dual_intersection_result = ERROR;
        }

        if (dual_intersection_result == ERROR)
        {
            kjb_print_error();
        }
        else if (dual_intersection_result == NO_SOLUTION)
        {
            verbose_pso(2, "Dual intersection result is NO_SOLUTION.\n");
        }
        else 
        {
            verbose_pso(3, "Testing intersection with vertices.\n"); 

            vertex_error_count = 0;

            for (v=0; v<dual_hp->vertex_mp->num_rows; v++)
            {
                max_distance_from_intersection = 0.0;

                EPETE(get_matrix_row(&test_vp, dual_hp->vertex_mp, v));

                inside = TRUE; 
                cur_elem = hull_list_head; 

                while (cur_elem != NULL)
                {
                    int is_inside;

                    EPETE(is_inside = is_point_inside_hull((const Hull*)cur_elem->contents, test_vp));

                    if ( ! is_inside)
                    {
                        EPETE(get_distance_to_hull((const Hull*)cur_elem->contents, test_vp, 
                                                   &distance_from_intersection));
                        if (distance_from_intersection > DISTANCE_TO_INTERSECTION_TOL)
                        {
                            if (distance_from_intersection > max_distance_from_intersection)
                            {
                                max_distance_from_intersection = distance_from_intersection;
                            }

                            inside = FALSE; 
                            break; 
                        }
                    }
                    cur_elem = cur_elem->next;
                }

                if (dual_intersection_result == NO_ERROR)
                {
                    EPETE(dual_inside = is_point_inside_hull(dual_hp, test_vp));
                }
                else
                {
                    dual_inside = FALSE;
                }
                    
                if ( ! inside)
                {
                    p_stderr("Max distance of vertex to hulls is %e!\n",
                        max_distance_from_intersection); 
                }

                if ( ! dual_inside)
                {
                    EPETE(get_distance_to_hull(dual_hp, test_vp, 
                                               &distance_from_intersection));
                    if (distance_from_intersection < DISTANCE_TO_INTERSECTION_TOL)
                    {
                        dual_inside = TRUE;
                    }
                    else 
                    {
                        p_stderr("Distance of vertex to intersection is %e!\n",
                            distance_from_intersection); 
                    }
                }

                if (( ! inside) || ( ! dual_inside))
                {
                    vertex_error_count++;
                }
            }

            if (vertex_error_count > 0)
            {
                sum_error_count += vertex_error_count; 
                
                p_stderr("%d vertex points out of %d where dual is wrong!\n", 
                    vertex_error_count, dual_hp->vertex_mp->num_rows);
            }
        }
        
        verbose_pso(3, "Testing intersection with coarse grid.\n"); 

        grid_error_count = 0;

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
                    int inside_hull;

                    EPETE(inside_hull = is_point_inside_hull((const Hull*)cur_elem->contents, 
                                                              test_vp));

                    if ( ! inside_hull)
                    {
                        inside = FALSE; 
                        break; 
                    }
                    cur_elem = cur_elem->next;
                }

                if (dual_intersection_result == NO_ERROR)
                {
                    EPETE(dual_inside = is_point_inside_hull(dual_hp, test_vp));
                }
                else
                {
                    dual_inside = FALSE;
                }
                    
                if (inside != dual_inside)
                {
                    grid_error_count++;

                    EPETE(get_distance_to_hull(dual_hp, test_vp, 
                                               &distance_from_intersection));
                    if (distance_from_intersection < DISTANCE_TO_INTERSECTION_TOL)
                    {
                        p_stderr("Distance of problem grid point to intersection is %e!\n",
                            distance_from_intersection); 
                    }
                }
            }
        }

        if (grid_error_count > 0)
        {
            sum_error_count += grid_error_count; 
            p_stderr("%d grid points where dual is wrong!\n", grid_error_count);
        }
        
        verbose_pso(3, "Testing intersection with random samples.\n"); 

        random_error_count = 0;

        for (k=0; k<NUM_RANDOM_SAMPLES; k++)
        {
            test_vp->elements[ 0 ] = kjb_rand();
            test_vp->elements[ 1 ] = kjb_rand();

            inside = TRUE; 
            cur_elem = hull_list_head; 

            while (cur_elem != NULL)
            {
                if ( ! is_point_inside_hull((const Hull*)cur_elem->contents, test_vp))
                {
                    inside = FALSE; 
                    break; 
                }
                cur_elem = cur_elem->next;
            }

            if (dual_intersection_result == NO_ERROR)
            {
                EPETE(dual_inside = is_point_inside_hull(dual_hp, test_vp));
            }
            else
            {
                dual_inside = FALSE;
            }
                
            if (inside != dual_inside)
            {
                random_error_count++;

                EPETE(get_distance_to_hull(dual_hp, test_vp, 
                                           &distance_from_intersection));
                if (distance_from_intersection < DISTANCE_TO_INTERSECTION_TOL)
                {
                    p_stderr("Distance of problem random point to intersection is %e!\n",
                        distance_from_intersection); 
                }
            }
        }
        
        if (random_error_count > 0)
        {
            sum_error_count += random_error_count; 
            p_stderr("%d random points where dual is wrong!\n", random_error_count);
        }
        
        free_queue(&hull_list_head, (Queue_element**)NULL, 
                   (void(*)(void*))free_hull); 

        /*
        EPETE(set_random_options("seed", "")); 
        */
    }

    if (sum_error_count > 0)
    {
        p_stderr("%d points where dual is wrong!\n", sum_error_count);
        status = EXIT_BUG;
    }

    free_hull(dual_hp);
    free_vector(test_vp);
    free_vector(max_vp);
    free_vector(min_vp);
    free_matrix(point_mp); 
        
    return status; 
}

