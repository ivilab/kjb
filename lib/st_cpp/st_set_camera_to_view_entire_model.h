/* $Id: st_set_camera_to_view_entire_model.h 13676 2013-01-29 23:31:33Z elh $ */
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
|     Emily Hartley
|
* =========================================================================== */

#ifndef ST_SET_CAMERA_TO_VIEW_ENTIRE_MODEL_H_
#define ST_SET_CAMERA_TO_VIEW_ENTIRE_MODEL_H_

#include <st_cpp/st_perspective_camera.h>
#include <gr_cpp/gr_polymesh.h>

namespace kjb
{

Vector set_camera_to_view_entire_model
(
    Perspective_camera& c, 
    Polymesh&           p, 
    int                 imageWidth, 
    int                 imageHeight
);

} // namespace kjb

#endif /* ST_SET_CAMERA_TO_VIEW_ENTIRE_MODEL_H_ */

