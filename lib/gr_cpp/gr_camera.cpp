/**
 * This work is licensed under a Creative Commons
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 *
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 *
 * You are free:
 *
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 *
 * Under the following conditions:
 *
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 *
 *    Noncommercial. You may not use this work for commercial purposes.
 *
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 *
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 *
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 *
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
| Authors:
|     Luca Del Pero, Kyle Simek
|
* =========================================================================== */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include <typeinfo>
#include <cstring>
#include <cmath>
/*
#include <cassert>
*/

#include <i/i_float.h>
#include <i_cpp/i_image.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_headers.h>
#include <gr_cpp/gr_camera.h>
#include <l/l_sys_mal.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <i/i_float_io.h>
#include <n/n_qr.h>
#include <l/l_bits.h>

#include <l_cpp/l_exception.h>
#include <l_cpp/l_readable.h>
#include <l_cpp/l_int_matrix.h>
#include <gr_cpp/gr_polygon.h>


#warning "[Code police] Please don't start identifiers with underscore."
#warning "[Code police] (multiple places in this file)"

using namespace kjb_c;
using namespace kjb;
using namespace kjb::opengl;

#define KJB_DEG_TO_RAD 0.01745329251994
#define KJB_RAD_TO_DEG 57.29577951308232



/**
 * @param inear Distance of the near clipping plane from the camera
 * @param ifar Distance of the far clipping plane from the camera
 */
Base_gl_interface::Base_gl_interface(double inear, double ifar)
{
    using namespace kjb;
    if(inear <= 0)
    {
        KJB_THROW_2(kjb::Illegal_argument, "Negative near clipping plane");
    }

    if(inear >= ifar)
    {
        KJB_THROW_2(kjb::Illegal_argument, "Near clipping plane is farther than far clipping plane");
    }

    near = inear;
    far = ifar;
    modelview_matrix = create_identity_matrix(4);
    projection_matrix = create_identity_matrix(4);
    projection_matrix(3,3) = 0.0;

    /** See class description for details on the following */
    projection_matrix(3,2) = -1.0;
    projection_matrix(2,2) = near + far;
    projection_matrix(2,3) = near*far;

}

/**
 * @param mv input modelview matrix
 * @param pj input projection matrix
 * @param inear Distance of the near clipping plane from the camera
 * @param ifar Distance of the far clipping plane from the camera
 */
Base_gl_interface::Base_gl_interface(const Matrix & mv, const Matrix & pj, double inear, double ifar)
{
    if( (mv.get_num_rows() != 4 )   || (mv.get_num_cols() != 4) )
    {
        KJB_THROW_2(kjb::Illegal_argument, "Wrong dimensions of gl_modelview_matrix");
    }

    if( (pj.get_num_rows() != 4 )   || (pj.get_num_cols() != 4) )
    {
        KJB_THROW_2(kjb::Illegal_argument, "Wrong dimensions of gl_projection_matrix");
    }

    if(inear <= 0)
    {
        KJB_THROW_2(kjb::Illegal_argument, "Negative near clipping plane");
    }

    if(inear >= ifar)
    {
        KJB_THROW_2(kjb::Illegal_argument, "Near clipping plane is farther than far clipping plane");
    }

    /** We check that input planes and input projection matrix are consistent
     * See class description for further details
     */
    double epsilon = 0.00000001;
    if( fabs(pj(2,2) - (near + far)) > epsilon )
    {
        KJB_THROW_2(kjb::Illegal_argument, "Near and far clipping planes are not consistent with projection matrix");
    }

    if( fabs(pj(2,3) - (near*far)) > epsilon )
    {
        KJB_THROW_2(kjb::Illegal_argument, "Near and far clipping planes are not consistent with projection matrix");
    }

    near = inear;
    far = ifar;
    modelview_matrix = mv;
    projection_matrix = pj;
}

/**
 * @brief bgl the Base_gl_interface to copy into this one
 */
Base_gl_interface::Base_gl_interface(const Base_gl_interface & bgl)
: modelview_matrix(bgl.modelview_matrix), projection_matrix(bgl.projection_matrix)
{
    near = bgl.near;
    far = bgl.far;
}

/**
 * @brief bgl the Base_gl_interface to assign to this one
 */
Base_gl_interface& Base_gl_interface::operator=(const Base_gl_interface& bgl)
{
    near = bgl.near;
    far = bgl.far;
    modelview_matrix = bgl.modelview_matrix;
    projection_matrix = bgl.projection_matrix;
    return *this;
}

/**
 * @param inear Distance of the near clipping plane from the camera
 */
void Base_gl_interface::set_near_clipping_plane(double inear)
{
    if(inear <= 0)
    {
        KJB_THROW_2(kjb::Illegal_argument, "Negative near clipping plane");
    }

    if(inear >= far)
    {
        KJB_THROW_2(kjb::Illegal_argument, "Near clipping plane is farther than far clipping plane");
    }

    near = inear;

    /** See class description for details on the following */
    projection_matrix(2,2) = near + far;
    projection_matrix(2,3) = near*far;
}

