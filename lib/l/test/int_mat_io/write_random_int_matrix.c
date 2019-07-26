
/* $Id: write_random_int_matrix.c 4723 2009-11-16 18:57:09Z kobus $ */


/*
 * Writes a random matrix in raw form. 
*/
#include "m/m_incl.h" 
#include "l/l_incl.h" 

int main(argc, argv)
    int argc;
    char *argv[];
{
    Matrix* random_mp       = NULL;
    Int_matrix* mp       = NULL;
    int     num_rows;
    int     num_cols;
    extern int kjb_debug_level;

    kjb_debug_level = 0; 

    kjb_init();
    check_num_args(argc, 2, 3, NULL); 

    EPETE(ss1i(argv[ 1 ], &num_rows));
    EPETE(ss1i(argv[ 2 ], &num_cols));

    kjb_seed_rand(100, 200); 
    EPETE(get_random_matrix(&random_mp, num_rows, num_cols)); 
    EPETE(ow_multiply_matrix_by_scalar(random_mp, 100.0));
    EPETE(copy_matrix_to_int_matrix(&mp, random_mp)); 

    EPETE(write_int_matrix(mp, (argc > 3) ? argv[ 3 ]: (char*)NULL));

    free_int_matrix(mp); 
    free_matrix(random_mp); 

    return EXIT_SUCCESS; 
}

