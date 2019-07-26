/**
 * @file
 * @brief Code for an alternate Pixel wrapper supporting Hue, Saturation, Value
 * @author Andrew Predoehl
 */
/*
 * $Id: i_hsv.cpp 21612 2017-08-01 22:07:20Z jguan1 $
 */

#include "l/l_sys_std.h"
#include "l_cpp/l_exception.h"
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"
#include "i_cpp/i_hsv.h"

#include <cmath>

namespace {

inline
float dominant_mix(float low_h, float high_h, float tt )
{
    return high_h * tt + low_h * ( 1.0f - tt );
}


kjb::Vector rgb_from_hsy( const kjb::Vector& hsy )
{
    // this contains row-major inverse of the implicit matrix in the 
    // function HSLuma_space().
    kjb::Matrix::Value_type inv[ 9 ] = {
        +.7,        -.27713,    1,
        -.3,        +.30022,    1,
        -.3,        -.85448,    1
    };
    kjb::Matrix HVHL( 3, 3, inv );

    return HVHL * hsy;
}


}

namespace kjb {

PixelHSVA::PixelHSVA( float hh, float ss, float vv, float aa )
{
    const float FULL_ON = 255.0f;

    r = g = b = 0;
    extra.alpha = std::min( FULL_ON, FULL_ON * aa );

    /*
     * Hue hh can be a wild value, because black has an undefined hue,
     * just as the north pole has an undefined longitude.
     */
    if (    std::isnand( double( hh ) )
        ||  std::isnand(double(hh - hh) ) // !finite( double( hh ) )
        ||  hh < 0.0f
        ||  1.0f < hh )
    {
        return;
    }

    if ( ss < 0.0f || 1.0f < ss || vv < 0.0f || 1.0f < vv )
    {
        KJB_THROW( Illegal_argument );
    }

    /*
     * The terms tonic, dominant, and subdominant are borrowed from music 
     * theory.  In a triad,
     *
     * tonic = loudest (like brightest color channel),
     * subdominant = softest (like dimmest color channel),
     * dominant = between the two.
     */
    float   hh6 = 6.0f * hh,
            tonic = FULL_ON * vv,               /* brightest component */
            subdominant = tonic * (1.0f - ss);  /* dimmest component */

    switch( std::min( 5, static_cast< int >( std::floor( hh6 ) ) ) )
    {
        case 0: /* red to yellow */
            r = tonic;
            g = dominant_mix( subdominant, tonic, hh6 ); // green grows
            b = subdominant;
            break;
        /* NOTE:  you can tell what case 1 must be from case 0 like so:
           In case 0, green increased from subdominant "loser" to tonic
           "winner," so in case 1 and 2 it gets to relax and coast along as
           tonic.  That means the case-0 tonic, red, has to retreat, and
           fades to subdominant in case 1.  Meanwhile, blue is the doormat
           and stays mired in obscurity.  But it will rise in the future
           (case 2).  Using similar reasoning, you can figure out all the
           cases:
           A) subdominant-to-tonic alternates with tonic-to-subdominant;
           B) whichever component rises or falls in case i stays in that
              state for cases (i+1) % 6 and (i+2) % 6.
         */
        case 1: /* yellow to green */
            g = tonic;
            r = dominant_mix( tonic, subdominant, hh6 -= 1.0f ); //red fades
            b = subdominant;
            break;
        case 2: /* green to cyan */
            g = tonic;
            b = dominant_mix( subdominant, tonic, hh6 -= 2.0f ); //blue grows
            r = subdominant;
            break;
        case 3: /* cyan to blue */
            b = tonic;
            g = dominant_mix( tonic, subdominant, hh6 -= 3.0f );//green fades
            r = subdominant;
            break;
        case 4: /* blue to magenta */
            b = tonic;
            r = dominant_mix( subdominant, tonic, hh6 -= 4.0f ); //red grows
            g = subdominant;
            break;
        case 5: /* magenta to red */
            r = tonic;
            b = dominant_mix( tonic, subdominant, hh6 -= 5.0f ); //blue fades
            g = subdominant;
            break;
        default:
            KJB_THROW( Result_error );
            break;
    }
}


void PixelHSVA::get_hsv( float* ph , float* ps, float* pv ) const
{
    float   r01 = r / 255.0f,
            g01 = g / 255.0f,
            b01 = b / 255.0f,
            maxrgb = std::max( std::max( r01, g01 ), b01 ),
            minrgb = std::min( std::min( r01, g01 ), b01 ),
            chroma6 = 6.0f * ( maxrgb - minrgb ),
            value = maxrgb,
            sat = 1.0f - minrgb / maxrgb,
            hue;

    if ( 0 == chroma6 )
    {
        hue = 0; // NaN would also be an acceptable setting
    }
    else if ( r01 >= g01 && r01 >= b01 )
    {   // red is tonic (or tied with another)
        hue = ( g01 - b01 ) / chroma6;
        if ( hue < 0 )
            hue += 1.0f;
    }
    else if ( g01 > r01 && g01 >= b01 )
    {   // green is tonic (or tied with another)
        hue = 1.0f / 3.0f + ( b01 - r01 ) / chroma6;
    } 
    else
    {   // blue is tonic
        hue = 2.0f / 3.0f + ( r01 - g01 ) / chroma6;
    }

    if ( ph ) *ph = hue;
    if ( ps ) *ps = sat;
    if ( pv ) *pv = value;
}




Vector hsluma_space( const kjb_c::Pixel& p )
{
    kjb::Vector v( 3 );
    /* Compute components of hue and saturation in "hexagon space."
     * p = pure red     means   v(0)=+1,    v(1)=0
     * p = pure green   means   v(0)=-0.5,  v(1)=+0.866
     * p = pure blue    means   v(0)=-0.5,  v(1)=-0.866
     * p = pure yellow  means   v(0)=+0.5,  v(1)=+0.866
     */
    v[ 0 ] = ( p.r + p.r - p.g - p.b ) / 2.0f / 255.0f;
    v[ 1 ] = ( p.g - p.b ) * 0.866025404f / 255.0f; // a.k.a. sin 60
    
    /*
     * We deviate from the usual HSV definition so we can use luma for value.
     * This uses the NTSC 601 definition of luma.
     *
     * The point is, typical green is perceived by humans as brighter than the
     * other two colors; typical blue is perceived darker than the other two.
     * This is a sloppy approximation but not as bad as assuming the primaries
     * are all equal valued.
     *
     * This takes an elegant bicone and squashes it into a bizarre shape.
     */
    v[ 2 ] = ( 0.30 * p.r + 0.59 * p.g + 0.11 * p.b ) / 255.0f;
    return v;
}


kjb_c::Pixel get_pixel_from_hsluma_space( const Vector& vhsy )
{
    kjb::Vector rgb = rgb_from_hsy( vhsy );

    /* clampity clamp clamp clamp!! */
    return PixelRGBA(
        255.0f * std::max( std::min( static_cast< float >(rgb[0]), 1.0f),0.0f),
        255.0f * std::max( std::min( static_cast< float >(rgb[1]), 1.0f),0.0f),
        255.0f * std::max( std::min( static_cast< float >(rgb[2]), 1.0f),0.0f)
        );
}


int get_pixel_from_hsluma_space( const Vector& vhsy, kjb_c::Pixel* p )
{
    kjb::Vector rgb = rgb_from_hsy( vhsy );

    for( int iii = 0; iii < 3; ++iii )
    {
        if ( rgb[ iii ] < 0 || 1 < rgb[ iii ] )
        {
            return EXIT_FAILURE;
        }
    }

    if ( p )
    {
        p -> r = 255.0f * static_cast< float >( rgb[ 0 ] );
        p -> g = 255.0f * static_cast< float >( rgb[ 1 ] );
        p -> b = 255.0f * static_cast< float >( rgb[ 2 ] );
    }

    return EXIT_SUCCESS;
}



} //namespace kjb
