/* $Id: gr_frustum.cpp 18283 2014-11-25 05:05:59Z ksimek $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2014 by Kobus Barnard (author)
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

#ifdef KJB_HAVE_OPENGL
#include <gr_cpp/gr_frustum.h>
#include <gr_cpp/gr_opengl.h>

namespace kjb {
namespace opengl{

void Frustum_display::render() const
{
#ifdef KJB_HAVE_OPENGL
    using namespace kjb::opengl;

    glPushMatrix();
    glPushAttrib(GL_ENABLE_BIT);
    glPushAttrib(GL_CURRENT_BIT);

    glDisable(GL_LIGHTING);

    glColor3f(0.0, 0.0, 0.0);

    glTranslate(center_);
    glMultMatrix(m_inv_);

    double z = z_far_;

    if(show_frame_)
    {
        for(int i = 0; i < 2; ++i)
        {
            glBegin(GL_LINE_LOOP);
                glVertex3d(z * -width_/2.0, z * -height_/2.0, z);
                glVertex3d(z * width_/2.0, z * -height_/2.0, z);
                glVertex3d(z * width_/2.0, z * height_/2.0, z);
                glVertex3d(z * -width_/2.0, z * height_/2.0, z);
            glEnd();
            z = z_near_;
        }

        glBegin(GL_LINES);
            glVertex3d(z_near_ * -width_/2.0, z_near_ * -height_/2.0, z_near_);
            glVertex3d(z_far_ * -width_/2.0, z_far_ * -height_/2.0, z_far_);

            glVertex3d(z_near_ * width_/2.0, z_near_ * -height_/2.0, z_near_);
            glVertex3d(z_far_ * width_/2.0, z_far_ * -height_/2.0, z_far_);

            glVertex3d(z_near_ * width_/2.0, z_near_ * height_/2.0, z_near_);
            glVertex3d(z_far_ * width_/2.0, z_far_ * height_/2.0, z_far_);

            glVertex3d(z_near_ * -width_/2.0, z_near_ * height_/2.0, z_near_);
            glVertex3d(z_far_ * -width_/2.0, z_far_ * height_/2.0, z_far_);
        glEnd();
    }

    // render image data associated with this camera
    if(image_)
    {
        glPushAttrib(GL_TEXTURE_BIT);
        glEnable(GL_TEXTURE_2D);

        image_->bind();

        // don't use color and lighting; don't blend colors (other than glBlendFunc), just show the pixels as-is
        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 

        // this should be irrelevant, since the quad is the exact size of the texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
                        GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                        GL_LINEAR);

        // render on far plane
        z = z_near_;

        glEnable(GL_BLEND);
        if(image_alpha_ < 1.0)
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1.0, 1.0, 1.0, image_alpha_);
        }
        else
        {
            glColor3f(1.0, 1.0, 1.0);
        }

        glBegin(GL_QUADS);
            glNormal3d(0, 0, 1);
            glTexCoord2f(0, 0);
            glVertex3d(z * -width_/2.0, z * -height_/2.0, z);
            glTexCoord2f(1, 0);
            glVertex3d(z * width_/2.0, z * -height_/2.0, z);
            glTexCoord2f(1, 1);
            glVertex3d(z * width_/2.0, z * height_/2.0, z);
            glTexCoord2f(0, 1);
            glVertex3d(z * -width_/2.0, z * height_/2.0, z);
        glEnd();

        image_->unbind();

        glPopAttrib();
    }

    glPopAttrib();
    glPopAttrib();
    glPopMatrix();
#else
    KJB_THROW_2(Missing_dependency, "Missing library OpenGL");
#endif
}

} // namespace opengl
} // namespace kjb
#endif // KJB_HAVE_OPENGL
