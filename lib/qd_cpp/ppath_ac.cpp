/**
 * @file
 * @brief Implementation of the PixPathAc class
 * @author Andrew Predoehl
 */

/*
 * $Id: ppath_ac.cpp 21596 2017-07-30 23:33:36Z kobus $
 */


#include "l/l_sys_debug.h"  /* For ASSERT */
#include "qd_cpp/ppath_ac.h"
#include "l_cpp/l_util.h"
#include "l/l_sys_lib.h"
#include "l/l_sys_io.h"


namespace kjb
{
namespace qd
{

/**
 * @brief Return index of PixPoint in path at a given arclength ratio
 *
 * @param rat   Ratio of two polygonal path lengths.  The denominator is the
 *              path length of whole path.  The numerator is the path length
 *              from the path front to the index of the vertex to be returned,
 *              if that vertex exists.  If it does not exist, then there must
 *              be a vertex "before" and "after" the desired point, and we
 *              return the index of the one "before," i.e., closer to the
 *              front.  Confusing?  Example: if rat=0.5 then
 *              you get the index of the vertex at halfway or nearest halfway
 *              without going over halfway.  If the ratio is more than one,
 *              we return the index of the last point.
 *              One exception to the above description:  If the ratio is
 *              not positive, we return the index of the front point.
 *
 * The actual arclength at that point is at most the given value:  it is
 * the "floor" of possible arclength ratios.
 * This has to search the path for the desired ratio.  If the arclength is
 * already memoized, it takes time O(log N) where N is the path size.  If the
 * arclength is not memoized, it takes time O(N).
 *
 * @return index of the vertex as described above.
 * @throws Too_small if the path has no vertices
 */
size_t PixPathAc::whereis_arclength_ratio( float rat ) const
{
    if ( 0 == m_path.size() )
    {
        KJB_THROW_2(Illegal_argument, "Too small for a/l ratio" );
    }
    if ( rat <= 0 )
    {
        return 0;
    }
    if ( 1 <= rat )
    {
        return m_path.size() - 1;
    }
    if ( m_path.size() <= 2 )
    {
        return 0;
    }

    KJB( ASSERT( 0 == arclength( size_t( 0 ) ) ) );
    KJB( ASSERT( arclength() == arclength( m_path.size() - 1 ) ) );
    const float target_al = rat * arclength();

    size_t low = 0, high = m_path.size() - 1, mid; // OK to leave mid uninitialized
    while( low < high && ( mid = (low + high) / 2 ) != low )
    {
        KJB( ASSERT( arclength( low ) <= target_al ) );
        KJB( ASSERT( arclength( high ) > target_al ) );
        if ( target_al < arclength( mid ) )
        {
            high = mid;
        }
        else
        {
            low = mid;
        }
    }

    return mid;
}


float PixPathAc::arclength() const
{
    refresh_the_cache_if_stale();
    if ( 0 == m_path.size() )
    {
        KJB_THROW_2(Illegal_argument, "PixPath too small to have arclength" );
    }
    return arclen_cache.back();
}



} // end namespace qd
} // end namespace kjb

