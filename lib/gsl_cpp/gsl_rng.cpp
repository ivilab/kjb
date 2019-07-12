/**
 * @file
 * @brief GSL random number generator code -- serialization code, that is
 * @author Andrew Predoehl
 *
 * GSL is the GNU Scientific Library.
 */

/*
 * $Id: gsl_rng.cpp 9821 2011-06-27 00:52:55Z predoehl $
 */

#include <l/l_sys_lib.h>
#include <l/l_sys_io.h>
#include <l/l_error.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_util.h>

#include "gsl_cpp/gsl_rng.h"

#include <sstream>

namespace {

FILE* make_ephemeral_file()
{
    // Make a temporary file.
    char fname[ 512 ];
    KJB( ETX( BUFF_GET_TEMP_FILE_NAME( fname ) ) );
    FILE* fp = kjb_c::kjb_fopen( fname, "w+" );
    ETX( 00 == fp );

    // This is the ol' unix trick, "unlink filename while file is still open."
    int rc1 = kjb_c::kjb_unlink( fname );
    if ( kjb_c::ERROR == rc1 )
    {
        ETX( kjb_c::kjb_fclose( fp ) );
        KJB_THROW_2( kjb::IO_error,
                                "Failed to unlink serialization temp file" );
    }

    return fp;
}


} // end anonymous ns



namespace kjb {

#ifdef KJB_HAVE_GSL
std::string gsl_rng_serialize_implementation( const gsl_rng* rng )
{
    FILE* fp = make_ephemeral_file();

    // Write the state bytes into the temporary file.
    int rc2 = gsl_rng_fwrite( fp, rng );

    // Try to read it back again into a string (if the write succeeded).
    if ( 0 == rc2 )
    {
        std::string state;
        rewind( fp );
        for( int ccc; ( ccc = kjb_c::kjb_fgetc( fp ) ) != EOF; )
        {
            state.push_back( char( ccc ) );
        }
        ETX( kjb_c::kjb_fclose( fp ) );
        return state;
    }

    // Write has failed us.
    ETX( kjb_c::kjb_fclose( fp ) );
    GSL_ETX( rc2 );

    /* NOTREACHED */
    KJB_THROW_2( KJB_error, "gsl_rng_fwrite() returned malformed error code" );
}


void gsl_rng_deserialize_implementation(
    gsl_rng* rng,
    const std::string& state
)
{
    FILE* fp = make_ephemeral_file();

    // write state into temporary file
    for( size_t iii = 0; iii < state.size(); ++iii )
    {
        int rc1 = kjb_c::kjb_fputc( fp, state[ iii ] );
        if ( EOF == rc1 )
        {
            ETX( kjb_c::kjb_fclose( fp ) );
            KJB_THROW_2( KJB_error, "write failure deserializing GSL RNG" );
        }
    }

    // read state from temporary file
    rewind( fp );
    int rc2 = gsl_rng_fread( fp, rng );
    ETX( kjb_c::kjb_fclose( fp ) );
    GSL_ETX( rc2 );
}
#endif


}
