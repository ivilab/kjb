/* $Id: gr_opengl_texture.h 21599 2017-07-31 00:44:30Z kobus $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
#ifndef KJB_CPP_OPENGL_TEXTURE
#define KJB_CPP_OPENGL_TEXTURE

// this must be before opengl.h
#include "gr_cpp/gr_opengl_headers.h"

#ifdef DEBUGGING
#include <gr_cpp/gr_glut.h>
#endif

#ifdef KJB_HAVE_OPENGL

#include <map>
#include <m_cpp/m_matrix.h>
#include <i_cpp/i_image.h>
#include <l_cpp/l_util.h>

typedef unsigned int GLunum;
namespace kjb 
{
namespace opengl
{

class Int_matrix;

/**
 * Thin wrapper for Opengl 2D texture objects.
 *
 * The benefit of this class:
 *    Automatic garbage collection.
 *    Adds type safety to opengl objects.
 *
 * TODO:
 *    * Add methods for setting each of the texture's parameters
 *
 * @author Kyle Simek
 */
class Texture
{
typedef Texture Self;

public:
    /**
     * Create a texture object, but don't allocate it.  User may then 
     * pass this object to glTexImage2D (for example) to allocate and iniitialize.
     *
     * @warning If this is called before an opengl context is created (e.g. when creating a Glut window), this will crash on some systems (e.g. OSX).  There appears to be no way to check this at runtime and throw an exceptions, so you must be careful to initialize OpenGL before this is called.  
     *
     */
    Texture();

    /**
     * Claim ownership of a texture id.  This class
     * is now responsible for deleting the texture on 
     * destruction.
     */
    Texture(GLuint texture_id, int width, int height);

    Texture(const Texture& other);

    int get_width() const;

    int get_height() const;

    int get_num_cols() const;

    int get_num_rows() const;


    /**
     * Reduce the reference count of this texture by 1.  Free GL texture if
     * reference count reaches zero.
     */
    void free();

    /**
     * Assignment.  Afterward, this object and the other object point
     * to the same GL texture.
     */
    Self& operator=(const Self& src);

    /**
     * Construct a texture and initialize it.
     *
     * This is a thin wrapper for glTexImage2D.
     *
     * @param target The target texture. Must be GL_TEXTURE_2D, GL_PROXY_TEXTURE_2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, or GL_PROXY_TEXTURE_CUBE_MAP.  GL_TEXTURE_2D is most common.
     * @level Specifies the level-of-detail number. Level 0 is the base image level. Level n is the nth mipmap reduction image. 0 is most common.
     * @param internal_format Specifies the number of color components in the texture. Must be 1, 2, 3, or 4, or one of the following symbolic constants: GL_ALPHA, GL_ALPHA4, GL_ALPHA8, GL_ALPHA12, GL_ALPHA16, GL_COMPRESSED_ALPHA, GL_COMPRESSED_LUMINANCE, GL_COMPRESSED_LUMINANCE_ALPHA, GL_COMPRESSED_INTENSITY, GL_COMPRESSED_RGB, GL_COMPRESSED_RGBA, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_LUMINANCE, GL_LUMINANCE4, GL_LUMINANCE8, GL_LUMINANCE12, GL_LUMINANCE16, GL_LUMINANCE_ALPHA, GL_LUMINANCE4_ALPHA4, GL_LUMINANCE6_ALPHA2, GL_LUMINANCE8_ALPHA8, GL_LUMINANCE12_ALPHA4, GL_LUMINANCE12_ALPHA12, GL_LUMINANCE16_ALPHA16, GL_INTENSITY, GL_INTENSITY4, GL_INTENSITY8, GL_INTENSITY12, GL_INTENSITY16, GL_R3_G3_B2, GL_RGB, GL_RGB4, GL_RGB5, GL_RGB8, GL_RGB10, GL_RGB12, GL_RGB16, GL_RGBA, GL_RGBA2, GL_RGBA4, GL_RGB5_A1, GL_RGBA8, GL_RGB10_A2, GL_RGBA12, GL_RGBA16, GL_SLUMINANCE, GL_SLUMINANCE8, GL_SLUMINANCE_ALPHA, GL_SLUMINANCE8_ALPHA8, GL_SRGB, GL_SRGB8, GL_SRGB_ALPHA, or GL_SRGB8_ALPHA8.  GL_RGBA is most common.
     * @param width Specifies the width of the texture image including the border if any. If the GL version does not support non-power-of-two sizes, this value must be 2 n + 2 ⁡ border for some integer n. All implementations support texture images that are at least 64 texels wide.
     * @param height Specifies the width of the texture image including the border if any. If the GL version does not support non-power-of-two sizes, this value must be 2 n + 2 ⁡ border for some integer n. All implementations support texture images that are at least 64 texels wide.
     * @param border Specifies the width of the border. Must be either 0 or 1.  0 is most common.
     * @param format Specifies the format of the pixel data. The following symbolic values are accepted: GL_COLOR_INDEX, GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA, GL_RGB, GL_BGR, GL_RGBA, GL_BGRA, GL_LUMINANCE, and GL_LUMINANCE_ALPHA.  GL_RGBA is most common.
     *
     * @see glTexImage2D
     */
    Texture& set(GLenum target, int level, GLint internal_format, GLsizei width, GLsizei height, int border, GLenum format, const GLfloat* data = 0);

