/* $Id: test_learned_discrete_prior2.cpp 15736 2013-10-19 05:44:01Z predoehl $ */
/*=========================================================================== *
  |
  | Copyright (c) 1994-2010 by Kobus Barnard (author)
  |
  | Personal and educational use of this code is granted, provided that this
  | header is kept intact, and that the authorship is not misrepresented, that
  | its use is acknowledged in publications, and relevant papers are cited.
  |
  | For other use contact the author (kobus AT cs DOT arizona DOT edu).
  |
  | Please note that the code in this file has not necessarily been adequately
  | tested. Naturally, there is no guarantee of performance, support, or fitness
  | for any particular task. Nonetheless, I am interested in hearing about
  | problems that you encounter.
  |
  | Author:  Luca Del Pero
 *=========================================================================== */

#include <l/l_sys_lib.h>

#include <iostream>
#include <fstream>
#include <string.h>

#include <likelihood_cpp/learned_discrete_prior.h>

using namespace kjb;


// Takes list of objects to create histograms for as arguments.
// Objects must be one of the following:
//     bed, couch, table, chair, cabinet, door, window, picture_frame, 
//     subcategories
int main(int argc, char* argv[])
{
    if (! kjb_c::is_interactive()) return EXIT_SUCCESS;

    std::string in_file_name(argv[1]);
    Learned_discrete_prior dp(40, 1.0, 0.0,
                in_file_name.c_str());
    dp.write("./dp1.txt");
    Learned_discrete_prior dp2("./dp1.txt");
    dp2.write("./dp2.txt");
    //dp.plot_histogram(argv[2]);
}
