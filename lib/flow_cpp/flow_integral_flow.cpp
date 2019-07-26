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
   |  Author:  Jinyan Guan, Ernesto Brau
 * =========================================================================== */

/* $Id: flow_integral_flow.cpp 18278 2014-11-25 01:42:10Z ksimek $ */

#include <flow_cpp/flow_integral_flow.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_exception.h>
#include <algorithm>
#include <utility>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string> 

#include <boost/assign/std/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

using namespace kjb;

Integral_flow::Integral_flow(const Matrix& flow, size_t subsample_rate) :
    ss_rate_(subsample_rate),
    img_width_(flow.get_num_cols()),
    img_height_(flow.get_num_rows()),
    width_(img_width_ / ss_rate_),
    height_(img_height_ / ss_rate_),
    image_(width_ * height_)
{
    Matrix iflow(img_height_, img_width_);

    iflow(0, 0) = flow(0, 0);
    for(size_t j = 1; j < img_width_; j++)
    {
        iflow(0, j) = iflow(0, j - 1) + flow(0, j);
    }

    for(size_t i = 1; i < img_height_; i++)
    {
        iflow(i, 0) = iflow(i - 1, 0) + flow(i, 0);
    }

    for(size_t i = 1; i < img_height_; i++)
    {
        for(size_t j = 1; j < img_width_; j++)
        {
            iflow(i, j) = flow(i, j) - iflow(i - 1, j - 1)
                            + iflow(i - 1, j) + iflow(i, j - 1);
        }
    }

    size_t k = 0;
    for(size_t i = 0; i < img_height_; i += ss_rate_)
    {
        for(size_t j = 0; j < img_width_; j += ss_rate_)
        {
            image_[k++] = iflow(i, j);
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Integral_flow::Integral_flow(const std::string& fname)
{
    using namespace std;
    ifstream ifs(fname.c_str());
    if(ifs.fail())
    {
        KJB_THROW_3(IO_error, "Can't open file %s", (fname.c_str()));
    }

    // get the first line which has the following information
    // subsampling rate, img_width, img_height
    string line;
    getline(ifs, line);
    vector<string> tokens;
    boost::trim(line);
    boost::split(tokens, line, boost::is_any_of("\t \n\r"), 
                 boost::token_compress_on);
    assert(tokens.size() == 3);

    img_width_ = boost::lexical_cast<size_t>(tokens[0]);
    img_height_ = boost::lexical_cast<size_t>(tokens[1]);
    ss_rate_ = boost::lexical_cast<size_t>(tokens[2]);

    width_ = img_width_/ss_rate_;
    height_ = img_height_/ss_rate_;

    size_t total_length = width_ * height_;
    image_.resize(width_ * height_, 0.0);

    // read in the precomputed integral image
    size_t i = 0;
    while(getline(ifs, line))
    {
        boost::trim(line);
        boost::split(tokens, line, boost::is_any_of("\t \n\r"), 
                     boost::token_compress_on);
        assert(tokens.size() == width_);
        BOOST_FOREACH(const string& token, tokens)
        {
            image_[i++] = boost::lexical_cast<float>(token);
        }
    }

    assert(i == total_length);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Integral_flow::flow_sum(double x, double y) const
{
    typedef std::pair<size_t, size_t> xy_pair;

    IFT(x <= img_width_ && y <= img_height_ && x >= 0 && y >= 0,
        Illegal_argument,
        "Integral_flow: cannot compute flow; point outside image.");

    //// bilinear interpolation

    // first get nearest existing point up and to the left
    xy_pair ulc = ul_corner(x, y);

    // compute integral values at corners around (x, y)
    // subtract one because integral image includes x, y
    // and we don't want that
    int ix1 = ulc.first - 1;
    int iy1 = ulc.second - 1;
    int ix2 = ix1 + 1;
    int iy2 = iy1 + 1;

    // compute II value at four corners; if one is negative
    // it means it's value is 0 (no image there)
    double f11 = ix1 < 0 || iy1 < 0 ? 0.0 : at(iy1, ix1);
    double f12 = ix1 < 0 ? 0.0 : at(iy2, ix1);
    double f21 = iy1 < 0 ? 0.0 : at(iy1, ix2);
    double f22 = at(iy2, ix2);

    // we need doubles in real space
    double x1 = ix1 * (int)ss_rate_;
    double y1 = iy1 * (int)ss_rate_;
    double x2 = ix2 * (int)ss_rate_;
    double y2 = iy2 * (int)ss_rate_;

    // subtract one from these for same reason as above
    x--;
    y--;

    // compute interpolated value of II
    double fxy = (f11*(x2 - x)*(y2 - y) +
                  f21*(x - x1)*(y2 - y) +
                  f12*(x2 - x)*(y - y1) +
                  f22*(x - x1)*(y - y1));

    fxy /= ((x2 - x1)*(y2 - y1));

    return fxy;
}       

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Integral_flow::write(const std::string& fname)
{
    std::ofstream ofs(fname.c_str());
    IFTD(ofs, IO_error, "can't open file %s", (fname.c_str()));
    // Write the header
    ofs << img_width_ << " " 
        << img_height_ << " " 
        << ss_rate_ << std::endl;

    ofs << std::scientific;
    // Write the integral image
    for(size_t i = 0; i < height_; i++)
    {
        for(size_t j = 0; j < width_; j++)
        {
            ofs << std::setw(16) << std::setprecision(8) << at(i, j) << " ";
        }
        ofs << std::endl;
    }
}

