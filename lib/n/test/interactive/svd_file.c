
/* $Id: svd_file.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "n/n_incl.h"

/*
*/
#define VERBOSE 1 


/*ARGSUSED*/
int main(int argc, char *argv[])
{
    int     num_rows;
    int     num_cols;
    int     count = 0;
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
#endif 
    int     rank;
    double diff_1        = 0.0;
    double diff_2        = 0.0;
    double diff_3        = 0.0;
    int    i;
    int    ordered;
    int    save_num_rows;


    kjb_init();

    if (! is_interactive()) 
    {
        p_stderr("This test program either needs to be adjusted for batch testing or moved.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    check_num_args(argc, 1, 1, NULL);

    EPETE(set_heap_options("heap-checking", "f")); 

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

    EPETE(read_matrix(&first_mp, argv[ 1 ])); 

    num_rows = first_mp->num_rows;
    num_cols = first_mp->num_cols;

#ifdef VERBOSE
    pso("\n --------------------- \n");
    pso("%d %d\n\n", num_rows, num_cols);
    EPETE(set_svd_options("svd-method", "")); 
#endif 


#ifdef VERBOSE
    db_mat(first_mp);
#endif 
    if (do_svd(first_mp, &u_mp, &diag_vp, &v_trans_mp, &rank) == ERROR)
    {
        kjb_print_error();
        db_mat(first_mp);
        return EXIT_BUG;
    }

#ifdef VERBOSE
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

#ifdef VERBOSE
    db_mat(diag_mp);
#endif 

    EPETE(multiply_matrices(&u_times_d_mp, u_mp, diag_mp));
    EPETE(multiply_matrices(&prod_mp, u_times_d_mp, v_trans_mp));

#ifdef VERBOSE
    db_mat(prod_mp);
#endif 

    diff_1 = max_abs_matrix_difference(prod_mp, first_mp);
    diff_1 /= (num_rows * num_cols); 
    diff_1 /= MAX_OF(num_rows, num_cols); 
    diff_1 /= 3.0; 
    

    EPETE(multiply_with_transpose(&u_trans_times_u_mp, u_mp, u_mp));
    EPETE(multiply_by_transpose(&v_times_v_trans_mp, v_trans_mp, v_trans_mp));

#ifdef VERBOSE
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
        set_bug("Singular values are not ordered.");
        return EXIT_BUG;
    }

    if (diff_1 > 10.0 * DBL_EPSILON)
    {
        p_stderr("(%d, %d, %d)  %e != 0.0.\n", count, num_rows, 
                 num_cols, diff_1); 
        set_bug("SVD does not equal input");
        return EXIT_BUG;
    }

    if (diff_2 > 10.0 * DBL_EPSILON)
    {
        p_stderr("(%d, %d, %d)  %e != 0.0.\n", count, num_rows,
                 num_cols, diff_2); 
        set_bug("U is not orthonormal");
        return EXIT_BUG;
    }

    if (diff_3 > 10.0 * DBL_EPSILON)
    {
        p_stderr("(%d, %d, %d)  %e != 0.0.\n", count, num_rows, 
                 num_cols, diff_3); 
        set_bug("V is not orthonormal");
        return EXIT_BUG;
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
                db_mat(first_mp);
                continue;
            }

            v_trans_mp->num_rows = save_num_rows; 
            if (get_row_fits(&fit_est_mp, first_mp, i+1, 
                               v_trans_mp, 
                               &fit_error)
                == ERROR)
            {
                kjb_print_error();
                db_mat(first_mp);
                continue;
            }


#ifdef VERBOSE
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
                set_bug("The two error methods do not agrees");
                return EXIT_BUG;
            }

            if (diff_2 > 10.0 * DBL_EPSILON)
            {
                p_stderr("%e != 0.0.\n", diff_2); 
                set_bug("The two fitting methods do not agrees");
                return EXIT_BUG;
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


    return EXIT_SUCCESS; 
}

