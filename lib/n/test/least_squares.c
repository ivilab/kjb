
/* $Id: least_squares.c 21662 2017-08-05 17:00:07Z kobus $ */


#include "n/n_incl.h" 

/*
#define VERBOSE 1
*/

#define BASE_NUM_TRIES 3000 
#define BASE_MAX_SIZE   100

/* -------------------------------------------------------------------------- */

static int old_least_squares
(
    Vector** result_vpp,
    Matrix*  mp,
    Vector*  vp 
);

/* -------------------------------------------------------------------------- */

/*ARGSUSED*/
int main(int argc, char *argv[])
{
    int status = EXIT_SUCCESS;
    Matrix* mp = NULL;
    Vector* vp = NULL; 
    Vector* result_vp = NULL; 
    Vector* result_2_vp = NULL; 
    int count; 
    int num_unknowns;
    int num_eq; 
    double diff; 
    double rel_diff; 
    int  num_tries = NOT_SET;
    int max_size = NOT_SET;
    int  test_factor = 1;
    double fit_error, old_fit_error, fit_error_diff;
    const char* least_squares_method_str; 
    const char* invert_method_str; 


    kjb_init(); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (test_factor <= 0)
    {
        num_tries = 5;
        max_size = 20;
    }
    else
    {   
        double factor_for_linear = pow((double)test_factor, 1.0/4.0);

        num_tries = kjb_rint(factor_for_linear * (double)BASE_NUM_TRIES);
        max_size = kjb_rint(factor_for_linear * (double)BASE_MAX_SIZE);
    } 

    if (is_interactive())
    {
        EPETE(set_verbose_options("verbose", "1")); 
    }

    for (count = 0; count<num_tries; count++)
    {
        num_unknowns = 1 + ((double)max_size) * kjb_rand();
        num_eq = num_unknowns + ((double)max_size) * kjb_rand();

        EPETE(get_random_matrix(&mp, num_eq, num_unknowns));
        EPETE(get_random_vector(&vp, num_eq)); 

        if (count % 4 < 2)
        {
            least_squares_method_str = "svd"; 
            EPETE(set_least_squares_options("least-squares-method", 
                                            least_squares_method_str)); 
        }
        else
        {
            least_squares_method_str = "pi"; 
            EPETE(set_least_squares_options("least-squares-method", 
                                            least_squares_method_str)); 
        }

        if (count % 2 == 0)
        {
            invert_method_str = "ge";
            EPETE(set_matrix_inversion_options("matrix-inversion", invert_method_str)); 
        }
        else 
        {
            invert_method_str = "svd";
            EPETE(set_matrix_inversion_options("matrix-inversion", invert_method_str)); 
        }

        EPETE(least_squares_2(&result_vp, mp, vp, &fit_error)); 

        EPETE(old_least_squares(&result_2_vp, mp, vp)); 

        EPETE(get_linear_equation_rms_error(mp, result_2_vp, vp, &old_fit_error));

        fit_error_diff = fit_error - old_fit_error;

        diff = rms_absolute_error_between_vectors(result_vp, result_2_vp);
        rel_diff = rms_relative_error_between_vectors(result_vp, result_2_vp);
        
        verbose_pso(1, "Test %d: [ %d eq, %d unknowns] ==> %.3e (rel %.3e). Fit error diff: %.3e\n", 
                    count+1, num_eq, num_unknowns, diff, rel_diff, 
                    fit_error_diff); 

        if (fit_error_diff > 1.0e5 * DBL_EPSILON)
        {
            p_stderr("\n");
            p_stderr("Problem found in test %d.\n", count + 1); 
            p_stderr("Comparing least squares method %s with an older method.\n", 
                     least_squares_method_str); 
            p_stderr("Where appropriate, matrix inversion is done with method %s.\n",
                     invert_method_str);
            p_stderr("Size is %d equations and %d unknowns.\n", num_eq, num_unknowns);
            p_stderr("Absolute diff is %.3e. Relative diff is %.3e\n", diff, rel_diff);
            p_stderr("Fit error diff is %.3e (negative is preferred).\n", fit_error_diff);
            p_stderr("\n");
            status = EXIT_BUG;
        }
    }

    free_vector(result_vp);
    free_vector(result_2_vp);
    free_vector(vp);
    free_matrix(mp); 

    return status;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int old_least_squares(Vector**result_vpp, Matrix* mp, Vector* vp) 
{
    Matrix* a_mp;
    Vector* b_vp;
    int     i;
    int     j;
    int     k;
    double    sum;
    Matrix* inv_a_mp = NULL;
    int     num_eq;
    int     num_unknowns;
    int     result; 


    num_eq = mp->num_rows;
    num_unknowns = mp->num_cols;

    if (num_eq != vp->length) 
    {
        add_error("Argument error in least_squares");
        set_error("The number of matrix rows does not match vector length.");
        return ERROR;
    }

    if (num_unknowns > num_eq) 
    {
        set_error("More unknowns than equations in least_squares.");
        return ERROR;
    }

    NRE(a_mp = create_matrix(num_unknowns, num_unknowns));
    NRE(b_vp = create_vector(num_unknowns));

    for (i=0; i<num_unknowns; i++) 
    {
        for (j=0; j<num_unknowns; j++) 
        {
            sum = 0.0;

            for (k=0; k<num_eq; k++) 
            {
                sum += (mp->elements)[k][i] * (mp->elements)[k][j];
            }
            (a_mp->elements)[ i ][ j ] = sum;
        }
     }

     for (i=0; i<num_unknowns; i++) 
     {
         sum = 0.0;

         for (k=0; k<num_eq; k++) 
         {
             sum += (vp->elements)[k] * (mp->elements)[k][i];
         }
         (b_vp->elements)[ i ] = sum; 
     }
    
     result = get_matrix_inverse(&inv_a_mp, a_mp);

     if (result != ERROR)
     {
         result = multiply_matrix_and_vector(result_vpp, inv_a_mp, b_vp);
     }

     free_matrix(a_mp);
     free_vector(b_vp);

     free_matrix(inv_a_mp);

     if (result == ERROR)
     {
         add_error("Error occurred computing least squares soluiton of Ax=b.");
     }

     return result;
}

