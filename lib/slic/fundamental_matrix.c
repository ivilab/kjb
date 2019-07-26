/* $Id: fundamental_matrix.c 15688 2013-10-14 08:46:32Z predoehl $
 */
#include "slic/fundamental_matrix.h"

static int get_Sampson_distance
(
    const Vector *x_vp,
    const Vector *y_vp,
    const Matrix *F_mp, 
    const Matrix *transpose_F_mp, 
    double       *ptr
);

/* Compute fundamental matrix
   y'*F*x' = 0
*/
int fit_fundamental_matrix
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    Matrix       **F_mpp,
    double       *fit_err_ptr
)
{
    Matrix *norm_x_mp = NULL; 
    Matrix *norm_y_mp = NULL; 
    Matrix *x_t_mp = NULL; 
    Matrix *y_t_mp = NULL; 
    Matrix *A_mp = NULL;
    Matrix *V_mp = NULL;
    Matrix *mp = NULL;
    Matrix *F_mp = NULL;
    Matrix *U_mp = NULL;
    Matrix *D_mp = NULL;
    int n = x_mp->num_rows;
    int i;
    Vector *s_vp = NULL;

    if(F_mpp == NULL) return NO_ERROR;

    if(x_mp->num_cols != 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }
    
    if(x_mp->num_rows != y_mp->num_rows ||
       x_mp->num_cols != y_mp->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(n < 8)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(normalize_2Dpoints(x_mp, &norm_x_mp, &x_t_mp));
    ERE(normalize_2Dpoints(y_mp, &norm_y_mp, &y_t_mp));
    /*write_matrix(norm_x_mp, 0);
    write_matrix(x_t_mp, 0);
    pso("\n");
    write_matrix(norm_y_mp, 0);
    write_matrix(y_t_mp, 0);*/

    ERE(get_initialized_matrix(&A_mp, n, 9, 1.0));   
    for(i=0; i<n; i++)
    {
        A_mp->elements[i][0] = norm_y_mp->elements[i][0]*norm_x_mp->elements[i][0];
        A_mp->elements[i][1] = norm_y_mp->elements[i][0]*norm_x_mp->elements[i][1];
        A_mp->elements[i][2] = norm_y_mp->elements[i][0];
        A_mp->elements[i][3] = norm_y_mp->elements[i][1]*norm_x_mp->elements[i][0];
        A_mp->elements[i][4] = norm_y_mp->elements[i][1]*norm_x_mp->elements[i][1];
        A_mp->elements[i][5] = norm_y_mp->elements[i][1];
        A_mp->elements[i][6] = norm_x_mp->elements[i][0];
        A_mp->elements[i][7] = norm_x_mp->elements[i][1];
    }
    /*pso("A_mp\n");
    write_matrix(A_mp, 0);*/

    /* currently, the third argument cannot be null even it's not needed here.*/
    ERE(do_svd(A_mp, NULL, &s_vp, &V_mp, NULL));

    ERE(get_target_matrix(&F_mp, 3, 3));
    F_mp->elements[0][0] = V_mp->elements[8][0];
    F_mp->elements[0][1] = V_mp->elements[8][1];
    F_mp->elements[0][2] = V_mp->elements[8][2];
    F_mp->elements[1][0] = V_mp->elements[8][3];
    F_mp->elements[1][1] = V_mp->elements[8][4];
    F_mp->elements[1][2] = V_mp->elements[8][5];
    F_mp->elements[2][0] = V_mp->elements[8][6];
    F_mp->elements[2][1] = V_mp->elements[8][7];
    F_mp->elements[2][2] = V_mp->elements[8][8];
    /*pso("F_mp\n");
    write_matrix(F_mp, 0);*/

 /* Enforce constraint that fundamental matrix has rank 2 by performing
     a svd and then reconstructing with the two largest singular values.*/
    ERE(do_svd(F_mp, &U_mp, &s_vp, &V_mp, NULL));

    /* [U,D,V] = svd(F,0);
    F = U*diag([D(1,1) D(2,2) 0])*V' */
    ERE(get_initialized_matrix(&D_mp, 3, 3, 0.0));
    D_mp->elements[0][0] = s_vp->elements[0];
    D_mp->elements[1][1] = s_vp->elements[1];
    D_mp->elements[2][2] = 0.0;
    /*pso("\n");
    write_matrix(D_mp, 0);*/
    
    ERE(multiply_matrices(&mp, U_mp, D_mp));

    /* denormalize */
    ERE(multiply_matrices(&mp, F_mp, x_t_mp));
    ERE(multiply_with_transpose(F_mpp, y_t_mp, mp));
   
    if(fit_err_ptr != NULL)
    {
        ERE(get_fundamental_matrix_fitting_error(x_mp, y_mp, *F_mpp, fit_err_ptr));
    }

    free_matrix(norm_x_mp);
    free_matrix(norm_y_mp);
    free_matrix(x_t_mp);
    free_matrix(y_t_mp);
    free_matrix(A_mp);
    free_matrix(V_mp);
    free_matrix(mp);
    free_matrix(F_mp);
    free_vector(s_vp);
    free_matrix(U_mp);
    free_matrix(D_mp);

    return NO_ERROR;
}

/*Sampson distance*/
static int get_Sampson_distance
(
    const Vector *x_vp,
    const Vector *y_vp,
    const Matrix *F_mp, 
    const Matrix *transpose_F_mp, 
    double       *ptr
)
{
    Vector *xF_vp = NULL;
    Vector *yF_vp = NULL;
    Vector *yFt_vp = NULL;
    int i;
    double a;
    double b;

    ERE(multiply_matrix_and_vector(&xF_vp, F_mp, x_vp));
    ERE(multiply_matrix_and_vector(&yFt_vp, transpose_F_mp, y_vp));
    
    a = 0;
    for(i=0; i<3; i++)
    {
        a += (xF_vp->elements[i] * y_vp->elements[i]);
    }

    a =a * a;
    b = xF_vp->elements[0] * xF_vp->elements[0];
    b += (xF_vp->elements[1] * xF_vp->elements[1]);
    b += (yFt_vp->elements[0] * yFt_vp->elements[0]);
    b += (yFt_vp->elements[1] * yFt_vp->elements[1]);
    if(b == 0) b = 0.00000001;

    if(ptr != NULL) *ptr = a / b;
    free_vector(xF_vp);
    free_vector(yFt_vp);

    return NO_ERROR;
}

/*Sampson distance*/
int get_fundamental_matrix_distance
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *F_mp, 
    Vector       **dist_vpp
)
{
    int i;
    int j;
    int num = x_mp->num_rows;
    Vector *dist_vp = NULL;
    Vector *x_vp = NULL;
    Vector *y_vp = NULL;
    Matrix *transpose_F_mp = NULL;

    if(dist_vpp == NULL) return NO_ERROR;

    ERE(get_matrix_transpose(&transpose_F_mp, F_mp));
    ERE(get_target_vector(dist_vpp, num));
    dist_vp = *dist_vpp;

    ERE(get_target_vector(&x_vp, 3));
    ERE(get_target_vector(&y_vp, 3));
    for(i=0; i<num; i++)
    {
        x_vp->elements[0] = x_mp->elements[i][0];
        x_vp->elements[1] = x_mp->elements[i][1];
        x_vp->elements[2] = 1.0;
        y_vp->elements[0] = y_mp->elements[i][0];
        y_vp->elements[1] = y_mp->elements[i][1];
        y_vp->elements[2] = 1.0;
        ERE(get_Sampson_distance(x_vp, y_vp, F_mp, transpose_F_mp, &(dist_vp->elements[i])));
    }

    free_vector(x_vp);
    free_vector(y_vp);
    free_matrix(transpose_F_mp);
    return NO_ERROR;
}


int get_fundamental_matrix_fitting_error
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *F_mp,
    double *fit_err_ptr
)
{
    Vector *dist_vp = NULL;
    int result;

    if(fit_err_ptr == NULL) return NO_ERROR;

    ERE(get_fundamental_matrix_distance(x_mp, y_mp, F_mp, &dist_vp));
    *fit_err_ptr = sum_vector_elements(dist_vp);

    write_row_vector(dist_vp,0);
    free_vector(dist_vp);

    return NO_ERROR;
}

int is_fundamental_matrix_degenerate
(
    const Matrix *x_mp
)
{
    return 0;
}

int test_fundamental_matrix()
{
    Matrix *x_mp = NULL;
    Matrix *y_mp = NULL;
    Matrix *F_mp = NULL;
    double err;

    read_matrix(&x_mp, "match_x_mp");
    read_matrix(&y_mp, "match_y_mp");
    ERE(fit_fundamental_matrix(x_mp, y_mp, &F_mp, &err));
    write_matrix(F_mp,0);
    pso("error: %f\n", err);
    return 0;
}
