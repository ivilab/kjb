/* $Id: homography.c 15688 2013-10-14 08:46:32Z predoehl $
 */
#include "slic/homography.h"
#include "slic/affine.h"

#define COLLINEAR(a, b, c) \
(fabs(((b)[1]-(a)[1])*((c)[0]-(a)[0])-((b)[0]-(a)[0])*((c)[1]-(a)[1])) <= 0.00001)

/* Compute 2D homography
   y=H*x
*/
int fit_homography
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    Matrix       **H_mpp,
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
    Matrix *inv_mp = NULL;
    Matrix *H_mp = NULL;
    int n;
    int i;
    double a1, a2, a3;
    Vector *s_vp = NULL;

    if(H_mpp == NULL) return NO_ERROR;

    if (x_mp == NULL || y_mp == NULL)
    {
        add_error("ERROR (%s +%d): At least one input parameter is NULL.", __FILE__, __LINE__);
        return ERROR;
    }

    if(x_mp->num_cols != 2)
    {
        add_error("ERROR (%s +%d): Input matrix has %d instead of 2 columns.", __FILE__, __LINE__, x_mp->num_cols);
        /*SET_ARGUMENT_BUG();*/
        return ERROR;
    }
    
    if(x_mp->num_rows != y_mp->num_rows ||
       x_mp->num_cols != y_mp->num_cols)
    {
        add_error("ERROR (%s +%d): Input matrix dimensions don't match (columns %d != %d or rows %d != %d).", 
                  __FILE__, __LINE__, x_mp->num_cols, y_mp->num_cols, x_mp->num_rows, y_mp->num_rows);
        /*SET_ARGUMENT_BUG();*/
        return ERROR;
    }

    n = x_mp->num_rows;
    if(n < 4)
    {
        add_error("ERROR (%s +%d): n (%d) should not be < 4.", __FILE__, __LINE__, n);
        /*SET_ARGUMENT_BUG();*/
        return ERROR;
    }

    ERE(normalize_2Dpoints(x_mp, &norm_x_mp, &x_t_mp));
    ERE(normalize_2Dpoints(y_mp, &norm_y_mp, &y_t_mp));
   /* write_matrix(norm_x_mp, 0);
    write_matrix(x_t_mp, 0);
    pso("\n");
    write_matrix(norm_y_mp, 0);
    write_matrix(y_t_mp, 0);*/

    ERE(get_initialized_matrix(&A_mp, 3 * n, 9, 0.0));   
    for(i=0; i<n; i++)
    {
        a1 = norm_y_mp->elements[i][0];
        a2 = norm_y_mp->elements[i][1];
        a3 = norm_y_mp->elements[i][2];
        A_mp->elements[3*i][3] = -a3 * norm_x_mp->elements[i][0];
        A_mp->elements[3*i][4] = -a3 * norm_x_mp->elements[i][1];
        A_mp->elements[3*i][5] = -a3 * norm_x_mp->elements[i][2];
        A_mp->elements[3*i][6] =  a2 * norm_x_mp->elements[i][0];
        A_mp->elements[3*i][7] =  a2 * norm_x_mp->elements[i][1];
        A_mp->elements[3*i][8] =  a2 * norm_x_mp->elements[i][2];
        
        A_mp->elements[3*i+1][0] = a3 * norm_x_mp->elements[i][0];
        A_mp->elements[3*i+1][1] = a3 * norm_x_mp->elements[i][1];
        A_mp->elements[3*i+1][2] = a3 * norm_x_mp->elements[i][2];
        A_mp->elements[3*i+1][6] = -a1 * norm_x_mp->elements[i][0];
        A_mp->elements[3*i+1][7] = -a1 * norm_x_mp->elements[i][1];
        A_mp->elements[3*i+1][8] = -a1 * norm_x_mp->elements[i][2];
        
        A_mp->elements[3*i+2][0] = -a2 * norm_x_mp->elements[i][0];
        A_mp->elements[3*i+2][1] = -a2 * norm_x_mp->elements[i][1];
        A_mp->elements[3*i+2][2] = -a2 * norm_x_mp->elements[i][2];
        A_mp->elements[3*i+2][3] =  a1 * norm_x_mp->elements[i][0];
        A_mp->elements[3*i+2][4] =  a1 * norm_x_mp->elements[i][1];
        A_mp->elements[3*i+2][5] =  a1 * norm_x_mp->elements[i][2];
    }
    /*pso("A_mp\n");
    write_matrix(A_mp, 0);*/

    /* currently, the third argument cannot be null even it's not needed here.*/
    ERE(do_svd(A_mp, NULL, &s_vp, &V_mp, NULL));

    ERE(get_target_matrix(&H_mp, 3, 3));
    H_mp->elements[0][0] = V_mp->elements[8][0];
    H_mp->elements[0][1] = V_mp->elements[8][1];
    H_mp->elements[0][2] = V_mp->elements[8][2];
    H_mp->elements[1][0] = V_mp->elements[8][3];
    H_mp->elements[1][1] = V_mp->elements[8][4];
    H_mp->elements[1][2] = V_mp->elements[8][5];
    H_mp->elements[2][0] = V_mp->elements[8][6];
    H_mp->elements[2][1] = V_mp->elements[8][7];
    H_mp->elements[2][2] = V_mp->elements[8][8];
   /* pso("H_mp\n");
    write_matrix(H_mp, 0);*/
   
    /* denormalize */
    ERE(get_matrix_inverse(&inv_mp, y_t_mp));
    ERE(multiply_matrices(&mp, inv_mp, H_mp));
    ERE(multiply_matrices(H_mpp, mp, x_t_mp));
   
    if(fit_err_ptr != NULL)
    {
        ERE(get_homography_fitting_error(x_mp, y_mp, *H_mpp, fit_err_ptr));
    }

    free_matrix(norm_x_mp);
    free_matrix(norm_y_mp);
    free_matrix(x_t_mp);
    free_matrix(y_t_mp);
    free_matrix(A_mp);
    free_matrix(V_mp);
    free_matrix(mp);
    free_matrix(inv_mp);
    free_matrix(H_mp);
    free_vector(s_vp);

    return NO_ERROR;
}

