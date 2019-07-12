/* $Id: m_mat_util.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
#include "m_cpp/m_mat_util.h"
#include "m_cpp/m_matrix.h"

namespace kjb {

Matrix_stl_view get_matrix_stl_view(kjb::Matrix& mat)
{
    return Matrix_stl_view(&mat(0,0), boost::extents[mat.get_num_rows()][mat.get_num_cols()]);
}
} // namespace kjb
