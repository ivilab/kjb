/* $Id: gr_sprite.cpp 21596 2017-07-30 23:33:36Z kobus $ */
/* =========================================================================== *
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
 * =========================================================================== */

#ifdef KJB_HAVE_OPENGL
#include "gr_cpp/gr_sprite.h"
#include <map>
#include "gr_cpp/gr_opengl.h"
#include <iostream>

using namespace kjb;
using namespace kjb::opengl;
using namespace std;

map<int, int> Sprite::m_ref_count;

Sprite::Sprite(float z_pos) : 
    m_texture(),
    m_u(0),
    m_v(0),
    m_base_width(0),
    m_base_height(0),
    m_width(0),
    m_height(0),
    m_depth_test(false),
    m_min_filter(GL_NEAREST),
    m_mag_filter(GL_NEAREST),
    m_z_pos(z_pos),
    m_origin(TOP_LEFT),
    m_crop_left(0.0),
    m_crop_right(1.0),
    m_crop_bottom(0.0),
    m_crop_top(1.0)
{
    m_init_texture();
}

Sprite::Sprite(const Texture& tex, float z_pos) :
    m_texture(tex),
    m_u(0),
    m_v(0),
    m_base_width(0),
    m_base_height(0),
    m_width(0),
    m_height(0),
    m_depth_test(false),
    m_min_filter(GL_NEAREST),
    m_mag_filter(GL_NEAREST),
    m_z_pos(z_pos),
    m_origin(TOP_LEFT),
    m_crop_left(0.0),
    m_crop_right(1.0),
    m_crop_bottom(0.0),
    m_crop_top(1.0)
{
    m_init_texture();
    set(tex);
}

Sprite::Sprite(const Image& img, float z_pos) :
    m_texture(),
    m_u(0),
    m_v(0),
    m_base_width(0),
    m_base_height(0),
    m_width(0),
    m_height(0),
    m_depth_test(false),
    m_min_filter(GL_NEAREST),
    m_mag_filter(GL_NEAREST),
    m_z_pos(z_pos),
    m_origin(TOP_LEFT),
    m_crop_left(0.0),
    m_crop_right(1.0),
    m_crop_bottom(0.0),
    m_crop_top(1.0)
{
    m_init_texture();
    set(img);
}

/**
 * Construct a grayscale sprite.  As you might expect, this
 * consumes less video memory than a full-color sprite.
 */
Sprite::Sprite(const Matrix& img, float z_pos) :
    m_texture(),
    m_u(0),
    m_v(0),
    m_base_width(0),
    m_base_height(0),
    m_width(0),
    m_height(0),
    m_depth_test(false),
    m_min_filter(GL_NEAREST),
    m_mag_filter(GL_NEAREST),
    m_z_pos(z_pos),
    m_origin(TOP_LEFT),
    m_crop_left(0.0),
    m_crop_right(1.0),
    m_crop_bottom(0.0),
    m_crop_top(1.0)
{
    m_init_texture();
    set(img);
}




//Sprite::Sprite(const char* fname, float z_pos) :
//    m_texture(),
//    m_u(0),
//    m_v(0),
//    m_base_width(0),
//    m_base_height(0),
//    m_width(0),
//    m_height(0),
//    m_depth_test(false),
//    m_min_filter(GL_NEAREST),
//    m_mag_filter(GL_NEAREST),
//    m_z_pos(z_pos),
//    m_origin(TOP_LEFT),
//    m_crop_left(0.0),
//    m_crop_right(1.0),
//    m_crop_bottom(0.0),
//    m_crop_top(1.0)
//{
//    m_init_texture();
//    m_set(Image(fname));
//}


Sprite::Sprite(const Sprite& src) :
    Renderable(),
    m_texture(src.m_texture),
    m_u(src.m_u),
    m_v(src.m_v),
    m_base_width(src.m_base_width),
    m_base_height(src.m_base_height),
    m_width(src.m_width),
    m_height(src.m_height),
    m_depth_test(src.m_depth_test),
    m_min_filter(src.m_min_filter),
    m_mag_filter(src.m_mag_filter),
    m_z_pos(src.m_z_pos),
    m_origin(TOP_LEFT),
    m_crop_left(0.0),
    m_crop_right(1.0),
    m_crop_bottom(0.0),
    m_crop_top(1.0)
{
    m_init_texture();
}

Sprite::~Sprite()
{
}



void Sprite::enable_depth_test(bool enable)
{   
    m_depth_test = enable;
}



void Sprite::set_filters(GLenum min_filter, GLenum mag_filter)
{
    switch(mag_filter)
    {
        case GL_NEAREST:
        case GL_LINEAR:
            break;
        default:
            KJB_THROW(Illegal_argument);
            break;

    }

    switch(min_filter)
    {
        case GL_NEAREST:
        case GL_LINEAR:
            break;
        case GL_NEAREST_MIPMAP_NEAREST:
        case GL_LINEAR_MIPMAP_NEAREST:
        case GL_NEAREST_MIPMAP_LINEAR:
        case GL_LINEAR_MIPMAP_LINEAR:
            KJB_THROW(Not_implemented);
            break;
        default:
            KJB_THROW(Illegal_argument);

    }

    m_min_filter = min_filter;
    m_mag_filter = mag_filter;
}


