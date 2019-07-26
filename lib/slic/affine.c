/* $Id: affine.c 15688 2013-10-14 08:46:32Z predoehl $
 */
#include "slic/affine.h"

#define COLLINEAR(a, b, c) \
(fabs(((b)[1]-(a)[1])*((c)[0]-(a)[0])-((b)[0]-(a)[0])*((c)[1]-(a)[1])) <= 0.00001)

/* Compute the affine transformation between two data sets x and y, which
   follow,
   y1=a11 * x1 + a12 * x2 + a13 * x3 + c1
   y2=a21 * x2 + a22 * x2 + a23 * x3 + c2
   y3=a31 * x2 + a32 * x2 + a33 * x3 + c3
   where (y1, y2, y3) and (x1, x2, x3) are a correspondent pair from x and y, respectively.

   The return matrix is a 3x4 (3D) or 2x3 (2D) matrix arranged as follows,
   | a11 a12 a13 c1 |
   | a21 a22 a23 c2 |
   | a31 a32 a33 c3 |

   Note if x and y are colinear, an error will be thrown out.
*/
int fit_affine
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

    if(dim != 2 && dim != 3)
    {
        add_error("The dimension can only be 2 or 3.\n");
        return ERROR;
    }

    if((dim == 2 && num < 3) || (dim == 3 && num < 4))
    {
        add_error("Number of points required by affine transformation is at least %d.\n", 
            dim+1);
        return ERROR;
    } 

    
    ERE(get_target_matrix(&A_mp, dim * num, dim * (dim + 1)));
    ERE(get_target_vector(&B_vp, dim * num));
  
    for(i=0 ;i<num; i++)
    {
        if(dim == 2)
        {
            A_mp->elements[2*i][0] = x_mp->elements[i][0];
            A_mp->elements[2*i][1] = x_mp->elements[i][1];
            A_mp->elements[2*i][2] = 0;
            A_mp->elements[2*i][3] = 0;
            A_mp->elements[2*i][4] = 1;
            A_mp->elements[2*i][5] = 0;
            B_vp->elements[2*i] = y_mp->elements[i][0];

        A_mp->elements[2*i+1][0] = 0;
            A_mp->elements[2*i+1][1] = 0;
            A_mp->elements[2*i+1][2] = x_mp->elements[i][0];
        A_mp->elements[2*i+1][3] = x_mp->elements[i][1];
        A_mp->elements[2*i+1][4] = 0;
        A_mp->elements[2*i+1][5] = 1;
            B_vp->elements[2*i+1] = y_mp->elements[i][1];
        }
        else
        {
            A_mp->elements[3*i][0] = x_mp->elements[i][0];
            A_mp->elements[3*i][1] = x_mp->elements[i][1];
            A_mp->elements[3*i][2] = x_mp->elements[i][2];
            A_mp->elements[3*i][3] = 0;
            A_mp->elements[3*i][4] = 0;
            A_mp->elements[3*i][5] = 0;
            A_mp->elements[3*i][6] = 0;
            A_mp->elements[3*i][7] = 0;
            A_mp->elements[3*i][8] = 0;
            A_mp->elements[3*i][9] = 1;
            A_mp->elements[3*i][10] = 0;
            A_mp->elements[3*i][11] = 0;
            B_vp->elements[3*i] = y_mp->elements[i][0];
            
            A_mp->elements[3*i+1][0] = 0;
            A_mp->elements[3*i+1][1] = 0;
            A_mp->elements[3*i+1][2] = 0;
            A_mp->elements[3*i+1][3] = x_mp->elements[i][0];
            A_mp->elements[3*i+1][4] = x_mp->elements[i][1];
            A_mp->elements[3*i+1][5] = x_mp->elements[i][2];
            A_mp->elements[3*i+1][6] = 0;
            A_mp->elements[3*i+1][7] = 0;
            A_mp->elements[3*i+1][8] = 0;
            A_mp->elements[3*i+1][9] = 0;
            A_mp->elements[3*i+1][10] = 1;
            A_mp->elements[3*i+1][11] = 0;
            B_vp->elements[3*i+1] = y_mp->elements[i][1];
            
            A_mp->elements[3*i+2][0] = 0;
            A_mp->elements[3*i+2][1] = 0;
            A_mp->elements[3*i+2][2] = 0;
            A_mp->elements[3*i+2][3] = 0;
            A_mp->elements[3*i+2][4] = 0;
            A_mp->elements[3*i+2][5] = 0;
            A_mp->elements[3*i+2][6] = x_mp->elements[i][0];
            A_mp->elements[3*i+2][7] = x_mp->elements[i][1];
            A_mp->elements[3*i+2][8] = x_mp->elements[i][2];
            A_mp->elements[3*i+2][9] = 0;
            A_mp->elements[3*i+2][10] = 0; 
            A_mp->elements[3*i+2][11] = 1;
            B_vp->elements[3*i+2] = y_mp->elements[i][2];
        }
    }

    /*write_matrix(A_mp, 0);*/
    result = least_squares_solve(&result_vp, A_mp, B_vp);

    if(result != ERROR)
    {
        ERE(get_target_matrix(&a_mp, dim, dim+1));
        if(dim == 2)
        {
            a_mp->elements[0][0] = result_vp->elements[0];
            a_mp->elements[0][1] = result_vp->elements[1];
            a_mp->elements[0][2] = result_vp->elements[4];
            a_mp->elements[1][0] = result_vp->elements[2];
            a_mp->elements[1][1] = result_vp->elements[3];
            a_mp->elements[1][2] = result_vp->elements[5];
        }
        else
        {
            a_mp->elements[0][0] = result_vp->elements[0];
            a_mp->elements[0][1] = result_vp->elements[1];
            a_mp->elements[0][2] = result_vp->elements[2];
            a_mp->elements[0][3] = result_vp->elements[9];
            a_mp->elements[1][0] = result_vp->elements[3];
            a_mp->elements[1][1] = result_vp->elements[4];
            a_mp->elements[1][2] = result_vp->elements[5];
            a_mp->elements[1][3] = result_vp->elements[10];
            a_mp->elements[2][0] = result_vp->elements[6];
            a_mp->elements[2][1] = result_vp->elements[7];
            a_mp->elements[2][2] = result_vp->elements[8];
            a_mp->elements[2][3] = result_vp->elements[11];
        }
 
        if(a_mpp != NULL)
        {
            ERE(copy_matrix(a_mpp, a_mp));
        }
 
        if(fit_err_ptr != NULL)
        {
            ERE(get_affine_fitting_error(a_mp, x_mp, y_mp, fit_err_ptr));
        }
    }

    free_matrix(a_mp);
    free_matrix(A_mp);
    free_vector(B_vp);
    free_vector(result_vp);

    return result;
}

