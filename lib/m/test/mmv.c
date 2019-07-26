/* $Id: mmv.c 21491 2017-07-20 13:19:02Z kobus $
 *
 * Test creation and destruction of Matrix_vector_vector and also the
 * function concat_matrices_vertically.
 */

#include <m/m_matrix.h>
#include <m/m_mat_vector.h>

int main(void)
{
    Matrix *m = NULL;
    Matrix_vector_vector *z = NULL;
    const Matrix* a[3] = {0,0,0};

    EPETE(get_target_matrix_vector_vector(&z, 17));
    EPETE(concat_matrices_vertically(&m, 3, a));
    free_matrix_vector_vector(z);
    return EXIT_SUCCESS;
}
