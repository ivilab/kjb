/* $Id: gr_opengl_object.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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

#include "gr_cpp/gr_opengl.h"
#include "gr_cpp/gr_glew.h"
#include "gr_cpp/gr_opengl_object.h"
#include "gr_cpp/gr_opengl_texture.h"

#ifdef KJB_HAVE_OPENGL
namespace kjb
{
namespace opengl
{

#ifdef KJB_HAVE_GLEW
Renderbuffer::Renderbuffer(GLuint renderbuffer_id) :
    handle_(renderbuffer_id)
{

}

Renderbuffer::Renderbuffer() :
    handle_(0)
{
#if defined(DEBUGGING) && defined(KJB_HAVE_GLUT)
    Glut::test_initialized("kjb::opengl::Renderbuffer");
#endif

#ifdef DEBUGGING
    Glew::test_initialized("kjb::opengl::Renderbuffer");
#endif

    glGenRenderbuffers(1, &handle_);
}

void Renderbuffer::allocate(GLenum internal_format, GLsizei width, GLsizei height)
{
    glBindRenderbuffer(GL_RENDERBUFFER, handle_);
    glRenderbufferStorage(GL_RENDERBUFFER, internal_format, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    GL_EPETE();
    GL_ETX();
}

Renderbuffer::~Renderbuffer()
{
    glDeleteRenderbuffers(1, &handle_);
    GL_ETX();
}

void Renderbuffer::bind() const
{
    glBindRenderbuffer(GL_RENDERBUFFER, handle_);
    GL_ETX();
}

void Renderbuffer::unbind() const
{
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    GL_ETX();
}

Renderbuffer::operator GLuint() const
{
    return handle_;
}


GLuint Renderbuffer::get() const
{
    return handle_;
}
#endif //KJB_HAVE_GLEW

#ifdef KJB_HAVE_GLEW
Framebuffer_object::Framebuffer_object() :
    handle_(0)
{
#if defined(DEBUGGING) && defined(KJB_HAVE_GLUT)
    Glut::test_initialized("kjb::opengl::Framebuffer_object");
#endif

#ifdef DEBUGGING
    Glew::test_initialized("kjb::opengl::Framebuffer_object");
#endif

    glGenFramebuffers(1, &handle_);

    GL_ETX();
}

Framebuffer_object::Framebuffer_object(GLuint fbo_id) : 
    handle_(fbo_id)
{

}

Framebuffer_object::~Framebuffer_object()
{
    glDeleteFramebuffers(1, &handle_);
}

void Framebuffer_object::attach(const Renderbuffer& renderbuffer, GLenum target)
{
    glBindFramebuffer(GL_FRAMEBUFFER, handle_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, target, GL_RENDERBUFFER, renderbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GL_ETX();
}

void Framebuffer_object::attach(const Texture& texture, GLenum target)
{
    glBindFramebuffer(GL_FRAMEBUFFER, handle_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, target, GL_TEXTURE_2D, texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GL_ETX();
}

void Framebuffer_object::attach_color(const Renderbuffer& renderbuffer, int i)
{
    switch(i)
    {
        case 0:
            attach(renderbuffer, GL_COLOR_ATTACHMENT0);
            return;
        case 1:
            attach(renderbuffer, GL_COLOR_ATTACHMENT1);
            return;
        case 2:
            attach(renderbuffer, GL_COLOR_ATTACHMENT2);
            return;
        case 3:
            attach(renderbuffer, GL_COLOR_ATTACHMENT3);
            return;
        case 4:
            attach(renderbuffer, GL_COLOR_ATTACHMENT4);
            return;
        case 5:
            attach(renderbuffer, GL_COLOR_ATTACHMENT5);
            return;
        case 6:
            attach(renderbuffer, GL_COLOR_ATTACHMENT6);
            return;
        case 7:
            attach(renderbuffer, GL_COLOR_ATTACHMENT7);
            return;
        case 8:
            attach(renderbuffer, GL_COLOR_ATTACHMENT8);
            return;
        case 9:
            attach(renderbuffer, GL_COLOR_ATTACHMENT9);
            return;
        case 10:
            attach(renderbuffer, GL_COLOR_ATTACHMENT10);
            return;
        case 11:
            attach(renderbuffer, GL_COLOR_ATTACHMENT11);
            return;
        case 12:
            attach(renderbuffer, GL_COLOR_ATTACHMENT12);
            return;
        case 13:
            attach(renderbuffer, GL_COLOR_ATTACHMENT13);
            return;
        case 14:
            attach(renderbuffer, GL_COLOR_ATTACHMENT14);
            return;
        case 15:
            attach(renderbuffer, GL_COLOR_ATTACHMENT15);
            return;
        default:
            KJB_THROW_2(Illegal_argument, "Maximum of 15 color attachments allowed for FBOs");
    }

}

void Framebuffer_object::attach_depth(const Renderbuffer& renderbuffer)
{
    attach(renderbuffer, GL_DEPTH_ATTACHMENT);
}

void Framebuffer_object::attach_stencil(const Renderbuffer& renderbuffer)
{
    attach(renderbuffer, GL_STENCIL_ATTACHMENT);
}


void Framebuffer_object::attach_color(const Texture& texture, int i)
{
    switch(i)
    {
        case 0:
            attach(texture, GL_COLOR_ATTACHMENT0);
            return;
        case 1:
            attach(texture, GL_COLOR_ATTACHMENT1);
            return;
        case 2:
            attach(texture, GL_COLOR_ATTACHMENT2);
            return;
        case 3:
            attach(texture, GL_COLOR_ATTACHMENT3);
            return;
        case 4:
            attach(texture, GL_COLOR_ATTACHMENT4);
            return;
        case 5:
            attach(texture, GL_COLOR_ATTACHMENT5);
            return;
        case 6:
            attach(texture, GL_COLOR_ATTACHMENT6);
            return;
        case 7:
            attach(texture, GL_COLOR_ATTACHMENT7);
            return;
        case 8:
            attach(texture, GL_COLOR_ATTACHMENT8);
            return;
        case 9:
            attach(texture, GL_COLOR_ATTACHMENT9);
            return;
        case 10:
            attach(texture, GL_COLOR_ATTACHMENT10);
            return;
        case 11:
            attach(texture, GL_COLOR_ATTACHMENT11);
            return;
        case 12:
            attach(texture, GL_COLOR_ATTACHMENT12);
            return;
        case 13:
            attach(texture, GL_COLOR_ATTACHMENT13);
            return;
        case 14:
            attach(texture, GL_COLOR_ATTACHMENT14);
            return;
        case 15:
            attach(texture, GL_COLOR_ATTACHMENT15);
            return;
        default:
            KJB_THROW_2(Illegal_argument, "Maximum of 15 color attachments allowed for FBOs");
    }

}

void Framebuffer_object::attach_depth(const Texture& texture)
{
    attach(texture, GL_DEPTH_ATTACHMENT);
}

void Framebuffer_object::attach_stencil(const Texture& texture)
{
    attach(texture, GL_STENCIL_ATTACHMENT);
}


void Framebuffer_object::check() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, handle_);
    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    switch(status)
    {
    case GL_FRAMEBUFFER_COMPLETE:
        return;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        throw Opengl_error(
                GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                "Framebuffer incomplete: Attachment is NOT complete.",
                __FILE__,
                __LINE__);
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        throw Opengl_error(
                GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
                "Framebuffer incomplete: No image is attached to FBO.",
                __FILE__,
                __LINE__);
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        throw Opengl_error(
                GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT,
                "Framebuffer incomplete: Attached images have different dimensions.",
                __FILE__,
                __LINE__);
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        throw Opengl_error(
                GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT,
                "Framebuffer incomplete: Color attached images have different internal formats.",
                __FILE__,
                __LINE__);
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        throw Opengl_error(
                GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
                "Framebuffer incomplete: Draw buffer.",
                __FILE__,
                __LINE__);
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        throw Opengl_error(
                GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
                "Framebuffer incomplete: Read buffer.",
                __FILE__,
                __LINE__);
    case GL_FRAMEBUFFER_UNSUPPORTED:
        throw Opengl_error(
                GL_FRAMEBUFFER_UNSUPPORTED,
                "Unsupported by FBO implementation.",
                __FILE__,
                __LINE__);
    default:
        throw Opengl_error(
                "Unknown error with framebuffer.",
                __FILE__,
                __LINE__);
    }

}

void Framebuffer_object::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, handle_);
}

void Framebuffer_object::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer_object::operator GLuint() const
{
    return handle_;
}

GLuint Framebuffer_object::get() const
{
    return handle_;
}
#endif // KJB_HAVE_GLEW

#ifdef KJB_HAVE_GLEW
Buffer::Buffer(GLuint buffer_id) :
    handle_(buffer_id),
    type_(0)
{}

Buffer::Buffer() :
    handle_(0),
    type_(0)
{
#if defined(DEBUGGING) && defined(KJB_HAVE_GLUT)
    Glut::test_initialized("kjb::opengl::Buffer");
#endif

#ifdef DEBUGGING
    Glew::test_initialized("kjb::opengl::Buffer");
#endif

    glGenBuffers(1, &handle_);
    GL_ETX();
}

void Buffer::allocate(GLenum type, int size, GLvoid* data, GLenum usage_hint)
{
    type_ = type;

    bind();
    glBufferData(type_, size, data, usage_hint);
    unbind();

    GL_ETX();
}

void Buffer::allocate(GLenum type, int size, GLenum usage)
{
    allocate(type, size, 0, usage);
}

void Buffer::copy(size_t offset, size_t size, GLvoid* data)
{
    glBufferSubData(handle_, offset, size, data);
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &handle_);
    GL_ETX();
}

void Buffer::bind() const
{
    glBindBuffer(type_, handle_);
}

void Buffer::bind(GLenum type) const
{
    glBindBuffer(type, handle_);
}

void Buffer::unbind() const
{
    glBindBuffer(type_, 0);
}

void Buffer::unbind(GLenum type) const
{
    glBindBuffer(type, 0);
}

Buffer::operator GLuint() const
{
    return handle_;
}

GLuint Buffer::get() const
{
    return handle_;
}

void allocate_grayscale_color_buffer(::kjb::opengl::Renderbuffer& color, int width, int height)
{
    if(GLEW_ARB_texture_rg)
        color.allocate(GL_R32F, width, height);
    else
    {
        KJB_THROW(Not_implemented);
//            KJB(UNTESTED_CODE());
//            color.allocate(GL_RED, width_, height_);
    }

}
#endif //KJB_HAVE_GLEW

/**********************
 * The functions below are poached from songho.ca 
 *
 * Eventually, this status querying functionality would be nice to inorporate into the API/
 */

