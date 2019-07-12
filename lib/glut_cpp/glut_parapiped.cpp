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

#include <glut_cpp/glut_parapiped.h>
#include <gr_cpp/gr_opengl.h>

using namespace kjb;

bool Glut_parapiped::parapiped_enabled;
unsigned int Glut_parapiped::selected_parameter;
Parametric_parapiped * Glut_parapiped::parapiped;
unsigned char Glut_parapiped::increment_character;
unsigned char Glut_parapiped::decrement_character;
kjb::Vector Glut_parapiped::increment_steps;

/*
 * @param iparapiped_submenu_callback The user defined callback that will be executed
 *        any time a parapiped submenu entry is selected
 * @param pp The parapiped associated with this submenu
 * @param enable_translation specifies whether to add entries for translating the parapiped to the submenu
 * @param enable_pitch specifies whether to add entries for changing the pitch to the submenu
 * @param enable_yaw specifies whether to add entries for changing the yaw to the submenu
 * @param enable_roll specifies whether to add entries for changing the roll to the submenu
 * @param enable_width specifies whether to add entries for changing the pitch to the submenu
 * @param enable_height specifies whether to add entries for changing the width to the submenu
 * @param enable_length specifies whether to add entries for changing the height to the submenu
 * @param iincrement_char The character that will trigger an increment of the selected parameter. Default is 'k'
 * @param idecrement_char The character that will trigger an decrement of the selected parameter. Default is 'j'
 */
int Glut_parapiped::create_glut_parapiped_submenu(void (*iparapiped_submenu_callback)(int i), Parametric_parapiped *pa,
        bool enable_translation, bool enable_pitch, bool enable_yaw, bool enable_roll, bool enable_width,
        bool enable_height, bool enable_length, unsigned char iincrement_char,
        unsigned char idecrement_char)
{
#ifdef KJB_HAVE_GLUT
    int parapiped_menu = glutCreateMenu(Glut_parapiped::parapiped_submenu_glut);
    if(enable_translation)
    {
        glutAddMenuEntry("Translate X", GLUT_PA_CENTRE_X);
        glutAddMenuEntry("Translate Y", GLUT_PA_CENTRE_Y);
        glutAddMenuEntry("Translate Z", GLUT_PA_CENTRE_Z);
    }
    if(enable_pitch)
    {
        glutAddMenuEntry("Pitch", GLUT_PA_PITCH);
    }
    if(enable_yaw)
    {
        glutAddMenuEntry("Yaw", GLUT_PA_YAW);
    }
    if(enable_roll)
    {
        glutAddMenuEntry("Roll", GLUT_PA_ROLL);
    }
    if(enable_width)
    {
        glutAddMenuEntry("Width", GLUT_PA_WIDTH);
    }
    if(enable_height)
    {
        glutAddMenuEntry("Height", GLUT_PA_HEIGHT);
    }
    if(enable_length)
    {
        glutAddMenuEntry("Length", GLUT_PA_LENGTH);
    }

    parapiped_submenu_callback = iparapiped_submenu_callback;
    parapiped = pa;

    parapiped_enabled = true;

    increment_character = iincrement_char;
    decrement_character = idecrement_char;

    increment_steps.resize(GLUT_PA_NUMBER_OF_PARAPIPED_PARAMETERS, 1.0);

    return parapiped_menu;

#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

void Glut_parapiped::parapiped_submenu_glut(int i)
{
    parapiped_submenu_callback(i);
    selected_parameter = i;
}

void Glut_parapiped::keyboard_callback(unsigned char key)
{
    if(!parapiped_enabled)
    {
        return;
    }

    if(!parapiped)
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

    switch (selected_parameter)
    {
        case GLUT_PA_CENTRE_X:
            parapiped->set_centre_x(parapiped->get_centre_x() + parameter_step);
            break;
        case GLUT_PA_CENTRE_Y:
            parapiped->set_centre_y(parapiped->get_centre_y() + parameter_step);
            break;
        case GLUT_PA_CENTRE_Z:
            parapiped->set_centre_z(parapiped->get_centre_z() + parameter_step);
            break;
        case GLUT_PA_PITCH:
            parapiped->rotate_around_x_axis(parameter_step);
            break;
        case GLUT_PA_YAW:
            parapiped->rotate_around_y_axis(parameter_step);
            break;
        case GLUT_PA_ROLL:
            parapiped->rotate_around_z_axis(parameter_step);
            break;
        case GLUT_PA_WIDTH:
            parapiped->set_width(parapiped->get_width() + parameter_step);
            break;
        case GLUT_PA_HEIGHT:
            parapiped->set_height(parapiped->get_height() + parameter_step);
            //parapiped->stretch_along_axis(1, parameter_step, true);
            break;
        case GLUT_PA_LENGTH:
            parapiped->set_length(parapiped->get_length() + parameter_step);
            //parapiped->stretch_along_axis(1, parameter_step, false);
            break;
        default:
            break;
    }
}
