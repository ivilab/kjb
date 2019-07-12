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
|     Luca Del Pero
|
* =========================================================================== */

#include <glut_cpp/glut_perspective_camera.h>
#include <gr_cpp/gr_opengl.h>
#include <iostream>

using namespace kjb;

bool Glut_perspective_camera::camera_enabled;
unsigned int Glut_perspective_camera::selected_parameter;
Perspective_camera * Glut_perspective_camera::camera;
unsigned char Glut_perspective_camera::increment_character;
unsigned char Glut_perspective_camera::decrement_character;
kjb::Vector Glut_perspective_camera::increment_steps;
bool Glut_perspective_camera::centre_when_scaling_viewport;

/*
 * @param icamera_submenu_callback The user defined callback that will be executed
 *        any time a camera submenu entry is selected
 * @param pc The perspective camera associated with this submenu
 * @param enable_translation specifies whether to add entries for translating the camera to the submenu
 * @param enable_pitch specifies whether to add entries for changing the pitch to the submenu
 * @param enable_yaw specifies whether to add entries for changing the yaw to the submenu
 * @param enable_roll specifies whether to add entries for changing the roll to the submenu
 * @param enable_focal_length specifies whether to add entries for changing the focal length to the submenu
 * @param enable_principal_point specifies whether to add entries for changing the principal point to the submenu
 * @param enable_skew specifies whether to add entries for changing the skew to the submenu
 * @param enable_aspect_ratio specifies whether to add entries for changing the aspect ratio to the submenu
 * @param enable_clipping specifies whether to add entries for changing the clipping planes to the submenu
 * @param enable_viewport specifies whether to add entries for changing the viewport size to the submenu
 * @param enable_scale sspecifies whether to add an entry fro changing the world scale to this submenu
 * @param icentre_when_scaling_viewport specifies whether to centre the viewport when scaling or whether
 *        to keep it where it is
 * @param iincrement_char The character that will trigger an increment of the selected parameter. Default is 'k'
 * @param idecrement_char The character that will trigger an decrement of the selected parameter. Default is 'j'
 */
int Glut_perspective_camera::create_glut_perspective_camera_submenu(void (*icamera_submenu_callback)(int i), Perspective_camera *pc,
        bool enable_translation, bool enable_pitch, bool enable_yaw, bool enable_roll, bool enable_focal_length,
        bool enable_principal_point, bool enable_skew, bool enable_aspect_ratio,bool enable_clipping, bool enable_viewport,
        bool enable_world_scale,
        bool icentre_when_scaling_viewport, unsigned char iincrement_char, unsigned char idecrement_char)
{
#ifdef KJB_HAVE_GLUT
    int camera_menu = glutCreateMenu(camera_submenu_glut);
    if(enable_translation)
    {
        glutAddMenuEntry("Translate X", GLUT_PC_CENTRE_X);
        glutAddMenuEntry("Translate Y", GLUT_PC_CENTRE_Y);
        glutAddMenuEntry("Translate Z", GLUT_PC_CENTRE_Z);
    }
    if(enable_pitch)
    {
        glutAddMenuEntry("Pitch", GLUT_PC_PITCH);
    }
    if(enable_yaw)
    {
        glutAddMenuEntry("Yaw", GLUT_PC_YAW);
    }
    if(enable_roll)
    {
        glutAddMenuEntry("Roll", GLUT_PC_ROLL);
    }
    if(enable_focal_length)
    {
        glutAddMenuEntry("Focal length", GLUT_PC_FOCAL_LENGTH);
    }
    if(enable_principal_point)
    {
        glutAddMenuEntry("Principal point X", GLUT_PC_PRINCIPAL_POINT_X);
        glutAddMenuEntry("Principal point Y", GLUT_PC_PRINCIPAL_POINT_Y);
    }
    if(enable_skew)
    {
        glutAddMenuEntry("Skew", GLUT_PC_SKEW);
    }
    if(enable_aspect_ratio)
    {
        glutAddMenuEntry("Aspect ratio", GLUT_PC_ASPECT_RATIO);
    }
    if(enable_world_scale)
    {
        glutAddMenuEntry("World scale", GLUT_PC_WORLD_SCALE);
    }
    if(enable_clipping)
    {
        glutAddMenuEntry("Near clipping", GLUT_PC_NEAR);
        glutAddMenuEntry("Far clipping", GLUT_PC_FAR);
    }
    if(enable_viewport)
    {
        glutAddMenuEntry("Viewport width", GLUT_PC_WIDTH);
        glutAddMenuEntry("Viewport height", GLUT_PC_HEIGHT);
    }

    camera_submenu_callback = icamera_submenu_callback;
    camera = pc;
    centre_when_scaling_viewport = icentre_when_scaling_viewport;

    camera_enabled = true;

    increment_character = iincrement_char;
    decrement_character = idecrement_char;

    /** The increment steps are all initialized to 1.0, except for th
     * viewport width and height, which we initialize to an even number
     */
    increment_steps.resize(GLUT_PC_NUMBER_OF_CAMERA_PARAMETERS, 1.0);
    increment_steps(GLUT_PC_WIDTH) = 2.0;
    increment_steps(GLUT_PC_HEIGHT) = 2.0;

    return camera_menu;
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif

}

