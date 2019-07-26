
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
#include "n/n_incl.h"
#include "p/p_incl.h"
#include "r/r_incl.h"
#include "sample/sample_incl.h"

/* -------------------------------------------------------------------------- 
 * Pass in the number of clusters, then the file name to which to write the
 * data. Otherwise, the program will use the default values of 3 clusters and
 * data_2.txt file name.
 * -------------------------------------------------------------------------- */

#define DEGREES_PER_RADIAN (180.0 / M_PI)
#define NUM_POINTS  (2000)

#define GAUSS_WEIGHT  (2.0) 
#define UNIFORM_WEIGHT (0.0) 
/*
#define GAUSS_WEIGHT  (2.0) 
#define UNIFORM_WEIGHT (5.0) 
*/
/*
#define GAUSS_WEIGHT  (0.0) 
#define UNIFORM_WEIGHT (0.0) 
*/


#define NUM_CLUSTERS  (3)

/*
#define SET_BY_HAND
*/
/*
#define DO_INDEPENDENT
#define DO_BALLS
*/

int main(int argc, char* argv[])
{
    Matrix*     U_mp       = NULL;
    Matrix*     data_mp    = NULL;
    Vector*     g_vp       = NULL;
    Vector*     u_vp       = NULL;
    Vector*     y_vp       = NULL;
    Vector*     x_vp       = NULL;
    Matrix*     cov_mp     = NULL;
    Vector*     mean_vp    = NULL;
    int         i;
    int         plot_id;
    FILE*       data_fp    = NULL;
    Vector*     angle_vp   = NULL;
    Vector*     mean_x_vp  = NULL;
    Vector*     mean_y_vp  = NULL;
    Vector*     stdev_x_vp = NULL;
    Vector*     stdev_y_vp = NULL;
    int         cluster;
    Matrix*     out_mp     = NULL;
    Int_vector* index_vp   = NULL;
    Matrix*     P_mp       = NULL;
    Int_vector* counts_vp = NULL; 
    Matrix_vector* data_mvp = NULL; 
    int num_clusters = NUM_CLUSTERS;
    char         data_file_name[MAX_FILE_NAME_SIZE ]; 

    kjb_init();   /* Best to do this if using KJB library. */

    if (! is_interactive()) 
    {
        p_stderr("This program is only for generating data.\n");
        p_stderr("We currently do not use it in batch mode.\n"); 
        return EXIT_FAILURE;
    }
    
    /*
    kjb_set_verbose_level(200); 
    */
    kjb_disable_paging(); 

    if (argc < 2)
    {
        /* Default data file name */
        BUFF_CPY(data_file_name, "data_2.txt");
    }
    else if (argc == 2)
    {
        EPETE(ss1pi(argv[ 1 ], &num_clusters)); 
    }
    else if (argc == 3)
    {
        EPETE(ss1pi(argv[ 1 ], &num_clusters)); 
        BUFF_CPY(data_file_name, argv[ 2 ] ); 
        
        verbose_pso(3, "# of clusters is %d \n", num_clusters);
        verbose_pso(3, "Saving data to file '%s' \n", data_file_name);
    }

    kjb_seed_rand_with_tod(); 

    EPETE(set_em_cluster_options("cluster-tie-cluster-var", "t")); 
    EPETE(set_em_cluster_options("cluster-var-offset", ".1")); 
    EPETE(set_em_cluster_options("cluster-max-num-iterations", "100")); 

#ifdef SET_BY_HAND
    num_clusters = 3;

    EPETE(get_target_vector(&mean_x_vp, num_clusters)); 
    EPETE(get_target_vector(&mean_y_vp, num_clusters)); 
    mean_x_vp->elements[ 0 ] = -2.5;
    mean_y_vp->elements[ 0 ] =  0.0;
    mean_x_vp->elements[ 1 ] =  2.5;
    mean_y_vp->elements[ 1 ] =  0.0;
    mean_x_vp->elements[ 2 ] =  0.0;
    mean_y_vp->elements[ 2 ] =  5.0;

    EPETE(get_target_vector(&stdev_x_vp, num_clusters)); 
    EPETE(get_target_vector(&stdev_y_vp, num_clusters)); 
    stdev_x_vp->elements[ 0 ] = 1.0;
    stdev_y_vp->elements[ 0 ] = 1.0;
    stdev_x_vp->elements[ 1 ] =  1.0;
    stdev_y_vp->elements[ 1 ] =  1.0;
    stdev_x_vp->elements[ 2 ] =  2.0;
    stdev_y_vp->elements[ 2 ] =  2.0;
#else 
    EPETE(get_random_vector(&mean_x_vp, num_clusters)); 
    EPETE(ow_subtract_scalar_from_vector(mean_x_vp, 0.5)); 
    EPETE(ow_multiply_vector_by_scalar(mean_x_vp, 10.0)); 

    EPETE(get_random_vector(&mean_y_vp, num_clusters)); 
    EPETE(ow_subtract_scalar_from_vector(mean_y_vp, 0.5)); 
    EPETE(ow_multiply_vector_by_scalar(mean_y_vp, 10.0)); 

    EPETE(get_random_vector(&stdev_x_vp, num_clusters)); 
    EPETE(ow_multiply_vector_by_scalar(stdev_x_vp, 2.0)); 
#ifdef DO_BALLS
    EPETE(copy_vector(&stdev_y_vp, stdev_x_vp)); 
#else
    EPETE(get_random_vector(&stdev_y_vp, num_clusters)); 
    EPETE(ow_add_scalar_to_vector(stdev_y_vp, 0.1)); 
#endif 

#endif 

#ifdef DO_INDEPENDENT
    EPETE(get_zero_vector(&angle_vp, num_clusters)); 
#else
    EPETE(get_random_vector(&angle_vp, num_clusters)); 
    EPETE(ow_multiply_vector_by_scalar(angle_vp, M_PI)); 
#endif 

    EPETE(get_zero_int_vector(&counts_vp, num_clusters));
    EPETE(get_target_matrix_vector(&data_mvp, num_clusters));

    for (cluster = 0; cluster < num_clusters; cluster++)
    {
        EPETE(get_target_matrix(&(data_mvp->elements[ cluster ]), NUM_POINTS, 2)); 
    }

    EPETE(plot_id = plot_open());

    dbp("Making data up"); 

    EPETE(get_target_matrix(&data_mp, NUM_POINTS, 2));

    for (i = 0; i < NUM_POINTS; i++)
    {
        int cluster = num_clusters * kjb_rand(); 
        double angle = angle_vp->elements[ cluster ]; 
        double mean_x = mean_x_vp->elements[ cluster ]; 
        double stdev_x = stdev_x_vp->elements[ cluster ]; 
        double mean_y = mean_y_vp->elements[ cluster ]; 
        double stdev_y = stdev_y_vp->elements[ cluster ]; 

        EPETE(get_target_matrix(&U_mp, 2, 2)); 

        U_mp->elements[ 0 ][ 0 ] = cos(angle); 
        U_mp->elements[ 0 ][ 1 ] = -sin(angle); 
        U_mp->elements[ 1 ][ 0 ] = sin(angle); 
        U_mp->elements[ 1 ][ 1 ] = cos(angle); 

        EPETE(get_gauss_random_vector(&g_vp, 2));
        EPETE(ow_multiply_vector_by_scalar(g_vp, GAUSS_WEIGHT)); 
        
        EPETE(get_random_vector(&u_vp, 2));
        EPETE(ow_multiply_vector_by_scalar(u_vp, UNIFORM_WEIGHT)); 

        EPETE(add_vectors(&y_vp, g_vp, u_vp));

        y_vp->elements[ 0 ] *= stdev_x;
        y_vp->elements[ 1 ] *= stdev_y;

        EPETE(multiply_matrix_and_vector(&x_vp, U_mp, y_vp));

        x_vp->elements[ 0 ] += mean_x;
        x_vp->elements[ 1 ] += mean_y;

        EPETE(put_matrix_row(data_mp, x_vp, i)); 

        EPETE(put_matrix_row(data_mvp->elements[ cluster ], x_vp, counts_vp->elements[ cluster ])); 
        (counts_vp->elements[ cluster ])++;
    }

    EPETE(write_matrix(data_mp, data_file_name)); 

    for (cluster = 0; cluster < num_clusters; cluster++)
    {
        data_mvp->elements[ cluster ]->num_rows = counts_vp->elements[ cluster ];

        EPETE(plot_matrix_row_points(plot_id, data_mvp->elements[ cluster ], NULL)); 
    }

    free_vector(x_vp); 
    free_vector(y_vp); 
    free_vector(g_vp); 
    free_vector(u_vp); 
    free_matrix(U_mp); 
    free_matrix(data_mp); 
    free_matrix(out_mp); 
    free_matrix(cov_mp); 
    free_matrix(P_mp); 
    free_int_vector(index_vp); 
    free_int_vector(counts_vp); 
    free_vector(mean_vp); 
    free_vector(mean_x_vp); 
    free_vector(mean_y_vp); 
    free_vector(stdev_x_vp); 
    free_vector(stdev_y_vp); 
    free_vector(angle_vp); 
    free_matrix_vector(data_mvp); 

    kjb_fclose(data_fp); 

    prompt_to_continue(); 

    kjb_cleanup(); /* Almost never needed, but doing it twice is OK. */

    return EXIT_SUCCESS;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

