/**
 * @file
 * @author Andrew Predoehl
 * @brief declaration of fast (FFT-based) reentrant image convolution class.
 */
/*
 * $Id: i_mt_convo.h 17393 2014-08-23 20:19:14Z predoehl $
 */

#ifndef I_CPP_I_MT_CONVO_H_INCLUDED_IVILAB
#define I_CPP_I_MT_CONVO_H_INCLUDED_IVILAB

#include <m_cpp/m_convolve.h>
#include <i_cpp/i_image.h>

namespace kjb
{


/**
 * @brief this is a simple adaptation of Fftw_convolution_2d to Image input.
 *
 * Formerly this was a child class of Fftw_convolution_2d but now I see that
 * was a poor design decision.  Their interfaces are very similar, but these
 * classes do not have a sensible IS-A relationship.
 *
 * The optimum use case for this class is when you must convolve (e.g., blur)
 * a lot of images with a single large convolutional kernel (a.k.a. mask).
 * The same mask is applied to each color channel (red, green, and blue).
 * If the mask is a Gaussian, there is a convenience function to make it
 * easier -- just tell us the sigma size.
 *
 * You can use an object of this class in a multithreaded context.  The
 * recommended way to do so is to have one thread instantiate the object and
 * set the mask.  Each worker thread needs its own work buffer, but allocation
 * of the work buffer is NOT guaranteed reentrant, so the allocations must be
 * serialized somehow.  Once these preliminaries are handled, each worker
 * thread can convolve images with the predefined mask at will, in parallel.
 * Only the following functions are promised to be reentrant in this class:
 * - get_sizes()
 * - is_mask_set()
 * - reflect_and_convolve()
 * - convolve()
 *
 * To clarify, the above paradigm is for many images, and one thread per image,
 * i.e., one thread per convolution.  That is the computing paradigm we support
 * here.  This is in contrast to another popular unsupported alternative:
 * multiple threads computing in parallel the convolution of a single image.
 * We don't do that!
 *
 * @ingroup kjbImageProc
 * @ingroup kjbThreads
 */
class Fftw_image_convolution
{
    Fftw_convolution_2d m_convo;

public:
    /// @brief please see ctor of class Fftw_convolution_2d
    Fftw_image_convolution(
        int img_num_rows,
        int img_num_cols,
        int mask_max_rows,
        int mask_max_cols,
        int fft_alg_type = FFTW_MEASURE // Macro defined in m_convolve.h
    )
    :   m_convo(img_num_rows, img_num_cols,
                                mask_max_rows, mask_max_cols, fft_alg_type)
    {}


    /// @brief set mask, which must fit with mask size maxima given to ctor
    void set_mask(const Matrix& m)
    {
        m_convo.set_mask(m);    // simple forwarding function
    }

    /// @brief set mask to a circular gaussian kernel of given sigma (pixels)
    void set_gaussian_mask(double sigma)
    {
        m_convo.set_gaussian_mask(sigma);    // simple forwarding function
    }

    Fftw_convolution_2d::Work_buffer allocate_work_buffer() const
    {
        return m_convo.allocate_work_buffer();    // simple forwarding function
    }


/*  ABOVE THE LINE:  NOT THREAD SAFE
 * -----------------------------------------------------------------
 *  BELOW THE LINE:  THREAD SAFE
 */

    /// @brief read access to the sizes specified at ctor time
    const Fftw_convolution_2d::Sizes& get_sizes() const
    {
        return m_convo.get_sizes();
    }

    /// @brief read access of the flag indicating whether the mask has been set
    bool is_mask_set() const
    {
        return m_convo.is_mask_set();
    }

    void reflect_and_convolve(
        const Image&,
        Image&,
        Fftw_convolution_2d::Work_buffer
    ) const;

    void convolve(
        const Image&,
        Image&,
        Fftw_convolution_2d::Work_buffer
    ) const;
};


}

#endif /* I_CPP_I_MT_CONVO_H_INCLUDED_IVILAB */
