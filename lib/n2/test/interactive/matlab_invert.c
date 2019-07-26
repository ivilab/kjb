
/* $Id: matlab_invert.c 21491 2017-07-20 13:19:02Z kobus $ */


#define NUM_ITS 100000000
#define MATRIX_SIZE 300

#include "n2/n2_incl.h" 

/*ARGSUSED*/
int main(void)
{
#ifdef KJB_HAVE_MATLAB
    Matrix *mp, *inv_mp; 
    Matrix *ident_mp; 
    Matrix *test_ident_mp = NULL; 
    double diff; 


    if (! is_interactive()) 
    {
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    kjb_l_set("page", "f"); 

    EPETE(get_random_matrix(&mp, 8, 8));
    inv_mp = MATLAB_invert_matrix(mp); 
    EPETE(multiply_matrices(&test_ident_mp, mp, inv_mp)); 
    db_mat(test_ident_mp); 

    NPETE(get_random_matrix(&test_ident_mp, MATRIX_SIZE, MATRIX_SIZE));

    pso("Invert matrix of size %d\n", MATRIX_SIZE);

    init_real_time(); 

    inv_mp = MATLAB_invert_matrix(mp); 

    display_real_time();

    pso("\n");

    NPETE(get_identity_matrix(&ident_mp, MATRIX_SIZE));

    EPETE(multiply_matrices(&test_ident_mp, mp, inv_mp)); 

    diff = max_abs_matrix_difference(ident_mp,test_ident_mp);

    dbf(diff); 

    free_matrix(test_ident_mp); 
    free_matrix(mp); 
    free_matrix(ident_mp); 
#endif 

    return EXIT_SUCCESS; 
}

