/* $Id: m_serialization.h 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifndef KJB_M_CPP_M_SERLIALIZE_H
#define KJB_M_CPP_M_SERLIALIZE_H


namespace boost {
namespace archive {
    class text_iarchive;
    class text_oarchive;
} // namespace archive
} // namespace boost

namespace kjb
{

class Matrix;
class Vector;
//class Int_matrix;
//class Int_vector;

void kjb_serialize(boost::archive::text_iarchive& ar, Matrix& obj, const unsigned int /* version */);
void kjb_serialize(boost::archive::text_oarchive& ar, Matrix& obj, const unsigned int /* version */);
//
//void kjb_serialize(boost::archive::text_iarchive& ar, Int_matrix& obj, const unsigned int /* version */);
//void kjb_serialize(boost::archive::text_oarchive& ar, Int_matrix& obj, const unsigned int /* version */);
//
void kjb_serialize(boost::archive::text_iarchive& ar, Vector& obj, const unsigned int /* version */);
void kjb_serialize(boost::archive::text_oarchive& ar, Vector& obj, const unsigned int /* version */);
//
//void kjb_serialize(boost::archive::text_iarchive& ar, Int_vector& obj, const unsigned int /* version */);
//void kjb_serialize(boost::archive::text_oarchive& ar, Int_vector& obj, const unsigned int /* version */);

}

#endif
