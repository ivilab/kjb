
/* $Id: g_quaternion.h 6946 2010-10-05 17:12:13Z ksimek $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
* =========================================================================== */

#ifndef KJB_ORTHOGONAL_CORNER_H
#define KJB_ORTHOGONAL_CORNER_H


#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"


namespace kjb {

    void get_3D_corner_orientation_from_2D_corner_lines
    (
        const kjb::Vector & corner2D_1,
        const kjb::Vector & corner2D_2,
        const kjb::Vector & corner2D_3,
        const kjb::Vector & position_2D,
        const kjb::Vector & position_3D,
        double focal_length,
        int epsilon,
        kjb::Vector & corner3D_1,
        kjb::Vector & corner3D_2,
        kjb::Vector & corner3D_3
    );
} // namespace kjb
#endif
