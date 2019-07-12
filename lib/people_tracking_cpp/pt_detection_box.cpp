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
   |  Author:  Ernesto Brau, Jinyan Guan
 * =========================================================================== */

/* $Id: pt_detection_box.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "people_tracking_cpp/pt_detection_box.h"
#include "l_cpp/l_exception.h"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

using namespace kjb;
using namespace kjb::pt; 

std::vector<Detection_box> kjb::pt::parse_detection_file
(
    const std::string& filename
)
{
    std::ifstream ifs(filename.c_str());
    if(ifs.fail())
    {
        KJB_THROW_3(IO_error, "can't open file %s", (filename.c_str()));
    }
    std::vector<Detection_box> dboxes; 
    std::string line;
    while(std::getline(ifs, line))
    {
        Detection_box dbox = parse_detection_box(line);
        dboxes.push_back(dbox);
    }
    return dboxes; 
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool kjb::pt::similar_boxes
(
    const Bbox& model_box,
    const Bbox& data_box,
    double area_thresh_lower,
    double area_thresh_upper
)
{
    double olap = get_rectangle_intersection(data_box, model_box);
    //double drea = data_box.get_area();
    double mrea = model_box.get_area();
    //double small_area = drea < mrea? drea : mrea; 
    //double big_area = drea > mrea? drea: mrea;
    double distance = vector_distance(model_box.get_center(), 
                                      data_box.get_center());
    double dist_thresh = (data_box.get_width() + model_box.get_width())/4.0;

    /////////////////////////////////////////////////
    //std::cout << "(" << model_box.get_left() << ", " 
    //<< model_box.get_right() << ", " 
    //<< model_box.get_bottom() << ", " 
    //<< model_box.get_top() << ") VS " 
    //<< "(" << data_box.get_left() << ", " 
    //<< data_box.get_right() << ", " 
    //<< data_box.get_bottom() << ", " 
    //<< data_box.get_top() << ")\n";
    //if(olap > 0)
    //{
    //    std::cout << "olap = " << olap << std::endl;
    //    std::cout << "drea = " << drea << std::endl;
    //    std::cout << "mrea = " << mrea << std::endl;
    //    //std::cout << "small_area = " << small_area << std::endl;
    //    //std::cout << "big_area = " << big_area << std::endl;
    //    std::cout << "distance = " << distance << std::endl;
    //    std::cout << "dist_thresh = " << dist_thresh << std::endl;
    //}
    /////////////////////////////////////////////////

    //if(olap > small_area * area_thresh_small 
    //   && olap > big_area * area_thresh_big
    if( olap > mrea * area_thresh_lower
        && olap < mrea * area_thresh_upper
        //&& drea < mrea * 1.5
       && distance < dist_thresh)
    {
        return true;
    }
    //else
    //{
    //    if(olap > 0)
    //    {
    //        std::cout << "mrea * area_thresh_lower: " << mrea *area_thresh_lower 
    //           << " | mrea * area_thresh_upper: " << mrea * area_thresh_upper 
    //           << std::endl;
    //    }
    //}

    /*const Vector& mc = model_box.get_center();
    const Vector& dc = data_box.get_center();

    // If both box centers are contained in each other,
    // they are considered to correspond to the same object
    if(mc[0] < data_box.get_right() &&
       mc[0] > data_box.get_left() &&
       mc[1] < data_box.get_top() &&
       mc[1] > data_box.get_bottom() && 
       dc[0] < model_box.get_right() &&
       dc[0] > model_box.get_left() &&
       dc[1] < model_box.get_top() &&
       dc[1] > model_box.get_bottom())
        return true;
        */
    return false;
}

