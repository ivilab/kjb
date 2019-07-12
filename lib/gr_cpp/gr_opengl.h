
/* $Id: gr_opengl.h 21599 2017-07-31 00:44:30Z kobus $ */

#ifndef KJB_CPP_OPENGL_H
#define KJB_CPP_OPENGL_H

/*******************************************************************
 *  A collection of functions and classes for using KJB c++ objects
 *  with opengl.
 *
 * In general, the functions below mimick native opengl functions,
 * but use objects instead of opengl primitives.
 *
 * This doesn't include the OpenGL library headers; to get them
 * include gr_cpp/gr_opengl_headers.h
 *
 *******************************************************************/

#include "l_cpp/l_exception.h"
#include <string.h>
#include <iostream>

//#include <gr_cpp/gr_opengl_headers.h>

namespace kjb
{

class Quaternion;
class Vector;
class Matrix;
class Int_matrix;
class Image;
struct PixelRGBA;

template <size_t D>
class Vector_d;
typedef Vector_d<2> Vector2;
typedef Vector_d<3> Vector3;
typedef Vector_d<4> Vector4;

template <size_t M, size_t N, bool Transposed>
class Matrix_d;
typedef Matrix_d<4,4,false> Matrix4;


namespace opengl
{

/*-----------------------------------------------------------------------------
 *
 * Utility overloads
 *
 * These functions are overloaded versions of standard OpenGL functions.
 *-----------------------------------------------------------------------------*/

/** @brief  glRotate for a kjb::Quaternion. */
void glRotate(const kjb::Quaternion & q);

/** @brief  glTranslate for a kjb::Vector. */
void glTranslate(const kjb::Vector& p);

/** @brief  glTranslate for a kjb::Vector3. */
void glTranslate(const kjb::Vector3& p);

void glTranslate(const kjb::Vector2& p);

/** @brief  glVertex for a kjb::Vector. */
void glVertex(const Vector& p);

void glVertex(const Vector2& p);

void glVertex(const Vector3& p);

void glVertex(const Vector4& p);

/** @brief  glColor for a kjb::Vector. */
void glColor(const Vector& color);

void glColor(const kjb::PixelRGBA& color);

void glColor(const Vector3& color);

void glColor(const Vector4& color);

/** @brief  glLoadMatrix for a kjb::Matrix. */
void glLoadMatrix(const kjb::Matrix& m);

/** @brief  glLoadMatrix for a kjb::Matrix4. */
void glLoadMatrix(const kjb::Matrix4& m);

/** @brief  glMultMatrix for a kjb::Matrix. */
void glMultMatrix(const kjb::Matrix& m);

/** @brief  glLoadMatrix for a kjb::Matrix4. */
void glMultMatrix(const kjb::Matrix4& m);

/** @brief  NEEDS COMMENTING!!!. */
void gluBuild2DMipmaps(const Image& img);

#ifdef KJB_HAVE_OPENGL
/**
 * @brief   Initialize array with image pixels. Also available through
 *          Texture::init(Image)
 *
 * @see     Testure::init(const Image&)
 */
void glTexImage2D(unsigned int target, int level, int border, const Image& img);
#endif

/** @brief  Copy kjb::Image into an opengl texture. */
void glTexImage2D(const Image& img);

// Why doesn't this use kjb::Matrix's for modelview and projection? --Ernesto (2011/02/17)
//
// This is a thin wrapper for gluUnProject, which is intended to receive matrices received from
// a call to glGetDoublev().  This is called from the higher-level unproject() function.
// Please feel free to overload for kjb::Matrix. -- Kyle (2011/11/16)
//
/** @brief  Wraps gluUnproject using kjb::Vertex.*/
Vector gluUnProject(const Vector& vertex, double* modelview, double* projection, int* viewport);
Vector3 gluUnProject(const Vector2& vertex, double* modelview, double* projection, int* viewport);
Vector3 gluUnProject(const Vector3& vertex, double* modelview, double* projection, int* viewport);

/** @brief  Computes a gluLookAt matrix and multiplies it by the given matrix. */
void gluLookAt(Matrix& M, const Vector& eye, const Vector& target, const Vector& up);


/**
 * right-multiply the state matrix with the equivalent of 
 * opengl's glOrtho matrix.
 * @pre state.get_num_rows() == 4
 * @pre state.get_num_cols() == 4
 */
void glOrtho(
        Matrix& state,
        double left,
        double right,
        double bottom,
        double top,
        double znear,
        double zfar);


/**
 * right-multiply the state matrix with the equivalent of 
 * opengl's glOrtho matrix.
 * @pre state.get_num_rows() == 4
 * @pre state.get_num_cols() == 4
 */
void glOrtho(
        Matrix4& state,
        double left,
        double right,
        double bottom,
        double top,
        double znear,
        double zfar);


void glClearDepthBuffer();


void glcolormask(bool b1, bool b2, bool b3, bool b4);


void glEnableLineSmooth();


void glDisableLineSmooth();



/*-----------------------------------------------------------------------------
 *  Non-standard OpenGL utility methods.
 *-----------------------------------------------------------------------------*/

/** @brief  Initializes OpenGL in a standard way. */
void default_init_opengl(unsigned int width = 100, unsigned int height = 100);



/** @brief  Get current GL viewport width. */
size_t get_viewport_width();


/** @brief  Get current GL viewport height. */
size_t get_viewport_height();



/** @brief  Returns a copy of the current modelview matrix. */
Matrix get_modelview_matrix();

/** @brief  Returns a copy of the current projection matrix. */
Matrix get_projection_matrix();

/**
 * @brief   Computes the window coordinates of the given point using
 *          current GL state.
 */
Vector project(const Vector& x);

Vector2 project(const Vector3& x);

/**
 * @brief   Unproject a point from screen coordinates to world coordinates.
 *
 * This function performs all the steps of unprojecting a vertex from screen
 * coordinates into world coordinates, including querying OpenGl for the
 * transformation matrices and viewport, and calling gluUnproject().
 */
Vector unproject(const Vector& vertex);

/**
 * @param   z   Depth of the quad. Values must be insiderange [-1, 1],
 *              otherwise the quad might not appear.
 */
void draw_full_screen_quad(float z = 0.0);

/** @brief  Returns true if OpenGL version supports stencil buffer (right?).  */
bool has_stencil_buffer();

/** @brief  Puts the contents of an image into the framebuffer. */
void set_framebuffer(const Image& img);

/**
 * @brief   Puts the contents of a matrix into the framebuffer.
 *
 * @note    At the moment, this function only sets the red channel. Later, it
 *          might provide the option of setting other channels.
 */
void set_framebuffer(const Matrix& mat);

/**
 * @brief   Puts the contents of a int matrix into the framebuffer.
 *
 * The matrix is intepreted as having the RGBA values as bytes packed
 * in each integer.
 */
void set_framebuffer(const Int_matrix& mat);

/**
 * @brief   Read pixels from opengl's back-buffer into an image.
 *
 * @param x         Horizontal offset
 * @param y         Vertical offset
 * @param width     Image width
 * @param height    Image height
 */
Image get_framebuffer_as_image(size_t x, size_t y, size_t width, size_t height);

/**
 * @brief   Read pixels from opengl's whole back-buffer into an image.
 */
Image get_framebuffer_as_image();

/**
 * Read pixels from opengl's back-buffer into a matrix.
 *
 * @param x         Horizontal offset
 * @param y         Vertical offset
 * @param width     Matrix width
 * @param height    Matrix height
 * @param out       Matrix to store result
 */
void get_framebuffer_as_matrix(size_t x, size_t y, size_t width, size_t height, Matrix& out);

/**
 * Read pixels from opengl's (back) depth-buffer into a matrix.
 *
 * @param x         Horizontal offset
 * @param y         Vertical offset
 * @param width     Matrix width
 * @param height    Matrix height
 * @param out       Matrix to store result
 */
void get_depth_buffer_as_matrix
(
    size_t x,
    size_t y,
    size_t width,
    size_t height,
    Matrix& result
);

/**
 * Read pixels from opengl's back-buffer into a matrix.
 *
 * @param x         Horizontal offset
 * @param y         Vertical offset
 * @param width     Matrix width
 * @param height    Matrix height
 */
Matrix get_framebuffer_as_matrix(size_t x, size_t y, size_t width, size_t height);


/**
 * Read pixels from opengl's (back) depth-buffer into a matrix.
 *
 * @param x         Horizontal offset
 * @param y         Vertical offset
 * @param width     Matrix width
 * @param height    Matrix height
 */
Matrix get_depth_buffer_as_matrix(size_t x, size_t y, size_t width, size_t height);



/**
 * @brief   Read pixels from opengl's whole back-buffer into a matrix.
 */
Matrix get_framebuffer_as_matrix();


/**
 * @brief   Read pixels from opengl's whole (back) depth-buffer into a matrix.
 */
Matrix get_depth_buffer_as_matrix();


/**
 * Read pixels from opengl's back-buffer into a int matrix.
 *
 * @param x         Horizontal offset
 * @param y         Vertical offset
 * @param width     Matrix width
 * @param height    Matrix height
 */
Int_matrix get_framebuffer_as_int_matrix(size_t x, size_t y, size_t width, size_t height);

/**
 * @brief   Read pixels from opengl's whole back-buffer into a int matrix.
 */
Int_matrix get_framebuffer_as_int_matrix();


/** @brief  Draws a bitmap string at the specified location. */
void bitmap_string
(
    const std::string& s,
    double x,
    double y,
    void* font = NULL // GLUT_BITMAP_8_BY_13
);

/** @brief Translate a point so it projects to a screen point */
void move_in_plane(kjb::Vector3& plane_pt, const kjb::Vector2& new_pt);

/** @brief Get backprojection line using current opengl state */
void get_backprojection_line(const Vector2& pt, Vector3& line_pt, Vector3& line_dir);

/**
 * @brief   Draw a grid representing the ground plane.
 *
 * This function draws the a grid on the XZ-plane, extending from xmin
 * to xmax in the X direction and from zmin to zmax in the Z direction.
 *
 * @param   xmin    How far to extend in -X.
 * @param   xmax    How far to extend in X.
 * @param   zmin    How far to extend in -Z.
 * @param   zmax    How far to extend in Z.
 * @param   csz     Size of grid cells (in world units).
 */
void render_xz_plane_grid
(
    double xmin,
    double xmax,
    double zmin,
    double zmax,
    double csz = 1.0
);

/**
 * @brief   Draw a grid representing the ground plane.
 *
 * This function draws the a grid on the XY-plane, extending from xmin
 * to xmax in the X direction and from zmin to zmax in the Y direction.
 *
 * @param   xmin    How far to extend in -X.
 * @param   xmax    How far to extend in X.
 * @param   zmin    How far to extend in -Y.
 * @param   zmax    How far to extend in Y.
 * @param   csz     Size of grid cells (in world units).
 */
void render_xy_plane_grid
(
    double xmin,
    double xmax,
    double ymin,
    double ymax,
    double csz = 1.0
);

/** @brief  Render an arrow from src to dest. */
void render_arrow(const Vector& src, const Vector& dest);

/** @brief  Render an 'x' at center. */
void render_x(const Vector& center, double sz);

/*-----------------------------------------------------------------------------
 *  Interface classes
 *-----------------------------------------------------------------------------*/
class Opengl_callable
{
public:
    virtual void gl_call() const = 0;
};

/* --------------------------------------------------------------------------------
 *  ERROR HANDLING
 * --------------------------------------------------------------------------------*/

#ifdef KJB_HAVE_OPENGL
/**
 * on Error Throw Exception: Opengl Version.  This version
 * takes zero arguments, because there's only one way to get an error
 * code in opengl:  by calling glGetError().
 *
 * After any opengl call that might result in an error, follow it with:
 *      GL_ETX();
 */
#define GL_ETX()                                             \
    {                                                        \
        GLenum error = glGetError();                         \
        if((error))                                          \
        {                                                    \
            KJB_THROW_2(::kjb::opengl::Opengl_error, error);                \
        }                                                    \
    }

/**
 * on Error Print Error and Terminate with Error code: Opengl Version.  This version
 * takes zero arguments, because there's only one way to get an error
 * code in opengl:  by calling glGetError().
 *
 */
#define GL_EPETE()                                             \
    {                                                        \
        GLenum error = glGetError();                         \
        if((error))                                          \
        {                                                    \
            std:: cout << "Opengl error: " << gluErrorString(error) << std::endl; \
            abort(); \
        }                                                    \
    }

class Opengl_error : public Runtime_error
{
public:
    /**
     * Use default opengl error message
     */
    Opengl_error(unsigned int error_code, const char* file, int line);

    Opengl_error(unsigned int error_code, const std::string& msg, const char* file, int line);

    /**
     * Unknown error
     */
    Opengl_error(const std::string& msg, const char* file, int line);

    unsigned int get_code() { return code_; }
private:
    unsigned int code_;
};
#endif /* #ifdef KJB_HAVE_OPENGL */


}
}

#endif