/* Compute the affine transformation between two data sets x and y, which
   follow,
   y1=a11 * x1 + a12 * x2 + a13 * x3 + ... + a1n * xn + c1
   y2=a21 * x2 + a22 * x2 + a23 * x3 + ... + a2n * xn + c2
   y3=a31 * x2 + a32 * x2 + a33 * x3 + ... + a3n * xn + c3
   ...
   yn=an1 * x2 + an2 * x2 + an3 * x3 + ... + ann * xn + cn

   The return matrix is a 3x4 (3D) or 2x3 (2D) matrix arranged as follows,
   | a11 a12 a13 ... a1n c1 |
   | a21 a22 a23 ... a2n c2 |
   | a31 a32 a33 ... a3n c3 |
   | ...                    |
   | an1 an2 an3 ... ann cn |

   Note if x and y are colinear, an error will be thrown out.
*/
int fit_n_dimension_affine
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
    int i, j, k;
    int result = NO_ERROR;

    ASSERT(x_mp->num_cols == y_mp->num_cols && 
           x_mp->num_rows == y_mp->num_rows);  


    if((dim < 1) || (num <= dim))
    {
        add_error("Number of points required by affine transformation is at least %d.\n", 
            dim+1);
        return ERROR;
    } 

    
    ERE(get_target_matrix(&A_mp, dim * num, dim * (dim + 1)));
    ERE(get_target_vector(&B_vp, dim * num));
  
    for(i=0 ;i<num; i++)
    {
      for (j = 0; j < dim; j++)
      {
    for (k = 0; k < dim; k++)
    {
      A_mp->elements[dim*i + j][dim*j + k] = x_mp->elements[i][j];
    }
    A_mp->elements[dim*i + j][dim*dim + j] = 1;
    B_vp->elements[dim*i + j] = y_mp->elements[i][j];
      }
    }

    result = least_squares_solve(&result_vp, A_mp, B_vp);

    if(result != ERROR)
    {
        ERE(get_target_matrix(&a_mp, dim, dim+1));
    for (i = 0; i < dim; i++)
    {
      for (j = 0; j < dim; j++)
      {
        a_mp->elements[i][j] = result_vp->elements[i*dim + j];
      }
      a_mp->elements[i][dim] = result_vp->elements[dim*dim + i];
    }
 
        if(a_mpp != NULL)
        {
            ERE(copy_matrix(a_mpp, a_mp));
        }
 
        if(fit_err_ptr != NULL)
        {
            ERE(get_affine_fitting_error(a_mp, x_mp, y_mp, fit_err_ptr));
        }
    }

    free_matrix(a_mp);
    free_matrix(A_mp);
    free_vector(B_vp);
    free_vector(result_vp);

    return result;
}

