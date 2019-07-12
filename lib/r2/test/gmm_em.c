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
#include "l/l_sys_term.h"
#include "l/l_sys_lib.h"
#include "sample/sample_incl.h"


/* -------------------------------------------------------------------------- 
 * Pass in the number of clusters, then the file name to which to write the
 * data. Otherwise, the program will use the default values of 3 clusters and
 * data_2.txt file name.
 * -------------------------------------------------------------------------- */

#define DEGREES_PER_RADIAN (180.0 / M_PI)
#define MAX_NUM_TESTS 10
#define DEFAULT_NUM_CLUSTERS 3

double X_DELTA = 0.1;

typedef struct Options_ {
    int DO_HELD_OUT;
    int DO_MISSING;
    int DO_INDEPENDENT;
    int SAVE_PLOT;
} Options;

static const Options test_independent = 
{
    0, /*  DO_HELD_OUT */
    0, /*  DO_MISSING */
    1, /*  DO_INDEPENDENT */
    1  /*  SAVE_PLOT  */
};

static const Options test_independent_w_held_out = 
{
    1, /*  DO_HELD_OUT */
    0, /*  DO_MISSING */
    1, /*  DO_INDEPENDENT */
    1  /*  SAVE_PLOT  */
};

static const Options test_independent_missing = 
{
    0, /*  DO_HELD_OUT */
    1, /*  DO_MISSING */
    1, /*  DO_INDEPENDENT */
    1  /*  SAVE_PLOT  */
};

static const Options invalid_test_1 =  /* missing and held out */
{
    1, /*  DO_HELD_OUT */
    1, /*  DO_MISSING */
    1, /*  DO_INDEPENDENT */
    1  /*  SAVE_PLOT  */
};

static const Options invalid_test_2 =  /* missing and full covariance */
{
    0, /*  DO_HELD_OUT */
    1, /*  DO_MISSING */
    0, /*  DO_INDEPENDENT */
    1  /*  SAVE_PLOT  */
};

static const Options test_full = 
{
    0, /*  DO_HELD_OUT */
    0, /*  DO_MISSING */
    0, /*  DO_INDEPENDENT */
    1  /*  SAVE_PLOT  */
};

static const Options test_full_w_held_out = 
{
    1, /*  DO_HELD_OUT */
    0, /*  DO_MISSING */
    0, /*  DO_INDEPENDENT */
    1  /*  SAVE_PLOT  */
};

const Options* preset_tests[] = {
    &test_independent,
    &test_independent_w_held_out,
    &test_independent_missing,
    &test_full,
    &test_full_w_held_out
};
const int NUM_PRESET_TESTS = sizeof(preset_tests) / sizeof(Options*);

const Options* default_options = &test_independent_w_held_out;

int run_test(Options options, int num_clusters, const Matrix* data_mp, const char* plot_file_name, int do_plotting);


