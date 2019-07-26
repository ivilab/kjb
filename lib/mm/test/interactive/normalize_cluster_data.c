
/* $Id: normalize_cluster_data.c 21491 2017-07-20 13:19:02Z kobus $ */


/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003, by members of University of Arizona Computer Vision     |
|  group (the authors) including                                               ||
|        Kobus Barnard.                                                        |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */

/*
//  Most programs need at least the "m" library. 
*/

#include "m/m_incl.h" 
#include "r/r_incl.h" 

/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */

#define NUM_DATA_MATRICES  2
#define NUM_HELD_OUT_DATA_MATRICES  1
#define NUM_ROWS 5
#define NUM_COLS 5 
#define NUM_LOOPS 4

int main(int argc, char** argv)
{
    Matrix_vector* data_mvp = NULL; 
    Matrix_vector* held_out_data_mvp = NULL; 
    Matrix_vector* norm_data_mvp = NULL; 
    Matrix_vector* norm_held_out_data_mvp = NULL; 
    Vector* mean_vp = NULL;
    Vector* var_vp = NULL; 
    int i, j, k; 
    IMPORT int kjb_debug_level;
    int count; 


    kjb_init();   /* Best to do this if using KJB library. */

    if (! is_interactive()) 
    {
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }

    kjb_set_verbose_level(5);
    kjb_debug_level = 10;

    dbi(respect_missing_values());

    EPETE(get_target_matrix_vector(&data_mvp, NUM_DATA_MATRICES));
    EPETE(get_target_matrix_vector(&held_out_data_mvp, NUM_HELD_OUT_DATA_MATRICES));

    for (i = 0; i < NUM_DATA_MATRICES; i++)
    {
        EPETE(get_random_matrix(&(data_mvp->elements[ i ]), NUM_ROWS, NUM_COLS));
    }

    for (i = 0; i < NUM_HELD_OUT_DATA_MATRICES; i++)
    {
        EPETE(get_random_matrix(&(held_out_data_mvp->elements[ i ]), NUM_ROWS, NUM_COLS));
    }

    for (count = 0; count < NUM_LOOPS; count++)
    {
        EPETE(copy_matrix_vector(&norm_data_mvp, data_mvp));
        EPETE(copy_matrix_vector(&norm_held_out_data_mvp, held_out_data_mvp));

        if (count > 0)
        {
            for (i = 0; i < NUM_DATA_MATRICES; i++)
            {
                for (j = 0; j < NUM_ROWS; j++)
                {
                    for (k = 0; k < NUM_COLS; k++)
                    {
                        if (kjb_rand() < (double)count / (double)NUM_LOOPS)
                        {
                            norm_data_mvp->elements[ i ]->elements[ j ][ k ] = DBL_MISSING;
                        }
                    }
                }
            }

            for (i = 0; i < NUM_HELD_OUT_DATA_MATRICES; i++)
            {
                for (j = 0; j < NUM_ROWS; j++)
                {
                    for (k = 0; k < NUM_COLS; k++)
                    {
                        if (kjb_rand() < (double)count / (double)NUM_LOOPS)
                        {
                            norm_held_out_data_mvp->elements[ i ]->elements[ j ][ k ] = DBL_MISSING;
                        }
                    }
                }
            }
        }

        for (i = 0; i < NUM_DATA_MATRICES; i++)
        {
            db_mat(norm_data_mvp->elements[ i ]); 
        }

        for (i = 0; i < NUM_HELD_OUT_DATA_MATRICES; i++)
        {
            db_mat(norm_held_out_data_mvp->elements[ i ]); 
        }

        EPETE(ow_normalize_composite_cluster_data(norm_data_mvp, norm_held_out_data_mvp, 
                                                  &mean_vp, &var_vp, 1.0, NULL)); 

        db_rv(mean_vp);
        db_rv(var_vp); 

        dbp("---------------------------------------"); 

        for (i = 0; i < NUM_DATA_MATRICES; i++)
        {
            db_mat(norm_data_mvp->elements[ i ]); 
        }

        dbp("---------------------------------------"); 
        
        EPETE(ow_perturb_composite_cluster_data(norm_data_mvp, 0.1, NULL)); 

        for (i = 0; i < NUM_DATA_MATRICES; i++)
        {
            db_mat(norm_data_mvp->elements[ i ]); 
        }


    }

    free_matrix_vector(data_mvp);
    free_matrix_vector(held_out_data_mvp);
    free_matrix_vector(norm_data_mvp);
    free_matrix_vector(norm_held_out_data_mvp);
    free_vector(mean_vp);
    free_vector(var_vp);

    kjb_cleanup(); /* Almost never needed, but doing it twice is OK. */

    return EXIT_SUCCESS; 
} 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

