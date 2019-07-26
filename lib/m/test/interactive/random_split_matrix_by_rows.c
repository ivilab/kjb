
/* $Id: remove_matrix_row.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "m/m_incl.h" 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    Matrix* mp = NULL;
    Matrix* s1_mp = NULL;
    Matrix* s2_mp = NULL;

    EPETE(get_random_matrix(&mp, 10, 5));

    db_mat(mp); 

    pso("\n----------------------------------------------------------\n"); 

    EPE(random_split_matrix_by_rows(&s1_mp, &s2_mp, mp, -1.0)); 
    db_mat(s1_mp); 
    db_mat(s2_mp); 

    pso("\n----------------------------------------------------------\n"); 


    EPE(random_split_matrix_by_rows(&s1_mp, &s2_mp, mp, 0.0)); 
    db_mat(s1_mp); 
    db_mat(s2_mp); 

    pso("\n----------------------------------------------------------\n"); 


    EPE(random_split_matrix_by_rows(&s1_mp, &s2_mp, mp, 0.3)); 
    db_mat(s1_mp); 
    db_mat(s2_mp); 

    pso("\n----------------------------------------------------------\n"); 


    EPE(random_split_matrix_by_rows(&s1_mp, &s2_mp, mp, 0.3)); 
    db_mat(s1_mp); 
    db_mat(s2_mp); 

    pso("\n----------------------------------------------------------\n"); 


    EPE(random_split_matrix_by_rows(&s1_mp, &s2_mp, mp, 0.5)); 
    db_mat(s1_mp); 
    db_mat(s2_mp); 

    pso("\n----------------------------------------------------------\n"); 


    EPE(random_split_matrix_by_rows(&s1_mp, &s2_mp, mp, 0.5)); 
    db_mat(s1_mp); 
    db_mat(s2_mp); 

    pso("\n----------------------------------------------------------\n"); 

    EPE(random_split_matrix_by_rows(&s1_mp, &s2_mp, mp, 1.0)); 
    db_mat(s1_mp); 
    db_mat(s2_mp); 

    pso("\n----------------------------------------------------------\n"); 

    EPE(random_split_matrix_by_rows(&s1_mp, &s2_mp, mp, 10.0)); 
    db_mat(s1_mp); 
    db_mat(s2_mp); 

    pso("\n----------------------------------------------------------\n"); 

    free_matrix(mp); 
    free_matrix(s1_mp); 
    free_matrix(s2_mp); 

    return EXIT_SUCCESS; 
}

