/**
 * @file
 * @brief Implementation of the lat/long conversion
 * @author Scott Morris
 *
 * Reference ellipsoids derived from Peter H. Dana's website,
 * http://www.utexas.edu/depts/grg/gcraft/notes/datum/elist.html
 * - Department of Geography, University of Texas at Austin
 * - Internet: pdana@mail.utexas.edu
 * - 3/22/95
 *
 * Source:
 * Defense Mapping Agency. 1987b. DMA Technical Report: Supplement to
 * Department of Defense World Geodetic System 1984 Technical Report.
 * Part I and II. Washington, DC: Defense Mapping Agency
 */
/*
 * $Id: LatLong-UTMconversion.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "l/l_sys_io.h"
#include "l_cpp/l_exception.h"
#include "l_cpp/l_util.h"
#include "topo_cpp/layer.h"
#include "topo_cpp/LatLong-UTMconversion.h"

#include <algorithm>
#include <functional>
#include <valarray>


namespace
{

const double deg2rad = M_PI / 180;      ///< Number of radians in 1 degree
const double rad2deg = 180.0 / M_PI;    ///< Number of degrees in 1 radian

typedef std::valarray<double> VDub;

struct get_easting {
    double operator()(const kjb::TopoFusion::pt& p) const { return p.x; }
};
struct get_northing {
    double operator()(const kjb::TopoFusion::pt& p) const { return p.y; }
};

void get_easting_northing(
    const std::vector<kjb::TopoFusion::pt>& utm,
    VDub* e, 
    VDub* n
)
{
    std::vector<double> ve(utm.size()), vn(utm.size());
    std::transform(utm.begin(), utm.end(), ve.begin(), get_easting());
    std::transform(utm.begin(), utm.end(), vn.begin(), get_northing());
    double &efront = (*e)[0], &nfront = (*n)[0];
    std::copy(ve.begin(), ve.end(), &efront);
    std::copy(vn.begin(), vn.end(), &nfront);
}

struct z_center_to_lon {
    // return longitude of the center of the given zone, in degrees
    double operator()(const kjb::TopoFusion::pt& p) const
    {
        // +3 puts origin in middle of zone
        return (p.zone - 1)*6 - 180 + 3;
    }
};

void zone_center_to_longitude(
    const std::vector<kjb::TopoFusion::pt>& utm,
    std::valarray<double>* lon_o
)
{
    double &lo_front = lon_o->operator[](0);
    std::transform(utm.begin(), utm.end(), &lo_front, z_center_to_lon());
}


inline enum kjb::TopoFusion::ELLIPSOID_ID operator++(
    enum kjb::TopoFusion::ELLIPSOID_ID& e
)
{
    int i = static_cast<int>(e);
    return e = static_cast<enum kjb::TopoFusion::ELLIPSOID_ID>(i+1);
}


}


namespace kjb
{
namespace TopoFusion
{

/**
 * @brief table of famous ellipsoids of rotation for geodesy.
 *
 * First is a placeholder only, To allow array indices to match id numbers
 * The last entry is apparently the best one.
 */
