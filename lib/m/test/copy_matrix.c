
/* $Id: copy_matrix.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h" 

#define MAX_MATRIX_VECTOR_LEN  20
#define MAX_NUM_ROWS   500
#define MAX_NUM_COLS   500
#define BASE_NUM_TRIES 500

/*ARGSUSED*/
int main(int argc, char **argv)
{
    Matrix* mp = NULL;
    Matrix* select_mp = NULL;
    Matrix* mp2 = NULL;
    Matrix* cat_mp = NULL;
    Matrix* s1_mp = NULL;
    Matrix* s2_mp = NULL;
    Int_vector* select_cols_vp = NULL; 
    Vector* c1_vp = NULL; 
    Vector* c2_vp = NULL; 
    Matrix_vector* mvp = NULL; 
    int i;
    int select_count;
    int matrix_vector_len;
    long memcpy_cpu = 0;
    long std_cpy_cpu   = 0; 
    int  num_tries = BASE_NUM_TRIES;
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
    }
    else
    {
        num_tries *= test_factor;
    }

    kjb_l_set("debug", "2");
    kjb_l_set("page", "off");

    /* Test corner case first with empty matrices. */
    EPETE(get_random_matrix(&mp, 0, 0));
    EPETE(copy_matrix(&mp2, mp));

    for (i = 0; i < num_tries; i++)
    {
        int    r;
        int    c;
        int    m;
        double diff;
        int    num_rows = 1 + MAX_NUM_ROWS * kjb_rand();
        int    num_cols = 1 + MAX_NUM_COLS * kjb_rand();

        kjb_l_set("use-memcpy", "f");

        EPETE(get_random_matrix(&mp, num_rows, num_cols));

        r = kjb_rint((double)num_rows * kjb_rand());
        r = MIN_OF(r, num_rows - 1);

        c = kjb_rint((double)num_cols * kjb_rand());
        c = MIN_OF(c , num_cols - 1);

        ASSERT_IS_LESS_INT(r, num_rows);
        ASSERT_IS_LESS_INT(c, num_cols);

        mp->elements[ r ][ c ] = i;

        init_cpu_time();
        EPETE(copy_matrix(&mp2, mp));
        std_cpy_cpu += get_cpu_time(); 

        diff = max_abs_matrix_difference(mp, mp2);

        if (diff > DBL_MIN) 
        {
            p_stderr("Max diff (%.3e) is not zero (without memcpy).\n", diff);
            status = EXIT_BUG;
        }

        kjb_l_set("use-memcpy", "t");

        r = kjb_rint((double)num_rows * kjb_rand());
        r = MIN_OF(r, num_rows - 1);

        c = kjb_rint((double)num_cols * kjb_rand());
        c = MIN_OF(c , num_cols - 1);

        ASSERT_IS_LESS_INT(r, num_rows);
        ASSERT_IS_LESS_INT(c, num_cols);

        mp->elements[ r ][ c ] = i;

        init_cpu_time();
        EPETE(copy_matrix(&mp2, mp));
        memcpy_cpu += get_cpu_time(); 

        diff = max_abs_matrix_difference(mp, mp2);

        if (diff > DBL_MIN) 
        {
            p_stderr("Max diff (%.3e) is not zero (memcpy).\n", diff);
            status = EXIT_BUG;
        }

        /*
         * Now, column selection. 
        */

        ERE(get_initialized_int_vector(&select_cols_vp, num_cols, FALSE));
        for (c = 0; c < num_cols; c++)
        {
            if (kjb_rand() < 0.3)
            {
                select_cols_vp->elements[ c ] = TRUE;
            }
        }
        ERE(select_matrix_cols(&select_mp, mp, select_cols_vp));

        select_count = 0;

        for (c = 0; c < num_cols; c++)
        {
            if (select_cols_vp->elements[ c ])
            {
                ERE(get_matrix_col(&c1_vp, mp, c));
                ERE(get_matrix_col(&c2_vp, select_mp, select_count));

                diff = max_abs_vector_difference(c1_vp, c2_vp);

                if (diff > DBL_MIN) 
                {
                    p_stderr("Max diff (%.3e) is not zero for selected column.\n", diff);
                    status = EXIT_BUG;
                }

                select_count++; 
            }
        }

        matrix_vector_len = kjb_rint(MAX_MATRIX_VECTOR_LEN * kjb_rand());

        ERE(get_target_matrix_vector(&mvp, matrix_vector_len));

        for (m = 0; m <matrix_vector_len; m++)
        {
            if (kjb_rand() > 0.5) 
            {
                if(r > 0 && c > 0)
                {
                    ERE(get_random_matrix(&(mvp->elements[ m ]), r, c)); 
                }
            }
            else 
            {
                free_matrix(mvp->elements[ m ]);
                mvp->elements[ m ] = NULL;
            }
        }

        ERE(get_matrix_from_matrix_vector(&cat_mp, mvp));

        ERE(select_matrix_cols(&s1_mp, cat_mp, select_cols_vp));
        ERE(get_matrix_from_matrix_vector_with_col_selection(&s2_mp, mvp, select_cols_vp));

        diff = max_abs_matrix_difference(s1_mp, s2_mp);

        if (diff > DBL_MIN) 
        {
            p_stderr("Max diff (%.3e) is not zero matrices from matrix vectors with selection.\n", diff);
            status = EXIT_BUG;
        }
    }

    if (is_interactive())
    {
        dbi(std_cpy_cpu);
        dbi(memcpy_cpu);
    }

    free_vector(c1_vp);
    free_vector(c2_vp);
    free_matrix(mp); 
    free_matrix(mp2); 
    free_matrix(cat_mp);
    free_matrix(s1_mp);
    free_matrix(s2_mp);
    free_matrix(select_mp); 
    free_matrix_vector(mvp); 
    free_int_vector(select_cols_vp); 

    return status; 
}

