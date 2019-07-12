
/* $Id: quadratic.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "n/n_incl.h"
#include "n2/n2_incl.h"
#include "wrap_slatec/wrap_slatec.h"

/*ARGSUSED*/
int main(void)
{
    Matrix *mp, *le_constraint_mp;
    Vector *target_vp;
    Vector *le_constraint_col_vp;
    Vector *eq_constraint_col_vp;
    Matrix *eq_constraint_mp;
    Vector *lb_row_vp, *ub_row_vp;
    Vector *result_vp = NULL;
    int res; 
    int n, m, c;
    int method;
 
 
    kjb_init();

    le_constraint_mp=create_matrix(3,2);
    le_constraint_col_vp=create_vector(3);
    eq_constraint_mp=create_matrix(2,2);
    eq_constraint_col_vp=create_vector(2);
    lb_row_vp=create_vector(2);
    ub_row_vp=create_vector(2);
 
    lb_row_vp->elements [0] = 0.0; 
    lb_row_vp->elements [1] = 0.0; 
    ub_row_vp->elements [0] = DBL_HALF_MOST_POSITIVE; 
    ub_row_vp->elements [1] = DBL_HALF_MOST_POSITIVE;
 
    eq_constraint_mp->elements[0][0]=3.0;
    eq_constraint_mp->elements[0][1]=2.0;
    eq_constraint_mp->elements[1][0]=3.0;
    eq_constraint_mp->elements[1][1]=2.0;
 
    eq_constraint_col_vp->elements[0]=250.0;
    eq_constraint_col_vp->elements[1]=250.0;
 
    le_constraint_mp->elements[0][0]=2.0;
    le_constraint_mp->elements[0][1]=1.0;
    le_constraint_mp->elements[1][0]=1.0;
    le_constraint_mp->elements[1][1]=2.0;
    le_constraint_mp->elements[2][0]=2.0;
    le_constraint_mp->elements[2][1]=3.0;
 
    le_constraint_col_vp->elements[0]=195.0;
    le_constraint_col_vp->elements[1]=240;
    le_constraint_col_vp->elements[2]=390;
 
    mp = create_matrix(2, 2);
    (mp->elements)[0][0]=1.0;
    (mp->elements)[0][1]=1.0;
    (mp->elements)[1][0]=2.0;
    (mp->elements)[1][1]=1.0;
 
    target_vp = create_vector(2);
    (target_vp->elements)[0] = 150.0;
    (target_vp->elements)[1] = 200.0;
 
    EPETE(res = least_squares(&result_vp, mp, target_vp));
 
    fp_write_col_vector_with_title(result_vp,stdout,"Least squares RESULT");
 
    EPETE(res = constrained_least_squares(&result_vp, mp, target_vp ,
                                          le_constraint_mp,
                                          le_constraint_col_vp,
                                          NULL, NULL,
                                          lb_row_vp, ub_row_vp));
 
    fp_write_col_vector_with_title(result_vp,stdout,"Constrained least squares RESULT");
 
    EPETE(res = do_dbocls_quadratic(&result_vp, mp, target_vp ,
                                    le_constraint_mp,le_constraint_col_vp,
                                    NULL, NULL,
                                    lb_row_vp, ub_row_vp));
 
    fp_write_col_vector_with_title(result_vp,stdout,"DBOCLS RESULT");
 

    EPETE(res = do_dlsei_quadratic(&result_vp, mp, target_vp ,
                                   le_constraint_mp,le_constraint_col_vp,
                                   NULL, NULL,
                                   lb_row_vp, ub_row_vp));
 
    fp_write_col_vector_with_title(result_vp,stdout,"DLSEI RESULT");
     
#ifdef KJB_HAVE_MATLAB
    EPETE(res = do_matlab_quadratic(&result_vp, mp, target_vp ,
                                    le_constraint_mp, le_constraint_col_vp,
                                    NULL, NULL,
                                    lb_row_vp, ub_row_vp));
 
    fp_write_col_vector_with_title(result_vp,stdout,"MATLAB RESULT");
#endif 

#ifdef KJB_HAVE_LOQO

    EPETE(res = do_loqo_quadratic(&result_vp, mp, target_vp ,
                                  le_constraint_mp, le_constraint_col_vp,
                                  NULL, NULL,
                                  lb_row_vp, ub_row_vp));
 
    fp_write_col_vector_with_title(result_vp,stdout,"LOQO RESULT");
#endif 


/* -------------------------------------------------------------------------- */

   
    /*
     * The next tests print results to standard output, but they are not really
     * ready for any sensible batch testing. 
    */

    if (! is_interactive())
    {
        return EXIT_SUCCESS;
    }

    
/* -------------------------------------------------------------------------- */

 
    n = 10;
    m = 20;
    c = 5;

    dbi(res); 
 
    EPETE(get_random_matrix(&mp, m, n));
    EPETE(get_random_vector(&target_vp, m));
    EPETE(get_zero_vector(&lb_row_vp, n)); 
    EPETE(get_initialized_vector(&ub_row_vp, n, 10.0)); 
    EPETE(get_random_matrix(&le_constraint_mp, c, n)); 
    EPETE(get_random_vector(&le_constraint_col_vp, c)); 

    for (method = 0; method < 3; method++)
    {
        const char* method_str; 
        char title_str[ 1000 ]; 

        if (method == 0)
        {
            EPE(res = least_squares(&result_vp, mp, target_vp));
         
            fp_write_col_vector_with_title(result_vp,stdout,"Constrained least squares (no constraints) RESULT");

            continue;
        }
        else if (method == 1)
        {
            method_str = "dbocls";
        }
        else if (method == 2)
        {
            method_str = "dlsei";
        }
        else
        {
            SET_CANT_HAPPEN_BUG();
            kjb_exit(EXIT_FAILURE);
        }

        EPETE(kjb_n_set("constrained-least-squares-method", method_str));

        EPETE(res = constrained_least_squares(&result_vp, mp, target_vp ,
                                              NULL, NULL,
                                              NULL, NULL,
                                              NULL, NULL));
 
        EPETE(kjb_sprintf(title_str, sizeof(title_str),
                          "Constrained least squares (no constraints) RESULT (%s)", 
                          method_str));

        fp_write_col_vector_with_title(result_vp, stdout, title_str);
 
        EPETE(res = do_dbocls_quadratic(&result_vp, mp, target_vp ,
                                        NULL, NULL, NULL, NULL,
                                        lb_row_vp, ub_row_vp));
     
        EPETE(kjb_sprintf(title_str, sizeof(title_str),
                          "Constrained least squares (bounds only) RESULT (%s)", 
                          method_str));

        fp_write_col_vector_with_title(result_vp,stdout, title_str);
 
        EPETE(res = do_dbocls_quadratic(&result_vp, mp, target_vp ,
                                        le_constraint_mp,le_constraint_col_vp,
                                        NULL, NULL,
                                        lb_row_vp, ub_row_vp));

        EPETE(kjb_sprintf(title_str, sizeof(title_str),
                          "Constrained least squares RESULT (%s)", 
                          method_str));

        fp_write_col_vector_with_title(result_vp,stdout, title_str);
    }
 
 
/* -------------------------------------------------------------------------- */

    free_matrix(le_constraint_mp);
    free_vector(le_constraint_col_vp);
    free_matrix(eq_constraint_mp);
    free_vector(eq_constraint_col_vp);
    free_vector(lb_row_vp);
    free_vector(ub_row_vp);
    free_vector(result_vp);
    free_matrix(mp);
    free_vector(target_vp);

    return EXIT_SUCCESS; 
   }


