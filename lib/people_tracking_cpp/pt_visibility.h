/* =========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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
   |  Author:  Ernesto Brau, Jinyan Guan
 * =========================================================================== */

/* $Id: pt_visibility.h 18971 2015-04-29 23:31:48Z jguan1 $ */

#ifndef PT_VISIBILITY_H
#define PT_VISIBILITY_H

//#include <people_tracking_cpp/pt_scene.h>
#include <m_cpp/m_vector.h>
#include <vector>

#include <boost/optional.hpp>

namespace kjb {
namespace pt {

/**
 * @struct  Visibility
 * @brief   Represents the information regarding visibility of an actor
 *          at a given frame and given all other actors.
 */
struct Visibility
{
    std::vector<Vector> visible_cells;
    double visible;
    double cell_width;
    double cell_height;

    Visibility() : visible(1.0), cell_width(-1.0), cell_height(-1.0) {}
};

// forward declaration to avoid cycles
class Scene;

/** @brief  Update all the visibilities in a scene. */
void update_visibilities(const Scene& scene, bool infer_head = true);

/** @brief  Update visibility for a single frame (and leave others intact). */
void update_visibilities
(
    const Scene& scene,
    size_t frame,
    bool infer_head = true
);

}} //namespace kjb::pt

#endif /*PT_VISIBILITY_H */

