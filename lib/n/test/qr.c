/* $Id */

/*
 * This program tests the qr-decomposition routines
 */


#include "m/m_incl.h"
#include "n/n_qr.h" 
#include "n/n_svd.h"

/*
#define VERBOSE           1
*/


#define MAX_DIMENSION   25
#define BASE_NUM_TRIES  2

#define EPEAE(x)      if ((x) == ERROR)                                        \
                      {                                                        \
                          /* extern int kjb_debug_level; */                    \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(EPEAE on line %d of %s.)",           \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          kjb_print_error();                                   \
                          abort();                                             \
                      }                                                        \
                      else                                                     \
                      {                                                        \
                          ; /* Do nothing */                                   \
                      }

static int test_rq_decompose();
static int test_qr_decompose();
static int test_lq_decompose();
static int test_ql_decompose();
static void test_orthonormal(const Matrix* Q_mp);
static void test_recompose(const Matrix* M_mp, const Matrix* M1_mp, const Matrix* M2_mp);


int num_rows;
int num_cols;
int count;
int  num_tries = BASE_NUM_TRIES;
int  test_factor = 1;
int max_dimension = MAX_DIMENSION;
int i,j;

double det;

double diff;

Matrix* M_mp = NULL;
Matrix* Q_mp = NULL;
Matrix* R_mp = NULL;
Matrix* L_mp = NULL;
Matrix* I_mp = NULL;
Matrix* temp_mp = NULL;

Matrix* result_mp = NULL;
int status = EXIT_SUCCESS;

/*ARGSUSED*/
int main(int argc, char **argv)
{
    kjb_init();


    if (argc > 1)
    {
        EPEAE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (test_factor == 0)
    {
        num_tries = 1;
        max_dimension = 10;
        
    }
    else
    {
        num_tries *= test_factor;
    }

    EPEAE(get_target_matrix(&Q_mp, 0, 0));
    EPEAE(get_target_matrix(&R_mp, 0, 0));

    for (count=0; count<num_tries; count++)
    {
        for (num_rows=1; num_rows<max_dimension; num_rows++)
        {
            for (num_cols=1; num_cols<max_dimension; num_cols++)
            {

                /*
                //   pso("%d %d %d\n", count, num_rows, num_cols);
                */   

                EPEAE(get_random_matrix(&M_mp, num_rows, num_cols));

                test_qr_decompose();
                test_rq_decompose();
                test_ql_decompose();
                test_lq_decompose();
            }
        }
    }
    /* finish cleaning up */

    free_matrix(M_mp);
    free_matrix(Q_mp);
    free_matrix(R_mp);
    free_matrix(L_mp);
    free_matrix(I_mp);
    free_matrix(temp_mp);
    free_matrix(result_mp);

    return status;
}

int test_lq_decompose()
{
    int M, N;
    int done;
    EPEAE(lq_decompose(M_mp, &L_mp, &Q_mp));
    
    test_recompose(M_mp, L_mp, Q_mp);
    test_orthonormal(Q_mp);

    M = L_mp->num_rows;
    N = L_mp->num_cols;
    /* test that L is left-triangular */
    done = 0;
    for(i = 0; i < M; i++)
    {
        for(j = 0; j < N; j++)
        {
            if( i < j )
            {
                if(!IS_ZERO_DBL(L_mp->elements[i][j]))
                {
                    p_stderr("Problem with test (L is not left-triangular).\n", det);
                    status = EXIT_BUG;
                    done = 1;
                }

                if(done) break;
            }
            if(done) break;
        }
        if(done) break;
    }

    return NO_ERROR;
}

int test_rq_decompose()
{
    int M, N;
    int done;

    EPEAE(rq_decompose(M_mp, &R_mp, &Q_mp));

    test_recompose(M_mp, R_mp, Q_mp);
    test_orthonormal(Q_mp);


    M = R_mp->num_rows;
    N = R_mp->num_cols;
    /* test that R is right-triangular */
    done = 0;
    for(i = 0; i < R_mp->num_rows; i++)
    {
        for(j = 0; j < R_mp->num_cols; j++)
        {
            if( i > j + M - N)
            {
                if(!IS_ZERO_DBL(R_mp->elements[i][j]))
                {
                    p_stderr("Problem with test (R is not right-triangular).\n", det);
                    status = EXIT_BUG;
                    done = 1;
                }

                if(done) break;
            }
            if(done) break;
        }
        if(done) break;
    }

    return NO_ERROR;
}


int test_ql_decompose()
{
    int M, N;

    EPEAE(ql_decompose(M_mp, &Q_mp, &L_mp));

    test_recompose(M_mp, Q_mp, L_mp);
    test_orthonormal(Q_mp);

    M = L_mp->num_rows;
    N = L_mp->num_cols;

    /* test that L is left-triangular */
    for(i = 0; i < L_mp->num_rows; i++)
    {
        for(j = 0; j < MIN(i, L_mp->num_cols); j++)
        {
            if( i < j + M - N)
            {
                if(!IS_ZERO_DBL(L_mp->elements[i][j]))
                {
                    p_stderr("Problem with test (L is not L-triangular).\n", det);
                    status = EXIT_BUG;
                }
            }

        }
    }

    return NO_ERROR;
}

int test_qr_decompose()
{
    EPEAE(qr_decompose(M_mp, &Q_mp, &R_mp));

    test_recompose(M_mp, Q_mp, R_mp);
    test_orthonormal(Q_mp);

    /* test that R is right-triangular */
    for(i = 0; i < R_mp->num_rows; i++)
    {
        for(j = 0; j < MIN(i, R_mp->num_cols); j++)
        {
            if(j < i)
            {
                if(!IS_ZERO_DBL(R_mp->elements[i][j]))
                {
                    p_stderr("Problem with test (R is not right-triangular).\n", det);
                    status = EXIT_BUG;
                }
            }

        }
    }

    return NO_ERROR;
}

void test_orthonormal(const Matrix* Q_mp)
{
    double det;

    get_determinant_abs(Q_mp, &det);
    if( fabs(det-1) >  1e-14)
    {
        p_stderr("Problem with test (|Q| = %e != 1.0).\n", det);
        status = EXIT_BUG;
    }

    multiply_by_own_transpose(&result_mp, Q_mp);
    get_identity_matrix(&I_mp, Q_mp->num_rows);
    diff = max_abs_matrix_difference(I_mp, result_mp);

    if (diff > 1e-14)
    {
        p_stderr("Problem with test (max(|Q' Q - I|) = %e != 0.0).\n", diff);
        status = EXIT_BUG;
    }

}

void test_recompose(const Matrix* M_mp, const Matrix* M1_mp, const Matrix* M2_mp)
{
    EPEAE(multiply_matrices(&result_mp, M1_mp, M2_mp));

    diff = max_abs_matrix_difference(M_mp, result_mp);

    if ( diff > 1e-14)
    {
        p_stderr("Problem with test (%e != 0.0).\n", diff);
        status = EXIT_BUG;
    }
}