/**
 * @param ifar Distance of the far clipping plane from the camera
 */
void Base_gl_interface::set_far_clipping_plane(double ifar) 
{
    if(near >= ifar)
    {
        KJB_THROW_2(kjb::Illegal_argument, "Near clipping plane is farther than far clipping plane");
    }

    far = ifar;

    /** See class description for details on the following */
    projection_matrix(2,2) = near + far;
    projection_matrix(2,3) = near*far;
}

/**
 * @param inear Distance of the near clipping plane from the camera
 * @param far Distance of the far clipping plane from the camera
 */
void Base_gl_interface::set_clipping_planes(double inear, double ifar) 
{
    if(inear <= 0)
    {
        KJB_THROW_2(kjb::Illegal_argument, "Negative near clipping plane");
    }

    if(inear >= ifar)
    {
        KJB_THROW_2(kjb::Illegal_argument, "Near clipping plane is farther than far clipping plane");
    }

    near = inear;
    far = ifar;

    /** See class description for details on the following */
    projection_matrix(2,2) = near + far;
    projection_matrix(2,3) = near*far;
}

kjb::Matrix Base_gl_interface::get_gl_projection_matrix() const 
{
    // previous implmentation didn't pre-multiply the matrix by glortho.
    // New implementation returns exactly what is passed to opengl
    double width, height;
    get_gl_viewport_size(&width, &height);

    // this converts from screen coordinates to normalized device coordinates
    kjb::Matrix state = create_identity_matrix(4);

    kjb::opengl::glOrtho(state,
            -width/2,
            width/2,
            -height/2,
            height/2,
            near,
            far);

    return state *= projection_matrix;
}

/** @return The aspect ration of width to height for the GL viewport. */
double Base_gl_interface::get_gl_viewport_aspect()
{
#ifdef KJB_HAVE_OPENGL
    GLdouble viewport[4] = {0};

    glGetDoublev(GL_VIEWPORT, viewport);
    return viewport[2] / viewport[3];
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}


/** @return The viewport width in the GL. */
double Base_gl_interface::get_gl_viewport_width()
{
#ifdef KJB_HAVE_OPENGL
    GLdouble viewport[4] = {0};

    glGetDoublev(GL_VIEWPORT, viewport);
    return viewport[2];
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}


