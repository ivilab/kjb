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

/* $Id$ */

#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_detection_box.h>
#include <detector_cpp/d_bbox.h>
#include "utils.h"
#include <l_cpp/l_test.h>
#include <l_cpp/l_filesystem.h>
#include <l_cpp/l_exception.h>
#include <l/l_sys_io.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace kjb;
using namespace pt;
using namespace std;

const bool VERBOSE = false;

/** @brief  Compare two Detection_box's. */
bool compare_dboxes(const Detection_box& dbox1, const Detection_box& dbox2);

/** @brief  Main. */
int main(int argc, char** argv)
{
    try
    {
        double img_width = 800;
        double img_height = 600;
        size_t num_frames = 50;

        // create output directory
        string outdir = "output/box_data_cpp";
        kjb_c::kjb_mkdir(outdir.c_str());

        // create data
        Box_data data(img_width, img_height);
        Facemark_data fm_data;
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        create_or_read_scene(
            argc, argv, num_frames, img_width, img_height,
            data, fm_data, flows_x, flows_y, scene, 3);

        // sanity check
        assert(data.size() == num_frames);

        // write data
        string fmt = outdir + "/%05d.txt";
        vector<string> fnames = strings_from_format(fmt, num_frames);
        data.write(fnames);

        // read data
        Box_data data2(img_width, img_height);
        data2.read(fnames);

        // compare data
        TEST_TRUE(data2.size() == num_frames);
        for(size_t t = 0; t < num_frames; t++)
        {
            if(VERBOSE) cout << "FRAME " << (t + 1) << endl;

            TEST_TRUE(equal(
                        data[t].begin(),
                        data[t].end(),
                        data2[t].begin(),
                        compare_dboxes));

            if(VERBOSE) cout << endl;
        }
    }
    catch(const kjb::Exception& ex)
    {
        ex.print_details();
        cerr << endl;
        return EXIT_FAILURE;
    }

    RETURN_VICTORIOUSLY();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool compare_dboxes(const Detection_box& dbox1, const Detection_box& dbox2)
{
    const Bbox& b1 = dbox1.bbox;
    const Bbox& b2 = dbox2.bbox;

    if(VERBOSE)
    {
        cout << "c_x: " << b1.get_centre_x() << " == "
                        << b2.get_centre_x() << endl;
        cout << "c_y: " << b1.get_centre_y() << " == "
                        << b2.get_centre_y() << endl;
        cout << "width: " << b1.get_width() << " == "
                          << b2.get_width() << endl;
        cout << "height: " << b1.get_height() << " == "
                           << b2.get_height() << endl;
        cout << "p_n: " << dbox1.prob_noise << " == "
                        << dbox2.prob_noise << endl;
        cout << "type: " << dbox1.type << " == " << dbox2.type << endl;
        cout << endl;
    }

    const double wpres = 1e-5;
    return (fabs(b1.get_centre_x() - b2.get_centre_x() < wpres))
            && (fabs(b1.get_centre_y() - b2.get_centre_y() < wpres))
            && (fabs(b1.get_width() - b2.get_width() < wpres))
            && (fabs(b1.get_height() - b2.get_height() < wpres))
            && (fabs(dbox1.prob_noise - dbox2.prob_noise < wpres))
            && (dbox1.type == dbox2.type);
}

