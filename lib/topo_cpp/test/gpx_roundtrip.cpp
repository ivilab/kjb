/**
 * @file
 * @author Andy Predoehl
 * @brief test file for GPX load then save (GPX round-trip)
 */
/*
 * $Id: gpx_roundtrip.cpp 21357 2017-03-30 05:35:22Z kobus $
 */

#include <l/l_incl.h>
#include <l_cpp/l_stdio_wrap.h>
#include <topo_cpp/autolayer.h>

#include <iostream>
#include <vector>

// make up some random UTM coordinates, save to file, read file, compare.
int main2( int, const char* const* )
{
    const std::string filename1 = "points.gpx", filename2 = "schmoints.gpx";

    // load gpx file
    kjb::TopoFusion::AutoLayer l( filename1 );

    // write it
    KJB( ERE( l.write( filename2 ) ) );

    off_t fs1, fs2;
    KJB( ERE( get_file_size( filename1.c_str(), &fs1 ) ) );
    KJB( ERE( get_file_size( filename2.c_str(), &fs2 ) ) );
    if ( fs1 != fs2 )
    {
        kjb_c::set_error( "Files differ in size" );
        return kjb_c::ERROR;
    }

    kjb::File_Ptr_Read f1( filename1 ), f2( filename2 );

    // verify files are identical
    off_t ix = 0;
    for( int c1,c2; (c1 = fgetc( f1 )) != EOF && (c2 = fgetc( f2 )) != EOF; )
    {
        if ( c1 != c2 )
        {
            kjb_c::set_error( "Files differ at offset %u\n", ix );
            return kjb_c::ERROR;
        }
        ++ix;
    }

    KJB( ERE( kjb_unlink( filename2.c_str() ) ) );
    return kjb_c::NO_ERROR;
}

int main( int argc, const char* const* argv )
{
    KJB( EPETE( kjb_init() ) );

    try
    {
        KJB( EPETE( main2( argc, argv ) ) );
    }
    catch( const kjb::KJB_error& e )
    {
        e.print_details_exit();
        /* NOTREACHED */
        exit( 1 );
    }

    if ( kjb_c::is_interactive() )
    {
        kjb_c::pso( "Success!\n" );
    }

    kjb_c::kjb_cleanup();

    return EXIT_SUCCESS;
}