static Ellipsoid ellipsoid[] =
{
    //   id, Ellipsoid name, Equatorial Radius, square of eccentricity

    // Name                 Equatorial rad, eccentricity2,  1/flattening
    //----------------------------------------------------------------------
    Ellipsoid( -1,
    "Placeholder",          0,              0,              0               ),
    Ellipsoid( ELLIPSOID_Airy,
    "Airy",                 6377563,        0.00667054,     299.3249646     ),
    Ellipsoid( ELLIPSOID_Australian_National,
    "Australian National",  6378160,        0.006694542,    298.25          ),
    Ellipsoid( ELLIPSOID_Bessel_1841,
    "Bessel 1841",          6377397,        0.006674372,    299.1528128     ),
    Ellipsoid( ELLIPSOID_Bessel_1841_Nambia,
    "Bessel 1841 (Nambia)", 6377484,        0.006674372,    299.152812      ),
    Ellipsoid( ELLIPSOID_Clarke_1866,
    "Clarke 1866",          6378206.4,      0.006768658,    294.9786982     ),
    Ellipsoid( ELLIPSOID_Clarke_1880,
    "Clarke 1880",          6378249.145,    0.006803511,    293.465         ),
    Ellipsoid( ELLIPSOID_Everest,
    "Everest",              6377276,        0.006637847,    300.8017        ),
    Ellipsoid( ELLIPSOID_Fischer_1960_Mercury,
    "Fischer 1960 (Mercury)",6378166,       0.006693422,    298.3           ),
    Ellipsoid( ELLIPSOID_Fischer_1968,
    "Fischer 1968",         6378150,        0.006693422,    298.3           ),
    Ellipsoid( ELLIPSOID_GRS_1967,
    "GRS 1967",             6378160,        0.006694605,    298.257222101   ),
    Ellipsoid( ELLIPSOID_GRS_1980,
    "GRS 1980",             6378137,        0.00669438,     298.257222101   ),
    Ellipsoid( ELLIPSOID_Helmert_1906,
    "Helmert 1906",         6378200,        0.006693422,    298.3           ),
    Ellipsoid( ELLIPSOID_Hough,
    "Hough",                6378270,        0.00672267,     297             ),
    Ellipsoid( ELLIPSOID_International,
    "International",        6378388,        0.00672267,     297             ),
    Ellipsoid( ELLIPSOID_Krassovsky,
    "Krassovsky",           6378245,        0.006693422,    298.3           ),
    Ellipsoid( ELLIPSOID_Modified_Airy,
    "Modified Airy",        6377340,        0.00667054,     299.3249646     ),
    Ellipsoid( ELLIPSOID_Modified_Everest,
    "Modified Everest",     6377304,        0.006637847,    300.8017        ),
    Ellipsoid( ELLIPSOID_Modified_Fischer_1960,
    "Modified Fischer 1960",6378155,        0.006693422,    298.3           ),
    Ellipsoid( ELLIPSOID_South_American_1969,
    "South American 1969",  6378160,        0.006694542,    298.25          ),
    Ellipsoid( ELLIPSOID_WGS_60,
    "WGS 60",               6378165,        0.006693422,    0               ),
    Ellipsoid( ELLIPSOID_WGS_66,
    "WGS 66",               6378145,        0.006694542,    0               ),
    Ellipsoid( ELLIPSOID_WGS_72,
    "WGS-72",               6378135,        0.006694318,    298.26          ),
    Ellipsoid( ELLIPSOID_WGS_84,
    "WGS-84",               6378137,        0.00669438,     298.257223563   )
};


int validate_ellipsoid_table()
{
    enum ELLIPSOID_ID e;
    for (e = ELLIPSOID_START_OF_LIST; e != ELLIPSOID_END_OF_LIST; ++e)
    {
        if (e > 0 && ellipsoid[e].id != e)
        {
            using namespace kjb_c;
            const Ellipsoid &el = ellipsoid[e];
            add_error("Ellipsoid table is corrupt: entry %d contains\n"
                    "{ %d, \"%s\", %e,"
                    " %e, %e }\n",
                    int(e),
                    el.id, el.ellipsoidName, el.EquatorialRadius,
                    el.eccentricitySquared, el.oneOverFlattening
                );
            return ERROR;
        }
    }
    return kjb_c::NO_ERROR;
}


#if 0
/// @brief table of geodesy data based on ellipsoid and reference displacement
static datum data[] =
{
    //      Id  Name                        Ellip.  dx      dy      dz
    datum(  -1, "PlaceHolder",              0,      0,      0,      0       ),
    datum(  1,  "WGS-84",                   23,     0,      0,      0       ),
    datum(  2,  "WGS-72",                   22,     0,      0,      0       ),
    datum(  3,  "NAD-27 United States",     5,      -8,     160,    176     ),
    datum(  4,  "NAD-27 Mexico",            5,      -12,    130,    190     ),
    datum(  5,  "NAD-27 Canada",            5,      -10,    158,    187     ),
    datum(  6,  "Ordnance Survey 1936",     1,      371,    -112,   434     ),
    datum(  7,  "Australian Geodetic 1966", 2,      -133,   -48,    148     ),
    datum(  8,  "Australian Geodetic 1984", 2,      -134,   -48,    149     ),
    datum(  9,  "Old Hawaiian",             5,      61,     -285,   -181    )
};
#endif



