
/* $Id: vec_arith_ow_ew.c 21491 2017-07-20 13:19:02Z kobus $ */



#include "m/m_incl.h"

/*
#define VERBOSE 1
*/


static void test_bug_handler(const char* mess);


#define NUM_LOOPS       1000
#define BASE_NUM_TRIES   100


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int length;
    int count;
    Vector* a_vp = NULL; 
    Vector* b_vp = NULL; 
    Vector* c_vp = NULL; 
    Vector* d_vp = NULL; 
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;
    Vector* first_vp = NULL;
    Vector* second_vp = NULL;


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



    set_bug_handler(test_bug_handler); 

    for (count=0; count<num_tries; count++)
    {
        for (length=1; length<NUM_LOOPS; length++)
        {
            int     factor; 
            int     i; 
            int     result; 


#ifdef VERBOSE
            pso("%d %d %d\n", count, length, count+2);
#endif 

            EPETE(get_random_vector(&first_vp, length)); 
            EPETE(get_target_vector(&second_vp, count+2)); 

            for (i=0; i<second_vp->length; i++)
            {
                second_vp->elements[ i ] = 2;
            }

            EPETE(copy_vector(&a_vp, first_vp)); 
            result = ow_multiply_vectors(a_vp, second_vp);

            factor = length / (count+2);

            if (length == (factor * (count+2)))
            {
                double  diff;
                double  diff_2;
                double  diff_3;

#ifdef VERBOSE 
                db_rv(first_vp);
                db_rv(second_vp);
                dbp("Times two"); 
                db_rv(a_vp);
#endif 

                EPETE(copy_vector(&b_vp, first_vp)); 
                EPETE(ow_add_vectors(b_vp, first_vp));


#ifdef VERBOSE   
                dbp("Added to myself (should also be times two)"); 
                db_rv(b_vp); 
#endif 

                diff = max_abs_vector_difference(a_vp, b_vp);

                EPETE(copy_vector(&c_vp, b_vp)); 
                EPETE(ow_subtract_vectors(c_vp, first_vp));


#ifdef VERBOSE   
                dbp("Subtract original from added (should be original)"); 
                db_rv(c_vp);
#endif 

                diff_2 = max_abs_vector_difference(c_vp, first_vp);

                EPETE(copy_vector(&d_vp, b_vp)); 
                EPETE(ow_divide_vectors(d_vp, second_vp));

#ifdef VERBOSE   
                dbp("Divde two times by two (should be original)"); 
                db_rv(d_vp); 
#endif 

                diff_3 = max_abs_vector_difference(d_vp, first_vp); 

                if ( ! IS_ZERO_DBL(diff))
                {
                    p_stderr("(%d %d)  %e != 0.0.\n", count, length, 
                             diff); 
                    p_stderr("Problem comparing add with mult times 2.\n");
                    status = EXIT_BUG;
                }

                if ( ! IS_ZERO_DBL(diff_2) )
                {
                    p_stderr("(%d %d)  %e != 0.0.\n", count, length, 
                             diff_2); 
                    p_stderr("Problem with add then subtract being no-op.\n");
                    status = EXIT_BUG;
                }
                if ( ! IS_ZERO_DBL(diff_3) )
                {
                    p_stderr("(%d %d)  %e != 0.0.\n", count, length, 
                             diff_3); 
                    p_stderr("Problem with times then divide being no-op.\n");
                    status = EXIT_BUG;
                }
            }
            else if (result != ERROR)
            {
                p_stderr("Problem rejecting dimensions %d and %d.\n",
                        length, count+2); 
                status = EXIT_BUG;
            }

        }
    }
    
    free_vector(first_vp);
    free_vector(second_vp);
    free_vector(a_vp); 
    free_vector(b_vp);
    free_vector(c_vp);
    free_vector(d_vp);

    return status; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void test_bug_handler(const char* mess)
{
    /*
    kjb_puts(mess); 
    kjb_puts("\n"); 
    */
    set_error(mess);
}


