/* $Id: psi_deva_skeleton.h 17393 2014-08-23 20:19:14Z predoehl $ */
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

#ifndef PSI_DEVA_SKELETON_H
#define PSI_DEVA_SKELETON_H

#include <gr_cpp/gr_2D_bounding_box.h>
#include <iostream>
#include <vector>

namespace kjb
{
namespace psi
{

typedef Axis_aligned_rectangle_2d Bbox; 

/**
 *
 *            
 *                       0
 *                       |
 *                  2----1----14
 *                 / \       /  \
 *                /   |     |    \
 *               3    7     19    15
 *               |    |     |     |
 *               4    8     20    16
 *               |    |     |     |
 *               5    9     21    17
 *               |    |     |     |
 *               6    10    22    18
 *                    |     |
 *                    11    23
 *                    |     |
 *                    12    24
 *                    |     |
 *                    13    25
**/

class Deva_skeleton_boxes : public std::vector<Bbox>
{
private:
    typedef std::vector<Bbox> Base;

    //friend std::vector<Deva_skeleton_boxes> parse_deva_skeletons(std::istream& is);

public:
    Deva_skeleton_boxes() :
        Base(26)
    {}

    ~Deva_skeleton_boxes(){}
    const Bbox& get_part(size_t index) const 
    {
        IFT(index < this->size(), Illegal_argument,  "Part index is out of range.");
        return (*this)[index];
    }
    
    void set_part(size_t index, const Bbox& box)
    {
        IFT(index < this->size(), Illegal_argument,  "Part index is out of range.");
        (*this)[index] = box;
    }
    
    void set_score(double score) { score_ = score; }
    double get_score() const { return score_; }

private:
    double score_;

};

/**
 * Parse the output from Deva skeleton detectors into a vector of 
 * Deva_skeleton_boxes.
 *
 */
std::vector<Deva_skeleton_boxes> parse_deva_skeletons(std::istream& is);

/**
 * Parse the output from Deva skeleton tracks into a vector of 
 * vector of Deva_skeleton_boxes.
 *
 */
std::vector<std::vector<Deva_skeleton_boxes> > parse_deva_skeleton_tracks
(
    std::istream& is, 
    size_t num_frames
);

/// @brief move coordinate ssystem origin to center of image
void standardize(Deva_skeleton_boxes& boxes, double cam_width, double cam_height);

}} //namespace kjb::psi

#endif /* PSI_DEVA_SKELETON_H */
