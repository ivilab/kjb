
/* $Id: m_fortran.c 21522 2017-07-22 15:14:27Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
* =========================================================================== */

#include "m/m_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "m/m_fortran.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                       get_fortran_1D_dp_array_from_matrix
 *
 *
 * Index: matrices, fortran
 *
 * -----------------------------------------------------------------------------
*/

double* get_fortran_1D_dp_array_from_matrix(const Matrix* mp)
{
    int  num_rows, num_cols;
    long num_elements;
    double* data_array;
    double*   matrix_row_pos;
    int i, j;

    
    /* TODO.
     *
     * We need to decide if mp being NULL should be considered a bug, an
     * ERROR, or simply the right thing to do. 
    */
    if (mp == NULL) return NULL; 

    /* This routine typically is used to marshall arguments for fortran
     * libraries such as LAPACK, which typically have no protection for NaN. So,
     * we can check a lot of code by putting the verification routine here. 
    */
    verify_matrix(mp, NULL);
    
    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    num_elements = num_rows * num_cols;

    NRN(data_array = DBL_MALLOC(num_elements));

    for (i=0; i<num_rows; i++)
    {
        matrix_row_pos = mp->elements[ i ];

        for(j=0; j<num_cols; j++)
        {
            data_array[ j * num_rows + i ] = *matrix_row_pos++;
        }
    }

    return data_array;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     get_matrix_from_fortran_1D_dp_array
 *
 *
 * Index: matrices, fortran
 *
 * -----------------------------------------------------------------------------
*/

int get_matrix_from_fortran_1D_dp_array
(
    Matrix**      target_mpp,
    int           num_rows,
    int           num_cols,
    const double* data_ptr
)
{
    Matrix* mp;
    int     i;
    int     j;
    double*   matrix_row_pos;


    ERE(get_target_matrix(target_mpp, num_rows, num_cols));
    mp = *target_mpp;

    for (i=0; i<num_rows; i++)
    {
        matrix_row_pos = mp->elements[ i ];

        for(j=0; j<num_cols; j++)
        {
            *matrix_row_pos++ = data_ptr[ j * num_rows + i ];
        }
    }

    /*
     * This routine typically is used to marshall arguments from the retun of
     * fortran libraries such as LAPACK. We can check a lot of code by putting
     * the verification routine here. 
    */
    verify_matrix(mp, NULL);
    
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      create_matrix_from_fortran_1D_dp_array
 *
 *
 * Index: matrices, fortran
 *
 * -----------------------------------------------------------------------------
*/

Matrix* create_matrix_from_fortran_1D_dp_array
(
    int           num_rows,
    int           num_cols,
    const double* data_ptr
)
{
    Matrix* mp;
    int     i;
    int     j;
    double*   matrix_row_pos;


    NRN(mp = create_matrix(num_rows, num_cols));

    for (i=0; i<num_rows; i++)
    {
        matrix_row_pos = mp->elements[ i ];

        for(j=0; j<num_cols; j++)
        {
            *matrix_row_pos++ = data_ptr[ j * num_rows + i ];
        }
    }

    return mp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

