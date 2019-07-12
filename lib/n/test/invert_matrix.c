
/* $Id: invert_matrix.c 21660 2017-08-05 16:02:48Z kobus $ */

/* Suspicious code --- calling XXX multiple times on the mac (yet to
 * study on linux) with the same input leads to slightly different answers?
 *
 * This might be due to multi-threated stochastic optimization? 
 *
 * We cannot even reproduce when the failure occurs. We need to study kjb_rand
 * and see if the underlying routine is being screwed with. 
 *
 * Study with NUM_IDENTICAL_COMPUTATIONS greater than 1. Currently we call small
 * numbers in the answers 0, which allows regression testing to succeeed. 
*/

#include "n/n_incl.h"

#ifdef TEST_REPRODUCIBLE
#    define NUM_IDENTICAL_COMPUTATIONS 5
#    define MAKE_REPRODUCIBLE(X)  
#else 
#    define NUM_IDENTICAL_COMPUTATIONS 1
#    define MAKE_REPRODUCIBLE(X)  {X = (((X>-1.0e-14)&&(X<1.0e-14)) ? 0.0 : X);}
#endif 

/*
#define VERBOSE 1
#define VERBOSE_VERBOSE 1
*/

#define BASE_NUM_TRIES 1
#define BASE_MAX_SIZE  300

#define TOLERENCE  (100.0 * DBL_EPSILON)


