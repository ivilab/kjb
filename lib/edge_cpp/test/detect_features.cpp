/* $Id$ */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
 * =========================================================================== */

#include <string>
#include <fstream>
#include <iostream>
#include <ctime>

#include "i_cpp/i_image.h"
#include "edge_cpp/vanishing_point_detector.h"
#include "edge_cpp/features_manager.h"
#include "l/l_sys_rand.h"

using namespace std;

int main(int argc, char **argv)
{
    using namespace kjb;

    if(argc != 3)
    {
        std::cout << "Usage: ./detect_features <path_to_base_dir> <image_name> " << std::endl;
        return 1;
    }

    try
    {
    string img_name(argv[1]);
    img_name.append("/");
    string features(img_name);
    features.append("features/");

    string cmuf(features);
    cmuf.append(argv[2]);
    cmuf.append("_cmu_features.txt");

    std::cout << cmuf << std::endl;

    std::vector<Vanishing_point> cmuvpts;
    double focal;
    //kjb::read_CMU_vanishing_points(cmuvpts, focal, cmuf);

    img_name.append(argv[2]);
    img_name.append(".jpg");

    string assignments(features);
    assignments.append("/images/");
    assignments.append(argv[2]);
    string vpts(assignments);
    assignments.append("_as.jpg");
    vpts.append("_vpts.jpg");
    string corners(assignments);
    corners.append("_corners.jpg");
    features.append(argv[2]);
    features.append("_features.txt");


    Image img(img_name.c_str());
    Image img2 = img;
    Image img3 = img;
    Image img4 = img;

    std::vector<Vanishing_point> thevpts;
    double focal2;
    for(unsigned int i = 0; i < thevpts.size(); i++)
    {
        thevpts[i].write(std::cout);
    }


    bool success = detect_vanishing_points(thevpts, focal, img_name);
    Line_segment_set ls;
    detect_long_connected_segments(ls, img_name, 30);
    ls.randomly_color(img2);
    img2.write("Kovesi.jpg");
    /*bool success =  robustly_estimate_vanishing_points_Kovesi(thevpts, focal, img, ls);
    if(!success)
    {
        Line_segment_set ls2;
        ls2.read_from_Kovesi_format("./prova2.txt");
        img2 = img;
        ls2.randomly_color(img2);
        img2.write("Kovesi2.jpg");
        success =  robustly_estimate_vanishing_points_Kovesi(thevpts, focal, img, ls2);
        ls = ls2;
    }*/
    std::cout << "Success:" << success <<  std::endl;

    for(unsigned int i = 0; i < thevpts.size(); i++)
    {
        thevpts[i].write(std::cout);
    }
    for(unsigned int i = 0; i < ls.size(); i++)
    {
        unsigned int _index = assign_to_vanishing_point(0.20, &(ls.get_segment(i)), thevpts);
        double r = 0.0;
        double g = 0.0;
        double b = 0.0;
        if(_index == 0)
        {
            r = 255.0;
        }
        else if(_index == 1)
        {
            g = 255.0;
        }
        else
        {
            b = 255.0;
        }
        if(_index < 3)
        {
            draw_mid_point_to_vanishing_point(img3, ls.get_segment(i), thevpts[_index], r, g, b, 1.0 );
        }
    }

    std::string outpath(argv[1]);
    outpath.append("/features/images/");
    outpath.append(argv[2]);
    outpath.append(".jpg");

    img3.write(outpath.c_str());

    std::string outfpath(argv[1]);
    outfpath.append("/features/");
    outfpath.append(argv[2]);
    outfpath.append("_proper_features.txt");



    Manhattan_world * mw = new Manhattan_world(thevpts, focal);

    kjb::Edge_set * es = ls.convert_to_edge_set(img.get_num_rows(), img.get_num_cols());
    Edge_segment_set * ess = new Edge_segment_set();
    for(unsigned int i = 0; i < ls.size(); i++)
    {
        ess->add_segment(Edge_segment(es->get_edge(i), ls.get_segment(i)));
    }
    mw->assign_segments_to_vpts(*ess);
    Features_manager fem(es, ess, mw);
    fem.write(outfpath.c_str());

    return 1;
    Features_manager fm(img);


    fm.get_manhattan_world().draw_segments_with_vp_assignments(img2);
    fm.get_manhattan_world().draw_lines_from_segments_midpoint_to_vp(img3);
    fm.get_manhattan_world().draw_corners(img4);
    img2.write("assignments.jpg");
    img3.write("vpts.jpg");
    img2.write(assignments.c_str());
    img3.write(vpts.c_str());
    img4.write(corners.c_str());
    fm.write(features.c_str());
    }
    catch(KJB_error e)
    {
        e.print(std::cout);
    }
    catch(Illegal_argument e)
    {
        e.print(std::cout);
     }

    return 0;
}

