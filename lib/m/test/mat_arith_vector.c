
/* $Id: mat_arith_vector.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h"

/*
#define VERBOSE 1
*/

#define NUM_LOOPS        50
#define BASE_NUM_TRIES   10


static void test_bug_handler(const char* mess);


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int num_rows;
    int num_cols; 
    int count;
    Matrix* a_mp = NULL; 
    Matrix* b_mp = NULL; 
    Matrix* c_mp = NULL; 
    Matrix* d_mp = NULL; 
    Matrix* e_mp = NULL; 
    Matrix* first_mp = NULL;
    Vector* row_vp = NULL; 
    Vector* col_vp = NULL; 
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;


    kjb_init(); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (test_factor == 0)
    {
        num_tries = 1;
    }
    else
    {
        num_tries *= test_factor;
    }



    set_bug_handler(test_bug_handler); 


    for (count=0; count<num_tries; count++)
    {
        if (IS_EVEN(count))
        {
            enable_respect_missing_values();
        }
        else 
        {
            disable_respect_missing_values();
        }

        for (num_rows=1; num_rows<NUM_LOOPS; num_rows++)
        {
            for (num_cols=1; num_cols<NUM_LOOPS; num_cols++)
            {
                double  diff[ 16 ];
                int     i; 
                int     result; 
                int     diff_count = 0;


#ifdef VERBOSE
                pso("\n-------------------------------------------------\n\n");
                pso("%d %d %d\n\n", count, num_rows, num_cols);
#endif 

                
                EPETE(get_random_matrix(&first_mp, num_rows, num_cols)); 

                first_mp->elements[ num_rows / 2 ][ num_cols / 2 ] = DBL_MISSING; 

                EPETE(get_initialized_vector(&row_vp, num_cols, (double)count));

                EPETE(get_initialized_vector(&col_vp, num_rows, (double)count));

#ifdef VERBOSE
                dbp(" ----------- \n");
                db_mat(first_mp);
                dbp("\n ADD \n"); 
                db_rv(row_vp); 
                db_cv(col_vp); 
#endif 
                EPETE(add_scalar_to_matrix(&a_mp, first_mp, (double)count));

                EPETE(add_row_vector_to_matrix(&b_mp, first_mp, row_vp)); 


                diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, b_mp); 

                EPETE(copy_matrix(&c_mp, first_mp)); 
                EPETE(ow_add_row_vector_to_matrix(c_mp, row_vp)); 

                diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, c_mp); 

                EPETE(add_col_vector_to_matrix(&d_mp, first_mp, col_vp)); 

                diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, d_mp); 

                EPETE(copy_matrix(&e_mp, first_mp)); 
                EPETE(ow_add_col_vector_to_matrix(e_mp, col_vp)); 

                diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, e_mp); 

#ifdef VERBOSE
                db_mat(a_mp);
                db_mat(b_mp);
                db_mat(c_mp);
                db_mat(d_mp);
                db_mat(e_mp);