/**
 * @brief convert @ref latlon to @ref utm
 * @author Chuck Gantz- chuck.gantz@globalstar.com
 * @param[in] ReferenceEllipsoid    spheroid used to model the earth's shape
 * @param[in] Lat   Latitude in dec. degrees, north positive and south negative
 * @param[in] Long  Longitude in dec. degrees, east positive and west negative.
 * @param[out] UTMNorthing  UTM northing in meters from the equator.
 * @param[out] UTMEasting   UTM easting in meters from west edge of zone
 * @param[out] UTMZone      UTM zone (band of similar longitudes)
 *
 * converts lat/long to UTM coords. Equations from USGS Bulletin 1532
 * East Longitudes are positive, West longitudes are negative.
 * North latitudes are positive, South latitudes are negative
 * Lat and Long are in decimal degrees
 */
void LLtoUTM(
    int ReferenceEllipsoid,
    const double Lat,
    const double Long,
    pt& utm
)
{
    const Ellipsoid &e = ellipsoid[ReferenceEllipsoid];
    const double    a = e.EquatorialRadius,
                    eccSquared = e.eccentricitySquared,
                    k0 = 0.9996;

    double LongTemp = Long;
    while (LongTemp > 180.0) LongTemp -= 360.0;
    while (LongTemp <= -180) LongTemp += 360.0;

    const double    LatRad = Lat*deg2rad,
                    LongRad = LongTemp*deg2rad;
    int ZoneNumber = int((LongTemp + 180)/6) + 1;

    if (Lat >= 56.0 && Lat < 64.0 && LongTemp >= 3.0 && LongTemp < 12.0)
    {
        ZoneNumber = 32;
    }

    // Special zones for Svalbard /// do we need this????????
    if (Lat >= 72.0 && Lat < 84.0)
    {
        if(      LongTemp >= 0.0  && LongTemp <  9.0 ) ZoneNumber = 31;
        else if( LongTemp >= 9.0  && LongTemp < 21.0 ) ZoneNumber = 33;
        else if( LongTemp >= 21.0 && LongTemp < 33.0 ) ZoneNumber = 35;
        else if( LongTemp >= 33.0 && LongTemp < 42.0 ) ZoneNumber = 37;
    }

    // 0.5 puts origin in middle of zone
    const double    LongOrigin = (ZoneNumber - 0.5)*6 - 180,
                    LongOriginRad = LongOrigin * deg2rad;

    //compute the UTM Zone from the latitude and longitude
    //  sprintf(UTMZone, "%d%c", ZoneNumber, UTMLetterDesignator(Lat));
    utm.zone = static_cast< char >( ZoneNumber );

    const double    eccPrimeSquared = (eccSquared)/(1-eccSquared),
                    N = a/sqrt(1-eccSquared*sin(LatRad)*sin(LatRad)),
                    T = tan(LatRad)*tan(LatRad),
                    C = eccPrimeSquared*cos(LatRad)*cos(LatRad),
                    A = cos(LatRad)*(LongRad-LongOriginRad),
                    M = a*(  (1 - eccSquared/4 - 3*eccSquared*eccSquared/64
                              - 5*eccSquared*eccSquared*eccSquared/256)*LatRad
                           - (3*eccSquared/8 + 3*eccSquared*eccSquared/32
                               + 45*eccSquared*eccSquared*eccSquared/1024)
                             *sin(2*LatRad)
                           + (15*eccSquared*eccSquared/256
                               + 45*eccSquared*eccSquared*eccSquared/1024)
                             *sin(4*LatRad)
                           - (35*eccSquared*eccSquared*eccSquared/3072)
                             *sin(6*LatRad)
                          );

    // UTM Easting
    utm.x = 500000.0+k0*N*(A+(1-T+C)*A*A*A/6
                          +(5-18*T+T*T+72*C-58*eccPrimeSquared)*A*A*A*A*A/120);

    // UTM Northing
    utm.y = k0*(M+N*tan(LatRad)*(A*A/2+(5-T+9*C+4*C*C)*A*A*A*A/24
                 + (61-58*T+T*T+600*C-330*eccPrimeSquared)*A*A*A*A*A*A/720));
#if 0
    if(Lat < 0)
    {
        UTMNorthing += 10000000.0; // 10,000 km offset for southern hemisphere
    }
#endif
}


