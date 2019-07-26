
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
#include "r2/r2_incl.h"
#include "l/l_sys_time.h"
#include "sample/sample_incl.h"

/* -------------------------------------------------------------------------- 
 * Pass in the number of clusters, then the file name to which to write the
 * data. Otherwise, the program will use the default values of 3 clusters and
 * data_2.txt file name.
 * -------------------------------------------------------------------------- */

#define DEGREES_PER_RADIAN (180.0 / M_PI)

#define X_DELTA (0.1)

#define NUM_CLUSTERS  (3)

#define DO_HELD_OUT 

/*#define DO_INDEPENDENT */

#define SAVE_PLOT

int main(int argc, char* argv[])
{
    Matrix*     U_mp       = NULL;
    Matrix*     data_mp    = NULL;
    Vector*     y_vp       = NULL;
    Vector*     x_vp       = NULL;
    Matrix*     cov_mp     = NULL;
    Vector*     mean_vp    = NULL;
    int         num_points; 
    int         i;
    int         plot_id;
    int         plot_id_2;
    int         num_iterations;
    FILE*       data_fp    = NULL;
    int         cluster;
    Matrix*     out_mp     = NULL;
    Int_vector* index_vp   = NULL;
    Matrix*     P_mp       = NULL;
    Vector* a_vp = NULL;
    Matrix* mean_mp = NULL; 
    Matrix* var_mp = NULL; 
    int num_clusters = NUM_CLUSTERS; 
    Int_vector* held_out_vp = NULL;
    char         data_file_name[MAX_FILE_NAME_SIZE ]; 
    char         plot_file_name[MAX_FILE_NAME_SIZE ]; 
    long        cpu_time;

    kjb_init();   /* Best to do this if using KJB library. */

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
        
        verbose_pso(3, "Using data from file '%s' \n", data_file_name);
    }
    else if (argc == 4)
    {
        EPETE(ss1pi(argv[ 1 ], &num_clusters)); 
        BUFF_CPY(data_file_name, argv[ 2 ] ); 
        BUFF_CPY(plot_file_name, argv[ 3 ] ); 
        
        verbose_pso(3, "Using data from file '%s' \n", data_file_name);
        verbose_pso(3, "Writing plot to file '%s' \n", plot_file_name);
    }

    /* kjb_seed_rand_with_tod(); */
    kjb_set_verbose_level(5);

    /*
    EPETE(set_em_cluster_options("cluster-tie-cluster-var", "t")); 
    */
    EPETE(set_em_cluster_options("cluster-var-offset", "0.0")); 
    EPETE(set_em_cluster_options("cluster-max-num-iterations", "200")); 

    EPETE(read_matrix(&data_mp, data_file_name)); 
    num_points = data_mp->num_rows;

    if (is_interactive())
    {
        EPETE(plot_id = plot_open());
        EPETE(plot_matrix_row_points(plot_id, data_mp, NULL)); 
        EPETE(plot_close(plot_id));
    }

#ifdef DO_INDEPENDENT
#ifdef DO_HELD_OUT
    EPETE(get_zero_int_vector(&held_out_vp, num_points)); 

    for (i = 0; i < num_points; i++)
    {
        if (i % 10 == 0)
        {
            held_out_vp->elements[ i ] = TRUE;
        }
    }

    verbose_pso(3, "Using serial independent_GMM EM ");
    init_cpu_time();
    EPETE(get_independent_GMM_3(num_clusters, data_mp, held_out_vp, 
                                (const Vector*)NULL, (const Matrix*)NULL, (const Matrix*)NULL,
                                (Vector**)NULL, &mean_mp, &var_mp, &P_mp,
                                (double*)NULL, (double*)NULL, &num_iterations));
    cpu_time = get_cpu_time();
    display_cpu_time();
    verbose_pso(3, "Running time of serial independent GMM is: %10ld\n", cpu_time); 
    verbose_pso(3, "Number of iterations: %2d\n", num_iterations);
#else
    EPETE(get_independent_GMM(num_clusters, data_mp, NULL, &mean_mp, &var_mp, &P_mp));
    db_mat(mean_mp);
    db_mat(var_mp); 
#endif /*DO HELD OUT */ 
#else  /* NOT DO INDEPENDENT */
#ifdef DO_HELD_OUT
    EPETE(get_zero_int_vector(&held_out_vp, num_points)); 
    for (i = 0; i < num_points; i++)
    {
        if (i % 10 == 0)
        {
            held_out_vp->elements[ i ] = TRUE;
        }
    }
    EPETE(get_full_GMM_3(num_clusters, data_mp, (const Matrix*)NULL, (Vector**)NULL, &mean_mp,
                (Matrix_vector**)NULL, &P_mp, (double*)NULL, held_out_vp, (const Vector*)NULL, 
                (const Matrix*)NULL, (const Matrix*)NULL, (double*)NULL, &num_iterations));
#else
    EPETE(get_full_GMM(num_clusters, data_mp, NULL, NULL, NULL, NULL, &P_mp));
#endif
#endif  

    EPETE(get_max_matrix_row_elements_2(NULL, &index_vp, P_mp)); 

    EPETE(get_target_matrix(&out_mp, num_points, 2)); 

    if (is_interactive())
    {
        EPETE(plot_id_2 = plot_open());

        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            int count = 0; 

            for (i = 0; i < num_points; i++)
            {
                if (index_vp->elements[ i ] == cluster)
                {
                    out_mp->elements[ count ][ 0 ] = data_mp->elements[ i ][ 0 ]; 
                    out_mp->elements[ count ][ 1 ] = data_mp->elements[ i ][ 1 ]; 
                    count++; 
                }
            }

            out_mp->num_rows = count; 

            EPETE(plot_matrix_row_points(plot_id_2, out_mp, NULL));
        }

        EPETE(plot_set_title(plot_id_2, "Clusters found", 0, 0)); 

#ifdef SAVE_PLOT
        EPETE( save_plot(plot_id_2, plot_file_name) );
#endif
        EPETE(plot_close(plot_id_2));
    }

    set_random_options("seed","?"); 

    free_matrix(var_mp); 
    free_matrix(mean_mp); 
    free_vector(a_vp); 
    free_vector(x_vp); 
    free_vector(y_vp); 
    free_matrix(U_mp); 
    free_matrix(data_mp); 
    free_int_vector(held_out_vp); 
    free_matrix(out_mp); 
    free_matrix(cov_mp); 
    free_matrix(P_mp); 
    free_int_vector(index_vp); 
    free_vector(mean_vp); 

    kjb_fclose(data_fp); 

    /*prompt_to_continue();  */

    kjb_cleanup(); /* Almost never needed, but doing it twice is OK. */

    return EXIT_SUCCESS;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

