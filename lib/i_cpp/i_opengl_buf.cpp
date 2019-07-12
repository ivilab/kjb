/**
 * @file
 * @brief Def for deprecated OpenGL class
 * @author Kyle Simek
 */

/* $Id: i_opengl_buf.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "i_cpp/i_opengl_buf.h"

#include <string>
#include <iostream>

#ifdef KJB_HAVE_OPENGL
    #if defined(MAC_OSX)
        #include <OpenGL/glu.h>
        #include <GLUT/glut.h>
    #elif defined(WIN32) || defined(_WIN32)
        #include <GL/glu.h>
        #include <glut.h>
    #else
        #include <GL/glu.h>
        #include <GL/glut.h>
    #endif
#endif

namespace kjb {


/**
 * Initialize image from opengl framebuffer
 *
 * @note Should re-implement this using Joe's faster offscreen buffer code
 * @note This method is DEPRECATED in favor of the kjb::opengl::framebuffer_to_image() function.
 * */
Opengl_framebuffer_image::Opengl_framebuffer_image(uint32_t x, uint32_t y, uint32_t width, uint32_t height) :
    Image(height, width)
{
#ifdef KJB_HAVE_OPENGL
#ifdef TEST
    std::cerr << "WARNING: executing deprecated code in file " << __FILE__ << ", line " << __LINE__ << std::endl;
#endif
    float* rgb = new float[height * width * END_CHANNELS];
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(x,y,width,height,
            GL_RGB,
            GL_FLOAT,
            &rgb[0]);

    const uint32_t num_rows = height - y;
    for(uint32_t row = y; row < height; row++)
    {
        for(uint32_t col = x; col < width; col++)
        {
//            const uint32_t image_offset = col + row*width;
            const uint32_t rgb_offset = END_CHANNELS
                                        * (col + (num_rows - row - 1)*width);

            m_image->pixels[row][col].r = 255*rgb[rgb_offset];
            m_image->pixels[row][col].g = 255*rgb[rgb_offset + 1];
            m_image->pixels[row][col].b = 255*rgb[rgb_offset + 2];
        }
    }

    delete[] rgb;
#else
    KJB_THROW_2(Missing_dependency, "OpenGL");
#endif
}

}
