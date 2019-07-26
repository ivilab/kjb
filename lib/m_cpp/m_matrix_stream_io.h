/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2007-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Luca Del Pero                                       |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: m_matrix_stream_io.h 18278 2014-11-25 01:42:10Z ksimek $ */

/**
 * @file
 * @author Kobus Barnard
 * @author Luca Del Pero
 * @brief Declare functions to read and write matrix classes with iostream.
 *
 * The format is compatible with kjb matrices' raw
 * format at c level (see l/l_int_mat_io.h).
 */

#ifndef MATRIX_STREAM_IO_H_INCLUDED
#define MATRIX_STREAM_IO_H_INCLUDED

#include <iosfwd>

namespace kjb {

class Matrix;
class Int_matrix;

/**
 * @brief static functions to read and write matrix classes with iostream.
 * @ingroup kjbLinearAlgebra
 */
class Matrix_stream_io
{
public:

    /// @brief Writes an integer matrix to an ostream using binary format.
    static void write_int_matrix(std::ostream& out, const Int_matrix & imat);

    /// @brief Reads an integer matrix from an istream using binary format.
    static void read_int_matrix(std::istream& in, Int_matrix & imat);

    /// @brief Writes a matrix to an ostream using binary format.
    static void write_matrix(std::ostream& out, const Matrix & imat);

    /// @brief Reads a matrix from an istream using binary format.
    static void read_matrix(std::istream& in, Matrix & imat);

};

}

#endif