/* inverse affine */
int affine_inverse
(
    const Matrix *a_mp,
    const Matrix *mp,
    Matrix       **res_mpp
)
{
    int i, j;
    int dim = a_mp->num_rows;
    Matrix *tmp_a_mp = NULL;
    Matrix *inv_mp = NULL;
    Matrix *res_mp = NULL;
    int num = mp->num_rows;
    double d0, d1;

    if(res_mpp == NULL) return NO_ERROR;
    ERE(get_target_matrix(res_mpp, num, dim));
    res_mp = *res_mpp;

    ERE(get_target_matrix(&tmp_a_mp, dim, dim));
    for(i=0; i<dim; i++)
    {
        for(j=0; j<dim; j++)
        {
            tmp_a_mp->elements[i][j] = a_mp->elements[i][j];
        }
    }

    
    ERE(get_matrix_inverse(&inv_mp, tmp_a_mp));
   /* write_matrix(a_mp,0);
    write_matrix(inv_mp, 0);*/

    for(i=0; i<num; i++)
    {
        d0 = mp->elements[i][0] - a_mp->elements[0][2];
        d1 = mp->elements[i][1] - a_mp->elements[1][2];
        res_mp->elements[i][0] = inv_mp->elements[0][0] * d0 + inv_mp->elements[0][1] * d1;
        res_mp->elements[i][1] = inv_mp->elements[1][0] * d0 + inv_mp->elements[1][1] * d1;
    } 


   /* write_matrix(mp, 0);
    pso("\n");
    write_matrix(*res_mpp, 0);
    pso("\n");*/

    free_matrix(tmp_a_mp);
    free_matrix(inv_mp);

    return NO_ERROR;
}

int affine_transform_single_point
(
    const Matrix *a_mp,
    const Vector *x_vp,
    Vector       **y_vpp
)
{
    int dim = a_mp->num_rows;

    if(y_vpp == NULL) return NO_ERROR;

    ERE(get_target_vector(y_vpp, dim));
    
    if(dim == 2)
    {
        (*y_vpp)->elements[0] = a_mp->elements[0][0] * x_vp->elements[0] + 
            a_mp->elements[0][1] * x_vp->elements[1] + a_mp->elements[0][2];
        (*y_vpp)->elements[1] = a_mp->elements[1][0] * x_vp->elements[0] + 
            a_mp->elements[1][1] * x_vp->elements[1] + a_mp->elements[1][2];
    }
    else
    {
        (*y_vpp)->elements[0] = a_mp->elements[0][0] * x_vp->elements[0] + 
                                a_mp->elements[0][1] * x_vp->elements[1] +
                                a_mp->elements[0][2] * x_vp->elements[2] + 
                                a_mp->elements[0][3];
        (*y_vpp)->elements[1] = a_mp->elements[1][0] * x_vp->elements[0] + 
                                a_mp->elements[1][1] * x_vp->elements[1] +
                                a_mp->elements[1][2] * x_vp->elements[2] + 
                                a_mp->elements[1][3];
        (*y_vpp)->elements[2] = a_mp->elements[2][0] * x_vp->elements[0] + 
                                a_mp->elements[2][1] * x_vp->elements[1] +
                                a_mp->elements[2][2] * x_vp->elements[2] + 
                                a_mp->elements[2][3];
    }

    return NO_ERROR;
}

