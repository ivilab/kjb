
/* $Id: m_mat_flip.c 21712 2017-08-20 18:21:41Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2013 by members of the Interdisciplinary Visual
|  Intelligence Lab.
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT sista DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
|  Authors:  Kobus Barnard, Andrew Predoehl
* =========================================================================== */

#include <m/m_mat_flip.h>

#ifdef __cplusplus
extern "C" {
#endif

static void swap_elts(Matrix* m, int r1, int c1, int r2, int c2)
{
    double e = m -> elements[r1][c1];
    m -> elements[r1][c1] = m -> elements[r2][c2];
    m -> elements[r2][c2] = e;
}


/*
 * =============================================================================
 *                            ow_horizontal_flip_matrix
 *
 * Performs in-place reordering of the matrix's columns
 *
 * This routine alters the order of the columns, such that the entire contents
 * of the matrix are flipped left to right.  The previous first column becomes
 * the last column, and so on.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Author: Andrew Predoehl
 *
 * Documentor:
 *     Andrew Predoehl and Kobus Barnard
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
 */
int ow_horizontal_flip_matrix(Matrix* m_p)
{
    int row, col, end_col, num_rows, num_cols;

    NRE(m_p);

    num_rows = m_p -> num_rows;
    num_cols = m_p -> num_cols;

    if (num_cols < 2)
    {
        return NO_ERROR;
    }

    end_col = num_cols / 2;

    for (row = 0; row < num_rows; ++row)
    {
        int opposite_col = num_cols - 1;
        for (col = 0; col < end_col; ++col)
        {
            swap_elts(m_p, row, col, row, opposite_col);
            --opposite_col;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_vertical_flip_matrix
 *
 * Performs in-place reordering the matrix's rows
 *
 * This routine alters the order of the rows, such that the entire contents
 * of the matrix are flipped top to bottom.  The previous first row becomes
 * the last row, and so on.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Author: Andrew Predoehl
 *
 * Documentor:
 *     Andrew Predoehl and Kobus Barnard
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
 */
int ow_vertical_flip_matrix(Matrix* m_p)
{
    int row, col, num_rows, num_cols, end_row, opposite_row;

    NRE(m_p);

    num_rows = m_p -> num_rows;
    num_cols = m_p -> num_cols;

    if (num_rows < 2)
    {
        return NO_ERROR;
    }

    end_row = num_rows / 2;
    opposite_row = num_rows - 1;

    for (row = 0; row < end_row; ++row)
    {
        for (col = 0; col < num_cols; ++col)
        {
            swap_elts(m_p, row, col, opposite_row, col);
        }
        --opposite_row;
    }

    return NO_ERROR;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */


