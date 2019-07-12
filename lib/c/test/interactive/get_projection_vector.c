
/* $Id: get_projection_vector.c 21491 2017-07-20 13:19:02Z kobus $ */


/*
// NOT TESTED : Errors and behaviour under small denoms! 
*/

#include "c/c_incl.h" 

int main(int argc, char *argv[])
{
    Vector* vp = NULL;
    Vector* projected_vp = NULL; 

    dbw();

    /* Not ready to do anything in batch mode.  */
    if (! is_interactive()) 
    {
    dbw();
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    dbw();
    
    EPETE(get_random_vector(&vp, 3));


    EPETE(get_projection_vector(&projected_vp, vp, DIVIDE_BY_RED, 
                                DBL_NOT_SET));


    db_rv(vp); 
    pso("DIVIDE_BY_RED\n\n");
    db_rv(projected_vp); 
    pso("\n-----------------------------\n"); 
    

    EPETE(get_projection_vector(&projected_vp, vp, DIVIDE_BY_GREEN, 
                                DBL_NOT_SET));

    db_rv(vp); 
    pso("DIVIDE_BY_GREEN\n\n");
    db_rv(projected_vp); 
    pso("\n-----------------------------\n"); 
    

    EPETE(get_projection_vector(&projected_vp, vp, DIVIDE_BY_BLUE, 
                                DBL_NOT_SET));

    db_rv(vp); 
    pso("DIVIDE_BY_BLUE\n\n");
    db_rv(projected_vp); 
    pso("\n-----------------------------\n"); 
    

    EPETE(get_projection_vector(&projected_vp, vp, DIVIDE_BY_SUM, DBL_NOT_SET));

    db_rv(vp); 
    pso("DIVIDE_BY_SUM\n\n");
    db_rv(projected_vp); 
    pso("\n-----------------------------\n"); 
    

    EPETE(get_projection_vector(&projected_vp, vp, ONTO_RG_PLANE, DBL_NOT_SET));

    db_rv(vp); 
    pso("ONTO_RG_PLANE\n\n");
    db_rv(projected_vp); 
    pso("\n-----------------------------\n"); 
    

    EPETE(get_projection_vector(&projected_vp, vp, ONTO_RB_PLANE, DBL_NOT_SET));

    db_rv(vp); 
    pso("ONTO_RB_PLANE\n\n");
    db_rv(projected_vp); 
    pso("\n-----------------------------\n"); 
    

    EPETE(get_projection_vector(&projected_vp, vp, ONTO_GB_PLANE, DBL_NOT_SET));

    db_rv(vp); 
    pso("ONTO_GB_PLANE\n\n");
    db_rv(projected_vp); 
    

    free_vector(vp);
    free_vector(projected_vp);
   
    return EXIT_SUCCESS; 
}

