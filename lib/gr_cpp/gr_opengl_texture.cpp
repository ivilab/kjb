/* $Id: gr_opengl_texture.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "gr_cpp/gr_glew.h"
#include "gr_cpp/gr_opengl.h"
#include "gr_cpp/gr_opengl_texture.h"
#include "m_cpp/m_int_matrix.h"

#ifdef KJB_HAVE_OPENGL
namespace kjb
{
namespace opengl
{

std::map<int, int> Texture::ref_count_;



/**
 * This struct maps primitive types to their GLenum value in opengl
 */
template <class T>
struct opengl_types
{
    static const GLenum type;
};

// the generic version is invalid
template <class T>
const GLenum opengl_types<T>::type = GL_INVALID_ENUM; 

template <>
struct opengl_types<GLfloat>
{
    static const GLenum type;
};

template <>
struct opengl_types<GLint>
{
    static const GLenum type;
};

template <>
struct opengl_types<GLuint>
{
    static const GLenum type;
};

template <>
struct opengl_types<GLubyte>
{
    static const GLenum type;
};

template <>
struct opengl_types<GLushort>
{
    static const GLenum type;
};




const GLenum opengl_types<GLfloat>::type = GL_FLOAT;
const GLenum GLfloat_type = GL_FLOAT;

const GLenum opengl_types<GLint>::type = GL_INT;
const GLenum GLint_type = GL_INT;

const GLenum opengl_types<GLuint>::type = GL_UNSIGNED_INT;
const GLenum GLuint_type = GL_UNSIGNED_INT;

const GLenum opengl_types<GLubyte>::type = GL_UNSIGNED_BYTE;
const GLenum GLubyte_type = GL_UNSIGNED_BYTE;

const GLenum opengl_types<GLushort>::type = GL_UNSIGNED_SHORT;
const GLenum GLushort_type = GL_UNSIGNED_SHORT;

/**
 * Create a single-channel floating point mask (i.e. 1f) where non-zero elements become 1.0.
 */
template <class Matrix_type>
float* create_gl_mask_1f(const Matrix_type& mat)
{
    float* array = new float[mat.get_length()];
    float* cur = array;

    const int num_rows = mat.get_num_rows();
    const int num_cols = mat.get_num_cols();

    for(int row = num_rows - 1; row >= 0; row--)
    for(int col = 0; col < num_cols; col++)
    {
        *cur++ = (mat(row, col) == 0 ? 0 : 1.0);
    }

    return array;
}

template <class Matrix_type>
void set_mask_1f_dispatch_(Texture& tx, const Matrix_type& mat, GLenum target = GL_TEXTURE_2D)
{
    float* array = create_gl_mask_1f(mat);


#ifdef KJB_HAVE_GLEW
    if(GLEW_ARB_texture_rg)
        tx.set(target, 0, GL_R32F, mat.get_num_cols(), mat.get_num_rows(), 0, GL_RED, array);
    else
    {
        KJB_THROW(Not_implemented);
//            KJB(UNTESTED_CODE());
//            color.allocate(GL_RED, width_, height_);
    }
#else
    KJB_THROW(Not_implemented);
//        KJB(UNTESTED_CODE());
//        color.allocate(GL_RED, width_, height_);
#endif

    delete[] array;
}

// **************  TEXTURE ********************/
Texture::Texture() :
    handle_(0),
    width_(0),
    height_(0)
{
#if defined(DEBUGGING) && defined(KJB_HAVE_GLUT)
    Glut::test_initialized("kjb::opengl::Texture");
#endif

#if defined(DEBUGGING) && defined(KJB_HAVE_GLEW)
    Glew::test_initialized("kjb::opengl::Texture");
#endif

    glGenTextures(1, &handle_);
    GL_ETX();
}

Texture::Texture(GLuint texture_id, int width, int height) :
    handle_(texture_id),
    width_(width),
    height_(height)
{
}

Texture::Texture(const Texture& other) :
    handle_(other.handle_),
    width_(other.width_),
    height_(other.height_)
{
    if(other.handle_)
    {
        ref_count_[handle_]++;
    }

}

int Texture::get_width() const
{
    return width_;
}

int Texture::get_height() const
{
    return height_;
}

int Texture::get_num_cols() const
{
    return get_width();
}

int Texture::get_num_rows() const
{
    return get_height();
}

