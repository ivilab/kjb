/* $Id: m_flip.h 21596 2017-07-30 23:33:36Z kobus $ */

#ifndef KJB_CPP_EDGE_FLIP_H
#define KJB_CPP_EDGE_FLIP_H

#include "m_cpp/m_matrix.h"

namespace kjb
{

/// @brief deprecated synonym for Matrix::ow_vertical_flip
inline void flip_matrix_ud(Matrix& m)
{
    m.ow_vertical_flip();
}

/// @brief deprecated synonym for Matrix::ow_horizontal_flip
inline void flip_matrix_lr(Matrix& m)
{
    m.ow_horizontal_flip();
}

} // namespace kjb

#endif