#if 0
Debug_spy_utm_ll_conversion spy[2] =
{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};
#endif

#if 0
void UTMtoLL(
    int ReferenceEllipsoid,
    const pt& utm,
    double& Lat,
    double& Long
)
{
//converts UTM coords to lat/long.  Equations from USGS Bulletin 1532
//East Longitudes are positive, West longitudes are negative.
//North latitudes are positive, South latitudes are negative
//Lat and Long are in decimal degrees.
    //Written by Chuck Gantz- chuck.gantz@globalstar.com

    double k0 = 0.9996;
    double a = ellipsoid[ReferenceEllipsoid].EquatorialRadius;
    double eccSquared = ellipsoid[ReferenceEllipsoid].eccentricitySquared;
    double eccPrimeSquared;
    double e1 = (1-sqrt(1-eccSquared))/(1+sqrt(1-eccSquared));
    double N1, T1, C1, R1, D, M;
    double LongOrigin;
    double mu, phi1, phi1Rad;
    double x, y;

    x = utm.x - 500000.0; //remove 500,000 meter offset for longitude
    y = utm.y;

    LongOrigin = (utm.zone - 1)*6 - 180 + 3;  //+3 puts origin in middle of zone

    eccPrimeSquared = (eccSquared)/(1-eccSquared);

    M = y / k0;
    mu = M/(a*(1-eccSquared/4-3*eccSquared*eccSquared/64-5*eccSquared*eccSquared*eccSquared/256));

    phi1Rad = mu    + (3*e1/2-27*e1*e1*e1/32)*sin(2*mu)
                + (21*e1*e1/16-55*e1*e1*e1*e1/32)*sin(4*mu)
                +(151*e1*e1*e1/96)*sin(6*mu);
    phi1 = phi1Rad*rad2deg;

    N1 = a/sqrt(1-eccSquared*sin(phi1Rad)*sin(phi1Rad));
    T1 = tan(phi1Rad)*tan(phi1Rad);
    C1 = eccPrimeSquared*cos(phi1Rad)*cos(phi1Rad);
    R1 = a*(1-eccSquared)/pow(1-eccSquared*sin(phi1Rad)*sin(phi1Rad), 1.5);
    D = x/(N1*k0);

    Lat = phi1Rad - (N1*tan(phi1Rad)/R1)*(D*D/2-(5+3*T1+10*C1-4*C1*C1-9*eccPrimeSquared)*D*D*D*D/24
                    +(61+90*T1+298*C1+45*T1*T1-252*eccPrimeSquared-3*C1*C1)*D*D*D*D*D*D/720);
    Lat = Lat * rad2deg;

    Long = (D-(1+2*T1+C1)*D*D*D/6+(5-2*C1+28*T1-3*C1*C1+8*eccPrimeSquared+24*T1*T1)
                    *D*D*D*D*D/120)/cos(phi1Rad);
    Long = LongOrigin + Long * rad2deg;

    //  if (Long > 0) Long = -((180 - Long) + 180);

#if 0
    spy[0].eccPrimeSquared = eccPrimeSquared;
    spy[0].e1 = e1;
    spy[0].mu = mu;
    spy[0].phi1Rad = phi1Rad;
    spy[0].N1 = N1;
    spy[0].T1 = T1;
    spy[0].C1 = C1;
    spy[0].R1 = R1;
    spy[0].D = D;
    spy[0].Lat = Lat;
    spy[0].Long = Long;
#endif
}
#endif


