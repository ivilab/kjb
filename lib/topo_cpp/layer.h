/**
 * @file
 * @brief Contains definitions of basic TopoFusion data structures
 * @author Scott Morris
 * @author Alan Morris
 *
 * Definitions of basic data structures used throughout TopoFusion.
 * including "pt" - basic point, "seg" - basic segment (polyline)
 * "layer" - representation of a GPX file (collection of tracks/waypoints).
 *
 * Originally from TopoFusion code.
 */
/*
 * $Id: layer.h 17606 2014-09-26 01:09:51Z predoehl $
 */

#ifndef LAYER_H_INCLUDED_UOFARIZONAVISION
#define LAYER_H_INCLUDED_UOFARIZONAVISION

namespace kjb
{

/// @brief this namespace hold structures and code written by Scott Morris.
namespace TopoFusion
{

const size_t MAX_TRACKSTRING = 255; ///< max len of string field in track, etc
const int NO_ELEV = -1;             ///< sentinel value for "blank" pt::ele

const size_t LAYER_FN_SIZE = 2048;  ///< max length of a layer's filename field


/// @brief definition for a TopoFusion pt
typedef struct
{
    double x;       ///< easting of a location, meters
    double y;       ///< northing of a location, meters
    char zone;      ///< utm zone number of a location
    double ele;     ///< elevation of a location, meters
} pt;               ///< TopoFusion data structure for a UTM point

/// @brief construct a point from its parameters but not as a ctor.
inline
pt make_pt( double eee, double nnn, char zzz, double elev = NO_ELEV )
{
    pt p;
    p.x = eee;
    p.y = nnn;
    p.zone = zzz;
    p.ele = elev;
    return p;
}

/// @brief old TopoFusion data structure for a sequence of pt structures
typedef struct
{
    int numPoints;          ///< number of pt objects in the points array
    pt *points;             ///< array of pt objects

    mutable double length;  ///< polygonal path length of the seg
} seg;                      ///< TopoFusion data struc used for arrays of pt


/// @brief old TopoFusion data structure:  metadata for a seg
typedef struct
{
    char name[MAX_TRACKSTRING];     ///< name field of this track
    seg s;                          ///< sequence of points in the track
} track;                            ///< TopoFusion data str. for a named seg


/// @brief definition used for waypoint
typedef struct
{
    char name[MAX_TRACKSTRING];     ///< name field of the pt
    pt point;                       ///< UTM location of this waypoint
} waypoint;                         ///< TopoFusion data str = point + metadata



/// @brief definition used for layer
typedef struct
{
    int numWaypoints;       ///< number of waypoints in this layer
    waypoint *waypoints;    ///< array of waypoints for the layer

    char filename[ LAYER_FN_SIZE ]; ///< each layer is associated with a file

    int numTracks;          ///< number of tracks for the layer
    track *tracks;          ///< array of tracks for the layer
} layer; ///< TopoFusion data structure simplified:  some tracks and waypoints


double dist(const pt a, const pt b);

double segmentLength( const seg *s );

void initLayer (layer *l);

void destroyLayer( layer *l );

void initTrack (track *t);

void initWaypoint (waypoint *w);

} // end namespace TopoFusion
} // end namespace kjb

#endif  /* LAYER_H_INCLUDED_UOFARIZONAVISION */

