/* $Id: test_trajectory.cpp 14087 2013-03-12 20:44:24Z jguan1 $ */
/* {{{=========================================================================== *
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#include <psi_cpp/psi_trajectory.h>
#include <psi_cpp/psi_entity_type.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include <boost/assign/list_of.hpp>

using namespace std;
using namespace kjb;
using namespace kjb::psi;

using boost::assign::list_of;

int main(int argc, char* argv[])
{
    // read
    std::string fname = "input/trajectory_%04d.txt";


    // number of non-empty things
    vector<Entity_id> entities = list_of<Entity_id>
        (Entity_id(PERSON_ENTITY, 0))
        (Entity_id(PERSON_ENTITY, 1))
        (Entity_id(DOG_ENTITY, 0));

    vector<size_t> entity_size = list_of<size_t>
        (1)(2)(3);

    vector<Entity_id> non_entities = list_of<Entity_id>
        (Entity_id(PERSON_ENTITY, 3))
        (Entity_id(CAR_ENTITY, 0))
        (Entity_id(BICYCLE_ENTITY, 0));

    // parse
    Trajectory_map trajectories;
    static const size_t NUM_FRAMES = 100;
    trajectories.parse(fname, NUM_FRAMES);
    if(trajectories.size() != entities.size())
    {
        cerr << "number of trajectories incorrect after parsing" << endl;
        return 1;
    }

    if((*trajectories.begin()).second.size() != NUM_FRAMES)
    {
        cerr << "num_frames incorrect after parsing" << endl;
        return 1;
    }

    for(size_t i = 0; i < non_entities.size(); i++)
    {
        if(trajectories.count(non_entities[i]) != 0)
        {
            cerr << "non-entity seems to exist!" << endl;
            return 1;
        }
    }

    for(size_t i = 0; i < entities.size(); i++)
    {
        if(trajectories.count(entities[i]) == 0)
        {
            cerr << "entity doesn't exist" << endl;
            return 1;
        }
        
        const Trajectory& traj = trajectories[entities[i]];

        if(traj.size() != NUM_FRAMES)
        {
            cerr << "entity trajectory isn't correct." << endl;
            return 1;
        }
        
        size_t count = 0;
        for(size_t f = 0; f < traj.size(); f++)
        {
            if(!(!traj[f]))
            {
                count++;
            }
        }

        if(count != entity_size[i])
        {
            cerr << "number of non-empty frames isn't right." << endl;
            return 1;
        }
    }

    Model model = trajectories_to_model(trajectories, 30);

    cout << "Success" << endl;

    return 0;
}
