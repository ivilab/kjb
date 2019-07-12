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
   |  Author:  Kyle Simek, Jinyan Guan
 * =========================================================================== */

/* $Id: d_deva_detection.cpp 18278 2014-11-25 01:42:10Z ksimek $ */

#include <m_cpp/m_vector.h>
#include <l_cpp/l_exception.h>
#include <camera_cpp/camera_backproject.h>
#include <gr_cpp/gr_2D_bounding_box.h>
#include <detector_cpp/d_deva_detection.h>

#include <boost/assign/std/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <string>
#include <vector>
#include <iostream>

using namespace kjb;

double Deva_detection::probability_of_noise() const
{
    double a;
    double b;

    if(type_ == "person")
    {
        //a = -0.805;
        //b = 3.394;
        a = 0.08483;
        b = 4.02716;
    }
    else if(type_ == "person_inria")
    {
        a = 0.43169;
        b = 4.16437;
    }
    else if(type_ == "person_inria_0.5")
    {
        a = 0.22195;
        b = 1.81290;
    }
    else if(type_ == "car")
    {
        a = -1.491;
        b = 3.124;
    }
    else if(type_ == "motobike")
    {
        a = 3.688;
        b = 8.546;
    }
    else if(type_ == "bicycle")
    { 
        a = 9.658;
        b = 19.223;
    }
    else
    {
        KJB_THROW_2(Not_implemented, "No probability for this entity.");
    }

    return 1.0 / (1.0 + exp(a + b*score()));
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Deva_detection> kjb::parse_deva_detection
(
    std::istream& is, 
    const std::string& type
)
{
    // reorganize tokens into detection_bboxes
    std::vector<Deva_detection> detections;

    std::string line;

    while(std::getline(is, line))
    {
        Deva_detection cur_detection = parse_deva_detection_line(line, type);
        detections.push_back(cur_detection);
    }


    return detections;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Deva_detection> kjb::parse_deva_detection
(
    std::istream& is, 
    double score_thresh,
    const std::string& type
)
{
    using namespace std;
    vector<Deva_detection> detections;
    string line;
    while(getline(is, line))
    {
        Deva_detection cur_detection = parse_deva_detection_line(line, type);
        if(cur_detection.score() > score_thresh)
        {
            detections.push_back(cur_detection);
        }
    }
    return detections;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Deva_detection kjb::parse_deva_detection_line
(
    const std::string& line,
    const std::string& type
)
{ 
    const size_t BOXES_PER_ACTOR = 9;
    const size_t TOKENS_PER_BOX = 4;
    const size_t TOKENS_PER_ACTOR = BOXES_PER_ACTOR * TOKENS_PER_BOX;

    std::string cur_line(line);
    Deva_detection cur_detection;

    // split line on whitespace
    std::vector<std::string> tokens;
    boost::trim(cur_line);
    boost::split(tokens, cur_line, boost::is_any_of("\t \n\r"), boost::token_compress_on);
    if(tokens.size() == TOKENS_PER_ACTOR ||
           tokens.size() == TOKENS_PER_ACTOR + 1)
    {

        size_t i = 0;
        for(size_t box_i = 0; box_i < BOXES_PER_ACTOR; box_i++)
        {
            Vector p1(2), p2(2);

            p1[0] = boost::lexical_cast<double>(tokens[i++]);
            p1[1] = boost::lexical_cast<double>(tokens[i++]);
            p2[0] = boost::lexical_cast<double>(tokens[i++]);
            p2[1] = boost::lexical_cast<double>(tokens[i++]);

            cur_detection[box_i] = Bbox(p1, p2);
        }

        if(tokens.size() == TOKENS_PER_ACTOR + 1)
        {
            // score is also available
            
            assert(i == TOKENS_PER_ACTOR);
            cur_detection.score() = boost::lexical_cast<double>(tokens[i]);
        }  
        else // if the score is not available 
        {
            cur_detection.score() = (-DBL_MAX);
        }

    }
    // For detections using inria model, we only stored the full full body 
    else
    {
        size_t i = 0;
        size_t box_i = 0;
        assert(tokens.size() == 5);
        Vector p1(2), p2(2);
        p1[0] = boost::lexical_cast<double>(tokens[i++]);
        p1[1] = boost::lexical_cast<double>(tokens[i++]);
        p2[0] = boost::lexical_cast<double>(tokens[i++]);
        p2[1] = boost::lexical_cast<double>(tokens[i++]);
        cur_detection[box_i] = Bbox(p1, p2);
        cur_detection.score() = boost::lexical_cast<double>(tokens[i]);
    }

    cur_detection.type() = type;

    return cur_detection;
}