int affine_transform
(
    const Matrix *a_mp,
    const Matrix *x_mp,
    Matrix       **y_mpp
)
{
    int dim = a_mp->num_rows;
    int num = x_mp->num_rows;
    int i;

    if(y_mpp == NULL) return NO_ERROR;

    ERE(get_target_matrix(y_mpp, num, dim));
 
    for(i=0; i<num; i++)
    {   
        if(dim == 2)
        {
            (*y_mpp)->elements[i][0] = a_mp->elements[0][0] * x_mp->elements[i][0] + 
                a_mp->elements[0][1] * x_mp->elements[i][1] + a_mp->elements[0][2];
            (*y_mpp)->elements[i][1] = a_mp->elements[1][0] * x_mp->elements[i][0] + 
                a_mp->elements[1][1] * x_mp->elements[i][1] + a_mp->elements[1][2];
        }
        else
        {
            (*y_mpp)->elements[i][0] = a_mp->elements[0][0] * x_mp->elements[i][0] + 
                                a_mp->elements[0][1] * x_mp->elements[i][1] +
                                a_mp->elements[0][2] * x_mp->elements[i][2] + 
                                a_mp->elements[0][3];
            (*y_mpp)->elements[i][1] = a_mp->elements[1][0] * x_mp->elements[i][0] + 
                                a_mp->elements[1][1] * x_mp->elements[i][1] +
                                a_mp->elements[1][2] * x_mp->elements[i][2] + 
                                a_mp->elements[1][3];
            (*y_mpp)->elements[i][2] = a_mp->elements[2][0] * x_mp->elements[i][0] + 
                                a_mp->elements[2][1] * x_mp->elements[i][1] +
                                a_mp->elements[2][2] * x_mp->elements[i][2] + 
                                a_mp->elements[2][3];
        }
    }

    return NO_ERROR;
}
        
int get_affine_distance
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *a_mp, 
    Vector       **dist_vpp
)
{
    int i;
    int j;
    int dim = a_mp->num_rows;
    int num = x_mp->num_rows;
    Matrix *res_mp = NULL;
    Vector *dist_vp = NULL;
    double error;
    double tmp;
  
    if(dist_vpp == NULL) return NO_ERROR;

    ERE(get_target_vector(dist_vpp, num));
    dist_vp = *dist_vpp;

    ERE(affine_transform(a_mp, x_mp, &res_mp));
    
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

int is_affine_degenerate
(
    const Matrix *x_mp
)
{
    double a[2];
    double b[2];
    double c[2];

    a[0] = x_mp->elements[0][0];
    a[1] = x_mp->elements[0][1];
    b[0] = x_mp->elements[1][0];
    b[1] = x_mp->elements[1][1];
    c[0] = x_mp->elements[2][0];
    c[1] = x_mp->elements[2][1];
 
    return COLLINEAR(a, b, c);
}

int get_affine_fitting_error
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *a_mp,
    double *fit_err_ptr
)
{
    Vector *dist_vp = NULL;

    if(fit_err_ptr == NULL) return NO_ERROR;

    ERE(get_affine_distance(x_mp, y_mp, a_mp, &dist_vp));
    *fit_err_ptr = sum_vector_elements(dist_vp);

    free_vector(dist_vp);

    return NO_ERROR;
}

/* 
 * Cholesky decomposition of a symmetric and positive-definite matrix 
 * There is another version under lib, but was added after SLIC by Ernesto. Hence,
 * this method was renamed to cholesky_decomposition_SLIC 
 */
int cholesky_decomposition_SLIC
(
    const Matrix *mp,
    Matrix       **res_mpp
)
{
    Matrix *res_mp = NULL;
    Vector *diag_vp = NULL;
    int n = mp->num_rows;
    int i, j, k;
    double sum;
    int result = NO_ERROR;
    
    ASSERT(mp->num_rows == mp->num_cols);

    if(res_mpp == NULL) return NO_ERROR;

    ERE(copy_matrix(&res_mp, mp));
    ERE(get_target_vector(&diag_vp, n));

    for(i=0; i<n; i++)
    {
        for(j=i; j<n; j++)
        {
            sum = res_mp->elements[i][j];
            for(k=i-1; k>=0; k--)
            {
                sum -= res_mp->elements[i][k] * res_mp->elements[j][k];
            }
            if(i == j)
            {
                if(sum <= 0)
                {
                    pso("The input is not positive definite and decomposition failed!\n");
                    add_error("The input is not positive definite!\n");
                    result = ERROR;
                    break;
                }
                diag_vp->elements[i] = sqrt(sum); 
            }
            else
            {
               res_mp->elements[j][i] = sum / diag_vp->elements[i];
            }
        }
        if(result == ERROR) break;
    }   

    if(result != ERROR)
    {
        for(i=0; i<n; i++)
        {
            for(j=i; j<n; j++)
            {
                if(j == i)
                {
                    res_mp->elements[i][j] = diag_vp->elements[i];
                }
                else
                {
                    res_mp->elements[i][j] = res_mp->elements[j][i];
                    res_mp->elements[j][i] = 0.0;
                }
            }
        }
        ERE(copy_matrix(res_mpp, res_mp));
    }

    free_matrix(res_mp);
    free_vector(diag_vp);
    
    return result;
}

