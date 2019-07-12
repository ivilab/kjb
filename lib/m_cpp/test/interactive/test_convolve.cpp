/* $Id: test_convolve.cpp 15114 2013-08-04 22:36:36Z predoehl $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2012 by Kobus Barnard (author)
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker


#include <m_cpp/m_convolve.h>
#include <m2/m2_ncc.h>
#include <i_cpp/i_image.h>
#include <l/l_sys_time.h>


int main(int argc, char** argv)
{
    using namespace kjb;

    Image img("../input/smile.jpg");
    Matrix mat = img.to_grayscale_matrix();
    Matrix out;
    double sigma = 5.0;
    size_t mask_size = sigma * 6 + 0.5;

    std::cout << "Initializing Fftw_convolution_2d: " << std::endl;
    kjb_c::init_real_time();
    Fftw_convolution_2d conv(
            mat.get_num_rows(),
            mat.get_num_cols(),
            mask_size,
            mask_size,
            FFTW_MEASURE);
//            FFTW_ESTIMATE);
    kjb_c::display_real_time();

    conv.set_gaussian_mask(sigma);
    conv.execute(mat, out);
    Image img_out(out);
    img_out.write("smile_blurred.jpg");

    kjb::Matrix mask(mask_size, mask_size);

    if(argc == 1)  return 0;

    // do time trials
    std::cout << "50 runs of Fftw_convolution_2d: " << std::endl;
    kjb_c::init_real_time();
    for(size_t i = 0; i < 50; ++i)
    {
        conv.execute(mat, out);
    }
    kjb_c::display_real_time();

    std::cout << "50 runs of kjb_c::fourier_convolve_matrix: " << std::endl;
    kjb_c::init_real_time();
    for(size_t i = 0; i < 50; ++i)
    {
        fourier_convolve_matrix(
                &out.get_underlying_representation_with_guilt(),
                mat.get_c_matrix(),
                mask.get_c_matrix());
    }
    kjb_c::display_real_time();


    return 0;
}