    Texture& set(GLenum target, int level, GLint internal_format, GLsizei width, GLsizei height, int border, GLenum format, const GLint* data = 0);

    Texture& set(GLenum target, int level, GLint internal_format, GLsizei width, GLsizei height, int border, GLenum format, const GLuint* data = 0);

    Texture& set(GLenum target, int level, GLint internal_format, GLsizei width, GLsizei height, int border, GLenum format, const GLubyte* data = 0);

    Texture& set(GLenum target, int level, GLint internal_format, GLsizei width, GLsizei height, int border, GLenum format, const GLushort* data = 0);

    /**
     * Construct a texture and initialize it from an array.
     *
     * This simplified version uses defaults for target, level, and border:
     *  <li> target = GL_TEXTURE_2D
     *  <li> level = 0
     *  <li> border = 0
     *
     *  See the full version for full documentation of the parameters.
     */
    template <class T>
    Texture& set(GLsizei width, GLsizei height, GLenum format, const T* data = 0)
    {
        return set(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, data);
    }

    /**
     * Construct an unitialized texture.  Useful when using a texture
     * as a rendering destination
     */
    Self& allocate(GLenum internal_format, GLsizei width, GLsizei height);

    /**
     * Construct an unitialized texture.  Useful when using a texture
     * as a rendering destination
     */
    Self& allocate(GLenum internal_format, GLsizei width, GLsizei height, GLenum format);

    /**
     * Construct a texture and initialize it with image pixel data.
     *
     * This simplified version uses defaults for target, level, and border:
     *  <li> target = GL_TEXTURE_2D
     *  <li> level = 0
     *  <li> border = 0
     *
     */
    Texture& set(const Image& img);

    /**
     * Construct a texture and initialize it with image pixel data.
     *
     * A thin wrapper for glTexImage2D.  If you don't care about the details,
     * use the simpler version, init(img).
     */
    Texture& set(
            const Image& img,
            GLenum target,
            GLint level,
            GLint border = 0 );


    /**
     * Construct a texture and initialize it iwth a grayscale image.
     *
     * This simplified version uses defaults for target, level, and border:
     *  
     *  <li> target = GL_TEXTURE_2D
     *  <li> level = 0
     *  <li> border = 0
     *
     *  @warning untested
     */
    Texture& set(const Matrix& mat);


    /**
     * Construct a texture and initialize it with a grayscale image.
     *
     * Values are assumed to be in the range [0,255], as produced by
     * Image::to_grayscale_matrix().  This function first normalizes to 
     * [0,1], converts to an array and passes to glTexImage2D.
     *
     * @warning untested
     */
    Texture& set(const Matrix& mat, GLenum target, GLenum level, GLint border);


    Self& set_float(const Matrix& mat, GLenum target = GL_TEXTURE_2D);



