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

/* $Id: deva_facemark.cpp 21293 2017-03-07 02:32:04Z jguan1 $ */

#include <detector_cpp/d_deva_facemark.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_test.h>
#include <l_cpp/l_exception.h>
#include <l/l_sys_io.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

using namespace kjb;
using namespace std;

bool equal(const Deva_facemark& f1, const Deva_facemark& f2)
{
    const std::vector<Vector>& le1 = f1.left_eye();
    const std::vector<Vector>& re1 = f1.right_eye();
    const std::vector<Vector>& no1 = f1.nose();
    const std::vector<Vector>& mo1 = f1.mouth();
    const std::vector<Vector>& ch1 = f1.chin();
    double yaw1 = f1.yaw();

    const std::vector<Vector>& le2 = f2.left_eye();
    const std::vector<Vector>& re2 = f2.right_eye();
    const std::vector<Vector>& no2 = f2.nose();
    const std::vector<Vector>& mo2 = f2.mouth();
    const std::vector<Vector>& ch2 = f2.chin();
    double yaw2 = f2.yaw();

    bool same = true;
    for(size_t i = 0; i < le1.size(); i++)
    {
        if(vector_distance(le1[i], le2[i]) > FLT_EPSILON)
        {
            same = false;
            break;
        }
    }
    if(!same) return same;
    for(size_t i = 0; i < re1.size(); i++)
    {
        if(vector_distance(re1[i], re2[i]) > FLT_EPSILON)
        {
            same = false;
            break;
        }
    }
    if(!same) return same;
    for(size_t i = 0; i < no1.size(); i++)
    {
        if(vector_distance(no1[i], no2[i]) > FLT_EPSILON)
        {
            same = false;
            break;
        }
    }
    if(!same) return same;
    for(size_t i = 0; i < mo1.size(); i++)
    {
        if(vector_distance(mo1[i], mo2[i]) > FLT_EPSILON)
        {
            same = false;
            break;
        }
    }
    if(!same) return same;
    for(size_t i = 0; i < ch1.size(); i++)
    {
        if(vector_distance(ch1[i], ch2[i]) > FLT_EPSILON)
        {
            same = false;
            break;
        }
    }
    if(!same) return same;

    return same;

}

int main(int argc, char** argv)
{
    string fpath("input/deva_facemark.txt");
    std::ifstream ifs(fpath.c_str());
    if(ifs.fail())
    {
        cout << "Test failed! "
             << "(Cannot read file '" << fpath << "')"
             << endl; 
        return EXIT_FAILURE;
    }

    // parse 
    vector<Deva_facemark> dfm = parse_deva_facemark(ifs);
    // write 
    string out_fpath("input/temp.txt");
    ofstream ofs(out_fpath.c_str());
    if(ofs.fail())
    {
        cout << "Test failed! "
             << "(Cannot open file '" << out_fpath << "')"
             << endl; 
        return EXIT_FAILURE;
    }

    write_deva_facemark(dfm, ofs);
    ifstream ifs_2(out_fpath.c_str());
    if(ifs_2.fail())
    {
        cout << "Test failed! "
             << "(Cannot read file '" << out_fpath << "')"
             << endl; 
        return EXIT_FAILURE;
    }

    vector<Deva_facemark> dfm2 = parse_deva_facemark(ifs_2);

    TEST_TRUE(dfm2.size() == dfm.size());

    for(size_t i = 0; i < dfm2.size(); i++)
    {
        TEST_TRUE(equal(dfm2[i], dfm[i]));
    }

    ETX(kjb_c::kjb_unlink(out_fpath.c_str()));

    RETURN_VICTORIOUSLY();
}

