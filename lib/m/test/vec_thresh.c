
/* $Id: vec_thresh.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"

/*
#define VERBOSE 1
*/

#define NUM_LOOPS       2000
#define BASE_NUM_TRIES   100


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int length;
    int count;
    Vector* first_vp = NULL;
    Vector* second_vp = NULL;
    int  num_tries = BASE_NUM_TRIES;
    int  num_loops;
    int  test_factor = 1;


    kjb_init(); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (test_factor == 0)
    {
        num_tries = 1;
        num_loops = 10;
    }
    else
    {   
        double factor_for_linear = pow((double)test_factor, 1.0/3.0);

        num_loops = kjb_rint((double)NUM_LOOPS * factor_for_linear);
        num_tries = kjb_rint((double)BASE_NUM_TRIES * factor_for_linear);
    } 

    for (count=0; count<num_tries; count++)
    {
        for (length=1; length<num_loops; length++)
        {
            double diff;


#ifdef VERBOSE
            pso("\n-------------------------------------------------\n\n");
            pso("%d %d\n\n", count, length);
#endif 

            
            EPETE(get_random_vector(&first_vp, length)); 

#ifdef VERBOSE
            db_rv(first_vp); 
#endif 

            EPETE(min_thresh_vector(&second_vp, first_vp, 0.2)); 
            EPETE(ow_min_thresh_vector(first_vp, 0.2)); 

#ifdef VERBOSE
            db_rv(second_vp); 
            db_rv(first_vp); 
#endif 

            if (min_vector_element(first_vp) < 0.2)
            {
                p_stderr("Min vector element is less than threshold\n");
                status = EXIT_BUG;
            }

            if (min_vector_element(second_vp) < 0.2)
            {
                p_stderr("Min vector element is less than threshold\n");
                status = EXIT_BUG;
            }

            diff = max_abs_vector_difference(first_vp, second_vp); 

            if ( ! (IS_ZERO_DBL(diff)) )
            {
                p_stderr("Min thresh and ow min thresh not the same.\n"); 
                status = EXIT_BUG;
            }




            EPETE(get_random_vector(&first_vp, length)); 

#ifdef VERBOSE
            db_rv(first_vp); 
#endif 

            EPETE(max_thresh_vector(&second_vp, first_vp, 0.8)); 
            EPETE(ow_max_thresh_vector(first_vp, 0.8)); 

#ifdef VERBOSE
            db_rv(second_vp); 
            db_rv(first_vp); 
#endif 

            if (max_vector_element(first_vp) > 0.8)
            {
                p_stderr("Max vector element is less than threshold\n");
                status = EXIT_BUG;
            }

            if (max_vector_element(second_vp) > 0.8)
            {
                p_stderr("Max vector element is less than threshold\n");
                status = EXIT_BUG;
            }

            diff = max_abs_vector_difference(first_vp, second_vp); 

            if ( ! (IS_ZERO_DBL(diff)) )
            {
                p_stderr("Max thresh and ow max thresh not the same.\n"); 
                status = EXIT_BUG;
            }

        }
    }
    
    free_vector(first_vp); 
    free_vector(second_vp); 


    return status; 
}

