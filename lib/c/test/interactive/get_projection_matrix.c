
/* $Id: get_projection_matrix.c 21491 2017-07-20 13:19:02Z kobus $ */


/*
// NOT TESTED : Errors and behaviour under small denoms! 
*/

#include "c/c_incl.h" 

int main(int argc, char *argv[])
{
    Matrix* mp = NULL;
    Matrix* projected_mp = NULL; 
    Matrix* back_projected_mp = NULL; 

    /* Not ready to do anything in batch mode.  */
    if (! is_interactive()) 
    {
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    EPETE(get_random_matrix(&mp, 6, 3));

    mp->elements[ 2 ][ 0 ] = 0.000000001;
    mp->elements[ 2 ][ 1 ] = 0.000000001;
    mp->elements[ 2 ][ 2 ] = 0.000000001;
    mp->elements[ 3 ][ 0 ] = 0.000000001;
    mp->elements[ 4 ][ 1 ] = 0.000000001;
    mp->elements[ 5 ][ 2 ] = 0.000000001;


    EPETE(get_projection_matrix(&projected_mp, mp, DIVIDE_BY_RED, 
                                ALL_CHROMATICITIES_MUST_BE_VALID, 
                                DBL_NOT_SET, DBL_NOT_SET,
                                DBL_NOT_SET, NULL, NULL));

    EPETE(back_project_matrix(&back_projected_mp, projected_mp, DIVIDE_BY_RED));

    db_mat(mp); 
    pso("DIVIDE_BY_RED\n\n");
    db_mat(projected_mp); 
    db_mat(back_projected_mp); 

    pso("\n-----------------------------\n"); 
    

    EPETE(get_projection_matrix(&projected_mp, mp, DIVIDE_BY_GREEN, 
                                ALL_CHROMATICITIES_MUST_BE_VALID, 
                                DBL_NOT_SET, DBL_NOT_SET,
                                DBL_NOT_SET, NULL, NULL));

    EPETE(back_project_matrix(&back_projected_mp, projected_mp,
                              DIVIDE_BY_GREEN));

    db_mat(mp); 
    pso("DIVIDE_BY_GREEN\n\n");
    db_mat(projected_mp); 
    db_mat(back_projected_mp); 
    pso("\n-----------------------------\n"); 
    

    EPETE(get_projection_matrix(&projected_mp, mp, DIVIDE_BY_BLUE, 
                                ALL_CHROMATICITIES_MUST_BE_VALID, 
                                DBL_NOT_SET, DBL_NOT_SET,
                                DBL_NOT_SET, NULL, NULL));

    EPETE(back_project_matrix(&back_projected_mp, projected_mp, 
                              DIVIDE_BY_BLUE));

    db_mat(mp); 
    pso("DIVIDE_BY_BLUE\n\n");
    db_mat(projected_mp); 
    db_mat(back_projected_mp); 
    pso("\n-----------------------------\n"); 
    

    EPETE(get_projection_matrix(&projected_mp, mp, DIVIDE_BY_SUM, 
                                ALL_CHROMATICITIES_MUST_BE_VALID, 
                                DBL_NOT_SET, DBL_NOT_SET,
                                DBL_NOT_SET, NULL, NULL));

    EPETE(back_project_matrix(&back_projected_mp, projected_mp, DIVIDE_BY_SUM));

    db_mat(mp); 
    pso("DIVIDE_BY_SUM\n\n");
    db_mat(projected_mp); 
    db_mat(back_projected_mp); 
    pso("\n-----------------------------\n"); 
    

    EPETE(get_projection_matrix(&projected_mp, mp, ONTO_RG_PLANE, 
                                ALL_CHROMATICITIES_MUST_BE_VALID, 
                                DBL_NOT_SET, DBL_NOT_SET,
                                DBL_NOT_SET, NULL, NULL));

    db_mat(mp); 
    pso("ONTO_RG_PLANE\n\n");
    db_mat(projected_mp); 
    

    EPETE(get_projection_matrix(&projected_mp, mp, ONTO_RB_PLANE, 
                                ALL_CHROMATICITIES_MUST_BE_VALID, 
                                DBL_NOT_SET, DBL_NOT_SET,
                                DBL_NOT_SET, NULL, NULL));

    db_mat(mp); 
    pso("ONTO_RB_PLANE\n\n");
    db_mat(projected_mp); 
    pso("\n-----------------------------\n"); 
    

    EPETE(get_projection_matrix(&projected_mp, mp, ONTO_GB_PLANE, 
                                ALL_CHROMATICITIES_MUST_BE_VALID, 
                                DBL_NOT_SET, DBL_NOT_SET,
                                DBL_NOT_SET, NULL, NULL));

    db_mat(mp); 
    pso("ONTO_GB_PLANE\n\n");
    db_mat(projected_mp); 
    

    free_matrix(mp);
    free_matrix(projected_mp);
    free_matrix(back_projected_mp);
   
    return EXIT_SUCCESS; 
}

