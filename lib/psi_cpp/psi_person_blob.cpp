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
#include <psi_cpp/psi_person_blob.h>
#include <iostream> 
#include <sstream>

namespace kjb
{
namespace psi
{

/** /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ **/

Person_flow_blob parse_person_blob(const std::string& line)
{
    using namespace std; 
    const int tokens_per_line = 5; 

    istringstream istr(line);
    vector<double> elems;

    copy(istream_iterator<double>(istr), istream_iterator<double>(),
         back_inserter(elems));

    IFT(istr.eof() || !istr.fail() || elems.size() != tokens_per_line, 
            IO_error, "Person blob detection file has invalid format. ");

    Bbox box(Vector(elems[0], elems[1]), elems[2], elems[3]);
    double flow_velocity = elems[4]; 
    return Person_flow_blob(box, flow_velocity); 
}

/** /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ **/

std::vector<Person_flow_blob> parse_person_blobs(const std::string& fname)
{
    std::ifstream ifs(fname.c_str());
    if(ifs.fail())
    {
        KJB_THROW_3(IO_error, "can't open file %s ", (fname.c_str())); 
    }
    std::string line; 

    std::vector<Person_flow_blob> pfbs; 
    while(std::getline(ifs, line))
    {
        pfbs.push_back(parse_person_blob(line)); 
    }

    return pfbs; 
}

}
}
