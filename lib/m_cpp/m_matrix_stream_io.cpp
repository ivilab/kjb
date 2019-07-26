/* $Id: m_matrix_stream_io.cpp 21596 2017-07-30 23:33:36Z kobus $ */

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
|  Author: Luca Del Pero
|
* =========================================================================== */

#include <iostream>
#include "m_cpp/m_matrix_stream_io.h"
#include "l/l_bits.h"
#include "m_cpp/m_matrix.h"
#include "l_cpp/l_int_matrix.h"



using namespace kjb;

/**
 * Writes an integer matrix to an ostream using binary format.
 * The format is: num_rows,num_cols followed by the matrix elements
 *
 * @param out the ostream to write to
 * @param imat the matrix to write
 */
void Matrix_stream_io::write_int_matrix(
    std::ostream& out,
    const Int_matrix & imat
)
{
     using namespace kjb_c;

     int num_rows   = imat.get_num_rows();
     int num_cols   = imat.get_num_cols();

     if(! kjb_is_bigendian() )
     {
         bswap_u32((uint32_t*)&(num_rows));
         bswap_u32((uint32_t*)&(num_cols));
     }

     out.write((char*)&num_rows, sizeof(int));
     out.write((char*)&num_cols, sizeof(int));

     if(out.fail() || out.eof())
     {
         throw IO_error("Could not write int matrix dimensions");
     }

     for(int i = 0; i < imat.get_num_rows(); i++)
     {
         for(int j = 0; j < imat.get_num_cols(); j++)
         {
             int value = imat(i,j);
             if(! kjb_is_bigendian() )
             {
                 bswap_u32((uint32_t*)&(value));
             }
             out.write((char*)&value, sizeof(int));
             if(out.fail() || out.eof())
             {
                 throw IO_error("Could not write int matrix dimensions");
             }
         }
     }
}

/**
 * Reads an integer matrix from an istream using binary format.
 * The format is: num_rows,num_cols followed by the matrix elements
 *
 * @param in the istream to read from
 * @param imat the matrix will be stored here
 */
void Matrix_stream_io::read_int_matrix(std::istream& in, Int_matrix & imat)
{
    using namespace kjb_c;

    int num_rows;
    int num_cols;
    int value;

    in.read((char*)&num_rows, sizeof(int));
    in.read((char*)&num_cols, sizeof(int));

    if(in.fail() || in.eof())
    {
        throw IO_error("Could not read int matrix dimensions");
    }

    if(! kjb_is_bigendian() )
    {
        bswap_u32((uint32_t*)&(num_rows));
        bswap_u32((uint32_t*)&(num_cols));
    }

    if( (num_rows < 0) || num_cols < 0 )
    {
        throw IO_error("Could not read int matrix dimensions, negative size was read");
    }
    imat.zero_out(num_rows, num_cols);

    for(int i = 0; i < num_rows; i++)
    {
        for(int j = 0; j < num_cols; j++)
        {
            in.read((char*)&value, sizeof(int));
            if(in.fail() || in.eof())
            {
                throw IO_error("Could not read matrix elements");
            }

            if(! kjb_is_bigendian() )
            {
                bswap_u32((uint32_t*)&(value));
            }
            imat(i,j) = value;
        }
    }

}

/**
 * Writes a matrix to an ostream using binary format.
 * The format is: num_rows,num_cols followed by the matrix elements
 *
 * @param out the ostream to write to
 * @param imat the matrix to write
 */
void Matrix_stream_io::write_matrix(std::ostream& out, const Matrix & imat)
{
     using namespace kjb_c;

     int num_rows   = imat.get_num_rows();
     int num_cols   = imat.get_num_cols();

     if(! kjb_is_bigendian() )
     {
         bswap_u32((uint32_t*)&(num_rows));
         bswap_u32((uint32_t*)&(num_cols));
     }

     out.write((char*)&num_rows, sizeof(int));
     out.write((char*)&num_cols, sizeof(int));

     if(out.fail() || out.eof())
     {
         throw IO_error("Could not write int matrix dimensions");
     }

     for(int i = 0; i < imat.get_num_rows(); i++)
     {
         for(int j = 0; j < imat.get_num_cols(); j++)
         {
             double value = imat(i,j);
             if(! kjb_is_bigendian() )
             {
                 bswap_u64((uint64_t*)&(value));
             }
             out.write((char*)&value, sizeof(double));
             if(out.fail() || out.eof())
             {
                 throw IO_error("Could not write int matrix elements");
             }
         }
     }
}

/**
 * Reads a matrix from an istream using binary format.
 * The format is: num_rows,num_cols followed by the matrix elements
 *
 * @param in the istream to read from
 * @param imat the matrix will be stored here
 */
void Matrix_stream_io::read_matrix(std::istream& in, Matrix & imat)
{
    using namespace kjb_c;

    int num_rows;
    int num_cols;
    double value;

    in.read((char*)&num_rows, sizeof(int));
    in.read((char*)&num_cols, sizeof(int));

    if(in.fail() || in.eof())
    {
        throw IO_error("Could not read int matrix dimensions");
    }

    if(! kjb_is_bigendian() )
    {
        bswap_u32((uint32_t*)&(num_rows));
        bswap_u32((uint32_t*)&(num_cols));
    }

    if( (num_rows < 0) || num_cols < 0 )
    {
        throw IO_error("Could not read int matrix dimensions, negative size was read");
    }
    imat.zero_out(num_rows, num_cols);

    for(int i = 0; i < num_rows; i++)
    {
        for(int j = 0; j < num_cols; j++)
        {
            in.read((char*)&value, sizeof(double));
            if(in.fail() || in.eof())
            {
                throw IO_error("Could not read matrix elements");
            }

            if(! kjb_is_bigendian() )
            {
                bswap_u64((uint64_t*)&(value));
            }
            imat(i,j) = value;
        }
    }

}
