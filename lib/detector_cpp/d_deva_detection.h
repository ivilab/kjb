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

/* $Id: d_deva_detection.h 18278 2014-11-25 01:42:10Z ksimek $ */


#ifndef D_DEVA_DETECTION_H
#define D_DEVA_DETECTION_H

#include <detector_cpp/d_bbox.h>
#include <camera_cpp/perspective_camera.h>

#include <string>
#include <vector>
#include <iostream>

namespace kjb
{

/**
 * Class representing a collection of "Deva" bounding boxes for a single part
 * detection.
 *
 * @author Kyle Simek, Jinyan Guan
 */
class Deva_detection
{

public:

    Deva_detection() : boxes_(9){}

    Deva_detection
    (
        double score, 
        const std::vector<Bbox>& boxes, 
        std::string type
    ) : score_(score),
        boxes_(boxes),
        type_(type)
    {}
    
    const Bbox& full_body() const { return boxes_[0]; }
    const Bbox& rhead() const { return boxes_[1]; }
    const Bbox& lhead() const { return boxes_[2]; }
    const Bbox& rshoulder() const { return boxes_[3]; }
    const Bbox& lshoulder() const { return boxes_[4]; }
    const Bbox& rhip() const { return boxes_[5]; }
    const Bbox& lhip() const { return boxes_[6]; }
    const Bbox& rfoot() const { return boxes_[7]; }
    const Bbox& lfoot() const { return boxes_[8]; }
    const Bbox& operator[] (size_t i) const 
    { 
        if( i > 8)
        {
            KJB_THROW_2(Index_out_of_bounds, 
                    "Index out of bound in Deva_detection ");
        }
        return boxes_[i];
    }

    Bbox& full_body() { return boxes_[0]; }
    Bbox& rhead() { return boxes_[1]; }
    Bbox& lhead() { return boxes_[2]; }
    Bbox& rshoulder() { return boxes_[3]; }
    Bbox& lshoulder() { return boxes_[4]; }
    Bbox& rhip() { return boxes_[5]; }
    Bbox& lhip() { return boxes_[6]; }
    Bbox& rfoot() { return boxes_[7]; }
    Bbox& lfoot() { return boxes_[8]; }
    Bbox& operator[] (size_t i) 
    { 
        if( i > 8)
        {
            KJB_THROW_2(Index_out_of_bounds, 
                    "Index out of bound in Deva_detection ");
        }
        return boxes_[i];
    }
    size_t size() { return boxes_.size(); } 

    double score() const { return score_; }
    double& score() { return score_; }

    const std::string& type() const { return type_; }
    std::string& type() { return type_; }

    double probability_of_noise() const; 

private:
    double score_;
    std::vector<Bbox> boxes_;
    std::string type_;
};

/**
 * @brief   Parse the output from Deva's part detector into a vector of
 *          Deva_detection 
 */
std::vector<Deva_detection> parse_deva_detection
(   
    std::istream& is, 
    const std::string& type = std::string("person")
);

/**
 * @brief   Parse the output from Deva's part detector 
 *          and prune by the confidence score
 */
std::vector<Deva_detection> parse_deva_detection
(
    std::istream& is, 
    double score_thresh,
    const std::string& type = std::string("person")
);

/**
 * @brief   Parse the output from Deva's part detector 
 */
Deva_detection parse_deva_detection_line
(
    const std::string& line,
    const std::string& type = std::string("person")
);

/** @brief Functor for comparing box distance */
class Compare_box_distance 
{
public:
    Compare_box_distance(const Vector& center)
        : compared_center(center) 
    {}

    bool operator()
    (
        const Deva_detection& box_1,
        const Deva_detection& box_2
    )
    {
        const Vector& center_1 = box_1.full_body().get_center();
        const Vector& center_2 = box_2.full_body().get_center();

        // Only compare the distance between the Deva detection 
        // and the compared box center for now
        return vector_distance(center_1, compared_center) < 
                vector_distance(center_2, compared_center);
    }

private: 
    Vector compared_center;
};

} // namespace kjb

#endif /* D_DEVA_DETECTION_H */