int main(int argc, char* argv[])
{
    Matrix*     data_mp    = NULL;
    char        data_file_name[MAX_FILE_NAME_SIZE ]; 
    char        plot_file_name[MAX_FILE_NAME_SIZE ]; 
    const Options*    test_list[MAX_NUM_TESTS];
    int         num_test = 0;
    int         arg_position = 0;
    int         result;
    int         i;
    int         num_clusters = DEFAULT_NUM_CLUSTERS;
    int         do_plotting = TRUE;

    kjb_init();   /* Best to do this if using KJB library. */

    kjb_disable_paging();

    /* set defaults */
    kjb_strncpy(plot_file_name, "gmm_plot", MAX_FILE_NAME_SIZE);
    kjb_strncpy(data_file_name, "data_2.txt", MAX_FILE_NAME_SIZE);

    argv++; argc--;
    while(argc > 0)
    {
        if (     kjb_strcmp(*argv, "-h") == EQUAL_STRINGS 
             ||  kjb_strcmp(*argv, "--help") == EQUAL_STRINGS
           )
        {
            kjb_printf("Usage: \n");
            kjb_printf("gmm_em [num_clusters] [data_fname] [plot_fname] \n");
            kjb_printf("     (equivalent to --test 2 below) \n");
            kjb_printf("\n");
            kjb_printf("gmm_em --test N [num_clusters] [data_fname] [plot_fname]\n");
            kjb_printf("    --test N    Run preset test N=1-5\n");
            kjb_printf("         1. Independent GMM      \n");
            kjb_printf("         2. Independent GMM w/ held-out data     \n");
            kjb_printf("         3. Independent GMM w/ missing data     \n");
            kjb_printf("         4. Full -covariance GMM \n");
            kjb_printf("         5. Full -covariance GMM w/ held out data \n");
            kjb_printf("         (default is 2) \n");
            kjb_printf("\n");
            kjb_printf("gmm_em --test-all [num_clusters] [data_fname] \n");
            kjb_printf("         Run all tests 1-5 above \n");

            kjb_exit(EXIT_SUCCESS);
        }
        else if( kjb_strcmp(*argv, "--test-all") == EQUAL_STRINGS)
        {
            /* add add presets to the test suite */
            kjb_memcpy(test_list, preset_tests, sizeof(Options*)*NUM_PRESET_TESTS);
            num_test = NUM_PRESET_TESTS;
        }
        else if( kjb_strcmp(*argv, "--test") == EQUAL_STRINGS)
        {
            /* add a test preset to the test suite */
            int test_num;
            argc--;
            argv++;

            if(argc == 0)
            {
                set_error("missing test number");
                kjb_print_error();
                kjb_exit(EXIT_FAILURE);
            }

            result = ss1pi(*argv, &test_num);

            if(result == ERROR || test_num < 1 || test_num > NUM_PRESET_TESTS)
            {
                set_error("Invalid test number");
                kjb_print_error();
                kjb_exit(EXIT_FAILURE);
            }

            if(num_test >= MAX_NUM_TESTS)
            {
                set_error("Maximum test count exceeded.");
                kjb_print_error();
                kjb_exit(EXIT_FAILURE);
            }

            test_list[num_test++] = preset_tests[test_num-1];

        }
        else
        {
            /* position options */

            if(arg_position == 0)
            {
                EPETE(ss1pi(*argv, &num_clusters)); 
            }
            else if(arg_position == 1)
            {
                BUFF_CPY(data_file_name, *argv ); 
            }
            else if(arg_position == 2)
            {
                BUFF_CPY(plot_file_name, *argv ); 
            }
            else
            {
                set_error("Too many positional options");
                kjb_print_error();
                kjb_exit(EXIT_FAILURE);
            }

            arg_position++;
        }

        argc--; argv++;
    }

    if(num_test == 0)
    {
        if(is_interactive())
        {
            /* run default test */
            test_list[0] = default_options;
            num_test = 1;
        }
        else
        {
            /* run entire test suite */
            kjb_memcpy(test_list, preset_tests, sizeof(Options*)*NUM_PRESET_TESTS);
            num_test = NUM_PRESET_TESTS;
        }
    }

    verbose_pso(3, "Using data from file '%s' \n", data_file_name);
    verbose_pso(3, "Writing plot to file '%s' \n", plot_file_name);

    EPETE(read_matrix(&data_mp, data_file_name)); 

    /* kjb_seed_rand_with_tod(); */
    kjb_set_verbose_level(5);
    /* set_random_options("seed","?"); */
    /* EPETE(set_em_cluster_options("cluster-tie-cluster-var", "t")); */
    EPETE(set_em_cluster_options("cluster-var-offset", "0.0")); 
    EPETE(set_em_cluster_options("cluster-max-num-iterations", "200")); 

    if(num_test > 1) 
    {
        pso("Disabling plotting due to multiple tests.");
        do_plotting = FALSE;
    }
    else if(!is_interactive())
    {
        pso("Disabling plotting due to running non-interactively.");
        do_plotting = FALSE;
    }

    for(i = 0; i < num_test; ++i)
    {
        EPETE(run_test(*test_list[i], num_clusters, data_mp, plot_file_name, do_plotting));
    }

    free_matrix(data_mp); 

    pso("All tests ran without crashing (results not verified).\n");

    kjb_cleanup(); /* Almost never needed, but doing it twice is OK. */
    return EXIT_SUCCESS;
}

