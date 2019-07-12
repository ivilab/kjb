/**
 * @file
 * @brief Code for a wrapper class around the C struct Pixel
 * @author Andrew Predoehl
 */
/*
 * $Id: i_pixel.h 17393 2014-08-23 20:19:14Z predoehl $
 */

#ifndef KJB_CPP_PIXEL_H
#define KJB_CPP_PIXEL_H

#include <l/l_sys_std.h>
#include <l/l_debug.h>
#include <l_cpp/l_util.h>
#include <i/i_float.h>

#include <algorithm> /* for min and max */
#include <sstream>
#include <iomanip>

namespace kjb 
{

/**
 * @addtogroup kjbImageProc
 * @{
 */

std::string pixel_as_hex_triplet_string( const kjb_c::Pixel* );

/**
 * @brief Wrapped version of the C struct Pixel, with Alpha (opacity).
 *
 * This is a struct since there is (currently) no need to make such a simple
 * item have anything private.  The advantage of wrapping this at all is that
 * there are commonly-performed operations that are naturally expressed as
 * methods.  Namely:
 * - clamping
 * - adding two pixels
 * - scaling a pixel
 *
 * Example:  given pixels p, q this yields a Pixel:  0.5*(p.clamp()+q)
 *
 * This struct assumes the user is not interested in the "Invalid Pixel" 
 * structure, and is perhaps (but not necessarily) using alpha (opacity).
 * That's why the name has the RGBA suffix.
 *
 * Remember that if you add pixels or scale pixels, the alpha channels also
 * add and scale.  So for the common case (opaque pixels, and 8-bit channels),
 * the alpha channel is already at 255.0, so you might need to clamp() or
 * to ow_clamp() after an add or scale operation, otherwise alpha could go
 * overrange.
 *
 * [CODE POLICE] TODO: Rename this class to follow kjb standard for type names (No camelcase Only first letter may be capitalized): Pixel_rgba
 */
struct PixelRGBA : public kjb_c::Pixel
{
    /**
     * @brief Default ctor, which leaves all fields uninitialized.
     * @warning "Uninitialized" often means "bug-causing" and "troublesome."
     */
    PixelRGBA()
        : kjb_c::Pixel()
    {
        // hit KJB(UNTESTED_CODE());
    }

    /**
     * @brief This builds a valid pixel from another.
     *
     * Build a pixel from another. It receives a kjb_c::Pixel so as to
     * work with PixelRGBA as well.
     */
    PixelRGBA(const kjb_c::Pixel& p)
    {
        // hit KJB(UNTESTED_CODE());
        r = p.r;
        g = p.g;
        b = p.b;
        extra.alpha = p.extra.alpha;
    }

    /**
     * @brief This builds a valid pixel with given values for color channels.
     *
     * Note that despite our massive dynamic range, the default value of aa
     * (the alpha channel) is a mere 255.0f.  This is the most common use-case,
     * that of 8 bits of alpha, maximum opacity.  However, it is ugly.
     * For high dynamic range images, this is probably not your preferred
     * value for the default opacity.
     */
    PixelRGBA( float rr, float gg, float bb, float aa = 255.0f )
    {
        // hit KJB(UNTESTED_CODE());
        r = rr;
        g = gg;
        b = bb;
        extra.alpha = aa;
    }

    /** @brief A "named constructor" to build a single opaque grayscale pixel.
     */
    static PixelRGBA create_gray( float f )
    {
        // hit KJB(UNTESTED_CODE());
        return PixelRGBA( f, f, f );
    }


    /**
     * @brief Clamp one channel of the Pixel to the range 0..255 inclusive.
     *
     * Observe that this routine does not, however, truncate pixels to integer
     * values.  If you need that, use the ow_floor() or floor() method.
     * The 0..255 range is hardcoded in since it is such a common use case.
     */
    static
    void ow_clamp_channel( float* chan )
    {
        // hit KJB(UNTESTED_CODE());
        *chan = std::min( 255.0f, std::max( 0.0f, *chan ) );
    }

    /**
     * @brief Clamp one channel to a specified range
     */
    static
    void ow_clamp_channel( float* chan, float minval, float maxval )
    {
        KJB(UNTESTED_CODE());
        *chan = std::min( maxval, std::max( minval, *chan ) );
    }

    /**
     * @brief Clamp all channels of the pixel to the range 0..255 inclusive.
     * @return Lvalue of the modified PixelRGBA
     *
     * Observe that this routine does not, however, truncate pixels to integer
     * values.  If you need that, use the ow_floor() or floor() method.
     * This method has overwriting semantics.
     */
    PixelRGBA& ow_clamp()
    {
        // hit KJB(UNTESTED_CODE());
        ow_clamp_channel( & this -> r );
        ow_clamp_channel( & this -> g );
        ow_clamp_channel( & this -> b );
        ow_clamp_channel( & this -> extra.alpha );
        return *this;
    }

    /**
     * @brief Clamp all channels to a specified range, overwriting.
     * @return Lvalue of the modified PixelRGBA
     */
    PixelRGBA& ow_clamp( float minval, float maxval )
    {
        KJB(UNTESTED_CODE());
        ow_clamp_channel( & this -> r, minval, maxval );
        ow_clamp_channel( & this -> g, minval, maxval );
        ow_clamp_channel( & this -> b, minval, maxval );
        ow_clamp_channel( & this -> extra.alpha, minval, maxval );
        return *this;
    }

    /**
     * @brief Return a new pixel with all channels clamped to range 0..255.
     *
     * Observe that this routine does not, however, truncate pixels to integer
     * values.  If you need that, use the floor() method.
     */
    PixelRGBA clamp() const
    {
        // hit KJB(UNTESTED_CODE());
        PixelRGBA clamped( *this );
        clamped.ow_clamp();
        return clamped;
    }

