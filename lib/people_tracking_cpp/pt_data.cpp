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

/* $Id: pt_data.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_data.h"
#include "people_tracking_cpp/pt_detection_box.h"
//#include "people_tracking_cpp/pt_face_detection.h"
#include "people_tracking_cpp/pt_util.h"
#include "mcmcda_cpp/mcmcda_data.h"
#include "l_cpp/l_exception.h"
#include "detector_cpp/d_deva_facemark.h"

#include <string>
#include <fstream>
#include <set>
#include <vector>

using namespace kjb;
using namespace kjb::pt;
using namespace kjb::mcmcda;

Box_data::Box_set Box_data::read_single_time(const std::string& filename) const
{
    std::ifstream ifs(filename.c_str());
    if(ifs.fail())
    {
        KJB_THROW_3(IO_error, "can't open file %s", (filename.c_str()));
    }
    Box_set data_t;
    std::string line;

    while(std::getline(ifs, line))
    {
        Detection_box dbox = parse_detection_box(line);
        if(dbox.prob_noise <= max_pnoise_)
        {
            standardize(dbox.bbox, width_, height_);
            data_t.insert(dbox);
        }
    }

    return data_t;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Box_data::write_single_time
(
    const Box_set& data_t,
    const std::string& filename
) const
{
    std::ofstream ofs(filename.c_str());
    if(ofs.fail())
    {
        KJB_THROW_3(IO_error, "can't open file %s", (filename.c_str()));
    }

    BOOST_FOREACH(Detection_box dbox, data_t)
    {
        unstandardize(dbox.bbox, width_, height_);
        ofs << dbox << std::endl;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Box_data::thin_out(size_t beg_fr, size_t end_fr, size_t every_nth)
{
    for(size_t i = 0; i < this->size(); i++)
    {
        if(i < beg_fr - 1 || i > end_fr - 1 || i % every_nth)
        {
            (*this)[i].clear();
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Box_data::remove_overlapping(double thresh)
{
    for(size_t i = 0; i < this->size(); i++)
    {
        Box_set& boxes = (*this)[i];

        Box_set bad_boxes;
        for(Box_set::iterator box_p = boxes.begin();
                              box_p != boxes.end();
                              box_p++)
        {
            if(bad_boxes.count(*box_p) > 0) continue;

            Box_set::iterator box_q = box_p;
            for(++box_q; box_q != boxes.end(); box_q++)
            {
                if(bad_boxes.count(*box_q) > 0) continue;

                double area_p = box_p->bbox.get_area();
                double area_q = box_q->bbox.get_area();
                const Bbox* small_box
                    = area_p < area_q ? &(box_p->bbox) : &(box_q->bbox);

                double olap = get_rectangle_intersection(box_p->bbox,
                                                         box_q->bbox);
                if(olap >= thresh * small_box->get_area())
                {
                    if(box_p->prob_noise > box_q->prob_noise)
                    {
                        bad_boxes.insert(*box_p);
                        break;
                    }
                    else
                    {
                        bad_boxes.insert(*box_q);
                    }
                }
            }
        }

        BOOST_FOREACH(const Detection_box& bx, bad_boxes)
        {
            boxes.erase(bx);
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
//
//std::set<Face_detection> Face_data::read_single_time
//(
//    const std::string& filename
//) const
//{
//    std::ifstream ifs(filename.c_str());
//    std::set<Face_detection> data_t;
//    std::string line;
//
//    while(std::getline(ifs, line))
//    {
//        Face_detection fdet = parse_face_line(line);
//        standardize(fdet.box(), width_, height_);
//        data_t.insert(fdet);
//    }
//
//    return data_t;
//}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Deva_facemark kjb::pt::build_deva_facemark
(
    const Vector& left_eye,
    const Vector& right_eye,
    const Vector& nose,
    const Vector& left_mouth,
    const Vector& right_mouth,
    double yaw
)
{
    std::vector<Vector> marks;
    yaw = -yaw;
    if(left_eye.empty())
    {
        ASSERT(yaw >= 60);
        ASSERT(left_mouth.empty());
        marks.resize(39, Vector(2, 0.0));
        std::fill(marks.begin() + 11, marks.begin() + 24, right_mouth);
        std::fill(marks.begin() + 24, marks.begin() + 33, right_eye);
        std::fill(marks.begin() + 33, marks.end(), nose);
    }
    else if(right_eye.empty())
    {
        ASSERT(yaw <= -60);
        ASSERT(right_mouth.empty());
        marks.resize(39, Vector(2, 0.0));
        std::fill(marks.begin() + 11, marks.begin() + 24, left_mouth);
        std::fill(marks.begin() + 24, marks.begin() + 33, left_eye);
        std::fill(marks.begin() + 33, marks.end(), nose);
    }
    else
    {
        ASSERT(yaw >= -120 && yaw <= 120);
        marks.resize(68, Vector(2, 0.0));
        std::fill(marks.begin() + 17, marks.begin() + 29, left_mouth);
        std::fill(marks.begin() + 29, marks.begin() + 37, right_mouth);
        std::fill(marks.begin() + 37, marks.begin() + 48, left_eye);
        std::fill(marks.begin() + 48, marks.begin() + 59, right_eye);
        std::fill(marks.begin() + 59, marks.end(), nose);
    }

    return Deva_facemark(marks, yaw);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Facemark_data kjb::pt::parse_deva_facemarks
(
    const std::vector<std::string>& fps,
    double img_width,
    double img_height
)
{
    using namespace std;
    Facemark_data facemarks;
    facemarks.reserve(fps.size());

    BOOST_FOREACH(const string& fp, fps)
    {
        ifstream ifs(fp.c_str());
        IFTD(!ifs.fail(), IO_error, "can't open file %s", (fp.c_str()));
        vector<Deva_facemark> fm = parse_deva_facemark(ifs);

        // standardize the face
        BOOST_FOREACH(Deva_facemark& mark, fm)
        {
            standardize(mark, img_width, img_height);
        }

        facemarks.push_back(fm);
    }

    return facemarks;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::write_deva_facemarks
(
    const Facemark_data& fm_data,
    const std::vector<std::string>& fps,
    double img_width,
    double img_height
)
{
    const size_t T = fm_data.size();

    IFT(fps.size() == T, Illegal_argument,
        "Cannot write facemark data; wrong number of file names.");

    for(size_t i = 0; i < T; i++)
    {
        std::vector<Deva_facemark> fm = fm_data[i];

        // standardize the face
        BOOST_FOREACH(Deva_facemark& mark, fm)
        {
            unstandardize(mark, img_width, img_height);
        }

        const std::string& fp = fps[i];
        std::ofstream ofs(fp.c_str());
        IFTD(!ofs.fail(), IO_error, "can't open file %s", (fp.c_str()));

        write_deva_facemark(fm, ofs);
    }
}

