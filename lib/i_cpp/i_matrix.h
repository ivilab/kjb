/**
 * @file
 * @brief declaration of cpp wrapper for high-constract conversion fun
 * @author Andrew Predoehl
 */
/*
 * $Id: i_matrix.h 16764 2014-05-09 23:17:12Z ksimek $
 */

#ifndef I_CPP_I_MATRIX_H_IVI_LAB_INCLUDED
#define I_CPP_I_MATRIX_H_IVI_LAB_INCLUDED

#include <i/i_matrix.h>
#include <m_cpp/m_matrix.h>
#include <i_cpp/i_image.h>
#include <i_cpp/i_colormap.h>

namespace kjb {

/**
 * @addtogroup kjbImageProc
 * @{
 */

Image matrix_to_max_contrast_8bit_bw_image(const kjb::Matrix&);

/**
 * @brief Converts a matrix to a high-contrast color image, mapping values to
 *        colors using a kjb::Colormap.  Equivalent to Matlab's imagesc() function.
 */
Image matrix_to_color_image(const kjb::Matrix& m, const kjb::Colormap& map = "jet");

/** 
 * @brief Ported version of Matlab's "imagesc()" function; alias of 
 *        matrix_to_color_image */
inline Image imagesc(const kjb::Matrix& data, const kjb::Colormap& map = "jet")
{ 
    return matrix_to_color_image(data, map);
}

/// @}

}

#endif

