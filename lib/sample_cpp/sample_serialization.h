/* $Id: sample_serialization.h 17393 2014-08-23 20:19:14Z predoehl $ */
/* {{{=========================================================================== *
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
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#ifndef KJB_CPP_SAMPLER_SERIALIZATION_H
#define KJB_CPP_SAMPLER_SERIALIZATION_H


#include <sample_cpp/sample_base.h>

// KJB DEPENDENCY
#ifdef KJB_HAVE_BST_SERIAL
#include <boost/serialization/deque.hpp>

namespace boost 
{
namespace serialization
{

template<class Archive, class Model>
void serialize(Archive& ar, Step_result<Model>& obj, const unsigned int version)
{
    // this class is likely to change, so it's important to use the version functionality to handle changes
    using namespace boost;
    ar & obj.type;
    ar & obj.accept;
    ar & obj.lt;
}

template<class Archive, class Model>
void serialize(Archive& ar, Step_log<Model>& obj, const unsigned int version)
{
    ar & boost::serialization::base_object<std::deque<Step_result<Model> > >(obj);
}


} // namespace serialization
} // namespace boost

#endif  /* KJB_HAVE_BST_SERIAL */

#endif
