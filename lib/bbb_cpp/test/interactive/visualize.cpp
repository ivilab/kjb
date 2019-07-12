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

/* $Id$ */

#include <bbb_cpp/bbb_visualizer.h>
#include <bbb_cpp/bbb_likelihood.h>
#include <bbb_cpp/bbb_description_prior.h>
#include <bbb_cpp/bbb_activity_sequence_prior.h>
#include <bbb_cpp/bbb_parameter_prior.h>
#include <bbb_cpp/bbb_trajectory_prior.h>
#include <bbb_cpp/bbb_association_prior.h>
#include <bbb_cpp/bbb_activity_library.h>
#include <bbb_cpp/bbb_intentional_activity.h>
#include <bbb_cpp/bbb_description.h>
#include <bbb_cpp/bbb_data.h>
#include <l_cpp/l_test.h>
#include <l_cpp/l_functors.h>
#include <m_cpp/m_vector.h>
#include <gr_cpp/gr_glut.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_headers.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

using namespace kjb;
using namespace std;

/** @brief  Handle key input. */
void handle_key(bbb::Visualizer&, unsigned char, int, int);

int main(int argc, char** argv)
{
    // trajectories
    vector<size_t> trs(20);
    generate(trs.begin(), trs.end(), Increment<size_t>(0));
    bbb::Traj_set trajs(trs.begin(), trs.end());

    // times
    const size_t start = 0;
    const size_t end = 99;

    // prior
    bbb::Activity_library lib("/home/ernesto/.local/data/bbb/activity_library");
    bbb::Parameter_prior param_prior(lib);
    bbb::Activity_sequence_prior as_prior(param_prior, lib);
    bbb::Association_prior ass_prior(lib);
    bbb::Trajectory_prior traj_prior(2, lib);
    bbb::Intentional_activity meet("FFA", start, end, Vector(), trajs);
    bbb::Description_prior prior(meet, ass_prior, as_prior, traj_prior, lib);

    // likelihood
    bbb::Likelihood likelihood(lib);

    // sample a description and data
    bbb::Description desc = bbb::sample(prior);
    bbb::Data data = bbb::sample(likelihood, desc);

    // visualize
    bbb::Visualizer vis(desc, data, lib);
    vis.set_key_callback(boost::bind(handle_key, boost::ref(vis), _1, _2, _3));

    // start GLUT loop
    glutMainLoop();

    RETURN_VICTORIOUSLY();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void handle_key(bbb::Visualizer& vis, unsigned char key, int, int)
{
    const size_t frame_jump = 10;

    switch(key)
    {
        case 'n':
        {
            vis.advance_frame();
            break;
        }

        case 'p':
        {
            vis.rewind_frame();
            break;
        }

        case 'N':
        {
            vis.advance_frame(frame_jump);
            break;
        }

        case 'P':
        {
            vis.rewind_frame(frame_jump);
            break;
        }

        case 'q':
        {
            exit(0);
        }

        default:
        {
            break;
        }
    }

    glutPostRedisplay();
}

