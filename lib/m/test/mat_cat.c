/*
 * Test program for the concat functions
 *
 * $Id: mat_cat.c 21491 2017-07-20 13:19:02Z kobus $
 */

#include <m/m_mat_vector.h>
#include <m/m_mat_io.h>

#define SIZE 10

int fail(int);
int fail(int line)
{
    kjb_fprintf(stderr, "failure line %d\n", line);
    return EXIT_BUG;
}

#define FAIL fail(__LINE__)

int main(void)
{
    int i, j;
    Matrix *m1 = NULL, *m2 = NULL, *m3 = NULL;
    Matrix *mh = NULL, *mv = NULL;
    const Matrix *ma[3];
    double x = 0;

    EPETE(get_target_matrix(&m1, SIZE, SIZE));
    EPETE(get_target_matrix(&m2, SIZE, SIZE));
    EPETE(get_target_matrix(&m3, SIZE, SIZE));
    ma[0] = m1;
    ma[1] = m2;
    ma[2] = m3;

    for (i = 0; i < SIZE; ++i)
    {
        for (j = 0; j < SIZE; ++j, x += 1.0)
        {
            m1 -> elements[i][j] = x + 0*SIZE*SIZE;
            m2 -> elements[i][j] = x + 1*SIZE*SIZE;
            m3 -> elements[i][j] = x + 2*SIZE*SIZE;
        }
    }

    EPETE(concat_matrices_vertically(&mv, 3, ma));
    EPETE(concat_matrices_horizontally(&mh, 3, ma));

    if (is_interactive())
    {
        kjb_puts("Vertical concatention:\n");
        EPETE(fp_write_matrix(mv, stdout));
        kjb_puts("Horizontal concatention:\n");
        EPETE(fp_write_matrix(mh, stdout));
    }

    /* floating point exact equality will work for small integers. */
    x = 0.0;
    for (i = 0; i < SIZE; ++i)
    {
        for (j = 0; j < SIZE; ++j, x += 1.0)
        {
            if (mv -> elements[i][j] != x) return FAIL;
            if (mv -> elements[i+SIZE][j] != x+SIZE*SIZE) return FAIL;
            if (mv -> elements[i+2*SIZE][j] != x+2*SIZE*SIZE) return FAIL;

            if (mh -> elements[i][j] != x) return FAIL;
            if (mh -> elements[i][j+SIZE] != x+SIZE*SIZE) return FAIL;
            if (mh -> elements[i][j+2*SIZE] != x+2*SIZE*SIZE) return FAIL;
        }
    }
    free_matrix(m1);
    free_matrix(m2);
    free_matrix(m3);
    free_matrix(mh);
    free_matrix(mv);

    return EXIT_SUCCESS;
}