/* decompose the affine matrix into 3 components, rotation,
   scaling and shearing.
   A = R*S*Z
   S is diagonal and Z is upper triangular with ones along the
   diagonal.  */
int affine_decomposition
(
    const Matrix *a_mp,
    Matrix       **R_mpp,
    Matrix       **S_mpp,
    Matrix       **Z_mpp
)
{
    int i;
    Matrix *transpose_mp = NULL;
    Matrix *target_mp = NULL;
    Matrix *c_mp = NULL;
    Matrix *d_mp = NULL;
    Matrix *inv_mp = NULL;
    int num = a_mp->num_rows;
    int result;

    transpose_mp = create_matrix_transpose(a_mp);
    if(!transpose_mp) return ERROR;

    result = multiply_matrices(&target_mp, transpose_mp, a_mp);
    if(result != ERROR)
    {
        result = cholesky_decomposition_SLIC(target_mp, &c_mp);
    }

    if(result != ERROR)
    {
        ERE(get_initialized_matrix(&d_mp, num, num, 0.0));
        for(i=0; i<num; i++)
        {
            d_mp->elements[i][i] = c_mp->elements[i][i];
        }

        if(S_mpp != NULL)
        {
            ERE(copy_matrix(S_mpp, d_mp));
        }

        if(Z_mpp != NULL)
        {
            /* inverse it */
            for(i=0; i<num; i++)
            {
                d_mp->elements[i][i] = 1.0 / d_mp->elements[i][i];
            }
            ERE(multiply_matrices(Z_mpp, d_mp, c_mp));
        }

        if(R_mpp != NULL)
        {
            result = get_matrix_inverse(&inv_mp, c_mp);
            if(result != ERROR)
            {
                ERE(multiply_matrices(R_mpp, a_mp, inv_mp));
            }
        }
    }

    free_matrix(target_mp);
    free_matrix(transpose_mp);
    free_matrix(c_mp);
    free_matrix(d_mp);
    free_matrix(inv_mp);

    return result;
}

int constrained_affine_1
(
    const Matrix *a_mp
)
{
     double sx, sy;
     double r;

     sx = a_mp->elements[0][0];
     sy = a_mp->elements[1][1];
     
     if(sx < 0 || sy < 0 ) return 0;
     if(sx < 0.125 || sx > 8.0) return 0;
     if(sy < 0.125 || sy > 8.0) return 0;
     
     r = sx / sy;
     if(r < 0.6667 || r > 1.50) return 0;

     return 1;
}

/*
 *   get_color_constancy_matrix
 *
 *  Computes the matrix that transforms
 *  
 *
 *
 */
int get_color_constancy_matrix
(
    const KJB_image *src_img,
    const KJB_image *target_img,
    const Int_matrix *mask_imp,
    Matrix **color_constancy_mpp
)
{
  Matrix *src_mp = NULL;
  Matrix *target_mp = NULL;
  Matrix *a_mp = NULL;
  int num_rows = src_img->num_rows;
  int num_cols = src_img->num_cols;
  int num = num_rows * num_cols;
  int i, j;
  int count;

  
  ERE(get_target_matrix(&src_mp, num, 3));
  ERE(get_target_matrix(&target_mp, num, 3));
  count = 0;

  for(i=0; i<num_rows; i++)
  {
    for(j=0; j<num_cols; j++)
    {
      if(mask_imp != NULL && mask_imp->elements[i][j])
      {
    src_mp->elements[count][0] = (src_img->pixels[i][j]).r;
    src_mp->elements[count][1] = (src_img->pixels[i][j]).g;
    src_mp->elements[count][2] = (src_img->pixels[i][j]).b;
    target_mp->elements[count][0] = (target_img->pixels[i][j]).r;
    target_mp->elements[count][1] = (target_img->pixels[i][j]).g;
    target_mp->elements[count][2] = (target_img->pixels[i][j]).b;
      }
      else
      {
    src_mp->elements[count][0] = 0;
    src_mp->elements[count][1] = 0;
    src_mp->elements[count][2] = 0;
    target_mp->elements[count][0] = 0;
    target_mp->elements[count][1] = 0;
    target_mp->elements[count][2] = 0;
      }
      count++;
    }
  }

  ERE(fit_affine(src_mp, target_mp, &a_mp, NULL));
    
  if (*color_constancy_mpp != NULL)
  {
    free_matrix(*color_constancy_mpp);
  }

  *color_constancy_mpp = a_mp;

  free_matrix(src_mp);
  free_matrix(target_mp);

  return NO_ERROR;
}