int normalize_2Dpoints
(
    const Matrix *pts_mp,
    Matrix       **new_pts_mpp,
    Matrix       **t_mpp
)
{
    Matrix *ext_pts_mp = NULL;
    Vector *mean_vp = NULL;
    Matrix *t_mp = NULL;
    int num;
    int i;
    double d, mdist;
    double scale;

    if (new_pts_mpp == NULL || t_mpp == NULL)
    {
        return NO_ERROR;
    }

    if (pts_mp == NULL)
    {
        add_error("ERROR (%s +%d): Input parameter is NULL.", __FILE__, __LINE__);
        return ERROR;
    }

    if(pts_mp->num_cols != 2)
    {
        add_error("ERROR (%s +%d): Points matrix should have 2 columns (not %d).", __FILE__, __LINE__, pts_mp->num_cols);
        /*SET_ARGUMENT_BUG();*/
        return ERROR;
    }
    num = pts_mp->num_rows;

    ERE(get_initialized_matrix(&ext_pts_mp, num, 3, 1.0));
    ERE(ow_copy_matrix_block(ext_pts_mp, 0, 0, pts_mp, 0, 0, num, 2));

    ERE(average_matrix_rows(&mean_vp, ext_pts_mp));
    ERE(ow_subtract_row_vector_from_matrix(ext_pts_mp, mean_vp));
    mdist = 0;
    for(i=0; i<num; i++)
    {
        d = ext_pts_mp->elements[i][0] * ext_pts_mp->elements[i][0];
        d += ext_pts_mp->elements[i][1] * ext_pts_mp->elements[i][1];
        mdist += sqrt(d);
    }
    mdist /= num;

    ASSERT(mdist > 0);

    scale = sqrt(2.0)/mdist;
    ERE(get_initialized_matrix(&t_mp, 3, 3, 0.0));
    t_mp->elements[0][0] = scale;
    t_mp->elements[0][2] = -scale * mean_vp->elements[0];
    t_mp->elements[1][1] = scale;
    t_mp->elements[1][2] = -scale * mean_vp->elements[1];
    t_mp->elements[2][2] = 1.0;

    ERE(ow_multiply_vector_by_scalar(mean_vp, -1.0));
    ERE(ow_subtract_row_vector_from_matrix(ext_pts_mp, mean_vp)); /*make it be the original matrix */
    if(new_pts_mpp != NULL) ERE(multiply_by_transpose(new_pts_mpp, ext_pts_mp, t_mp));
    if(t_mpp != NULL) ERE(copy_matrix(t_mpp, t_mp));

    free_matrix(ext_pts_mp);
    free_matrix(t_mp);
    free_vector(mean_vp);
    
    return NO_ERROR;
}

