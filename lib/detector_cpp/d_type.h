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
   |  Author:  Kyle Simek, Jinyan Guan
 * =========================================================================== */

/* $Id: d_type.h 14240 2013-04-11 08:13:35Z jguan1 $ */

#ifndef KJB_D_TYPE_H
#define KJB_D_TYPE_H

#include <string>
#include <vector>
#include <istream>
#include <ostream>

namespace kjb {

/**
 * @file Structs and functions for representing, reading, and 
 * writing detection types. 
 */

// To add a new type, add entries to detection_type, and detection_type_map;
enum Detection_type { DEVA, CV_FRONTAL_DEFAULT, CV_FRONTAL_ALT, 
                      CV_FRONTAL_ALT2, CV_FRONTAL_ALT_TREE, 
                      CV_PROFILE, FACE_COM, NUM_DETECTION_TYPES};
        
/** @brief  Get all types. */
std::vector<std::string> get_all_detection_type_names();

/** @brief  Get the name of a detection type. */
const std::string& get_detection_type_name(Detection_type type);

/** @brief  Get the type of a detection name. */
Detection_type get_detection_type(const std::string& name);

/** @brief  Stream out an detection. */
std::ostream& operator<<(std::ostream& ost, Detection_type type);

/** @brief  Stream in an detection. */
std::istream& operator>>(std::istream& ist, Detection_type& type);

} // namespace kjb

#endif