/**
 * @brief convert @ref utm to @ref latlon
 *
 * @param ReferenceEllipsoid    index of the ellipsoid used to model Earth
 *
 * @param[i] utm    @ref utm coordinates of a point
 * @param[o] Lat    Latitude in decimal degrees (see @ref latlon)
 * @param[o] Long   Longitude in decimal degrees (see @ref latlon)
 *
 * @author Chuck Gantz- chuck.gantz@globalstar.com
 * @author Scott Morris
 * @author Andrew Predoehl
 *
 * Equations from USGS Bulletin 1532
 * East Longitudes are positive, West longitudes are negative.
 * North latitudes are positive, South latitudes are negative
 * Lat and Long are in decimal degrees.
 */
void utm_to_lat_long(
    int ReferenceEllipsoid,
    const pt& utm,
    double& Lat,
    double& Long
)
{
    const Ellipsoid &ref_e = ellipsoid[ReferenceEllipsoid];

    const double    k0 = 0.9996,
                    a = ref_e.EquatorialRadius,

                    eccSquared = ref_e.eccentricitySquared,
                    um_ecc2 = 1 - eccSquared,
                    eccPrimeSquared = eccSquared / um_ecc2,
                    squm_ecc2 = sqrt(um_ecc2),
                    e1 = (1 - squm_ecc2) / (1 + squm_ecc2),
                    e1e1 = e1 * e1,

                    mu_den = 1 - eccSquared/4
                                    *(1 + eccSquared/16
                                        *(3 + 5*eccSquared/4)),
                    mu = utm.y / k0 / a / mu_den,

                    phi1Rad = mu + e1/2*( 3*(1-9*e1e1/16)       * sin(2*mu)
                                         +e1/8*( (21-55*e1e1/2) * sin(4*mu)
                                                +151*e1/6       * sin(6*mu))),
                    sp1 = sin(phi1Rad),
                    cp1 = cos(phi1Rad),
                    tp1 = sp1 / cp1,

                    um_e2spsp = 1 - eccSquared * sp1 * sp1,
                    N1 = a / sqrt(um_e2spsp),
                    T1 = tp1 * tp1,
                    C1 = eccPrimeSquared * cp1 * cp1,
                    R1 = um_ecc2 * N1 / um_e2spsp,

                    // remove 500,000 meter offset for longitude
                    D = (utm.x - 500000.0) / N1 / k0,
                    DD = D * D;

    Lat =  phi1Rad
         - N1*tp1/R1
            * DD/2*(1 - DD/12*( 5 + 3*T1 + (5-2*C1)*2*C1 - 9*eccPrimeSquared
                               +DD/30*( 61 + (2+T1)*45*T1 - 252*eccPrimeSquared
                                       +(99-C1)*3*C1)));
    Lat *= rad2deg;

#if 0
    // +3 puts origin in middle of zone
    Long = (utm.zone - 1)*6 - 180 + 3;

    Long +=   rad2deg
#else
    Long = (utm.zone - 0.5)*6 - 180
          +   rad2deg
#endif
            * D/cp1
            * (1 - DD/6*(  1 + 2*T1 + C1
                         - DD/20*( 5 -(2 + 3*C1)*C1 + 8*eccPrimeSquared
                                  + (7 + 6*T1)*4*T1)));

    ASSERT(-180 < Long && Long <= 180);

#if 0
    // code used to validate the cleanup seen here.
    spy[1].eccPrimeSquared = eccPrimeSquared;
    spy[1].e1 = e1;
    spy[1].mu = mu;
    spy[1].phi1Rad = phi1Rad;
    spy[1].N1 = N1;
    spy[1].T1 = T1;
    spy[1].C1 = C1;
    spy[1].R1 = R1;
    spy[1].D = D;
    spy[1].Lat = Lat;
    spy[1].Long = Long;
#endif
}