/* Transform a set points with homography matrix */
int homography_transform
(
    const Matrix *p_mp,
    const Matrix *x_mp,
    Matrix       **y_mpp
)
{
    int n;
    int dim;
    Vector *vp = NULL;
    Matrix *y_mp = NULL;
    int result = NO_ERROR;
    int i;

    if (p_mp == NULL || x_mp == NULL)
    {
       add_error("ERROR (%s +%d): At least one input parameter is NULL.", __FILE__, __LINE__);
       return ERROR;
    }
    n = x_mp->num_rows;
    dim = x_mp->num_cols;

    if(y_mpp == NULL) return NO_ERROR;

    if(p_mp->num_rows != 3 || p_mp->num_cols != 3)
    {
        add_error("ERROR (%s +%d): Rows and columns of a homography matrix should be = 3.", __FILE__, __LINE__);
        add_error("ERROR (%s +%d): However, here # rows = %d, # columns = %d.", __FILE__, __LINE__, p_mp->num_rows, p_mp->num_cols);
        /*SET_ARGUMENT_BUG();*/
        return ERROR;
    }

    ERE(get_target_vector(&vp, 3));
    ERE(get_target_matrix(y_mpp, n, dim));
    y_mp = *y_mpp;

    for(i=0; i<n; i++)
    {
        if(dim == 2)
        {
            vp->elements[0] = p_mp->elements[0][0] * x_mp->elements[i][0] +
                              p_mp->elements[0][1] * x_mp->elements[i][1] +
                              p_mp->elements[0][2] * 1.0;
            vp->elements[1] = p_mp->elements[1][0] * x_mp->elements[i][0] +
                              p_mp->elements[1][1] * x_mp->elements[i][1] +
                              p_mp->elements[1][2] * 1.0;
            vp->elements[2] = p_mp->elements[2][0] * x_mp->elements[i][0] +
                              p_mp->elements[2][1] * x_mp->elements[i][1] +
                              p_mp->elements[2][2] * 1.0;
        }
        else
        {
            vp->elements[0] = p_mp->elements[0][0] * x_mp->elements[i][0] +
                              p_mp->elements[0][1] * x_mp->elements[i][1] +
                              p_mp->elements[0][2] * x_mp->elements[i][2];
            vp->elements[1] = p_mp->elements[1][0] * x_mp->elements[i][0] +
                              p_mp->elements[1][1] * x_mp->elements[i][1] +
                              p_mp->elements[1][2] * x_mp->elements[i][2];
            vp->elements[2] = p_mp->elements[2][0] * x_mp->elements[i][0] +
                              p_mp->elements[2][1] * x_mp->elements[i][1] +
                              p_mp->elements[2][2] * x_mp->elements[i][2];
        }

        if(vp->elements[2] == 0) 
        {
            result = ERROR;
            break;
        }
        
        y_mp->elements[i][0] = vp->elements[0] / vp->elements[2];
        y_mp->elements[i][1] = vp->elements[1] / vp->elements[2];
        if(dim == 3)
        {
            y_mp->elements[i][1] = 1.0;
        }
    }

    if(result == ERROR)
    {
        free_matrix(*y_mpp);
    }

    free_vector(vp);

    return result;
}