#endif 


                if (respect_missing_values())
                {
                    if (a_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, a_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                    if (b_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, b_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                    if (c_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, c_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                    if (d_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, d_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                    if (e_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, e_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                }


#ifdef VERBOSE
                dbp(" ----------- \n");
                db_mat(first_mp);
                dbp("\n SUBTRACT \n"); 
                db_rv(row_vp); 
                db_cv(col_vp); 
#endif 
                EPETE(subtract_scalar_from_matrix(&a_mp, first_mp, (double)count));

                EPETE(subtract_row_vector_from_matrix(&b_mp, first_mp, row_vp)); 

                diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, b_mp); 

                EPETE(copy_matrix(&c_mp, first_mp)); 
                EPETE(ow_subtract_row_vector_from_matrix(c_mp, row_vp)); 

                diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, c_mp); 

                EPETE(subtract_col_vector_from_matrix(&d_mp, first_mp, col_vp)); 

                diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, d_mp); 

                EPETE(copy_matrix(&e_mp, first_mp)); 
                EPETE(ow_subtract_col_vector_from_matrix(e_mp, col_vp)); 

                diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, e_mp); 

#ifdef VERBOSE
                db_mat(a_mp);
                db_mat(b_mp);
                db_mat(c_mp);
                db_mat(d_mp);
                db_mat(e_mp);
#endif 


                if (respect_missing_values())
                {
                    if (a_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, a_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                    if (b_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, b_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                    if (c_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, c_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                    if (d_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, d_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                    if (e_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, e_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                }



#ifdef VERBOSE
                dbp(" ----------- \n");
                db_mat(first_mp);
                dbp("\n MULTIPLY \n"); 
                db_rv(row_vp); 
                db_cv(col_vp); 
#endif 
                EPETE(multiply_matrix_by_scalar(&a_mp, first_mp, 
                                                (double)count));

                EPETE(multiply_matrix_by_row_vector_ew(&b_mp, first_mp, row_vp)); 

                diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, b_mp); 

                EPETE(copy_matrix(&c_mp, first_mp)); 
                EPETE(ow_multiply_matrix_by_row_vector_ew(c_mp, row_vp)); 

                diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, c_mp); 

                EPETE(multiply_matrix_by_col_vector_ew(&d_mp, first_mp, col_vp)); 

                diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, d_mp); 

                EPETE(copy_matrix(&e_mp, first_mp)); 
                EPETE(ow_multiply_matrix_by_col_vector_ew(e_mp, col_vp)); 

                diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, e_mp); 

#ifdef VERBOSE
                db_mat(a_mp);
                db_mat(b_mp);
                db_mat(c_mp);
                db_mat(d_mp);
                db_mat(e_mp);
#endif 


                if (respect_missing_values())
                {
                    if (a_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, a_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                    if (b_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, b_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                    if (c_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, c_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                    if (d_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, d_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                    if (e_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, e_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                }




#ifdef VERBOSE
                dbp(" ----------- \n");
                db_mat(first_mp);
                dbp("\n MULTIPLY \n"); 
                db_rv(row_vp); 
                db_cv(col_vp); 
#endif 
                result = divide_matrix_by_scalar(&a_mp, first_mp, 
                                                (double)count);

                if (count == 0)
                {
                    if (result != ERROR)
                    {
                        p_stderr("divide_matrix_by_scalar missed divide by zero.\n");
                        status = EXIT_BUG;
                    }
#ifdef VERBOSE
                    else 
                    {
                        p_stderr("Verifying divide by zero error caught.\n"); 
                        kjb_print_error();
                    }
#endif 
                }
                else
                {
#ifdef VERBOSE
                    db_mat(a_mp);
#else              
                    /*EMPTY*/
                    ; /* Do nothing */
#endif 
                }
                        
                result = divide_matrix_by_row_vector(&b_mp, first_mp, row_vp); 

                if (count == 0)
                {
                    if (result != ERROR)
                    {
                        p_stderr("divide_matrix_by_row_vector missed divide by zero.\n");
                        status = EXIT_BUG;
                    }
                    else 
                    {
                        diff[ diff_count++ ] = 0.0;  
#ifdef VERBOSE
                        kjb_print_error();
#endif 
                    }
                }
                else 
                {
                    diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, b_mp); 
#ifdef VERBOSE
                    db_mat(b_mp);
#endif 
                }

                EPETE(copy_matrix(&c_mp, first_mp)); 
                result = ow_divide_matrix_by_row_vector(c_mp, row_vp); 

                if (count == 0)
                {
                    if (result != ERROR)
                    {
                        p_stderr("ow_divide_matrix_by_row_vector missed divide by zero.\n");
                        status = EXIT_BUG;
                    }
                    else 
                    {
                        diff[ diff_count++ ] = 0.0;  
#ifdef VERBOSE
                        kjb_print_error();
#endif 
                    }
                }
                else 
                {
                    diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, c_mp); 
#ifdef VERBOSE
                    db_mat(c_mp);
#endif 
                }


                result = divide_matrix_by_col_vector(&d_mp, first_mp, col_vp); 

                if (count == 0)
                {
                    if (result != ERROR)
                    {
                        p_stderr("divide_matrix_by_col_vector missed divide by zero.\n");
                        status = EXIT_BUG;
                    }
                    else 
                    {
                        diff[ diff_count++ ] = 0.0;  
#ifdef VERBOSE
                        kjb_print_error();
#endif 
                    }
                }
                else 
                {
                    diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, d_mp); 
#ifdef VERBOSE
                    db_mat(d_mp);
#endif 
                }

                EPETE(copy_matrix(&e_mp, first_mp)); 
                result = ow_divide_matrix_by_col_vector(e_mp, col_vp); 

                if (count == 0)
                {
                    if (result != ERROR)
                    {
                        p_stderr("ow_divide_matrix_by_col_vector missed divide by zero.\n");
                        status = EXIT_BUG;
                    }
                    else 
                    {
                        diff[ diff_count++ ] = 0.0;  
#ifdef VERBOSE
                        kjb_print_error();
#endif 
                    }
                }
                else 
                {
                    diff[ diff_count++ ] = max_abs_matrix_difference(a_mp, e_mp); 
#ifdef VERBOSE
                    db_mat(e_mp);
#endif 
                }


                if (respect_missing_values())
                {
                    if (a_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, a_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                    if (b_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, b_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                    if (c_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, c_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                    if (d_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, d_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                    if (e_mp->elements[ num_rows / 2 ][ num_cols / 2 ] != DBL_MISSING)
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, e_mp->elements[ num_rows / 2 ][ num_cols / 2 ] - DBL_MISSING); 
                        p_stderr("Problem with missing values.\n");
                        status = EXIT_BUG;
                    }

                }


                for (i=0; i<diff_count; i++) 
                {
                    if ( ! IS_ZERO_DBL(diff[ i ]))
                    {
                        p_stderr("(%d %d %d) %e ! 0.0\n", count, num_rows, 
                                 num_cols, diff[ i ]); 
                        p_stderr("Problem with test %d.\n", i);
                        status = EXIT_BUG;
                    }

                }
            }
        }
    }

#ifdef VERBOSE
    for (count=0; count<num_tries; count++)
    {
        dbi(respect_missing_values());
        restore_respect_missing_values();
    }
#endif 

    
    free_matrix(first_mp);
    free_vector(row_vp);
    free_vector(col_vp);
    free_matrix(a_mp); 
    free_matrix(b_mp);
    free_matrix(c_mp);
    free_matrix(d_mp);
    free_matrix(e_mp);

    return status; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void test_bug_handler(const char* mess)
{
    /*
    kjb_puts(mess); 
    kjb_puts("\n"); 
    */
    set_error(mess);
}