#if 0
std::string convertInternalFormatToString(GLenum format)
{
    std::string formatName;

    switch(format)
    {
    case GL_STENCIL_INDEX:
        formatName = "GL_STENCIL_INDEX";
        break;
    case GL_DEPTH_COMPONENT:
        formatName = "GL_DEPTH_COMPONENT";
        break;
    case GL_ALPHA:
        formatName = "GL_ALPHA";
        break;
    case GL_RGB:
        formatName = "GL_RGB";
        break;
    case GL_RGBA:
        formatName = "GL_RGBA";
        break;
    case GL_LUMINANCE:
        formatName = "GL_LUMINANCE";
        break;
    case GL_LUMINANCE_ALPHA:
        formatName = "GL_LUMINANCE_ALPHA";
        break;
    case GL_ALPHA4:
        formatName = "GL_ALPHA4";
        break;
    case GL_ALPHA8:
        formatName = "GL_ALPHA8";
        break;
    case GL_ALPHA12:
        formatName = "GL_ALPHA12";
        break;
    case GL_ALPHA16:
        formatName = "GL_ALPHA16";
        break;
    case GL_LUMINANCE4:
        formatName = "GL_LUMINANCE4";
        break;
    case GL_LUMINANCE8:
        formatName = "GL_LUMINANCE8";
        break;
    case GL_LUMINANCE12:
        formatName = "GL_LUMINANCE12";
        break;
    case GL_LUMINANCE16:
        formatName = "GL_LUMINANCE16";
        break;
    case GL_LUMINANCE4_ALPHA4:
        formatName = "GL_LUMINANCE4_ALPHA4";
        break;
    case GL_LUMINANCE6_ALPHA2:
        formatName = "GL_LUMINANCE6_ALPHA2";
        break;
    case GL_LUMINANCE8_ALPHA8:
        formatName = "GL_LUMINANCE8_ALPHA8";
        break;
    case GL_LUMINANCE12_ALPHA4:
        formatName = "GL_LUMINANCE12_ALPHA4";
        break;
    case GL_LUMINANCE12_ALPHA12:
        formatName = "GL_LUMINANCE12_ALPHA12";
        break;
    case GL_LUMINANCE16_ALPHA16:
        formatName = "GL_LUMINANCE16_ALPHA16";
        break;
    case GL_INTENSITY:
        formatName = "GL_INTENSITY";
        break;
    case GL_INTENSITY4:
        formatName = "GL_INTENSITY4";
        break;
    case GL_INTENSITY8:
        formatName = "GL_INTENSITY8";
        break;
    case GL_INTENSITY12:
        formatName = "GL_INTENSITY12";
        break;
    case GL_INTENSITY16:
        formatName = "GL_INTENSITY16";
        break;
    case GL_R3_G3_B2:
        formatName = "GL_R3_G3_B2";
        break;
    case GL_RGB4:
        formatName = "GL_RGB4";
        break;
    case GL_RGB5:
        formatName = "GL_RGB4";
        break;
    case GL_RGB8:
        formatName = "GL_RGB8";
        break;
    case GL_RGB10:
        formatName = "GL_RGB10";
        break;
    case GL_RGB12:
        formatName = "GL_RGB12";
        break;
    case GL_RGB16:
        formatName = "GL_RGB16";
        break;
    case GL_RGBA2:
        formatName = "GL_RGBA2";
        break;
    case GL_RGBA4:
        formatName = "GL_RGBA4";
        break;
    case GL_RGB5_A1:
        formatName = "GL_RGB5_A1";
        break;
    case GL_RGBA8:
        formatName = "GL_RGBA8";
        break;
    case GL_RGB10_A2:
        formatName = "GL_RGB10_A2";
        break;
    case GL_RGBA12:
        formatName = "GL_RGBA12";
        break;
    case GL_RGBA16:
        formatName = "GL_RGBA16";
        break;
    default:
        formatName = "Unknown Format";
    }

    return formatName;
}

