
/* $Id: is_symmetric.c 21491 2017-07-20 13:19:02Z kobus $ */


/*
 * This program tests the general eigen-solver in the case of symmetric
 * matrices (ONLY).
*/

#include "m/m_incl.h"


#define NUM_LOOPS     100
#define FIRST_ROW       1
#define FIRST_COL       1
#define BASE_NUM_TRIES  5


/*ARGSUSED*/
int main(int argc, char* argv[])
{
    int     num_rows;
    int     num_cols;
    int     count;
    Matrix* first_mp           = NULL;
    Matrix* sym_mp           = NULL;
    int num_elements; 
    int i, j;
    int  num_tries = NOT_SET;
    int  num_loops = NOT_SET;
    int  test_factor = 1;
    int status = EXIT_SUCCESS;


    kjb_init(); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (test_factor <= 0)
    {
        num_tries = 1;
        num_loops = 5;
    }
    else
    {   
        double factor_for_linear = pow((double)test_factor, 1.0/3.0);

        num_loops = kjb_rint((double)NUM_LOOPS * factor_for_linear);
        num_tries = kjb_rint((double)BASE_NUM_TRIES * factor_for_linear);
    } 

    if ( ! is_symmetric_matrix(NULL)) 
    {
        p_stderr("Test 0 (null matrix) failed.\n");
        kjb_exit(EXIT_BUG);
    }

    for (count=0; count<num_tries; count++)
    {
        for (num_rows=FIRST_ROW; num_rows<NUM_LOOPS; num_rows++)
        {
            for (num_cols=num_rows; num_cols<NUM_LOOPS; num_cols++)
            {
                if (kjb_rand() < 0.5)
                {
                    EPETE(get_random_matrix(&first_mp, num_rows, num_cols)); 
                    EPETE(ow_subtract_scalar_from_matrix(first_mp, 0.5)); 

                    if (((num_rows != 1) || (num_cols != 1)) && (is_symmetric_matrix(first_mp)))
                    {
                        p_stderr("Presumed non-symmetric matrix is symmetric.\n"); 
                        db_mat(first_mp);
                        kjb_exit(EXIT_BUG); 
                    }

                    EPETE(multiply_by_own_transpose(&sym_mp, first_mp));

                    ASSERT(sym_mp->num_rows == MIN_OF(first_mp->num_rows,
                                                      first_mp->num_cols)); 
                }
                else
                {
                    EPETE(get_random_matrix(&sym_mp,
                                            MIN_OF(num_rows, num_cols), 
                                            MIN_OF(num_rows, num_cols)));
                    EPETE(ow_subtract_scalar_from_matrix(sym_mp, 0.5)); 

                    if ((sym_mp->num_rows != 1) && (is_symmetric_matrix(sym_mp)))
                    {
                        p_stderr("Presumed non-symmetric matrix is symmetric.\n"); 
                        db_mat(sym_mp);
                        kjb_exit(EXIT_BUG); 
                    }

                    for (i = 0; i < MIN_OF(num_rows, num_cols); i++)
                    {
                        for (j = 0; j < MIN_OF(num_rows, num_cols); j++)
                        {
                            if (j < i)
                            {
                                sym_mp->elements[ i ][ j ] = sym_mp->elements[ j ][ i ];
                            }
                        }
                    }
                }

                ASSERT(sym_mp->num_rows == sym_mp->num_cols); 

                num_elements = sym_mp->num_rows * sym_mp->num_cols;

                if (! is_symmetric_matrix(sym_mp))
                {
                    p_stderr("Presumed symmetric matrix is not symmetric.\n"); 
                    db_mat(sym_mp);
                    kjb_exit(EXIT_BUG); 
                }

            }
        }
    }

    free_matrix(sym_mp); 
    free_matrix(first_mp); 


    return status; 
}