/**
 * @brief this is a fast, vectorized (bulk) version of the single-pt function.
 *
 * @param ReferenceEllipsoid    index of the ellipsoid used to model Earth
 *
 * @param[i] utm   array of @ref utm coordinates of points to convert
 * @param[o] Lat   array storing converted decimal latitudes (see @ref latlon)
 * @param[o] Long  array storing converted decimal longitudes (see @ref latlon)
 *
 * @author Chuck Gantz
 * @author Scott Morris
 * @author Andrew Predoehl
 *
 * Equations from USGS Bulletin 1532.
 * East Longitudes are positive, West longitudes are negative.
 * North latitudes are positive, South latitudes are negative
 * Lat and Long are in decimal degrees.
 * This code was vectorized by hand by Andrew Predoehl based on the
 * single-input version.  Empirical tests suggest it produces indistinguishable
 * results, and it is much faster (by a factor of over 100, for big inputs).
 */
void utm_to_lat_long(
    int ReferenceEllipsoid,
    const std::vector<pt>& utm_in,
    std::vector<double>* latitude_out,
    std::vector<double>* longitude_out
)
{
    NTX(latitude_out);
    NTX(longitude_out);

    const Ellipsoid &ref_e = ellipsoid[ReferenceEllipsoid];

    const double    k0 = 0.9996,
                    a = ref_e.EquatorialRadius,

                    eccSquared = ref_e.eccentricitySquared,
                    um_ecc2 = 1 - eccSquared,
                    eccPrimeSquared = eccSquared / um_ecc2,
                    squm_ecc2 = sqrt(um_ecc2),
                    e1 = (1 - squm_ecc2) / (1 + squm_ecc2),
                    e1e1 = e1 * e1,

                    mu_den = 1 - eccSquared/4
                                    *(1 + eccSquared/16
                                        *(3 + 5*eccSquared/4));

    // convert UTM easting, northing, to valarray format.
    // zone is converted directly to longitude of center of that zone.
    const size_t N = utm_in.size();
    VDub east(N), north(N), Long(N);
    get_easting_northing(utm_in, &east, &north);
    zone_center_to_longitude(utm_in, &Long);

    VDub    mu = north / k0 / a / mu_den,
            double_mu = mu * 2.0,
            s2mu = sin(double_mu),
            s4mu = sin(double_mu * 2.0),
            s6mu = sin(double_mu * 3.0),
            phi1Rad = mu + e1/2*( 3*(1-9*e1e1/16)       * s2mu
                                 +e1/8*( (21-55*e1e1/2) * s4mu
                                        +151*e1/6       * s6mu)),
            sp1 = sin(phi1Rad),
            cp1 = cos(phi1Rad),
            tp1 = sp1 / cp1,    // tangent
            um_e2spsp = 1 - eccSquared * sp1 * sp1,
            N1 = a / sqrt(um_e2spsp),
            T1 = tp1 * tp1,
            C1 = eccPrimeSquared * cp1 * cp1,
            R1 = um_ecc2 * N1 / um_e2spsp,

            // remove 500,000 meter offset for longitude
            D = (east - 500000.0) / N1 / k0,

            DD_1 = D * D,
            DD_2 = DD_1 / 2.0,
            DD_6 = DD_2 / 3.0,
            DD_12 = DD_6 / 2.0,
            DD_20 = DD_2 / 10.0,
            DD_30 = DD_6 / 5.0,

            T1x2 = T1 * 2.0,
            T1x3 = T1 * 3.0,
            T1x4 = T1x2 * 2.0,
            T1x6 = T1x2 * 3.0,
            T1x45 = T1x3 * 15.0,
            C1x2 = C1 * 2.0,
            C1x3 = C1 * 3.0,

            Lat =  phi1Rad
                 - N1*tp1/R1*DD_2*(  1.0
                                   - DD_12*(  5.0
                                            - 9*eccPrimeSquared
                                            + (5.0 - C1x2) * C1x2
                                            + T1x3
                                            + DD_30*(  61.0
                                                     - 252*eccPrimeSquared
                                                     + (99.0 - C1)*C1x3
                                                     + (2.0 + T1)*T1x45
                                                    )
                                           )
                                  );

    Lat *= rad2deg;

    Long += rad2deg * D/cp1 * (  1.0
                               - DD_6*(  1.0
                                       + C1
                                       + T1x2
                                       - DD_20*(  5.0
                                                + 8*eccPrimeSquared
                                                - (2.0 + C1x3)*C1
                                                + (7.0 + T1x6)*T1x4
                                               )
                                      )
                              );

#if 0
    ASSERT(-180 < Long.min() && Long.max() <= 180);
#else
    if (Long.min() <= -180)
    {
        KJB_THROW_2(Runtime_error, "Invalid Longitude (too negative)");
    }
    if (Long.max() > 180)
    {
        KJB_THROW_2(Runtime_error, "Invalid Longitude (too positive)");
    }
#endif

    // transfer results to the output containers
    latitude_out -> resize(N);
    longitude_out -> resize(N);
    std::copy(&Lat[0],  &Lat[0] + N,  latitude_out -> begin());
    std::copy(&Long[0], &Long[0] + N, longitude_out -> begin());
}


