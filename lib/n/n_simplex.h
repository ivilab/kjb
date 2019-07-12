
/* $Id: n_simplex.h 4727 2009-11-16 20:53:54Z kobus $ */

/*
   Copyright (c) 1994-2008 by Kobus Barnard (author).

   Personal and educational use of this code is granted, provided
   that this header is kept intact, and that the authorship is not
   misrepresented. Commercial use is not permitted.
*/

#ifndef N_SIMPEX_INCLUDED
#define N_SIMPEX_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#ifdef DOUBLE_PRECISION
#    define LP_FEASIBILITY_CHECK_EPSILON      1.0e-6
#    define LP_AUX_FEASIBILITY_CHECK_EPSILON  1.0e-6
#    define LP_FEASIBILITY_EPSILON            1.0e-12
#    define LP_ENTER_EPSILON                  1.0e-10
#    define LP_LEAVE_EPSILON                  1.0e-10
#    define LP_AUX_ENTER_EPSILON              1.0e-15
#    define LP_AUX_USE_X_ZERO_EPSILON         1.0e-10
#else
#    define LP_FEASIBILITY_CHECK_EPSILON      1.0e-3
#    define LP_AUX_FEASIBILITY_CHECK_EPSILON  1.0e-3
#    define LP_FEASIBILITY_EPSILON            1.0e-8
#    define LP_ENTER_EPSILON                  1.0e-5
#    define LP_LEAVE_EPSILON                  1.0e-5
#    define LP_AUX_ENTER_EPSILON              1.0e-10
#    define LP_AUX_USE_X_ZERO_EPSILON         1.0e-6
#endif

#define LP_PROBLEM_IS_INFEASIBLE           (ERROR - 1)
#define LP_PROBLEM_IS_UNBOUNDED            (ERROR - 2)
#define LP_MAX_NUM_ITERATIONS_EXCEEDED     (ERROR - 3)
#define LP_PROBLEM_IS_OVERCONSTRAINED      (ERROR - 4)
#define LP_SKIP_LEAVE_AND_ENTER            (NOT_SET - 1)


int do_simplex
(
    Matrix* constraint_mp,
    Vector* constraint_col_vp,
    Vector* obj_row_vp,
    int     max_num_iterations,
    int     report,
    Vector* result_vp
);

int do_reg_simplex
(
    Matrix*     basis_mp,
    Matrix*     inverse_basis_mp,
    Matrix*     non_basis_mp,
    Vector*     solution_col_vp,
    Vector*     basis_obj_row_vp,
    Vector*     non_basis_obj_row_vp,
    Int_vector* basis_var_index_vp,
    Int_vector* non_basis_var_index_vp,
    int         max_num_iterations,
    int         report,
    Vector*     result_vp
);

int do_aux_simplex
(
    Matrix*     basis_mp,
    Matrix*     inverse_basis_mp,
    Matrix*     non_basis_mp,
    Vector*     solution_col_vp,
    Vector*     basis_obj_row_vp,
    Vector*     non_basis_obj_row_vp,
    Int_vector* basis_var_index_vp,
    Int_vector* non_basis_var_index_vp,
    int         leave_col,
    int         max_num_iterations,
    int         report,
    Vector*     result_vp
);

int simplex_update_inverse
(
    int     m,
    Matrix* inverse_mp,
    int     new_col,
    Vector* d_col_vp,
    Vector* temp_d_col_vp
);

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
);

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
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