void Sprite::render() const
{
    // TODO save/restore state
    
    struct {
        int x;
        int y;
        int width;
        int height;
    } vp;

    glGetIntegerv(GL_VIEWPORT, (GLint*) &vp);

    glPushAttrib(GL_DEPTH_BUFFER_BIT);

    // set modelview to identity
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // set projection to screen coordinates
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    ::glOrtho(0, vp.width, vp.height, 0, -1, 1);
    // Traditional u,v coordinate system: 
    // origin at top-left, positive y direction is down

    glEnable( GL_TEXTURE_2D);
    m_texture.bind();
// TODO: set this in the member function
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
                    m_mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                    m_min_filter);

    if(!m_depth_test)
    {
        glDisable(GL_DEPTH_TEST);
    }

    float crop_width = (m_crop_right - m_crop_left) * m_width;
    float crop_height = (m_crop_top - m_crop_bottom) * m_height;

    // for origin = TOP_LEFT
    float left =  m_u;
    float right = m_u + crop_width;
    float top = m_v;
    float bottom = m_v + crop_height;

    // for origin = CENTER
    if(m_origin == CENTER)
    {
        left -= crop_width / 2;
        right -= crop_width / 2;
        top -= crop_height / 2;
        bottom -= crop_height / 2;
    }

    glColor3i(0,0,0);
    glBegin(GL_QUADS);
     glTexCoord2d(m_crop_left,  m_crop_bottom);
     glVertex3f(left, bottom, m_z_pos);
     glTexCoord2d(m_crop_right, m_crop_bottom);
     glVertex3f(right, bottom, m_z_pos);
     glTexCoord2d(m_crop_right, m_crop_top);
     glVertex3f(right, top, m_z_pos);
     glTexCoord2d(m_crop_left, m_crop_top);
     glVertex3f(left, top, m_z_pos);
    glEnd();
    glEnable(GL_DEPTH_TEST); // better to save/restore this?
    glDisable(GL_TEXTURE_2D);

    m_texture.unbind();

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glPopAttrib();

    GL_ETX();
}

void Sprite::render_quad() const
{
    glEnable( GL_TEXTURE_2D);
    m_texture.bind();
// TODO: set this in the member function
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
                    m_mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                    m_min_filter);

    glColor3i(0,0,0);
    glBegin(GL_QUADS);
     glTexCoord2d(0,  1);
     glVertex3f(0, 0, 0);
     glTexCoord2d(1, 1);
     glVertex3f(1, 0, 0);
     glTexCoord2d(1, 0);
     glVertex3f(1, 1, 0);
     glTexCoord2d(0, 0);
     glVertex3f(0, 1, 0);
    glEnd();
    m_texture.unbind();

    GL_ETX();
}

Sprite& Sprite::operator=(const Sprite& src)
{
    if (this == &src) {
        return *this;
    }

    m_texture = src.m_texture;
    m_u = src.m_u;
    m_v = src.m_v;
    m_base_width = src.m_base_width;
    m_base_height = src.m_base_height;
    m_width = src.m_width;
    m_height = src.m_height;
    m_depth_test = src.m_depth_test;
    m_min_filter = src.m_min_filter;
    m_mag_filter = src.m_mag_filter;
    m_z_pos = src.m_z_pos;

    return *this;
}


/**
 * Initialize opengl texture properties so sprites
 * appear correctly.
 */
void Sprite::m_init_texture()
{
    glEnable( GL_TEXTURE_2D);


    // TODO save and restore bound texture state
    m_texture.bind();

    // don't use color and lighting; don't blend colors (other than glBlendFunc), just show the pixels as-is
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 

    // this should be irrelevant, since the quad is the exact size of the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    m_texture.unbind();

    // throw exception if error occurred
    GL_ETX();
}

/**
 * @pre Currently bound texture is this one
 */
void Sprite::set(const Image& img)
{

    m_width  = m_base_width  = img.get_num_cols();
    m_height = m_base_height = img.get_num_rows();

    m_texture.set(img);
}

/**
 * @pre Currently bound texture is this one
 */
void Sprite::set(const Matrix& img)
{

    m_width  = m_base_width  = img.get_num_cols();
    m_height = m_base_height = img.get_num_rows();

    m_texture.set(img);
}

/**
 * Set the texture bound to this object.
 *
 * This will reset any existing width or height to the native
 * dimensions of the texture.
 */
void Sprite::set(const Texture& tex)
{

    m_width  = m_base_width  = tex.get_width();
    m_height = m_base_height = tex.get_height();

    m_texture = tex;

    // set up texture properties
    m_init_texture();
}

#endif /* KJB_HAVE_OPENGL */

