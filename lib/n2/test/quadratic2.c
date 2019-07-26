
/* $Id: quadratic2.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"
#include "n2/n2_incl.h"
#include "wrap_slatec/wrap_slatec.h"

#define TOL    1e-06
#define MIN_SIZE  4
#define MAX_SIZE  500
#define BASE_NUM_TRIES   3

/*ARGSUSED*/
int main(int argc, char *argv[])
{
    int status = EXIT_SUCCESS;
    Matrix* mp = NULL;
    Matrix* le_constraint_mp = NULL;
    Vector* target_vp = NULL;
    Vector* le_constraint_col_vp = NULL;
    Vector* eq_constraint_col_vp = NULL;
    Matrix* eq_constraint_mp = NULL;
    Vector* lb_row_vp = NULL;
    Vector* ub_row_vp = NULL;
    Vector* dbocls_result_vp            = NULL;
    Vector* dlsei_result_vp            = NULL;
#ifdef KJB_HAVE_LOQO
    Vector* loqo_result_vp            = NULL;
#endif 
    int     res;
    int     size, i, j;
    Vector* est_vp = NULL; 
    double  dbocls_dlsei; 
#ifdef KJB_HAVE_LOQO
    double  loqo_dlsei; 
    double  loqo_dbocls; 
#endif 
    int num_vars, num_eq, num_data_eq, num_smooth_eq; 
    int try_count; 
    long dbocls_cpu, dlsei_cpu; 
    int  num_tries = BASE_NUM_TRIES;
    int max_size  = MAX_SIZE;
    int  test_factor = 1;


    kjb_init(); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (test_factor <= 0)
    {
        num_tries = 1;
        max_size = 20;
    }
    else
    {
        /*
         * I am not sure of the complexity of the underlying algorithm. 1/4
         * power is the correct answer if the complixity goes up as the cube of
         * the size, as factor_for_linear also increases the number of
         * iterations.
        */
        double factor_for_linear = pow((double)test_factor, 1.0/4.0);

        max_size = kjb_rint((double)MAX_SIZE * factor_for_linear);
        num_tries = kjb_rint((double)BASE_NUM_TRIES * factor_for_linear);
    } 

    EPETE(set_heap_options("heap-checking", "f")); 

    if (is_interactive())
    {
        EPETE(kjb_set_debug_level(2));
        EPETE(kjb_set_verbose_level(1)); 
    }
    else
    {
        EPETE(kjb_set_debug_level(0));
        EPETE(kjb_set_verbose_level(0)); 
    }

    size = max_size;

    while (size > MIN_SIZE)
    {
        for (try_count = 0; try_count < num_tries; try_count++)
        {
            num_vars = size;
            num_data_eq = 3 * size;
            num_smooth_eq = size - 2;
            num_eq = num_data_eq + num_smooth_eq; 

            dbp("--------------------------"); 

            dbi(size); 

            EPETE(get_zero_vector(&lb_row_vp, num_vars)); 
            EPETE(get_initialized_vector(&ub_row_vp, num_vars, 
                                         DBL_HALF_MOST_POSITIVE)); 
            EPETE(get_random_matrix(&mp, num_eq, num_vars)); 
            /* EPETE(ow_subtract_scalar_from_matrix(mp, 0.5)); */
            EPETE(ow_multiply_matrix_by_scalar(mp, 0.01)); 

            for (i = 0; i < num_eq; i++)
            {
                for (j = 0; j < num_vars; j++)
                {
                    mp->elements[ i ][ j ] *= mp->elements[ i ][ j ];
                }
            }

            EPETE(get_random_vector(&target_vp, 4 * size - 2)); 
            /* EPETE(ow_subtract_scalar_from_vector(target_vp, 0.5)); */
            EPETE(ow_multiply_vector_by_scalar(target_vp, 500.0)); 

            for (i = 0; i < num_smooth_eq; i++)
            {
                for (j = 0; j < num_vars; j++)
                {
                    mp->elements[ num_data_eq + i ][ j ] = 0.0; 
                }

                mp->elements[ num_data_eq + i ][ i ] = -1.0; 
                mp->elements[ num_data_eq + i ][ i + 1 ] = 2.0; 
                mp->elements[ num_data_eq + i ][ i + 2 ] = -1.0; 

                target_vp->elements[ num_data_eq + i ] = 0.0; 
            }

            /*
            EPETE(ow_multiply_matrix_by_scalar(mp, 100.0));
            EPETE(ow_multiply_vector_by_scalar(target_vp, 100.0)); 
            */


            init_cpu_time();
            EPETE(res = do_dbocls_quadratic(&dbocls_result_vp, mp, target_vp ,
                                            le_constraint_mp,le_constraint_col_vp,
                                            NULL, NULL,
                                            lb_row_vp, ub_row_vp));
            dbocls_cpu = get_cpu_time();

            /*
            dbi(res); 
            output_col_vector(stdout, dbocls_result_vp,"RESULT");
            */

            EPETE(multiply_matrix_and_vector(&est_vp, mp, dbocls_result_vp));
            verbose_pso(1, "DBOCLS distance: %.5e.\n", 
                (double)vector_distance(est_vp, target_vp)); 
            verbose_pso(1, "DBOCLS cpu: %ld.\n", dbocls_cpu);  
         
            init_cpu_time();
            EPETE(res = do_dlsei_quadratic(&dlsei_result_vp, mp, target_vp ,
                                           le_constraint_mp,le_constraint_col_vp,
                                           NULL, NULL,
                                           lb_row_vp, ub_row_vp));
            dlsei_cpu = get_cpu_time();
            /*
            dbi(res); 
            output_col_vector(stdout, dlsei_result_vp,"RESULT");
            */

            EPETE(multiply_matrix_and_vector(&est_vp, mp, dlsei_result_vp));
            verbose_pso(1, "DLSEI  distance: %.5e.\n", 
                (double)vector_distance(est_vp, target_vp)); 
            verbose_pso(1, "DLSEI cpu: %ld.\n", dlsei_cpu);  

         
#ifdef KJB_HAVE_LOQO
            EPE(res = do_loqo_quadratic(&loqo_result_vp, mp, target_vp ,
                                        le_constraint_mp, le_constraint_col_vp,
                                        NULL, NULL,
                                        lb_row_vp, ub_row_vp);
            /*
            db_cv(loqo_result_vp); 
            */

            /*
            dbi(res); 
            output_col_vector(stdout, loqo_result_vp,"RESULT");
            */

            EPETE(multiply_matrix_and_vector(&est_vp, mp, loqo_result_vp));
            verbose_pso(1, "LOQO   distance: %.5e.\n", 
                (double)vector_distance(est_vp, target_vp)); 
#endif 

            verbose_pso(1, "DBOCLS feasible: %.5e.\n", 
                (double)min_vector_element(dbocls_result_vp)); 
            verbose_pso(1, "DLSEI feasible: %.5e.\n", 
                (double)min_vector_element(dlsei_result_vp)); 
#ifdef KJB_HAVE_LOQO
            verbose_pso(1, "LOQO feasible: %.5e.\n", 
                (double)min_vector_element(loqo_result_vp)); 
#endif 

#ifdef KJB_HAVE_LOQO
            loqo_dbocls = max_abs_vector_difference(loqo_result_vp, 
                                                    dbocls_result_vp); 
            loqo_dbocls /= (num_vars * num_eq); 
            verbose_pso(1, "| LOQO - DBOCLS | = %.5e.\n", loqo_dbocls); 

            if (loqo_dbocls > TOL)
            {
                p_stderr("Test of loqo versus dbocls failed.\n");
                p_stderr("%.5e > %.5e\n", loqo_dbocls, TOL);
                status = EXIT_BUG;
            }
#endif 

#ifdef KJB_HAVE_LOQO
            loqo_dlsei = max_abs_vector_difference(loqo_result_vp,
                                                   dlsei_result_vp); 
            loqo_dlsei /= (num_vars * num_eq); 
            verbose_pso(1, "| LOQO - DLSEI  | = %.5e.\n", loqo_dlsei); 

            if (loqo_dlsei > TOL)
            {
                p_stderr("Test of loqo versus dlsei failed.\n");
                p_stderr("%.5e > %.5e\n", loqo_dlsei, TOL);
                status = EXIT_BUG;
            }
#endif 

            dbocls_dlsei = max_abs_vector_difference(dbocls_result_vp,
                                                     dlsei_result_vp); 
            dbocls_dlsei /= (num_vars * num_eq); 
            verbose_pso(1, "| DBOCLS - DLSEI | = %.5e.\n", dbocls_dlsei); 

            if (dbocls_dlsei > TOL)
            {
                p_stderr("Test of dbocls versus dlsei failed.\n");
                p_stderr("%.5e > %.5e\n", dbocls_dlsei, TOL);
                status = EXIT_BUG;
            }
        }

        size /= 2;
    }

    free_matrix(le_constraint_mp);
    free_vector(le_constraint_col_vp);
    free_matrix(eq_constraint_mp);
    free_vector(eq_constraint_col_vp);
    free_vector(lb_row_vp);
    free_vector(ub_row_vp);
    free_vector(dbocls_result_vp);
    free_vector(dlsei_result_vp);
#ifdef KJB_HAVE_LOQO
    free_vector(loqo_result_vp);
#endif 
    free_matrix(mp);
    free_vector(target_vp);
    free_vector(est_vp);

    return status; 
   }