    /**
     * Construct a texture and initialize it with the values from a matrix.
     * This differs from init(matrix) by converting all values to 32-bit floats and
     * packing the float into an BGRA value.  The user must be very careful
     * to ensure the exact BGRA values are preserved when rendering this texture,
     * otherwise the underlying float will be corrupted.  In particular:
     *  
     *  <li> min filter and max filter must be GL_NEAREST to avoid interpolation of color values
     *  <li> glTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE) must be set to ensure no blending occurs and that this texture's alpha value is preserved.
     *  <li> glutDisplayMode(GL_RGBA | GLUT_ALPHA | ...) must be set (as opposed to GL_RGB).  This ensures color buffer has an alpha channel.
     *  <li> the graphics hardware should be checked to ensure it supports alpha channel color buffers. The glGetIntegerv(GL_ALPHA_BITS, ...) function should return 8.
     *  <li> if reading with glReadPixels(), use GL_BGRA for format (not GL_RGBA, see note below), GL_UNSIGNED_BYTE for type, and use an array of unsigned bytes to read into.  Then, cast the array to a float pointer to get the float data out.
     *  <li> If reading values in a shader or cuda, take care that endian-ness is the same on device as the host cpu.  If not, you'll probably need to reverse the byte-order before casting to a float (I haven't tested this though.  YMMV).
     *
     *  @note Note that the packing format is GL_BGRA, not GL_RGBA.  As BGRA is the native storage format, this results in a huge speed (>2x) increase when reading the bytes back out using glReadPixels.  Just make sure to use GL_BGRA when calling glReadPixels, too.
     *
     */
    Self& set_packed_float(const Matrix& mat, GLenum target = GL_TEXTURE_2D);


    /**
     * Create a 4-channel "mask" sprite:
     *   (R,G,B,A) = (0,0,0,0) ; If matrix value == 0.
     *   (R,G,B,A) = (255,255,255,255) ; otherwise
     */
    template <class Matrix_type>
    Self& set_mask_4ui(const Matrix_type& mat, GLenum target = GL_TEXTURE_2D)
    {
        GLubyte* array = create_gl_mask_4ui(mat);

        set(target, 0, GL_RGBA8, mat.get_num_cols(), mat.get_num_rows(), 0, GL_RGBA, array);

        delete[] array;
        return *this;
    }

    /**
     * Create a single-channel "mask" sprite:
     *   RED = 0; If matrix value == 0.
     *   RED = 255 ; otherwise
     */
    Self& set_mask_1f(const kjb::Matrix& mat, GLenum target = GL_TEXTURE_2D);


    /**
     * Create a single-channel "mask" sprite:
     *   RED = 0; If matrix value == 0.
     *   RED = 255 ; otherwise
     */
    Self& set_mask_1f(const kjb::Int_matrix& mat, GLenum target = GL_TEXTURE_2D);
    


    ~Texture();

    /**
     * Set this as the active texture object
     */
    void bind() const;
    
    /**
     * revert to no texture being active
     *
     * @warning If another texture is active, this will deactivate it.
     */
    void unbind() const;

    static int max_color_attachments();

    /**
     * Implicit conversion to GLUint.
     * This allows the object to be passed to native OpenGL routines.
     */
    operator GLuint() const;

    GLuint get() const;


protected:

    /**
     * Convert to a float array, with increasing indices correpsonding
     * to increasing column, with bottom row first, and top row last.
     *
     * This corresponds to opengl's texture format.  Note that values
     * are unchanged, so if you will be using these values as RGBA values
     * you should first normalize to the range [0.0, 1.0].  
     */
    static float* create_gl_array(const Matrix& mat);

    /**
     * Create a 4-channel unsigned int (4ui) array suitable for passing to glTexImage2D such that:
     *  (R,G,B,A) = (0,0,0,0) ; if matrix value is 0
     *  (R,G,B,A) = (255,255,255,255) ; otherwise
     *
     *  Works with both Int_matrix and Matrix.
     *
     *  Caller is responsible for deleting.
     */
    template <class Matrix_type>
    static GLubyte* create_gl_mask_4ui(const Matrix_type& mat)
    {
        GLubyte* array = new GLubyte[mat.get_length() * 4];
        GLubyte* cur = array;

        const int num_rows = mat.get_num_rows();
        const int num_cols = mat.get_num_cols();

        for(int row = num_rows - 1; row >= 0; row--)
        for(int col = 0; col < num_cols; col++)
        {
            *cur++ = (mat(row, col) == 0 ? 0 : 255);
            *cur++ = (mat(row, col) == 0 ? 0 : 255);
            *cur++ = (mat(row, col) == 0 ? 0 : 255);
            *cur++ = (mat(row, col) == 0 ? 0 : 255);
        }

        return array;
    }

private:
    template <class T>
    Texture& set(
            GLenum target,
            int level,
            GLint internal_format,
            GLsizei width,
            GLsizei height,
            int border,
            GLenum format,
            const T* data = 0);
private:
    static std::map<int, int> ref_count_;

    GLuint handle_;
    int width_;
    int height_;
};

void draw_fullscreen_textured_quad(const Texture& texture);

} // namespace opengl
} // namespace kjb

#endif /* KJB_HAVE_OPENGL */
#endif /* KJB_OPENGL_TEXTURE */
