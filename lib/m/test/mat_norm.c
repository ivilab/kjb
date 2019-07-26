
/* $Id: mat_norm.c 21664 2017-08-05 17:53:40Z kobus $ */


#include "m/m_incl.h"

/*
#define VERBOSE 1
*/


#define NUM_LOOPS         50
#define BASE_NUM_TRIES    50

/* ------------------------------------------------------------------------- */

static Matrix* IH_NormMatrixCols(Matrix* matrix);
static Matrix* IH_NormMatrixRows(Matrix* matrix);

/* ------------------------------------------------------------------------- */

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int num_rows;
    int num_cols; 
    int count;
    Matrix* a_mp = NULL; 
    Matrix* b_mp = NULL; 
    Matrix* c_mp = NULL; 
    Matrix* d_mp = NULL; 
    Matrix* e_mp = NULL; 
    Matrix* f_mp = NULL; 
    Matrix* first_mp = NULL;
    Vector* sum_vp = NULL; 
    double    max_sum;
    int     max_sum_index;
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;


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

    for (count=0; count<num_tries; count++)
    {
        for (num_rows=1; num_rows<NUM_LOOPS; num_rows++)
        {
            for (num_cols=1; num_cols<NUM_LOOPS; num_cols++)
            {
                double  diff[ 4 ];
                int     diff_count = 0;
                int     i; 


#ifdef VERBOSE
                pso("\n-------------------------------------------------\n\n");
                pso("%d %d %d\n\n", count, num_rows, num_cols);
#endif 

                
                EPETE(get_random_matrix(&first_mp, num_rows, num_cols)); 

                EPETE(copy_matrix(&a_mp, first_mp)); 
                EPETE(copy_matrix(&b_mp, first_mp)); 
                EPETE(copy_matrix(&c_mp, first_mp)); 
                EPETE(copy_matrix(&d_mp, first_mp)); 

                NPETE(IH_NormMatrixRows(a_mp)); 
                NPETE(IH_NormMatrixCols(b_mp)); 
                EPETE(ow_normalize_matrix_rows(c_mp));
                EPETE(ow_normalize_matrix_cols(d_mp));
                EPETE(normalize_matrix_rows(&e_mp, first_mp));
                EPETE(normalize_matrix_cols(&f_mp, first_mp));

#ifdef VERBOSE
                db_mat(first_mp);

                db_mat(a_mp);
                db_mat(c_mp);
                db_mat(e_mp);
                db_mat(b_mp);
                db_mat(d_mp);
                db_mat(f_mp);
#endif 

                diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, c_mp) / num_cols; 
                diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, e_mp) / num_cols; 
                diff[ diff_count++ ] = max_abs_matrix_difference(b_mp, d_mp) / num_rows; 
                diff[ diff_count++ ] = max_abs_matrix_difference(b_mp, f_mp) / num_rows; 

                for (i=0; i<diff_count; i++) 
                {
                    if (ABS_OF(diff[ i ]) > 3.0 * DBL_EPSILON)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, diff[ i ]); 
                        p_stderr("Problem with test %d.\n", i);
                        status = EXIT_BUG;
                    }

                }

                EPETE(sum_matrix_cols(&sum_vp, first_mp));
                EPETE(max_sum_index = get_max_matrix_row_sum(first_mp, &max_sum));

                if (sum_vp->elements[ max_sum_index ] < SUB_RELATIVE_DBL(max_sum, 2.0 * num_rows * DBL_EPSILON))
                {
                    p_stderr("Sum at max index is %e but max sum is %e.\n",
                             (double)(sum_vp->elements[ max_sum_index ]), 
                             (double)max_sum); 
                    p_stderr("Problem with max sum test.\n");
                    status = EXIT_BUG; 
                }

                for (i=0; i<num_rows; i++) 
                {
                    if (sum_vp->elements[ i ] > ADD_RELATIVE_DBL(max_sum, 2.0 * num_rows * DBL_EPSILON))
                    {
                        p_stderr("Sum is %e but max sum is %e.\n",
                                 (double)(sum_vp->elements[ i ]), 
                                 (double)max_sum); 
                        p_stderr("Problem with max sum test.\n");
                        status = EXIT_BUG; 
                    }
                }
            }
        }
    }
    
    free_matrix(first_mp); 
    free_matrix(a_mp); 
    free_matrix(b_mp);
    free_matrix(c_mp);
    free_matrix(d_mp);
    free_matrix(e_mp);
    free_matrix(f_mp);
    free_vector(sum_vp); 

    return status; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Matrix *IH_NormMatrixCols(Matrix *matrix)
{
/* Normalizes the columns of the input matrix such that their vector
   magnitudes are all 1.0.  A pointer to the resulting matrix is
   returned on success, NULL otherwise.
*/
   int    row, col;
   double mag;
   
   /* verify input */
   if (matrix == NULL || matrix->num_rows < 0 || matrix ->num_cols < 0) {
      return (matrix);
   }
   for (col = 0; col < matrix->num_cols; col++) {
   
      /* calculate row magnitude */
      for (row = 0, mag = 0.0; row < matrix->num_rows; row++)
         mag += matrix->elements[row][col] * matrix->elements[row][col];
      mag = sqrt(mag);
         
      /* a zero magnitude column cannot be normalized */
      if (mag == 0.0) {
         return ((Matrix *)NULL);
      }                    
      /* scale the elements */
      for (row = 0; row < matrix->num_rows; row++)
         matrix->elements[row][col] /= mag;
   }
   return (matrix);
}     

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Matrix *IH_NormMatrixRows(Matrix *matrix)
{
/* Normalizes the rows of the input matrix such that their vector
   magnitudes are all 1.0.  A pointer to the resulting matrix is
   returned on success, NULL otherwise.
*/
   int    row, col;
   double mag;
   
   /* verify input */
   if (matrix == NULL || matrix->num_rows < 0 || matrix->num_cols < 0) {
      return (matrix);
   }
   for (row = 0; row < matrix->num_rows; row++) {
   
      /* calculate row magnitude */
      for (col = 0, mag = 0.0; col < matrix->num_cols; col++)
         mag += matrix->elements[row][col] * matrix->elements[row][col];
      mag = sqrt(mag);

      /* a zero magnitude row cannot be normalized */
      if (mag == 0.0) {
         return ((Matrix *)NULL);
      }              
      /* scale the elements */
      for (col = 0; col < matrix->num_cols; col++)
         matrix->elements[row][col] /= mag;
   }
   return (matrix);
}     



