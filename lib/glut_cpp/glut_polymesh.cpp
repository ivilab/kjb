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

#include <glut_cpp/glut_polymesh.h>
#include <gr_cpp/gr_opengl.h>
#include <iostream>

using namespace kjb;

bool Glut_polymesh::polymesh_enabled;
unsigned int Glut_polymesh::selected_parameter;
Polymesh * Glut_polymesh::polymesh;
unsigned char Glut_polymesh::increment_character;
unsigned char Glut_polymesh::decrement_character;
kjb::Vector Glut_polymesh::increment_steps;
kjb::Vector Glut_polymesh::_angles(3, 0.0);

/*
 * @param ipolymesh_submenu_callback The user defined callback that will be executed
 *        any time a polymesh submenu entry is selected
 * @param pp The polymesh associated with this submenu
 * @param enable_translation specifies whether to add entries for translating the polymesh to the submenu
 * @param enable_pitch specifies whether to add entries for changing the pitch to the submenu
 * @param enable_yaw specifies whether to add entries for changing the yaw to the submenu
 * @param enable_roll specifies whether to add entries for changing the roll to the submenu
 * @param enable_width specifies whether to add entries for changing the pitch to the submenu
 * @param enable_height specifies whether to add entries for changing the width to the submenu
 * @param enable_length specifies whether to add entries for changing the height to the submenu
 * @param iincrement_char The character that will trigger an increment of the selected parameter. Default is 'k'
 * @param idecrement_char The character that will trigger an decrement of the selected parameter. Default is 'j'
 */
int Glut_polymesh::create_glut_polymesh_submenu(void (*ipolymesh_submenu_callback)(int i), Polymesh *pa,
        bool enable_translation, bool enable_pitch, bool enable_yaw, bool enable_roll, bool enable_width,
        bool enable_height, bool enable_length, unsigned char iincrement_char,
        unsigned char idecrement_char)
{
#ifdef KJB_HAVE_GLUT
    int polymesh_menu = glutCreateMenu(Glut_polymesh::polymesh_submenu_glut);
    if(enable_translation)
    {
        glutAddMenuEntry("Translate X", GLUT_PM_TRANSLATE_X);
        glutAddMenuEntry("Translate Y", GLUT_PM_TRANSLATE_Y);
        glutAddMenuEntry("Translate Z", GLUT_PM_TRANSLATE_Z);
    }
    if(enable_pitch)
    {
        glutAddMenuEntry("Pitch", GLUT_PM_PITCH);
    }
    if(enable_yaw)
    {
        glutAddMenuEntry("Yaw", GLUT_PM_YAW);
    }
    if(enable_roll)
    {
        glutAddMenuEntry("Roll", GLUT_PM_ROLL);
    }
    if(enable_width)
    {
        glutAddMenuEntry("Width", GLUT_PM_SCALE_X);
    }
    if(enable_height)
    {
        glutAddMenuEntry("Height", GLUT_PM_SCALE_Y);
    }
    if(enable_length)
    {
        glutAddMenuEntry("Length", GLUT_PM_SCALE_Z);
    }

    polymesh_submenu_callback = ipolymesh_submenu_callback;
    polymesh = pa;

    polymesh_enabled = true;

    increment_character = iincrement_char;
    decrement_character = idecrement_char;

    increment_steps.resize(GLUT_PM_NUMBER_OF_POLYMESH_PARAMETERS, 0.1);

    return polymesh_menu;

#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

void Glut_polymesh::polymesh_submenu_glut(int i)
{
    polymesh_submenu_callback(i);
    selected_parameter = i;
}

void Glut_polymesh::keyboard_callback(unsigned char key)
{
    if(!polymesh_enabled)
    {
        return;
    }

    if(!polymesh)
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
    std::cout << "Parameter step: " << parameter_step << std::endl;

    switch (selected_parameter)
    {
        case GLUT_PM_TRANSLATE_X:
            polymesh->translate(parameter_step, 0.0, 0.0);
            break;
        case GLUT_PM_TRANSLATE_Y:
            polymesh->translate(0.0, parameter_step, 0.0);
            break;
        case GLUT_PM_TRANSLATE_Z:
            polymesh->translate(0.0, 0.0, parameter_step);
            break;
        case GLUT_PM_PITCH:
            polymesh->rotate(parameter_step, 1.0, 0.0, 0.0);
            break;
        case GLUT_PM_YAW:
            polymesh->rotate(parameter_step, 0.0, 1.0, 0.0);
            break;
        case GLUT_PM_ROLL:
            polymesh->rotate(parameter_step, 0.0, 0.0, 1.0);
            break;
        case GLUT_PM_SCALE_X:
            polymesh->scale(1.0 + parameter_step, 1.0, 1.0);
            break;
        case GLUT_PM_SCALE_Y:
            polymesh->scale(1.0, 1.0 + parameter_step, 1.0);
            break;
        case GLUT_PM_SCALE_Z:
            polymesh->scale(1.0, 1.0, 1.0 + parameter_step);
            break;
        default:
            break;
    }
}
