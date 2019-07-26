
/* $Id: nr_svd.c 4725 2009-11-16 19:50:08Z kobus $ */


#include "nr/nr_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "nr/nr_svd.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int do_numerical_recipes_svd_guts
(
    Matrix*  A,
    Vector** d_vpp,
    Matrix** v_mpp
);

/* -------------------------------------------------------------------------- */


/*
// Note: This routine computes the "reduced" SVD.
*/
int do_numerical_recipes_svd
(
    const Matrix* a_mp,
    Matrix**      u_mpp,
    Vector**      d_vpp,
    Matrix**      v_trans_mpp
)
{
    Matrix*         v_mp               = NULL;
    Matrix*         A_mp               = NULL;
    Vector*         d_vp               = NULL;
    int             num_rows;
    int             num_cols;
    int             result = NO_ERROR;
    Indexed_vector* indexed_vector_ptr = NULL;
    int             max_rank;
    int             max_dim;
    int             i;
    int             j;
    int             index;


    num_rows = a_mp->num_rows;
    num_cols = a_mp->num_cols;
    max_rank = MIN_OF(num_rows, num_cols);
    max_dim  = MAX_OF(num_rows, num_cols);

    if ((result != ERROR) && (u_mpp != NULL) )
    {
       result = get_zero_matrix(u_mpp, num_rows, max_rank);
    }

    if ((result != ERROR) && (v_trans_mpp != NULL))
    {
        result = get_zero_matrix(v_trans_mpp, num_cols, num_cols);
    }

    if ((result != ERROR) && (d_vpp != NULL))
    {
        result = get_zero_vector(d_vpp, max_rank);
    }

    if (result != ERROR)
    {
        result = get_zero_matrix(&A_mp, max_dim, num_cols);
    }

    if (result != ERROR)
    {
        A_mp->num_rows = num_rows;
        result = copy_matrix(&A_mp, a_mp);
    }

    if (result != ERROR)
    {
        A_mp->num_rows = max_dim;
        result = do_numerical_recipes_svd_guts(A_mp, &d_vp, &v_mp);
    }

    if (result != ERROR)
    {
        indexed_vector_ptr = vp_create_indexed_vector(d_vp);

        if (indexed_vector_ptr == NULL)
        {
            result = ERROR;
        }
        else
        {
            result = descend_sort_indexed_vector(indexed_vector_ptr);
        }
    }

    if (result != ERROR)
    {
        for (j=0; j<num_cols; j++)
        {
            index = indexed_vector_ptr->elements[ j ].index;

            if (j < max_rank)
            {
                if (d_vpp != NULL)
                {
                    (*d_vpp)->elements[ j ] = d_vp->elements[ index ];
                }

                if (u_mpp != NULL)
                {
                    for (i=0; i<num_rows; i++)
                    {
                        (*u_mpp)->elements[ i ][ j ] =
                                                   A_mp->elements[ i ][ index ];
                    }
                }
            }

            if (v_trans_mpp != NULL)
            {
                for (i=0; i<num_cols; i++)
                {
                    (*v_trans_mpp)->elements[ j ][ i ] =
                                                v_mp->elements[ i ][ index ];
                }
            }
        }
    }

    free_matrix(A_mp);
    free_vector(d_vp);
    free_matrix(v_mp);
    free_indexed_vector(indexed_vector_ptr);

    return result;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/***************************************************************
*
*  Routine SVD() in svd.c changed to do_svd_guts() by Kobus.
*  Several modifications have been made, but the heart of the
*  procedure was not changed. Original header follows ...
*
*  File:       svd.c
*  Adapted by: Ian Harder
*  Date:       1991.07.27
*
*  This singular value decomposition routine is adapted from
*  one provided in "Numerical Recipes in C", by W. Press et al.,
*  Cambridge Press, 1988.  Changes have been made to make this
*  consistent with the abstract data types Vector and Matrix I
*  have created.  I apologize for being too lazy to change the
*  original cryptic variable names to more meaningful ones.
*
****************************************************************/


/* macros */

/* compute sqrt(a^2 + b^2) without destructive over or underflow */
static double at, bt, ct;
#define Pythag(a, b) ((at = fabs(a)) > (bt = fabs(b)) ? \
        (ct = bt/at, at*sqrt(1.0+ct*ct)) : \
        (bt ? (ct = at/bt, bt*sqrt(1.0+ct*ct)) : 0.0))

/* compute max while reducing function calls */
static double maxa, maxb;
#define Max(a, b) (maxa = (a), maxb = (b), (maxa) > (maxb) ? \
        (maxa) : (maxb))

