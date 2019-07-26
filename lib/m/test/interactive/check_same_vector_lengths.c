
/* $Id: check_same_vector_lengths.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "m/m_incl.h" 

int main(int argc, char **argv)
{
    Vector* vp1 = NULL;
    Vector* vp2 = NULL;


    check_num_args(argc, 2, 2, NULL);

    EPETE(read_vector(&vp1, argv[ 1 ]));
    EPETE(read_vector(&vp2, argv[ 2 ]));

    EPE(check_same_vector_lengths(vp1, vp1, NULL));
    EPE(check_same_vector_lengths(vp2, vp2, NULL));
    EPE(check_same_vector_lengths(vp1, vp2, NULL));
    EPE(check_same_vector_lengths(vp2, vp1, "test routine"));

    free_vector(vp1);
    free_vector(vp2);

    return EXIT_SUCCESS; 
}