/* 
 *    apply_color_constancy
 *
 *  Given the source image and the color constancy matrixk
 *  create the appropriate resulting image.
 *
 *
 */
int apply_color_constancy
(
    const KJB_image *src_img,
    const Int_matrix *mask_imp,
    const Matrix *color_constancy_mp,
    KJB_image       **res_img
)
{
  Matrix *src_mp = NULL;
  int num_rows = src_img->num_rows;
  int num_cols = src_img->num_cols;
  int num = num_rows * num_cols;
  Matrix *res_mp = NULL;
  int row;
  int col;
  int i, j, count;
  KJB_image *img = NULL;
  

  ERE(get_target_matrix(&src_mp, num, 3));

  count = 0;
  for(i=0; i<num_rows; i++)
  {
    for(j=0; j<num_cols; j++)
    {
      if(mask_imp != NULL && mask_imp->elements[i][j])
      {
    src_mp->elements[count][0] = (src_img->pixels[i][j]).r;
    src_mp->elements[count][1] = (src_img->pixels[i][j]).g;
    src_mp->elements[count][2] = (src_img->pixels[i][j]).b;
      }
      else
      {
    src_mp->elements[count][0] = 0;
    src_mp->elements[count][1] = 0;
    src_mp->elements[count][2] = 0;
      }
      count++;
    }
  }


  ERE(affine_transform(color_constancy_mp, src_mp, &res_mp));
  ERE(get_target_image(res_img, num_rows, num_cols));
  img = *res_img;



  for(i=0; i<num; i++)
  {
    row = i / num_cols;
    col = i % num_cols;
    if(res_mp->elements[i][0] < 0) res_mp->elements[i][0] = 0;
    if(res_mp->elements[i][0] > 255) res_mp->elements[i][0] = 255;
    if(res_mp->elements[i][1] < 0) res_mp->elements[i][1] = 0;
    if(res_mp->elements[i][1] > 255) res_mp->elements[i][1] = 255;
    if(res_mp->elements[i][2] < 0) res_mp->elements[i][2] = 0;
    if(res_mp->elements[i][2] > 255) res_mp->elements[i][2] = 255;
            
    (img->pixels[row][col]).r = res_mp->elements[i][0];
    (img->pixels[row][col]).g = res_mp->elements[i][1];
    (img->pixels[row][col]).b = res_mp->elements[i][2];
  }

  
  free_matrix(src_mp);
  free_matrix(res_mp);

  return NO_ERROR;
}

/*     do_color_constancy
 *  
 *  Can be implemented with 
 *  get_color_constancy_matrix and apply_color_constancy
 *  but would be inefficient (basically converting src_img and target_img to 
 *  src_mp and target_mp twice).
 *
*/
int do_color_constancy
(
    const KJB_image *src_img,
    const KJB_image *target_img,
    const Int_matrix *mask_imp,
    KJB_image       **res_img
)
{
    Matrix *src_mp = NULL;
    Matrix *target_mp = NULL;
    Matrix *res_mp = NULL;
    Matrix *a_mp = NULL;
    int num_rows = src_img->num_rows;
    int num_cols = src_img->num_cols;
    int num = num_rows * num_cols;
    int i, j;
    int count;
    KJB_image *img = NULL;
    int row;
    int col;
    int result = NO_ERROR;

    ERE(get_target_matrix(&src_mp, num, 3));
    ERE(get_target_matrix(&target_mp, num, 3));
    count = 0;
    for(i=0; i<num_rows; i++)
    {
        for(j=0; j<num_cols; j++)
        {
            if(mask_imp != NULL && mask_imp->elements[i][j])
            {
                src_mp->elements[count][0] = (src_img->pixels[i][j]).r;
                src_mp->elements[count][1] = (src_img->pixels[i][j]).g;
                src_mp->elements[count][2] = (src_img->pixels[i][j]).b;
                target_mp->elements[count][0] = (target_img->pixels[i][j]).r;
                target_mp->elements[count][1] = (target_img->pixels[i][j]).g;
                target_mp->elements[count][2] = (target_img->pixels[i][j]).b;
            }
            else
            {
                src_mp->elements[count][0] = 0;
                src_mp->elements[count][1] = 0;
                src_mp->elements[count][2] = 0;
                target_mp->elements[count][0] = 0;
                target_mp->elements[count][1] = 0;
                target_mp->elements[count][2] = 0;
            }
            count++;
        }
    }

    result = fit_affine(src_mp, target_mp, &a_mp, NULL);
    if(result != ERROR)
    {
        ERE(affine_transform(a_mp, src_mp, &res_mp));
    }
    
    if(result != ERROR)
    {
        ERE(get_target_image(res_img, num_rows, num_cols));
        img = *res_img;

        for(i=0; i<num; i++)
        {
            row = i / num_cols;
            col = i % num_cols;
            if(res_mp->elements[i][0] < 0) res_mp->elements[i][0] = 0;
            if(res_mp->elements[i][0] > 255) res_mp->elements[i][0] = 255;
            if(res_mp->elements[i][1] < 0) res_mp->elements[i][1] = 0;
            if(res_mp->elements[i][1] > 255) res_mp->elements[i][1] = 255;
            if(res_mp->elements[i][2] < 0) res_mp->elements[i][2] = 0;
            if(res_mp->elements[i][2] > 255) res_mp->elements[i][2] = 255;
            
            (img->pixels[row][col]).r = res_mp->elements[i][0];
            (img->pixels[row][col]).g = res_mp->elements[i][1];
            (img->pixels[row][col]).b = res_mp->elements[i][2];
        }
    }

    free_matrix(src_mp);
    free_matrix(target_mp);
    free_matrix(res_mp);
    free_matrix(a_mp);

    return result;
}

