
/* $Id: point_inside_2D_hulls.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "h/h_incl.h" 

#define BASE_NUM_TRIES   1000

static void call_free_hull (void* ptr);


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int i, j;
    Queue_element* hull_list_head = NULL;
    Queue_element* cur_elem;
    Hull*          hp;
    Hull*          dummy_hp  = NULL;
    Matrix*        point_mp  = NULL;
    Vector*        result_vp = NULL;
    int            old_intersection_result;
    int            num_hulls;
    int            num_points;
    Matrix*        dummy_normal_mp     = NULL;
    Vector*        dummy_constraint_vp = NULL;
    int            num_tries           = BASE_NUM_TRIES;
    int            test_factor         = 1;


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

    EPETB(set_hull_options("hir", "300")); 
    EPETB(set_qhull_options("qhull-error-file", "qhull-error-log")); 

    if (is_interactive())
    {
        EPETB(set_verbose_options("verbose", "1")); 
    }

    for (i=0; i<num_tries; i++)
    {
        num_hulls = 50.0 * kjb_rand() + 1;

        verbose_pso(1, "Test %d with %d hulls.\n", i + 1, num_hulls); 

        for (j=0; j<num_hulls; j++)
        {
            num_points = 5 + 50.0 * kjb_rand();

            EPETE(get_random_matrix(&point_mp, num_points, 2));

            NPETE(hp = find_convex_hull(point_mp, DEFAULT_HULL_OPTIONS));

            EPETE(insert_into_queue(&hull_list_head, (Queue_element**)NULL, 
                                    hp)); 
        }


        verbose_pso(2, "Doing old intersection.\n"); 
        EPE(old_intersection_result = intersect_hulls(hull_list_head, 
                                                      DEFAULT_HULL_OPTIONS, 
                                                      &dummy_hp));

        verbose_pso(2, "Finding point.\n"); 
        if (find_point_in_2D_hull_intersection(hull_list_head, 100,
                                               &result_vp, &dummy_normal_mp,
                                               &dummy_constraint_vp))
        {
            if (old_intersection_result == NO_SOLUTION) 
            {
                p_stderr("Old intersection routine failed, but we found a point in the intersection!\n"); 
                status = EXIT_BUG;
            }
            cur_elem = hull_list_head; 

            while (cur_elem != NULL)
            {
                if ( ! is_point_inside_hull((const Hull*)cur_elem->contents, result_vp))
                {
                    p_stderr("Interior point is reported to be outside one of the hulls!\n");
                    status = EXIT_BUG;
                }
                cur_elem = cur_elem->next;
            }
        }
        else 
        {
            if (old_intersection_result == NO_ERROR) 
            {
                p_stderr("Old intersection routine succeeded, but no intesection point was found!\n"); 
                status = EXIT_BUG;
            }
        }

        /* pso("Freeing hulls.\n"); */

        free_queue(&hull_list_head, (Queue_element**)NULL, 
                   call_free_hull); 
    }

    free_hull(dummy_hp); 
    free_vector(result_vp);
    free_vector(dummy_constraint_vp);
    free_matrix(point_mp); 
    free_matrix(dummy_normal_mp); 

        
    return status;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void call_free_hull(void* ptr)
{
    free_hull((Hull*)ptr);
}

