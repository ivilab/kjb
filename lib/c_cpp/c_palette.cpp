/**
 * @file
 * @brief Implementation for class Palette, giving you random colors.
 * @author Andrew Predoehl
 */

/*
 * $Id: c_palette.cpp 19803 2015-09-20 04:05:01Z predoehl $
 */

#include <l/l_def.h>
#include <l_cpp/l_exception.h>
#include <i_cpp/i_hsv.h>
#include <m_cpp/m_vector.h>
#include <c_cpp/c_palette.h>

/**
 * Set this to 1 or 0 to to turn on or off the behavior of generating extra
 * colors, then throwing away (one of) too-close color pairs, until we've
 * thinned out the herd enough.
 *
 * Advice:  leave it zero, which runs faster and produces better results IMO.
 */
#define GEN_EXTRA_THEN_CULL 0


#if GEN_EXTRA_THEN_CULL
    /*
     * which space to use to determine color similarity?
     * My opinion:  they work about equally well and HSY is faster.
     */
    #define HSY_SPACE 1 /**< flag: 1=use Hue, Sat., Luma space for distance */
    #define LAB_SPACE 0 /**< flag: 0=do not use LAB* space for color distance*/
#endif


#if LAB_SPACE
#include <c/c_colour_space.h>
#endif


#include <gsl_cpp/gsl_qrng.h>

#include <vector>
#include <ostream>


namespace
{


/**
 * @brief makes distinct 24-bit colors
 *
 * The colors are only a little bit random -- actually the goal is to make
 * them all as distinguishable as possible from one another.  So, there should
 * not be two very similar greens, or purply-blues, or anything.  All should
 * be as distinct as we can make them.  If you randomly choose RGB, the
 * result will NOT have this property for, say, 100 colors.  In fact it is very
 * difficult to make 100 pairwise-distinctive colors (495 comparisons).
 *
 * The original motivation was to serve as "colorist" for a visualization of
 * superpixel segmentation, where each superpixel gets painted a different
 * color.  We want always to see clearly the boundary between superpixels.
 * The color value itself tells us nothing, we only care that it is distinct
 * from its neighbors.
 * 
 * This class is lazy.  Color 0 is 100% transparent (alpha==0) when the class
 * has (lazily) not built itself yet.  After that it goes opaque (alpha==255).
 */
class ColorPick
{

    std::vector< kjb::PixelRGBA > colors;

    void build(); // complicated overkill method, but it makes good colors.

    bool is_not_yet_built() const
    {
        return 0 == colors[ 0 ].extra.alpha; // check for the sentinel value
    }

public:

    ColorPick( size_t num_colors )
    :   colors( num_colors )
    {
        if ( 0 == num_colors )
        {
            KJB_THROW_2(kjb::Illegal_argument,"Palette size must be positive");
        }
        colors[ 0 ].extra.alpha = 0; // sentinel:  class hasn't built itself
    }

    void swap( ColorPick& other )
    {
        colors.swap( other.colors );
    }

    kjb::PixelRGBA operator[]( size_t index )
    {
        if ( is_not_yet_built() ) build();
        return colors.at( index );
    }

    size_t size() const
    {
        return colors.size();
    }