void Texture::free()
{
    if(handle_)
    {
        ref_count_[handle_]--;
        handle_ = 0;

        ASSERT(ref_count_[handle_] >= 0);

        if(ref_count_[handle_] == 0)
        {
            // opengl delete texture
            glDeleteTextures(1, &handle_);

            GL_ETX();
        }
    }
}

Texture& Texture::operator=(const Texture& src)
{
    if (this == &src) {
        return *this;
    }

    free();

    handle_ = src.handle_;
    width_ = src.width_;
    height_ = src.height_;

    if(handle_)
    {
        ref_count_[handle_]++;
    }

    return *this;
}


template <class T>
Texture& Texture::set(
        GLenum target,
        int level,
        GLint internal_format,
        GLsizei width,
        GLsizei height,
        int border,
        GLenum format,
        const T* data)
{
//        GLenum data_type = opengl_types<T>::type;
    GLenum data_type = opengl_types<T>::type;

    if(data_type == GL_INVALID_ENUM)
    {
        KJB_THROW_2(Opengl_error, "Invalid data type for texture.");
    }

    bind();

    ::glTexImage2D(
            target,
            level,
            internal_format,
            width,
            height,
            border,
            format,
            data_type,
            data);

    unbind();

    GL_ETX();

    width_ = width;
    height_ = height;

    return *this;
}

Texture& Texture::set(GLenum target, int level, GLint internal_format, GLsizei width, GLsizei height, int border, GLenum format, const GLfloat* data)
{
    return set<GLfloat>(target, level, internal_format, width, height, border, format, data);
}


Texture& Texture::set(GLenum target, int level, GLint internal_format, GLsizei width, GLsizei height, int border, GLenum format, const GLint* data)
{
    return set<GLint>(target, level, internal_format, width, height, border, format, data);
}

Texture& Texture::set(GLenum target, int level, GLint internal_format, GLsizei width, GLsizei height, int border, GLenum format, const GLuint* data)
{
    return set<GLuint>(target, level, internal_format, width, height, border, format, data);
}

Texture& Texture::set(GLenum target, int level, GLint internal_format, GLsizei width, GLsizei height, int border, GLenum format, const GLubyte* data)
{
    return set<GLubyte>(target, level, internal_format, width, height, border, format, data);
}

Texture& Texture::set(GLenum target, int level, GLint internal_format, GLsizei width, GLsizei height, int border, GLenum format, const GLushort* data)
{
    return set<GLushort>(target, level, internal_format, width, height, border, format, data);
}

Texture& Texture::allocate(GLenum internal_format, GLsizei width, GLsizei height)
{
    return set(GL_TEXTURE_2D, 0, internal_format, width, height, 0, GL_RGBA, (GLubyte*) 0);
}

Texture& Texture::allocate(GLenum internal_format, GLsizei width, GLsizei height, GLenum format)
{
    return set(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, (GLubyte*) 0);
}

Texture& Texture::set(const Image& img)
{
    return set(img, GL_TEXTURE_2D, 0, 0);
}

Texture& Texture::set(
        const Image& img,
        GLenum target,
        GLint level,
        GLint border)
{
    bind();
    kjb::opengl::glTexImage2D(target, level, border, img);
// TODO: make a version of this that takes target, level, and border
// TODO: This creates mucked-up looking textures...
//        kjb::opengl::gluBuild2DMipmaps(img);
    unbind();
    GL_ETX();

    width_ = img.get_num_cols();
    height_ = img.get_num_rows();

    return *this;
}

Texture& Texture::set(const Matrix& mat)
{
    return set(mat, GL_TEXTURE_2D, (GLenum) 0, (GLint) 0);
}

Texture& Texture::set(const Matrix& mat, GLenum target, GLenum level, GLint border)
{
    float* array = create_gl_array(mat/255);

    KJB(UNTESTED_CODE());

    set(target, level, GL_LUMINANCE, mat.get_num_cols(), mat.get_num_rows(), border, GL_LUMINANCE, array);

    delete[] array;

    return *this;
}