/**
 * @brief converts @ref utm coords to @ref latlon.
 * @param ReferenceEllipsoid    index into array ellipsoid[] in constants.h
 * @param UTMNorthing northing coordinate in meters
 * @param UTMEasting easting coordinate in meters
 * @param UTMZone zone number
 * @param Lat latitude in decimal degrees.
 * @param Long longitude in decimal degrees.
 * @author Chuck Gantz- chuck.gantz@globalstar.com
 *
 * Equations from USGS Bulletin 1532
 * East Longitudes are positive, West longitudes are negative.
 * North latitudes are positive, South latitudes are negative.
 * Lat and Long are in decimal degrees.
 */
void UTMtoGPXLL(
    int ReferenceEllipsoid,
    const pt& utm,
    double& Lat,
    double& Long
)
{

    double k0 = 0.9996;
    double a = ellipsoid[ReferenceEllipsoid].EquatorialRadius;
    double eccSquared = ellipsoid[ReferenceEllipsoid].eccentricitySquared;
    double eccPrimeSquared;
    double e1 = (1-sqrt(1-eccSquared))/(1+sqrt(1-eccSquared));
    double N1, T1, C1, R1, D, M;
    double LongOrigin;
    double mu, phi1, phi1Rad;
    double x, y;

    x = utm.x - 500000.0; //remove 500,000 meter offset for longitude
    y = utm.y;

    LongOrigin = (utm.zone - 1)*6 - 180 + 3;  //+3 puts origin in middle of zone

    eccPrimeSquared = (eccSquared)/(1-eccSquared);

    M = y / k0;
    mu = M/(a*(1-eccSquared/4-3*eccSquared*eccSquared/64-5*eccSquared*eccSquared*eccSquared/256));

    phi1Rad = mu    + (3*e1/2-27*e1*e1*e1/32)*sin(2*mu)
                + (21*e1*e1/16-55*e1*e1*e1*e1/32)*sin(4*mu)
                +(151*e1*e1*e1/96)*sin(6*mu);
    phi1 = phi1Rad*rad2deg;

    N1 = a/sqrt(1-eccSquared*sin(phi1Rad)*sin(phi1Rad));
    T1 = tan(phi1Rad)*tan(phi1Rad);
    C1 = eccPrimeSquared*cos(phi1Rad)*cos(phi1Rad);
    R1 = a*(1-eccSquared)/pow(1-eccSquared*sin(phi1Rad)*sin(phi1Rad), 1.5);
    D = x/(N1*k0);

    Lat = phi1Rad - (N1*tan(phi1Rad)/R1)*(D*D/2-(5+3*T1+10*C1-4*C1*C1-9*eccPrimeSquared)*D*D*D*D/24
                    +(61+90*T1+298*C1+45*T1*T1-252*eccPrimeSquared-3*C1*C1)*D*D*D*D*D*D/720);
    Lat = Lat * rad2deg;

    Long = (D-(1+2*T1+C1)*D*D*D/6+(5-2*C1+28*T1-3*C1*C1+8*eccPrimeSquared+24*T1*T1)
                    *D*D*D*D*D/120)/cos(phi1Rad);
    Long = LongOrigin + Long * rad2deg;

    if (Long < -180) Long = 180 + (Long+180);
}


