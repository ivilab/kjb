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

/* $Id: pt_data.h 18751 2015-04-03 20:33:13Z ernesto $ */

#ifndef PT_DATA_H
#define PT_DATA_H

#include <mcmcda_cpp/mcmcda_data.h>
#include <people_tracking_cpp/pt_detection_box.h>
//#include <people_tracking_cpp/pt_face_detection.h>
#include <detector_cpp/d_deva_facemark.h>
#include <set>
#include <string>
#include <vector>

namespace kjb {
namespace pt {

/**
 * @class   Box_data
 *
 * Class representing tracking box-data. This class inherits from the
 * general mcmcda::Data class, and overwrites the read member function.
 */
class Box_data : public mcmcda::Data<Detection_box>
{
public:
    typedef std::set<Detection_box> Box_set;

private:
    double width_;
    double height_;
    double max_pnoise_;

public:
    /** @brief   Constructor */
    Box_data
    (
        double width,
        double height,
        double max_prob_noise = 1.0
    ) :
        width_(width),
        height_(height),
        max_pnoise_(max_prob_noise)
    {}

    /**
     * @brief   Reads boxes for single time from single file.
     */
    Box_set read_single_time(const std::string& filename) const;

    /** @brief  Write boxes for single time to single file.  */
    void write_single_time
    (
        const Box_set& data_t,
        const std::string& filename
    ) const;

    /**
     * @brief   Thins out the data. Only frames in [beg_fr, end_fr] are kept,
     *          and only every every_nth frame is kept.
     */
    void thin_out(size_t beg_fr, size_t end_fr, size_t every_nth);

    /** @brief  Removes overlapping boxes from a set. */
    void remove_overlapping(double thresh);

    /** @brief  Returns the image width. */
    double image_width() const { return width_; }

    /** @brief  Returns the image height. */
    double image_height() const { return height_; }
};

///**
// * @class   Face_data
// *
// * Class representing tracking face-data. This class inherits from the
// * general mcmcda::Data class, and overwrites the read member function.
// */
//class Face_data : public mcmcda::Data<Face_detection>
//{
//private:
//    double width_;
//    double height_;
//
//public:
//    /** @brief   Constructor */
//    Face_data(double width, double height) : width_(width), height_(height)
//    {}
//
//    /**
//     * @brief   Reads boxes for single time from single file.
//     */
//    std::set<Face_detection> read_single_time
//    (
//        const std::string& filename
//    ) const;
//};

/** @brief   Return a Deva_facemark from face mark */ 
Deva_facemark build_deva_facemark
(
    const Vector& lefe_eye,
    const Vector& right_eye,
    const Vector& nose,
    const Vector& left_mouth,
    const Vector& right_mouth,
    double yaw
);

typedef std::vector<std::vector<Deva_facemark> > Facemark_data;

Facemark_data parse_deva_facemarks
(
    const std::vector<std::string>& fps,
    double img_width,
    double img_height
);

void write_deva_facemarks
(
    const Facemark_data& fm_data,
    const std::vector<std::string>& fps,
    double img_width,
    double img_height
);

}} //namespace kjb::pt

#endif /*PT_DATA_H */