#define Sign(a, b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

#define MAXITS 100

static int do_numerical_recipes_svd_guts
(
    Matrix*  A,
    Vector** d_vpp,
    Matrix** v_mpp
)
{
/* Given a matrix A[1..m][1..n], this routine computes its singular
   value decomposition, A = U*W*Vt (where Vt is the transpose of V).
   The matrix U replaces A on output.  The diagonal matrix of singular
   values W is output as a vector W[1..n].  The matrix V (not Vt) is
   output as V[1..n][1..n].  The input matrices and vector must
   already have been created, however only A must contain valid input
   data.  No restrictions are put on matrix sizes, (aside from the
   obvious requirement that A be at least 1x1).  Both W and V will be
   initialized to their correct sizes if required.  On success 1 is
   returned, otherwise 0.
*/
   Vector *W;
   Matrix *V;
   int    flag, i, its, j, jj, k, l = NOT_SET, nm = NOT_SET;
   double c, f, h, s, x, y, z;
   double anorm = 0.0, g = 0.0, scale = 0.0;
   double **a, *w, **v, *rv1;
   Vector *Rv1;


   ERE(get_zero_vector(d_vpp, A->num_cols));
   W = *d_vpp;

   ERE(get_zero_matrix(v_mpp, A->num_cols, A->num_cols));
   V = *v_mpp;

   ASSERT(V->num_rows == A->num_cols);
   ASSERT(V->num_cols == A->num_cols);
   ASSERT(W->length == A->num_cols);
   ASSERT(A->num_rows >= A->num_cols);

   a = A->elements; w = W->elements; v = V->elements;

   /* create working vector */
   NRE(Rv1 = create_vector(A->num_cols));
   rv1 = Rv1->elements;

   /* Householder reduction to bidiagonal form */
   for (i = 0; i < A->num_cols; i++) {
      l = i+1;
      rv1[i] = scale*g;
      g = s = scale = 0.0;
      if (i < A->num_rows) {
         for (k = i; k < A->num_rows; k++) scale += fabs(a[k][i]);
         if (scale) {
            for (k = i; k < A->num_rows; k++) {
               a[k][i] /= scale;
               s += a[k][i] * a[k][i];
            }
            f = a[i][i];
            g = -Sign(sqrt(s), f);
            h = f*g - s;
            a[i][i] = f-g;
            if (i != A->num_cols-1) {
               for (j = l; j < A->num_cols; j++) {
                  for (s = 0.0, k = i; k < A->num_rows; k++)
                     s += a[k][i]*a[k][j];
                  f = s/h;
                  for (k = i; k < A->num_rows; k++) a[k][j] += f*a[k][i];
               }
            }
            for (k = i; k < A->num_rows; k++) a[k][i] *= scale;
         }
      }
      w[i] = scale * g;
      g = s = scale = 0.0;
      if (i < A->num_rows && i != A->num_cols-1) {
         for (k = l; k < A->num_cols; k++) scale += fabs(a[i][k]);
         if (scale) {
            for (k = l; k < A->num_cols; k++) {
               a[i][k] /= scale;
               s += a[i][k] * a[i][k];
            }
            f = a[i][l];
            g = -Sign(sqrt(s), f);
            h = f*g - s;
            a[i][l] = f-g;
            for (k = l; k < A->num_cols; k++) rv1[k] = a[i][k]/h;
            if (i != A->num_rows-1) {
               for (j = l; j < A->num_rows; j++) {
                  for (s = 0.0, k = l; k < A->num_cols; k++)
                     s += a[j][k] * a[i][k];
                  for (k = l; k < A->num_cols; k++) a[j][k] += s * rv1[k];
               }
            }
            for (k = l; k < A->num_cols; k++) a[i][k] *= scale;
         }
      }
      anorm = Max(anorm, (fabs(w[i]) + fabs(rv1[i])));
   }
   /* accumulation of right-hand transformations */
   for (i = A->num_cols-1; i >= 0; i--) {
      if (i < A->num_cols-1) {
         if (g) {
            for (j = l; j < A->num_cols; j++)
               /* double division to avoid possible underflow */
               v[j][i] = (a[i][j] / a[i][l]) / g;
            for (j = l; j < A->num_cols; j++) {
               for (s = 0.0, k = l; k < A->num_cols; k++)
                  s += a[i][k] * v[k][j];
               for (k = l; k < A->num_cols; k++)
                  v[k][j] += s * v[k][i];
            }
         }
         for (j = l; j < A->num_cols; j++) v[i][j] = v[j][i] = 0.0;
      }
      v[i][i] = 1.0;
      g = rv1[i];
      l = i;
   }
   /* accumulation of left-hand transformations */
   for (i = A->num_cols-1; i >= 0; i--) {
      l = i+1;
      g = w[i];
      if (i < A->num_cols-1)
         for (j = l; j < A->num_cols; j++) a[i][j] = 0.0;
      if (g) {
         g = 1.0 / g;
         if (i != A->num_cols-1) {
            for (j = l; j < A->num_cols; j++) {
               for (s = 0.0, k = l; k < A->num_rows; k++)
                  s += a[k][i] * a[k][j];
               f = (s/a[i][i]) * g;
               for (k = i; k < A->num_rows; k++) a[k][j] += f * a[k][i];
            }
         }
         for (j = i; j < A->num_rows; j++) a[j][i] *= g;
      }
      else
         for (j = i; j < A->num_rows; j++) a[j][i] = 0.0;
      ++a[i][i];
   }
   /* diagonalization of the bidiagonal form */
   for (k = A->num_cols-1; k >= 0; k--) {
      for (its = 1; its <= MAXITS; its++) {
         flag = 1;
         for (l = k; l >= 0; l--) {
            /* test for splitting */
            nm = l-1;
            /* note that rv1[0] is always 0 */
            if (fabs(rv1[l])+anorm == anorm) {
               flag = 0;
               break;
            }
            if (fabs(w[nm])+anorm == anorm) break;
         }
         if (flag) {
            c = 0.0;
            s = 1.0;
            for (i = l; i <= k; i++) {
               f = s * rv1[i];
               if (fabs(f) + anorm != anorm) {
                  g = w[i];
                  h = Pythag(f, g);
                  w[i] = h;
                  h = 1.0 / h;
                  c = g * h;
                  s = (-f * h);
                  for (j = 0; j < A->num_rows; j++) {
                     y = a[j][nm];
                     z = a[j][i];
                     a[j][nm] = y*c + z*s;
                     a[j][i] = z*c - y*s;
                  }
               }
            }
         }
         z = w[k];
         /* convergence if l == k */
         if (l == k) {
            /* singular value is made nonnegative */
            if (z < 0.0) {
               w[k] = -z;
               for (j = 0; j < A->num_cols; j++) v[j][k] = (-v[j][k]);
            }
            /* 1991.01.08 (iah) first column of a & v nonnegative */
            /* this is a kludge!! */
            if (k == 0) {
               for (j = 0; j < A->num_rows; j++)
                  a[j][k] = (-a[j][k]);
               for (j = 0; j < A->num_cols; j++)
                  v[j][k] = (-v[j][k]);
             }
            break;
         }
         if (its == MAXITS) {
            /*
            // Modfication by Kobus.
            //
            // SetVisionError(VE_CONVERGE, "in function do_svd");
            // return (0);
            */
            free_vector(Rv1);
            set_error("Singular value decomposition failed to converge.\n");
            return ERROR;
         }
         x = w[l];
         nm = k - 1;
         y = w[nm];
         g = rv1[nm];
         h = rv1[k];
         f = ((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
         g = Pythag(f, 1.0);
         f = ((x-z)*(x+z)+h*((y/(f+Sign(g, f)))-h))/x;
         /* next QR transformation */
         c = s = 1.0;
         for (j = l; j <= nm; j++) {
            i = j + 1;
            g = rv1[i];
            y = w[i];
            h = s * g;
            g = c * g;
            z = Pythag(f, h);
            rv1[j] = z;
            c = f / z;
            s = h / z;
            f = x*c + g*s;
            g = g*c - x*s;
            h = y * s;
            y = y * c;
            for (jj = 0; jj < A->num_cols; jj++) {
               x = v[jj][j];
               z = v[jj][i];
               v[jj][j] = x*c + z*s;
               v[jj][i] = z*c - x*s;
            }
            z = Pythag(f, h);
            w[j] = z;
            if (z) {
               z = 1.0 / z;
               c = f * z;
               s = h * z;
            }
            f = c*g + s*y;
            x = c*y - s*g;
            for (jj = 0; jj < A->num_rows; jj++) {
               y = a[jj][j];
               z = a[jj][i];
               a[jj][j] = y*c + z*s;
               a[jj][i] = z*c - y*s;
            }
         }
         rv1[l] = 0.0;
         rv1[k] = f;
         w[k] = x;
      }
   }
   free_vector(Rv1);

   return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