int do_color_constancy_with_position
(
    const KJB_image *src_img,
    const KJB_image *target_img,
    const Int_matrix *mask_imp,
    KJB_image       **res_img
)
{
    Matrix *src_mp = NULL;
    Matrix *target_mp = NULL;
    Matrix *res_mp = NULL;
    Matrix *a_mp = NULL;
    int num_rows = src_img->num_rows;
    int num_cols = src_img->num_cols;
    int num = num_rows * num_cols;
    int i, j;
    int count;
    KJB_image *img = NULL;
    int row;
    int col;
    int result = NO_ERROR;

    ERE(get_target_matrix(&src_mp, num, 5));
    ERE(get_target_matrix(&target_mp, num, 5));
    count = 0;
    for(i=0; i<num_rows; i++)
    {
        for(j=0; j<num_cols; j++)
        {
            if(mask_imp != NULL && mask_imp->elements[i][j])
            {
                src_mp->elements[count][0] = (src_img->pixels[i][j]).r;
                src_mp->elements[count][1] = (src_img->pixels[i][j]).g;
                src_mp->elements[count][2] = (src_img->pixels[i][j]).b;
                src_mp->elements[count][3] = j;
                src_mp->elements[count][4] = i;
                target_mp->elements[count][0] = (target_img->pixels[i][j]).r;
                target_mp->elements[count][1] = (target_img->pixels[i][j]).g;
                target_mp->elements[count][2] = (target_img->pixels[i][j]).b;
                target_mp->elements[count][3] = j;
                target_mp->elements[count][4] = i;
            }
            else
            {
                src_mp->elements[count][0] = 0;
                src_mp->elements[count][1] = 0;
                src_mp->elements[count][2] = 0;
                src_mp->elements[count][3] = 0;
                src_mp->elements[count][4] = 0;
                target_mp->elements[count][0] = 0;
                target_mp->elements[count][1] = 0;
                target_mp->elements[count][2] = 0;
                target_mp->elements[count][3] = 0;
                target_mp->elements[count][4] = 0;
            }
            count++;
        }
    }

    result = fit_n_dimension_affine(src_mp, target_mp, &a_mp, NULL);
    if(result != ERROR)
    {
        ERE(affine_transform(a_mp, src_mp, &res_mp));
    }
    
    if(result != ERROR)
    {
        ERE(get_target_image(res_img, num_rows, num_cols));
        img = *res_img;

        for(i=0; i<num; i++)
        {
            row = i / num_cols;
            col = i % num_cols;
            if(res_mp->elements[i][0] < 0)   res_mp->elements[i][0] = 0;
            if(res_mp->elements[i][0] > 255) res_mp->elements[i][0] = 255;
            if(res_mp->elements[i][1] < 0)   res_mp->elements[i][1] = 0;
            if(res_mp->elements[i][1] > 255) res_mp->elements[i][1] = 255;
            if(res_mp->elements[i][2] < 0)   res_mp->elements[i][2] = 0;
            if(res_mp->elements[i][2] > 255) res_mp->elements[i][2] = 255;
            
            (img->pixels[row][col]).r = res_mp->elements[i][0];
            (img->pixels[row][col]).g = res_mp->elements[i][1];
            (img->pixels[row][col]).b = res_mp->elements[i][2];
        }
    }

    free_matrix(src_mp);
    free_matrix(target_mp);
    free_matrix(res_mp);
    free_matrix(a_mp);

    return result;
}


