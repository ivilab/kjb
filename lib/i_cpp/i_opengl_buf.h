/**
 * @file
 * @brief Class def. for deprecated Opengl framebuffer
 * @author Kyle Simek
 */
/*
 * $Id: i_opengl_buf.h 17790 2014-10-20 09:13:26Z predoehl $
 */

#ifndef KJB_CPP_OPENGL_IMAGE_BUF_H
#define KJB_CPP_OPENGL_IMAGE_BUF_H

#include <i_cpp/i_image.h>

namespace kjb
{

/**
 * @ingroup kjbImageProc Image Processing
 */

/**
 * Class for constructing an image from the opengl framebuffer
 * @note This method is DEPRECATED in favor of the kjb::opengl::framebuffer_to_image() function.
 * @deprecated
 *
 * Do not use this class in new code.
 */
class Opengl_framebuffer_image : public Image
{
public:
//    KJB_LEGACY(Opengl_framebuffer_image(uint32_t x, uint32_t y, uint32_t height, uint32_t width));
    Opengl_framebuffer_image(uint32_t x, uint32_t y, uint32_t height, uint32_t width);
};


} //namespace kjb

#endif /* KJB_CPP_OPENGL_IMAGE_BUF_H */
