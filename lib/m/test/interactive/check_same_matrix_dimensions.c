
/* $Id: check_same_matrix_dimensions.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "m/m_incl.h" 

int main(int argc, char **argv)
{
    Matrix* mp1 = NULL;
    Matrix* mp2 = NULL;


    check_num_args(argc, 2, 2, NULL);

    EPETE(read_matrix(&mp1, argv[ 1 ]));
    EPETE(read_matrix(&mp2, argv[ 2 ]));

    EPE(check_same_matrix_dimensions(mp1, mp1, NULL));
    EPE(check_same_matrix_dimensions(mp2, mp2, NULL));
    EPE(check_same_matrix_dimensions(mp1, mp2, NULL));
    EPE(check_same_matrix_dimensions(mp2, mp1, "test routine"));

    free_matrix(mp1);
    free_matrix(mp2);

    return EXIT_SUCCESS; 
}