/* inverse homography */
int homography_inverse
(
    const Matrix *H_mp,
    const Matrix *mp,
    Matrix       **res_mpp
)
{
    Matrix *inv_mp = NULL;

    ERE(get_matrix_inverse(&inv_mp, H_mp));
    ERE(homography_transform(inv_mp, mp, res_mpp));
    free_matrix(inv_mp);

    return NO_ERROR;
}

/* compute 
   (x-Hx')^2 + (x'-inv(H)*x)^2
*/
int get_dual_homography_distance
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *H_mp, 
    Vector       **dist_vpp
)
{
    int i;
    int j;
    int dim;
    int num;
    Matrix *inv_H_mp = NULL;
    Matrix *res1_mp = NULL;
    Matrix *res2_mp = NULL;
    Vector *dist_vp = NULL;
    double error;
    double tmp;
  
    if (x_mp == NULL || y_mp == NULL || H_mp == NULL)
    {
        add_error("ERROR (%s +%d): At least one input parameter is NULL.", __FILE__, __LINE__);
        return ERROR;
    }

    if(dist_vpp == NULL) return NO_ERROR;

    dim = x_mp->num_cols;
    num = x_mp->num_rows;

    ERE(get_target_vector(dist_vpp, num));
    dist_vp = *dist_vpp;

    ERE(get_matrix_inverse(&inv_H_mp, H_mp));

    ERE(homography_transform(H_mp, x_mp, &res1_mp));
    ERE(homography_transform(inv_H_mp, y_mp, &res2_mp));

    for(i=0; i<num; i++)
    {
        error = 0;
        for(j=0; j<dim; j++)
        {
            tmp = res1_mp->elements[i][j] - y_mp->elements[i][j];
            error += (tmp * tmp);
            tmp = res2_mp->elements[i][j] - x_mp->elements[i][j];
            error += (tmp * tmp);
        }
        dist_vp->elements[i] = sqrt(error/2.0);
    }

    free_matrix(res1_mp);
    free_matrix(res2_mp);
    free_matrix(inv_H_mp);

    return NO_ERROR;
}

int get_homography_distance
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *H_mp, 
    Vector       **dist_vpp
)
{
    int i;
    int j;
    int dim;
    int num;
    Matrix *res_mp = NULL;
    Vector *dist_vp = NULL;
    double error;
    double tmp;
  
    if (x_mp == NULL || y_mp == NULL || H_mp == NULL)
    {
        add_error("ERROR (%s +%d): At least one input parameter is NULL.", __FILE__, __LINE__);
        return ERROR;
    }

    if(dist_vpp == NULL) return NO_ERROR;

    dim = x_mp->num_cols;
    num = x_mp->num_rows;
    ERE(get_target_vector(dist_vpp, num));
    dist_vp = *dist_vpp;

    ERE(homography_transform(H_mp, x_mp, &res_mp));
    
    for(i=0; i<num; i++)
    {
        error = 0;
        for(j=0; j<dim; j++)
        {
            tmp = res_mp->elements[i][j] - y_mp->elements[i][j];
            error += (tmp * tmp);
        }
        dist_vp->elements[i] = sqrt(error);
    }

    free_matrix(res_mp);

    return NO_ERROR;
}

int get_homography_fitting_error
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *H_mp,
    double *fit_err_ptr
)
{
    Vector *dist_vp = NULL;

    if(fit_err_ptr == NULL) return NO_ERROR;
  
    if (x_mp == NULL || y_mp == NULL || H_mp == NULL)
    {
        add_error("ERROR (%s +%d): At least one input parameter is NULL.", __FILE__, __LINE__);
        return ERROR;
    }

    ERE(get_homography_distance(x_mp, y_mp, H_mp, &dist_vp));
    /*ERE(get_dual_homography_distance(x_mp, y_mp, H_mp, &dist_vp));*/
    *fit_err_ptr = sum_vector_elements(dist_vp);

    free_vector(dist_vp);

    return NO_ERROR;
}

