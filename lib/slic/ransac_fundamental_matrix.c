/* $Id: ransac_fundamental_matrix.c 21596 2017-07-30 23:33:36Z kobus $  */
#include "slic/ransac_fit.h"

int ransac_fit_fundamental_matrix
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int          max_num_tries,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
)
{
    int min_num_samples;

    min_num_samples = 8;

    return ransac_fit(x_mp, y_mp, max_num_tries, min_num_samples, good_fit_threshold, 
                      a_mpp, index_ivpp, fit_err_ptr, 
                      fit_fundamental_matrix, 
                      get_fundamental_matrix_distance, 
                      is_fundamental_matrix_degenerate);
}
