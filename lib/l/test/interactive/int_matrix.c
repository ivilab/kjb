
/* $Id: int_matrix.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h" 

#define MAX_SIZE 7

int main(int argc, char **argv)
{
    int dim, r, c;
    int num_rows;
    int num_cols;
    Int_vector* row_vp = NULL;
    Int_vector* col_vp = NULL;
    Int_matrix* mp = NULL;

    for (dim = 1; dim < MAX_SIZE; dim++)
    {
        if (IS_ODD(dim)) 
        {
            num_cols = dim;
            num_rows = dim + 3;
        }
        else if (IS_EVEN(dim/2))
        {
            num_cols = dim + 5; 
            num_rows = dim;
        }
        else 
        {
            num_cols = dim; 
            num_rows = dim;
        }

        EPETE(get_initialized_int_vector(&row_vp, num_cols, 5));
        EPETE(get_initialized_int_vector(&col_vp, num_rows, 10));

        EPETE(get_initialized_int_matrix(&mp, num_rows, num_cols, -9)); 

        dbi_mat(mp);
        for (r = 0; r < num_rows; r++)
        {
            EPETE(put_int_matrix_row(mp, row_vp, r)); 
            dbi_mat(mp);
        }

        EPETE(get_initialized_int_matrix(&mp, num_rows, num_cols, -9)); 

        dbi_mat(mp);
        for (c = 0; c < num_cols; c++)
        {
            EPETE(put_int_matrix_col(mp, col_vp, c)); 
            dbi_mat(mp);
        }
    }

    free_int_vector(col_vp);
    free_int_vector(row_vp);
    free_int_matrix(mp); 

    return EXIT_SUCCESS; 
}

