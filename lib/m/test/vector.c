
/* $Id: vector.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"

/*
#define VERBOSE 1
*/

#define NUM_LOOPS      5000
#define BASE_NUM_TRIES   100

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int length;
    int count;
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;
    Vector* first_vp = NULL;
    Vector* second_vp = NULL;
    Vector* third_vp = NULL;
    Vector* fourth_vp = NULL;


    kjb_init(); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (test_factor == 0)
    {
        num_tries = 1;
    }
    else
    {
        num_tries *= test_factor;
    }

    for (count=0; count<num_tries; count++)
    {
        for (length=1; length<NUM_LOOPS; length++)
        {
            double  diff_1;
            double  diff_2;
            int     i; 


#ifdef VERBOSE
            pso("%d %d\n", count, length);
#endif 

            EPETE(get_zero_vector(&first_vp, length)); 
            EPETE(get_target_vector(&second_vp, length)); 

            for (i=0; i<second_vp->length; i++)
            {
                second_vp->elements[ i ] = 1.0; 
            }

            EPETE(multiply_vector_by_scalar(&third_vp, second_vp, 
                                            (double)count)); 

            EPETE(get_target_vector(&fourth_vp, length)); 

            EPETE(ow_set_vector(fourth_vp, (double)count));


#ifdef VERBOSE 
            db_rv(first_vp);
            db_rv(second_vp);
            db_rv(third_vp);
            db_rv(fourth_vp);
#endif 

            diff_1 = max_abs_vector_element(first_vp);

            diff_2 = max_abs_vector_difference(third_vp, fourth_vp);

            if ( ! IS_ZERO_DBL(diff_1))
            {
                p_stderr("%e != 0.0.\n", diff_1); 
                p_stderr("Problem with test one.\n");
                status = EXIT_BUG;
            }

            if ( ! IS_ZERO_DBL(diff_2) )
            {
                p_stderr("%e != 0.0.\n", diff_2); 
                p_stderr("Problem with thest two.\n");
                status = EXIT_BUG;
            }
        }
    }

    free_vector(first_vp); 
    free_vector(second_vp); 
    free_vector(third_vp); 
    free_vector(fourth_vp); 

    
    return status; 
}

