/**
 * @file
 * @brief interface for low-level NED reader functions
 * @author Andrew Predoehl
 *
 * NED stands for the National Elevation Dataset, which is a family of GIS
 * (Geographical Information Systems) databases of elevation data in the
 * U.S.A., also known as a Digital Elevation Models (DEMs).  There are a few
 * NED databases, with different resolutions.  The one used by FindTrails is
 * NED13, which has one-third arcsecond resolution.  NED13 data is available in
 * grids that are approximately square, with an edge length of just over
 * one degree.  As of this writing, a number of such grids are located in
 * /data_3/trails/ned_13as/original.
 */
/*
 * $Id: nedget.h 17601 2014-09-25 22:40:22Z predoehl $
 */

#ifndef NEDGET_H_UOFARIZONA_VISION
#define NEDGET_H_UOFARIZONA_VISION

#include <m_cpp/m_matrix.h>
#include <string>
#include <deque>


namespace kjb
{

const float NED_MISSING = -9999.0;  ///< sentinel for missing elevation data
const float NED_MIN = -100;         ///< Death Valley is -86 meters elevation
const float NED_MAX = 7000;         ///< Mt. McKinley is 6194 meters elevation


enum NED13_FLOAT_AUTODETECT
{
    NED_AD_MSBFIRST,                ///< byteorder is MSB-first, network order
    NED_AD_LSBFIRST,                ///< byteorder is LSB-first, like Intel
    NED_AD_UNCERTAIN                ///< byteorder is not known
};

// call this if you KNOW the byteorder at compile time
int get_ned_fdeq( const std::string&, std::deque< float >*, bool );

// call this if you don't know the byteorder early, or at all.
int get_ned_fdeq(
    const std::string&,
    std::deque< float >*,
    enum NED13_FLOAT_AUTODETECT byteorder = NED_AD_UNCERTAIN
);

// reshape deque into a square matrix
int ned_fdeq_to_matrix( const std::deque< float >&, Matrix* );

// call this if you know the byteorder at compile time
int get_ned_matrix( const std::string&, Matrix*, bool );

// call this if you don't know the byteorder early, or at all.
int get_ned_matrix(
    const std::string&,
    Matrix*,
    enum NED13_FLOAT_AUTODETECT byteorder = NED_AD_UNCERTAIN
);


int autodetect_ned_byteorder(const std::string&, enum NED13_FLOAT_AUTODETECT*);


}

#endif /* NEDGET_H_UOFARIZONA_VISION */
