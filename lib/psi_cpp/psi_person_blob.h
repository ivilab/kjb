/* $Id */
/* {{{=========================================================================== *
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
 * =========================================================================== }}}*/
#ifndef PSI_PERSON_BLOB
#define PSI_PERSON_BLOB

#include <gr_cpp/gr_2D_bounding_box.h>
#include <psi_cpp/psi_util.h>
#include <vector> 
#include <string> 

namespace kjb
{
namespace psi
{

/**
 * @brief   A class represent the blob detection based on the optical 
 * flow magnitude images 
 *
 */
struct Person_flow_blob
{
    Person_flow_blob() {}
    Person_flow_blob
    (
         const Bbox& box_, 
         double velocity
    ) : box(box_),
        flow_velocity(velocity)
    {}  

    Bbox box; 
    double flow_velocity; 
};

/**
 * @brief   Parse the optical flow blob 
 *
 */ 
Person_flow_blob parse_person_blob(const std::string& line); 

std::vector<Person_flow_blob> parse_person_blobs(const std::string& fname); 

}
}

#endif
