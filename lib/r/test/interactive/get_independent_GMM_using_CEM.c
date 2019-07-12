
/* $Id: get_independent_GMM_using_CEM.c 21491 2017-07-20 13:19:02Z kobus $ */


/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003, by members of University of Arizona Computer Vision     |
|  group (the authors) including                                               ||
|        Kobus Barnard.                                                        |
|        Prasad Gabbur.                                                                      |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */

/*
//  Most programs need at least the "m" library. 
*/

#include "m/m_incl.h" 
#include "r/r_incl.h" 
#include "sample/sample_gauss.h" 


/* -------------------------------------------------------------------------- */

#define NUM_CLUSTERS  5
#define NUM_POINTS    1000
#define NUM_FEATURES  8

/* -------------------------------------------------------------------------- */

int main(int argc, char** argv)
{
    Matrix* P_mp         = NULL; 
    Vector* P_vp         = NULL; 
    Matrix* data_mp      = NULL; 
    Matrix* mean_mp      = NULL;
    Matrix* var_mp       = NULL; 
    Vector* init_a_vp    = NULL;
    Matrix* init_mean_mp = NULL;
    Matrix* init_var_mp  = NULL;
    Vector* est_a_vp     = NULL;
    Matrix* est_var_mp   = NULL; 
    Matrix* est_mean_mp  = NULL;
    Vector* mean_vp      = NULL;
    Vector* var_vp       = NULL; 
    int cluster; 
    double p; 
    int i,j;
    int init_num_clusters;
    int est_num_clusters;
    double beta, gamma;
    IMPORT int kjb_debug_level;

    kjb_init();   /* Best to do this if using KJB library. */

    if (! is_interactive()) 
    {
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }

    kjb_set_verbose_level(5);
    kjb_debug_level = 10;

    EPETE(get_target_matrix(&data_mp, NUM_POINTS, NUM_FEATURES));

    EPETE(get_target_matrix(&mean_mp, NUM_CLUSTERS, NUM_FEATURES));
    EPETE(get_target_matrix(&var_mp, NUM_CLUSTERS, NUM_FEATURES));

    for (cluster = 0; cluster < NUM_CLUSTERS; cluster++)
    {
        ERE(get_random_vector(&mean_vp, NUM_FEATURES));
        ERE(put_matrix_row(mean_mp, mean_vp, cluster)); 
        ERE(get_random_vector(&var_vp, NUM_FEATURES));
        ERE(put_matrix_row(var_mp, var_vp, cluster)); 
    }

    kjb_seed_rand(0,0);
    kjb_seed_rand_2(0);

    for (i = 0; i < NUM_POINTS; i++)
    {
        cluster = NUM_CLUSTERS * kjb_rand(); 

        for (j = 0; j < NUM_FEATURES; j++)
        {
            p = gauss_rand(); 
            p *= sqrt(var_mp->elements[ cluster ][ j ]);
            p += mean_mp->elements[ cluster ][ j ]; 

            data_mp->elements[ i ][ j ] = p; 
        }
    }

    /* Explicit initialization. */
    init_num_clusters = 1;

    EPETE(get_random_matrix(&init_mean_mp, init_num_clusters, NUM_FEATURES));
    EPETE(get_random_matrix(&init_var_mp, init_num_clusters, NUM_FEATURES));
    EPETE(get_random_vector(&init_a_vp, init_num_clusters));
    EPETE(ow_scale_vector_by_sum(init_a_vp));
    
    beta  = 0.1;
    gamma = 1000;

    EPETE(get_independent_GMM_using_CEM(init_num_clusters, 
                                        data_mp, 
                                        init_a_vp,
                                        init_mean_mp,
                                        init_var_mp,
                                        &est_a_vp,
                                        &est_mean_mp,
                                        &est_var_mp, 
                                        &P_mp,
                                        beta,
                                        gamma)); 

    est_num_clusters = est_mean_mp->num_rows;

    dbp("\n---------------------------------------------\n");
    
    dbi(est_num_clusters);

    dbp("\n---------------------------------------------\n");

    db_mat(mean_mp);

    dbp("\n---------------------------------------------\n");

    db_mat(est_mean_mp);

    dbp("\n---------------------------------------------\n");

    db_rv(est_a_vp); 

    EPETE(get_matrix_row(&P_vp, P_mp, 0));
    db_rv(P_vp);

    /*
    db_mat(mean_mp);
    db_mat(var_mp);
    db_rv(a_vp); 
    */
    
    /* Random initialization. */
    init_num_clusters = 1;
    
    beta  = 0.1;
    gamma = 1000;

    EPETE(get_independent_GMM_using_CEM(init_num_clusters, 
                                        data_mp, 
                                        (Vector*) NULL,
                                        (Matrix*) NULL,
                                        (Matrix*) NULL,
                                        &est_a_vp,
                                        &est_mean_mp,
                                        &est_var_mp, 
                                        &P_mp,
                                        beta,
                                        gamma)); 

    est_num_clusters = est_mean_mp->num_rows;

    dbp("\n---------------------------------------------\n");
    
    dbi(est_num_clusters);

    dbp("\n---------------------------------------------\n");

    db_mat(mean_mp);

    dbp("\n---------------------------------------------\n");

    db_mat(est_mean_mp);

    dbp("\n---------------------------------------------\n");

    db_rv(est_a_vp); 

    EPETE(get_matrix_row(&P_vp, P_mp, 0));
    db_rv(P_vp);

    free_matrix(data_mp);
    free_matrix(mean_mp);
    free_matrix(var_mp);
    free_vector(mean_vp); 
    free_vector(var_vp); 
    free_vector(init_a_vp);
    free_matrix(init_mean_mp);
    free_matrix(init_var_mp);
    free_vector(est_a_vp); 
    free_matrix(est_mean_mp); 
    free_matrix(est_var_mp); 
    free_matrix(P_mp); 
    free_vector(P_vp); 

    kjb_cleanup(); /* Almost never needed, but doing it twice is OK. */

    return EXIT_SUCCESS; 
} 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
