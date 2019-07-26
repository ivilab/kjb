
/* $Id: gr_opengl.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "gr_cpp/gr_opengl.h"
#include "i_cpp/i_pixel.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_vector_d.h"
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_matrix_d.h"
#include "g_cpp/g_quaternion.h"
#include "i_cpp/i_image.h"
#include "g_cpp/g_util.h"
#include "l_cpp/l_int_matrix.h"
#include "gr_cpp/gr_opengl_headers.h"
#include "g_cpp/g_line.h"

#include <boost/multi_array.hpp>
namespace kjb {
namespace opengl {

static const int NOT_SET = -1;
static int m_has_stencil_buffer = NOT_SET;

/* Define symbol as a void NULL in case of NO_LIBS */
#ifndef KJB_HAVE_GLUT
#define GLUT_BITMAP_8_BY_13 0
#endif

/**
 * @brief   Allocate and build a float array representing the pixel data.
 *
 * Data will be arranged in row-major format with BGR or RGB order, with
 * values in [0,1]. Caller is responsible for freeing the result.
 *
 * This is useful for interfacing C functions like Opengl that require a pixel
 * array as input.
 *
 * @tparam  bgr_format  If true, the order of channels will be blue, green, red;
 * if false, the channel order will be red, green, blue. Most opengl implementations
 * store textures in BGR format, so this is generally faster when loading textures.
 *
 * @param img The image to convert to an array.
 *
 * @note    This function is not available from the outside. It's exclusively
 *          used in this file.
 */
template <bool bgr_format>
float* to_gl_array(const Image& img)
{
    float* array = new float[img.get_length() * Image::END_CHANNELS];
    float* cur = array;

    // opengl expects rows in reversed order
    for(int row = img.get_num_rows()-1; row >= 0; row--)
    {
        for(int col = 0; col < img.get_num_cols(); col++)
        {
            if(bgr_format)
            {
                *cur++ = img(row, col, Image::BLUE)/255;
                *cur++ = img(row, col, Image::GREEN)/255;
                *cur++ = img(row, col, Image::RED)/255;
            }
            else
            {
                *cur++ = img(row, col, Image::RED)/255;
                *cur++ = img(row, col, Image::GREEN)/255;
                *cur++ = img(row, col, Image::BLUE)/255;
            }
        }
    }

    return array;
}

// "private" to this file.
// Called by get_modelview and get_projection
#ifdef  KJB_HAVE_OPENGL
static Matrix get_gl_matrix(GLenum matrix_type)
{
    double gl_matrix[16];
    glGetDoublev(matrix_type, gl_matrix);

    Matrix result(4,4, 0.0);
    for(int i = 0; i < 16; i++)
    {
        result(i % 4, i / 4) = gl_matrix[i];
    }

    return result;
}
#endif


