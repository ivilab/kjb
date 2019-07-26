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
#include "edge_cpp/feature_histogram.h"

using namespace std;

int main(int argc, char **argv)
{
    using namespace kjb;

    if(argc < 4)
    {
        std::cout << "Usage: <base_dir> <out_dir> <input_image>" << std::endl;
        return 1;
    }

    std::string img_name(argv[1]);
    img_name.append("/");
    img_name.append(argv[3]);
    img_name.append(".jpg");

    std::string histo_name(argv[1]);
    histo_name.append("/histos/");
    histo_name.append(argv[3]);
    histo_name.append("_chi_bin");
    DTLib::CImg<DTLib::FloatCHistogramPtr> * color_histo = DTLib::ReadColorHistoImg(histo_name);

    Image img(img_name.c_str());
    Feature_histogram histo(*color_histo, 5, img.get_num_rows(), img.get_num_cols(), 10);
    std::cout << "Num darts:" << histo.get_num_darts() << std::endl;
    Image img2;
    histo.draw_darts(img2);
    img2.write("darts.tiff");

    zap(color_histo);

    return 0;
}

