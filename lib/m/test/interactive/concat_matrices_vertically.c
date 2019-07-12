
/* $Id: concat_matrices_vertically.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "m/m_incl.h" 

#define NUM_MATRICES 5
#define NUM_ROWS 3
#define NUM_COLS 5

/*ARGSUSED*/
int main(int argc, char **argv)
{
    Matrix* matrix_list[ NUM_MATRICES ]; 
    Matrix* mp = NULL;
    int i, j;
    Matrix_vector* mvp = NULL;


    for (i=0; i<NUM_MATRICES; i++)
    {
        matrix_list[ i ] = NULL;
    }

    for (i=0; i<NUM_MATRICES; i++)
    {
        for (j=0; j<i; j++)
        {
            if (IS_EVEN(i) && ((j==0) || (j == (i-1))))
            {
                free_matrix(matrix_list[ j ]);
                matrix_list[ j ] = NULL;
            }
            else if (IS_ODD(i) && IS_EVEN(j))
            {
                free_matrix(matrix_list[ j ]);
                matrix_list[ j ] = NULL;
            }
            else
            {
                EPETE(get_random_matrix(&(matrix_list[ j ]), 
                                        (int)(1.1 + 5.0 * kjb_rand()), 5));
            }
            db_mat(matrix_list[ j ]);
        }

        dbp("--------");

        EPETE(concat_matrices_vertically(&mp, j, matrix_list));

        db_mat(mp); 
        dbp("===================");
    }

    
    EPETE(get_target_matrix_vector(&mvp, NUM_MATRICES));

    for (i = 0; i < NUM_MATRICES; i++)
    {
        if (IS_ODD(i))
        {
            EPETE(get_random_matrix(&(mvp->elements[ i ]), NUM_ROWS, NUM_COLS));
        }
        else 
        {
            free_matrix(mvp->elements[ i ]);
            mvp->elements[ i ] = NULL;            
        }

        db_mat(mvp->elements[ i ]);

    }

    dbp("-----------------");

    EPETE(get_matrix_from_matrix_vector(&mp, mvp));

    db_mat(mp);

    dbp("===================");

    for (i = 0; i < NUM_MATRICES; i++)
    {
        if (IS_EVEN(i))
        {
            EPETE(get_random_matrix(&(mvp->elements[ i ]), NUM_ROWS, NUM_COLS));
        }
        else 
        {
            free_matrix(mvp->elements[ i ]);
            mvp->elements[ i ] = NULL;            
        }

        db_mat(mvp->elements[ i ]);

    }

    dbp("-----------------");

    EPETE(get_matrix_from_matrix_vector(&mp, mvp));

    db_mat(mp);

    dbp("===================");

    free_matrix(mp); 

    for (i=0; i<NUM_MATRICES; i++)
    {
        free_matrix(matrix_list[ i ]);
    }

    free_matrix_vector(mvp);

    return EXIT_SUCCESS; 
}

