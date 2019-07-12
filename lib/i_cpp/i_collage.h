/**
 * @file
 * @brief wrapper for kjb_c function to make a collage
 */
/*
 * $Id: i_collage.h 18278 2014-11-25 01:42:10Z ksimek $
 */

#ifndef I_CPP_I_COLLAGE_H_IVI_LAB_LIBRARY
#define I_CPP_I_COLLAGE_H_IVI_LAB_LIBRARY

#include <i/i_collage.h>
namespace kjb {

class Image;

/**
 * @addtogroup kjbImageProc
 * @{
 */

/// @brief wrapper for kjb_c::make_image_collage
Image make_collage(
    Image const* const* input_images,
    int num_horizontal,
    int num_vertical
);

/// @brief also works for an array of Image objects
Image make_collage(
    Image const* input_img_array,
    int num_horizontal,
    int num_vertical
);

/// @}

}

#endif /* I_CPP_I_COLLAGE_H_IVI_LAB_LIBRARY */