/**
 * This is called everytime a submenu entry
 * is selected. It selects the appropriate
 * parameter and calls the user defined
 * callback.
 */
void Glut_perspective_camera::camera_submenu_glut(int i)
{
    (*camera_submenu_callback)(i);
    Glut_perspective_camera::selected_parameter = i;
}

/** This should be called in the GLUT main keyboard callback */
void Glut_perspective_camera::keyboard_callback(unsigned char key)
{
    if(!camera_enabled)
    {
        return;
    }

    if(!camera)
    {
        return;
    }

    int increment_sign = 1;

    if(key == increment_character)
    {
        increment_sign = 1;
    }
    else if(key == decrement_character)
    {
        increment_sign = -1;
    }
    else
    {
        return;
    }

    double parameter_step = increment_sign*increment_steps[selected_parameter];

    switch (Glut_perspective_camera::selected_parameter)
    {
        case GLUT_PC_CENTRE_X:
            camera->set_camera_centre_x(camera->get_camera_centre_x() + parameter_step);
            break;
        case GLUT_PC_CENTRE_Y:
            camera->set_camera_centre_y(camera->get_camera_centre_y() + parameter_step);
            break;
        case GLUT_PC_CENTRE_Z:
            camera->set_camera_centre_z(camera->get_camera_centre_z() + parameter_step);
            break;
        case GLUT_PC_PITCH:
            camera->rotate_around_x_axis(parameter_step);
            break;
        case GLUT_PC_YAW:
            camera->rotate_around_y_axis(parameter_step);
            break;
        case GLUT_PC_ROLL:
            camera->rotate_around_z_axis(parameter_step);
            break;
        case GLUT_PC_FOCAL_LENGTH:
            camera->set_focal_length(camera->get_focal_length() + parameter_step);
            break;
        case GLUT_PC_PRINCIPAL_POINT_X:
            camera->set_principal_point_x(camera->get_principal_point_x() + parameter_step);
            break;
        case GLUT_PC_PRINCIPAL_POINT_Y:
            camera->set_principal_point_y(camera->get_principal_point_y() + parameter_step);
            break;
        case GLUT_PC_SKEW:
            camera->set_skew(camera->get_skew() + parameter_step);
            break;
        case GLUT_PC_ASPECT_RATIO:
            camera->set_aspect_ratio(camera->get_aspect_ratio() + parameter_step);
            break;
        case GLUT_PC_WORLD_SCALE:
            camera->set_world_scale(camera->get_world_scale() + parameter_step);
            break;
        case GLUT_PC_NEAR:
            camera->get_rendering_interface().set_near_clipping_plane(camera->get_rendering_interface().get_near() +
                                                                      parameter_step);
            break;
        case GLUT_PC_FAR:
            camera->get_rendering_interface().set_far_clipping_plane(camera->get_rendering_interface().get_far() +
                                                                      parameter_step);
            break;
        case GLUT_PC_WIDTH:
            if(!centre_when_scaling_viewport)
            {
                Base_gl_interface::set_gl_viewport_size(Base_gl_interface::get_gl_viewport_width() + parameter_step,
                                                        Base_gl_interface::get_gl_viewport_height());
            }
            else
            {
                int int_increment = round(parameter_step);
                if( (int_increment % 2) != 0)
                {
                    if(int_increment > 0)
                    {
                        int_increment++;
                    }
                    else
                    {
                        int_increment--;
                    }
                }
                parameter_step = (double) int_increment;
                double x, y, w, h;
                Base_gl_interface::get_gl_viewport(&x, &y, &w, &h);
                std::cout << "Viewport:" << x << "  -  " << y << "     | " << w << "   -   "   << h << std::endl;
                Base_gl_interface::set_gl_viewport(x - (parameter_step)/2, y, w + parameter_step, h);
                Base_gl_interface::get_gl_viewport(&x, &y, &w, &h);
                std::cout << "Viewport:" << x << "  -  " << y << "     | " << w << "   -   "   << h << std::endl;
            }
            break;
        case GLUT_PC_HEIGHT:
            if(!centre_when_scaling_viewport)
            {
                Base_gl_interface::set_gl_viewport_size(Base_gl_interface::get_gl_viewport_width(),
                                                        Base_gl_interface::get_gl_viewport_height() + parameter_step);
            }
            else
            {
                int int_increment = round(parameter_step);
                if( (int_increment % 2) != 0)
                {
                    if(int_increment > 0)
                    {
                        int_increment++;
                    }
                    else
                    {
                        int_increment--;
                    }
                }
                parameter_step = (double) int_increment;
                double x, y, w, h;
                Base_gl_interface::get_gl_viewport(&x, &y, &w, &h);
                std::cout << "Viewport:" << x << "  -  " << y << "     | " << w << "   -   "   << h << std::endl;
                Base_gl_interface::set_gl_viewport(x, y - (parameter_step)/2, w, h + parameter_step);
                Base_gl_interface::get_gl_viewport(&x, &y, &w, &h);
                std::cout << "Viewport:" << x << "  -  " << y << "     | " << w << "   -   "   << h << std::endl;

            }
        break;
        default:
            break;
    }
}
