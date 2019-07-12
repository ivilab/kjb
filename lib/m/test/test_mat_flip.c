/*
 * $Id: test_mat_flip.c 21712 2017-08-20 18:21:41Z kobus $
 */

#include "m/m_mat_flip.h"
#include "m/m_mat_io.h"

#define SIZE 4

int main(void)
{
    int rm, row, col;
    Matrix* m = NULL;

    EPETE(get_target_matrix(&m, SIZE, SIZE));

    for (row = 0, rm = 0; row < SIZE; ++row)
    {
        for (col = 0; col < SIZE; ++col)
        {
            m -> elements[row][col] = rm++;
        }
    }
    
    if (is_interactive())
    {
        kjb_puts("Starting matrix:\n");
        fp_write_matrix(m, stdout);
    }

    EPETE(ow_horizontal_flip_matrix(m));

    if (is_interactive())
    {
        kjb_puts("Horizontally flipped matrix:\n");
        fp_write_matrix(m, stdout);
    }

    for (row = 0, rm = 0; row < SIZE; ++row)
    {
        for (col = 0; col < SIZE; ++col)
        {
            if (m -> elements[row][SIZE-col-1] != rm++)
            {
                return EXIT_BUG;
            }
        }
    }
    EPETE(ow_horizontal_flip_matrix(m)); /* reset it */

    EPETE(ow_vertical_flip_matrix(m));
    if (is_interactive())
    {
        kjb_puts("Vertically flipped matrix:\n");
        fp_write_matrix(m, stdout);
    }
    for (row = 0, rm = 0; row < SIZE; ++row)
    {
        for (col = 0; col < SIZE; ++col)
        {
            if (m -> elements[SIZE-row-1][col] != rm++)
            {
                return EXIT_BUG;
            }
        }
    }

    free_matrix(m);

    return EXIT_SUCCESS;
}
