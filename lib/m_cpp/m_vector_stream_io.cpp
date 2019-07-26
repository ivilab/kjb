/* $Id: m_vector_stream_io.cpp 21596 2017-07-30 23:33:36Z kobus $ */

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
#include "m_cpp/m_vector_stream_io.h"
#include "l/l_bits.h"
#include "l_cpp/l_exception.h"
#include "l_cpp/l_int_vector.h"
#include "m_cpp/m_vector.h"


namespace kjb
{

/**
 * Writes an integer vector to an ostream using binary format.
 * The format is: num_rows,num_cols followed by the vector elements
 *
 * @param out the ostream to write to
 * @param ivec the vector to write
 */
void Vector_stream_io::write_int_vector(
    std::ostream& out,
    const Int_vector & ivec
)
{
     using namespace kjb_c;

     int num_elts   = ivec.size();

     if(! kjb_is_bigendian() )
     {
         bswap_u32((uint32_t*)&(num_elts));
     }

     out.write((char*)&num_elts, sizeof(int));

     if(out.fail() || out.eof())
     {
         throw IO_error("Could not write int vector dimensions");
     }

     for(int i = 0; i < ivec.size(); i++)
     {
         int value = ivec(i);
         if(! kjb_is_bigendian() )
         {
             bswap_u32((uint32_t*)&(value));
         }
         out.write((char*)&value, sizeof(int));
         if(out.fail() || out.eof())
         {
             throw IO_error("Could not write int vector dimensions");
         }
     }
}

/**
 * Reads an integer vector from an istream using binary format.
 * The format is: num_rows,num_cols followed by the vector elements
 *
 * @param in the istream to read from
 * @param ivec the vector will be stored here
 */
void Vector_stream_io::read_int_vector(
    std::istream& in,
    Int_vector & ivec
)
{
    using namespace kjb_c;

    int num_elts;
    int value;

    in.read((char*)&num_elts, sizeof(int));

    if(in.fail() || in.eof())
    {
        throw IO_error("Could not read int vector dimensions");
    }

    if(! kjb_is_bigendian() )
    {
        bswap_u32((uint32_t*)&(num_elts));
    }

    if( num_elts < 0)
    {
        throw IO_error("Could not read int vector dimensions, negative size was read");
    }
    ivec.resize(num_elts, 0.0);

    for(int i = 0; i < num_elts; i++)
    {
        in.read((char*)&value, sizeof(int));
        if(in.fail() || in.eof())
        {
            throw IO_error("Could not read vector elements");
        }

        if(! kjb_is_bigendian() )
        {
            bswap_u32((uint32_t*)&(value));
        }
        ivec(i) = value;
    }

}

/**
 * Writes a vector to an ostream using binary format.
 * The format is: num_rows,num_cols followed by the vector elements
 *
 * @param out the ostream to write to
 * @param ivec the vector to write
 */
void Vector_stream_io::write_vector(std::ostream& out, const Vector & ivec)
{
     using namespace kjb_c;

     int num_elts   = ivec.size();

     if(! kjb_is_bigendian() )
     {
         bswap_u32((uint32_t*)&(num_elts));
     }

     out.write((char*)&num_elts, sizeof(int));

     if(out.fail() || out.eof())
     {
         throw IO_error("Could not write vector dimensions");
     }

     for(int i = 0; i < ivec.size(); i++)
     {
         double value = ivec(i);
         if(! kjb_is_bigendian() )
         {
             bswap_u64((uint64_t*)&(value));
         }
         out.write((char*)&value, sizeof(double));
         if(out.fail() || out.eof())
         {
             throw IO_error("Could not write vector dimensions");
         }
     }
}

/**
 * Reads a vector from an istream using binary format.
 * The format is: num_rows,num_cols followed by the vector elements
 *
 * @param in the istream to read from
 * @param ivec the vector will be stored here
 */
void Vector_stream_io::read_vector(std::istream& in, Vector & ivec)
{
    using namespace kjb_c;

    int num_elts;
    double value;

    in.read((char*)&num_elts, sizeof(int));

    if(in.fail() || in.eof())
    {
        throw IO_error("Could not read vector dimensions");
    }

    if(! kjb_is_bigendian() )
    {
        bswap_u32((uint32_t*)&(num_elts));
    }

    if( num_elts < 0)
    {
        throw IO_error("Could not read vector dimensions, "
                       "negative size was read");
    }
    ivec.zero_out(num_elts);

    for(int i = 0; i < num_elts; i++)
    {
        in.read((char*)&value, sizeof(double));
        if(in.fail() || in.eof())
        {
            throw IO_error("Could not read vector elements");
        }

        if(! kjb_is_bigendian() )
        {
            bswap_u64((uint64_t*)&(value));
        }
        ivec(i) = value;
    }

}

} // namespace kjb