int run_test(Options options, int num_clusters, const Matrix* data_mp, const char* plot_file_name, int do_plotting)
{
    int         num_iterations;
    int         cluster;
    long        cpu_time;
    Matrix*     U_mp       = NULL;
    Vector*     y_vp       = NULL;
    Vector*     x_vp       = NULL;
    Matrix*     cov_mp     = NULL;
    Matrix*     P_mp       = NULL;
    Vector* a_vp = NULL;
    Matrix* mean_mp = NULL; 
    Matrix* var_mp = NULL; 
    Int_vector* index_vp   = NULL;
    Int_vector* held_out_vp = NULL;


    int         num_points; 
    int i,j;

    static int did_plot = FALSE;

    /* only allow plotting for one test per run to avoid 1M windows */
    if(do_plotting)
    {
        if(did_plot)
        {
            set_error("Error: plotting only allowed for one test per execution.");
            return ERROR;
        }
        else
        {
            did_plot = TRUE;
        }
    }

    num_points = data_mp->num_rows;
    dbi(num_points); 

    if(do_plotting)
    {
        int plot_id;
        EPETE(plot_id = plot_open());
        EPETE(plot_matrix_row_points(plot_id, data_mp, NULL)); 
    }

    if(options.DO_MISSING)
    {
        Matrix* data_w_missing_mp = NULL;
        copy_matrix(&data_w_missing_mp, data_mp);
        pso("Running diagonal-covariance GMM w/ missing data \n");
        enable_respect_missing_values();
        for (i = 0; i < data_w_missing_mp->num_rows; i++)
        for (j = 0; j < data_w_missing_mp->num_cols; j++)
        {
            if (i % 2 == 0)
            {
                if ((i/2) % 2 == 0)
                    data_w_missing_mp->elements[i][0] = DBL_MISSING;
                else
                    data_w_missing_mp->elements[i][1] = DBL_MISSING;
            }
        }
        init_cpu_time();
        EPETE(get_independent_GMM(
                    num_clusters,
                    data_w_missing_mp,
                    &a_vp,
                    &mean_mp,
                    &var_mp,
                    &P_mp));
        cpu_time = get_cpu_time();
        display_cpu_time();

        free_matrix(data_w_missing_mp);
    }
    else if(options.DO_INDEPENDENT)
    {
        if(options.DO_HELD_OUT)
        {
            int num_held_out = 0;

            pso("Running diagonal-covariance GMM w/ held out \n");
            EPETE(get_zero_int_vector(&held_out_vp, num_points)); 

            for (i = 0; i < num_points; i++)
            {
                if (i % 10 == 0)
                {
                    held_out_vp->elements[ i ] = TRUE;
                    num_held_out++; 
                }
            }
            dbi(num_held_out); 

            init_cpu_time();
            EPETE(get_independent_GMM_3(num_clusters, data_mp, held_out_vp, 
                                        (const Vector*)NULL, (const Matrix*)NULL, (const Matrix*)NULL,
                                        (Vector**)&a_vp, &mean_mp, &var_mp, &P_mp,
                                        (double*)NULL, (double*)NULL, &num_iterations));
            cpu_time = get_cpu_time();
            display_cpu_time();
            verbose_pso(3, "Running time of serial independent GMM is: %10ld\n", cpu_time); 
            verbose_pso(3, "Number of iterations: %2d\n", num_iterations);
        }
        else
        {
            pso("Running diagonal-covariance GMM \n");

            init_cpu_time();
            EPETE(get_independent_GMM(num_clusters, data_mp, &a_vp, &mean_mp, &var_mp, &P_mp));
            cpu_time = get_cpu_time();
            display_cpu_time();
        }
    } 
    else 
    {
        Matrix_vector* var_mvp = NULL;

        if(options.DO_HELD_OUT)
        {
            pso("Running full-covariance GMM w/ held out\n");
            pso("NOTE: classification results will not reflect off-diagonal covariance entries.\n");
            pso("(this is an idiosyncrasy of get_full_GMM_3)\n");
            EPETE(get_zero_int_vector(&held_out_vp, num_points)); 
            for (i = 0; i < num_points; i++)
            {
                if (i % 100 == 0)
                {
                    held_out_vp->elements[ i ] = TRUE;
                }
            }
            EPETE(get_full_GMM_3(num_clusters, data_mp, (const Matrix*)NULL, &a_vp, &mean_mp,
                        &var_mvp, &P_mp, (double*)NULL, held_out_vp, (const Vector*)NULL, 
                        (const Matrix*)NULL, (const Matrix*)NULL, (double*)NULL, &num_iterations));
        } else {
            pso("Running full-covariance GMM \n");
            EPETE(get_full_GMM(num_clusters, data_mp, NULL, &a_vp, &mean_mp, &var_mvp, &P_mp));
        }

        for(i = 0; i < num_clusters; ++i)
        {
            kjb_printf(" Covariance of cluster %d\n", i);
            db_mat(var_mvp->elements[i]); 
        }

        free_matrix_vector(var_mvp);
    }

    db_rv(a_vp);
    db_mat(mean_mp);
    db_mat(var_mp); 

    EPETE(get_max_matrix_row_elements_2(NULL, &index_vp, P_mp)); 


    if(do_plotting)
    {
        Matrix*     out_mp     = NULL;
        int plot_id_2;

        EPETE(get_target_matrix(&out_mp, num_points, 2)); 
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

        if(options.SAVE_PLOT)
        {
            EPETE( save_plot(plot_id_2, plot_file_name) );
        }

        if (is_interactive())
        {
            prompt_to_continue(); 
        }

        free_matrix(out_mp); 
    }


    free_matrix(U_mp); 
    free_vector(y_vp); 
    free_vector(x_vp); 
    free_matrix(cov_mp); 
    free_matrix(P_mp); 
    free_vector(a_vp); 
    free_matrix(mean_mp); 
    free_matrix(var_mp); 
    free_int_vector(index_vp); 
    free_int_vector(held_out_vp); 

    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

