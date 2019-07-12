
/* $Id: mat_arith_row.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"

#define BASE_NUM_LOOPS_FOR_EW_TEST     30


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int     num_second_rows;
    int     num_cols;
    int     num_first_rows;
    Matrix* a_mp            = NULL;
    Matrix* first_mp        = NULL;
    Matrix* second_mp       = NULL;
    int  num_loops = BASE_NUM_LOOPS_FOR_EW_TEST;
    int  test_factor = 1;


    kjb_init(); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    num_loops = kjb_rint((double)num_loops * pow((double)test_factor, 1.0/3.0));

    for (num_first_rows=1; num_first_rows<=num_loops; num_first_rows++)
    {
        for (num_second_rows=1; num_second_rows<=num_loops; num_second_rows++)
        {
            for (num_cols=1; num_cols<=num_loops; num_cols++)
            {
                int num_rows = num_first_rows * num_second_rows;
                int i, j; 
                double x, y; 


                /*
                pso("%d %d %d\n", num_first_rows, num_second_rows, num_cols);
                */

                EPETE(get_random_matrix(&first_mp, num_first_rows, num_cols)); 
                EPETE(get_random_matrix(&second_mp, num_second_rows, num_cols));

                EPETE(multiply_matrix_rows(&a_mp, first_mp, second_mp));

                /*
                db_mat(first_mp);
                db_mat(second_mp);
                db_mat(a_mp);
                */

                for (i=0; i<num_rows; i++)
                {
                    for (j=0; j<num_cols; j++) 
                    {
                        x = a_mp->elements[ i ][ j ];
                        y = first_mp->elements[ i / num_second_rows][ j ] * 
                                second_mp->elements[ i % num_second_rows ][ j ];

                        if (fabs(x - y) > 0.00001) 
                        {
                            p_stderr("Difference found (%e != %e).\n",
                                     x, y);
                            p_stderr("Coords of first_mp: (%d, %d).\n",
                                     i / num_second_rows, j);
                            p_stderr("Coords of second_mp: (%d, %d).\n",
                                     i % num_first_rows, j);
                            p_stderr("Coords of a_mp: (%d, %d).\n", i, j);

                            status = EXIT_BUG;
                        }
                    }
                }


            }
        }
    }
    
    free_matrix(first_mp); 
    free_matrix(second_mp); 
    free_matrix(a_mp); 

    return status; 
}

