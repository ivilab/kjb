
/* $Id: check_same_matrix_vector_dimensions.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "m/m_incl.h" 

int main(int argc, char **argv)
{
    Matrix_vector* mvp1 = NULL;
    Matrix_vector* mvp2 = NULL;


    check_num_args(argc, 2, 2, NULL);

    EPETE(read_matrix_vector(&mvp1, argv[ 1 ]));
    EPETE(read_matrix_vector(&mvp2, argv[ 2 ]));

    EPE(check_same_matrix_vector_dimensions(mvp1, mvp1, NULL));
    EPE(check_same_matrix_vector_dimensions(mvp2, mvp2, NULL));
    EPE(check_same_matrix_vector_dimensions(mvp1, mvp2, NULL));
    EPE(check_same_matrix_vector_dimensions(mvp2, mvp1, "test routine"));

    free_matrix_vector(mvp1);
    free_matrix_vector(mvp2);

    return EXIT_SUCCESS; 
}


