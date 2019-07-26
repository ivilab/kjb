/**
 * @file
 * @brief Definitions for lat/long to/from UTM.
 * @author Scott Morris
 *
 *
 * Definitions for converting between latitude-and-longitude coordinates and
 * UTM (Universal Transverse Mercator) coordinates.
 * See copyright info in LatLon-UTMconversion.cpp
 *
 * @section latlon Latitude and Longitude
 * As you probably know, latitude and longitude are (together) a system of
 * specifying locations on the earth when regarded as a sphere.
 * The sphere has one point called the north pole.  The point on the sphere
 * farthest from the north pole is called the south pole.  Points on the sphere
 * equidistant between north and south pole are the equator.
 * Latitude describes north-south position:  it is
 * expressed as an angle from -90 degrees to +90 degrees, and for any point
 * except the poles or on the equator it is the angle
 * formed by three points:  the point of interest, the center of the sphere,
 * and the nearest point on the equator.  Points closer to the north pole than
 * the south pole have a positive angle and can be said to have "north
 * latitude"; "south latitude" latitude and a negative latitude angle
 * correspond to the complementary case.   Naturally the north pole has
 * latitude +90 degrees, the south pole has latitude -90 degrees, and the
 * equator has 0 degrees.  Contours of constant latitude are circles, except at
 * the poles.  When facing the north pole and moving right along such a circle,
 * that direction is known called east, whereas to the left is west.
 * Longitude quantizes location in the east or west direction.  It is expressed
 * as an angle from -180 to +180 degrees.
 * There is a semicircle on the sphere that runs from north pole to south pole
 * through Greenwich, England (when the earth is modeled as a sphere)
 * which is called the prime meridian.  Any point P
 * not on the Prime Meridian and not one of the poles has a longitude defined
 * by the angle of the nearest equatorial point to P, the center of the sphere,
 * and the intersection of the prime meridian and the equator.  Any point in
 * the open hemisphere whose boundary is defined by the prime meridian has a
 * longitude that can be called east longitude or west longitude.  A point
 * that is infinitesimally west of the prime meridian, and all other points on
 * that open hemisphere, have west longitude.  The points of the other open
 * hemisphere have east longitude.  East and west longitude are indicated by
 * the sign (positive or negative) of the angle; east longitudes are positive,
 * west longitudes are negative.
 * Since in fact the earth is not a sphere, in practice it is more useful to
 * redefine these terms for an ellipsoid, but I don't know how.
 * Captain Obvious thanks the reader for her or his attention.
 *
 * @section utm Universal Transverse Mercator coordinates
 * as an alternative to latitude and longitude, which are angles, it is common
 * to use a geolocation coordinate system called UTM.  I don't have the energy
 * now to describe it all, but it is based on the Mercator projection, and
 * consists of three coordinates, called easting, northing, and zone.
 * Similar to @ref latlon, this coordinate system depends on the notions of
 * the equator and east and west.  There are a fixed number of zones, specified
 * by small positive integers, used to describe compact, nonoverlapping
 * regions of similar longitude.  The easting coordinate is a measurement in
 * meters of the distance between the point of interest and the westmost edge
 * of the zone; I am not sure if that is a measurement along a circle of
 * constant latitude, or along a "great circle" (a circle on the sphere through
 * the sphere's center), or Euclidean distance; nor am I sure whether it is a
 * distance between the point of interest and the nearest point on the zone's
 * western boundary, or on the point of the zone's western boundary with the
 * same latitude.  In the continental USA, all of these variations will amount
 * to nearly the same answer, and our imagery has resolution only down to the
 * nearest meter, so it probably does not matter.  Northing is a measurement in
 * meters between the point of interest and the nearest point on the equator.
 * Points north of the equator have positive northing, otherwise not.
 * I could say more but I'm tired of all this prose.
 */
/*
 * $Id: LatLong-UTMconversion.h 17606 2014-09-26 01:09:51Z predoehl $
 */

#ifndef LATLONGCONV_H_INCLUDED_UOFARIZONA_VISION
#define LATLONGCONV_H_INCLUDED_UOFARIZONA_VISION

#include <topo_cpp/layer.h>
#include <vector>

