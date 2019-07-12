/* $Id: psi_skeleton_detection.h 17393 2014-08-23 20:19:14Z predoehl $ */
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
   |  Author:  Jinyan Guan
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#ifndef PSI_SKELETON_DETECTION_H
#define PSI_SKELETON_DETECTION_H

#include <vector>
#include <iostream>

#include <psi_cpp/psi_util.h>
#include <psi_cpp/psi_bbox.h>

#include <l_cpp/l_exception.h>

#include <string>
namespace kjb
{
namespace psi
{

/**
 * @brief Skeleton class, a vector of body parts
 */
class Skeleton_detection : public std::vector<Bbox>
{
private: 
    typedef std::vector<Bbox> Base;
    double score; 
    friend std::ostream& operator<<(std::ostream& ost, const Skeleton_detection& skeleton);
    friend std::istream& operator<<(std::istream& ist, const Skeleton_detection& skeleton);

public:
    Skeleton_detection() : 
        Base(26), 
        score(-DBL_MAX)
    {}

    ~Skeleton_detection(){}

    void set_body_part(size_t body_part, const Bbox& part)
    {
        (*this)[body_part] = part;
    }

    void set_score(double score_)
    {
        score = score_;
    }

    const Bbox& get_body_part(size_t body_part) const
    {
        return (*this)[body_part];
    }

    double get_score() const 
    {
        return score; 
    }

    Bbox get_bounding_box() const;
};

/** @brief writes part into out stream */
inline
std::ostream& operator<<(std::ostream& ost, const Skeleton_detection& skeleton)
{
    std::streamsize w = ost.width();
    std::streamsize p = ost.precision();
    std::ios::fmtflags f = ost.flags();
    //ost << std::scientific;

    Skeleton_detection::const_iterator it;
    for(it = skeleton.begin(); it < skeleton.end(); it++)
    {
        Bbox box(*it);
        Vector offset(box.get_width()/2.0, box.get_height()/2.0);
        Vector tl = box.get_center() - offset;
        Vector br = box.get_center() + offset;

        ost << tl[0] << " " << tl[1] << " "
            << br[0] << " " << br[1] << " ";
    }
    ost << skeleton.get_score(); 

    ost.width( w );
    ost.precision( p );
    ost.flags( f );

    return ost;
}

std::vector<Skeleton_detection> parse_skeleton_detection(std::istream& ist);

}// namespace psi
}// namespace kjb

#endif /*PSI_SKELETON_DETECTION_H */
