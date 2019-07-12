
/* $Id: svd.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "n/n_incl.h"

/*
#define VERBOSE 1 
*/

#ifdef EXTRA_VERBOSE
#    define MAX_SIZE      6
#    define FIRST_ROW      3
#    define FIRST_COL      3
#else
#    define MAX_SIZE      100
#    define FIRST_ROW      1
#    define FIRST_COL      1
#endif 


#define BASE_NUM_TRIES 1000 


/*ARGSUSED*/
int main(int argc, char *argv[])
{
    int status = EXIT_SUCCESS;
    int     num_rows;
    int     num_cols;
    int     count;
    Matrix* diag_mp            = NULL;
    Matrix* first_mp           = NULL;
    Matrix* fit_est_mp         = NULL;
    Matrix* ident_mp           = NULL;
    Matrix* prod_mp            = NULL;
    Matrix* projection_est_mp  = NULL;
    Matrix* u_mp               = NULL;
    Matrix* lp_u_mp            = NULL;
    Matrix* nr_u_mp            = NULL;
    Matrix* u_times_d_mp       = NULL;
    Matrix* u_trans_times_u_mp = NULL;
    Matrix* v_times_v_trans_mp = NULL;
    Matrix* v_trans_mp         = NULL;
    Matrix* lp_v_trans_mp      = NULL;
    Matrix* nr_v_trans_mp      = NULL;
    Vector* diag_vp            = NULL;
    Vector* lp_diag_vp         = NULL;
    Vector* nr_diag_vp         = NULL;
    double    projection_error;
    double    fit_error;
#ifdef KJB_HAVE_NUMERICAL_RECIPES
    int     alternate          = FALSE;
    double    max_d_diff;
#endif 
    int     rank;
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

    EPETE(set_heap_options("heap-checking", "f")); 

    if (is_interactive())
    {
        EPETE(set_verbose_options("verbose", "1")); 
    }

    for (count=0; count<num_tries; count++)
    {
        num_rows = 1 + kjb_rint(MAX_SIZE * kjb_rand());
        num_cols = 1 + kjb_rint(MAX_SIZE * kjb_rand());

        EPETE(get_random_matrix(&first_mp, num_rows, num_cols)); 

#ifdef KJB_HAVE_NUMERICAL_RECIPES
        EPETE(set_svd_options("svd-method", "nr")); 

        if (do_svd(first_mp, (Matrix**)NULL, (Vector**)NULL,
                   (Matrix**)NULL, &rank) == ERROR)
        {
            kjb_print_error();
#ifdef VERBOSE
            db_mat(first_mp);
#endif
            status = EXIT_BUG;

            continue;
        }

        if (do_svd(first_mp, &nr_u_mp, &nr_diag_vp, &nr_v_trans_mp,
                   &rank) == ERROR)
        {
            kjb_print_error();
#ifdef VERBOSE
            db_mat(first_mp);
#endif
            status = EXIT_BUG;

            continue;
        }
#endif 

        EPETE(set_svd_options("svd-method", "lapack")); 

        if (do_svd(first_mp, (Matrix**)NULL, (Vector**)NULL,
                   (Matrix**)NULL, &rank) == ERROR)
        {
            kjb_print_error();
#ifdef VERBOSE
            db_mat(first_mp);
#endif
            status = EXIT_BUG;

            continue;
        }

        if (do_svd(first_mp, &lp_u_mp, &lp_diag_vp, &lp_v_trans_mp,
                   &rank) == ERROR)
        {
            kjb_print_error();
#ifdef VERBOSE
            db_mat(first_mp);
#endif
            status = EXIT_BUG;

            continue;
        }


#ifdef KJB_HAVE_NUMERICAL_RECIPES
        /*
        // Note that nr_u_mp, lp_u_mp, and nr_v_trans_mp, lp_v_trans_mp
        // are only the same up to signs, and then only if A is square
        // and the singular values are distinct.
         */

        max_d_diff = max_abs_vector_difference(nr_diag_vp, lp_diag_vp); 

        if (max_d_diff > 100.0 * DBL_EPSILON)
        {
            dbe(max_d_diff); 
        }
#endif 
    }

    for (count=0; count<num_tries; count++)
    {
        num_rows = 1 + kjb_rint(MAX_SIZE * kjb_rand());
        num_cols = num_rows + kjb_rint(MAX_SIZE * kjb_rand());

        {
            double diff_1        = 0.0;
            double diff_2        = 0.0;
            double diff_3        = 0.0;
            int    i;
            int    ordered;
            int    save_num_rows;


#ifdef KJB_HAVE_NUMERICAL_RECIPES
            if (alternate)
            {
                alternate = FALSE; 
                EPETE(set_svd_options("svd-method", "lapack")); 
            }
            else 
            {
                alternate = TRUE; 
                EPETE(set_svd_options("svd-method", "nr")); 
            }
#else 
            EPETE(set_svd_options("svd-method", "lapack")); 
#endif 

            verbose_pso(1, "\n --------------------- \n");
            verbose_pso(1, "%d %d %d\n\n", count, num_rows, num_cols);

#ifdef VERBOSE
            EPETE(set_svd_options("svd-method", "")); 
#endif 

            EPETE(get_random_matrix(&first_mp, num_rows, num_cols)); 

#ifdef EXTRA_VERBOSE
            db_mat(first_mp);
#endif 
            if (do_svd(first_mp, &u_mp, &diag_vp, &v_trans_mp, &rank) == ERROR)
            {
                kjb_print_error();
#ifdef VERBOSE
                db_mat(first_mp);
#endif
                status = EXIT_BUG;

                continue;
            }

#ifdef EXTRA_VERBOSE
            dbi(rank); 
            db_mat(u_mp);
            db_rv(diag_vp); 
            db_mat(v_trans_mp);
#endif 
            EPETE(get_zero_matrix(&diag_mp, 
                                  MIN_OF(num_rows, num_cols),
                                  num_cols)); 

            ordered = TRUE; 

            for (i=0; i<diag_vp->length; i++)
            {
                diag_mp->elements[ i ][ i ] = diag_vp->elements[ i ];

                if (    (i > 0)
                     && (diag_vp->elements[ i-1 ] < diag_vp->elements[ i ])
                   )
                {
                    ordered = FALSE;
                }
            }

#ifdef EXTRA_VERBOSE
            db_mat(diag_mp);
#endif 

            EPETE(multiply_matrices(&u_times_d_mp, u_mp, diag_mp));
            EPETE(multiply_matrices(&prod_mp, u_times_d_mp, v_trans_mp));

#ifdef EXTRA_VERBOSE
            db_mat(prod_mp);
#endif 

            diff_1 = max_abs_matrix_difference(prod_mp, first_mp);
            diff_1 /= (num_rows * num_cols); 
            diff_1 /= MAX_OF(num_rows, num_cols); 
            diff_1 /= 3.0; 
            

            EPETE(multiply_with_transpose(&u_trans_times_u_mp, u_mp, u_mp));
            EPETE(multiply_by_transpose(&v_times_v_trans_mp, v_trans_mp, v_trans_mp));

#ifdef EXTRA_VERBOSE
            db_mat(u_trans_times_u_mp);
            db_mat(v_times_v_trans_mp);
#endif 

            EPETE(get_identity_matrix(&ident_mp, u_mp->num_cols));
            diff_2 = max_abs_matrix_difference(ident_mp, 
                                               u_trans_times_u_mp);
            diff_2 /= (num_rows * num_cols); 
            diff_2 /= MAX_OF(num_rows, num_cols); 
            diff_2 /= 3.0; 

            EPETE(get_identity_matrix(&ident_mp, v_trans_mp->num_rows));
            diff_3 = max_abs_matrix_difference(ident_mp, 
                                               v_times_v_trans_mp); 
            diff_3 /= (num_rows * num_cols); 
            diff_3 /= MAX_OF(num_rows, num_cols); 
            diff_3 /= 3.0; 

            if ( ! ordered )
            {
                p_stderr("Singular values are not ordered.\n");
                status = EXIT_BUG;
            }

            if (diff_1 > 10.0 * DBL_EPSILON)
            {
                p_stderr("(%d, %d, %d)  %e != 0.0.\n", count, num_rows, 
                         num_cols, diff_1); 
                p_stderr("SVD does not equal input\n");
                status = EXIT_BUG;
            }

            if (diff_2 > 10.0 * DBL_EPSILON)
            {
                p_stderr("(%d, %d, %d)  %e != 0.0.\n", count, num_rows,
                         num_cols, diff_2); 
                p_stderr("U is not orthonormal\n");
                status = EXIT_BUG;
            }

            if (diff_3 > 10.0 * DBL_EPSILON)
            {
                p_stderr("(%d, %d, %d)  %e != 0.0.\n", count, num_rows, 
                         num_cols, diff_3); 
                p_stderr("V is not orthonormal\n");
                status = EXIT_BUG;
            }

            if (    ((num_rows < 6) && (num_cols < 6)) 
                 || (    (num_rows < 10) 
                      && (num_cols < 10) 
                      && (abs(num_rows - num_cols) < 3)
                    )
                 || (kjb_rand() < 0.05) 
               )
            {
                save_num_rows = v_trans_mp->num_rows;

                for (i=0; i<save_num_rows; i++)
                {
                    v_trans_mp->num_rows = i+1;


                    if (project_rows_onto_basis(&projection_est_mp, 
                                                  first_mp, 
                                                  v_trans_mp, 
                                                  &projection_error) 
                        == ERROR)
                    {
                        kjb_print_error();
#ifdef VERBOSE
                        db_mat(first_mp);
#endif
                        status = EXIT_BUG;

                        continue;
                    }

                    v_trans_mp->num_rows = save_num_rows; 
                    if (get_row_fits(&fit_est_mp, first_mp, i+1, 
                                       v_trans_mp, 
                                       &fit_error)
                        == ERROR)
                    {
                        kjb_print_error();
#ifdef VERBOSE
                        db_mat(first_mp);
#endif
                        status = EXIT_BUG;

                        continue;
                    }


#ifdef EXTRA_VERBOSE
                    dbi(i); 
                    db_mat(first_mp); 
                    db_mat(projection_est_mp);
                    dbf(projection_error);
                    db_mat(fit_est_mp);
                    dbf(fit_error);
#endif 

                    diff_1 = (fit_error - projection_error);
                    diff_1 /= (num_rows * num_cols);
                    diff_1 /= MAX_OF(num_rows, num_cols); 
                    diff_1 /= 3.0; 

                    diff_2 = max_abs_matrix_difference(fit_est_mp, 
                                                       projection_est_mp); 
                    diff_2 /= (num_rows * num_cols);
                    diff_2 /= MAX_OF(num_rows, num_cols); 
                    diff_2 /= 3.0; 

                    if (diff_1 > 10.0 * DBL_EPSILON)
                    {
                        p_stderr("%e != 0.0.\n", diff_1); 
                        p_stderr("The two error methods do not agrees\n");
                        status = EXIT_BUG;
                    }

                    if (diff_2 > 10.0 * DBL_EPSILON)
                    {
                        p_stderr("%e != 0.0.\n", diff_2); 
                        p_stderr("The two fitting methods do not agrees\n");
                        status = EXIT_BUG;
                    }

                }
            }
        }
    }
    
    free_matrix(diag_mp);
    free_matrix(first_mp); 
    free_matrix(fit_est_mp);
    free_matrix(ident_mp); 
    free_matrix(prod_mp);
    free_matrix(projection_est_mp);
    free_matrix(u_mp); 
    free_matrix(lp_u_mp); 
    free_matrix(nr_u_mp); 
    free_matrix(u_trans_times_u_mp);
    free_matrix(v_times_v_trans_mp);
    free_matrix(v_trans_mp);
    free_matrix(lp_v_trans_mp);
    free_matrix(nr_v_trans_mp);
    free_vector(diag_vp); 
    free_vector(lp_diag_vp); 
    free_vector(nr_diag_vp); 

    return status;
}