/// @brief return an easting coordinate (from zone) relative to new_zone
double getNewEasting(
    const pt& utm,
    int new_zone
)
{

    if (utm.zone == new_zone) return utm.x;

    // convert to Lat/Lon first
    double lat,lon;
    utm_to_lat_long(23 /*WGS84*/, utm, lat,lon);

    int ReferenceEllipsoid = 23; // WGS84

    double newEasting;


    double a = ellipsoid[ReferenceEllipsoid].EquatorialRadius;
    double eccSquared = ellipsoid[ReferenceEllipsoid].eccentricitySquared;
    double k0 = 0.9996;

    double LongOrigin;
    double eccPrimeSquared;
    double N, T, C, A, M;

    //Make sure the longitude is between -180.00 .. 179.9
    //double LongTemp = (lon+180)-int((lon+180)/360)*360-180; // -180.00 .. 179.9;
    double LongTemp = lon;  // don't need to do the above check since we use UTMtoLL

    double LatRad = lat*deg2rad;
    double LongRad = LongTemp*deg2rad;
    double LongOriginRad;

    LongOrigin = (new_zone - 1)*6 - 180 + 3;  //+3 puts origin in middle of zone
    LongOriginRad = LongOrigin * deg2rad;

    //if (abs(LongOrigin - lon) > 90) return -1;


    eccPrimeSquared = (eccSquared)/(1-eccSquared);

    N = a/sqrt(1-eccSquared*sin(LatRad)*sin(LatRad));
    T = tan(LatRad)*tan(LatRad);
    C = eccPrimeSquared*cos(LatRad)*cos(LatRad);
    A = cos(LatRad)*(LongRad-LongOriginRad);

    M = a*((1 - eccSquared/4    - 3*eccSquared*eccSquared/64    - 5*eccSquared*eccSquared*eccSquared/256)*LatRad
              - (3*eccSquared/8 + 3*eccSquared*eccSquared/32    + 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*LatRad)
                                + (15*eccSquared*eccSquared/256 + 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*LatRad)
                                - (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*LatRad));

    newEasting = (double)(k0*N*(A+(1-T+C)*A*A*A/6
                    + (5-18*T+T*T+72*C-58*eccPrimeSquared)*A*A*A*A*A/120)
                    + 500000.0);

    return newEasting;
}


/**
 * @brief return the proper zone number for the given point
 * @param ReferenceEllipsoid index of r.e. to use for utm-to-lat/long conv.
 * @param p query point, possibly with an "improper" zone number due to overlap
 *
 * This does NOT just fetch the zone field of the input point!  The input point
 * is allowed to have the "wrong" zone, and this will try to return the proper
 * zone.
 *
 * Zones cover six degrees of longitude, aligned to integer six-degree
 * multiples of meridian number.  UTM supports overlapping zones (i.e., you can
 * modestly stray by a handful of kilometers across the 6-degree bands, yet
 * retain pretty good accuracy) but the system works best when your UTM
 * coordinates are in the proper zone.
 * This function will return the proper zone for the given input point.
 * Accuracy has not been quantified, but it would be best if the input point
 * does not ask the zones to overlap more than about 40 km (according to
 * Wikipedia).
 *
 * Example:  if query point p is near the east end of Lake Mead on the Colorado
 *           River, barely in zone 11, you can get fairly accurate coordinates
 *           for upriver points by adding to the "easting" of p and retaining
 *           zone 11.  But you'll get better accuracy if you switch to zone 12
 *           (the proper zone for most upriver points in Arizona) and use a new
 *           easting value appropriate to zone 12.
 *
 * @see getNewEasting()
 */
char zone_of(int ReferenceEllipsoid, const pt& p)
{
    double lat, lon;
    utm_to_lat_long(ReferenceEllipsoid, p, lat, lon);
    return static_cast<char>( 31 + floor(lon / 6.0) );
}


} // end namespace TopoFusion
} // end namespace kjb