/*ARGSUSED*/
int main(int argc, char* argv[ ])
{
    int status = EXIT_SUCCESS;
    int size; 
    int count;
    Matrix* mp = NULL; 
    Matrix* save_mp = NULL; 
    Matrix* target_mp = NULL; 
    Matrix* svd_target_mp = NULL; 
    Matrix* ge_target_mp = NULL; 
    Matrix* lapack_target_mp = NULL; 
    Matrix* prod_mp = NULL;
    Matrix* i_mp = NULL;
    Matrix* u_mp = NULL;
    Matrix* v_trans_mp = NULL;
    Vector* d_vp = NULL;
    double    condition_number = DBL_NOT_SET;
    int i, j;
    int     invert_result; 
    double  diff;
    long ge_cpu = 0;
    long svd_cpu = 0;
    long lapack_cpu = 0; 
    int  num_tries = NOT_SET;
    int  max_size  = NOT_SET;
    int  test_factor = 1;
    int  again;


    kjb_init(); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (test_factor <= 0)
    {
        num_tries = 1;
        max_size  = 20;
    }
    else
    {   
        double factor_for_linear = pow((double)test_factor, 1.0/4.0);

        num_tries = kjb_rint(factor_for_linear * (double)BASE_NUM_TRIES);
        max_size = kjb_rint(factor_for_linear * (double)BASE_MAX_SIZE);
    } 

    if (is_interactive())
    {
        EPETE(set_verbose_options("verbose", "1")); 
    }

    EPETE(kjb_l_set("debug", "2")); 

#ifndef VERBOSE_VERBOSE
    EPETE(kjb_l_set("page", "off")); 
#endif 

    if (is_interactive())
    {
        for (i = 1; i < argc; i++)
        {
            EPETE(read_matrix(&mp, argv[ i ]));

            if (mp->num_rows != mp->num_cols)
            {
                p_stderr("Matrix in file %s is not square.\n",
                         argv[ i ]);
                continue;
            }

            size = mp->num_rows;

            EPETE(do_svd(mp, &u_mp, &d_vp, &v_trans_mp, (int*)NULL));
            condition_number = d_vp->elements[ 0 ] / d_vp->elements[ size - 1];

            for (j = 0; j < 3; j++)
            {
                if (j == 0)
                {
                    EPETE(set_matrix_inversion_options("matrix-inversion-method", 
                                                       "svd")); 
                }
                else if (j == 1) 
                {
                    EPETE(set_matrix_inversion_options("matrix-inversion-method", 
                                                       "ge")); 
                }
                else if (j == 2)
                {
                    EPETE(set_matrix_inversion_options("matrix-inversion-method", 
                                                       "lapack")); 
                }

                invert_result = get_matrix_inverse(&target_mp, mp);

                if (invert_result == ERROR)
                {
                    kjb_print_error();
                }
                else 
                {
                    EPETE(multiply_matrices(&prod_mp, mp, target_mp));
#ifdef VERBOSE_VERBOSE
                    db_mat(prod_mp);
#endif 

                    EPETE(get_identity_matrix(&i_mp, size));

                    diff = max_abs_matrix_difference(i_mp, prod_mp) / (size * size);
                
#ifdef VERBOSE
                    pso("%d %d %d (%s) [ %.1e ] ==> %e\n",
                        i, size, size, (j == 0) ? "svd" : (j == 1) ? "g-e" : "lapack" , 
                        condition_number, diff);
#endif

                    if (diff > 100.0 * DBL_EPSILON)
                    {
                        p_stderr("Problem found! ===>  ");
                        p_stderr("(%d %d)  %e != 0.0.\n", size, size, diff); 

                        kjb_exit(EXIT_BUG);
                    }
                }
            }

        }

        if (argc > 1) 
        {
            kjb_exit(EXIT_SUCCESS); 
        }
    }


#ifdef TEST_BUG_CATCH
    EPETE(get_random_matrix(&mp, 4, 3));

    if (get_matrix_inverse(&target_mp, mp) != ERROR)
    {
        set_bug("Missed error catch on non-square matrix.\n");
        return EXIT_BUG;
    }
#endif 

#ifdef VERBOSE_VERBOSE
    if (is_interactive())
    {
        EPETE(kjb_l_set("abort-interactive-math-errors", "f")); 

        for (size=1; size<max_size; size++)
        {
            EPETE(get_zero_matrix(&mp, size, size));

            for (j = 0; j < 3; j++)
            {
                if (j == 0)
                {
                    EPETE(set_matrix_inversion_options("matrix-inversion-method", 
                                                       "svd")); 
                }
                else if (j == 1) 
                {
                    EPETE(set_matrix_inversion_options("matrix-inversion-method", 
                                                       "ge")); 
                }
                else if (j == 2)
                {
                    EPETE(set_matrix_inversion_options("matrix-inversion-method", 
                                                       "lapack")); 
                }

                invert_result = get_matrix_inverse(&target_mp, mp);

                if (invert_result == ERROR)
                {
                    kjb_print_error();
                }
                else 
                {
                    kjb_exit(EXIT_BUG);
                }
            }
        }
    }
#endif


    for (count=0; count<num_tries; count++)
    {
        for (size=1; size<max_size; size++)
        {

            EPETE(get_random_matrix(&mp, size, size));
            EPETE(ow_subtract_scalar_from_matrix(mp, 0.5));

#ifdef VERBOSE_VERBOSE
            pso("\n ------------------------- \n\n %d %d %d \n\n",
                count, size, size);
#endif 


            EPETE(do_svd(mp, &u_mp, &d_vp, &v_trans_mp, (int*)NULL));

            /*
            d_vp->elements[ size - 1 ] *= 10000.0;
            d_vp->elements[ size - 1 ] *= DBL_EPSILON;
            */

            EPETE(ow_multiply_matrix_by_row_vector_ew(u_mp, d_vp));
            EPETE(multiply_matrices(&mp, u_mp, v_trans_mp));

            EPETE(do_svd(mp, &u_mp, &d_vp, &v_trans_mp, (int*)NULL));
            condition_number = d_vp->elements[ 0 ] / d_vp->elements[ size-1];

#ifdef VERBOSE_VERBOSE
            db_mat(mp);
#endif 

            for (j = 0; j < 3; j++)
            {
                if (j == 0)
                {
                    EPETE(set_matrix_inversion_options("matrix-inversion-method", 
                                                       "svd")); 
                }
                else if (j == 1) 
                {
                    EPETE(set_matrix_inversion_options("matrix-inversion-method", 
                                                       "ge")); 
                }
                else if (j == 2)
                {
                    EPETE(set_matrix_inversion_options("matrix-inversion-method", 
                                                       "lapack")); 
                }

#ifdef VERBOSE
                EPETE(set_matrix_inversion_options("matrix-inversion-method", "")); 
#endif 

                for (again = 0; again < NUM_IDENTICAL_COMPUTATIONS; again++)
                {

                    if (again > 0) 
                    {
                        diff = max_abs_matrix_difference(mp, save_mp);
                        ASSERT(diff == 0.0);
                    }
                    EPETE(copy_matrix(&save_mp, mp));

                    init_cpu_time();
                    
                    invert_result = get_matrix_inverse(&target_mp, mp);
                    
                    if (j == 0)
                    {
                        svd_cpu += get_cpu_time(); 

                        if (again > 0) 
                        {
                            diff = max_abs_matrix_difference(target_mp, svd_target_mp);
                            ASSERT(diff == 0.0);
                        }
                        EPETE(copy_matrix(&svd_target_mp, target_mp));
                    }
                    else if (j == 1) 
                    {
                        ge_cpu += get_cpu_time(); 

                        if (again > 0) 
                        {
                            diff = max_abs_matrix_difference(target_mp, ge_target_mp);
                            ASSERT(diff == 0.0);
                        }
                        EPETE(copy_matrix(&ge_target_mp, target_mp));
                    }
                    else if (j == 2)
                    {
                        lapack_cpu += get_cpu_time(); 

                        if (again > 0) 
                        {
                            diff = max_abs_matrix_difference(target_mp, lapack_target_mp);
                            ASSERT(diff == 0.0);
                        }
                        EPETE(copy_matrix(&lapack_target_mp, target_mp));
                    }

                    if (invert_result == ERROR)
                    {
                        kjb_print_error();
                    }
                    else 
                    {
                        EPETE(multiply_matrices(&prod_mp, mp, target_mp));
#ifdef VERBOSE_VERBOSE
                        db_mat(prod_mp);
#endif 

                        EPETE(get_identity_matrix(&i_mp, size));

                        diff = max_abs_matrix_difference(i_mp, prod_mp) / (size * size);
                        MAKE_REPRODUCIBLE(diff)
                    
                        verbose_pso(1, "%d %d %d (%s) [ %.1e ] ==> %e\n",
                                    1 + count, size, size, (j == 0) ? "svd" : (j == 1) ? "g-e" : "lapack" , 
                                    condition_number, diff);

                        if (diff > TOLERENCE)
                        {
                            p_stderr("Problem found! ===>  ");
                            p_stderr("(%d %d)  %e > %e.\n", size, size, diff, TOLERENCE); 
                            status = EXIT_BUG;
                        }
                    }
                }
            }
        }
    }

    if (is_interactive())
    {
        pso("ge cpu    : %ld\n", ge_cpu);
        pso("svd cpu   : %ld\n", svd_cpu);
        pso("lapack cpu: %ld\n", lapack_cpu);
    }

    free_matrix(prod_mp);
    free_matrix(i_mp);
    free_matrix(mp);
    free_matrix(save_mp);
    free_matrix(target_mp);
    free_matrix(svd_target_mp);
    free_matrix(ge_target_mp);
    free_matrix(lapack_target_mp);
    free_matrix(u_mp);
    free_matrix(v_trans_mp);
    free_vector(d_vp);

    return status;
}

