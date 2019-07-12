
/* $Id: perturb.c 21491 2017-07-20 13:19:02Z kobus $ */


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

#define NUM_MATRICES  10
#define NUM_ROWS      5
#define NUM_COLS      8
#define PERTURBATION  (0.10)

int main(int argc, char** argv)
{
    Matrix_vector* input_mvp = NULL; 
    Matrix_vector* perturb_mvp = NULL; 
    Matrix* perturb_mp = NULL; 
    IMPORT int kjb_debug_level;
    int i;
    Int_vector* disable_feature_input_vp = NULL; 
    Int_vector* disable_feature_vp = NULL; 
    Matrix* diff_mp = NULL; 


    kjb_init();   /* Best to do this if using KJB library. */

    if (! is_interactive()) 
    {
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }

    kjb_set_verbose_level(5);
    kjb_debug_level = 10;

    if (argc > 1)
    {
        EPETE(read_int_vector(&disable_feature_input_vp, argv[ 1 ]));
        EPETE(get_zero_int_vector(&disable_feature_vp, NUM_COLS));

        for (i = 0; i < NUM_COLS; i++)
        {
            if (disable_feature_input_vp->length > i)
            {
                disable_feature_vp->elements[ i ] = disable_feature_input_vp->elements[ i ];
            }
        }
    }

    if (disable_feature_vp != NULL)
    {
        db_irv(disable_feature_vp);
    }

    dbp("---------------------------------------------------------------------------------"); 
    dbp("INPUT, value"); 

    EPETE(get_target_matrix_vector(&input_mvp, NUM_MATRICES));

    for (i = 0; i < NUM_MATRICES; i++)
    {
        EPETE(get_unity_matrix(&(input_mvp->elements[ i ]), NUM_ROWS, NUM_COLS));
        EPETE(ow_multiply_matrix_by_scalar(input_mvp->elements[ i ], (double)(i + 1)));
    }

    for (i = 0; i < NUM_MATRICES; i++)
    {
        db_mat(input_mvp->elements[ i ]);
    }

    dbp("---------------------------------------------------------------------------------"); 
    dbp("RESULT, value, composite"); 

    EPETE(set_cluster_lib_options("cluster-perturb-data-by-range", "f"));

    EPETE(perturb_composite_cluster_data(&perturb_mvp, input_mvp, PERTURBATION, disable_feature_vp));
    
    for (i = 0; i < NUM_MATRICES; i++)
    {
        db_mat(perturb_mvp->elements[ i ]);
    }

    dbp("---------------------------------------------------------------------------------"); 
    dbp("RESULT, value, unary"); 

    for (i = 0; i < NUM_MATRICES; i++)
    {
        EPETE(perturb_unary_cluster_data(&perturb_mp, input_mvp->elements[ i ], PERTURBATION, disable_feature_vp));
        db_mat(perturb_mp);
    }


    dbp("---------------------------------------------------------------------------------"); 
    dbp("INPUT, range"); 

    EPETE(set_cluster_lib_options("cluster-perturb-data-by-range", "t"));

    for (i = 0; i < NUM_MATRICES; i++)
    {
        EPETE(get_random_matrix(&(input_mvp->elements[ i ]), NUM_ROWS, NUM_COLS));
        EPETE(ow_multiply_matrix_by_scalar(input_mvp->elements[ i ], (double)(i + 1)));
    }

    for (i = 0; i < NUM_MATRICES; i++)
    {
        db_mat(input_mvp->elements[ i ]);
    }

    dbp("---------------------------------------------------------------------------------"); 
    dbp("RESULT, range, composite"); 

    EPETE(perturb_composite_cluster_data(&perturb_mvp, input_mvp, PERTURBATION, disable_feature_vp));
    
    for (i = 0; i < NUM_MATRICES; i++)
    {
        db_mat(perturb_mvp->elements[ i ]);

        EPETE(subtract_matrices(&diff_mp, 
                                perturb_mvp->elements[ i ], 
                                input_mvp->elements[ i ]));
        db_mat(diff_mp);
        dbe(frobenius_matrix_norm(diff_mp));
    }

    dbp("---------------------------------------------------------------------------------"); 
    dbp("RESULT, value, unary"); 

    for (i = 0; i < NUM_MATRICES; i++)
    {
        EPETE(perturb_unary_cluster_data(&perturb_mp, input_mvp->elements[ i ], PERTURBATION, disable_feature_vp));
        db_mat(perturb_mp);

        EPETE(subtract_matrices(&diff_mp, 
                                perturb_mp, 
                                input_mvp->elements[ i ]));
        db_mat(diff_mp);
        dbe(frobenius_matrix_norm(diff_mp));
    }


    free_int_vector(disable_feature_input_vp);
    free_int_vector(disable_feature_vp);
    free_matrix(diff_mp);
    free_matrix(perturb_mp);
    free_matrix_vector(input_mvp);
    free_matrix_vector(perturb_mvp);

    kjb_cleanup(); /* Almost never needed, but doing it twice is OK. */

    return EXIT_SUCCESS; 
} 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