namespace kjb
{
namespace TopoFusion
{

void LLtoUTM(
    int ReferenceEllipsoid,
    const double Lat,
    const double Long,
    pt& utm
);

#if 0
/// @brief convert @ref utm to @ref latlon
void UTMtoLL(
    int ReferenceEllipsoid,
    const pt& utm,
    double& Lat,
    double& Long
);
#endif

void UTMtoGPXLL(
    int ReferenceEllipsoid,
    const pt& utm,
    double& Lat,
    double& Long
);


void utm_to_lat_long(
    int ReferenceEllipsoid,
    const pt& utm,
    double& Lat,
    double& Long
);


void utm_to_lat_long(
    int,
    const std::vector<pt>&,
    std::vector<double>*,
    std::vector<double>* 
);


double getNewEasting(
    const pt& utm,
    int new_zone
);


char zone_of(int ReferenceEllipspoid, const pt&);


#if 0
struct Debug_spy_utm_ll_conversion {
    double eccPrimeSquared, e1, mu, phi1Rad, N1, T1, C1, R1, D, Lat, Long;
};

extern Debug_spy_utm_ll_conversion spy[2];
#endif

/**
 * @brief a TopoFusion data structure used to represent an ellipsoid earth
 *
 * We are talking about an ellipsoid of revolution, also known as a spheroid.
 * That means we model the equator as a circle (not an ellipse).
 */
struct Ellipsoid
{
    int id;                     ///< each ellipsoid model gets a unique number
    const char* ellipsoidName;  ///< each ellipsoid gets a descriptive string
    double EquatorialRadius;    ///< the equator is still a circle
    double eccentricitySquared; ///< square eccentricity of longituinal ellipse
    double oneOverFlattening;   ///< elliptical flattening (related to ecc.)

    /// @brief ctor fills in all datum fields
    Ellipsoid(int Id, const char* name, double radius, double ecc, double flat)
    :   id(Id),
        ellipsoidName(name),
        EquatorialRadius(radius),
        eccentricitySquared(ecc),
        oneOverFlattening(flat)
    {}
};

enum ELLIPSOID_ID {
    ELLIPSOID_Airy = 1,
    ELLIPSOID_Australian_National,
    ELLIPSOID_Bessel_1841,
    ELLIPSOID_Bessel_1841_Nambia,
    ELLIPSOID_Clarke_1866,              ///< standard for UTM coordinates
    ELLIPSOID_Clarke_1880,
    ELLIPSOID_Everest,
    ELLIPSOID_Fischer_1960_Mercury,
    ELLIPSOID_Fischer_1968,
    ELLIPSOID_GRS_1967,
    ELLIPSOID_GRS_1980,                 ///< also fairly common
    ELLIPSOID_Helmert_1906,
    ELLIPSOID_Hough,
    ELLIPSOID_International,
    ELLIPSOID_Krassovsky,
    ELLIPSOID_Modified_Airy,
    ELLIPSOID_Modified_Everest,
    ELLIPSOID_Modified_Fischer_1960,
    ELLIPSOID_South_American_1969,
    ELLIPSOID_WGS_60,
    ELLIPSOID_WGS_66,
    ELLIPSOID_WGS_72,
    ELLIPSOID_WGS_84,                   ///< this is the most popular choice
    ELLIPSOID_END_OF_LIST,

    ELLIPSOID_START_OF_LIST = ELLIPSOID_Airy
};


/**
 * @brief verify that ellipsoid ids correspond to positions in ellipsoid list
 * @returns ERROR or NO_ERROR as appropriate.   Does not throw.
 *
 * You should probably call this early in the run of any important GIS program.
 */
int validate_ellipsoid_table();


#if 0
/// @brief parameters of an abstract geodetic reference system
struct datum
{
    int id;                     ///< each geodesy datum gets a unique number
    const char* dName;          ///< each also gets a descriptive string
    int ellipsoid;              ///< each references an ellipsoid by its id
    int dx, dy, dz;             // offsets relative to WGS-84

    /// @brief ctor fills in all datum fields
    datum( int Id, const char* name, int el, int Dx, int Dy, int Dz )
    :   id( Id ),
        dName( name ),
        ellipsoid( el ),
        dx( Dx ),
        dy( Dy ),
        dz( Dz )
    {}
};
#endif


} // end namespace TopoFusion
} // end namespace kjb


#endif
