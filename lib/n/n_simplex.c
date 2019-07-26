
/* $Id: n_simplex.c 8780 2011-02-27 23:42:02Z predoehl $ */

/*
   Copyright (c) 1994-2008 by Kobus Barnard (author).

   Personal and educational use of this code is granted, provided
   that this header is kept intact, and that the authorship is not
   misrepresented. Commercial use is not permitted.
*/


#include "n/n_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "n/n_invert.h"
#include "n/n_simplex.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int do_simplex_guts
(
    Matrix* basis_mp,
    Matrix* non_basis_mp,
    Vector* basis_obj_row_vp,
    Vector* non_basis_obj_row_vp,
    Vector* solution_col_vp,
    Int_vector* basis_var_index_vp,
    Int_vector* non_basis_var_index_vp,
    int     max_num_iterations,
    int     report,
    Matrix* inverse_basis_mp,
    Vector* a_col_vp,
    Vector* d_col_vp,
    Vector* temp_d_col_vp,
    Vector* y_row_vp,
    Vector* leave_col_vp,
    Vector* temp_solution_col_vp
);

static int do_aux_simplex_guts
(
    Matrix* basis_mp,
    Matrix* non_basis_mp,
    Vector* basis_obj_row_vp,
    Vector* non_basis_obj_row_vp,
    Vector* solution_col_vp,
    Int_vector* basis_var_index_vp,
    Int_vector* non_basis_var_index_vp,
    int     max_num_iterations,
    int     report,
    int     aux_var_basis_index,
    Matrix* inverse_basis_mp,
    Vector* a_col_vp,
    Vector* d_col_vp,
    Vector* temp_d_col_vp,
    Vector* y_row_vp,
    Vector* leave_col_vp,
    Vector* temp_solution_col_vp
);

static int do_gen_simplex_guts
(
    Matrix* basis_mp,
    Matrix* non_basis_mp,
    Matrix* inverse_basis_mp,
    Vector* solution_col_vp,
    Vector* non_basis_solution_col_vp,
    Vector* basis_obj_row_vp,
    Vector* non_basis_obj_row_vp,
    Vector* basis_lb_row_vp,
    Vector* non_basis_lb_row_vp,
    Vector* basis_ub_row_vp,
    Vector* non_basis_ub_row_vp,
    Int_vector* basis_var_index_vp,
    Int_vector* non_basis_var_index_vp,
    int     max_num_iterations,
    int     report,
    Vector* a_col_vp,
    Vector* d_col_vp,
    Vector* temp_d_col_vp,
    Vector* y_row_vp,
    Vector* leave_col_vp,
    Vector* temp_solution_col_vp
);

/* -------------------------------------------------------------------------- */

#ifdef DEBUG
    EXPORT Matrix *db_constraint_mp = NULL;
    EXPORT Vector *db_constraint_col_vp = NULL;
#endif

/* -------------------------------------------------------------------------- */

