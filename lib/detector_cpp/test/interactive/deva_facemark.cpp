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

/* $Id: deva_facemark.cpp 19991 2015-10-29 19:50:11Z jguan1 $ */

#include <i_cpp/i_image.h>
#include <m_cpp/m_vector.h>
#include <detector_cpp/d_deva_facemark.h>
#include <l_cpp/l_test.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

using namespace kjb;
using namespace std;

void draw_point
(
    Image& img, 
    const Vector& pt,
    const Image::Pixel_type& pix,
    int pw
);

void draw_point
(
    Image& img, 
    const Vector& pt,
    const Image::Pixel_type& pix,
    int pw
)
{
    assert(pt.size() == 2);
    img.draw_point(pt(1), pt(0), pw, pix);
}

int main(int argc, char** argv)
{
    if(argc != 5)
    {
        cout << "Usuage: " << argv[0] << " img-fp detection-fp out-img-fp out-fp\n";
        return EXIT_SUCCESS;
    }
    string img_fp(argv[1]);
    string det_fp(argv[2]);
    string out_img_fp(argv[3]);
    string out_fp(argv[4]);
    //int mouth_index = atoi(argv[4]);

    ifstream ifs(det_fp.c_str());
    if(ifs.fail())
    {
        cout << "Test failed! "
             << "(Cannot read file '" << det_fp << "')"
             << endl; 
        return EXIT_FAILURE;
    }

    vector<Deva_facemark> dfms = parse_deva_facemark(ifs);
    Image img(img_fp);
    for(size_t i = 0; i < dfms.size(); i++)
    {
        const Deva_facemark& dfm = dfms[i];
        if(dfm.score() < -0.8) break;
        const vector<Vector>& left_eye = dfm.left_eye();
        const vector<Vector>& right_eye = dfm.right_eye();
        const vector<Vector>& nose = dfm.nose();
        const vector<Vector>& mouth = dfm.mouth();
        const vector<Vector>& chin = dfm.chin();
        double ps = 1.0;
        // blue
        Image::Pixel_type le_pix = {0.0, 0.0, 255.0};
        // green
        Image::Pixel_type re_pix = {0.0, 255.0, 0.0};
        // red
        Image::Pixel_type no_pix = {255.0, 0.0, 0.0};
        // white
        Image::Pixel_type mo_pix = {255.0, 255.0, 255.0};
        // yellow
        Image::Pixel_type lm_pix = {255.0, 255.0, 0.0};
        // purple
        Image::Pixel_type rm_pix = {255.0, 0.0, 255.0};
        // teal
        Image::Pixel_type ch_pix = {0.0, 255.0, 255.0};

        std::cout << dfm.score() << std::endl;

        // draw
        /*std::for_each(left_eye.begin(), left_eye.end(), 
                      boost::bind(draw_point, boost::ref(img), _1, le_pix, ps));

        std::for_each(right_eye.begin(), right_eye.end(), 
                      boost::bind(draw_point, boost::ref(img), _1, re_pix, ps));

        std::for_each(nose.begin(), nose.end(), 
                      boost::bind(draw_point, boost::ref(img), _1, no_pix, ps));
        */

        /*cout << "num of faces: " << mouth.size() << std::endl;
        std::for_each(mouth.begin(), mouth.end(), 
                      boost::bind(draw_point, boost::ref(img), _1, mo_pix, ps));
        Image::Pixel_type pix = {0.0, 0.0, 0.0};
        draw_point(img, mouth[mouth_index], pix, 3);
        */

        //std::for_each(chin.begin(), chin.end(), 
        //              boost::bind(draw_point, boost::ref(img), _1, ch_pix, ps));

        Vector le = dfm.left_eye_mark();
        Vector re = dfm.right_eye_mark();
        Vector no = dfm.nose_mark();
        Vector lm = dfm.left_mouth_mark();
        Vector rm = dfm.right_mouth_mark();

        ps = 2.0;
        if(!le.empty()) draw_point(img, le, le_pix, ps);
        if(!re.empty()) draw_point(img, re, re_pix, ps);
        draw_point(img, no, no_pix, ps);
        if(!lm.empty()) draw_point(img, lm, lm_pix, ps);
        if(!rm.empty()) draw_point(img, rm, rm_pix, ps);

        
        img.write(out_img_fp);
    }

    //write out the deva face marks
    ofstream ofs(out_fp.c_str());
    IFT(!ofs.fail(), IO_error, "Can't open file %s");
    write_deva_facemark(dfms, ofs);

    return EXIT_SUCCESS;
}
