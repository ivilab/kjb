/**
 * @file
 * @brief interface for low-level NED reader functions
 * @author Andrew Predoehl
 *
 * This performs interpolation on NED data using Gaussian processes, a
 * technique historically called "kriging."  This file uses its own
 * implementation rather than the libkjb Gaussian process code (sorry).
 */
/*
 * $Id: nedget.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "l/l_sys_io.h"
#include "l_cpp/l_stdio_wrap.h"
#include "l_cpp/l_cpp_bits.h"
#include "l/l_debug.h"

#include <iterator>
#include <algorithm>

#include "topo_cpp/nedget.h"
#include "l_cpp/l_util.h"



namespace
{

typedef float Ned_Float_t;

const char* NONSQUARE_FMT = "Input expected to have positive square size, "
                            "instead has size %u.\n";

const char* BAD_FILE_FMT = "Input file %s has invalid contents.";

const char* AMBI_BYTEORDER_FMT = "Ambiguous byteorder in %s.\n";


/*
 * Test whether a buffer, putatively of floats representing elevation values in
 * the USA in meters, indeed seems to contain such values.  Return true if so.
 * If the buffer seems to contain any junk values or out-of-range values,
 * return false.  Valid values lie within the range NED_MIN to NED_MAX or are
 * equal to NED_MISSING.
 */
bool seems_legit( const float* begin, const float* end )
{
    for( const float* ppp = begin; ppp < end; ++ppp )
    {
        if ( ! finite( *ppp ) || isnand( *ppp ) )
        {
            return false;   // infinity or NaN is not legit.  Not even once.
        }
        if ( kjb::NED_MISSING == *ppp )
        {
            continue;       // the MISSING sentinel gets a pass.
        }
        if ( *ppp < kjb::NED_MIN || kjb::NED_MAX < *ppp )
        {
            return false; // value is out of range
        }
    }
    return true;
}


/*
 * Given a filename, try to decide the file's byte order (little-endian or
 * big-endian).  If the byte order is discernable, we return NO_ERROR after we
 * store the byte order flag to *byteorder (which must not equal null).
 * If we cannot discern the byte order, we return ERROR.
 */
int try_to_resolve_byteorder(
    const std::string& fn,
    enum kjb::NED13_FLOAT_AUTODETECT* byteorder
)
{
    KJB( NRE( byteorder ) );
    if ( kjb::NED_AD_UNCERTAIN == *byteorder )
    {
        KJB( ERE( autodetect_ned_byteorder( fn, byteorder ) ) );
        if ( kjb::NED_AD_UNCERTAIN == *byteorder )
        {
            kjb_c::set_error( AMBI_BYTEORDER_FMT, fn.c_str() );
            return kjb_c::ERROR;
        }
    }
    return kjb_c::NO_ERROR;
}


} // end anonymous ns