int do_simplex
(
    Matrix* constraint_mp,
    Vector* constraint_col_vp,
    Vector* obj_row_vp,
    int     max_num_iterations,
    int     report,
    Vector* result_vp
)
{
#ifdef DEBUG
    IMPORT int kjb_debug_level;
#endif
    Matrix* basis_mp;
    Matrix* non_basis_mp;
    Matrix* aux_non_basis_mp;
    Matrix* inverse_basis_mp;
    Vector* aux_non_basis_mat_col_vp;
    Vector* aux_non_basis_obj_row_vp;
    Vector* aux_result_vp;
    Vector* basis_obj_row_vp;
    Vector* non_basis_obj_row_vp;
    Int_vector* basis_var_index_vp;
    Int_vector* non_basis_var_index_vp;
    Int_vector* aux_non_basis_var_index_vp;
    Vector* solution_col_vp = NULL;
    Vector* temp_constraint_col_vp;
    int     n, m, i, j;
    double    smallest_constraint_value, temp;
    int     two_phase_flag;
    int     aux_leave_var, var;
    int     res;
    double    max_feasibility_error;


    UNTESTED_CODE();

#ifdef DEBUG
    ERE(copy_matrix(&db_constraint_mp, constraint_mp));
    ERE(copy_vector(&db_constraint_col_vp, constraint_col_vp));
#endif

    m = constraint_mp->num_rows;
    n = constraint_mp->num_cols;

    if (    (constraint_col_vp->length != m)
         || ((obj_row_vp != NULL) && (obj_row_vp->length != n))
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    smallest_constraint_value = 0.0;
    aux_leave_var = NOT_SET;

    for (i=0; i<m; i++)
    {
        if (constraint_col_vp->elements[i]<smallest_constraint_value)
        {
            smallest_constraint_value = constraint_col_vp->elements[i];
            aux_leave_var = i+1;
        }
     }

    if (smallest_constraint_value < -LP_FEASIBILITY_EPSILON)
    {
        two_phase_flag = TRUE;
        res = NO_ERROR;   /* Keep error checkers happy. */
    }
    else
    {
        two_phase_flag = FALSE;

        if (obj_row_vp==NULL)
        {
            for (i=0; i<n; i++)
            {
                result_vp->elements[i] = 0.0;
            }

            return NO_ERROR;
        }
        else
        {
            res = NO_ERROR;
        }
    }

    NRE(basis_var_index_vp     = create_int_vector(m));
    NRE(non_basis_var_index_vp = create_int_vector(n));
    NRE(basis_mp               = create_identity_matrix(m));
    NRE(inverse_basis_mp       = create_identity_matrix(m));
    NRE(basis_obj_row_vp       = create_zero_vector(m));
    NRE(temp_constraint_col_vp = create_vector(m));
    NRE(non_basis_mp           = create_matrix(m,n));
    NRE(non_basis_obj_row_vp   = create_vector(n));

    ERE(copy_vector(&solution_col_vp, constraint_col_vp));

    for (i=0; i<m; i++)
    {
        basis_var_index_vp->elements [i] = i+1+n;
    }

    if (two_phase_flag)
    {
        NRE(aux_non_basis_mp           = create_matrix(m,n+1));
        NRE(aux_non_basis_var_index_vp = create_int_vector(n+1));
        NRE(aux_result_vp              = create_vector(n+1));
        NRE(aux_non_basis_obj_row_vp   = create_zero_vector(n+1));
        NRE(aux_non_basis_mat_col_vp   = create_vector(m));

        aux_non_basis_obj_row_vp->elements[0] = -1.0;

        for (i=0; i<=n; i++)
        {
            aux_non_basis_var_index_vp->elements[i] = i;
        }
        for (i=0; i<m; i++)
        {
            basis_var_index_vp->elements[i] = i+1+n;
        }

        for (i=0; i<m;i++)
        {
            aux_non_basis_mp->elements[i][0] = -1.0;
        }
        for (j=1; j<n+1;j++)
        {
            ERE(get_matrix_col(&aux_non_basis_mat_col_vp, constraint_mp,
                               j-1));
            ERE(put_matrix_col(aux_non_basis_mp, aux_non_basis_mat_col_vp,j));
        }

        res = do_aux_simplex(basis_mp, inverse_basis_mp,
                             aux_non_basis_mp,
                             solution_col_vp,
                             basis_obj_row_vp,
                             aux_non_basis_obj_row_vp,
                             basis_var_index_vp,
                             aux_non_basis_var_index_vp,
                             aux_leave_var,
                             max_num_iterations, report,
                             aux_result_vp);

        if (res == NO_ERROR)
        {
            if (aux_result_vp->elements[0] > LP_FEASIBILITY_EPSILON)
            {
                res = LP_PROBLEM_IS_INFEASIBLE;
            }
        }

        if (res == NO_ERROR)
        {
            for (i=0;i<n;i++)
            {
                result_vp->elements[i] = aux_result_vp->elements[i+1] ;
            }
        }

        if (res == NO_ERROR)
        {
            /*
                Check Feasible
            */
            max_feasibility_error = 0;

            ERE(multiply_matrix_and_vector(&temp_constraint_col_vp,
                                           constraint_mp, result_vp));

            for (i=0; i<m; i++)
            {
                temp = temp_constraint_col_vp->elements[i] -
                                        constraint_col_vp->elements[i];

                if (temp > LP_AUX_FEASIBILITY_CHECK_EPSILON)
                {
                    kjb_fprintf(stderr,
                                "Proposed feasible may be infeasible. ");
                    kjb_fprintf(stderr,
                                "Constraint %d off by %e\n", (i+1), temp);
                }

                if (temp > max_feasibility_error)
                {
                    max_feasibility_error = temp;
                }
            }

            /*
            kjb_fprintf(stderr, "AUX Max constraint feasibility error is %e.\n",
                        max_feasibility_error);
            */

            for (i=0; i<n; i++)
            {
                temp = -(result_vp->elements[i]);

                if (temp > LP_AUX_FEASIBILITY_CHECK_EPSILON)
                {
                    kjb_fprintf(stderr,
                                "Proposed feasible may be infeasible. ");
                    kjb_fprintf(stderr, "Variable %d is %e\n", (i+1),
                                (result_vp->elements[i]));
                }
                if (temp > max_feasibility_error)
                {
                    max_feasibility_error = temp;
                }
            }
            /*
            kjb_fprintf(stderr, "AUX Max overall feasibility error is %e.\n",
                        max_feasibility_error);
            */
        }

        if ((res == NO_ERROR) && (obj_row_vp != NULL))
        {
            j = 0;

            for (i=0; i<=n; i++)
            {
                var = aux_non_basis_var_index_vp->elements[i];

                if (var!=0)
                {
                    non_basis_var_index_vp->elements[j] = var;

                    ERE(get_matrix_col(&aux_non_basis_mat_col_vp,
                                       aux_non_basis_mp, i));
                    ERE(put_matrix_col(non_basis_mp, aux_non_basis_mat_col_vp,
                                       j));
                    j++;
                }
            }

            for (i=0; i<n; i++)
            {
                var = non_basis_var_index_vp->elements[i];

                if (var <= n)
                {
                    non_basis_obj_row_vp->elements[i]=
                                         obj_row_vp->elements[var-1];
                }
                else
                {
                    non_basis_obj_row_vp->elements[i] = 0.0;
                }
            }

            for (i=0; i<m; i++)
            {
                var = basis_var_index_vp->elements[i];

                if (var <= n)
                {
                    basis_obj_row_vp->elements[i]=
                                         obj_row_vp->elements[var-1];
                }
                else
                {
                    basis_obj_row_vp->elements[i] = 0.0;
                }
            }

        }
        free_matrix(aux_non_basis_mp);
        free_int_vector(aux_non_basis_var_index_vp);
        free_vector(aux_result_vp);
        free_vector(aux_non_basis_obj_row_vp);
        free_vector(aux_non_basis_mat_col_vp);
    }
    else
    {
        ERE(copy_matrix(&non_basis_mp, constraint_mp));
        ERE(copy_vector(&non_basis_obj_row_vp,obj_row_vp));

        for (i=0; i<n; i++)
        {
            non_basis_var_index_vp->elements [i] = i+1;
        }

    }

    if ((res == NO_ERROR) && (obj_row_vp != NULL))
    {
        res = do_reg_simplex(basis_mp, inverse_basis_mp,
                             non_basis_mp, solution_col_vp,
                             basis_obj_row_vp, non_basis_obj_row_vp,
                             basis_var_index_vp,
                             non_basis_var_index_vp,
                             max_num_iterations, report,
                             result_vp);

        if (res == NO_ERROR)
        {
            /*
                Check Feasible
            */
            max_feasibility_error = 0;

            ERE(multiply_matrix_and_vector(&temp_constraint_col_vp,
                                           constraint_mp, result_vp));

            for (i=0; i<m; i++)
            {
                temp = temp_constraint_col_vp->elements[i] -
                                        constraint_col_vp->elements[i];

                if (temp > LP_FEASIBILITY_CHECK_EPSILON)
                {
                    kjb_fprintf(stderr,
                                "Proposed solution may be infeasible. ");
                    kjb_fprintf(stderr,
                                "Constraint %d off by %f\n", (i+1), temp);
                }

                if (temp > max_feasibility_error)
                {
                    max_feasibility_error = temp;
                }
            }

            /*
            kjb_fprintf(stderr, "Max constraint feasibility error is %f.\n",
                        max_feasibility_error);
            */

            for (i=0; i<n; i++)
            {
                temp = -(result_vp->elements[i]);

                if (temp > LP_FEASIBILITY_CHECK_EPSILON)
                {
                    kjb_fprintf(stderr,
                                "Proposed feasible may be infeasible. ");
                    kjb_fprintf(stderr, "Variable %d is %f\n", (i+1),
                                (result_vp->elements[i]));
                }

                if (temp > max_feasibility_error)
                {
                    max_feasibility_error = temp;
                }
            }

        }
    }

#ifdef DEBUG
    if (kjb_debug_level >= 10)
    {
        dbm("Grand result is:");
        db_cv(result_vp);
    }
#endif
    free_matrix(inverse_basis_mp);
    free_int_vector(basis_var_index_vp);
    free_int_vector(non_basis_var_index_vp);
    free_vector(solution_col_vp);
    free_matrix(basis_mp);
    free_vector(basis_obj_row_vp);
    free_vector(temp_constraint_col_vp);
    free_matrix(non_basis_mp);
    free_vector(non_basis_obj_row_vp);

    return res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int do_reg_simplex
(
    Matrix* basis_mp,
    Matrix* inverse_basis_mp,
    Matrix* non_basis_mp,
    Vector* solution_col_vp,
    Vector* basis_obj_row_vp,
    Vector* non_basis_obj_row_vp,
    Int_vector* basis_var_index_vp,
    Int_vector* non_basis_var_index_vp,
    int     max_num_iterations,
    int     report,
    Vector* result_vp
)
{
    Vector* y_row_vp;
    Vector* a_col_vp;
    Vector* d_col_vp;
    Vector* temp_d_col_vp;
    Vector* leave_col_vp;
    Vector* temp_solution_col_vp;
    int     n, m, i;
    int     index_temp;
    int     res;


    UNTESTED_CODE();

    m = basis_mp->num_rows;
    n = non_basis_mp->num_cols;

    NRE(a_col_vp             = create_vector(m));
    NRE(d_col_vp             = create_vector(m));
    NRE(temp_d_col_vp        = create_vector(m));
    NRE(y_row_vp             = create_vector(m));
    NRE(leave_col_vp         = create_vector(m));
    NRE(temp_solution_col_vp = create_vector(m));

    res = do_simplex_guts(basis_mp, non_basis_mp, solution_col_vp,
                          basis_obj_row_vp,
                          non_basis_obj_row_vp,
                          basis_var_index_vp,
                          non_basis_var_index_vp,
                          max_num_iterations,
                          report,
                          inverse_basis_mp,
                          a_col_vp, d_col_vp, temp_d_col_vp, y_row_vp,
                          leave_col_vp, temp_solution_col_vp);


    if (res == NO_ERROR)
    {
         for (i=0; i<n; i++)
         {
             result_vp->elements [i] = 0.0;
         }

         for (i=0; i<m; i++)
         {
             index_temp = basis_var_index_vp->elements[i];

             if (index_temp <= n)
             {
                 result_vp->elements[index_temp-1] =
                                        solution_col_vp->elements[i];
             }
         }
     }

    free_vector(a_col_vp);
    free_vector(d_col_vp);
    free_vector(y_row_vp);
    free_vector(temp_d_col_vp);
    free_vector(leave_col_vp);
    free_vector(temp_solution_col_vp);

    return res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int do_simplex_guts
(
    Matrix* basis_mp,
    Matrix* non_basis_mp,
    Vector* solution_col_vp,
    Vector* basis_obj_row_vp,
    Vector* non_basis_obj_row_vp,
    Int_vector* basis_var_index_vp,
    Int_vector* non_basis_var_index_vp,
    int     max_num_iterations,
    int     report,
    Matrix* inverse_basis_mp,
    Vector* a_col_vp,
    Vector* d_col_vp,
    Vector* temp_d_col_vp,
    Vector* y_row_vp,
    Vector* leave_col_vp,
    Vector* temp_solution_col_vp
)
{
    double y_a_prod;
    int n, m, i, j, ent_col, leave_col;
    int num_iterations;
    double t, temp, max;
    int index_temp;
#ifdef DEBUG
    IMPORT int kjb_debug_level;
    Vector*    db_result_vp;
    Vector*    db_temp_col_vp;
    Vector*    db_diff_vp = NULL;
    Matrix*    db_temp_mp;
    double       diff;
#endif


    m = basis_mp->num_rows;
    n = non_basis_mp->num_cols;

#ifdef DEBUG
    NRE(db_result_vp   = create_vector(n));
    NRE(db_temp_col_vp = create_vector(m));
    NRE(db_temp_mp     = create_matrix(m, 3));

    if (kjb_debug_level > 20)
    {
        dbm("In do regular simplex guts");
    }
#endif

    num_iterations = 0;

    while (num_iterations < max_num_iterations)
    {
#ifdef DEBUG
         if (kjb_debug_level >= 50)
         {
             for (i=0; i<n; i++)
             {
                 db_result_vp->elements [i] = 0.0;
             }

             for (i=0; i<m; i++)
             {
                 index_temp = basis_var_index_vp->elements[i];

                 if (index_temp <= n)
                 {
                     db_result_vp->elements[index_temp-1] =
                                            solution_col_vp->elements[i];
                 }
             }

            ERE(multiply_matrix_and_vector(&db_temp_col_vp, db_constraint_mp,
                                           db_result_vp));

            ERE(subtract_vectors(&db_diff_vp, db_constraint_col_vp,
                                 db_temp_col_vp));

            ERE(put_matrix_col(db_temp_mp, db_temp_col_vp, 0));
            ERE(put_matrix_col(db_temp_mp, db_constraint_col_vp, 1));
            ERE(put_matrix_col(db_temp_mp, db_diff_vp, 2));

            diff = min_vector_element(db_diff_vp);

            dbe(diff);

            if (kjb_debug_level >= 100)
            {
                db_cv(db_result_vp);
                db_mat(db_temp_mp);
            }
        }

#endif

        /* Step 1 */

        ERE(multiply_vector_and_matrix(&y_row_vp, basis_obj_row_vp,
                                       inverse_basis_mp));

        /* Step 2 */

        ent_col = NOT_SET;
        max = 0.0;

        for (i=1; i <= n; i++)
        {
            ERE(get_matrix_col(&a_col_vp, non_basis_mp, i-1));

            y_a_prod = 0.0;

            for (j=0; j<y_row_vp->length; j++)
            {
                y_a_prod += (y_row_vp->elements)[j] * (a_col_vp->elements)[j];
            }

            temp = (non_basis_obj_row_vp->elements)[i-1] - y_a_prod;

            if (temp > LP_ENTER_EPSILON)
            {
                if (temp > max)
                {
                    max = temp;
                    ent_col = i;
                }
            }
        }

        if (ent_col == NOT_SET)
        {
            if (report > 0)
            {
                term_blank_out_line();
                pso("%d iterations used.\n", num_iterations);
                fflush(stdout);
            }

            return NO_ERROR; /* Solved ! */
        }

        ERE(get_matrix_col(&a_col_vp, non_basis_mp, ent_col-1));

        /* Step 3 */

        ERE(multiply_matrix_and_vector(&d_col_vp,inverse_basis_mp, a_col_vp));

        /* Step FOUR */

        leave_col = NOT_SET;
        t = 0.0;    /* Keep error checkers happy. */

        for (i=0; i<m; i++)
        {
            if ((d_col_vp->elements)[i] > LP_LEAVE_EPSILON)
            {
                temp = (solution_col_vp->elements)[i]/ (d_col_vp->elements)[i];

                if (leave_col == NOT_SET)
                {
                    t = temp;
                    leave_col = i + 1;
                }
                else if (temp < t)
                {
                    t = temp;
                    leave_col = i + 1;
                }
            }
        }

        if (leave_col == NOT_SET)
        {
            return LP_PROBLEM_IS_UNBOUNDED;
        }

        if (t<0.0) t = 0.0;

        /* Step FIVE */

        ERE(simplex_update_inverse(m,inverse_basis_mp,leave_col,d_col_vp,
                                   temp_d_col_vp));

        ERE(ow_multiply_vector_by_scalar(d_col_vp, t));
        ERE(copy_vector(&temp_solution_col_vp, solution_col_vp));
        ERE(subtract_vectors(&solution_col_vp, temp_solution_col_vp,d_col_vp));

        (solution_col_vp->elements)[leave_col-1] = t;

        ERE(get_matrix_col(&leave_col_vp,basis_mp,leave_col-1));
        ERE(put_matrix_col(non_basis_mp,leave_col_vp, ent_col-1));
        ERE(put_matrix_col(basis_mp, a_col_vp, leave_col-1));

        temp = basis_obj_row_vp->elements[leave_col-1];
        basis_obj_row_vp->elements[leave_col-1] =
                                    non_basis_obj_row_vp->elements[ent_col-1];
        non_basis_obj_row_vp->elements[ent_col-1] = temp;

        index_temp = basis_var_index_vp->elements [leave_col-1];
        basis_var_index_vp->elements[leave_col-1] =
                               non_basis_var_index_vp->elements [ent_col-1];
        non_basis_var_index_vp->elements [ent_col-1] = index_temp;

        num_iterations++;

        if (report > 0)
        {
            if ((num_iterations % report == 0) && (kjb_isatty(fileno(stdout))))
            {
                pso("\rIteration %d", num_iterations);
                fflush(stdout);
            }
        }
    }

    term_blank_out_line();
    fflush(stdout);

    return LP_MAX_NUM_ITERATIONS_EXCEEDED;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int do_aux_simplex
(
    Matrix* basis_mp,
    Matrix* inverse_basis_mp,
    Matrix* non_basis_mp,
    Vector* solution_col_vp,
    Vector* basis_obj_row_vp,
    Vector* non_basis_obj_row_vp,
    Int_vector* basis_var_index_vp,
    Int_vector* non_basis_var_index_vp,
    int     leave_col,
    int     max_num_iterations,
    int     report,
    Vector* result_vp
)
{
    Vector* y_row_vp;
    Vector* a_col_vp = NULL;
    Vector* d_col_vp;
    Vector* temp_d_col_vp;
    Vector* leave_col_vp = NULL;
    Vector* temp_solution_col_vp;
    int     n, m, i, ent_col;
    double    temp;
    int     index_temp;
    int     res;


    UNTESTED_CODE();

#ifdef DEBUG
    dbi(leave_col);
#endif

    m = basis_mp->num_rows;
    n = non_basis_mp->num_cols;

    NRE(d_col_vp             = create_vector(m));
    NRE(temp_d_col_vp        = create_vector(m));
    NRE(y_row_vp             = create_vector(m));
    NRE(temp_solution_col_vp = create_vector(m));

    ent_col = 1;

    ERE(get_matrix_col(&a_col_vp, non_basis_mp, 0));
    ERE(get_matrix_col(&leave_col_vp, basis_mp, leave_col-1));
    ERE(put_matrix_col(non_basis_mp,leave_col_vp, ent_col-1));
    ERE(put_matrix_col(basis_mp, a_col_vp, leave_col-1));

    temp = basis_obj_row_vp->elements[leave_col-1];

    basis_obj_row_vp->elements[leave_col-1] =
                   non_basis_obj_row_vp->elements[ent_col-1];

    non_basis_obj_row_vp->elements[ent_col-1] = temp;

    index_temp = basis_var_index_vp->elements [leave_col-1];

    basis_var_index_vp->elements[leave_col-1] =
              non_basis_var_index_vp->elements[ent_col-1];

    non_basis_var_index_vp->elements [ent_col-1] = index_temp;

    /*
        In the current case, where B = I, d_col = a_col.
    */
    ERE(simplex_update_inverse(m, inverse_basis_mp, leave_col, a_col_vp,
                               temp_d_col_vp));

    ERE(multiply_matrix_and_vector(&temp_solution_col_vp, inverse_basis_mp,
                                   solution_col_vp));

    ERE(copy_vector(&solution_col_vp, temp_solution_col_vp));

    res = do_aux_simplex_guts(basis_mp, non_basis_mp, solution_col_vp,
                              basis_obj_row_vp,
                              non_basis_obj_row_vp,
                              basis_var_index_vp,
                              non_basis_var_index_vp,
                              leave_col - 1,
                              max_num_iterations,
                              report,
                              inverse_basis_mp,
                              a_col_vp, d_col_vp,
                              temp_d_col_vp, y_row_vp,
                              leave_col_vp, temp_solution_col_vp);

    if (res == NO_ERROR)
    {
         for (i=0; i<n; i++)
         {
             result_vp->elements [i] = 0.0;
         }

         for (i=0; i<m; i++)
         {
             index_temp = basis_var_index_vp->elements[i];

             if (index_temp < n)
             {
                 result_vp->elements[index_temp] =
                                    solution_col_vp->elements[i];
             }
         }
     }

    free_vector(a_col_vp);
    free_vector(d_col_vp);
    free_vector(temp_d_col_vp);
    free_vector(y_row_vp);
    free_vector(leave_col_vp);
    free_vector(temp_solution_col_vp);

    return res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int do_aux_simplex_guts
(
    Matrix* basis_mp,
    Matrix* non_basis_mp,
    Vector* solution_col_vp,
    Vector* basis_obj_row_vp,
    Vector* non_basis_obj_row_vp,
    Int_vector* basis_var_index_vp,
    Int_vector* non_basis_var_index_vp,
    int     aux_var_basis_index,
    int     max_num_iterations,
    int     report,
    Matrix* inverse_basis_mp,
    Vector* a_col_vp,
    Vector* d_col_vp,
    Vector* temp_d_col_vp,
    Vector* y_row_vp,
    Vector* leave_col_vp,
    Vector* temp_solution_col_vp
)
{
    double y_a_prod;
    int  n, m, i, ent_col, leave_col;
    int  num_iterations;
    double t, temp, max;
    int  index_temp;
#ifdef DEBUG
    IMPORT int kjb_debug_level;
    Vector*    db_result_vp,   *db_aux_result_vp;
    Vector*    db_temp_col_vp;
    Vector*    db_diff_vp = NULL;
    Matrix*    db_temp_mp;
    double       diff;
#endif


    m = basis_mp->num_rows;
    n = non_basis_mp->num_cols;

#ifdef DEBUG
    NRE(db_aux_result_vp = create_vector(n));
    NRE(db_result_vp     = create_vector(n - 1));
    NRE(db_temp_col_vp   = create_vector(m));
    NRE(db_temp_mp       = create_matrix(m, 3));

    if (kjb_debug_level >= 10)
    {
        dbm("In do AUX simplex guts");
    }
#endif

    num_iterations = 0;

    while (num_iterations < max_num_iterations)
    {
#ifdef DEBUG
         if (kjb_debug_level >= 50)
         {
             for (i=0; i<n; i++)
             {
                 db_aux_result_vp->elements [i] = 0.0;
             }

             for (i=0; i<m; i++)
             {
                 index_temp = basis_var_index_vp->elements[i];

                 if (index_temp < n)
                 {
                     db_aux_result_vp->elements[index_temp] =
                                        solution_col_vp->elements[i];
                 }
             }

            for (i=0;i<n-1;i++)
            {
                db_result_vp->elements[i] = db_aux_result_vp->elements[i+1] ;
            }

            ERE(multiply_matrix_and_vector(&db_temp_col_vp, db_constraint_mp,
                                           db_result_vp));

            ERE(subtract_vectors(&db_diff_vp, db_constraint_col_vp,
                                 db_temp_col_vp));

            ERE(put_matrix_col(db_temp_mp, db_temp_col_vp, 0));
            ERE(put_matrix_col(db_temp_mp, db_constraint_col_vp, 1));
            ERE(put_matrix_col(db_temp_mp, db_diff_vp, 2));

            diff = min_vector_element(db_diff_vp);

            dbe(diff);
            db_cv(db_result_vp);

            if (kjb_debug_level >= 100)
            {
                db_mat(db_temp_mp);
            }

        }
#endif

        /* Step 1 */

        ERE(multiply_vector_and_matrix(&y_row_vp, basis_obj_row_vp,
                                       inverse_basis_mp));

        /* Step 2 */

        ent_col = NOT_SET;
        max = 0.0;

        for (i=1; i <= n; i++)
        {
            ERE(get_matrix_col(&a_col_vp, non_basis_mp, i-1));
            ERE(get_dot_product(y_row_vp, a_col_vp, &y_a_prod));

            temp = (non_basis_obj_row_vp->elements)[i-1] - y_a_prod;

            if (temp > LP_AUX_ENTER_EPSILON)
            {
                if (temp > max)
                {
                    max = temp;
                    ent_col = i;
                }
            }
        }

        if (ent_col == NOT_SET)
        {
            if (report > 0)
            {
                term_blank_out_line();
                pso("%d iterations used.\n", num_iterations);
                fflush(stdout);
            }

            return NO_ERROR; /* Solved ! */
        }

        ERE(get_matrix_col(&a_col_vp, non_basis_mp, ent_col-1));

        /* Step 3 */

        ERE(multiply_matrix_and_vector(&d_col_vp,inverse_basis_mp,
                                       a_col_vp));

        /* Step FOUR */

        leave_col = NOT_SET;
        t = 0.0;    /* Keep error checkers happy. */

        for (i=0; i<m; i++)
        {
            if ((d_col_vp->elements)[i] > LP_LEAVE_EPSILON)
            {
                temp = ((solution_col_vp->elements)[i])/
                                           ((d_col_vp->elements)[i]);

                if (leave_col == NOT_SET)
                {
                    t = temp;
                    leave_col = i + 1;
                }
                else if (temp < t)
                {
                    t = temp;
                    leave_col = i + 1;
                }
            }
        }

        if (leave_col == NOT_SET)
        {
            return LP_PROBLEM_IS_UNBOUNDED;
        }

        if (t<0.0) t = 0.0;

        /*
           If it was possible to select X0, then we shoud have selected it.
           It is possible that it was shunned for a candidate that was
           only slightly better, by LP_AUX_USE_X_ZERO_EPSILON. In this case,
           use X0 instead. See Chavatal page 42 for more.
        */
        if ((aux_var_basis_index != NOT_SET) &&
            (aux_var_basis_index != leave_col - 1))
        {
            if ((d_col_vp->elements)[aux_var_basis_index] > LP_LEAVE_EPSILON)
            {
                temp = ((solution_col_vp->elements)[aux_var_basis_index])/
                                           ((d_col_vp->elements)[aux_var_basis_index]);

                if (temp < t + LP_AUX_USE_X_ZERO_EPSILON)
                {
                    t = temp;
                    leave_col = aux_var_basis_index + 1;
                }
            }
        }

        /* Step FIVE */

        ERE(simplex_update_inverse(m,inverse_basis_mp,leave_col,d_col_vp,
                                   temp_d_col_vp));

        ERE(ow_multiply_vector_by_scalar(d_col_vp, t));
        ERE(copy_vector(&temp_solution_col_vp, solution_col_vp));
        ERE(subtract_vectors(&solution_col_vp,temp_solution_col_vp,d_col_vp));

        (solution_col_vp->elements)[leave_col-1] = t;

        ERE(get_matrix_col(&leave_col_vp, basis_mp,leave_col-1));
        ERE(put_matrix_col(non_basis_mp,leave_col_vp,
                         ent_col-1));
        ERE(put_matrix_col(basis_mp, a_col_vp, leave_col-1));

        temp = basis_obj_row_vp->elements[leave_col-1];
        basis_obj_row_vp->elements[leave_col-1] =
                       non_basis_obj_row_vp->elements[ent_col-1];
        non_basis_obj_row_vp->elements[ent_col-1] = temp;

        index_temp = basis_var_index_vp->elements [leave_col-1];
        basis_var_index_vp->elements[leave_col-1] =
                  non_basis_var_index_vp->elements [ent_col-1];

        if (index_temp == 0)
        {
            aux_var_basis_index = NOT_SET;
        }
        else if (non_basis_var_index_vp->elements[ent_col-1]==0)
        {
            aux_var_basis_index = ent_col-1;
        }

        non_basis_var_index_vp->elements [ent_col-1] = index_temp;

        num_iterations++;

        if (report > 0)
        {
            if ((num_iterations % report == 0) && (kjb_isatty(fileno(stdout))))
            {
                pso("\rIteration %d", num_iterations);
                fflush(stdout);
            }
        }
    }

    term_blank_out_line();
    fflush(stdout);

    return LP_MAX_NUM_ITERATIONS_EXCEEDED;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int simplex_update_inverse
(
    int     m,
    Matrix* inverse_mp,
    int     new_col,
    Vector* d_col_vp,
    Vector* temp_d_col_vp
)
{
    int    i;
    int    j;
    double pivot_elem_neg_inv;
    double*  temp_d_pos;
    double*  d_pos;
    double*  m_pos;
    double*  m_row_pos;
    double*  save_m_row_pos;


    UNTESTED_CODE();

    new_col--;
    pivot_elem_neg_inv = -1.0 / (d_col_vp->elements[new_col]);

    temp_d_pos = temp_d_col_vp->elements;
    d_pos = d_col_vp->elements;

    for (j=0; j<m; j++)
    {
        if (j==new_col)
        {
            *temp_d_pos = -pivot_elem_neg_inv;
         }
        else
        {
            *temp_d_pos = (*d_pos) * pivot_elem_neg_inv;
        }
        d_pos++;
        temp_d_pos++;
    }

    save_m_row_pos = inverse_mp->elements[new_col];
    temp_d_pos = temp_d_col_vp->elements;

    for (i=0; i<m; i++)
    {
        if (i != new_col)
        {
            m_pos = inverse_mp->elements[i];
            m_row_pos = save_m_row_pos;

            for (j=0; j<m; j++)
            {
                *m_pos += (*m_row_pos) * (*temp_d_pos);
                m_pos++;
                m_row_pos++;
            }
        }
        temp_d_pos++;
    }

    m_pos = save_m_row_pos;
    temp_d_pos = temp_d_col_vp->elements + new_col;

    for (j=0; j<m; j++)
    {
        *m_pos = (*m_pos) * (*temp_d_pos);
        m_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int do_gen_simplex
(
    Matrix* le_constraint_mp,
    Vector* le_constraint_col_vp,
    Matrix* eq_constraint_mp,
    Vector* eq_constraint_col_vp,
    Vector* lb_row_vp,
    Vector* ub_row_vp,
    Vector* obj_row_vp,
    int     max_num_iterations,
    int     report,
    Vector* initial_solution_vp,
    Vector* result_vp
)
{
    Matrix* basis_mp  = NULL;
    Matrix* inverse_basis_mp = NULL;
    Matrix* non_basis_mp;
    Vector* result_with_slack_vp;
    Vector* non_basis_row_vp = NULL;
    Vector* aux_non_basis_mat_col_vp;
    Vector* aux_non_basis_obj_row_vp;
    Vector* aux_basis_obj_row_vp;
    Vector* basis_obj_row_vp;
    Vector* non_basis_obj_row_vp;
    Int_vector* basis_var_index_vp;
    Int_vector* non_basis_var_index_vp;
    Vector* solution_col_vp;
    Vector* non_basis_solution_col_vp;
    Vector* temp_constraint_row_vp = NULL;
    Vector* basis_lb_row_vp;
    Vector* basis_ub_row_vp;
    Vector* non_basis_lb_row_vp;
    Vector* non_basis_ub_row_vp;
    int     eq_n, eq_m, le_n, le_m, non_slack_n;
    int     n, m, i, j;
    int     temp_index;
    double    temp;
    int     res;
    double    lb, ub, dot_product;
    int     two_phase_flag;


    UNTESTED_CODE();

    if (le_constraint_mp == NULL)
    {
        le_n = le_m = 0;
    }
    else
    {
        le_m = le_constraint_mp->num_rows;
        le_n = le_constraint_mp->num_cols;

        if ((le_constraint_col_vp == NULL) ||
            (le_constraint_col_vp->length != le_m))
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    if (eq_constraint_mp == NULL)
    {
        eq_n = eq_m = 0;
    }
    else
    {
        eq_m = eq_constraint_mp->num_rows;
        eq_n = eq_constraint_mp->num_cols;

        if ((eq_constraint_col_vp == NULL) ||
            (eq_constraint_col_vp->length != eq_m))
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    if ((le_n != 0) && (eq_n != 0))
    {
        if (le_n != eq_n)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
        n = le_n;
    }
    else if (le_n != 0)
    {
        n = le_n;
    }
    else if (eq_n != 0)
    {
        n = eq_n;
    }
    else
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (    (result_vp == NULL)
         || ((obj_row_vp != NULL) && (obj_row_vp->length != n))
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    non_slack_n = n;
    m = le_m + eq_m;
    n += le_m;

    NRE(basis_var_index_vp        = create_int_vector(m));
    NRE(non_basis_var_index_vp    = create_int_vector(n));

    NRE(non_basis_mp              = create_matrix(m, n));

    NRE(basis_obj_row_vp          = create_zero_vector(m));
    NRE(non_basis_obj_row_vp      = create_zero_vector(n));
    NRE(aux_basis_obj_row_vp      = create_vector(m));
    NRE(aux_non_basis_obj_row_vp  = create_zero_vector(n));

    NRE(basis_lb_row_vp           = create_vector(m));
    NRE(basis_ub_row_vp           = create_vector(m))
    NRE(non_basis_lb_row_vp       = create_vector(n));
    NRE(non_basis_ub_row_vp       = create_vector(n));

    NRE(result_with_slack_vp      = create_vector(n+m));
    NRE(aux_non_basis_mat_col_vp  = create_vector(m));
    NRE(non_basis_solution_col_vp = create_vector(n));
    NRE(solution_col_vp           = create_vector(m));

    two_phase_flag = FALSE;

    for (i=0; i<le_m; i++)
    {
        for (j=0; j<non_slack_n; j++)
        {
            (non_basis_mp->elements)[i][j] = (le_constraint_mp->elements)[i][j];
        }
        for (j = 0; j<le_m; j++)
        {
            if (i == j)
            {
                (non_basis_mp->elements)[i][le_n+j] = 1.0;
            }
            else
            {
                (non_basis_mp->elements)[i][le_n+j] = 0.0;
            }
        }
    }

    for (i=0; i<eq_m; i++)
    {
        for (j=0; j<non_slack_n; j++)
        {
            (non_basis_mp->elements)[le_m + i][j] =
                                    (eq_constraint_mp->elements)[i][j];
        }
        for (j = 0; j<le_m; j++)
        {
            (non_basis_mp->elements)[le_m + i][le_n+j] = 0.0;
        }
    }

    for (j=0; j<non_slack_n; j++)
    {
        (non_basis_lb_row_vp->elements)[j] = (lb_row_vp->elements)[j];
        (non_basis_ub_row_vp->elements)[j] = (ub_row_vp->elements)[j];
    }
    for (j = 0; j<le_m; j++)
    {
        (non_basis_lb_row_vp->elements)[j+non_slack_n] = 0.0;
        (non_basis_ub_row_vp->elements)[j+non_slack_n] = DBL_MOST_POSITIVE;
    }


    for (j=0; j<non_slack_n; j++)
    {
        lb = (non_basis_lb_row_vp->elements)[j];
        ub = (non_basis_ub_row_vp->elements)[j];

        if ((initial_solution_vp != NULL) &&
            (j < initial_solution_vp->length))
        {
            temp = initial_solution_vp->elements[j];

            non_basis_solution_col_vp->elements[j] = temp;

            if (    (IS_GREATER_DBL(temp, ub))
                 || (IS_LESSER_DBL(temp, lb))
                )
            {
                two_phase_flag = TRUE;
            }
        }
        else if (IS_GREATER_DBL(lb, DBL_MOST_NEGATIVE))
        {
            non_basis_solution_col_vp->elements[j] = lb;
        }
        else if (IS_LESSER_DBL(ub, DBL_MOST_POSITIVE))
        {
            non_basis_solution_col_vp->elements[j] = ub;
        }
        else
        {
            non_basis_solution_col_vp->elements[j] = 0.0;
        }
    }

    for (j=0; j<le_m; j++)
    {
        ERE(get_matrix_row(&temp_constraint_row_vp, le_constraint_mp,j));

        non_basis_solution_col_vp->length = non_slack_n;

        ERE(get_dot_product(temp_constraint_row_vp, non_basis_solution_col_vp,
                            &dot_product));

        non_basis_solution_col_vp->length = n;

        temp = (le_constraint_col_vp->elements)[j] - dot_product;

        if (temp >= 0.0)
        {
            (non_basis_solution_col_vp->elements)[j+non_slack_n] = temp;
        }
        else
        {
            (non_basis_solution_col_vp->elements)[j+non_slack_n] = 0;
         }

        if (temp < - LP_FEASIBILITY_EPSILON)
        {
            two_phase_flag = TRUE;
        }
    }

    for (j=0; j<m; j++)
    {
        ERE(get_matrix_row(&non_basis_row_vp, non_basis_mp, j));

        ERE(get_dot_product(non_basis_row_vp, non_basis_solution_col_vp,
                            &dot_product));

        if (j < le_m)
        {
            temp = (le_constraint_col_vp->elements)[j] - dot_product;
        }
        else
        {
            temp =(eq_constraint_col_vp->elements)[j-le_m]-dot_product;
        }

        (solution_col_vp->elements)[j] = temp;

        if (temp < 0.0)
        {
            (basis_lb_row_vp->elements)[j] = DBL_MOST_NEGATIVE;
            (basis_ub_row_vp->elements)[j] = 0.0;
        }
        else
        {
            (basis_lb_row_vp->elements)[j] = 0.0;
            (basis_ub_row_vp->elements)[j] = DBL_MOST_POSITIVE;
        }

        if (fabs(temp) >  LP_FEASIBILITY_EPSILON)
        {
            two_phase_flag = TRUE;
        }
    }

    if (two_phase_flag)
    {
        ERE(get_identity_matrix(&basis_mp, m));
        ERE(get_identity_matrix(&inverse_basis_mp,m));

        for (i=0; i<n; i++)
        {
            non_basis_var_index_vp->elements[i] = i+1;
        }
        for (i=0; i<m; i++)
        {
            basis_var_index_vp->elements[i] = i+1+n;
        }

        for (j=0; j<m; j++)
        {
            if ((basis_ub_row_vp->elements)[j] == 0.0)
            {
                (aux_basis_obj_row_vp->elements)[j] = 1.0;
            }
            else
            {
                (aux_basis_obj_row_vp->elements)[j] = -1.0;
            }
        }

        res = do_reg_gen_simplex(basis_mp,
                                 inverse_basis_mp,
                                 non_basis_mp,
                                 solution_col_vp,
                                 non_basis_solution_col_vp,
                                 aux_basis_obj_row_vp,
                                 aux_non_basis_obj_row_vp,
                                 basis_lb_row_vp,
                                 non_basis_lb_row_vp,
                                 basis_ub_row_vp,
                                 non_basis_ub_row_vp,
                                 basis_var_index_vp,
                                 non_basis_var_index_vp,
                                 max_num_iterations,
                                 report,
                                 result_with_slack_vp);

        if (res == NO_ERROR)
        {
            for (i=0; i<m; i++)
            {
                temp = fabs(result_with_slack_vp->elements[i+n]);

                if (temp > LP_FEASIBILITY_EPSILON)
                {
                    res = LP_PROBLEM_IS_INFEASIBLE;
                }
            }
        }
    }
    else
    {
        NRE(basis_mp = create_matrix(m, m));

        res = NO_ERROR;

        if (m >= n)
        {
            res = LP_PROBLEM_IS_OVERCONSTRAINED;
        }
        else
        {
            if (obj_row_vp == NULL)
            {
                ERE(copy_vector(&result_with_slack_vp,
                                non_basis_solution_col_vp));
            }
            else
            {
                for (i=0; i<m; i++)
                {
                    for (j=0; j<m; j++)
                    {
                        (basis_mp->elements)[i][j] =
                                   (non_basis_mp->elements)[i][n-m+j];
                    }
                }

                if (get_matrix_inverse(&inverse_basis_mp, basis_mp) == ERROR)
                {
                    res = LP_PROBLEM_IS_OVERCONSTRAINED;
                }

                non_basis_mp->num_cols = n-m;

                for (i=0; i<n-m; i++)
                {
                    non_basis_var_index_vp->elements[i] = i+1;
                }
                for (i=0; i<m; i++)
                {
                    basis_var_index_vp->elements[i] = i+1+n-m;
                }
                non_basis_var_index_vp->length = n-m;
                non_basis_lb_row_vp->length = n-m;
                non_basis_ub_row_vp->length = n-m;
                result_with_slack_vp->length = n-m;
            }
        }
    }

    if ((res == NO_ERROR) && (obj_row_vp != NULL))
    {
        if (two_phase_flag)
        {
            for (i=0; i<n; i++)
            {
                temp_index = (non_basis_var_index_vp->elements)[i];

                /*
                    If var is original, then fix objective.
                */
                if (temp_index <= non_slack_n)
                {
                    (non_basis_obj_row_vp->elements)[i] =
                                        (obj_row_vp->elements)[temp_index-1];
                }
                /*
                    If var is aux slack then lock at zero.
                */
                else if (temp_index > n)
                {
                    (non_basis_solution_col_vp->elements)[i] = 0.0;
                    (non_basis_lb_row_vp->elements)[i] = 0.0;
                    (non_basis_ub_row_vp->elements)[i] = 0.0;
                }
            }

            for (i=0; i<m; i++)
            {
                temp_index = (basis_var_index_vp->elements)[i];

                /*
                    If var is original, then fix objective.
                */
                if (temp_index <= non_slack_n)
                {
                    (basis_obj_row_vp->elements)[i] =
                                    (obj_row_vp->elements)[temp_index-1];
                }
                /*
                    If var is aux slack then lock at zero.
                */
                else if (temp_index > n)
                {
                    (solution_col_vp->elements)[i] = 0.0;
                    (basis_lb_row_vp->elements)[i] = 0.0;
                    (basis_ub_row_vp->elements)[i] = 0.0;
                }
            }
        }
        else
        {
            non_basis_obj_row_vp->length = n-m;

            for (i=0; i<n-m; i++)
            {
                (non_basis_obj_row_vp->elements)[i] =
                                            (obj_row_vp->elements)[i];
            }
            for (i=n-m; i<non_slack_n; i++)
            {
                (basis_obj_row_vp->elements)[i-n+m] =
                                            (obj_row_vp->elements)[i];
                (solution_col_vp->elements)[i-n+m] =
                             (non_basis_solution_col_vp->elements)[i];
                (basis_lb_row_vp->elements)[i-n+m] =
                                    (non_basis_lb_row_vp->elements)[i];
                (basis_ub_row_vp->elements)[i-n+m] =
                                    (non_basis_ub_row_vp->elements)[i];

            }
            for (i=non_slack_n; i<n; i++)
            {
                (basis_obj_row_vp->elements)[i-n+m] = 0.0;

                (solution_col_vp->elements)[i-n+m] =
                             (non_basis_solution_col_vp->elements)[i];
                (basis_lb_row_vp->elements)[i-n+m] =
                                    (non_basis_lb_row_vp->elements)[i];
                (basis_ub_row_vp->elements)[i-n+m] =
                                    (non_basis_ub_row_vp->elements)[i];
            }
            non_basis_solution_col_vp->length = n-m;
            non_basis_ub_row_vp->length = n-m;
            non_basis_lb_row_vp->length = n-m;
        }

        res = do_reg_gen_simplex(basis_mp, inverse_basis_mp,
                                 non_basis_mp,
                                 solution_col_vp,
                                 non_basis_solution_col_vp,
                                 basis_obj_row_vp,
                                 non_basis_obj_row_vp,
                                 basis_lb_row_vp,
                                 non_basis_lb_row_vp,
                                 basis_ub_row_vp,
                                 non_basis_ub_row_vp,
                                 basis_var_index_vp,
                                 non_basis_var_index_vp,
                                 max_num_iterations,
                                 report,
                                 result_with_slack_vp);

    }

    if (res == NO_ERROR)
    {
        for (i=0;i<non_slack_n;i++)
        {
            result_vp->elements[i] = result_with_slack_vp->elements[i];
        }
    }

    free_matrix(basis_mp);
    free_matrix(inverse_basis_mp);

    free_int_vector(basis_var_index_vp);
    free_int_vector(non_basis_var_index_vp);

    free_matrix(non_basis_mp);
    free_vector(non_basis_row_vp);

    free_vector(basis_obj_row_vp);
    free_vector(non_basis_obj_row_vp);
    free_vector(aux_basis_obj_row_vp);
    free_vector(aux_non_basis_obj_row_vp);

    free_vector(basis_lb_row_vp);
    free_vector(basis_ub_row_vp);
    free_vector(non_basis_lb_row_vp);
    free_vector(non_basis_ub_row_vp);

    free_vector(result_with_slack_vp);
    free_vector(aux_non_basis_mat_col_vp);
    free_vector(non_basis_solution_col_vp);
    free_vector(temp_constraint_row_vp);
    free_vector(solution_col_vp);

    return res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int do_reg_gen_simplex
(
    Matrix* basis_mp,
    Matrix* inverse_basis_mp,
    Matrix* non_basis_mp,
    Vector* solution_col_vp,
    Vector* non_basis_solution_col_vp,
    Vector* basis_obj_row_vp,
    Vector* non_basis_obj_row_vp,
    Vector* basis_lb_row_vp,
    Vector* non_basis_lb_row_vp,
    Vector* basis_ub_row_vp,
    Vector* non_basis_ub_row_vp,
    Int_vector* basis_var_index_vp,
    Int_vector* non_basis_var_index_vp,
    int     max_num_iterations,
    int     report,
    Vector* result_vp
)
{
    Vector* y_row_vp;
    Vector* a_col_vp;
    Vector* d_col_vp;
    Vector* temp_d_col_vp;
    Vector* leave_col_vp;
    Vector* temp_solution_col_vp;
    int     n, m, i;
    int     index_temp;
    int     res;


    UNTESTED_CODE();

    m = basis_mp->num_rows;
    n = non_basis_mp->num_cols;

    NRE(a_col_vp             = create_vector(m));
    NRE(d_col_vp             = create_vector(m));
    NRE(temp_d_col_vp        = create_vector(m));
    NRE(y_row_vp             = create_vector(m));
    NRE(leave_col_vp         = create_vector(m));
    NRE(temp_solution_col_vp = create_vector(m));

    res = do_gen_simplex_guts(basis_mp, inverse_basis_mp,
                              non_basis_mp, solution_col_vp,
                              non_basis_solution_col_vp,
                              basis_obj_row_vp,
                              non_basis_obj_row_vp,
                              basis_lb_row_vp,
                              non_basis_lb_row_vp,
                              basis_ub_row_vp,
                              non_basis_ub_row_vp,
                              basis_var_index_vp,
                              non_basis_var_index_vp,
                              max_num_iterations,
                              report,
                              a_col_vp, d_col_vp, temp_d_col_vp,
                              y_row_vp, leave_col_vp,
                              temp_solution_col_vp);


    if (res == NO_ERROR)
    {
         for (i=0; i<n; i++)
         {
             index_temp = non_basis_var_index_vp->elements[i];

             result_vp->elements[index_temp-1] =
                                non_basis_solution_col_vp->elements[i];
         }

         for (i=0; i<m; i++)
         {
             index_temp = basis_var_index_vp->elements[i];

             result_vp->elements[index_temp-1] =
                                        solution_col_vp->elements[i];
         }
     }

    free_vector(a_col_vp);
    free_vector(d_col_vp);
    free_vector(temp_d_col_vp);
    free_vector(y_row_vp);
    free_vector(leave_col_vp);
    free_vector(temp_solution_col_vp);

    return res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int do_gen_simplex_guts
(
    Matrix* basis_mp,
    Matrix* inverse_basis_mp,
    Matrix* non_basis_mp,
    Vector* solution_col_vp,
    Vector* non_basis_solution_col_vp,
    Vector* basis_obj_row_vp,
    Vector* non_basis_obj_row_vp,
    Vector* basis_lb_row_vp,
    Vector* non_basis_lb_row_vp,
    Vector* basis_ub_row_vp,
    Vector* non_basis_ub_row_vp,
    Int_vector* basis_var_index_vp,
    Int_vector* non_basis_var_index_vp,
    int     max_num_iterations,
    int     report,
    Vector* a_col_vp,
    Vector* d_col_vp,
    Vector* temp_d_col_vp,
    Vector* y_row_vp,
    Vector* leave_col_vp,
    Vector* temp_solution_col_vp
)
{
    double y_a_prod;
    int  n, m, i, ent_col, leave_col;
    int  num_iterations;
    double t, temp, max;
    int  index_temp;
    double non_basis_x, ent_x, ent_ub, ent_lb, ub, lb;
    int  case_one, case_two;


    m = basis_mp->num_rows;
    n = non_basis_mp->num_cols;

    num_iterations = 0;

    leave_col = NOT_SET;

    while (num_iterations < max_num_iterations)
    {
        /* Step 1 */

        if (leave_col != LP_SKIP_LEAVE_AND_ENTER)
        {
            ERE(multiply_vector_and_matrix(&y_row_vp, basis_obj_row_vp,
                                           inverse_basis_mp));
        }

        /* Step 2 */

        ent_col = NOT_SET;
        max = 0.0;
        case_one = case_two = FALSE;
        ent_x = NOT_SET;      /* Keep error checkers happy. */
        ent_ub = 0.0;   /* Keep error checkers happy. */
        ent_lb = 0.0;   /* Keep error checkers happy. */

        for (i=1; i <= n; i++)
        {
            ERE(get_matrix_col(&a_col_vp, non_basis_mp, i-1));
            ERE(get_dot_product(y_row_vp, a_col_vp, &y_a_prod));
            temp = (non_basis_obj_row_vp->elements)[i-1] - y_a_prod;

            non_basis_x = non_basis_solution_col_vp->elements[ i-1 ];
            ub = non_basis_ub_row_vp->elements[i-1];
            lb = non_basis_lb_row_vp->elements[i-1];


            if ((IS_POSITIVE_DBL(temp)) &&
                (IS_LESSER_DBL(non_basis_x, ub)))
            {
                if (temp > max)
                {
                    max = temp;
                    ent_col = i;
                    ent_ub = ub;
                    ent_lb = lb;
                    ent_x = non_basis_x;

                    case_one = TRUE;
                    case_two = FALSE;
                }
            }
            else if (    (IS_NEGATIVE_DBL(temp))
                      && (IS_GREATER_DBL(non_basis_x, lb))
                    )
            {
                if (-temp > max)
                {
                    max = -temp;
                    ent_col = i;
                    ent_ub = ub;
                    ent_lb = lb;
                    ent_x = non_basis_x;

                    case_one = FALSE;
                    case_two = TRUE;
                }
            }
        }

         if (ent_col == NOT_SET)
         {
            if (report > 0)
            {
                term_blank_out_line();
                 pso("%d iterations used.\n", num_iterations);
                 fflush(stdout);
             }

             return NO_ERROR; /* Solved ! */
         }

         ERE(get_matrix_col(&a_col_vp, non_basis_mp, ent_col-1));

         /* Step 3 */

         ERE(multiply_matrix_and_vector(&d_col_vp,inverse_basis_mp,
                                        a_col_vp));

         /* Step FOUR */

         leave_col = NOT_SET;
         t = 0.0;    /* Keep error checkers happy. */

         if (case_one)
         {
             for (i=0; i<m; i++)
             {
                 if (IS_POSITIVE_DBL((d_col_vp->elements)[i]))
                 {
                     lb = basis_lb_row_vp->elements[i];

                     if (IS_GREATER_DBL(lb, DBL_MOST_NEGATIVE))
                     {
                         temp = ((solution_col_vp->elements)[i]-lb)/
                                               ((d_col_vp->elements)[i]);

                         if (leave_col == NOT_SET)
                         {
                             t = temp;
                             leave_col = i + 1;
                         }
                         else if (temp < t)
                         {
                             t = temp;
                             leave_col = i + 1;
                         }
                     }
                 }
                 else if (IS_NEGATIVE_DBL((d_col_vp->elements)[i]))
                 {
                     ub = basis_ub_row_vp->elements[i];

                     if (IS_LESSER_DBL(ub, DBL_MOST_POSITIVE))
                     {
                         temp = ((solution_col_vp->elements)[i]-ub)/
                                               ((d_col_vp->elements)[i]);

                         if (leave_col == NOT_SET)
                         {
                             t = temp;
                             leave_col = i + 1;
                         }
                         else if (temp < t)
                         {
                             t = temp;
                             leave_col = i + 1;
                         }
                     }
                 }
             }

             if (leave_col == NOT_SET)
             {
                 if (IS_LESSER_DBL(ent_ub, DBL_MOST_POSITIVE))
                 {
                     leave_col = LP_SKIP_LEAVE_AND_ENTER;
                     t = ent_ub - ent_x;
                 }
             }
             else if (    ( ! IS_LESSER_DBL(ent_ub, DBL_MOST_POSITIVE))
                       && (IS_GREATER_DBL(ent_x + t, ent_ub ))
                     )
             {
                 leave_col = LP_SKIP_LEAVE_AND_ENTER;
                 t = ent_ub - ent_x;
             }
         }
         else if (case_two)
         {
             for (i=0; i<m; i++)
             {
                 if (IS_POSITIVE_DBL((d_col_vp->elements)[i]))
                 {
                     ub = basis_ub_row_vp->elements[i];

                     if (IS_LESSER_DBL(ub, DBL_MOST_POSITIVE))
                     {
                         temp = (ub - (solution_col_vp->elements)[i])/
                                            ((d_col_vp->elements)[i]);

                         if (leave_col == NOT_SET)
                         {
                             t = temp;
                             leave_col = i + 1;
                         }
                         else if (temp < t)
                         {
                             t = temp;
                             leave_col = i + 1;
                         }
                     }
                 }
                 else if (IS_NEGATIVE_DBL((d_col_vp->elements)[i]))
                 {
                     lb = basis_lb_row_vp->elements[i];

                     if (IS_GREATER_DBL(lb, DBL_MOST_NEGATIVE))
                     {
                         temp = (lb - (solution_col_vp->elements)[i])/
                                            ((d_col_vp->elements)[i]);

                         if (leave_col == NOT_SET)
                         {
                             t = temp;
                             leave_col = i + 1;
                         }
                         else if (temp < t)
                         {
                             t = temp;
                             leave_col = i + 1;
                         }
                     }
                 }
             }

             if (leave_col == NOT_SET)
             {
                 if (IS_GREATER_DBL(ent_lb, DBL_MOST_NEGATIVE))
                 {
                     leave_col = LP_SKIP_LEAVE_AND_ENTER;
                     t = ent_x - ent_lb;
                 }
             }

             if (    (IS_GREATER_DBL(ent_lb, DBL_MOST_NEGATIVE))
                  && (IS_LESSER_DBL(ent_x - t, ent_lb ))
                )
             {
                 leave_col = LP_SKIP_LEAVE_AND_ENTER;
                 t = ent_x - ent_lb;
             }
         }
         else
         {
             SET_CANT_HAPPEN_BUG();
             return ERROR;
         }

         if (leave_col == NOT_SET)
         {
             return LP_PROBLEM_IS_UNBOUNDED;
         }

         /* Step FIVE */

        if (leave_col != LP_SKIP_LEAVE_AND_ENTER)
        {
            ERE(simplex_update_inverse(m, inverse_basis_mp, leave_col,
                                       d_col_vp, temp_d_col_vp));
         }

        ERE(ow_multiply_vector_by_scalar(d_col_vp, t));

        ERE(copy_vector(&temp_solution_col_vp, solution_col_vp));

        if (case_one)
        {
            ERE(subtract_vectors(&solution_col_vp, temp_solution_col_vp,
                                 d_col_vp));
            (non_basis_solution_col_vp->elements)[ent_col-1] += t;
        }
        else if (case_two)
        {
            ERE(add_vectors(&solution_col_vp,temp_solution_col_vp,
                            d_col_vp));
            (non_basis_solution_col_vp->elements)[ent_col-1] -= t;
        }
        else
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }

        if (leave_col != LP_SKIP_LEAVE_AND_ENTER)
        {
            temp = (solution_col_vp->elements)[leave_col-1];
            (solution_col_vp->elements)[leave_col-1] =
                  (non_basis_solution_col_vp->elements)[ent_col-1];
            (non_basis_solution_col_vp->elements)[ent_col-1] = temp;

            ERE(get_matrix_col(&leave_col_vp,basis_mp,leave_col-1));
            ERE(put_matrix_col(non_basis_mp,leave_col_vp,
                             ent_col-1));
            ERE(put_matrix_col(basis_mp, a_col_vp, leave_col-1));

            temp = basis_obj_row_vp->elements[leave_col-1];
            basis_obj_row_vp->elements[leave_col-1] =
                           non_basis_obj_row_vp->elements[ent_col-1];
            non_basis_obj_row_vp->elements[ent_col-1] = temp;

            temp = basis_lb_row_vp->elements[leave_col-1];
            basis_lb_row_vp->elements[leave_col-1] =
                   non_basis_lb_row_vp->elements[ent_col-1];
            non_basis_lb_row_vp->elements[ent_col-1] = temp;

            temp = basis_ub_row_vp->elements[leave_col-1];
            basis_ub_row_vp->elements[leave_col-1] =
                   non_basis_ub_row_vp->elements[ent_col-1];
            non_basis_ub_row_vp->elements[ent_col-1] = temp;

            index_temp = basis_var_index_vp->elements [leave_col-1];
            basis_var_index_vp->elements[leave_col-1] =
                      non_basis_var_index_vp->elements [ent_col-1];
            non_basis_var_index_vp->elements [ent_col-1] = index_temp;
        }

        num_iterations++;

        if (report > 0)
        {
            if ((num_iterations % report == 0) && (kjb_isatty(fileno(stdout))))
            {
                pso("\rIteration %d", num_iterations);
                fflush(stdout);
            }
        }
    }

    term_blank_out_line();
    fflush(stdout);

    return LP_MAX_NUM_ITERATIONS_EXCEEDED;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */



#ifdef __cplusplus
}
#endif

