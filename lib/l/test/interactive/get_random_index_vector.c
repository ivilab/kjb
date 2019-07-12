
/* $Id: int_matrix.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h" 

#define MAX_SIZE 20

int main(int argc, char **argv)
{
    int dim;
    Int_vector* row_vp = NULL;

    for (dim = 1; dim < MAX_SIZE; dim++)
    {
        EPETE(get_random_index_vector(&row_vp, dim, 0.0));
        db_irv(row_vp); 
        EPETE(get_random_index_vector(&row_vp, dim, 0.5));
        db_irv(row_vp); 
        EPETE(get_random_index_vector(&row_vp, dim, 1.0));
        db_irv(row_vp); 
        dbp("----------------------------------------------\n"); 
    }

    free_int_vector(row_vp);

    return EXIT_SUCCESS; 
}

