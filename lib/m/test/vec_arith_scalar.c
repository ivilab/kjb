
/* $Id: vec_arith_scalar.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"


static void test_bug_handler(const char* mess);




/*
#define VERBOSE 1
*/


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
    Vector* e_vp = NULL;
    Vector* f_vp = NULL;
    Vector* g_vp = NULL;
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
        /*
         * This just checks that we get the same answer when there are NO
         * missing values. Currently we do not check the answer when there are
         * missing values.
        */
        
        if (IS_EVEN(count))
        {
            enable_respect_missing_values();
        }
        else 
        {
            disable_respect_missing_values();
        }

#ifdef VERBOSE
        dbi(respect_missing_values());
#endif 

        for (length=1; length<NUM_LOOPS; length++)
        {
            double  diff[ 6 ];
            int     diff_count = 0;
            size_t  cur_diff; 
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

            result = multiply_vectors(&a_vp, first_vp, second_vp);

            factor = length / (count+2);

            if (length == (factor * (count+2)))
            {
#ifdef VERBOSE 
                db_rv(first_vp);
                db_rv(second_vp);
                dbp("Two times first"); 
                db_rv(a_vp);
#endif 

                EPETE(divide_vector_by_scalar(&b_vp, a_vp, 2.0));

#ifdef VERBOSE   
                dbp("First"); 
                db_rv(b_vp); 
#endif 

                diff[ diff_count++] = max_abs_vector_difference(first_vp, b_vp);
#ifdef VERBOSE
                dbi(diff_count);
                dbf(diff[ diff_count - 1 ]);
#endif 

                EPETE(ow_multiply_vector_by_scalar(b_vp, 10.0));

#ifdef VERBOSE   
                dbp("10 times First"); 
                db_rv(b_vp);
#endif 

                EPETE(ow_divide_vector_by_scalar(b_vp, 10.0));
                      
#ifdef VERBOSE   
                dbp("First"); 
                db_rv(b_vp);
#endif 

                diff[ diff_count++] = max_abs_vector_difference(first_vp, b_vp);
#ifdef VERBOSE
                dbi(diff_count);
                dbf(diff[ diff_count - 1 ]);
#endif 



                if (divide_vector_by_scalar(&c_vp, first_vp, 0.0) != ERROR)
                {
                    p_stderr("Divide by zero not caught\n");
                    status = EXIT_BUG;
                }
#ifdef VERBOSE
                else
                {
                    kjb_print_error();
                }
#endif 

                if (ow_divide_vector_by_scalar(first_vp, 0.0) != ERROR)
                {
                    p_stderr("Divide by zero not caught (ow)\n");
                    status = EXIT_BUG;
                }
#ifdef VERBOSE
                else
                {
                    kjb_print_error();
                }
#endif 



                EPETE(multiply_vector_by_scalar(&d_vp, first_vp, 3.0));

#ifdef VERBOSE
                dbp("Three times first"); 
                db_rv(d_vp);
#endif 

                EPETE(ow_divide_vector_by_scalar(d_vp, 3.0));

#ifdef VERBOSE   
                dbp("Equal first"); 
                db_rv(d_vp); 
#endif 

                diff[ diff_count++] = max_abs_vector_difference(d_vp, first_vp);
#ifdef VERBOSE
                dbi(diff_count);
                dbf(diff[ diff_count - 1 ]);
#endif 

                result = add_vectors(&e_vp, first_vp, second_vp);
#ifdef VERBOSE 
                db_rv(first_vp);
                db_rv(second_vp);
                dbp("Two plus first"); 
                db_rv(e_vp);
#endif 

                EPETE(subtract_scalar_from_vector(&f_vp, e_vp, 2.0));

#ifdef VERBOSE   
                dbp("First"); 
                db_rv(f_vp); 
#endif 

                diff[ diff_count++] = max_abs_vector_difference(first_vp, f_vp);
#ifdef VERBOSE
                dbi(diff_count);
                dbf(diff[ diff_count - 1 ]);
#endif 


                EPETE(ow_add_scalar_to_vector(f_vp, 10.0));

#ifdef VERBOSE   
                dbp("10 plus First"); 
                db_rv(f_vp);
#endif 

                EPETE(ow_subtract_scalar_from_vector(f_vp, 10.0));
                      
#ifdef VERBOSE   
                dbp("First"); 
                db_rv(f_vp);
#endif 

                diff[ diff_count++] = max_abs_vector_difference(first_vp, f_vp);
#ifdef VERBOSE
                dbi(diff_count);
                dbf(diff[ diff_count - 1 ]);
#endif 



                EPETE(add_scalar_to_vector(&g_vp, first_vp, 3.0));

#ifdef VERBOSE
                dbp("Three plus first"); 
                db_rv(g_vp);
#endif 

                EPETE(ow_subtract_scalar_from_vector(g_vp, 3.0));

#ifdef VERBOSE   
                dbp("Equal first"); 
                db_rv(g_vp); 
#endif 

                diff[ diff_count++] = max_abs_vector_difference(g_vp, first_vp);
#ifdef VERBOSE
                dbi(diff_count);
                dbf(diff[ diff_count - 1 ]);
#endif 

#ifdef VERBOSE   
                dbp("  --------------------       "); 
#endif 
            }

            if (length == (factor * (count+2)))
            {

                for (cur_diff = 0; cur_diff < sizeof(diff)/sizeof(diff[ 0 ]); cur_diff++)
                {
                    if ( ! IS_ZERO_DBL(diff[ cur_diff ]))
                    {
                        p_stderr("Problem with test %d (%e != 0)\n", i + 1, 
                                diff[ cur_diff ]);
                        status = EXIT_BUG;
                    }
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
    free_vector(e_vp);
    free_vector(f_vp);
    free_vector(g_vp);

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


