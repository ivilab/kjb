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
 * @brief functions to handle a glut submenu for a perspective camera in a glut
 * framework
 *
 */

#ifndef GLUT_PERSPECTIVE_CAMERA_H_
#define GLUT_PERSPECTIVE_CAMERA_H_

#include <gr_cpp/gr_opengl.h>

#ifdef KJB_HAVE_GLUT
#ifdef MAC_OSX
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#endif

enum { GLUT_PC_CENTRE_X, GLUT_PC_CENTRE_Y, GLUT_PC_CENTRE_Z, GLUT_PC_PITCH, GLUT_PC_YAW, GLUT_PC_ROLL,
       GLUT_PC_FOCAL_LENGTH, GLUT_PC_PRINCIPAL_POINT_X, GLUT_PC_PRINCIPAL_POINT_Y, GLUT_PC_SKEW, GLUT_PC_ASPECT_RATIO,
       GLUT_PC_WORLD_SCALE, GLUT_PC_NEAR, GLUT_PC_FAR, GLUT_PC_WIDTH, GLUT_PC_HEIGHT};

#define GLUT_PC_DECREMENT_CHARACTER 'j'
#define GLUT_PC_INCREMENT_CHARACTER 'k'
#define GLUT_PC_NUMBER_OF_CAMERA_PARAMETERS 16

#include <m_cpp/m_vector.h>
#include <camera_cpp/perspective_camera.h>

namespace kjb{

    /** @class Glut_perspective_camera This class provides an easy way to add a glut
     * submenu that handles a full perspective camera to your application.
     * This class only creates the menu, it's up to the user to add it to the
     * main glut menu. Upon creation of the submenu, the user has to provide a callback
     * (camera_submenu_callback) that will be called every time one entry in the camera
     * submenu will be selected. Optionally, an empty callback can be passed, if the
     * user does not need to take any further action. It is also up to the user to call the keyboard_callback
     * provided in this class the glut keyboard callback when appropriate.
     * This class provides a submenu entry for each parameter of the camera. Upon
     * selection, the keys 'j' and 'k' allow the user to increment or decrement
     * the value of each parameter by the increment_step for that parameter.
     * Each increment_step can be specified by the user. The user can also
     * decide to use different characters (ie characters other than 'j' and 'k').
     */
    class Glut_perspective_camera
    {
    public:
        static int create_glut_perspective_camera_submenu(void (*icamera_submenu_callback)(int i), kjb::Perspective_camera * pc,
                bool enable_translation = true, bool enable_pitch = true, bool enable_yaw = true, bool enable_roll = true,
                bool enable_focal_length = true, bool enable_principal_point = true, bool enable_skew = true,
                bool enable_aspect_ratio = true, bool enable_world_scale = true, bool enable_clipping = true,
                bool enable_viewport = true, bool icentre_when_scaling_viewport = true,
                unsigned char iincrement_char = GLUT_PC_INCREMENT_CHARACTER,
                unsigned char idecrement_char = GLUT_PC_DECREMENT_CHARACTER);

        /** @brief This callback will be executed any time an entry of the camera
         * submenu is selected. This is automatically called. Within this
         * function the user defined callback will be called.
         */
        static void camera_submenu_glut(int i);

        /** @brief This is the keyboard_callback for this camera submenu.
         *  The user HAS to call this in the GLUT keyboard callback
         *  when appropriate
         */
        static void keyboard_callback(unsigned char key);

        /** @brief Enables the camera. When the camera is not enabled
         * all submenu selections and the submenu keyboard callback
         * have no effect
         */
        static void enable_camera() {camera_enabled = true;}

        /** @brief Disable the camera. When the camera is not enabled
         * all submenu selections and the submenu keyboard callback
         * have no effect
         */
        static void disable_camera() {camera_enabled = false;}

