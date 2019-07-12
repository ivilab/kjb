/**
 * @file
 * @brief Contains definition for class Palette, plus helper funs.
 * @author Andrew Predoehl
 *
 * This file provides the interface to
 * a simple Palette class which makes up some random
 * colors for you for visualization.  In the Palette class you get a
 * few helper member functions to mix and operate on colors.
 * This class was developed in support of the Finding Trails project
 * (used to visualize superpixels).
 */
/*
 * $Id: c_palette.h 18148 2014-11-09 21:36:11Z predoehl $
 */

#ifndef PALETTE_H_PREDOEHL_UOFARIZONAVISION
#define PALETTE_H_PREDOEHL_UOFARIZONAVISION

#include <l_cpp/l_exception.h>
#include <i_cpp/i_pixel.h>
#include <m_cpp/m_vector.h>

#include <vector>


namespace kjb
{

/**
 * @brief Construct some colors, for visualizing grids of numbers
 *
 * The main property of the colors here is that they are supposed to be
 * pairwise-distinct, at least on computer monitors.  You pick however many
 * colors you want (and tell it to the ctor).  Also, this class supports
 * two methods to mix colors from your palette, given a vector of
 * mixing weights.
 */
class Palette
{
    std::vector< kjb::PixelRGBA > my_pal;   ///< storage for the palette colors

public:
    /**
     * @brief Build a palette of the indicated size
     *
     * If the size is two then it will just be black and white.  If the size
     * is larger then this will pick random colors for you, somewhat
     * intelligenly.  If the size is less than two then the result is
     * undefined.
     */
    Palette( size_t );

    /// @brief Build a palette from a vector of colored Pixels.
    Palette( const std::vector< kjb::PixelRGBA >& some_pal )
    :   my_pal( some_pal )
    {}

    /// @brief Return a palette entry, a color, in the form of a Pixel.
    const kjb::PixelRGBA& operator[]( size_t index ) const
    {
        return my_pal.at( index );
    }

    /// @brief Return the size of the palette (the number of entries)
    size_t size() const
    {
        return my_pal.size();
    }

    /// @brief Swap two palette entries, indicated by index
    void swap( size_t iii, size_t jjj )
    {
        std::swap( my_pal.at( iii ), my_pal.at( jjj ) );
    }

    /**
     * @brief convenience typedef for generating/mixing a color.
     *
     * This is a type for "mix my colors" operations:  input is a vector
     * of color weights, and the output is a color (in the form of a Pixel).
     *
     * In other words, you can use this to indicate weighted_sum or pick_max.
     */
    typedef kjb::PixelRGBA ( Palette::* PIXOP )( const kjb::Vector& ) const;


    kjb::PixelRGBA weighted_sum( const kjb::Vector& ) const;


    /**
     * @brief An unforgiving function that chooses the most popular color.
     * @return most popular color
     * @param weights   vector of numeric votes for the palette colors
     * @throws Illegal_argument if 'weights' is the wrong length.
     *
     * Less cryptically:  the input vector of doubles must be the same size as
     * the palette size.  We scan the input vector to find its maximum entry,
     * and whichever one it is, the color in the palette corresponding to that
     * index is returned.  In the event of a tie, the smallest of the tying
     * indices is returned.  This "mixes" the colors only in the sense that it
     * forms a mix comprising 100% of the most weighted color, and 0% of the
     * others.
     */
    kjb::PixelRGBA pick_max( const kjb::Vector& weights ) const
    {
        int max_index;
        if (weights.get_length() != (int)size() ) KJB_THROW(Illegal_argument);
        weights.max( &max_index );
        return my_pal[ max_index ];
    }
};

} // namespace kjb

#endif
