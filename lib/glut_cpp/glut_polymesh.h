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


/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief functions to handle a glut submenu for a polymesh in a glut
 * framework
 *
 */

#ifndef GLUT_POLYMESH_H_
#define GLUT_POLYMESH_H_

#include <gr_cpp/gr_opengl.h>

#ifdef KJB_HAVE_GLUT
#ifdef MAC_OSX
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#endif

enum { GLUT_PM_TRANSLATE_X, GLUT_PM_TRANSLATE_Y, GLUT_PM_TRANSLATE_Z, GLUT_PM_PITCH, GLUT_PM_YAW, GLUT_PM_ROLL,
       GLUT_PM_SCALE_X, GLUT_PM_SCALE_Y, GLUT_PM_SCALE_Z};

#define GLUT_PM_DECREMENT_CHARACTER 'j'
#define GLUT_PM_INCREMENT_CHARACTER 'k'
#define GLUT_PM_NUMBER_OF_POLYMESH_PARAMETERS 9

#include <m_cpp/m_vector.h>
#include <gr_cpp/gr_polymesh.h>
#include <g_cpp/g_quaternion.h>

namespace kjb{


    /** @class GLUT_polymesh This class provides an easy way to add a glut
     * submenu that handles a full polymesh to your application.
     * This class only creates the menu, it's up to the user to add it to the
     * main glut menu. Upon creation of the submenu, the user has to provide a callback
     * (polymesh_submenu_callback) that will be called every time one entry in the polymesh
     * submenu will be selected. Optionally, an empty callback can be passed, if the
     * user does not need to take any further action. It is also up to the user to call the keyboard_callback
     * provided in this class in the glut keyboard callback when appropriate.
     * This class provides a submenu entry for each parameter of the camera. Upon
     * selection, the keys 'j' and 'k' allow the user to increment or decrement
     * the value of each parameter by the increment_step for that parameter.
     * Each increment_step can be specified by the user. The user can also
     * decide to use different characters (ie characters other than 'j' and 'k').
     */
    class Glut_polymesh
    {
    public:
        static int create_glut_polymesh_submenu(void (*ipolymesh_submenu_callback)(int i), Polymesh * pa,
                bool enable_translation = true, bool enable_pitch = true, bool enable_yaw = true, bool enable_roll = true,
                bool enable_width = true, bool enable_height = true, bool enable_length = true,
                unsigned char iincrement_char = GLUT_PM_INCREMENT_CHARACTER,
                unsigned char idecrement_char = GLUT_PM_DECREMENT_CHARACTER);

        /** @brief This callback will be executed any time an entry of the polymesh
         * submenu is selected. This is automatically called. Within this
         * function the user defined callback will be called.
         */
        static void polymesh_submenu_glut(int i);

        /** @brief This is the keyboard_callback for this polymesh submenu.
         *  The user HAS to call this in the GLUT keyboard callback
         *  when appropriate
         */
        static void keyboard_callback(unsigned char key);


        /** @brief Enables the polymesh. When the camera is not enabled
         * all submenu selections and the submenu keyboard callback
         * have no effect
         */
        static void enable_polymesh() {polymesh_enabled = true;}

        /** @brief Disables the polymesh. When the camera is not enabled
         * all submenu selections and the submenu keyboard callback
         * have no effect
         */
        static void disable_polymesh() {polymesh_enabled = false;}

        /**@brief returns the increment step used for translating along the x-axis */
        static double retrieve_translation_x_increment() { return increment_steps(GLUT_PM_TRANSLATE_X);}
        /**@brief returns the increment step used for translating along the x-axis */
        static double retrieve_translation_y_increment() { return increment_steps(GLUT_PM_TRANSLATE_Y);}
        /**@brief returns the increment step used for translating along the z-axis */
        static double retrieve_translation_z_increment() { return increment_steps(GLUT_PM_TRANSLATE_Z);}
        /**@brief returns the increment step used for the pitch*/
        static double retrieve_pitch_increment() { return increment_steps(GLUT_PM_PITCH);}
        /**@brief returns the increment step used for the yaw*/
        static double retrieve_yaw_increment() { return increment_steps(GLUT_PM_YAW);}
        /**@brief returns the increment step used for the roll*/
        static double retrieve_roll_increment() { return increment_steps(GLUT_PM_ROLL);}
        /**@brief returns the increment step used for the width (along x axis)*/
        static double retrieve_width_increment() { return increment_steps(GLUT_PM_SCALE_X);}
        /**@brief returns the increment step used for the height (along y axis)*/
        static double retrieve_height_increment() { return increment_steps(GLUT_PM_SCALE_Y);}
        /**@brief returns the increment step used for the length (along z axis)*/
        static double retrieve_length_increment() { return increment_steps(GLUT_PM_SCALE_Z);}

        /**@brief sets the increment step used for translating along the x-axis */
        static void update_translation_x_increment(double iincrement) { increment_steps(GLUT_PM_TRANSLATE_X) = iincrement;}
        /**@brief sets the increment step used for translating along the y-axis */
        static void update_translation_y_increment(double iincrement) { increment_steps(GLUT_PM_TRANSLATE_Y) = iincrement;}
        /**@brief sets the increment step used for translating along the z-axis */
        static void update_translation_z_increment(double iincrement) { increment_steps(GLUT_PM_TRANSLATE_Z) = iincrement;}
        /**@brief sets the increment step used for the pitch*/
        static void update_pitch_increment(double iincrement) { increment_steps(GLUT_PM_PITCH) = iincrement;}
        /**@brief sets the increment step used for the yaw*/
        static void update_yaw_increment(double iincrement) { increment_steps(GLUT_PM_YAW) = iincrement;}
        /**@brief sets the increment step used for the roll*/
        static void update_roll_increment(double iincrement) { increment_steps(GLUT_PM_ROLL) = iincrement;}
        /**@brief sets the increment step used for the width (along x axis)*/
        static void update_width_increment(double iincrement) { increment_steps(GLUT_PM_SCALE_X) = iincrement;}
        /**@brief sets the increment step used for the height (along y axis)*/
        static void update_height_increment(double iincrement) { increment_steps(GLUT_PM_SCALE_Y) = iincrement;}
        /**@brief sets the increment step used for the length (along z axis)*/
        static void update_lenght_increment(double iincrement) { increment_steps(GLUT_PM_SCALE_Z) = iincrement;}

    private:
        static bool polymesh_enabled;
        static unsigned int selected_parameter;
        /** Pointer to the perspective camera associated to this submenu*/
        static Polymesh * polymesh;
        static unsigned char increment_character;
        static unsigned char decrement_character;
        static kjb::Vector increment_steps;
        static kjb::Vector _angles;
    };
}

static void (*polymesh_submenu_callback)(int i);

#endif /* GLUT_POLYMESH_H_ */