/** @return The viewport height in the GL. */
double Base_gl_interface::get_gl_viewport_height()
{
#ifdef KJB_HAVE_OPENGL
    GLdouble viewport[4] = {0};

    glGetDoublev(GL_VIEWPORT, viewport);
    return viewport[3];
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

/** @return The viewport height in the GL. */
void Base_gl_interface::get_gl_viewport_size(double *w, double *h)
{
#ifdef KJB_HAVE_OPENGL
    GLdouble viewport[4] = {0};

    glGetDoublev(GL_VIEWPORT, viewport);
    *w= viewport[2];
    *h = viewport[3];
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

/**
 * @param x it will contain the x parameter of the gl viewport
 * @param y it will contain the y parameter of the gl viewport
 * @param w it will contain the width of the gl viewport
 * @param h it will contain the height of the gl viewport
 */
void Base_gl_interface::get_gl_viewport(double * x, double * y, double * w, double *h)
{
#ifdef KJB_HAVE_OPENGL
    GLdouble viewport[4] = {0};

    glGetDoublev(GL_VIEWPORT, viewport);
    *x = viewport[0];
    *y = viewport[1];
    *w= viewport[2];
    *h = viewport[3];
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

/**
 * @param w the new gl viewport size
 * @param h the new gl viewport height
 */
void Base_gl_interface::set_gl_viewport_size(double w, double h)
{
#ifdef KJB_HAVE_OPENGL
    GLdouble viewport[4] = {0};

    glGetDoublev(GL_VIEWPORT, viewport);
    glViewport(viewport[0], viewport[1], w, h);
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

/**
 * @param x the new x parameter for the gl viewport
 * @param y the new y parameter for the gl viewport
 * @param w the new gl viewport size
 * @param h the new gl viewport height
 */
void Base_gl_interface::set_gl_viewport(double x, double y, double w, double h)
{
#ifdef KJB_HAVE_OPENGL
    glViewport( (GLint)x, (GLint)y, (GLsizei) w, (GLsizei) h);
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

/**
 * @param  img_out  Result parameter. If @em *img_out is 0, an image is
 *                  allocated; otherwise its space is re-used.
 *
 * @throw  kjb::IO_error
 */
void Base_gl_interface::capture_gl_view(kjb_c::KJB_image** img_out)
{
#ifdef KJB_HAVE_OPENGL
    using namespace kjb_c;

    unsigned int num_cols = (unsigned int)get_gl_viewport_width();
    unsigned int num_rows = (unsigned int)get_gl_viewport_height();

    float* img_buf = (float*)kjb_malloc(3*num_cols*num_rows*sizeof(float));
    ASSERT(img_buf);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glReadPixels(0, 0, num_cols, num_rows, GL_RGB, GL_FLOAT, img_buf);

    //(*img_out) = kjb_create_image(num_rows, num_cols);
    if (kjb_c::get_initialized_image_2(img_out, num_rows, num_cols, 0, 0, 0 ) != kjb_c::NO_ERROR)
    {
        throw KJB_error("Could not allocate memory to read content of the GL buffer");
    }

    KJB_image * img = *img_out;

    /** In the OpenGL coordinate system, (0,0) is the bottom left corner
     * of the buffer. In KJB_images, (0,0) is the top left corner.
     * Therefore we have to swap the rows to store the gl buffer in an
     * image correctly
     */
    for (uint32_t row = 0; row < num_rows; row++)
    {
        for (uint32_t col = 0; col < 3*num_cols; col += 3)
        {
            uint32_t r = num_rows - row - 1;

            float rr = img_buf[ col+0 + row*3*num_cols ];
            float gg = img_buf[ col+1 + row*3*num_cols ];
            float bb = img_buf[ col+2 + row*3*num_cols ];

            img->pixels[ r ][ col/3 ].r = rr*255;
            img->pixels[ r ][ col/3 ].g = gg*255;
            img->pixels[ r ][ col/3 ].b = bb*255;
        }
    }

    kjb_free(img_buf);
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

/**
 * Captures the gl view and stores it into a c++ image
 *
 * @param  img_out  Result parameter. The content of the GL
 * frame buffer will be copied into this image, freeing the
 * previous memory area pointed by this image
 *
 * @throw  kjb::IO_error
 */
void Base_gl_interface::capture_gl_view(Image & img_out)
{
#ifdef KJB_HAVE_OPENGL
    using namespace kjb_c;

    KJB_image * img = 0;
    unsigned int num_cols = (unsigned int)get_gl_viewport_width();
    unsigned int num_rows = (unsigned int)get_gl_viewport_height();

    float* img_buf = (float*)kjb_malloc(3*num_cols*num_rows*sizeof(float));
    ASSERT(img_buf);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glReadPixels(0, 0, num_cols, num_rows, GL_RGB, GL_FLOAT, img_buf);

    if (kjb_c::get_initialized_image_2(&img, num_rows, num_cols, 0, 0, 0 ) != kjb_c::NO_ERROR)
    {
        throw KJB_error("Could not allocate memory to read content of the GL buffer");
    }

    /** In the OpenGL coordinate system, (0,0) is the bottom left corner
     * of the buffer. In KJB_images, (0,0) is the top left corner.
     * Therefore we have to swap the rows to store the gl buffer in an
     * image correctly
     */
    for (uint32_t row = 0; row < num_rows; row++)
    {
        for (uint32_t col = 0; col < 3*num_cols; col += 3)
        {
            uint32_t r = num_rows - row - 1;

            float rr = img_buf[ col+0 + row*3*num_cols ];
            float gg = img_buf[ col+1 + row*3*num_cols ];
            float bb = img_buf[ col+2 + row*3*num_cols ];

            img->pixels[ r ][ col/3 ].r = rr*255;
            img->pixels[ r ][ col/3 ].g = gg*255;
            img->pixels[ r ][ col/3 ].b = bb*255;
        }
    }

    kjb_free(img_buf);
    img_out.set_c_ptr(img);
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

/**
 * @param  matrix  Result parameter. The gl view will be stored
 *                 into this int matrix. The four bytes used by GL to
 *                 represent the four channels (R,G,B,A) are packaged into
 *                 a single integer.
 *
 * @throw  kjb::IO_error
 */
void Base_gl_interface::capture_gl_view(kjb::Int_matrix & matrix)
{
#ifdef KJB_HAVE_OPENGL
    using namespace kjb_c;

    int num_cols = get_gl_viewport_width();
    int num_rows = get_gl_viewport_height();

    int row, col;
    int bot, top;

    ASSERT(sizeof(unsigned int) == sizeof(int));

    if( (matrix.get_num_rows() != num_rows ) || (matrix.get_num_cols() != num_cols ) )
    {
        matrix.zero_out(num_rows, num_cols);
    }

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, num_cols, num_rows, GL_RGBA, GL_UNSIGNED_BYTE,
            *(matrix.ptr_to_storage_area()) );

    /** In the OpenGL coordinate system, (0,0) is the bottom left corner
     *  of the buffer. In KJB_images, (0,0) is the top left corner.
     *  Therefore we have to swap the rows to store the gl buffer in an
     *  image correctly
     */
    for (row = 0; row < num_rows/2; row++)
    {
        for (col = 0; col < num_cols; col++)
        {
            top = matrix(row, col);
            bot = matrix(num_rows - row - 1, col);

            if (top != 0 || bot != 0)
            {
                matrix(row, col) = bot;
                matrix( num_rows - row - 1, col) = top;
            }
        }
    }

#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

/** Constructs an image from an int matrix. Each integer consists of four bytes, and
 *  each of them is interpreted as one of the GL channels (RGBA, or ABGR if the architecture
 *  is little endian). This is MOSTLY (if not only) used for DEBUGGING
 *
 *  @param im the image to be initialized from the matrix
 *  @param m  the matrix to create the image from
 */
void Base_gl_interface::construct_image_from_int_matrix(kjb::Image & im, kjb::Int_matrix & m)
{
    using namespace kjb_c;
    im = Image::create_zero_image(m.get_num_rows(), m.get_num_cols());
    kjb_c::KJB_image * cimg = im.non_const_c_ptr();
    for(int i = 0; i < m.get_num_rows(); i++)
    {
        for(int j = 0; j < m.get_num_cols(); j++)
        {
            int _value = m(i,j);
            if(! kjb_is_bigendian())
            {
                bswap_u32((uint32_t *) &(_value));
            }
            unsigned int r = (_value >> 24&0XFF);
            unsigned int g = (_value >> 16&0XFF);
            unsigned int b = (_value >> 8&0XFF);
            cimg->pixels[ i ][ j ].r = (float)r;
            cimg->pixels[ i ][ j ].g = (float)g;
            cimg->pixels[ i ][ j ].b = (float)b;
        }
    }
}


/**
 * @param  fname  File name to write the captured view image to.
 *
 * @throw  kjb::IO_error
 */
void Base_gl_interface::capture_gl_view(const char* fname) 
{
    using namespace kjb_c;
    KJB_image * img = 0;
    capture_gl_view(&img);

    if ((kjb_write_image(img, fname)) != kjb_c::NO_ERROR)
    {
        KJB_THROW_2(IO_error, "Could not write image");
    }

    kjb_free_image(img);
}


/**
 * An example for @em fname_fmt is
 *
 * @code image-%04d.tiff @endcode
 *
 * @param  fname_fmt   Format for the file name with one printf style integer
 *                     conversion for the file number.
 * @param  N           Number of the captured image.
 *
 * @throw  kjb::IO_error
 */
void Base_gl_interface::capture_gl_view(const char* fname_fmt, uint32_t N)
{
    char fname[256] = {0};

    snprintf(fname, 255, fname_fmt, N);

    capture_gl_view(fname);
}

/**
 * Put the content of the input image in the GL frame buffer
 *
 * @param  img_in  The input image
 *
 */
void Base_gl_interface::set_gl_view(const kjb::Image & img_in)
{
#ifdef KJB_HAVE_OPENGL
    glDisable(GL_DEPTH_TEST);
    int num_cols = get_gl_viewport_width();
    int num_rows = get_gl_viewport_height();

    float* img_buf = (float*)kjb_malloc(3*num_cols*num_rows*sizeof(float));
    if(!img_buf)
    {
        throw KJB_error("Could not allocate buffer to store image in GL frame buffer");
    }

    if( (num_cols != img_in.get_num_cols() ) || ( num_rows != img_in.get_num_rows() ) )
    {
        throw KJB_error("Input image is smaller than viewport size");
    }

    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    const kjb_c::KJB_image * cimg = img_in.c_ptr();

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < 3*num_cols; col += 3)
        {
            int r = num_rows - row - 1;

            img_buf[ col+0 + row*3*num_cols ] = (cimg->pixels[ r ][ col/3 ].r)/255.0;
            img_buf[ col+1 + row*3*num_cols ] = (cimg->pixels[ r ][ col/3 ].g)/255.0;
            img_buf[ col+2 + row*3*num_cols ] = (cimg->pixels[ r ][ col/3 ].b)/255.0;
        }
    }

    glDrawPixels(num_cols, num_rows, GL_RGB, GL_FLOAT, img_buf);

    kjb_free(img_buf);
    glEnable(GL_DEPTH_TEST);
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

/**
 * Sets the gl projection matrix using the current camera parameters.
 * For the details on math, please see the class description
 */
void Base_gl_interface::set_gl_projection() const
{
#ifdef KJB_HAVE_OPENGL
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    mult_gl_projection();
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif

}

void Base_gl_interface::mult_gl_projection() const
{
#ifdef KJB_HAVE_OPENGL
    double width, height;
    get_gl_viewport_size(&width, &height);

    // this converts from screen coordinates to normalized device coordinates
    glOrtho(-width/2, width/2, -height/2, height/2, near, far);

    // this converts from camera coordinates to screen coordinates.
    glMultMatrix(projection_matrix);
    glMatrixMode(GL_MODELVIEW);
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

/**
 * Sets the gl modelview matrix using the current camera parameters.
 */
void Base_gl_interface::set_gl_modelview() const
{
#ifdef KJB_HAVE_OPENGL
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrix(modelview_matrix);
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

/**
 *  Prepares the opengl for rendering by setting the gl modelview and projection
 *  matrix based on the current camera parameters
 *  It also clears the depth and the color buffers, and sets the background color
 *  to black
 *
 *  @param clean_buffers if true, the color and the depth buffer will be cleared
 */
void Base_gl_interface::prepare_for_rendering(bool clean_buffers) const
{
#ifdef KJB_HAVE_OPENGL
    if(clean_buffers)
    {
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    set_gl_modelview();
    set_gl_projection();
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}
/**
 * Scales the gl modelview matrix using glScaled.
 *
 * @param xscale the scale along the x axis
 * @param yscale the scale along the y axis
 * @param zscale the scale along the z axis
 */
void Base_gl_interface::scale_modelview(double xscale, double yscale, double zscale)
{
    if((xscale <= 0.0) || (yscale <= 0.0) || (zscale <= 0.0))
    {
        KJB_THROW_2(kjb::Illegal_argument, "Scale modelview, scaling factors must be all positive");
    }
#ifdef KJB_HAVE_OPENGL
    glMatrixMode(GL_MODELVIEW);
    glScaled(xscale, yscale, zscale);
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

void Base_gl_interface::project_point(double & x, double & y, double &z, const kjb::Vector & point3D, double img_height) const
{
#ifdef KJB_HAVE_OPENGL
    GLdouble mv[16];
    GLdouble pj[16];
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT,vp);
    glGetDoublev(GL_MODELVIEW_MATRIX, mv);
    glGetDoublev(GL_PROJECTION_MATRIX, pj);

    if(point3D.size() == 3)
    {
        gluProject
        (
            (GLdouble) point3D(0),
            (GLdouble) point3D(1),
            (GLdouble) point3D(2),
            mv,
            pj,
            vp,
            &x,
            &y,
            &z
       );
    }
    else if(point3D.size() == 4)
    {
        gluProject
        (
            (GLdouble) (point3D(0)/point3D(3)),
            (GLdouble) (point3D(1)/point3D(3)),
            (GLdouble) (point3D(2)/point3D(3)),
            mv,
            pj,
            vp,
            &x,
            &y,
            &z
       );
    }
    else
    {
        KJB_THROW_2(Illegal_argument, "Point 3D vector size should be 3 or 4 (if homogeneous)");
    }
#endif
    y = img_height - y;
}

/**
 * Checks whether a polygon is visible under the current camera by
 * checking that the dot product between the polygon normal
 * and the normalized vector between the camera and the polygon
 * center is bigger than epsilon (whose default value is zero)
 *
 * @param p the polygon to test
 * @param epsilon the constant to compare the dot product with (default = 0)
 */
bool Base_gl_interface::Polygon_visibility_test(const kjb::Polygon & p, double epsilon) const 
{
    const kjb::Vector & centroid = p.get_centroid();
    const kjb::Vector & normal = p.get_normal();

    double x = modelview_matrix(0,3) - centroid(0);
    double y = modelview_matrix(1,3) - centroid(1);
    double z = modelview_matrix(2,3) - centroid(2);

    /** Should we normalize here if we weant to use epsilon ?" */
    if((x*normal(0) + y*normal(1) - z*normal(2)) > epsilon)
        return true;

    return false;
}

/**
 * @param inear Distance of the near clipping plane from the camera
 * @param far Distance of the far clipping plane from the camera
 */
Parametric_camera_gl_interface::Parametric_camera_gl_interface(double inear, double ifar) 
: Base_gl_interface(inear,ifar), Rigid_object(), camera_center_in_world_coordinates(4, 0.0)
{
    camera_center_in_world_coordinates(3) = 1.0;
}

/**
 * @param pcgi the parametric camera to copy into this one
 */
Parametric_camera_gl_interface::Parametric_camera_gl_interface(const Parametric_camera_gl_interface &pcgi)
: Base_gl_interface(pcgi), Rigid_object(pcgi),
  camera_center_in_world_coordinates(pcgi.camera_center_in_world_coordinates)
{

}

/**
 * @param pcgi the parametric camera to assign to this one
 */
Parametric_camera_gl_interface& Parametric_camera_gl_interface::operator=(const Parametric_camera_gl_interface& pcgi)
{
    Base_gl_interface::operator=(pcgi);
    Rigid_object::operator=(pcgi);
    camera_center_in_world_coordinates = pcgi.camera_center_in_world_coordinates;
    return *this;
}

/**
 * Virtual copy constructor
 */
Parametric_camera_gl_interface* Parametric_camera_gl_interface::clone() const
{
    return new Parametric_camera_gl_interface(*this);
}

/**
 * @param px the new x coordinate of the principal point
 */
void Parametric_camera_gl_interface::set_principal_point_x(double px)
{
    set_projection_entry(0,2,-px);
}

/**
 * @param py the new y coordinate of the principal point
 */
void Parametric_camera_gl_interface::set_principal_point_y(double py)
{
    set_projection_entry(1,2,-py);
}

/**
 * @param focal_length the new focal length
 * @param skew the new skew
 * @param aspect_ratio the new aspect_ratio
 *
 * Skew and aspect ratio are necessary since a change in the focal length
 * impacts entries in the matrix that depend on skew and aspect ratio as well
 */
void Parametric_camera_gl_interface::set_focal_length(double focal_length, double skew, double aspect_ratio)
{
    set_projection_entry(0,0, focal_length*aspect_ratio);
    set_projection_entry(1,1, (focal_length)/sin(skew));
    set_projection_entry(0,1, -(focal_length)/tan(skew));
}

/**
 * @param aspect_ratio the new aspect_ratio
 * @param focal_length the new focal length
 * @param skew the new skew
 *
 * Skew and focal length are necessary since a change in the aspect ratio
 * impacts entries in the matrix that depend on skew and focal length as well
 */
void Parametric_camera_gl_interface::set_aspect_ratio(double aspect_ratio, double focal_length, double /* skew */)
{
    set_projection_entry(0,0, (focal_length*aspect_ratio)) ;
}

/**
 * @param skew the new skew
 * @param aspect_ratio the new aspect_ratio
 * @param focal_length the new focal length
 *
 * Skew and aspect ratio are necessary since a change in the skew
 * impacts entries in the matrix that depend on focal length and aspect ratio as well
 */
void Parametric_camera_gl_interface::set_skew(double skew, double /* aspect_ratio */, double focal_length)
{
    set_projection_entry(1,1,(focal_length)/sin(skew));
    set_projection_entry(0,1, -(focal_length)/tan(skew));
}

/**
 * This function assumes no skew
 *
 * @param aspect_ratio the new aspect_ratio
 * @param focal_length the new focal length
 *
 * Aspect ratio is necessary since a change in the skew
 * impacts entries in the matrix that depend on aspect ratio as well
 */
void Parametric_camera_gl_interface::set_focal_no_skew(double focal_length, double aspect_ratio)
{
    set_projection_entry(0,0, focal_length*aspect_ratio);
    set_projection_entry(1,1, focal_length);
    set_projection_entry(0,1, 0.0);
}

/**
 * This function assumes aspect_ratio=1
 *
 * @param focal_length the new focal length
 * @param skew the new skew
 *
 * Aspect ratio is necessary since a change in the focal length
 * impacts entries in the matrix that depend on skew as well
 */
void Parametric_camera_gl_interface::set_focal_no_aspect_ratio(double focal_length, double skew)
{
    set_projection_entry(0,0, focal_length);
    set_projection_entry(1,1, focal_length/sin(skew));
    set_projection_entry(0,1, -(focal_length)/tan(skew));
}

/**
 * This funcation assumes no skew and aspect_ration = 1
 *
 * @param focal_length The new focal length
 */
void Parametric_camera_gl_interface::set_focal_no_aspect_ratio_no_skew(double focal_length)
{
    set_projection_entry(0,0, focal_length);
    set_projection_entry(1,1, focal_length);
    set_projection_entry(0,1,0.0);
}

/*
 * @param focal_length the new focal length
 * @param skew the new skew
 * @param aspect_ratio the new aspect_ratio
 * @param px the new x coordinate of the principal point
 * @param py the new y coordinate of the principal point
 */
void Parametric_camera_gl_interface::set_intrinsic_parameters(double focal_length, double aspect_ratio, double skew, double px, double py)
{
    set_focal_length(focal_length, skew, aspect_ratio);
    set_principal_point_x(px);
    set_principal_point_y(py);
}

/*
 * @param center the new camera center (optionally in homogeneous coordinates)
 */
void Parametric_camera_gl_interface::set_camera_center(const kjb::Vector & center) 
{

    if(center.size() == 4)
    {
        for(unsigned int i = 0; i < 3; i++)
        {
            camera_center_in_world_coordinates(i) = center(i)/center(3);
        }
        camera_center_in_world_coordinates(3) = 1.0;
    }
    else if(center.size() == 3)
    {
        for(unsigned int i = 0; i < 3; i++)
        {
            camera_center_in_world_coordinates(i) = center(i);
        }
    }
    else
    {
        KJB_THROW_2(KJB_error, "Camera center vector has wrong dimensions");
    }

    /*
     * In the classic Forsyth and Ponce formulation, the extrinsic camera matrix
     * is 4X4 defined as [R -c*R] where R is a rotation matrix and c is the camera center
     */
    Vector camera_center_in_camera_coordinates = get_rotations()*camera_center_in_world_coordinates;
    for(unsigned int i = 0; i < 3; i++)
    {
        set_modelview_entry(i,3,-camera_center_in_camera_coordinates(i));
    }

}


/** @brief Sets the world origin in camera coordinates (the "t" vector in Forsyth) */
void Parametric_camera_gl_interface::set_world_origin(const kjb::Vector & t)
{
    Vector t_tmp = t;

    if (t_tmp.size() == 4)
    {
        t_tmp /= t_tmp(3);
        t_tmp.resize(3);
    }
    else if (t_tmp.size() == 3)
    {
        t_tmp.resize(4);
        t_tmp[3] = 1.0;
    }

    if (t_tmp.size() != 4)
    {
        KJB_THROW_2(KJB_error, "Camera center vector has wrong dimensions");
    }

    set_modelview_entry(0,3,t_tmp(0));
    set_modelview_entry(1,3,t_tmp(1));
    set_modelview_entry(2,3,t_tmp(2));

    camera_center_in_world_coordinates = get_rotations().transpose() * -t_tmp;
}

/*
 * @param rotation_matrix the new rotation_matrix
 */
void Parametric_camera_gl_interface::set_rotation_matrix(const kjb::Matrix & rotation_matrix) 
{

    if((rotation_matrix.get_num_rows() < 3) || (rotation_matrix.get_num_cols() < 3))
    {
        KJB_THROW_2(KJB_error, "Camera rotation matrix has wrong dimensions");
    }

    for(unsigned int i = 0; i < 3; i++)
    {
        for(unsigned int j = 0; j < 3; j++)
        {
            set_modelview_entry(i,j,rotation_matrix(i,j));
        }
    }

    /*
     * In the classic Forsyth and Ponce formulation, the extrinsic camera matrix
     * is 4X4 defined as [R -c*R] where R is a rotation matrix and c is the camera center
     */
    Vector camera_center_in_camera_coordinates = rotation_matrix*camera_center_in_world_coordinates;
    for(unsigned int i = 0; i < 3; i++)
    {
        set_modelview_entry(i,3,-camera_center_in_camera_coordinates(i));
    }
}

/*
 * Sets the camera rotation angles. The order of rotation is pitch, yaw, roll
 * The rotations are done such that at any given moment, if you change let's
 * say the pitch, the camera will rotate by the amount of change in the pitch
 * around the camera current x-axis
 *
 * @pitch the angle around the x axis
 * @yaw the angle around the y axis
 * @roll the angle around the z axis
 */
void Parametric_camera_gl_interface::set_rotation_angles(double pitch, double yaw, double roll)
{
    /*
     * This function will call function transform, which is inherited
     * from rigid_object. Transform will call set_rotation_matrix
     * that will update the gl modelview matrix accordingly
     */
    Rigid_object::set_rotations(pitch, yaw, roll);
}

/**
 * Transforms the current camera center and frame using the input 4X4 transformation matrix
 *
 * @param M the 3X3 upper left is used as the new rotation matrix,
 * whereas the leftmost column is interpreted as a translation vector
 */
void Parametric_camera_gl_interface::transform(const kjb::Matrix & M)
{
    set_rotation_matrix(M);
    if( (fabs(M(0,3)) > DBL_EPSILON) || (fabs(M(1,3)) > DBL_EPSILON) || (fabs(M(2,3)) > DBL_EPSILON) )
    {
        translate(M(0,3), M(1,3), M(2,3));
    }
}

/**
 * @param dx the amount of translation along the world x axis
 * @param dy the amount of translation along the world y axis
 * @param dz the amount of translation along the world z axis
 */
void Parametric_camera_gl_interface::translate(double dx, double dy, double dz)
{
    KJB(UNTESTED_CODE());
    Vector new_center = camera_center_in_world_coordinates;
    new_center(0) += dx;
    new_center(1) += dy;
    new_center(2) += dz;
    set_camera_center(new_center);
}

/*
 * Sets the camera rotation angles and translates the center. T
 * he order of rotation is pitch, yaw, roll
 * The rotations are done such that at any given moment, if you change let's
 * say the pitch, the camera will rotate by the amount of change in the pitch
 * around the camera current x-axis
 *
 * @pitch the angle around the x axis
 * @yaw the angle around the y axis
 * @roll the angle around the z axis
 * @param dx the amount of translation along the world x axis
 * @param dy the amount of translation along the world y axis
 * @param dz the amount of translation along the world z axis
 */
void Parametric_camera_gl_interface::set_rotations_and_translate(double pitch, double yaw, double roll, double dx, double dy, double dz)
{
    KJB(UNTESTED_CODE());
    set_rotation_angles(pitch, yaw, roll);
    translate(dx, dy, dz);
}

/*
 * Transforms the input point from world frame to the camera frame.
 *
 * @param point the point to transform. Must be in homogeneous coordinates
 */
void Parametric_camera_gl_interface::transform_point_to_camera_frame(kjb::Vector & point) const
{
    try{
        KJB(UNTESTED_CODE());
        point = get_rotations()*point;
        point = point - camera_center_in_world_coordinates;
    } catch(kjb::KJB_error ex)
    {
        KJB_THROW_2(KJB_error, "Transform point to eye frame, the input point must be in homogeneous coordinates");
    }
}

/*
 * Rotates the input point from world frame to the camera frame.Only rotation is done!!!
 *
 * @param point the point to transform. Must be in homogeneous coordinates
 */
void Parametric_camera_gl_interface::rotate_point_to_camera_frame(kjb::Vector & point) const
{
    try{
        point = get_rotations()*point;
    } catch(kjb::KJB_error ex)
    {
        KJB_THROW_2(KJB_error, "Transform point to eye frame, the input point must be in homogeneous coordinates");
    }
}

/** Transforms a point in camera coordinates to world coordinates */
void Parametric_camera_gl_interface::get_point_in_world_coordinates
(
    const kjb::Vector & point_in_camera_coordinates,
    kjb::Vector & point_in_world_coordinates
) const
{
    if(point_in_camera_coordinates.size() != 4)
    {
        KJB_THROW_2(Illegal_argument,"Point in camera coordinates must be in homogeneous coordinates");
    }
    if( fabs(point_in_camera_coordinates(3)) < DBL_EPSILON)
    {
        KJB_THROW_2(Illegal_argument,"Point in camera coordinates has homogeneous coordinate = 0");
    }

    point_in_world_coordinates = (get_rotations().transpose() )* (point_in_camera_coordinates/point_in_camera_coordinates(3))
                                 - camera_center_in_world_coordinates;
}

/**
 * Checks whether a polygon is visible under the current camera by
 * checking that the dot product between the polygon normal
 * and the normalized vector between the camera and the polygon
 * center is bigger than epsilon (whose default value is zero)
 *
 * @param p the polygon to test
 * @param epsilon the constant to compare the dot product with (default = 0)
 */
bool Parametric_camera_gl_interface::Polygon_visibility_test(const kjb::Polygon & p, double epsilon) const
{
    const kjb::Vector & centroid = p.get_centroid();
    const kjb::Vector & normal = p.get_normal();

    double x = (camera_center_in_world_coordinates(0) - centroid(0) );
    double y = (camera_center_in_world_coordinates(1) - centroid(1) );
    double z = (camera_center_in_world_coordinates(2) - centroid(2) );

    if((x*normal(0) + y*normal(1) + z*normal(2)) > epsilon)
        return true;

    return false;
}

/**
 * Given a point in world coordinates, it returns true if the point is in
 * the viewing frustum, false otherwise
 *
 * @param point_in_world_coordinates The input point in world coordinates
 * @param num_rows The height of the image
 * @param num_cols The width of the image
 *
 * @return true if the point is in the frustum, false otherwise
 */
bool Parametric_camera_gl_interface::is_point_in_camera_frustum
(
    const kjb::Vector & point_in_world_coordinates,
    double & ox_,
    double & oy_,
    unsigned int num_rows,
    unsigned int num_cols
) const
{
    kjb::Vector point_in_camera_coordinates = point_in_world_coordinates;
    transform_point_to_camera_frame(point_in_camera_coordinates);

    double _x, _y, _z;

    /** The camera is looking down the negative z axis */
    if(point_in_camera_coordinates(2) >= (-get_near()) )
    {
        return false;
    }

    if(point_in_camera_coordinates(2) <= (-get_far()))
    {
        return false;
    }

    project_point(_x, _y, _z, point_in_world_coordinates, num_rows);
    if(fabs(_z) <=  DBL_EPSILON)
    {
        return false;
    }
    //ox_ = (int)(_x/_z);
    //oy_ = (int)(_y/_z);
    ox_ = _x;
    oy_ = _y;

    if( (ox_ < 0) || (oy_ < 0) )
    {
        return false;
    }

    if( (ox_ >= (int) num_cols) || (oy_ >= (int) num_rows) )
    {
        return false;
    }

    return true;

}


/**

Multi_camera::Multi_camera(const Multi_camera& src) :
    Abstract_camera(),
    m_cameras(src.m_cameras.size()),
    m_active_camera(src.m_active_camera)
{
    for(size_t i = 0; i < src.m_cameras.size(); i++)
    {
        m_cameras[i] = src.m_cameras[i]->clone();
    }
}

Multi_camera::~Multi_camera()
{
    for(size_t i = 0; i < m_cameras.size(); i++)
    {
        delete m_cameras[i];
    }
}

Multi_camera& Multi_camera::operator++(int)
{
    m_active_camera++;
    m_active_camera %= m_cameras.size();
    return *this;
}

Multi_camera& Multi_camera::operator--(int)
{
    m_active_camera = (m_active_camera + m_cameras.size() - 1) % m_cameras.size();

    return *this;
}

*/