int is_homography_degenerate
(
    const Matrix *x_mp
)
{
    int i;
    double a[2];
    double b[2];
    double c[2];
  
    if (x_mp == NULL)
    {
        add_error("ERROR (%s +%d): Input matrix is NULL.", __FILE__, __LINE__);
        return ERROR;
    }

    for(i=0; i<4; i++)
    {
        a[0] = x_mp->elements[i][0];
        a[1] = x_mp->elements[i][1];
        b[0] = x_mp->elements[(i+1)%4][0];
        b[1] = x_mp->elements[(i+1)%4][1];
        c[0] = x_mp->elements[(i+2)%4][0];
        c[1] = x_mp->elements[(i+2)%4][1];
       if(COLLINEAR(a, b, c)) return 1;
    }
 
    return 0;
}

/* Assume that the transformation can be approximated by an affine
 * transformation and the aspect ratio is confined by that of the camera.
 Also, the image is not be scaled too much */
int constrained_homography_1
(
    const Matrix *H_mp
)
{
     double sx, sy;
     double d1, d2;
     double r;
     
    if (H_mp == NULL)
    {
        add_error("ERROR (%s +%d): Input matrix is NULL.", __FILE__, __LINE__);
        return ERROR;
    }

     sx = H_mp->elements[0][0] / H_mp->elements[2][2];
     sy = H_mp->elements[1][1] / H_mp->elements[2][2];
     
     if(sx < 0 || sy < 0 ) return 0;
     if(sx < 0.125 || sx > 8.0) return 0;
     if(sy < 0.125 || sy > 8.0) return 0;
     
     r = sx / sy;
     if(r < 0.6667 || r > 1.50) return 0;

     d1 = H_mp->elements[2][0] / H_mp->elements[2][2];
     if(fabs(d1) > 0.1) return 0;
     d2 = H_mp->elements[2][1] / H_mp->elements[2][2];
     if(fabs(d2) > 0.1) return 0;

     return 1;
}

int constrained_homography_2
(
    const Matrix *H_mp
)
{
     double sx, sy;
     double d1, d2;
     double r;
     double angle;
     Matrix *a_mp = NULL;
     Matrix *R_mp = NULL;
     Matrix *S_mp = NULL;
     Matrix *U_mp = NULL;
     int res = 1;
    
    if (H_mp == NULL)
    {
        add_error("ERROR (%s +%d): Input matrix is NULL.", __FILE__, __LINE__);
        return ERROR;
    }

    if(fabs(H_mp->elements[2][2]) < 0.0001) res = 0;

    if(res)
    {
        d1 = H_mp->elements[2][0] / H_mp->elements[2][2];
        if(fabs(d1) > 0.1) res = 0;
    }

    if(res)
    {
        d2 = H_mp->elements[2][1] / H_mp->elements[2][2];
        if(fabs(d2) > 0.1) res = 0;
    }
    
    /* compute the rotation */
    if(res)
    {
        ERE(get_target_matrix(&a_mp, 2, 2));
        a_mp->elements[0][0] = H_mp->elements[0][0] / H_mp->elements[2][2];
        a_mp->elements[0][1] = H_mp->elements[0][1] / H_mp->elements[2][2];
        a_mp->elements[1][0] = H_mp->elements[1][0] / H_mp->elements[2][2];
        a_mp->elements[1][1] = H_mp->elements[1][1] / H_mp->elements[2][2];

        if(affine_decomposition(a_mp, &R_mp, &S_mp, &U_mp) == ERROR)
        {
            add_error("ERROR (%s +%d): Unable to perform an affine decomposition.", __FILE__, __LINE__);
            free_matrix(a_mp);
            return ERROR;
        }

        sx = S_mp->elements[0][0];
        sy = S_mp->elements[1][1];
    
    /*scaling*/
        if(sx < 0 || sy < 0 ) res = 0;
        if(sx < 0.125 || sx > 8.0) res = 0;
        if(sy < 0.125 || sy > 8.0) res = 0;
        r = sx / sy;
        if(r < 0.6667 || r > 1.50) res = 0;

   /* write_matrix(S_mp, 0);
    pso("\n");
    write_matrix(R_mp, 0);
    pso("\n");
    write_matrix(U_mp, 0);*/

    /*rotation */
        angle = acos(R_mp->elements[0][0]);
        if(fabs(angle) > 20 * M_PI / 180) res = 0; 

    /*shearing*/
        if(fabs(U_mp->elements[0][1]) > 0.2) res = 0;
    }

    free_matrix(a_mp);
    free_matrix(R_mp);
    free_matrix(S_mp);
    free_matrix(U_mp);

    return res;
}