    void debug_print( std::ostream& os ) const;
};


#if LAB_SPACE
void convert_to_lab( const kjb::PixelRGBA cc, kjb_c::Vector** lab )
{
    kjb::Vector vv( 3 );

    vv.at( 0 ) = cc.r;
    vv.at( 1 ) = cc.g;
    vv.at( 2 ) = cc.b;

    ETX( kjb_c::convert_vector_rgb_to_lab( lab, vv.get_c_vector(), 00 ) );
}



double lab_dist2( const kjb::PixelRGBA& c1, const kjb::PixelRGBA& c2 )
{
    kjb_c::Vector *v1 = 00, *v2 = 00;

    convert_to_lab( c1, &v1 );
    convert_to_lab( c2, &v2 );

    kjb::Vector w1( v1 ), w2( v2 );
    w1 -= w2;
    return w1.magnitude_squared();
}
#endif



void ColorPick::build()
{
    // If already built, return.  Now say that with a double negative:
    if ( ! is_not_yet_built() ) return;

#if GEN_EXTRA_THEN_CULL
    const float     EXTRA_FACTOR = 1.5; // you can twiddle this ad libidum
#else
    const size_t    EXTRA_FACTOR = 1;
#endif

    const size_t    K_CLUST = colors.size(),
                    EXTRA_CLUST =  K_CLUST * EXTRA_FACTOR;

    std::vector< kjb::PixelRGBA > precolors;
    precolors.reserve( EXTRA_CLUST );

    //////// THIS ONE WORKS PRETTY WELL, even if EXTRA_FACTOR is 1.
    const double ISOSCALE = 2.0, Y_TOO_DARK = 0.15;
    kjb::Vector v( 3 );
    for ( kjb::Gsl_Qrng_Sobol qq( 3 ); precolors.size() < EXTRA_CLUST; )
    {

        v = qq.read();  // elements 0,1,2 correspond to H,S,Y.

        /*
         * Sadly (I guess) the Y value, luminance, must lie in range [0,1] and
         * the isotropic scaling below is going to blow a lot of them out of
         * range.  We humanely reject them now to save them from future
         * suffering, and also to skip an unnecessary for loop.
         * Also we reject values that are too dark, i.e., luminance below 0.15.
         */
        if ( v[ 2 ] < Y_TOO_DARK / ISOSCALE || 1.0 / ISOSCALE < v[ 2 ] )
        {
            continue; // reject:  Y component is too dark or too bright
        }

        /*
         * QRNG values are not random; there's something "funny" about their
         * spacing; so to preserve their "funny" distances we must scale them
         * by the same factor in each dimension.
         * We have to scale up by 2 because H values and S values range from -1
         * to +1.
         */
        v *= ISOSCALE;

        // Translate the H and S components from range [0,2] to [-1,1].
        v[ 0 ] -= 1;    // H component
        v[ 1 ] -= 1;    // S component

        /*
         * Test whether that point lies within the HSY polyhedron; the
         * polyhedron is not a cuboid, so not every point in
         * [-1,1] x [-1,1] x [0,1] is a valid HSY value.  So there is more
         * rejection going on at this step.  The function's return value tells
         * you whether v is valid, and if so, the RGB values are to be found in
         * Pixel p.
         */
        kjb_c::Pixel p;
        p.extra.alpha = 255; // doesn't get set by function, Valgrind is sad.
        int rc = get_pixel_from_hsluma_space( v, &p );
        if ( EXIT_SUCCESS == rc )
        {
            precolors.push_back( p );
        }
    }

#if GEN_EXTRA_THEN_CULL
    // HEURISTIC FIXUP:  First thin it out.  Then look for splitups.
    std::vector< double > dist2;
    std::vector< size_t > winner;
    kjb::Vector vk;

    /* This loop could be "while(true)" since the break below will exit us.
     * However, I am afraid of putting an infinite loop in the code for real.
     */
    for ( int irrelevant = 0; irrelevant < 9999; ++irrelevant )
    {
        using kjb_c::kjb_rand;

        // compute nearest color to each color
        dist2.assign( precolors.size(), DBL_MAX );
        winner.assign( precolors.size(), precolors.size() );
        for ( size_t kix = 0; kix < precolors.size(); ++kix )
        {
            vk = hsluma_space( precolors[ kix ] );
            for ( size_t jix = 0; jix < precolors.size(); ++jix )
            {
                if ( kix == jix ) continue;
#if HSY_SPACE
                // Compute spacing using HSY
                double d2 = ( vk
                    - hsluma_space( precolors[ jix ] ) ).magnitude_squared();
#endif
#if LAB_SPACE
                // Compute spacing using L*a*b
                double d2 = lab_dist2( precolors[ kix ], precolors[ jix ] );
#endif
                if ( d2 < dist2[ kix ] )
                {
                    dist2[ kix ] = d2;
                    winner[ kix ] = jix;
                }
            }
            KJB(ASSERT( winner[ kix ] < precolors.size() ));
        }

        // find which 2 colors are most cramped, least cramped
        size_t mcix = 0, lcix = 0;
        for ( size_t kix = 0; kix < precolors.size(); ++kix )
        {
            if ( dist2[ kix ] < dist2[ mcix ] )
            {
                mcix = kix;
            }
            if ( dist2[ kix ] > dist2[ lcix ] )
            {
                lcix = kix;
            }
        }
        KJB(ASSERT( dist2.at( mcix ) < dist2.at( lcix ) || mcix == lcix ));
        KJB(ASSERT( dist2[ mcix ] == dist2[ winner[ mcix ] ] ));

        // Cut one of the two most cramped colors if we have surplus colors
        if ( K_CLUST < precolors.size() )
        {
            size_t cutix = kjb_rand() < 0.5 ? mcix : winner[ mcix ];
            precolors[ cutix ] = precolors.back();
            precolors.pop_back();
        }

        // No more cuts, but can one of the cramped pair move in with you guys?
        else if ( dist2[ mcix ] * 4 < dist2[ lcix ] )
        {
            size_t shiftix = kjb_rand() < 0.5 ? mcix : winner[ mcix ];
            precolors[ shiftix ] =
                            precolors[ lcix ] + precolors[ winner[ lcix ] ];
            precolors[ shiftix ] *= 0.5;
        }
        else
        {
            break; //   <--- the usual exit condition
        }
    }
#endif
    KJB(ASSERT( K_CLUST == precolors.size() ));
    std::copy( precolors.begin(), precolors.end(), colors.begin() );
}


void ColorPick::debug_print( std::ostream& os ) const
{
    if ( is_not_yet_built() )
    {
        os << "Palette is lazy, and has not computed itself yet.\n";
    }
    else
    {
        for ( size_t kix = 0; kix < colors.size(); ++kix )
        {
            int rrr = static_cast< int >( colors[ kix ].r ),
                ggg = static_cast< int >( colors[ kix ].g ),
                bbb = static_cast< int >( colors[ kix ].b );
            os << rrr << ' ' << ggg << ' ' << bbb << '\n';
        }
    }
}


} // anonymous namespace



