
/* $Id: int_matrix.c 21491 2017-07-20 13:19:02Z kobus $ */


#define BASE_NUM_TRIES  10000
#define MAX_DIM         200

#include "l/l_incl.h" 

int main(int argc, char **argv)
{
    int    try_count, i, j;
    int    test_factor = 1; 
    int num_tries = BASE_NUM_TRIES; 
    Int_matrix* mp = NULL; 
    int prev_num_rows;
    int prev_num_cols;


    kjb_init();

    /* Test corner cases: 0 x 0 matrix, n x 0 matrix, or 0 x n */
    EPETE(get_target_int_matrix(&mp, 0, 0));
    dbx(mp->elements);
    ow_set_int_matrix(mp, -1); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 

        if (test_factor == 0) 
        {
            num_tries = 100;
        }
        else
        {
            num_tries *= test_factor;
        }
    }

    if (is_interactive())
    {
        kjb_set_debug_level(2); 
    }
    else 
    {
        kjb_set_debug_level(0); 
    }

    prev_num_rows = MAX_DIM / 10; 
    prev_num_cols = MAX_DIM / 10; 
    EPETE(ra_get_target_int_matrix(&mp, prev_num_rows, prev_num_cols));

    for (i = 0; i < mp->num_rows; i++)
    {
        for (j = 0; j < mp->num_cols; j++)
        {
            mp->elements[ i ][ j ] = i*j;
        }
    }

    for (try_count = 0; try_count< num_tries; try_count++)
    {
        int num_rows = MIN_OF(MAX_DIM - 1, 2.0 + (kjb_rand() * (double)MAX_DIM));
        int num_cols = MIN_OF(MAX_DIM - 1, 2.0 + (kjb_rand() * (double)MAX_DIM));

        if (try_count % 2 == 0)
        {
            num_cols = prev_num_cols; 
        }

        if (try_count % 7 == 0)
        {
            num_rows = 1;
        }

        if (try_count % 11 == 0)
        {
            num_cols = 1;
        }

        if (try_count % 13 == 0)
        {
            num_rows += 20 * MAX_DIM;
        }

        if (try_count % 17 == 0)
        {
            num_cols += 20 * MAX_DIM;
        }

#ifdef VERBOSE
        dbi(prev_num_rows);
        dbi(prev_num_cols);
        dbi(num_rows);
        dbi(num_cols);
        dbx(mp->elements); 

        if (mp->elements != NULL)
        {
            dbx(mp->elements[ 0 ]); 
        }

        dbi_mat(mp); 
#endif 

        EPETE(ra_get_target_int_matrix(&mp, num_rows, num_cols));

#ifdef VERBOSE
        dbi_mat(mp); 
#endif 

        for (i = 0; i < MIN_OF(num_rows, prev_num_rows); i++)
        {
            for (j = 0; j < MIN_OF(num_cols, prev_num_cols); j++)
            {
                if (mp->elements[ i ][ j ] != (try_count + i*j))
                {
                    dbi(mp->elements[ i ][ j ]); 
                    dbi(try_count + i*j); 
                    p_stderr("Preservation condition failed for element (%d, %d).\n", i, j);
                    return EXIT_BUG;
                }
            }
        }

#ifdef VERBOSE
        dbp("-----------------"); 
#endif 

        for (i = 0; i < mp->num_rows; i++)
        {
            for (j = 0; j < mp->num_cols; j++)
            {
                mp->elements[ i ][ j ] = 1 + try_count + i*j;
            }
        }

        prev_num_rows = num_rows;
        prev_num_cols = num_cols;
    }

    free_int_matrix(mp);

    kjb_cleanup();
    
    return EXIT_SUCCESS;
}

