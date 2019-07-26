/* $Id: m_serialization.cpp 21596 2017-07-30 23:33:36Z kobus $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2014 by Kobus Barnard (author)
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
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker


#include "m_cpp/m_serialization.h"
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <l_cpp/l_serialization.h>

namespace kjb
{


void kjb_serialize(boost::archive::text_iarchive& ar, Matrix& obj, const unsigned int version)
{
    return kjb_serialize_default(ar, obj, version);
}

void kjb_serialize(boost::archive::text_oarchive& ar, Matrix& obj, const unsigned int version)
{
    return kjb_serialize_default(ar, obj, version);
}

void kjb_serialize(boost::archive::text_iarchive& ar, Vector& obj, const unsigned int version)
{
    return kjb_serialize_default(ar, obj, version);
}

void kjb_serialize(boost::archive::text_oarchive& ar, Vector& obj, const unsigned int version)
{
    return kjb_serialize_default(ar, obj, version);
}

}
