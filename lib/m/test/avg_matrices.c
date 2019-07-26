
/* $Id$ */


#include "m/m_incl.h" 

#define MAX_MATRIX_VECTOR_LEN  20
#define MAX_NUM_ROWS   500
#define MAX_NUM_COLS   500
#define BASE_NUM_TRIES 500

/*ARGSUSED*/
int main(void)
{
    Matrix_vector* mats = NULL;
    Matrix* avg_mat = NULL;
    Matrix* sdv_mat = NULL;
    Matrix* sdv_mat2 = NULL;
    double avg;
    double sdv;
    const int N = 10;
    const int m = 5;
    int i, j;

    get_target_matrix_vector(&mats, N);

    avg = 0.0;
    for(i = 0; i < N; i++)
    {
        get_initialized_matrix(&mats->elements[i], m, m, i + 1);
        avg += i + 1;
    }
    avg /= N;

    average_matrices(&avg_mat, mats);
    for(i = 0; i < m; i++)
    {
        for(j = 0; j < m; j++)
        {
            if(avg_mat->elements[i][j] != avg)
            {
                free_matrix_vector(mats);
                free_matrix(avg_mat);
                return EXIT_BUG;
            }
        }
    }

    sdv = 0.0;
    for(i = 0; i < N; i++)
    {
        sdv += (i + 1 - avg) * (i + 1 - avg);
    }
    sdv /= (N - 1);
    sdv = sqrt(sdv);

    std_dev_matrices(&sdv_mat, mats, avg_mat);
    std_dev_matrices(&sdv_mat2, mats, NULL);
    for(i = 0; i < m; i++)
    {
        for(j = 0; j < m; j++)
        {
            if(sdv_mat->elements[i][j] != sdv || sdv_mat2->elements[i][j] != sdv)
            {
                free_matrix_vector(mats);
                free_matrix(avg_mat);
                free_matrix(sdv_mat);
                free_matrix(sdv_mat2);
                return EXIT_BUG;
            }
        }
    }


    free_matrix_vector(mats);
    free_matrix(avg_mat);
    free_matrix(sdv_mat);
    free_matrix(sdv_mat2);
    return EXIT_SUCCESS;
}

