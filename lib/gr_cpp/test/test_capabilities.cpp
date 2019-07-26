/* $Id$ */
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

#ifdef KJB_HAVE_OSMESA
#include <GL/osmesa.h>
#endif


#ifndef KJB_HAVE_OPENGL
// if opengl isn't installed, just run this 
#include <iostream>
int main()
{
    std::cerr << "Can't run test -- opengl not installed." << std::endl;
    return 1;
}
#else

#include <gr_cpp/gr_offscreen.h>
#include <gr_cpp/gr_parapiped.h>
#include <gr_cpp/gr_camera.h>

#include <l_cpp/l_test.h>

using namespace kjb;
using namespace std;

float width = 640;
float height = 480;
const float NEAR = 10;
const float FAR = 10000;

int main(int argc, char* argv[])
{

    Parapiped pp(0,1,0,1,1,0,1,1,1,1,0,1);

    Vector camera_centre(3,0.0);
    camera_centre[2] = 10.0;
    Parametric_camera_gl_interface camera;
    camera.set_focal_no_aspect_ratio_no_skew(1000);
    camera.set_camera_center(camera_centre);

    bool has_gl;
    bool has_osmesa;
    kjb::retrieve_offscreen_capabilities(&has_gl, &has_osmesa);
    std::cout << "Has gl?" << has_gl << std::endl;
    std::cout << "Has osmesa?" << has_osmesa << std::endl;

    bool has_pbuffers = kjb::supports_pbuffers();
    std::cout << "Has pbuffers?" << has_pbuffers << std::endl;

    bool has_pixmaps = kjb::supports_pixmaps();
    std::cout << "Has pixmap?" << has_pixmaps << std::endl;

    Offscreen_buffer * ob = 0;
    if(has_gl)
    {
        if(has_pbuffers)
        {
            kjb_c::KJB_image * cimg = 0;
            ob = kjb::create_default_offscreen_buffer(400,300);
            ob->activate();
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glViewport(0, 0, 400, 300);
            camera.prepare_for_rendering(true);
            glColor3f(255,0,0);
            pp.wire_render();
            camera.capture_gl_view(&cimg);
            ob->deactivate();
            kjb_c::kjb_write_image(cimg, "pbuffers.tiff");
            kjb_c::kjb_free_image(cimg);
            delete ob;

        }
        else if(has_pixmaps)
        {
            kjb_c::KJB_image * cimg = 0;
            ob = kjb::create_default_offscreen_buffer(400,300,false);
            ob->activate();
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glViewport(0, 0, 400, 300);
            camera.prepare_for_rendering(true);
            glColor3f(255,0,0);
            pp.wire_render();
            camera.capture_gl_view(&cimg);
            kjb_c::kjb_write_image(cimg, "pixmaps.tiff");
            kjb_c::kjb_free_image(cimg);
            std::cout << "Create pixel map" << std::endl;
            delete ob;
        }
        else
        {
            std::cout << "Could not use neither pixelmaps nor pbuffers, they are not available" << std::endl;
        }
    }

    if(has_osmesa)
    {
        kjb_c::KJB_image * cimg = 0;
        ob = kjb::create_default_offscreen_buffer(400,300);
        ob->activate();
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glViewport(0, 0, 400, 300);
        camera.prepare_for_rendering(true);
        glColor3f(255,0,0);
        pp.wire_render();
        camera.capture_gl_view(&cimg);
        kjb_c::kjb_write_image(cimg, "osmesa.tiff");
        kjb_c::kjb_free_image(cimg);
        std::cout << "Create osmesa buffer" << std::endl;
        delete ob;
    }
    return 0;
}



#endif
