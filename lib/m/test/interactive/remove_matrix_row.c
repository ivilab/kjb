
/* $Id: remove_matrix_row.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "m/m_incl.h" 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    Matrix* mp = NULL;
    Vector* vp = NULL; 

    EPETE(get_random_matrix(&mp, 10, 5));

    db_mat(mp); 

    EPETE(remove_matrix_row((Vector**)NULL, mp, 0)); 
    db_mat(mp); 

    EPETE(remove_matrix_row((Vector**)NULL, mp, 8)); 
    db_mat(mp); 

    EPETE(remove_matrix_row((Vector**)NULL, mp, 5)); 
    db_mat(mp); 

    EPETE(remove_matrix_row((Vector**)NULL, mp, 5)); 
    db_mat(mp); 

    EPETE(remove_matrix_row(&vp, mp, 0)); 
    db_mat(mp); 
    db_rv(vp); 

    EPETE(remove_matrix_row(&vp, mp, 4)); 
    db_mat(mp); 
    db_rv(vp); 

    EPETE(remove_matrix_row(&vp, mp, 2)); 
    db_mat(mp); 
    db_rv(vp); 

    EPETE(remove_matrix_row(&vp, mp, 2)); 
    db_mat(mp); 
    db_rv(vp); 

    free_vector(vp);
    free_matrix(mp); 

    return EXIT_SUCCESS; 
}

