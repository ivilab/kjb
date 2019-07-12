/* $Id: st_set_camera_to_view_entire_model.cpp 18278 2014-11-25 01:42:10Z ksimek $ */
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

#include <st_cpp/st_set_camera_to_view_entire_model.h>

using namespace kjb;
//using namespace kjb::opengl;

Vector kjb::set_camera_to_view_entire_model
(
    Perspective_camera& c, 
    Polymesh&           p, 
    int                 imageWidth, 
    int                 imageHeight
)
{
    int y_offset = 50; 
    int x_offset = 100;
    double world_x;
    double world_y;
    Vector cameraCenter = p.get_center();
    Vector polymeshCenter = p.get_center();
    Vector largest_bounds = p.get_largest_bounds();
    Vector smallest_bounds = p.get_smallest_bounds();
    double focal = c.get_focal_length();

    if(fabs(smallest_bounds(0) - polymeshCenter(0)) > 
       fabs(largest_bounds(0) - polymeshCenter(0)))
    {
        world_x = fabs(smallest_bounds(0)-polymeshCenter(0));
    }
    else
    {
        world_x = fabs(largest_bounds(0)-polymeshCenter(0));
    }

    if(fabs(smallest_bounds(1) - polymeshCenter(1)) > 
       fabs(largest_bounds(1) - polymeshCenter(1)))
    {
        world_y = fabs(smallest_bounds(1)-polymeshCenter(1));
    }
    else
    {
        world_y = fabs(largest_bounds(1)-polymeshCenter(1));
    }

    double z1 = (world_x * focal) / ((imageWidth/2.0) - x_offset);
    double z2 = (world_y * focal) / ((imageHeight/2.0) - y_offset);
    double z;

    if(z1 > z2)
    {
        z = polymeshCenter(2) + z1;
    }
    else
    {
        z = polymeshCenter(2) + z2;
    }

    if(z > largest_bounds(2))
    {
        cameraCenter(2) = z;
    }
    else
    {
        cameraCenter(2) = 3 * largest_bounds(2);
    }

    return cameraCenter;
}
