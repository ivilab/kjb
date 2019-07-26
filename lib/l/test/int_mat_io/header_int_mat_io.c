
/* $Id: header_int_mat_io.c 4723 2009-11-16 18:57:09Z kobus $ */


/*
 * Writes a random matrix in raw form. 
*/
#include "l/l_incl.h" 

int main(argc, argv)
    int argc;
    char *argv[];
{
    Int_matrix* mp       = NULL;
    int     num_rows;
    int     num_cols;

    kjb_init();
    check_num_args(argc, 1, 2, NULL); 

    EPETE(read_int_matrix(&mp, argv[ 1 ]));
    EPETE(write_int_matrix_with_header(mp, (argc > 2) ? argv[ 2 ] : (char*)NULL));

    free_int_matrix(mp); 

    return EXIT_SUCCESS; 
}

