/* $Id: psi_deva_skeleton.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
   |  Author:  Jinyan Guan
 * =========================================================================== */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "psi_cpp/psi_deva_skeleton.h"
//#include "psi_cpp/psi_util.h"
#include "people_tracking_cpp/pt_util.h"
#include "l_cpp/l_exception.h"

#include <string>

#include <boost/assign/std/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
namespace kjb
{
namespace psi
{

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Deva_skeleton_boxes> parse_deva_skeletons(std::istream& is)
{
    using std::vector;
    const size_t BOXES_PER_ACTOR = 26;
    const size_t TOKENS_PER_BOX = 4;
    const size_t TOKENS_PER_ACTOR = BOXES_PER_ACTOR * TOKENS_PER_BOX;

    // reorganize tokens into Deva_skeleton_boxes 
    std::vector<Deva_skeleton_boxes> skeletons;

    std::string line;

//    size_t num_actors = 0;

    while(std::getline(is, line))
    {
        Deva_skeleton_boxes cur_skeleton;

        // split line on whitespace
        std::vector<std::string> tokens;
        boost::trim(line);
        boost::split(tokens, line, boost::is_any_of("\t \n\r"), boost::token_compress_on);
        ASSERT(tokens.size() == TOKENS_PER_ACTOR ||
               tokens.size() == TOKENS_PER_ACTOR + 1);

        size_t i = 0;
        for(size_t box_i = 0; box_i < BOXES_PER_ACTOR; box_i++)
        {
            Vector p1(2), p2(2);

            p1[0] = boost::lexical_cast<double>(tokens[i++]);
            p1[1] = boost::lexical_cast<double>(tokens[i++]);
            p2[0] = boost::lexical_cast<double>(tokens[i++]);
            p2[1] = boost::lexical_cast<double>(tokens[i++]);

            cur_skeleton[box_i] = Bbox(p1, p2);
        }

        if(tokens.size() == TOKENS_PER_ACTOR + 1)
        {
            // score is also available
            
            ASSERT(i == TOKENS_PER_ACTOR);
            cur_skeleton.set_score(boost::lexical_cast<double>(tokens[i]));
        }  
        else // if the score is not available 
        {
            cur_skeleton.set_score(DBL_MAX);
        }

        skeletons.push_back(cur_skeleton);
    }


    return skeletons;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<std::vector<Deva_skeleton_boxes> > parse_deva_skeleton_tracks
(
    std::istream& is, 
    size_t num_frames
)
{
    // all_skeletons indexed by <frame> <track> 
    std::vector<std::vector<Deva_skeleton_boxes> > all_skeletons(num_frames);
    
    const size_t tokens_per_box = 4;
    const size_t boxes_per_skeleton = 26;

    // Each line has 26 * 4 boxes and <score> <frame_number> <track_id> 
    const size_t tokens_per_line = tokens_per_box * boxes_per_skeleton + 3; 

    size_t cur_frame = 0;
    std::string line;

    while(std::getline(is, line))
    {
        std::vector<std::string> tokens;

        boost::trim(line);
        boost::split(tokens, line, boost::is_any_of(" "), boost::token_compress_on);
        ASSERT(tokens.size() == tokens_per_line);
        size_t i = 0;

        Deva_skeleton_boxes sbox;
        for(size_t part = 0; part < boxes_per_skeleton; part++)
        {
            Vector p1(2), p2(2);
            p1[0] = boost::lexical_cast<double>(tokens[i++]);
            p1[1] = boost::lexical_cast<double>(tokens[i++]);
            p2[0] = boost::lexical_cast<double>(tokens[i++]);
            p2[1] = boost::lexical_cast<double>(tokens[i++]);
            sbox.set_part(part, Bbox(p1, p2));
        }

        double score = boost::lexical_cast<double>(tokens[i++]);
        size_t frame_number = boost::lexical_cast<size_t>(tokens[i++]);
        //size_t track_id = boost::lexical_cast<size_t>(tokens[i++]);

        sbox.set_score(score);
        
        if(frame_number > cur_frame) // create a new frame 
        {
            std::vector<Deva_skeleton_boxes> frame_box;
            frame_box.push_back(sbox);
            all_skeletons.at(frame_number-1) = frame_box;
            cur_frame = frame_number;
        }
        else
        {
            all_skeletons.at(frame_number-1).push_back(sbox);
        }
    }
    return all_skeletons;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void standardize(Deva_skeleton_boxes& boxes, double cam_width, double cam_height)
{
    for(size_t i = 0; i < boxes.size(); i++)
    {
        pt::standardize(boxes[i], cam_width, cam_height);
    }
}
} //namespace psi
} //namespace kjb
