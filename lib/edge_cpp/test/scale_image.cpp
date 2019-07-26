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

using namespace std;
using namespace kjb_c;

#define MAX_NUM_ROWS 400
#define MAX_NUM_COLS 400
#define MIN_NUM_ROWS 250
#define MIN_NUM_COLS 350

int main(int argc, char ** argv)
{
    using namespace kjb;

    if(argc != 4)
    {
        std::cout << "USAGE: <path_to_original_directory> <path_to_output_directory> <image_name>" << std::endl;
        return 1;
    }
    string name;
    string outname;

    name.append(argv[1]);
    name.append("/");
    name.append(argv[3]);
    name.append(".jpg");

    outname.append(argv[2]);
    outname.append("/");
    outname.append(argv[3]);
    outname.append(".jpg");

    Image img(name.c_str());
    kjb_c::KJB_image * cimg = 0;

    double scale_factor = 1.0;

    if( (img.get_num_rows() < MIN_NUM_ROWS) && (img.get_num_cols() < MIN_NUM_COLS))
    {
        scale_factor = ((double)MIN_NUM_ROWS/(double)img.get_num_rows());
    }
    else if(img.get_num_rows() > img.get_num_cols())
    {
        if(img.get_num_rows() > MAX_NUM_ROWS)
        {
            scale_factor = ((double)MAX_NUM_ROWS)/((double)img.get_num_rows());
        }
    }
    else if(img.get_num_cols() > MAX_NUM_COLS)
    {
        scale_factor = ((double)MAX_NUM_COLS)/((double)img.get_num_cols());
    }

    std::cout << "The scale factor is "<< scale_factor << std::endl;

    EPETE(scale_image_size
    (
        &cimg,
        img.c_ptr(),
        scale_factor
    ));

    Image outimg(cimg);
    outimg.write(outname);

    return 0;
}

