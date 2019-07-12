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
   |  Author:  Ernesto Brau
 * =========================================================================== */

/* $Id: pt_scene.h 19391 2015-06-05 10:26:30Z ernesto $ */

#ifndef PT_INIT_SCENE_H
#define PT_INIT_SCENE_H

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <people_tracking_cpp/pt_data.h>
#include <camera_cpp/perspective_camera.h>

namespace kjb {
namespace pt {

/**
 * @brief   Update the (mostly) 3D state of a scene.
 *          To be called after an association change.
 */
void update_scene_state
(
    const Scene& scene,
    const Facemark_data& fmdata,
    bool infer_head = true
);

/**
 * @brief   Update the (mostly) 3D state of a scene.
 *          To be called after an association change.
 */
void refine_scene_state
(
    const Scene& scene,
    const Scene_posterior& post
);

/** @brief  Refine trajectory estimate using single frame optimization. */
void refine_target_state
(
    const Target& target,
    const Perspective_camera& cam,
    const Box_data& box_data,
    const Scene_posterior& posterior
);

}} //namespace kjb::pt

#endif /*PT_INIT_SCENE_H */