void glRotate(const kjb::Quaternion & q)
{
#ifdef KJB_HAVE_OPENGL
    glRotatef(q.get_angle() * 180 / M_PI,
            q.get_axis()(0),
            q.get_axis()(1),
            q.get_axis()(2));
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void glTranslate(const kjb::Vector& p)
{
#ifdef KJB_HAVE_OPENGL
    IFT(p.get_length() == 2 || p.get_length() == 3, Illegal_argument,
        "glTranslate requires a vector of dimension 2 or 3.");

    if(p.get_length() == 2)
    {
        glTranslated(p[0], p[1], 0.0);
    }
    else
    {
        glTranslated(p[0], p[1], p[2]);
    }
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void glTranslate(const kjb::Vector3& p)
{
#ifdef KJB_HAVE_OPENGL
    glTranslated(p[0], p[1], p[2]);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void glVertex(const Vector& p)
{
#ifdef KJB_HAVE_OPENGL
    switch(p.get_length())
    {
        case 2:
            glVertex2d(p[0], p[1]);
            break;
        case 3:
            glVertex3d(p[0], p[1], p[2]);
            break;
        case 4:
            glVertex4d(p[0], p[1], p[2], p[3]);
            break;
        default:
            KJB_THROW_2(Illegal_argument, "glVertex requires a vector of dimension = {2, 3, 4}.");
            break;
    }
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void glTranslate(const kjb::Vector2& p)
{
#ifdef KJB_HAVE_OPENGL
    glTranslated(p[0], p[1], 0.0);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void glVertex(const Vector2& p)
{
#ifdef KJB_HAVE_OPENGL
    glVertex2d(p[0], p[1]);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}


void glVertex(const Vector3& p)
{
#ifdef KJB_HAVE_OPENGL
    glVertex3d(p[0], p[1], p[2]);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void glVertex(const Vector4& p)
{
#ifdef KJB_HAVE_OPENGL
    glVertex4d(p[0], p[1], p[2], p[3]);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void glColor(const Vector& color)
{
#ifdef KJB_HAVE_OPENGL
    IFT(color.get_length() == 3 || color.size() == 4, Illegal_argument,
                                   "Color vector must be size 3 or 4.");

    if(color.size() == 3)
    {
        glColor3d(color[0], color[1], color[2]);
    }
    else
    {
        glColor4d(color[0], color[1], color[2], color[3]);
    }
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void glColor(const kjb::PixelRGBA& color)
{
#ifdef KJB_HAVE_OPENGL
    glColor3d(color.r/255.0, color.g/255.0, color.b/255.0);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void glColor(const Vector3& color)
{
    Vector tmp(color.begin(), color.end());
    glColor(tmp);
}

void glColor(const Vector4& color)
{
    Vector tmp(color.begin(), color.end());
    glColor(tmp);
}

void glLoadMatrix(const Matrix& m)
{
#ifdef KJB_HAVE_OPENGL
    IFT(m.get_num_rows() == 4 && m.get_num_cols() == 4, Dimension_mismatch,
                                                        "Matrix must be 4x4.");
    double gl_matrix[16];
    for(int i = 0; i < 16; i++)
    {
        gl_matrix[i] = m(i % 4, i / 4);
    }

    glLoadMatrixd(gl_matrix);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void glLoadMatrix(const kjb::Matrix_d<4,4>& m)
{
#ifdef KJB_HAVE_OPENGL
    double gl_matrix[16];
    for(int i = 0; i < 16; i++)
    {
        gl_matrix[i] = m(i % 4, i / 4);
    }

    glLoadMatrixd(gl_matrix);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif

}

template <class Matrix_type>
void glMultMatrix_disp_(const Matrix_type& m)
{
#ifdef KJB_HAVE_OPENGL
    double gl_matrix[16];
    for(int i = 0; i < 16; i++)
    {
        gl_matrix[i] = m(i % 4, i / 4);
    }

    glMultMatrixd(gl_matrix);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif

}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void glMultMatrix(const Matrix& m)
{
    IFT(m.get_num_rows() == 4 && m.get_num_cols() == 4, Dimension_mismatch,
                                                        "Matrix must be 4x4.");
    glMultMatrix_disp_(m);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void glMultMatrix(const Matrix_d<4,4>& m)
{
    glMultMatrix_disp_(m);
}


void gluBuild2DMipmaps(const Image& img)
{
#ifdef KJB_HAVE_OPENGL
    float* array = to_gl_array<true>(img);

    ::gluBuild2DMipmaps(
            GL_TEXTURE_2D,
            3,
            img.get_num_cols(),
            img.get_num_rows(),
            GL_BGR,
            GL_FLOAT,
            array);

    delete[] array;
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

#ifdef KJB_HAVE_OPENGL
void glTexImage2D(GLenum target, GLint level, GLint border, const Image& img)
{
    float* array = to_gl_array<true>(img);

    // may need to change back to RGB here and in to_gl_array if ATI continues to fail
    ::glTexImage2D(target, level, GL_RGB8, img.get_num_cols(),
                   img.get_num_rows(), border, GL_BGR, GL_FLOAT, array);
    delete[] array;
}
#endif

void glTexImage2D(const Image& img)
{
#ifdef KJB_HAVE_OPENGL
    glTexImage2D(GL_TEXTURE_2D, 0, 0, img);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}



/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class ReturnVectorType, class VectorType>
ReturnVectorType gluUnProject_dispatch_(const VectorType& vertex, double* modelview, double* projection, int* viewport)
{
#ifdef  KJB_HAVE_OPENGL
    ReturnVectorType result;
    result.resize(3);

    bool status;
    if(vertex.size() == 3)
    {
        status = ::gluUnProject(vertex[0], vertex[1], vertex[2], modelview, projection,
                       viewport, &result[0], &result[1], &result[2]);
    }
    else if(vertex.size() == 2)
    {
        status = ::gluUnProject(vertex[0], vertex[1], 0, modelview, projection,
                       viewport, &result[0], &result[1], &result[2]);
    }
    else
    {
        KJB_THROW_2(Illegal_argument, "gluUnProject() vertex must be of dimension 2 or 3");
    }

    if(!status)
        KJB_THROW_2(Runtime_error, "gluUnProject() failed");

    return result;
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

Vector gluUnProject(const Vector& vertex, double* modelview, double* projection, int* viewport)
{
    return gluUnProject_dispatch_<Vector, Vector>(vertex, modelview, projection, viewport);
}

Vector3 gluUnProject(const Vector2& vertex, double* modelview, double* projection, int* viewport)
{
    return gluUnProject_dispatch_<Vector3, Vector2>(vertex, modelview, projection, viewport);
}

Vector3 gluUnProject(const Vector3& vertex, double* modelview, double* projection, int* viewport)
{
    return gluUnProject_dispatch_<Vector3, Vector3>(vertex, modelview, projection, viewport);
}


void gluLookAt(Matrix& M, const Vector& eye, const Vector& target, const Vector& up)
{
    Vector f = target - eye;
    f.normalize();

    Vector upp = up;
    upp.normalize();
    
    Vector s = cross(f, upp);
    Vector u = cross(s, f);

    Matrix L(3, 3);
    L.set_row(0, s);
    L.set_row(1, u);
    L.set_row(2, -f);

    Vector t = -(L * eye);
    t.resize(4, 1.0);

    L.resize(4, 4, 0.0);
    L.set_col(3, t);

    M *= L;
}


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/**
 * Template dispatch function; do not call directly.
 */
template <class Matrix_type>
void glOrtho_disp_(
        Matrix_type& state,
        double left,
        double right,
        double bottom,
        double top,
        double znear,
        double zfar)
{
    Matrix_type ortho;
    ortho.resize(4,4);
    ortho(0,0) = 2.0/(right - left); 
    ortho(1,1) = 2.0/(top  - bottom); 
    ortho(2,2) = -2.0/(zfar  - znear); 
    ortho(3,3) = 1.0;
    ortho(0,3) = - (right + left) / (right - left);
    ortho(1,3) = - (top + bottom) / (top - bottom);
    ortho(2,3) = - (zfar + znear) / (zfar - znear);

    state *= ortho;
}

// EXPLICIT TEMPLATE INSTANCIATIONS:
template 
void glOrtho_disp_(
        Matrix& state,
        double left,
        double right,
        double bottom,
        double top,
        double znear,
        double zfar);

template 
void glOrtho_disp_(
        Matrix_d<4,4>& state,
        double left,
        double right,
        double bottom,
        double top,
        double znear,
        double zfar);

void glOrtho(
        Matrix& state,
        double left,
        double right,
        double bottom,
        double top,
        double znear,
        double zfar)
{
    ASSERT(state.get_num_rows() == 4);
    ASSERT(state.get_num_cols() == 4);
    glOrtho_disp_(state, left, right, bottom, top, znear, zfar);
}

void glOrtho(
        Matrix_d<4,4>& state,
        double left,
        double right,
        double bottom,
        double top,
        double znear,
        double zfar)
{
    glOrtho_disp_(state, left, right, bottom, top, znear, zfar);
}

void glClearDepthBuffer()
{
#ifdef KJB_HAVE_OPENGL
    glClear(GL_DEPTH_BUFFER_BIT);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void glcolormask(bool b1, bool b2, bool b3, bool b4)
{
#ifdef KJB_HAVE_OPENGL
    /*GLboolean gb1 = GL_TRUE;
    GLboolean gb2 = GL_TRUE;
    GLboolean gb3 = GL_TRUE;
    GLboolean gb4 = GL_TRUE;

    if(!b1)
    {
        gb1 = GL_FALSE;
    }
    if(!b2)
    {
        gb2 = GL_FALSE;
    }
    if(!b3)
    {
        gb3 = GL_FALSE;
    }
    if(!b4)
    {
        gb4= GL_FALSE;
    }
    glColorMask(gb1, gb2, gb3, gb4);*/

    glColorMask((GLboolean)b1, (GLboolean)b2, (GLboolean)b3, (GLboolean)b4);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void glEnableLineSmooth()
{
#ifdef KJB_HAVE_OPENGL
    glEnable(GL_LINE_SMOOTH);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void glDisableLineSmooth()
{
#ifdef KJB_HAVE_OPENGL
    glDisable(GL_LINE_SMOOTH);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void default_init_opengl(unsigned int width, unsigned int height)
{
#ifdef KJB_HAVE_OPENGL
    // Why is lighting disabled by default?? --Ernesto (2011/02/17)

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

size_t get_viewport_width()
{
#ifdef KJB_HAVE_OPENGL
    
    GLint glv[4];
    glGetIntegerv(GL_VIEWPORT, glv);
    return glv[2];
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

size_t get_viewport_height()
{
#ifdef KJB_HAVE_OPENGL
    GLint glv[4];
    glGetIntegerv(GL_VIEWPORT, glv);
    return glv[3];
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

Image get_framebuffer_as_image()
{
#ifdef KJB_HAVE_OPENGL
    return get_framebuffer_as_image(0, 0, get_viewport_width(), get_viewport_height());
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix get_modelview_matrix()
{
#ifdef KJB_HAVE_OPENGL
    return get_gl_matrix(GL_MODELVIEW_MATRIX);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix get_projection_matrix()
{
#ifdef KJB_HAVE_OPENGL
    return get_gl_matrix(GL_PROJECTION_MATRIX);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector project(const Vector& x)
{
    Vector h_x = ::kjb::geometry::euclidean_to_projective(x);

    Matrix P = get_projection_matrix() * get_modelview_matrix();

    Vector y = P * h_x;

    double width = get_viewport_width();
    double height = get_viewport_height();

    double w = y[3];
    y.resize(2);
    y /= w;

    y[0] += 1.0; y[0] *= (0.5 * width);
    y[1] += 1.0; y[1] *= (0.5 * height);

    return y;
}

Vector2 project(const Vector3& x)
{
    Vector x_tmp(x.begin(), x.end());
    Vector result = project(x_tmp);
    ASSERT(result.size() == 2);
    return Vector2(result[0], result[1]);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
template <class ReturnVectorType, class VectorType>
ReturnVectorType unproject_dispatch_(const VectorType& vertex)
{
#ifdef  KJB_HAVE_OPENGL
    double modelview[16];
    double projection[16];
    int viewport[4];

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    return gluUnProject_dispatch_<ReturnVectorType, VectorType>(vertex, modelview, projection, viewport);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

Vector unproject(const Vector& vertex)
{
    return unproject_dispatch_<Vector, Vector>(vertex);
}

Vector3 unproject(const Vector2& vertex)
{
    return unproject_dispatch_<Vector3, Vector2>(vertex);
}

Vector3 unproject(const Vector3& vertex)
{
    return unproject_dispatch_<Vector3, Vector3>(vertex);
}



/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void draw_full_screen_quad(float z)
{
#ifdef KJB_HAVE_OPENGL
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_QUADS);
        glVertex3f(-1., -1., z);
        glVertex3f(-1.,  1., z);
        glVertex3f( 1.,  1., z);
        glVertex3f( 1., -1., z);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool has_stencil_buffer()
{
#ifdef KJB_HAVE_OPENGL
    if(m_has_stencil_buffer == NOT_SET)
    {
        int bits = 0;
        glGetIntegerv(GL_STENCIL_BITS, &bits);

        if(bits == 0)
        {
            m_has_stencil_buffer = 0;
        } else {
            m_has_stencil_buffer = 1;
        }
    }

    return m_has_stencil_buffer;
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void set_framebuffer(const Image& img)
{
#ifdef KJB_HAVE_OPENGL
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);

    const int NUM_CHANNELS = 3;

    size_t num_cols = get_viewport_width();
    size_t num_rows = get_viewport_height();
    IFTD(num_cols == static_cast<size_t>(img.get_num_cols())
            && num_rows == static_cast<size_t>(img.get_num_rows()), KJB_error,
            "Input image (%d x %d) size differs from viewport size (%d x %d)",
            (img.get_num_rows())(img.get_num_cols())(num_rows)(num_cols));

    float* img_buf = new float[NUM_CHANNELS * num_cols * num_rows];
    IFT(img_buf, KJB_error, "Could not allocate buffer to store image in GL framebuffer.");

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
//    glWindowPos2f(0.0, 0.0);

    for(size_t row = 0; row < num_rows; row++)
    {
        size_t r = num_rows - row - 1;
        for(size_t col = 0; col < 3*num_cols; col += 3)
        {

            img_buf[col + 0 + row*NUM_CHANNELS*num_cols] = img(r, col/3, Image::RED) / 255.0;
            img_buf[col + 1 + row*NUM_CHANNELS*num_cols] = img(r, col/NUM_CHANNELS, Image::GREEN) / 255.0;
            img_buf[col + 2 + row*NUM_CHANNELS*num_cols] = img(r, col/NUM_CHANNELS, Image::BLUE) / 255.0;
        }
    }

    glDrawPixels(num_cols, num_rows, GL_RGB, GL_FLOAT, img_buf);

    // Calling flush here is unnecessary, and could harm performance.  
    // calling scope should handle this if it needs it. 
//    glFlush();
    delete[] img_buf;

    glPopAttrib();
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void set_framebuffer(const Matrix& mat)
{
#ifdef KJB_HAVE_OPENGL
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);

    const int NUM_CHANNELS = 1;

    size_t num_cols = get_viewport_width();
    size_t num_rows = get_viewport_height();
    IFT(num_cols == static_cast<size_t>(mat.get_num_cols()) && num_rows == static_cast<size_t>(mat.get_num_rows()),
        KJB_error, "Input image is smaller than viewport size");


    float* img_buf = new float[NUM_CHANNELS * num_cols * num_rows];
    IFT(img_buf, KJB_error, "Could not allocate buffer to store image in GL framebuffer.");

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
//    glWindowPos2f(0.0, 0.0);

    for(size_t row = 0; row < num_rows; row++)
    {
        for(size_t col = 0; col < num_cols; col ++)
        {
            size_t r = num_rows - row - 1;
            img_buf[col + row*num_cols] = mat(r, col) / 255.0;
        }
    }

    glDrawPixels(num_cols, num_rows, GL_RED, GL_FLOAT, img_buf);

    delete[] img_buf;

    glPopAttrib();

    GL_ETX();
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void set_framebuffer(const Int_matrix& mat)
{
#ifdef KJB_HAVE_OPENGL
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);

    size_t num_cols = get_viewport_width();
    size_t num_rows = get_viewport_height();

    IFT(num_cols == static_cast<size_t>(mat.get_num_cols()) && num_rows == static_cast<size_t>(mat.get_num_rows()),
        KJB_error, "Input image is smaller than viewport size");

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
//    glWindowPos2f(0.0, 0.0);

    Int_matrix mat2(num_rows, num_cols);
    for(size_t row = 0; row < num_rows; row++)
    {
        for(size_t col = 0; col < num_cols; col++)
        {
            mat2(row, col) = mat(num_rows - 1 - row, col);
        }
    }

    // Has this been tested?  It sems like passing an int array while telling opengl it's an unsigned byte is a bug.
    // There was a bug in the version for Matrix, so there might be a bug here, too.
    // Also, This should probably be using GL_RED, not GL_RGBA.
    // kls, Oct 12, 2011
    // Found another bug.  This function almost certainly is not tested.  Use with caution!
    // kls, Oct 14, 2011
    glDrawPixels(num_cols, num_rows, GL_RGBA, GL_UNSIGNED_BYTE, *mat2.get_c_matrix()->elements);

    glPopAttrib();
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Image get_framebuffer_as_image
(
    size_t x,
    size_t y,
    size_t width,
    size_t height
)
{
#ifdef KJB_HAVE_OPENGL
    const int NUM_CHANNELS = 3;

    Image result(height, width);

    float* rgb = new float[height * width * NUM_CHANNELS];
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glReadPixels(x, y, width, height, GL_RGB, GL_FLOAT, rgb);

    GL_ETX();

    const size_t num_rows = height - y;
    for(size_t row = y; row < height; row++)
    {
        for(size_t col = x; col < width; col++)
        {
            const size_t rgb_offset = NUM_CHANNELS
                                        * (col + (num_rows - row - 1)*width);

            result(row, col).r = 255*rgb[rgb_offset];
            result(row, col).g = 255*rgb[rgb_offset + 1];
            result(row, col).b = 255*rgb[rgb_offset + 2];
        }
    }

    delete[] rgb;

    return result;
#else
    KJB_THROW_2(Missing_dependency, "OpenGL");
#endif

}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

// private function, called by get_depth_buffer_as_matrix and get_framebuffer_as_matrix
/**
 * Retrieve pixels from an opengl buffer.  all values will be between [0,1] unless some fancy glPixelTranfer tricks are in use.
 *
 * @param format The format of the pixel data.  Specifies which buffer will be retrieved.  See glReadPixels for valid values.
*/
void get_buffer_as_matrix_
(
    size_t x,
    size_t y,
    size_t width,
    size_t height,
    GLenum format,
    Matrix& result
)
{
#ifdef KJB_HAVE_OPENGL
    const int NUM_CHANNELS = 1;

    result.resize(height, width);

    float* rgb = new float[height * width * NUM_CHANNELS];
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glReadPixels(x, y, width, height, format, GL_FLOAT, rgb);

    size_t i = 0;
    for(size_t row = y; row < y + height; row++)
    {
        for(size_t col = x; col < x + width; col++, i++)
        {
            const size_t rev_row = height - row - 1 + y;
            result(rev_row, col) = rgb[i];
        }
    }

    delete[] rgb;
    GL_ETX();
#else
    KJB_THROW_2(Missing_dependency, "OpenGL");
#endif

}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void get_framebuffer_as_matrix
(
    size_t x,
    size_t y,
    size_t width,
    size_t height,
    Matrix& result
)
{
    get_buffer_as_matrix_(x,y,width,height,GL_RED,result);
    //result *= 255;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void get_depth_buffer_as_matrix
(
    size_t x,
    size_t y,
    size_t width,
    size_t height,
    Matrix& result
)
{
    get_buffer_as_matrix_(x,y,width,height,GL_DEPTH_COMPONENT,result);
}

Matrix get_framebuffer_as_matrix(size_t x, size_t y, size_t width, size_t height)
{
    Matrix result;
    get_framebuffer_as_matrix(x, y, width, height, result);
    return result;
}

Matrix get_depth_buffer_as_matrix(size_t x, size_t y, size_t width, size_t height)
{
    Matrix result;
    get_depth_buffer_as_matrix(x, y, width, height, result);
    return result;
}

Matrix get_framebuffer_as_matrix()
{
#ifdef KJB_HAVE_OPENGL
    return get_framebuffer_as_matrix(0, 0, get_viewport_width(), get_viewport_height());
#else
    KJB_THROW_2(Missing_dependency, "OpenGL");
#endif
}

Matrix get_depth_buffer_as_matrix()
{
#ifdef KJB_HAVE_OPENGL
    return get_depth_buffer_as_matrix(0, 0, get_viewport_width(), get_viewport_height());
#else
    KJB_THROW_2(Missing_dependency, "OpenGL");
#endif
}

Int_matrix get_framebuffer_as_int_matrix()
{
#ifdef KJB_HAVE_OPENGL
    return get_framebuffer_as_int_matrix(0, 0, get_viewport_width(), get_viewport_height());
#else
    KJB_THROW_2(Missing_dependency, "OpenGL");
#endif
}


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Int_matrix get_framebuffer_as_int_matrix
(
    size_t x,
    size_t y,
    size_t width,
    size_t height
)
{
#ifdef KJB_HAVE_OPENGL
    kjb_c::Int_matrix* temp_mat = NULL;
    get_target_int_matrix(&temp_mat, height, width);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, *temp_mat->elements);

    Int_matrix result(temp_mat);

    for(size_t i = 0; i < height / 2; i++)
    {
        for(size_t j = 0; j < width; j++)
        {
            using std::swap;
            swap(result(i, j), result(height - 1 - i, j));
        }
    }

    return result;
#else
    KJB_THROW_2(Missing_dependency, "OpenGL");
#endif

}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/**
 * @param font Glut font (default GLUT_BITMAP_8_BY_13)
 */
void bitmap_string(const std::string& s, double x, double y, void* font)
{
    if(font == 0)
        font = GLUT_BITMAP_8_BY_13;
#ifdef KJB_HAVE_GLUT
    glRasterPos2d(x, y);
    for(size_t i = 0; i < s.length(); i++)
    {
        glutBitmapCharacter(font, s.c_str()[i]);
    }
#else
    KJB_THROW_2(Missing_dependency, "OpenGL");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/**
 * Move a 3d point so that it projects to a given screen position.  Point
 * will remain in the plane parallel to the image plane (i.e. it's depth won't 
 * change).  Uses the current opengl matrix and viewport state to do camera
 * computations.
 *
 * @param world_point A 3D point to move, given in world coordinates.  
 * @param screen_point A 2d cursor position where the 3d point should project to
 * Note that cursor coordinates are relatve to bottom-left of screen, following OpenGL standard (as opposed to Glut-standard top-left)
 */
void move_in_plane(kjb::Vector3& plane_pt, const kjb::Vector2& new_pt)
{
#ifdef KJB_HAVE_OPENGL
    /*
     * The approach here might seem more roundabout than you might expect,
     * but this approach results in better precision and less drift than 
     * simpler methods I tried.
     */

    // get rotation matrix from world to camera axes 
//    const kjb::Matrix_d<3,3>& R = get_rotation_matrix();

    double modelview_array[16];
    double projection_array[16];
    int viewport[4];

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview_array);
    glGetDoublev(GL_PROJECTION_MATRIX, projection_array);
    glGetIntegerv(GL_VIEWPORT, viewport);

    typedef boost::multi_array_ref<double, 2> Gl_matrix;

    // wrap linear array with a matrix-style interface
    Gl_matrix modelview(
            modelview_array,
            boost::extents[4][4],
            boost::fortran_storage_order()); // column-major order (weird!)

//    Gl_matrix projection(projection_array, boost::extents[4][4]);

    // TODO: Test this; it's unlikely that I got this right the first time...
    // find plane parallel to image plane 
    // through current point .
    kjb::Vector3 plane_norm(0.0, 0.0, 1.0);
    Matrix_d<3,3> R;
    R(0,0) = modelview[0][0];
    R(0,1) = modelview[0][1];
    R(0,2) = modelview[0][2];
    R(1,0) = modelview[1][0];
    R(1,1) = modelview[1][1];
    R(1,2) = modelview[1][2];
    R(2,0) = modelview[2][0];
    R(2,1) = modelview[2][1];
    R(2,2) = modelview[2][2];

    // un-rotate normal vector
    plane_norm = R.transpose() * plane_norm;

    // still unit-length?
    ASSERT(fabs(plane_norm.magnitude() - 1.0) < 10 * FLT_EPSILON);

    // we now have the normal vector of the plane of motion, as well as a point on that plane; namely the current point of interest.

    // now find the back-projection line through the clicked point

    // get backprojection line

    // convert to a 3-vector, where the z coordinate indicates where between the front and
    // back plane of the frustum to intersect
    Vector3 tmp;
    tmp[0] = new_pt[0];
    tmp[1] = new_pt[1];


    tmp[2] = 0; // near-plane
    kjb::Vector3 line_pt1 = gluUnProject(tmp, modelview_array, projection_array, viewport);

    tmp[2] = 1; // far-plane
    kjb::Vector3 line_pt2 = gluUnProject(tmp, modelview_array, projection_array, viewport);

    kjb::Vector3 line_dir = (line_pt2 - line_pt1);
    line_dir.normalize(); // probably not necessary
    
    // intersect backprojection line with plane of motion to find the 3d location of the new point.

    double t = intersect_line_with_plane(line_pt1, line_dir, plane_pt, plane_norm);

    plane_pt = line_pt1 + t * line_dir;
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}

/**
 * Use current opengl matrix state to get the backprojection line for a given 
 * screen point
 *
 * @param pt Point in OpenGL screen coordinates (bottom-left origin)
 * @param line_pt Output.  point on the backprojection line
 * @param line_dir Output. direction vector for backprojection line
 */
void get_backprojection_line(const Vector2& pt, Vector3& line_pt, Vector3& line_dir)
{
#ifdef KJB_HAVE_OPENGL
    double modelview_array[16];
    double projection_array[16];
    int viewport[4];

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview_array);
    glGetDoublev(GL_PROJECTION_MATRIX, projection_array);
    glGetIntegerv(GL_VIEWPORT, viewport);

    // convert to a 3-vector, where the z coordinate indicates where between the front and
    // back plane of the frustum to intersect
    Vector3 tmp;
    tmp[0] = pt[0];
    tmp[1] = pt[1];


    tmp[2] = 0; // near-plane
    line_pt = gluUnProject(tmp, modelview_array, projection_array, viewport);

    tmp[2] = 1; // far-plane
    kjb::Vector3 pt2 = gluUnProject(tmp, modelview_array, projection_array, viewport);

    line_dir = (pt2 - line_pt);
    line_dir.normalize(); // probably not necessary
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void render_xz_plane_grid
(
    double xmin,
    double xmax,
    double zmin,
    double zmax,
    double csz
)
{
#ifdef KJB_HAVE_OPENGL
    // draw horizontal lines
    glBegin(GL_LINES);
    for(double z = zmin; z <= zmax; z += csz)
    {
        glVertex3d(xmin, 0.0, z);
        glVertex3d(xmax, 0.0, z);
    }

    // draw vertical lines
    for(double x = xmin; x <= xmax; x += csz)
    {
        glVertex3d(x, 0.0, zmin);
        glVertex3d(x, 0.0, zmax);
    }
    glEnd();

    GL_ETX();
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void render_xy_plane_grid
(
    double xmin,
    double xmax,
    double ymin,
    double ymax,
    double csz
)
{
#ifdef KJB_HAVE_OPENGL
    // draw horizontal lines
    glBegin(GL_LINES);
    for(double y = ymin; y <= ymax; y += csz)
    {
        glVertex2d(xmin, y);
        glVertex2d(xmax, y);
    }

    // draw vertical lines
    for(double x = xmin; x <= xmax; x += csz)
    {
        glVertex2d(x, ymin);
        glVertex2d(x, ymax);
    }
    glEnd();

    GL_ETX();
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void render_arrow(const Vector& src, const Vector& dest)
{
#ifdef KJB_HAVE_OPENGL
    Vector dir(dest - src);
    double mag = dir.magnitude();

    Vector l(0.8 * mag, -0.1 * mag);
    Vector r(0.8 * mag, 0.1 * mag);

    double angle = acos(dir[0] / mag);
    if(dir[1] < 0)
    {
        angle = -angle;
    }

    Matrix R = geometry::get_rotation_matrix(angle);

    // shaft of arrow
    glBegin(GL_LINES);
        glVertex(src);
        glVertex(dest);
    glEnd();

    // draw head
    glBegin(GL_POLYGON);
        glVertex(dest);
        glVertex(src + R*l);
        glVertex(src + R*r);
    glEnd();
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void render_x(const Vector& center, double sz)
{
#ifdef KJB_HAVE_OPENGL
    glBegin(GL_LINES);
        glVertex(center + Vector(-sz/2, sz/2));
        glVertex(center + Vector(sz/2, -sz/2));
        glVertex(center + Vector(-sz/2, -sz/2));
        glVertex(center + Vector(sz/2, sz/2));
    glEnd();
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

#ifdef KJB_HAVE_OPENGL
Opengl_error::Opengl_error(unsigned int error_code, const char* file, int line) :
    Runtime_error((const char*) gluErrorString(error_code), file, line),
    code_(error_code)
{}

Opengl_error::Opengl_error(unsigned int error_code, const std::string& msg, const char* file, int line) :
    Runtime_error(msg, file, line),
    code_(error_code)
{}

/**
 * Unknown error
 */
Opengl_error::Opengl_error(const std::string& msg, const char* file, int line) :
    Runtime_error(msg, file, line),
    code_(-1)
{}
#endif // KJB_HAVE_OPENGL

} // namespace opengl
} // namespace kjb

