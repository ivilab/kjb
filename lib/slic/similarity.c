/* $Id: similarity.c 15688 2013-10-14 08:46:32Z predoehl $
 */
#include "slic/similarity.h"
#include "slic/affine.h"

#define COVECTOR(a, b) \
(fabs((a)[0]*(b)[1] -(a)[1]*(b)[0]) <= 0.00001)

/* Compute the similarity transformation between two data sets x and y, which
   follow,
   y1=a * x1 - b * x2 +  t1
   y2=b * x2 + a * x2 +  t2
   where (y1, y2) and (x1, x2) are a correspondent pair from x and y, respectively.

   The return matrix is a 2x3 (2D) matrix arranged as follows,
   | s*cos(theta) -s*sin(theta) t1 |
   | s*sin(theta)  s*cos(theta) t2 |
   where s=sqrt(a*a + b * b) and theta = arctan(a/b).

   Note if x and y are colinear, an error will be thrown out.
*/
int fit_similarity
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    Matrix       **a_mpp,
    double       *fit_err_ptr
)
{
    Matrix *A_mp = NULL;
    Vector *B_vp = NULL;
    Vector *result_vp = NULL;
    Matrix *a_mp = NULL;
    int num = x_mp->num_rows;
    int dim = x_mp->num_cols;
    int i;
    int result = NO_ERROR;

    ASSERT(x_mp->num_cols == y_mp->num_cols && 
           x_mp->num_rows == y_mp->num_rows);  

    if(dim != 2)
    {
        add_error("The dimension can only be 2.\n");
        return ERROR;
    }

    if(dim == 2 && num < 2)
    {
        add_error("Number of points required by similarity transformation is at least %d.\n", 
            dim);
        return ERROR;
    } 

    
    ERE(get_target_matrix(&A_mp, dim * num, 4));
    ERE(get_target_vector(&B_vp, dim * num));
  
    for(i=0 ;i<num; i++)
    {
        A_mp->elements[2*i][0] = x_mp->elements[i][0];
        A_mp->elements[2*i][1] = -x_mp->elements[i][1];
        A_mp->elements[2*i][2] = 1;
        A_mp->elements[2*i][3] = 0;
        B_vp->elements[2*i] = y_mp->elements[i][0];
        
        A_mp->elements[2*i+1][0] = x_mp->elements[i][1];
        A_mp->elements[2*i+1][1] = x_mp->elements[i][0];
        A_mp->elements[2*i+1][2] = 0;
        A_mp->elements[2*i+1][3] = 1;
        B_vp->elements[2*i+1] = y_mp->elements[i][1];
    }

    /*write_matrix(A_mp, 0);*/
    result = least_squares_solve(&result_vp, A_mp, B_vp);

    if(result != ERROR)
    {
        ERE(get_target_matrix(&a_mp, dim, dim+1));
        a_mp->elements[0][0] = result_vp->elements[0];
        a_mp->elements[0][1] = -result_vp->elements[1];
        a_mp->elements[0][2] = result_vp->elements[2];
        a_mp->elements[1][0] = result_vp->elements[1];
        a_mp->elements[1][1] = result_vp->elements[0];
        a_mp->elements[1][2] = result_vp->elements[3];
        
        if(a_mpp != NULL)
        {
            ERE(copy_matrix(a_mpp, a_mp));
        }
 
        if(fit_err_ptr != NULL)
        {
            ERE(get_similarity_fitting_error(a_mp, x_mp, y_mp, fit_err_ptr));
        }
    }

    free_matrix(a_mp);
    free_matrix(A_mp);
    free_vector(B_vp);
    free_vector(result_vp);

    return result;
}

/* inverse affine */
int similarity_inverse
(
    const Matrix *a_mp,
    const Matrix *mp,
    Matrix       **res_mpp
)
{
    return affine_inverse(a_mp,mp, res_mpp);
}

int similarity_transform_single_point
(
    const Matrix *a_mp,
    const Vector *x_vp,
    Vector       **y_vpp
)
{
    return affine_transform_single_point(a_mp, x_vp, y_vpp);
}

int similarity_transform
(
    const Matrix *a_mp,
    const Matrix *x_mp,
    Matrix       **y_mpp
)
{
    return affine_transform(a_mp, x_mp, y_mpp);
}
        
int get_similarity_distance
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *a_mp, 
    Vector       **dist_vpp
)
{
    return get_affine_distance(x_mp, y_mp, a_mp, dist_vpp);
}

int is_similarity_degenerate
(
    const Matrix *x_mp
)
{
    double a[2];
    double b[2];

    a[0] = x_mp->elements[0][0];
    a[1] = x_mp->elements[0][1];
    b[0] = x_mp->elements[1][0];
    b[1] = x_mp->elements[1][1];
    return COVECTOR(a, b);
}

int get_similarity_fitting_error
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *a_mp,
    double *fit_err_ptr
)
{
    return get_affine_fitting_error(x_mp, y_mp, a_mp, fit_err_ptr);
}

int constrained_similarity
(
    const Matrix *a_mp
)
{
     double a, b;
     double s;
     double theta;

     a = a_mp->elements[0][0];
     b = a_mp->elements[1][0];
     s = sqrt(a*a + b *b);
     
     if(s < 0.125 || s > 8.0) return 0;
     
     if(a == 0) 
     {
         theta = 90;
     }
     else
     {
        theta = atan(b/a);
        theta /= M_PI;
        theta *= 180;
     }

     if(theta > 20 || theta < -20) return 0;

     return 1;
}
