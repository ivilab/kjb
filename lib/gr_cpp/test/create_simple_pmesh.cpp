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
 |  Author:  Luca Del Pero
 * =========================================================================== */



#include <gr_cpp/gr_parapiped.h>

using namespace kjb;

int main()
{

  Parapiped pp(0,1,0,1,1,0,1,1,1,1,0,1);

  const std::vector<kjb::Polygon>& faces = pp.get_faces();

    return 0;
}


