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

/* $Id: visibility.cpp 18671 2015-03-19 00:19:12Z ernesto $ */

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_association.h>
#include <people_tracking_cpp/pt_complete_trajectory.h>
#include <people_tracking_cpp/pt_visibility.h>
#include <people_tracking_cpp/pt_data.h>
#include <l_cpp/l_test.h>
#include <algorithm>

#include "utils.h"

using namespace std;
using namespace kjb;
using namespace kjb::pt; 

/**
 * @brief   Helper function that compares the equality of two visibilities
 */
bool operator!=(const Visibility& vis1, const Visibility& vis2);

/**
 * @brief   Helper function that compares the visibility of two scenes
 */
bool equal_visibility(const Scene& scene1, const Scene& scene2);

/** @brief  Main =) */
int main(int argc, char** argv)
{
    // CREATE SCENE
    size_t num_frames = 50;
    double img_width = 500;
    double img_height = 500;
    const size_t num_max_trajs = 1;

    Box_data data(img_width, img_height, 0.99);
    vector<Integral_flow> flows_x;
    vector<Integral_flow> flows_y;
    Facemark_data fm_data(num_frames);

    Scene scene1(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
    create_or_read_scene(
        argc, argv, num_frames, img_width, img_height,
        data, fm_data, flows_x, flows_y, scene1, num_max_trajs);
    Scene scene2 = scene1;

    update_visibilities(scene1);

    // TEST STUFF
    for(size_t f = 1; f <= num_frames; f++)
    {
        update_visibilities(scene2, f);
    }

    TEST_TRUE(equal_visibility(scene1, scene2));
   
    for(size_t f = 0; f < num_frames; f++)
    {
        // Move the trajectory
        Ascn::iterator move_target1 = scene1.association.begin();

        if(f + 1 < move_target1->get_start_time()) continue;
        if(f + 1 > move_target1->get_end_time()) break;
        move_target1->trajectory()[f]->value.position[0] += 0.05; 
        move_target1->trajectory()[f]->value.position[2] += 0.10; 

        Ascn::iterator move_target2 = scene2.association.begin();
        if(f + 1 < move_target2->get_start_time()) continue;
        if(f + 1 > move_target2->get_end_time()) break;
        move_target2->trajectory()[f]->value.position[0] += 0.05; 
        move_target2->trajectory()[f]->value.position[2] += 0.10; 
        
        // Update the visibilities
        update_visibilities(scene1);
        update_visibilities(scene2, f + 1);

        TEST_TRUE(equal_visibility(scene1, scene2));
    }
    
    RETURN_VICTORIOUSLY();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool operator!=(const Visibility& vis1, const Visibility& vis2)
{
    assert(vis1.visible_cells.size() == vis2.visible_cells.size());
    if(vis1.visible_cells != vis2.visible_cells
        || (fabs(vis1.visible - vis2.visible) > FLT_EPSILON)
        || (fabs(vis1.cell_width - vis2.cell_width) > FLT_EPSILON)
        || (fabs(vis1.cell_height - vis2.cell_height) > FLT_EPSILON))
    {
        return true;
    }

    return false; 
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool equal_visibility(const Scene& scene1, const Scene& scene2)
{
    assert(scene1.association.size() == scene2.association.size());
    
    Ascn::const_iterator cur_target_p1;
    Ascn::const_iterator cur_target_p2; 

    for(cur_target_p1 = scene1.association.begin(),
        cur_target_p2 = scene2.association.begin();
        cur_target_p1 != scene1.association.end(); 
        cur_target_p1++, cur_target_p2++)

    {
        const Body_2d_trajectory& btraj1 = cur_target_p1->body_trajectory();
        const Body_2d_trajectory& btraj2 = cur_target_p2->body_trajectory();
        assert(btraj1.size() == btraj2.size());
        size_t sf = btraj1.start_time();
        size_t ef = btraj1.end_time();
        for(size_t j = sf - 1; j < ef; j++)
        {
            const Visibility& vis1 = btraj1[j]->value.visibility;
            const Visibility& vis2 = btraj2[j]->value.visibility;
            if(vis1 != vis2)
            {
                return false;
            }
        }
    }

    return true; 
}

