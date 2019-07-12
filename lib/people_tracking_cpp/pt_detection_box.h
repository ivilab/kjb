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

/* $Id: pt_detection_box.h 18278 2014-11-25 01:42:10Z ksimek $ */

#ifndef PT_DETECTION_BOX_H
#define PT_DETECTION_BOX_H

#include <detector_cpp/d_bbox.h>
#include <m_cpp/m_vector.h>
#include <camera_cpp/perspective_camera.h>
#include <camera_cpp/camera_backproject.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ios>

namespace kjb {
namespace pt {

/**
 * @class   Detection_box
 *
 * @brief   Class that represents a detection bounding box from any source.
 */
struct Detection_box
{
    Bbox bbox;
    double prob_noise;
    std::string type;

    /**
     * @brief   Create data box from rectangle and 3D ground location.
     */
    Detection_box
    (
        const Bbox& bb,
        double pnoise,
        const std::string& model_type
    ) :
        bbox(bb),
        prob_noise(pnoise),
        type(model_type)
    {}

    /** @brief  Swap two detection boxes. */
    friend void swap(Detection_box& b1, Detection_box& b2)
    {
        using std::swap;
        swap(b1.bbox, b2.bbox);
        swap(b1.prob_noise, b2.prob_noise);
        swap(b1.type, b2.type);
    }
};

/**
 * @brief   Compares to boxes using middle of box. Needed because we
 *          have associated containers of these.
 */
inline
bool operator<(const Detection_box& b1, const Detection_box& b2)
{
    if(b1.bbox.get_bottom_center() < b2.bbox.get_bottom_center())
    {
        return true;
    }

    if(b1.bbox.get_bottom_center() > b2.bbox.get_bottom_center())
    {
        return false;
    }

    return b1.bbox.get_top_center() < b2.bbox.get_top_center();
}

/** @brief  Write the Detection_box. */
inline 
std::ostream& operator<<(std::ostream& ost, const Detection_box& dbox)
{
    std::streamsize w = ost.width();
    std::streamsize p = ost.precision();
    std::ios::fmtflags f = ost.flags();
    ost << std::scientific;
    
    ost << dbox.bbox;
    ost << std::setw(16) << std::setprecision(8) << dbox.prob_noise << " ";
    ost << dbox.type;

    ost.width(w);
    ost.precision(p);
    ost.flags(f);

    return ost;
}

/**
 * @brief Read in an Detection_box from a sigle line.
 */
inline 
Detection_box parse_detection_box(const std::string& line)
{
    using namespace std;

    istringstream istr(line);

    Vector center(2);
    istr >> center[0];
    istr >> center[1];

    double width;
    double height;
    istr >> width;
    istr >> height;

    Bbox box(center, width, height);

    double prob_noise;
    istr >> prob_noise;

    std::string tp;
    istr >> tp;

    IFT(istr.eof() || !istr.fail(), IO_error,
        "Cannot read Detection_box: line has wrong format.");

    return Detection_box(box, prob_noise, tp);
}

/**
 * @brief Read in a vector of Detection_box from a sigle file.
 */
std::vector<Detection_box> parse_detection_file(const std::string& filename);

/**
 * @brief   Check whether the deva detection box and the model box belong 
 *          to the same object based on their overlappings
 */
bool similar_boxes 
(
    const Bbox& model_box,
    const Bbox& data_box,
    double area_thresh_lower = 0.6,
    double area_thresh_upper = 1.2
);

/** @brief  Checks whether the bottom of a box back-projects to the ground. */
inline
bool box_on_ground(const Detection_box& dbox, const Perspective_camera& cam)
{
    Ground_back_projector gbp(cam, 0.0);
    Vector bbot = dbox.bbox.get_bottom_center();
    return !gbp(bbot[0], bbot[1]).empty();
}

}} // namespace kjb::pt

#endif /*PT_DETECTION_BOX_H */

