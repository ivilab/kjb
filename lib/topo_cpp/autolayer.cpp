/**
 * @file
 * @brief Implementation for the AutoLayer class.
 * @author Andrew Predoehl
 *
 * AutoLayer is an RAII wrapper for Scott's layer structure.
 */
/*
 * $Id: autolayer.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "l/l_incl.h"
#include "l_cpp/l_util.h"
#include "topo_cpp/autolayer.h"

namespace kjb
{
namespace TopoFusion
{

/* if you take the following namespace out and re-run Doxygen, look at the
 * return type on method AutoLayer::set_numTracks.  If it is "void" then the
 * problem is gone.  If the return type is listed as something all weird,
 * then congratulations, you are observing Doxygen making a mistake.
 */
namespace { }


/**
 * @brief change the number of tracks, allocating or freeing as necessary
 * @param num   new number of tracks.  If negative or if the same as the
 *              current number of tracks, we exit silently.  Otherwise we
 *              adjust the number of tracks.  Since we do not store
 *              separate size and capacity values, we always realloc.
 */
void AutoLayer::set_numTracks( int num )
{
    if ( num < 0 || my_layer.numTracks - num == 0 ) return;

    // If we already have some tracks, free them
    if ( my_layer.numTracks > 0 )
    {
        ASSERT( my_layer.tracks );
        while( my_layer.numTracks )
            kjb_c::kjb_free( my_layer.tracks[ --my_layer.numTracks ].s.points);
        kjb_c::kjb_free( my_layer.tracks );
    }
    ASSERT( 0 == my_layer.numTracks );
    my_layer.numTracks = num;
    if ( 0 < num )
    {
        KJB( my_layer.tracks = N_TYPE_MALLOC( track, num ) );
        for( int i = 0; i < num; ++i )
        {
            initTrack( & my_layer.tracks[ i ]  );
        }
    }
}


/**
 * @brief define the first track in a layer.
 * @param num number of points in the track
 * @param src array of points (pt objects) in the track
 * @return pointer to array of points where the input is stored
 *
 * A Layer can hold multiple tracks.  This method lets you specify the
 * points (an array of struct pt objects) for the first track.
 */
pt* AutoLayer::set_track0points( unsigned num, const pt* src )
{
    /*
     * GCC complains that the test below is always false
    if ( num < 0 )
        return 0;
    */
    if ( 0 == my_layer.numTracks ) set_numTracks( 1 );
    ASSERT( 0 <= my_layer.numTracks );
    ASSERT( my_layer.tracks );
    if ( 0 < my_layer.tracks[ 0 ].s.numPoints )
    {
        ASSERT( my_layer.tracks[ 0 ].s.points );
        kjb_c::kjb_free( my_layer.tracks[ 0 ].s.points );
    }
    my_layer.tracks[ 0 ].s.numPoints = num;
    my_layer.tracks[ 0 ].s.points = 0;
    if ( 0 < num )
    {
        using namespace kjb_c;
        pt *dest;
        my_layer.tracks[ 0 ].s.points = dest = N_TYPE_MALLOC( pt, num );
        if ( 0 == dest )
        {
            KJB_THROW_2( kjb::Runtime_error,
                                    "AutoLayer::set_track0points bad alloc" );
        }
        if ( src )
        {
            while( num-- )
            {
                *dest++ = *src++;
            }
        }
    }
    return my_layer.tracks[ 0 ].s.points;
}


/**
 * @brief A layer can hold a list of waypoints; this adds one to the list
 *
 * We CANNOT use the add waypoint method of layer.h because that assumes the
 * waypoint is to be inserted somewhere in the global Layer structure, which
 * we no longer use.  This method emulates that one; REALLOC makes it briefer.
 */
void AutoLayer::add_waypoint( const pt& p, const char* name, int index )
{
    using kjb_c::Malloc_size;

    waypoint*& w = my_layer.waypoints;  // easy-to-read aliases
    int& nw = my_layer.numWaypoints;
    int debug_pre = nw;
    ASSERT( 0 <= debug_pre );

    w = ( waypoint* ) kjb_c:: KJB_REALLOC( w, sizeof( waypoint ) * ++nw );
    ASSERT( w );
    waypoint* wnew = & w[ nw - 1 ]; 
    ASSERT( wnew );
    ASSERT( debug_pre + 1 == my_layer.numWaypoints );
    initWaypoint( wnew );
    wnew->point = p;
    int ct = snprintf( wnew->name, MAX_TRACKSTRING, "%s%d", name, index );
    ASSERT( ct < int( MAX_TRACKSTRING ) );
}

} // end ns TopoFusion
} // end namespace kjb

