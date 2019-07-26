
/* $Id: mat_max_and_min.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"

/*
#define VERBOSE 1
*/

#define NUM_LOOPS        20
#define BASE_NUM_TRIES   10


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int num_rows;
    int num_cols; 
    int count;
    Matrix* first_mp = NULL;
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;
    Vector* max_row_vp = NULL; 
    Int_vector* max_index_vp = NULL; 
    Vector* min_row_vp = NULL; 
    Int_vector* min_index_vp = NULL; 
    int i,j;


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
#ifdef VERBOSE
                pso("\n-------------------------------------------------\n\n");
                pso("%d %d %d\n\n", count, num_rows, num_cols);
#endif 

                
                EPETE(get_random_matrix(&first_mp, num_rows, num_cols)); 

#ifdef VERBOSE
                db_mat(first_mp); 
#endif 

                EPETE(get_max_matrix_row_elements_2(&max_row_vp, &max_index_vp, first_mp));

                for (i = 0; i < num_rows; i++)
                {
                    if (! IS_EQUAL_DBL(max_row_vp->elements[ i ],
                                       first_mp->elements[ i ][ max_index_vp->elements[ i ] ])
                       )
                    {
                        p_stderr("Row %d: %e != %e.\n", 
                                 i, max_row_vp->elements[ i ],
                                 first_mp->elements[ i ][ max_index_vp->elements[ i ] ]);
                        status = EXIT_BUG;
                    }

                    for (j = 0; j < num_cols; j++)
                    {
                         if (max_row_vp->elements[ i ] < first_mp->elements[ i ][ j ])
                         {
                            p_stderr("Row %d max (%e) < element in col %d (%e).\n", 
                                     i, max_row_vp->elements[ i ],
                                     j, first_mp->elements[ i ][ j ]);
                            status = EXIT_BUG;
                         }
                    }
                }

#ifdef VERBOSE
                db_cv(max_row_vp); 
                db_icv(max_index_vp); 
#endif 
                EPETE(get_min_matrix_row_elements_2(&min_row_vp, &min_index_vp, first_mp));

                for (i = 0; i < num_rows; i++)
                {
                    if (! IS_EQUAL_DBL(min_row_vp->elements[ i ],
                                       first_mp->elements[ i ][ min_index_vp->elements[ i ] ])
                       )
                    {
                        p_stderr("Row %d: %e != %e.\n", 
                                 i, min_row_vp->elements[ i ],
                                 first_mp->elements[ i ][ min_index_vp->elements[ i ] ]);
                        status = EXIT_BUG;
                    }

                    for (j = 0; j < num_cols; j++)
                    {
                         if (min_row_vp->elements[ i ] > first_mp->elements[ i ][ j ])
                         {
                            p_stderr("Row %d min (%e) > element in col %d (%e).\n", 
                                     i, min_row_vp->elements[ i ],
                                     j, first_mp->elements[ i ][ j ]);
                            status = EXIT_BUG;
                         }
                    }
                }

#ifdef VERBOSE
                db_cv(min_row_vp); 
                db_icv(min_index_vp); 
#endif 

            }
        }
    }
    
    free_matrix(first_mp); 
    free_vector(max_row_vp); 
    free_int_vector(max_index_vp); 
    free_vector(min_row_vp); 
    free_int_vector(min_index_vp); 


    return status; 
}