namespace kjb {


Palette::Palette( size_t size )
{
    my_pal.reserve( size );
    if ( 2 == size )
    {
        // Severe black and white for a two-color palette
        my_pal.push_back( kjb::PixelRGBA( 0, 0, 0 ) );
        my_pal.push_back( kjb::PixelRGBA( 250, 250, 250 ) );
    }
    else
    {
        // Random colors for a multicolor palette -- this works nicely
        ColorPick cp( size );
        for ( size_t kix = 0; kix < size; ++kix )
        {
            my_pal.push_back( cp[ kix ] );
        }
    }
}


/**
 * @brief compute a new color mixed from old.
 * @return a color mixed from the palette components
 * @param weights_d vector of weights, sum of 1, big means more of that color.
 * @throws Illegal_argument if 'weights' is the wrong length.
 * @throws Illegal_argument if 'weights' does not sum to unity.
 *
 * Less cryptically:  the input vector of doubles must be the same size as
 * the palette size(), and the values in the vector are treated as weights;
 * they must be normalized to sum to unity (or pretty close).  The output
 * pixel is a weighted sum of the palette colors, using these weights.
 */
kjb::PixelRGBA Palette::weighted_sum( const kjb::Vector& weights_d ) const
{
    if (weights_d.get_length() != (int)size() ) KJB_THROW(Illegal_argument);

    float sum = 0;
    for ( int iii = 0; iii < weights_d.get_length(); ++iii )
    {
        sum += weights_d( iii );
    }
    const float EPS = 1.0E-4f;
    if (fabs( sum - 1.0f ) > EPS ) KJB_THROW(Illegal_argument);
    kjb::PixelRGBA weighted_sum( 0, 0, 0 );
    std::vector< kjb::PixelRGBA >::const_iterator ppal = my_pal.begin();
    for ( int iii = 0; iii < weights_d.get_length(); ++iii )
    {
        weighted_sum += *ppal++ * weights_d( iii );
    }
    return weighted_sum.ow_clamp(); // alpha channel surely needs clamping
}


} // namespace kjb