namespace kjb
{


/**
 * @brief read the floating-point values of a NED13 file into a deque.
 * @return kjb_c::ERROR or kjb_c::NO_ERROR to indicate failure or success
 * @param[in] fn    Filename (possibly with path) of local file, which is
 *                  assumed to contain nothing except binary-format float data,
 *                  IEEE 754 single-precision.  The byte order is assumed to be
 *                  "network byte order," i.e., MSB-first.
 * @param[out] odq  Pointer to std::deque<float> into which to store the data.
 *                  It must not equal null.
 * @param[in] flip  Flag to indicate when input is in "network byte order,"
 *                  which on Intel-style architectures requires the order to be
 *                  reversed.  To repeat, TRUE indicates "network byte order,"
 *                  also known as MSB-first, common on Motorola-style machines.
 *
 * In case it is not clear, the purpose of this function is to handle the
 * low-level file input and byte-order conversion.
 */
int get_ned_fdeq( const std::string& fn, std::deque< float >* odq, bool flip )
{
    // preconditions
    KJB(NRE( odq ));

    // get ready to read

    Ned_Float_t buf[ 128 * 1024 ]; // size is relatively arbitrary
    FILE *fff = kjb_c::kjb_fopen( fn.c_str(), "r" );
    KJB( NRE( fff ) );

    // read from file, in chunks, optionally change endian order

    odq -> clear();
    for( long bct = -1; 0 < ( bct=kjb_c::kjb_fread( fff, buf, sizeof buf ) ); )
    {
        ASSERT( 0 == bct % sizeof( Ned_Float_t ) );
        const size_t float_count = size_t( bct ) / sizeof( Ned_Float_t );
        if ( flip )
        {
            swap_array_bytes(buf, float_count);
        }
        std::copy( buf, buf + float_count, std::back_inserter( *odq ) );
    }
    KJB(ERE( kjb_c::kjb_fclose( fff ) ));
    return kjb_c::NO_ERROR;
}



/**
 * @brief read the floating-point values of a NED13 file into a deque.
 * @return kjb_c::ERROR or kjb_c::NO_ERROR to indicate failure or success
 * @param[in] fn    Filename, same as 'fn' in sibling function (q.v.).
 * @param[out] odq  Pointer to std::deque<float> into which to store the data.
 *                  It must not equal null.
 * @param[in] byteorder If NED_AD_MSBFIRST or NED_AD_LSBFIRST, we read the
 *                  input under that assumption.  If NED_AD_UNCERTAIN, we try
 *                  to autodetect the byte order.
 * @see autodetect_ned_byteorder
 *
 * This is the same as
 * get_ned_fdeq( const std::string&, std::deque< float >*, bool )
 * except that this function tries to autodetect the byteorder of the data
 * if byteorder is set to NED_AD_UNCERTAIN.
 */
int get_ned_fdeq(
    const std::string& fn,
    std::deque< float >* odq,
    enum NED13_FLOAT_AUTODETECT byteorder
)
{
    KJB(NRE( odq ));
    KJB(ERE( try_to_resolve_byteorder( fn, &byteorder ) ));
    return get_ned_fdeq( fn, odq, NED_AD_MSBFIRST == byteorder );
}




/**
 * @brief convert a deque of floats into a square kjb Matrix
 * @return kjb_c::ERROR or kjb_c::NO_ERROR to indicate failure or success
 * @param[in]   ibuf    Deque of floating point values, possibly derived from
 *                      get_ned_fdeq().  The number of entries must be a
 *                      positive square.
 * @param[out]  omat    Pointer to matrix into which to store the data.
 *                      It must not equal null.  Previous contents (if any)
 *                      will be clobbered.  Data are copied in row-major order.
 */
int ned_fdeq_to_matrix( const std::deque< float >& ibuf, Matrix* omat )
{
    KJB(NRE( omat ));

    // compute matrix size
    const int edgesz = static_cast< int >( sqrt( double( ibuf.size() ) ) );

    if ( ibuf.size() < 1 || size_t( edgesz * edgesz ) != ibuf.size() )
    {
        kjb_c::set_error( NONSQUARE_FMT, ibuf.size() );
        return kjb_c::ERROR;
    }

    // read from deque, into matrix, in row-major order
    omat -> resize( edgesz, edgesz );
    std::deque< float >::const_iterator iii = ibuf.begin();
    for( int row = 0; row < edgesz; ++row, iii += edgesz )
    {
        std::copy( iii, iii + edgesz, & omat -> at( row, 0 ) );
    }
    ASSERT( ibuf.end() == iii );

    return kjb_c::NO_ERROR;
}



/**
 * @brief read the floating-point values of a NED13 file into a square matrix.
 * @return kjb_c::ERROR or kjb_c::NO_ERROR to indicate failure or success
 * @param[in] fn    Filename (possibly with path) of local file, which is
 *                  assumed to contain nothing except binary-format float data,
 *                  IEEE 754 single-precision.  The byte order is assumed to be
 *                  "network byte order," i.e., MSB-first.  The total number is
 *                  assumed to be a perfect square.
 * @param[out] omat Pointer to matrix into which to store the data.  It must
 *                  not equal null.
 * @param[in] flip  Flag used to indicate input in MSB-first order -- see
 *                  description of flip parameter of get_ned_fdeq().
 *
 * This function is implemented by calling get_ned_fdeq() and
 * ned_fdeq_to_matrix(), which is a little less efficient that possible --
 * we read the file into memory (in the deque) and then copy it into another
 * part of memory (in the Matrix).  But I doubt the penalty is significant, and
 * it's a natural way to factor the computation.
 */
int get_ned_matrix( const std::string& fn, Matrix* omat, bool flip )
{
    KJB(NRE( omat ));
    std::deque< float > buf;
    if ( kjb_c::ERROR == get_ned_fdeq( fn, &buf, flip ) )
    {
        kjb_c::add_error( BAD_FILE_FMT, fn.c_str() );
        return kjb_c::ERROR;
    }

    return ned_fdeq_to_matrix( buf, omat );
}



/**
 * @brief read the floating-point values of a NED13 file into a square matrix.
 * @return kjb_c::ERROR or kjb_c::NO_ERROR to indicate failure or success
 * @param[in] fn    same as 'fn' for get_ned_matrix( const std::string&,
 *                  Matrix*, bool ).
 * @param[out] omat Pointer to matrix into which to store the data.  It must
 *                  not equal null.
 * @param[in] ad    If NED_AD_MSBFIRST or NED_AD_LSBFIRST, we read the input
 *                  under that assumption.  If NED_AD_UNCERTAIN, we try to
 *                  autodetect the byte order.
 * @see autodetect_ned_byteorder
 */
int get_ned_matrix(
    const std::string& fn,
    Matrix* omat,
    enum NED13_FLOAT_AUTODETECT byteorder
)
{
    KJB(NRE( omat ));
    KJB(ERE( try_to_resolve_byteorder( fn, &byteorder ) ));
    return get_ned_matrix( fn, omat, NED_AD_MSBFIRST == byteorder );
}



/**
 * @brief attempt to determine the byteorder of the input file.
 * @return kjb_c::ERROR or kjb_c::NO_ERROR to indicate problems with the file.
 * @param[in]   fn      Filename of the float file to read
 * @param[out]  result  Output parameter containing a sentinel value indicating
 *                      either the byteorder (if the evidence is clear) or a
 *                      value indicating that the test was unsuccessful.
 *                      This pointer must not equal null.
 *
 * If the file IO fails, if number of floats in the file is not a square, or
 * if neither byte order yields plausible data, this returns ERROR.
 */
int autodetect_ned_byteorder(
    const std::string& fn,
    enum NED13_FLOAT_AUTODETECT *result
)
{
    KJB(NRE( result ));
    *result = NED_AD_UNCERTAIN;

    // read the file size, check that it is a square
    off_t fbsize;
    KJB(ERE( get_file_size( fn.c_str(), &fbsize ) ));
    const int edgesz = static_cast< int >( sqrt( double( fbsize ) ) );
    if ( edgesz < 1 || edgesz * edgesz != fbsize )
    {
        kjb_c::set_error( NONSQUARE_FMT, fbsize );
        return kjb_c::ERROR;
    }

    // read in some of the file
    const size_t SMALL = 32 * 1024; // any fairly modest size should do
    Ned_Float_t buf[ SMALL ];
    File_Ptr_Read fff( fn );
    long bct = kjb_c::kjb_fread( fff, buf, sizeof buf );
    if ( bct < std::min( off_t( sizeof buf ), fbsize ) )
    {
        kjb_c::set_error( "Error reading file %s.\n", fn.c_str() );
        return kjb_c::ERROR;
    }

    // test this buf using both possible byte orders
    const bool lsbfirst = seems_legit( buf, buf + SMALL );
    swap_array_bytes(buf, SMALL);
    const bool msbfirst = seems_legit( buf, buf + SMALL );

    if ( lsbfirst && msbfirst )
    {
        return kjb_c::NO_ERROR; // both seem OK, which is surprising.
    }
    if ( ! lsbfirst && ! msbfirst )
    {
        kjb_c::set_error( "File %s contains invalid floats.\n", fn.c_str() );
        return kjb_c::ERROR;    // both seem bad, which is disastrous.
    }

    *result = lsbfirst ?  NED_AD_LSBFIRST : NED_AD_MSBFIRST;
    return kjb_c::NO_ERROR;
}


}