Texture& Texture::set_float(const Matrix& mat, GLenum target)
{
    using kjb_c::kjb_endian_test;

    // convert matrix to float array
    float* array = create_gl_array(mat);

    const int num_rows = mat.get_num_rows();
    const int num_cols = mat.get_num_cols();

    /**
     * The code below may be necessary when working with big-endian architectures, but I'm guessing probbaly not.  In any case, I'll leave it here but commented out, to remind me that it may be an issue in the future.
     */
#if 0
    // only check this once.
    static const bool LITTLE_ENDIAN_ = !kjb_is_bigendian();
    if(!LITTLE_ENDIAN_)
    {
//            swap_array_bytes(array, num_rows * num_cols);
    }
#endif

    // can get this back out using glReadPixels with GL_UNSIGNED_BYTE and GL_BGRA.
//        set(target, 0, GL_FLOAT_R32_NV, num_cols, num_rows, 0, GL_RED, (float*) array);

#ifdef KJB_HAVE_GLEW
    if(GLEW_ARB_texture_rg)
        set(target, 0, GL_R32F, num_cols, num_rows, 0, GL_RED, (float*) array);
    else
    {
        KJB_THROW(Not_implemented);
//            KJB(UNTESTED_CODE());
//            color.allocate(GL_RED, width_, height_);
    }
#else
    KJB_THROW(Not_implemented);
    // this is trash, but added it to kill "unused parameter 'target'" warning
    set(target, 0, GL_RGB, num_cols, num_rows, 0, GL_RED, (float*) array);
//        KJB(UNTESTED_CODE());
//        color.allocate(GL_RED, width_, height_);
#endif

    delete[] array;

    return *this;

}

Texture& Texture::set_packed_float(const Matrix& mat, GLenum target)
{
    using kjb_c::kjb_endian_test;

    // convert matrix to float array
    float* array = create_gl_array(mat);


    const int num_rows = mat.get_num_rows();
    const int num_cols = mat.get_num_cols();

    /**
     * The code below may be necessary when working with big-endian architectures, but I'm guessing probbaly not.  In any case, I'll leave it here but commented out, to remind me that it may be an issue in the future.
     */
#if 0
    // only check this once.
    static const bool LITTLE_ENDIAN_ = !kjb_is_bigendian();
    if(!LITTLE_ENDIAN_)
    {
//            swap_array_bytes(array, num_rows * num_cols);
    }
#endif

    // can get this back out using glReadPixels with GL_UNSIGNED_BYTE and GL_BGRA.
    set(target, 0, GL_RGBA, num_cols, num_rows, 0, GL_BGRA, (unsigned char*) array);

    delete[] array;

    return *this;

}

Texture& Texture::set_mask_1f(const kjb::Matrix& mat, GLenum target)
{
    set_mask_1f_dispatch_(*this, mat, target);
    return *this;
}

Texture& Texture::set_mask_1f(const kjb::Int_matrix& mat, GLenum target)
{
    set_mask_1f_dispatch_(*this, mat, target);
    return *this;
}

Texture::~Texture()
{
    free();
    GL_ETX();
}

void Texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, handle_);
}


void Texture::unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

int Texture::max_color_attachments()
{
#ifdef KJB_HAVE_GLEW
    int result;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &result);
    return result;
#else
    KJB_THROW_2(Missing_dependency,"GLEW");
#endif
}

Texture::operator GLuint() const
{
    return handle_;
}

GLuint Texture::get() const
{
    return handle_;
}

float* Texture::create_gl_array(const Matrix& mat)
{
    // convert matrix to float array
    float* array = new float[mat.get_length()];
    float* cur = array;

    const int num_rows = mat.get_num_rows();
    const int num_cols = mat.get_num_cols();

    // opengl expects rows in reversed order
    for(int row = num_rows-1; row >= 0; row--)
    {
        for(int col = 0; col < num_cols; col++)
        {
            *cur++ = (float) mat(row, col);
        }
    }

    return array;
}

void draw_fullscreen_textured_quad(const Texture& texture)
{
    glPushAttrib(GL_TEXTURE_BIT);
    glPushAttrib(GL_ENABLE_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

#ifdef KJB_HAVE_GLEW
    glActiveTexture(GL_TEXTURE0);
#endif
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    texture.bind();
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
        glTexCoord2d(0.0,0.0); glVertex2f(-1.0,-1.0);
        glTexCoord2d(1.0,0.0); glVertex2f(1.0,-1.0);
        glTexCoord2d(1.0,1.0); glVertex2f(1.0,1.0);
        glTexCoord2d(0.0,1.0); glVertex2f(-1.0,1.0);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopAttrib();
    glPopAttrib();
}

} // namespace opengl
} // namespace kjb

#endif /* KJB_HAVE_OPENGL */

