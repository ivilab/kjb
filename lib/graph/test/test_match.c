
/* $Id: test_match.c 21491 2017-07-20 13:19:02Z kobus $ */


/* =========================================================================== *
|                                                                              |
|  Copyright (c) 1994-1998, by Kobus Barnard (author).                         |
|                                                                              |
|  For use outside the SFU vision lab please contact the author(s).            |
|                                                                              |
* =========================================================================== */

#include "l/l_incl.h" 
#include "m/m_incl.h" 
#include "graph/hungarian.h"
#include "graph/jv_lap.h"

#define ROW_FACTOR  500.0
#define COL_FACTOR  500.0
#define BASE_NUM_TRIES  100

/* YOU (PROBABLY) CANNOT HANDLE THE #define TEST_DBL_HUNGARIAN !! */
   
/* -------------------------------------------------------------------------- */

static int greedy_match    (const Matrix* mp, double* cost_ptr);
static int int_greedy_match(const Int_matrix* mp, int* cost_ptr);
  
/* -------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    Matrix* mp = NULL;
    Matrix* scaled_mp = NULL;
    Int_matrix* int_cost_mp = NULL; 
    Int_vector* hungarian_row_vp = NULL; 
    Int_vector* dbl_hungarian_row_vp = NULL; 
    Int_vector* jv_lap_row_vp = NULL; 
    Int_vector* int_hungarian_row_vp = NULL; 
    Int_vector* int_jv_lap_row_vp = NULL; 
    int result = EXIT_SUCCESS;
    double hungarian_cost; 
    double dbl_hungarian_cost; 
    double jv_lap_cost; 
    double greedy_cost; 
    double int_hungarian_cost; 
    double int_greedy_cost; 
    double int_jv_lap_cost; 
    int    int_int_hungarian_cost;
    int int_int_jv_lap_cost; 
    int int_int_greedy_cost; 
    double dbl_hungarian_cost_check = 0.0; 
    int i, m, n, count; 
    long dbl_hungarian_cpu = 0;
    long hungarian_cpu = 0;
    long greedy_cpu = 0;
    long jv_lap_cpu = 0;
    long int_hungarian_cpu = 0;
    long int_greedy_cpu = 0;
    long int_jv_lap_cpu = 0;
    double max_elem; 
    double factor; 
    double tol, greedy_tol; 
    double scale_up; 
    long overhead_cpu = 0; 
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;


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

    /* set_heap_options("heap-checking", "off"); */
    set_debug_options("debug", "3");
    set_term_io_options("page", "off"); 

    for (count = 0; count < num_tries; count++)
    {
        int temp; 

        /*
        dbi(count);
        */

        m = kjb_rint(ROW_FACTOR * kjb_rand());
        m++;

        temp = kjb_rint(m + COL_FACTOR * (kjb_rand() - 0.5));
        n = MAX_OF(0, temp);
        n++;

        /*
        n = m;
        */

        /* pso("%d by %d\n", m, n); */

        EPETE(get_random_matrix(&mp, m, n));

        /*
        EPETE(ow_multiply_matrix_by_scalar(mp, 100.0));
        */

        init_cpu_time();

        max_elem = max_matrix_element(mp); 

        scale_up = ((double)(INT_MAX / 4)) / MAX_OF(n, m); 
        factor = scale_up / max_elem;
        
        tol = 2.0 * MAX_OF(n, m) / factor; 

        /*
         * FIXME
         *
         * It is really unclear what bounds on accuracy we should expect in the
         * case of greedy. The above is ocasionally exceeded. While it seems
         * the right level for tolerance of optimal methods, the case of greedy
         * is not clear. We use a relatively big fudge factor for now. 
        */

        greedy_tol = tol * 100.0;


        /*
        dbe(max_elem); 
        dbe(scale_up); 
        dbe(factor); 
        dbe(tol); 
        */

        EPETE(multiply_matrix_by_scalar(&scaled_mp, mp, factor));
        EPETE(copy_matrix_to_int_matrix(&int_cost_mp, scaled_mp)); 

        overhead_cpu += get_cpu_time(); 

        init_cpu_time(); 
        EPETE(greedy_match(mp, &greedy_cost)); 

        greedy_cpu += get_cpu_time(); 

        init_cpu_time(); 
        EPETE(int_greedy_match(int_cost_mp, &int_int_greedy_cost)); 
        int_greedy_cpu += get_cpu_time(); 
        int_greedy_cost = ((double)int_int_greedy_cost) / factor;

        /*
        dbe(ABS_OF(int_greedy_cost - greedy_cost) / tol);
        */

        if (ABS_OF(int_greedy_cost - greedy_cost) > greedy_tol)
        {
            dbp("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"); 
            dbe(int_greedy_cost - greedy_cost);
            dbe(greedy_tol); 
            dbe(factor); 
            dbe(scale_up); 
            dbe(max_elem); 
            dbi(count); 
            dbi(m);
            dbi(n); 
            dbe((int_greedy_cost - greedy_cost) / int_greedy_cost);
            dbe(int_greedy_cost);
            dbe(greedy_cost); 
            dbp("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"); 

            result = EXIT_BUG;
        }

        init_cpu_time(); 

        if (hungarian(mp, &hungarian_row_vp, &hungarian_cost) == ERROR)
        {
            insert_error("Hungarian reported error"); 
            kjb_print_error();
            result = EXIT_BUG;
            continue;
        }

        hungarian_cpu += get_cpu_time(); 
        /* dbe(hungarian_cost - dbl_hungarian_cost); */


#ifdef TEST_DBL_HUNGARIAN
        init_cpu_time(); 

        if (dbl_hungarian(mp, &dbl_hungarian_row_vp, &dbl_hungarian_cost) == ERROR)
        {
            insert_error("DBL Hungarian reported error"); 
            kjb_print_error();
            continue;
        }

        dbl_hungarian_cpu += get_cpu_time(); 
        dbl_hungarian_cost_check = 0.0;

        for (i = 0; i < m; i++)
        {
            if (dbl_hungarian_row_vp->elements[ i ] != -1)
            {
                dbl_hungarian_cost_check += mp->elements[ i ][ dbl_hungarian_row_vp->elements[ i ] ];
            }
        }
            
        if (ABS_OF((dbl_hungarian_cost_check - dbl_hungarian_cost) / (ABS_OF(dbl_hungarian_cost_check) + ABS_OF(dbl_hungarian_cost))) > 10.0 * DBL_EPSILON)
        {
            dbp("++++++++++++++++++++++++"); 
            p_stderr("dbl_hungarian_cost_check != dbl_hungarian_cost (%.15e != %.15e).\n", dbl_hungarian_cost_check, dbl_hungarian_cost);
            dbp("++++++++++++++++++++++++"); 

            result = EXIT_BUG;
        }


        /*
        // Use absolute to compare int to float.
        */
        if (greedy_cost < dbl_hungarian_cost - tol)
        {
            dbe(greedy_cost);
            dbe(dbl_hungarian_cost); 
            dbe(greedy_cost - dbl_hungarian_cost); 

            result = EXIT_BUG;
        }

        /* --------------------------------------------- */

        if (ABS_OF(hungarian_cost - dbl_hungarian_cost) > tol)
        {
            dbp("===============================");
            p_stderr("jv_lap() and hungarian() have different costs (%d x %d).\n", m, n); 
            p_stderr("ABS_OF(jv_lap_cost - hungarian_cost) / tol:  %.15e\n", 
                     ABS_OF(jv_lap_cost - hungarian_cost) / tol); 
            dbp("===============================");

            result = EXIT_BUG;
        }

        for (i = 0; i < m; i++)
        {
            if (dbl_hungarian_row_vp->elements[ i ] != hungarian_row_vp->elements[ i ])
            {
                dbp("****************************");
                p_stderr("dbl_hungarian() and hungarian() have different assignments (%d x %d).\n", m, n); 
                p_stderr("ABS_OF(dbl_hungarian_cost - hungarian_cost) / tol:  %.15e\n",  
                         ABS_OF(dbl_hungarian_cost - hungarian_cost) / tol); 
                dbp("****************************");

                /*
                 * Not sufficient cause for failure.
                */

                break;  /* We only want one message per array. */
            }
        }
#endif

        /* --------------------------------------------- */

        init_cpu_time(); 

        if (jv_lap(mp, &jv_lap_row_vp, &jv_lap_cost) == ERROR)
        {
            insert_error("JV LAP reported error"); 
            kjb_print_error();
            result = EXIT_BUG;
            continue;
        }

        jv_lap_cpu += get_cpu_time(); 

        /* dbe(jv_lap_cost - hungarian_cost); */

        /*
        // Now absolute.
        //
        if (ABS_OF((jv_lap_cost - hungarian_cost) / hungarian_cost) > tol)
        */
        if (ABS_OF(jv_lap_cost - hungarian_cost) > tol)
        {
            dbp("===============================");
            p_stderr("jv_lap() and hungarian() have different costs (%d x %d).\n", m, n); 
            p_stderr("ABS_OF(jv_lap_cost - hungarian_cost) / tol:  %.15e\n", 
                     ABS_OF(jv_lap_cost - hungarian_cost) / tol); 
            dbp("===============================");

            result = EXIT_BUG;
        }

        for (i = 0; i < m; i++)
        {
            if (hungarian_row_vp->elements[ i ] != jv_lap_row_vp->elements[ i ])
            {
                dbp("****************************");
                p_stderr("jv_lap() and hungarian() have different assignments (%d x %d).\n", m, n); 
                p_stderr("ABS_OF(jv_lap_cost - hungarian_cost) / tol:  %.15e\n",  
                         ABS_OF(jv_lap_cost - hungarian_cost) / tol); 
                dbp("****************************");

                /*
                 * Not sufficient cause for failure.
                */

                break;  /* We only want one message per array. */
            }
        }

        /* --------------------------------------------- */

        init_cpu_time(); 

        if (int_hungarian(int_cost_mp, &int_hungarian_row_vp,
                          &int_int_hungarian_cost) == ERROR)
        {
            insert_error("Int hungarian reported error"); 
            kjb_print_error();
            result = EXIT_BUG;
            continue;
        }

        int_hungarian_cpu += get_cpu_time(); 

        int_hungarian_cost = (double)int_int_hungarian_cost / factor;

        if (ABS_OF(int_hungarian_cost - hungarian_cost) > tol)
        {
            dbp("===============================");
            p_stderr("int_hungarian() and hungarian() have different costs (%d x %d).\n", m, n); 
            p_stderr("ABS_OF(jv_lap_cost - hungarian_cost) / tol:  %.15e\n",  
                     ABS_OF(int_hungarian_cost - hungarian_cost) / tol); 
            dbp("===============================");

            result = EXIT_BUG;
        }

        for (i = 0; i < m; i++)
        {
            if (hungarian_row_vp->elements[ i ] != int_hungarian_row_vp->elements[ i ])
            {
                dbp("****************************");
                p_stderr("int_hungarian() and hungarian() have different assignments (%d x %d).\n", m, n); 
                p_stderr("ABS_OF(int_hungarian_cost - hungarian_cost) / tol:  %.15e\n",  
                         ABS_OF(int_hungarian_cost - hungarian_cost) / tol); 
                dbp("****************************");

                /*
                 * Not sufficient cause for failure.
                */

                break;  /* We only want one message per array. */
            }
        }

        /* --------------------------------------------- */

        init_cpu_time(); 

        if (int_jv_lap(int_cost_mp, &int_jv_lap_row_vp,
                          &int_int_jv_lap_cost) == ERROR)
        {
            insert_error("Int JV LAP reported error"); 
            kjb_print_error();
            continue;
        }

        int_jv_lap_cpu += get_cpu_time(); 
        int_jv_lap_cost = (double)int_int_jv_lap_cost / factor;

        /*
        // Use absolute to compare int to float.
        //
        if (ABS_OF((int_jv_lap_cost - hungarian_cost) / int_jv_lap_cost) > tol)
        */
        if (ABS_OF(int_jv_lap_cost - hungarian_cost) > tol)
        {
            dbp("===============================");
            p_stderr("int_jv_lap() and hungarian() have different costs (%d x %d).\n", m, n); 
            p_stderr("ABS_OF(int_jv_lap_cost - hungarian_cost) / tol:  %.15e\n", 
                     ABS_OF(int_jv_lap_cost - hungarian_cost) / tol); 
            dbp("===============================");

            result = EXIT_BUG;
        }

        for (i = 0; i < m; i++)
        {
            if (hungarian_row_vp->elements[ i ] != int_jv_lap_row_vp->elements[ i ])
            {
                dbp("****************************");
                p_stderr("int_jv_lap() and hungarian() have different assignments (%d x %d).\n", m, n); 
                p_stderr("ABS_OF(int_jv_lap_cost - hungarian_cost) / tol:  %.15e\n",  
                         ABS_OF(int_jv_lap_cost - hungarian_cost) / tol); 
                dbp("****************************");

                /*
                 * Not sufficient cause for failure.
                */

                break;  /* We only want one message per array. */
            }
        }
    }

    
    if (is_interactive())
    {
        dbi(overhead_cpu); 
        dbi(greedy_cpu); 
        dbi(dbl_hungarian_cpu); 
        dbi(jv_lap_cpu); 
        dbi(int_greedy_cpu); 
        dbi(hungarian_cpu); 
        dbi(int_hungarian_cpu); 
        dbi(int_jv_lap_cpu); 
    }

    free_matrix(mp);
    free_matrix(scaled_mp);
    free_int_matrix(int_cost_mp); 

    free_int_vector(dbl_hungarian_row_vp); 
    free_int_vector(hungarian_row_vp); 
    free_int_vector(jv_lap_row_vp); 
    free_int_vector(int_hungarian_row_vp); 
    free_int_vector(int_jv_lap_row_vp); 

    return result; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int greedy_match(const Matrix* mp, double* cost_ptr) 
{
    Matrix* copy_mp = NULL;
    int count;
    int m = mp->num_rows;
    int n = mp->num_cols;
    int max_i, max_j;
    double max; 
    double greedy_sum = 0.0;
    int i, j;


    ERE(copy_matrix(&copy_mp, mp));
    
    for (count = 0; count < MIN_OF(m, n); count++)
    {
        get_min_matrix_element(copy_mp, &max, &max_i, &max_j); 

        greedy_sum += max; 

        for (j = 0; j < n; j++)
        {
            copy_mp->elements[ max_i ][ j ] = DBL_MOST_POSITIVE; 
        }

        for (i = 0; i < m; i++)
        {
            copy_mp->elements[ i ][ max_j ] = DBL_MOST_POSITIVE; 
        }
    }

    free_matrix(copy_mp);

    *cost_ptr = greedy_sum; 

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int int_greedy_match(const Int_matrix* mp, int* cost_ptr) 
{
    Int_matrix* copy_mp    = NULL;
    int         count;
    int         m          = mp->num_rows;
    int         n          = mp->num_cols;
    int         max_i;
    int         max_j;
    int         max;
    int         greedy_sum = 0;
    int         i;
    int         j;


    ERE(copy_int_matrix(&copy_mp, mp));

    for (count = 0; count < MIN_OF(m, n); count++)
    {
        get_min_int_matrix_element(copy_mp, &max, &max_i, &max_j); 

        greedy_sum += max; 

        for (j = 0; j < n; j++)
        {
            copy_mp->elements[ max_i ][ j ] = INT_MAX; 
        }

        for (i = 0; i < m; i++)
        {
            copy_mp->elements[ i ][ max_j ] = INT_MAX; 
        }
    }

    free_int_matrix(copy_mp);

    *cost_ptr = greedy_sum; 

    return NO_ERROR;
}






