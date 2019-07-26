/**
 * @file
 */

/*
 * $Id: i_filter.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "i_cpp/i_filter.h"
#include "l/l_sys_lib.h"
#include "m2/m2_ncc.h"
#include "m/m_convolve.h"
#include "i/i_convolve.h"

namespace kjb {

Filter gaussian_filter(double sigma, int size)
{
    kjb_c::Matrix* mat = NULL;
    if(size % 2 == 0)
    {
        size++;
    }
    ETX(kjb_c::get_2D_gaussian_mask(&mat, size, sigma));
    return Filter(Matrix(mat));
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Filter laplacian_of_gaussian_filter(int size, double sigma)
{
    Filter K(size, size);

    for(int i = 0; i < size; i++)
    {
        for(int j = 0; j < size; j++)
        {
            double x = i - (size / 2);
            double y = j - (size / 2);
            K(i, j) = (-1 / (M_PI * pow(sigma, 4)))
                * (1 - ((x * x + y * y) / (2 * sigma * sigma)))
                * exp(-(x * x + y * y) / (2 * sigma * sigma));
        }
    }

    return K;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Image operator*(const Image& image, const Filter& kernel)
{
    kjb_c::KJB_image* temp = NULL;

#if 0
    // this has been disabled, since boundaries are handled differently and it's unclear what kernel sizes are faster
        ETX(kjb_c::fourier_convolve_image(&temp, image.c_ptr(), kernel.m_kernel.get_c_matrix()));
#else
        ETX(kjb_c::convolve_image(&temp, image.c_ptr(), kernel.m_kernel.get_c_matrix()));
#endif

    return Image(temp);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/// @brief this wraps C function kjb_c::gauss_sample_image (q.v.).
Image gauss_sample_image(const Image& in, int resolution, double sigma)
{
    kjb_c::KJB_image *out_ip = 0;
    ETX(kjb_c::gauss_sample_image(&out_ip, in.c_ptr(), resolution, sigma));
    return Image(out_ip);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix operator*(const Matrix& in, const Filter& mask)
{
    size_t nr = in.get_num_rows();
    size_t nc = in.get_num_cols();
    size_t mnr = mask.m_kernel.get_num_rows();
    size_t mnc = mask.m_kernel.get_num_cols();
    ASSERT(mnr < nr);
    ASSERT(mnc < nc);

    Matrix out(nr, nc);
    int mask_row_offset = mnr / 2;
    int mask_col_offset = mnc / 2;

    for(size_t i = 0; i < nr; i++)
    {
        for(size_t j = 0; j < nc; j++)
        {
            double sum = 0.0;
            for(size_t mi = 0; mi < mnr; mi++)
            {
                for(size_t mj = 0; mj < mnc; mj++)
                {
                    int m = i + mask_row_offset - mi;
                    int n = j + mask_col_offset - mj;

                    if (m < 0) m = -m - 1;
                    else if (m >= int(nr)) m = 2*nr - m -1;

                    if (n < 0) n = -n - 1;
                    else if (n >= int(nc)) n = 2*nc - n - 1;

                    sum += mask(mi, mj) * in(m, n);
                }
            }
            out(i, j) = sum;
        }
    }

    return out;
}

} //namespace kjb

