/**
 * @file
 * @brief Contains definition for class PixPathAc
 * @author Andrew Predoehl
 */
/*
 * $Id: ppath_ac.h 20057 2015-11-10 07:11:36Z predoehl $
 */

#ifndef PPATH_AC_H_UOFARIZONAVISION
#define PPATH_AC_H_UOFARIZONAVISION 1

#include <qd_cpp/pixpath.h>

namespace kjb
{
namespace qd
{

/**
 * @brief This is like PixPath except that it has an arclength cache,
 *          for teh performance.
 */
class PixPathAc
{
    PixPath m_path;

    /**
     * stores (sometimes) cumulative polygonal path lengths as measured from
     * the front element of the path.  Distance is Euclidean.
     */
    mutable std::vector< float > arclen_cache;

    /// @brief flag true iff field 'arclen_cache' contains good values
    mutable bool arclen_valid;

    /**
     * @brief update arclen_cache if it is marked as being stale
     * @throws KJB_error in case of a (rare) problem.
     */
    void refresh_the_cache_if_stale() const
    {
        if ( ! arclen_valid )
        {
            ETX( m_path.arclength( &arclen_cache ) );
            arclen_valid = true;
        }
    }

protected:
    /// @brief default ctor almost the same as the PixPath default ctor
    PixPathAc()
    :   m_path( PixPath::reserve() ),
        arclen_valid( false )
    {}

    /// @brief ctor receives the size to reserve
    PixPathAc( size_t potential_size )
    :   m_path( PixPath::reserve( potential_size ) ),
        arclen_valid( false )
    {}

    /// @brief ctor loads from a file, just like PixPath ctor
    PixPathAc( const std::string& fn )
    :   m_path( PixPath::load( fn ) ),
        arclen_valid( false )
    {}

public:

    typedef PixPath::const_iterator const_iterator;

    /// @brief ctor copies from a PixPath.  This is the only public ctor.
    PixPathAc( const PixPath& path )
    :   m_path( path ),
        arclen_valid( false )
    {}

    /// @brief named ctor creates an empty path but reserves some memory for it
    static PixPathAc reserve( size_t potential_size = 0 )
    {
        return PixPathAc( potential_size );
    }


    /// @brief assign a location in a PixPathAc
    PixPathAc& assign( unsigned index, const PixPoint& newp )
    {
        arclen_valid = false;
        m_path.assign( index, newp );
        return *this;
    }

    /// @brief swap representations of two PixPathAc objects
    void swap( PixPathAc& vpp )
    {
        m_path.swap( vpp.m_path );
        arclen_cache.swap( vpp.arclen_cache );
        std::swap( arclen_valid, vpp.arclen_valid );
    }

    /// @brief add a new point to the end of the path
    void push_back( const PixPoint& pp )
    {
        arclen_valid = false;
        m_path.push_back( pp );
    }

    /// @brief throw away all points
    void clear()
    {
        arclen_valid = false;
        m_path.clear();
    }

    /// @brief append range of another path to this
    PixPathAc& append( const_iterator begin, const_iterator end )
    {
        arclen_valid = false;
        m_path.append( begin, end );
        return *this;
    }

    /**
     * @brief tack on a copy of a given path to the end of this path
     * @param suffix path to be concatenated
     * @return reference to this path after the update
     */
    PixPathAc& append( const PixPath& suffix )  // PixPath input is adequate
    {
        arclen_valid = false;
        m_path.append( suffix );
        return *this;
    }

    /// @brief see description of PixPath::append_no_overlap()
    int append_no_overlap( const PixPath& suffix )
    {
        arclen_valid = false;
        return m_path.append_no_overlap( suffix );
    }

    /// @brief Reverse the order of this path, IN PLACE!
    PixPathAc& ow_reverse()
    {
        arclen_valid = false;
        m_path.ow_reverse();
        return *this;
    }

    /**
     * @brief compute polygonal path length along path, ret. vector of results.
     *
     * There is little use for this method, but it is here for completeness.
     * Instead you probably should use method arclength( size_t ) const.
     *
     * @param alvec Pointer to vector of floats into which we return the
     *              cumulative polygonal path lengths from start to each vertex
     * @return kjb_c::ERROR or kjb_c::NO_ERROR as appropriate
     */
    int arclength( std::vector< float >* alvec ) const
    {
        KJB( NRE( alvec ) );

        refresh_the_cache_if_stale();
        alvec -> resize( m_path.size() );
        std::copy( arclen_cache.begin(), arclen_cache.end(), alvec->begin() );
        return kjb_c::NO_ERROR;
    }

    /**
     * @brief Return the polygonal path length from front() to indexed point.
     *
     * @param index index of vector at which to compute polygonal path length.
     * @return Polygonal path length computed as summed Euclidean distances.
     *
     * This method makes the previous one sort of obsolete.  I recommend you
     * use this method.
     */
    float arclength( size_t index ) const
    {
        refresh_the_cache_if_stale();
        return arclen_cache.at( index );
    }

    /**
     * @brief Return the polygonal path length for this path, front() to back()
     */
    float arclength() const;

    size_t whereis_arclength_ratio( float ) const;

    /// @brief Return index of PixPoint closest to but not beyond mid-arclength
    size_t halfway() const
    {
        return whereis_arclength_ratio( 0.5 );
    }

    /**
     * @brief access the underlying pixpath
     *
     * Avoid its arclength methods, though, unless you're deliberately trying
     * to waste time.
     */
    const PixPath& get_pixpath() const
    {
        return m_path;
    }
};

}
}

#endif
