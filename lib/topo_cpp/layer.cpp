/**
 * @file
 * @brief Functions helping manage the layer struct.
 * @author Scott Morris
 * @author Alan Morris
 * @author Andrew Predoehl
 *
 * Functions for managing and computing statistics for 'layers' with possibly
 * multiple tracks, waypoints, etc.
 * Originally from TopoFusion.
 * I (Andrew) radically simplified many data structures, as apparent below.
 */
/*
 * $Id: layer.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "l/l_sys_lib.h"
#include "l/l_sys_io.h"
#include "l/l_sys_mal.h"
#include "l_cpp/l_util.h"

// C++ definitions from TopoFusion
#include "topo_cpp/layer.h"
#include "topo_cpp/LatLong-UTMconversion.h"


namespace kjb
{
namespace TopoFusion
{

/**
 * @brief compute approx. Great %Circle distance between two UTM points
 * @param a UTM location of first point.  Easting and northing in meters.
 * @param b UTM location of second point.  Easting and northing in meters.
 * @return approx. distance in meters between the two points
 *
 * If the points are in the same UTM zone (i.e., similar longitudes) then we
 * approximate the Earth as a cylinder, i.e., we use Euclidean distance with
 * the northing and easting differences.  If the UTM zones differ, then we do
 * likewise but we use a more sophisticated approximation to account for the
 * difference in easting.  In any case, the points have to be fairly close to
 * each other because this does not compute true geodesics.
 */
double dist(const pt a, const pt b)
{
    if (a.zone != b.zone)
    {
        double newEasting = getNewEasting(b, a.zone);
        return sqrt((a.x-newEasting)*(a.x-newEasting) + (a.y-b.y)*(a.y-b.y));
    }
    else
    {
        return sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
    }
}


/// @brief computes the length of a segment
double segmentLength( const seg *s )
{
    double d=0;

    if (!s || !s->points)
    {
        return 0;
    }

    int cur = 0;
    while (cur<s->numPoints && s->points[cur].x<0)
    {
        cur++;
    }
    for (int i=cur+1;i<s->numPoints;i++)
    {
        if (s->points[i].x>0)
        {
            d+=dist(s->points[cur],s->points[i]);
            cur=i;
        }
    }
    //seg* const_evasion_s = (seg*)s; const_evasion_s->length = d;
    s -> length = d;
    return d;
}


/// @brief clear all the fields of a layer (pointers assumed to be dangling)
void initLayer (layer *l)
{
    /* SIMPLIFY LAYER 2012-JAN-22
    l->enabled = true;
    l->modified = false;
    */

    l->tracks = NULL;
    l->numTracks = 0;

    l->waypoints = NULL;
    l->numWaypoints = 0;
}


/// @brief release memory in the layer's tracks and waypoints arrays
void destroyLayer( layer* l )
{
    ASSERT( l );
    for( unsigned iii=l->numTracks; iii--; )
        kjb_c::kjb_free( l->tracks[ iii ].s.points );
    kjb_c::kjb_free( l->tracks );
    kjb_c::kjb_free( l->waypoints );

    // Since we often REUSE these layer structures, we ought to clean it up.
    initLayer( l );
}


/// @brief clear the fields of a track (like a default constructor)
void initTrack (track *t)
{
    t->name[0] = 0;
    t->s.numPoints = 0;
    t->s.points = NULL;
    /* SIMPLIFY TRACK 2011-JAN-17
    t->comment[0] = 0;
    t->desc[0] = 0;
    t->source[0] = 0;
    t->isRoute = false;
    //t->s.elev_valid = false;
    //t->s.time_valid = false;
    //t->s.width = 3;
    //t->s.color = colors[CLR_TRACK_DEFAULT];
    strcpy(t->url,"http://www.topofusion.com");
    strcpy(t->urlname,"TopoFusion Home Page");
    //t->s.highlighted = false;
    */
}

/// @brief clear the point.ele field of a waypoint (and that's all).
void initWaypoint (waypoint *w)
{
    w->point.ele = NO_ELEV;
    /* SIMPLIFY PT 2012 JAN 28
    w->point.time = NO_TIME;
    */
    /* SIMPLIFY WAYPOINTS 2011-JAN-16
    w->comment[0] = 0;
    w->desc = NULL;
    w->url[0] = 0;
    w->urlname[0] = 0;
    w->waytype[0] = 0;
    w->symbol[0] = 0;
    */
}


} // end namespace TopoFusion
} // end namespace kjb
