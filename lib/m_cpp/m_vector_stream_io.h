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

/* $Id: m_vector_stream_io.h 21596 2017-07-30 23:33:36Z kobus $ */

/**
 * @file
 * @author Kobus Barnard
 * @author Luca Del Pero
 *
 * @brief Declares functions to read and write vector classes with streams.
 *
 * The format is compatible with kjb matrices raw
 * format at c level (see l/l_int_mat_io.h).
 */

#ifndef VECTOR_STREAM_IO_H_INCLUDED
#define VECTOR_STREAM_IO_H_INCLUDED

//#include "m_cpp/m_vector.h"
//#include "l_cpp/l_int_vector.h"
#include <iosfwd>
//#include <vector>

namespace kjb {

class Vector;
class Int_vector;

/**
 * @brief functions to read and write vector classes with streams.
 * @ingroup kjbLinearAlgebra
 */
class Vector_stream_io
{
public:

    /// @brief Writes an integer vector to an ostream using binary format.
    static void write_int_vector(std::ostream& out,const Int_vector & ivec);

    /// @brief Reads an integer vector from an istream using binary format.
    static void read_int_vector(std::istream& in, Int_vector & ivec);

    /// @brief Writes a vector to an ostream using binary format.
    static void write_vector(std::ostream& out,const Vector & imat);

    /// @brief Reads a vector from an istream using binary format.
    static void read_vector(std::istream& in, Vector & imat);


};

}

#endif


