
/* $Id: interleave_matrix_rows.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "m/m_incl.h" 

#define NUM_MATRICES 5
#define NUM_ROWS 5
#define NUM_COLS 3

/*ARGSUSED*/
int main(int argc, char **argv)
{
#ifdef HAVE_INTERLEAVE_MATRIX_COLS_2
    Matrix* list[ NUM_MATRICES ]; 
    int j;
#endif 
    int i;
    Matrix* mp = NULL;
    Matrix_vector* mvp = NULL;


#ifdef HAVE_INTERLEAVE_MATRIX_COLS_2
    for (i=0; i<NUM_MATRICES; i++)
    {
        list[ i ] = NULL;
    }

    for (i=0; i<NUM_MATRICES; i++)
    {
        int num_rows = 1.1 + 5.0 * kjb_rand();
        int num_cols = 1.1 + 5.0 * kjb_rand();

        for (j=0; j<i; j++)
        {
            if (IS_EVEN(i) && ((j==0) || (j == (i-1))))
            {
                free_matrix(list[ j ]);
                list[ j ] = NULL;
            }
            else if (IS_ODD(i) && IS_EVEN(j))
            {
                free_matrix(list[ j ]);
                list[ j ] = NULL;
            }
            else
            {
                EPETE(get_random_matrix(&(list[ j ]), num_rows, num_cols));
            }
            db_mat(list[ j ]);
        }

        dbp("--------");

        EPETE(interleave_matrix_rows_2(&mp, j, list));

        db_mat(mp); 
        dbp("===================");
    }
#endif 

    
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

    EPETE(interleave_matrix_rows(&mp, mvp));

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

    EPETE(interleave_matrix_rows(&mp, mvp));

    db_mat(mp);

    dbp("===================");

    free_matrix(mp); 

#ifdef HAVE_INTERLEAVE_MATRIX_COLS_2
    for (i=0; i<NUM_MATRICES; i++)
    {
        free_matrix(list[ i ]);
    }
#endif 

    free_matrix_vector(mvp);

    return EXIT_SUCCESS; 
}

