
/* $Id: mat_stat.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"


/*
#define  VERBOSE  1 
*/


static Vector_stat* regress_get_matrix_row_stats(const Matrix* mp);

#define NUM_LOOPS        40 
#define BASE_NUM_TRIES   10


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int num_rows;
    int num_cols; 
    int count;
    Vector* row_mean_vp = NULL; 
    Vector* row_stdev_vp = NULL; 
    Vector* col_mean_vp = NULL; 
    Vector* col_stdev_vp = NULL; 
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;
    Matrix* mp = NULL;
    Matrix* trans_mp = NULL;
    Vector*      row_sum_vp   = NULL;
    Vector*      col_sum_vp   = NULL;


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

    kjb_set_debug_level(2);

    for (count=0; count<num_tries; count++)
    {
        if (IS_EVEN(count))
        {
            enable_respect_missing_values();
        }
        else 
        {
            disable_respect_missing_values();
        }

#ifdef VERBOSE
        dbp("------------------------------------------------"); 
        dbp("------------------------------------------------"); 
        dbp("------------------------------------------------"); 
        dbi(respect_missing_values());
        dbi(count); 
        dbp("------------------------------------------------"); 
        dbp("------------------------------------------------"); 
        dbp("------------------------------------------------"); 
#endif 

        for (num_rows=2; num_rows<NUM_LOOPS; num_rows++)
        {
            for (num_cols=2; num_cols<NUM_LOOPS; num_cols++)
            {
                Vector_stat* row_stat_ptr = NULL;
                Vector_stat* col_stat_ptr = NULL;
                double       diff_1;
                double       diff_2;
                double       diff_3;
                double       diff_4;
                int          have_missing = FALSE;



                EPETE(get_random_matrix(&mp, num_rows, num_cols)); 

                if (IS_EVEN(count / 2))
                {
                    mp->elements[ num_rows / 2 ][ num_cols / 2 ] = DBL_MISSING; 
                    have_missing = TRUE;
                }

                EPETE(get_transpose(&trans_mp, mp));

#ifdef VERBOSE
                db_mat(mp); 
#endif 

                EPETE(get_matrix_row_stats(mp, &row_mean_vp, &row_stdev_vp)); 

                if (! have_missing)
                {
                    NPETE(row_stat_ptr = regress_get_matrix_row_stats(mp));
                }
    
#ifdef VERBOSE
                if (! have_missing)
                {
                    db_rv(row_stat_ptr->mean_vp); 
                }
                db_rv(row_mean_vp); 

                if (! have_missing)
                {
                    db_rv(row_stat_ptr->stdev_vp); 
                }
                db_rv(row_stdev_vp); 
#endif 

                if (! have_missing)
                {
                    NPETE(col_stat_ptr = regress_get_matrix_row_stats(trans_mp));
                }

                EPETE(get_matrix_row_stats(trans_mp, &col_mean_vp, &col_stdev_vp)); 

#ifdef VERBOSE
                if (! have_missing)
                {
                    db_rv(col_stat_ptr->mean_vp); 
                }
                db_rv(col_mean_vp); 
                if (! have_missing)
                {
                    db_rv(col_stat_ptr->stdev_vp); 
                }
                db_rv(col_stdev_vp); 
#endif 

                EPETE(sum_matrix_rows(&row_sum_vp, mp));

#ifdef VERBOSE
                db_rv(row_sum_vp); 
#endif 

                EPETE(sum_matrix_cols(&col_sum_vp, mp));

#ifdef VERBOSE
                db_rv(col_sum_vp); 
#endif 


                EPETE(average_matrix_rows(&row_mean_vp, mp));

#ifdef VERBOSE
                db_rv(row_mean_vp); 
#endif 

                EPETE(average_matrix_cols(&col_mean_vp, mp));

#ifdef VERBOSE
                db_rv(col_mean_vp); 
#endif 


                if (have_missing)
                {
                    EPETE(ow_divide_vector_by_scalar(row_sum_vp, 
                                                     (double)(mp->num_rows) - 1.0));
                }
                else
                {
                    EPETE(ow_divide_vector_by_scalar(row_sum_vp, 
                                                     (double)(mp->num_rows)));
                }

                if (have_missing)
                {
                    EPETE(ow_divide_vector_by_scalar(col_sum_vp, 
                                                     (double)(mp->num_cols) - 1.0));
                }
                else
                {
                    EPETE(ow_divide_vector_by_scalar(col_sum_vp, 
                                                     (double)(mp->num_cols)));
                }

                if (! have_missing)
                {
                    diff_1 = max_abs_vector_difference(row_stat_ptr->mean_vp,
                                                       row_mean_vp);

                    if (ABS_OF(diff_1) > 1e2 * DBL_EPSILON)
                    {
                        p_stderr("%e != 0.0. (count=%d, rows=%d, cols=%d)\n", 
                                 diff_1, count, num_rows, num_cols);
                        p_stderr("Problem with test 1.\n");
                        status = EXIT_BUG;
                    }

                    diff_2 = max_abs_vector_difference(row_stat_ptr->mean_vp,
                                                       row_sum_vp);

                    if (ABS_OF(diff_2) > 1e2 * DBL_EPSILON)
                    {
                        p_stderr("%e != 0.0. (count=%d, rows=%d, cols=%d)\n", 
                                 diff_2, count, num_rows, num_cols);
                        p_stderr("Problem with test 2.\n");
                        status = EXIT_BUG;
                    }

                    diff_3 = max_abs_vector_difference(col_stat_ptr->mean_vp,
                                                       col_mean_vp);

                    if (ABS_OF(diff_3) > 1e2 * DBL_EPSILON)
                    {
                        p_stderr("%e != 0.0. (count=%d, rows=%d, cols=%d)\n", 
                                 diff_3, count, num_rows, num_cols);
                        p_stderr("Problem with test 3.\n");
                        status = EXIT_BUG;
                    }

                    diff_4 = max_abs_vector_difference(col_stat_ptr->mean_vp,
                                                       col_sum_vp);

                    if (ABS_OF(diff_4) > 1e2 * DBL_EPSILON)
                    {
                        p_stderr("%e != 0.0. (count=%d, rows=%d, cols=%d)\n", 
                                 diff_4, count, num_rows, num_cols);
                        p_stderr("Problem with test 4.\n");
                        status = EXIT_BUG;
                    }

                    if (max_abs_vector_difference(row_mean_vp, row_stat_ptr->mean_vp) > 1e-10)
                    {
                        dbe(max_abs_vector_difference(row_mean_vp, row_stat_ptr->mean_vp));
                        db_rv(row_mean_vp);
                        db_rv(row_stat_ptr->mean_vp);
                    }

                    if (max_abs_vector_difference(row_stdev_vp, row_stat_ptr->stdev_vp) > 1e-10)
                    {
                        dbe(max_abs_vector_difference(row_stdev_vp, row_stat_ptr->stdev_vp));
                        db_rv(row_stdev_vp);
                        db_rv(row_stat_ptr->stdev_vp);
                    }

                    if (max_abs_vector_difference(col_mean_vp, col_stat_ptr->mean_vp) > 1e-10)
                    {
                        dbe(max_abs_vector_difference(col_mean_vp, col_stat_ptr->mean_vp));
                        db_rv(col_mean_vp);
                        db_rv(col_stat_ptr->mean_vp);
                    }

                    if (max_abs_vector_difference(col_stdev_vp, col_stat_ptr->stdev_vp) > 1e-10)
                    {
                        dbe(max_abs_vector_difference(col_stdev_vp, col_stat_ptr->stdev_vp));
                        db_rv(col_stdev_vp);
                        db_rv(col_stat_ptr->stdev_vp);
                    }

                    free_vector_stat(row_stat_ptr);
                    free_vector_stat(col_stat_ptr);
                }
            }
        }
    }
    
    free_matrix(mp);
    free_matrix(trans_mp);
    free_vector(row_sum_vp); 
    free_vector(col_sum_vp); 
    free_vector(row_mean_vp); 
    free_vector(col_mean_vp); 
    free_vector(row_stdev_vp); 
    free_vector(col_stdev_vp); 

    return status; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Vector_stat* regress_get_matrix_row_stats(const Matrix* mp)
{
    int          num_rows;
    int          i;
    Vector*      row_vp = NULL;
    Vector_stat* result_ptr;


    num_rows = mp->num_rows;

    ERN(clear_vector_stats()); 

    for (i=0; i<num_rows; i++) 
    {
        ERN(get_matrix_row(&row_vp, mp, i));
        ERN(add_vector_data_point(row_vp)); 
    }

    NRN(result_ptr = get_vector_data_stats()); 

    free_vector(row_vp); 

    ERN(clear_vector_stats());

    return result_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

