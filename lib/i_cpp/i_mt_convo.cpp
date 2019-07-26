/**
 * @file
 * @author Andrew Predoehl
 * @brief implementation of fast (FFT-based) reentrant image convolution
 */
/*
 * $Id: i_mt_convo.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "i/i_matrix.h"
#include "i_cpp/i_mt_convo.h"

#include <algorithm>

namespace
{

// typedef of a pointer-to-member-function to select the convolution style
typedef void (kjb::Fftw_convolution_2d::* Convo_pf_t)(
    const kjb::Matrix&,
    kjb::Matrix&,
    kjb::Fftw_convolution_2d::Work_buffer
)   const;


// this takes the user's choice of zero-pad or reflect, in 'convo_type' param.
void channel_convolve(
    const kjb::Image& in,
    kjb::Image& out,
    kjb::Fftw_convolution_2d::Work_buffer buf,
    const kjb::Fftw_convolution_2d* convolver,
    Convo_pf_t convo_type
)
{
    const int CHAN_CT = kjb::Image::END_CHANNELS;

    // Extract image channels into a Matrix_vector.
    kjb_c::Matrix_vector *mvc = NULL;
    ETX(kjb_c::image_to_matrix_vector(in.c_ptr(), &mvc));
    if (0 == mvc || mvc -> length != CHAN_CT || 0 == mvc -> elements)
    {
        KJB_THROW_2(kjb::KJB_error,"Bad result from image_to_matrix_vector()");
    }

    // Plunder the Matrix_vector and wrap contents; dispose of Matrix_vector.
    // The next three lines cannot throw, so this is exception-safe.
    kjb::Matrix r_chan(mvc -> elements[kjb::Image::RED]),
                g_chan(mvc -> elements[kjb::Image::GREEN]),
                b_chan(mvc -> elements[kjb::Image::BLUE]),
                *mv[CHAN_CT];
    std::fill_n(mvc -> elements, CHAN_CT, static_cast<kjb_c::Matrix*>(0));
    kjb_c::free_matrix_vector(mvc);
    mvc = 0;
    mv[kjb::Image::RED  ] = &r_chan;
    mv[kjb::Image::GREEN] = &g_chan;
    mv[kjb::Image::BLUE ] = &b_chan;

    // do convolution
    for (int i = 0; i < CHAN_CT; ++i)
    {
        (convolver ->* convo_type)(*mv[i], *mv[i], buf);
    }

    // reassemble the channels
    kjb::Image i2(kjb::rgb_matrices_to_image(r_chan, g_chan, b_chan));
    out.swap(i2);
}

}

namespace kjb
{

/**
 * @brief reentrant convolution based on reflecting the input at borders
 * @param[in]  in    input image to be filtered/blurred/convolved
 * @param[out] out   reference to output image object to which to write results
 * @param[in]  buf   individual working memory for (this thread's) computations
 *
 * This method will emulate the behavior of kjb_c::fourier_convolve_image(),
 * i.e., it assumes the borders of the image are surrounded by zeros.
 * This is usually a poor assumption for images;
 * you might prefer to use reflect_and_convolve().
 * See @ref fft_boundary_convo.
 *
 * Note that 'buf' must not be shared across threads.  It is a smart pointer so
 * it is lightweight and safe to copy by value.
 * This method is one of the few reentrant methods of this class.  See
 * @ref fft_mt_convo.
 *
 * It is perfectly fine to make 'in' and 'out' refer to the same Image.
 */
void Fftw_image_convolution::convolve(
    const Image& in,
    Image& out,
    Fftw_convolution_2d::Work_buffer buf
)   const
{
    channel_convolve(in, out, buf, & m_convo, & Fftw_convolution_2d::convolve);
}


/**
 * @brief reentrant convolution based on reflecting the input at borders
 * @param[in]  in    input image to be filtered/blurred/convolved
 * @param[out] out   reference to output image object to which to write results
 * @param[in]  buf   individual working memory for (this thread's) computations
 *
 * This method will emulate the behavior of kjb_c::convolve_image(),
 * i.e., it assumes the borders of the image are surrounded by reflection.
 * This is a common assumption for images.
 * See @ref fft_boundary_convo.
 *
 * Note that 'buf' must not be shared across threads.  It is a smart pointer so
 * it is lightweight and safe to copy by value.
 * This method is one of the few reentrant methods of this class.  See
 * @ref fft_mt_convo.
 *
 * It is perfectly fine to make 'in' and 'out' refer to the same Image.
 */
void Fftw_image_convolution::reflect_and_convolve(
    const Image& in,
    Image& out,
    Fftw_convolution_2d::Work_buffer buf
)   const
{
    channel_convolve(in, out, buf,
                       & m_convo, & Fftw_convolution_2d::reflect_and_convolve);
}

}