int get_unity_homography
(
    Matrix **h_mpp
)
{
    ERE(get_initialized_matrix(h_mpp, 3, 3, 0.0));
    (*h_mpp)->elements[0][0] = 1.0;
    (*h_mpp)->elements[1][1] = 1.0;
    (*h_mpp)->elements[2][2] = 1.0;

    return NO_ERROR;
}

int normalize_homography
(
    Matrix *h_mp
)
{
    if(h_mp == NULL) return NO_ERROR;

    if(h_mp->num_rows != 3 && h_mp->num_cols != 3)
    {
        add_error("ERROR (%s +%d): Rows and columns of a homography matrix should be = 3.", __FILE__, __LINE__);
        add_error("ERROR (%s +%d): However, here # rows = %d, # columns = %d.", __FILE__, __LINE__, h_mp->num_rows, h_mp->num_cols);
        /*SET_ARGUMENT_BUG();*/
        return ERROR;
    }

    h_mp->elements[0][0] /= h_mp->elements[2][2];    
    h_mp->elements[0][1] /= h_mp->elements[2][2];    
    h_mp->elements[0][2] /= h_mp->elements[2][2];    
    h_mp->elements[1][0] /= h_mp->elements[2][2];    
    h_mp->elements[1][1] /= h_mp->elements[2][2];    
    h_mp->elements[1][2] /= h_mp->elements[2][2];    
    h_mp->elements[2][0] /= h_mp->elements[2][2];    
    h_mp->elements[2][1] /= h_mp->elements[2][2];    
    h_mp->elements[2][2] = 1.0;

    return NO_ERROR;
}

/* verify whether or not a computed homography fits the input data. 
   It returns the point with the maximum error*/
int verify_homography
(
    const Matrix *H_mp,
    const Matrix *x_mp,
    const Matrix *y_mp,
    double       *err_ptr
)
{
    Vector *dist_vp = NULL;
    int id;

    ERE(get_homography_distance(x_mp, y_mp,H_mp, &dist_vp));

    if(err_ptr)
    {
        id = get_max_vector_element(dist_vp, err_ptr);
    }

    free_vector(dist_vp);

    return id;
}

int verify_homography_set
(
    const Matrix_vector *H_mvp,
    const Matrix_vector *x_mvp,
    const Matrix_vector *y_mvp,
    Vector              **err_vpp
)
{
    int i;
    int num = H_mvp->length;
    Vector *err_vp = NULL;

    if (err_vpp == NULL)
    {
        return NO_ERROR;
    }

    if (x_mvp == NULL || y_mvp == NULL || H_mvp == NULL)
    {
        add_error("ERROR (%s +%d): At least one input parameter is NULL.", __FILE__, __LINE__);
        return ERROR;
    }
    num = H_mvp->length;

    ERE(get_initialized_vector(&err_vp, num, -99.0));

    for(i=0; i<num; i++)
    {
        if(H_mvp->elements[i] != NULL)
        {
            ERE(verify_homography(H_mvp->elements[i],
                                  x_mvp->elements[i],
                                  y_mvp->elements[i],
                                  &(err_vp->elements[i])));
        }
    }

    if(err_vpp != NULL) ERE(copy_vector(err_vpp, err_vp));

    free_vector(err_vp);

    return NO_ERROR;
}