int constraint_affine_2
(
    const Matrix *a_mp
)
{
    int i, j;
    int result;
    Matrix *mp = NULL;
    int num_rows = a_mp->num_rows;
    int num_cols = 2;
    double xs, ys;
    double r;
    double angle;
    Matrix *R_mp = NULL;
    Matrix *S_mp = NULL;

    ERE(get_target_matrix(&mp, num_rows, num_cols));
    for(i=0; i<num_rows; i++)
    {
        for(j=0; j<num_cols; j++)
        {
            mp->elements[i][j] = a_mp->elements[i][j];
        }
    }

    result = affine_decomposition(mp, &R_mp, &S_mp, NULL);

    if(result != ERROR)
    {
        result = 1;

        xs = S_mp->elements[0][0];
        ys = S_mp->elements[1][1];
        r = xs / ys;
        if(xs > 8 ||
           xs < 0.125 ||
           ys >  8 ||
           ys < 0.125 )
        {
            result = 0;
        }
        else if( r > 2 || r < 0.5)
        {
            result = 0;
        }

        if(result)
        {
            angle = acos(R_mp->elements[0][0]);
            if(fabs(angle) > 25 * M_PI / 180) result = 0; 
        }
    }

    free_matrix(mp);
    free_matrix(R_mp);
    free_matrix(S_mp);
    
    return result;
}


int main1(int argc, char *argv[])
{
    Matrix *A_mp = NULL;
    Matrix *R_mp = NULL;
    Matrix *S_mp = NULL;
    Matrix *Z_mp = NULL;
    KJB_image *img1 = NULL;
    KJB_image *img2 = NULL;
    KJB_image *res_img = NULL;
    double d;
    Matrix_vector *model1_mvp = NULL;
    Matrix_vector *model2_mvp = NULL;
    int id;
    char fname[200];

    sprintf(fname, "/net/v06/space/quanfu/slides/randyi_test/homography/str-0.9/fit-3.0/match_model/ctorandyi_%s_match_model_mvp", argv[1]);
    pso("%s\n", fname);
    read_matrix_vector(&model1_mvp, fname);
    sprintf(fname, "/net/v06/space/quanfu/slides/randyi_test/affine/str-0.9/fit-3.0/match_model/ctorandyi_%s_match_model_mvp", argv[1]);
    pso("%s\n", fname);
    read_matrix_vector(&model2_mvp, fname);
    id = atoi(argv[2]) - 1;
    d = (model1_mvp->elements[id])->elements[2][2];
    ow_multiply_matrix_by_scalar(model1_mvp->elements[id], 1.0/d);
    write_matrix(model1_mvp->elements[id], 0);
    pso("\n");
    write_matrix(model2_mvp->elements[id], 0);
    exit(0);


   /* read_matrix(&A_mp, "A_mp");
    write_matrix(A_mp, 0);
    ERE(affine_decomposition(A_mp, &R_mp, &S_mp, &Z_mp));
 
    pso("\nR\n");
    write_matrix(R_mp, 0);
    pso("\nS\n");
    write_matrix(S_mp, 0);
    pso("\nZ\n");
    write_matrix(Z_mp, 0);*/

    kjb_read_image(&img1, argv[1]);
    
 /*   d = atof(argv[2]);
    ERE(get_initialized_vector(&d_vp, 3, d));
    ERE(ow_add_vector_to_image(img1, d_vp));
    kjb_display_image(img1, NULL);
    getchar();
    exit(0);*/

    kjb_read_image(&img2, argv[2]);
    ERE(do_color_constancy(img1, img2, NULL, &res_img));
 
    kjb_display_image(res_img, NULL);
    getchar();
    kjb_free_image(img1);
    kjb_free_image(img2);
    kjb_free_image(res_img);
    /*read_matrix(&A_mp, "B_mp");
    pso("ddd\n");   
    write_matrix(A_mp, 0);
    pso("ddd\n");   
    ERE(cholesky_decomposition_SLIC(A_mp, &R_mp));
    pso("ddd\n");   
    write_matrix(R_mp, 0);*/

    free_matrix(A_mp);
    free_matrix(R_mp);
    free_matrix(S_mp);
    free_matrix(Z_mp);
    
    return 0;
}