std::string getRenderbufferParameters(GLuint id)
{
    if(glIsRenderbuffer(id) == GL_FALSE)
        return "Not Renderbuffer object";

    int width, height, format;
    std::string formatName;
    glBindRenderbuffer(GL_RENDERBUFFER, id);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);    // get renderbuffer width
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);  // get renderbuffer height
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &format); // get renderbuffer internal format
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    formatName = convertInternalFormatToString(format);

    std::stringstream ss;
    ss << width << "x" << height << ", " << formatName;
    return ss.str();
}

std::string getTextureParameters(GLuint id)
{
    if(glIsTexture(id) == GL_FALSE)
        return "Not texture object";

    int width, height, format;
    std::string formatName;
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);            // get texture width
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);          // get texture height
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format); // get texture internal format
    glBindTexture(GL_TEXTURE_2D, 0);

    formatName = convertInternalFormatToString(format);

    std::stringstream ss;
    ss << width << "x" << height << ", " << formatName;
    return ss.str();
}


void printFramebufferInfo()
{
    cout << "\n***** FBO STATUS *****\n";

    // print max # of colorbuffers supported by FBO
    int colorBufferCount = 0;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &colorBufferCount);
    cout << "Max Number of Color Buffer Attachment Points: " << colorBufferCount << endl;

    int objectType;
    int objectId;

    // print info of the colorbuffer attachable image
    for(int i = 0; i < colorBufferCount; ++i)
    {
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                                 GL_COLOR_ATTACHMENT0+i,
                                                 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                                 &objectType);
        if(objectType != GL_NONE)
        {
            glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                                     GL_COLOR_ATTACHMENT0+i,
                                                     GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                                     &objectId);

            std::string formatName;

            cout << "Color Attachment " << i << ": ";
            if(objectType == GL_TEXTURE)
                cout << "GL_TEXTURE, " << getTextureParameters(objectId) << endl;
            else if(objectType == GL_RENDERBUFFER)
                cout << "GL_RENDERBUFFER, " << getRenderbufferParameters(objectId) << endl;
        }
    }

    // print info of the depthbuffer attachable image
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                             GL_DEPTH_ATTACHMENT,
                                             GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                             &objectType);
    if(objectType != GL_NONE)
    {
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                                 GL_DEPTH_ATTACHMENT,
                                                 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                                 &objectId);

        cout << "Depth Attachment: ";
        switch(objectType)
        {
        case GL_TEXTURE:
            cout << "GL_TEXTURE, " << getTextureParameters(objectId) << endl;
            break;
        case GL_RENDERBUFFER:
            cout << "GL_RENDERBUFFER, " << getRenderbufferParameters(objectId) << endl;
            break;
        }
    }

    // print info of the stencilbuffer attachable image
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                             GL_STENCIL_ATTACHMENT,
                                             GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                             &objectType);
    if(objectType != GL_NONE)
    {
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                                 GL_STENCIL_ATTACHMENT,
                                                 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                                 &objectId);

        cout << "Stencil Attachment: ";
        switch(objectType)
        {
        case GL_TEXTURE:
            cout << "GL_TEXTURE, " << getTextureParameters(objectId) << endl;
            break;
        case GL_RENDERBUFFER:
            cout << "GL_RENDERBUFFER, " << getRenderbufferParameters(objectId) << endl;
            break;
        }
    }

    cout << endl;
}

#endif /* if 0 */


} // namespace kjb

} // namespace opengl
#endif /*KJB_HAVE_OPENGL */