    /**
     * @brief Return a new pixel with all channels clamped; non-overwriting.
     */
    PixelRGBA clamp( float minval, float maxval ) const
    {
        KJB(UNTESTED_CODE());
        PixelRGBA clamped( *this );
        clamped.ow_clamp( minval, maxval );
        return clamped;
    }

    /**
     * @brief Truncate pixel channel values to integers; overwriting.
     * @return Lvalue of the modified PixelRGBA
     */
    PixelRGBA& ow_floor()
    {
        KJB(UNTESTED_CODE());
        r = floorf( r );
        g = floorf( g );
        b = floorf( b );
        extra.alpha = floorf( extra.alpha );
        return *this;
    }

    /**
     * @brief Return new pixel with integer channel values.
     */
    PixelRGBA floor() const
    {
        KJB(UNTESTED_CODE());
        PixelRGBA floored( *this );
        floored.ow_floor();
        return floored;
    }

    /**
     * @brief Adds another pixel (channel-wise) to this pixel, overwriting.
     *
     * @warning Don't forget to ow_clamp() if you think you might need to.
     */
    PixelRGBA& operator+=( const kjb_c::Pixel& p )
    {
        // hit KJB(UNTESTED_CODE());
        r += p.r;
        g += p.g;
        b += p.b;
        extra.alpha += p.extra.alpha;
        return *this;
    }

    /**
     * @brief This will add corresponding channels, creating a new pixel.
     *
     * @warning Don't forget to clamp() if you think you might need to.
     */
    PixelRGBA operator+( const kjb_c::Pixel& p ) const
    {
        // hit KJB(UNTESTED_CODE());
        PixelRGBA sum( *this );
        sum += p;
        return sum;
    }

    /**
     * @brief Overwrite a pixel by multiplying it by p.
     *
     * This performs a simple channel-wise product.
     *
     * @warning Don't forget to ow_clamp() if you think you might need to.
     */
    PixelRGBA& operator*=(const PixelRGBA& p)
    {
        // hit KJB(UNTESTED_CODE());
        r *= p.r;
        g *= p.g;
        b *= p.b;
        extra.alpha *= p.extra.alpha;
        return *this;
    }

    /**
     * @brief Overwrite a pixel by scaling its value (r, g, b, and alpha too)
     *
     * @warning Don't forget to ow_clamp() if you think you might need to.
     */
    PixelRGBA& operator*=( float k )
    {
        // hit KJB(UNTESTED_CODE());
        r *= k;
        g *= k;
        b *= k;
        extra.alpha *= k;
        return *this;
    }

    /**
     * @brief Test for EXACT pixel equality in all channels
     * @warning Pixel stores channel values as floating point numbers!
     *          So instead of p==q you might want p.floor()==q.floor()
     */
    bool operator==( const kjb_c::Pixel& p ) const
    {
        KJB(UNTESTED_CODE());
        return r==p.r && g==p.g && b==p.b && extra.alpha==p.extra.alpha;
    }

    /**
     * @brief Test for ANY pixel inequality in any channel
     * @warning Pixel stores channel values as floating point numbers!
     *          So instead of p != q you might want p.floor() != q.floor()
     */
    bool operator!=( const kjb_c::Pixel& p ) const
    {
        KJB(UNTESTED_CODE());
        return ! operator==( p );
    }

    /// @brief express RGB values in an HTML-style string
    std::string as_hex_triplet() const
    {
        return pixel_as_hex_triplet_string( this );
    }
};


/**
 * @brief Multiply two pixels together.
 *
 * This simply performs an channel-wise product.
 *
 * @warning Don't forget to clamp() if you think you might need to.
 */
inline
PixelRGBA operator*(const PixelRGBA& p, const PixelRGBA& q)
{
    // hit KJB(UNTESTED_CODE());
    PixelRGBA product = p;
    product *= q;
    return product;
}

/**
 * @brief Scale a pixel by a floating point value on the right.
 *
 * @warning Don't forget to clamp() if you think you might need to.
 */
inline
PixelRGBA operator*( const PixelRGBA& p, double k )
{
    // hit KJB(UNTESTED_CODE());
    PixelRGBA scaled = p;
    scaled *= k;
    return scaled;
}

/**
 * @brief Scale a pixel by a floating point value on the left.
 *
 * @warning Don't forget to clamp() if you think you might need to.
 */
inline
PixelRGBA operator*( double k, const PixelRGBA& p )
{
    // hit KJB(UNTESTED_CODE());
    return p*k;
}


/**
 * @brief Take the channel-wise absolute value of a kjb_c::Pixel.
 */
inline
kjb_c::Pixel abs(const kjb_c::Pixel& p)
{
    kjb_c::Pixel q;
    q.r = fabsf(p.r);
    q.g = fabsf(p.g);
    q.b = fabsf(p.b);

    return q;
}

/// @brief express color as HTML-style hex triplet, e.g., "#FFCC00," of pointer
inline
std::string pixel_as_hex_triplet_string( const kjb_c::Pixel* p )
{
    kjb::PixelRGBA q( *p );
    q.ow_clamp();
    std::ostringstream trip;
    trip    << "#" << std::hex << std::setfill('0')
            << std::setw(2) << int(q.r)
            << std::setw(2) << int(q.g)
            << std::setw(2) << int(q.b);
    return trip.str();
}

/// @brief express color as HTML-style hex triplet, e.g., "#FFCC00," reference
inline
std::string pixel_as_hex_triplet_string( const kjb_c::Pixel& p )
{
    return pixel_as_hex_triplet_string( &p );
}

/// @}

} //namespace kjb

#endif