        /**@brief returns the increment step used for the x-coordinate of the camera centre */
        static double retrieve_translation_x_increment() { return increment_steps(GLUT_PC_CENTRE_X);}
        /**@brief returns the increment step used for the y-coordinate of the camera centre */
        static double retrieve_translation_y_increment() { return increment_steps(GLUT_PC_CENTRE_Y);}
        /**@brief returns the increment step used for the z-coordinate of the camera centre */
        static double retrieve_translation_z_increment() { return increment_steps(GLUT_PC_CENTRE_Z);}
        /**@brief returns the increment step used for the pitch*/
        static double retrieve_pitch_increment() { return increment_steps(GLUT_PC_PITCH);}
        /**@brief returns the increment step used for the yaw*/
        static double retrieve_yaw_increment() { return increment_steps(GLUT_PC_YAW);}
        /**@brief returns the increment step used for the roll*/
        static double retrieve_roll_increment() { return increment_steps(GLUT_PC_ROLL);}
        /**@brief returns the increment step used for the focal length*/
        static double retrieve_focal_length_increment() { return increment_steps(GLUT_PC_FOCAL_LENGTH);}
        /**@brief returns the increment step used for the x coordinate of the principal point*/
        static double retrieve_principal_point_x_increment() { return increment_steps(GLUT_PC_PRINCIPAL_POINT_X);}
        /**@brief returns the increment step used for the y coordinate of the principal point*/
        static double retrieve_principal_point_y_increment() { return increment_steps(GLUT_PC_PRINCIPAL_POINT_Y);}
        /**@brief returns the increment step used for the skew*/
        static double retrieve_skew_increment() { return increment_steps(GLUT_PC_SKEW);}
        /**@brief returns the increment step used for the aspect ratio*/
        static double retrieve_aspect_ratio_increment() { return increment_steps(GLUT_PC_ASPECT_RATIO);}
        /**@brief returns the increment step used for the world scale*/
        static double retrieve_world_scale_increment() { return increment_steps(GLUT_PC_WORLD_SCALE);}
        /**@brief returns the increment step used for the near clipping plane */
        static double retrieve_near_clipping_increment() {return increment_steps(GLUT_PC_NEAR);}
        /**@brief returns the increment step used for the far clipping plane */
        static double retrieve_far_clipping_increment() {return increment_steps(GLUT_PC_FAR);}
        /**@brief returns the increment step used for the viewport width */
        static double retrieve_width_increment() {return increment_steps(GLUT_PC_WIDTH);}
        /**@brief returns the increment step used for the viewport height */
        static double retrieve_height_increment() {return increment_steps(GLUT_PC_HEIGHT);}

        /**@brief sets the increment step used for the x-coordinate of the camera centre */
        static void update_translation_x_increment(double iincrement) { increment_steps(GLUT_PC_CENTRE_X) = iincrement;}
        /**@brief sets the increment step used for the y-coordinate of the camera centre */
        static void update_translation_y_increment(double iincrement) { increment_steps(GLUT_PC_CENTRE_Y) = iincrement;}
        /**@brief sets the increment step used for the z-coordinate of the camera centre */
        static void update_translation_z_increment(double iincrement) { increment_steps(GLUT_PC_CENTRE_Z) = iincrement;}
        /**@brief sets the increment step used for the pitch */
        static void update_pitch_increment(double iincrement) { increment_steps(GLUT_PC_PITCH) = iincrement;}
        /**@brief sets the increment step used for the yaw */
        static void update_yaw_increment(double iincrement) { increment_steps(GLUT_PC_YAW) = iincrement;}
        /**@brief sets the increment step used for the roll */
        static void update_roll_increment(double iincrement) { increment_steps(GLUT_PC_ROLL) = iincrement;}
        /**@brief sets the increment step used for the focal length */
        static void update_focal_length_increment(double iincrement) { increment_steps(GLUT_PC_FOCAL_LENGTH) = iincrement;}
        /**@brief sets the increment step used for the x coordinate of the principal point*/
        static void update_principal_point_x_increment(double iincrement) { increment_steps(GLUT_PC_PRINCIPAL_POINT_X) = iincrement;}
        /**@brief sets the increment step used for the y coordinate of the principal point*/
        static void update_principal_point_y_increment(double iincrement) { increment_steps(GLUT_PC_PRINCIPAL_POINT_Y) = iincrement;}
        /**@brief sets the increment step used for the skew */
        static void update_skew_increment(double iincrement) { increment_steps(GLUT_PC_SKEW) = iincrement;}
        /**@brief sets the increment step used for the aspect ratio */
        static void update_aspect_ratio_increment(double iincrement) { increment_steps(GLUT_PC_ASPECT_RATIO) = iincrement;}
        /**@brief sets the increment step used for the world scale*/
        static void update_world_scale_increment(double iincrement) { increment_steps(GLUT_PC_WORLD_SCALE) = iincrement;}
        /**@brief sets the increment step used for the near clipping plane */
        static void update_near_clipping_increment(double iincrement) { increment_steps(GLUT_PC_NEAR) = iincrement;}
        /**@brief sets the increment step used for the far clipping plane */
        static void update_far_clipping_increment(double iincrement) { increment_steps(GLUT_PC_FAR) = iincrement;}
        /**@brief sets the increment step used for the viewport width */
        static void update_width_increment(double iincrement){ increment_steps(GLUT_PC_WIDTH) = iincrement;}
        /**@brief sets the increment step used for the viewport height */
        static void update_height_increment(double iincrement) { increment_steps(GLUT_PC_HEIGHT) = iincrement;}

    private:
        static bool camera_enabled;
        static unsigned int selected_parameter;
        /** Pointer to the perspective camera associated to this submenu*/
        static kjb::Perspective_camera * camera;
        static unsigned char increment_character;
        static unsigned char decrement_character;
        static kjb::Vector increment_steps;

        /** This parameter determines whether the vieport should be centred while scaling,
         * or kept in the same position. Default is true.
         */
        static bool centre_when_scaling_viewport;
    };
}
static void (*camera_submenu_callback)(int i);

#endif
