/**
 * @file
 * @brief Implementation for PixPath specializations that resize paths
 * @author Andrew Predoehl
 *
 * Sadly there is a fair amount of drek in here, stuff left over from when I
 * tried to reduce a path in a rather ad-hoc way.  Function polyline_approx()
 * is much better.
 */
/*
 * $Id: pathresize.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "l/l_def.h"
#include "qd_cpp/pathresize.h"

#include <vector>
#include <deque>
#include <algorithm>
#include <boost/shared_ptr.hpp>

#define ASSIDUOUS_DEBUG_CHECKING_SPRUCE 0   /**< macro: 1=expensive paranoia */

#if ASSIDUOUS_DEBUG_CHECKING_SPRUCE
#define ASSERTO(x) KJB(ASSERT(x)) /**< assertion when "assiduously" testing  */
#else
#define ASSERTO(x)                /**< stub non-assertion when less paranoid */
#endif


namespace
{

using kjb::qd::PixPoint;
using kjb::qd::PixPath;


/// @brief a functor much like fun sosq_error() but it memoizes its results
class Memo_sosq_error
{
    const PixPath m_path;   ///< path for which we compute the error values
    // I could have made this mutable, but I couldn't see the advantage here.
    kjb::Vector m_sosq_err; ///< row-maj repr of matrix of memoized answers
    std::vector< bool > mmm_good; ///< row-major array of valid flags

public:

    /// @brief ctor just needs to know the path we are working with
    Memo_sosq_error( const PixPath& path )
    :   m_path( path ),
        m_sosq_err( int( path.size() * path.size() ), 0.0 ),
        mmm_good( path.size() * path.size(), false )
    {}

    /// @brief evaluate functor at any two indices, maybe memoize result
    float operator()( size_t ix1, size_t ix2 )
    {
        const size_t rmix = ix1 * m_path.size() + ix2;
        if ( ! mmm_good.at( rmix ) )
        {
            const size_t rmco = ix2 * m_path.size() + ix1;
            KJB( ASSERT( ! mmm_good.at( rmco ) ) );
            m_sosq_err[ rmix ] = m_sosq_err[ rmco ]
                                            = sosq_error( m_path, ix1, ix2 );
            mmm_good[ rmix ] = mmm_good[ rmco ] = true;
        }
        return m_sosq_err[ rmix ];
    }

    /// @brief const getter provides access to the original path given to ctor
    const PixPath& get_path() const
    {
        return m_path;
    }
};


/**
 * @brief This class constructs a PixPath of the "right size" you pick.
 *
 * If the input has too many points, it kills off the "weak" points.
 * Weak points are points that are relatively less informative.
 * We currently use a dynamic programming approach based on a sun-of-squares
 * additive metric.  (We formerly used a slow, greedy heuristic.)
 *
 * In case you are curious:
 * PixPath_compress_1 was a lame idea that never even got off the ground.
 */
class PixPath_compress_2
:   public PixPath
{

    static PixPath spruce_up( const PixPath&, const PixPath& );

#if 0
    // superceded by polyline_approx(), which is much faster
    static PixPath devil_take_hindmost( const PixPath&, size_t );
#endif

    /// @brief expand the input path if necessary, or compress, and polish
    static PixPath spruce_up_2( const PixPath& p, size_t sz, float dtc )
    {
        try
        {
            kjb::qd::PixPath_expander pe( p, sz, dtc );
            //return spruce_up( pe, devil_take_hindmost( pe, sz ) );
            return spruce_up( pe, polyline_approx( pe, sz ) );
        }
        catch( PixPoint::Unused& )
        {
            kjb_c::p_stderr( "Caught PixPoint::unused in %s.\n", __func__ );
            throw;
        }
    }

public:

    /**
     * @brief Construct a path of the proper size, using cruelty and greed.
     *
     * @param p                     A path with potentially too many or too few
     *                              points in it, and potentially having points
     *                              that are too close together.
     *
     * @param sz                    The path we construct will contain exactly
     *                              this many trackpoints.
     *
     * @param distance_too_close    Any two path points that are separated by
     *                              a distance, in pixels, less than or equal
     *                              to this value are judged to be too close
     *                              together and at least one of them will be
     *                              excluded from the final result.
     *
     * We construct a PixPath with a fixed number of trackpoints,
     * The philosophy is procrustean:  if a
     * path has too few points, we stretch out the path by interpolating in
     * extra trackpoints.  That is relatively easy.  If a path has too many
     * trackpoints, we selectively kill some of them (cruelly).  We do this
     * using a greedy approach.  Then we try to "spruce up" the greedy result
     * to make it better:  greed does not lead to an optimal solution, but we
     * can do a few simple, small searches to glean some improvements.
     */
    PixPath_compress_2( const PixPath& p, size_t sz, float distance_too_close )
    :   PixPath( spruce_up_2( p, sz, distance_too_close ) )
    {}
};


/// @return squared length of line segment btw q and closest point on line v1v2
float perp_dist_sq( const PixPoint& v1, const PixPoint& v2, const PixPoint& q )
{
#if TEST_PIXPOINT_UNUSED
    if ( v1.is_unused() || v2.is_unused() || q.is_unused() )
    {
        throw PixPoint::Unused();
    }
#endif
    if ( v1 == v2 )
    {
        return v1.dist2( q );
    }

    const PixPoint v1v2( v1 - v2 ), qv2( q - v2 );
    float ttt = v1v2.idot( qv2 ) / float( v1v2.idot( v1v2 ) );
    kjb::Vector2 r( to_vector2(v2) );
    r += to_vector2( v1v2 ) * ttt;
    return float( ( r - to_vector2(q) ).magnitude_squared() );
}


/// @brief a cell of the dynamic programming table induced by Perez-Vidal alg
struct PerVid {
    size_t ix;
    float e;
    PerVid( float fff = FLT_MAX, size_t jjj = INT_MAX ): ix( jjj ), e( fff ) {}
};


/**
 * @brief compute an optimal polyline approximation for a given PixPath
 *
 * Optimality is defined by a sum-of-squares additive error criterion.  This
 * works by dynamic programming (DP).  This is a class and it is smart enough
 * to save state for you so you can try different subsampling sizes.  I.e., if
 * you use it to reduce a path to size 50, you can
 *
 * The DP here is refreshingly straightforward.  It does not leverage any fancy
 * geometry, and the optimal substructure here is obvious.  The key
 * "how does it end" insight is this.  Suppose you have a path, m_base_path, of
 * size at least N+1.  Suppose you know the optimal size-m polygonal
 * approximation for all prefixes of it up to the prefix of length N, and for
 * all 1 < m <= M for some M.  Now, the optimal size-M polygonal approximation
 * for the prefix of length N+1 will be the concatenation of the following:
 * - the optimal size M-1 approximation for the prefix of length J, plus
 * - the segment m_base_path[J-1] to m_base_path[M-1],
 * where J minimizes the error, and you just look for J by a linear search.
 *
 * This has a quadratic space requirement and a cubic time requirement.
 */
class PolylineApprox {
    PixPath m_base_path;            ///< original path to be approximated
    Memo_sosq_error m_se;  ///< squared error metric functor

    /**
     * Subsolution table definition:  mesa[i][j].e contains the error of an
     * optimal i+2-vertex polygonal approximation of m_base_path[j] to
     * m_base_path.back().  This table will be sort of triangular, with row 0
     * being the longest.  Actually more like a trapezoid.
     */
    std::vector< std::vector< PerVid > > mesa;

    float m_last_error;     ///< cache of sum-square-err of last approximation

    void fill_first_row();  // base case of table-filler
    void fill_next_row();   // DP base of table-filler

public:
    PolylineApprox(const PixPath& base_path)
    :   m_base_path(base_path),
        m_se(base_path),
        m_last_error(-1)
    {
        mesa.reserve(base_path.size());
    }

    /// @brief generate best pixpath approximation of given size (or smaller)
    PixPath approx(size_t goal_size);

    float get_last_error() const
    {
        return m_last_error;
    }
};


/*
 * First row corresponds to a 2-vertex polygonal approximation, i.e., a
 * line segment, thus the table's error value is just given by the error
 * function.
 */
void PolylineApprox::fill_first_row()
{
    mesa.resize(1);
    mesa[0].resize(m_base_path.size() - 1);
    for (size_t aaa = 0; aaa < m_base_path.size() - 1; ++aaa )
    {
        mesa[0].at( aaa ) = PerVid(     m_se( aaa, m_base_path.size() - 1 ),
                                        m_base_path.size() - 1
                                    );
    }
}


/*
 * Rows after the first require a (straightforward) minimization
 */
void PolylineApprox::fill_next_row()
{
    const size_t bbb = mesa.size();
    mesa.resize(1+bbb);

    const size_t EndA = m_base_path.size() - bbb - 1;
    mesa.at( bbb ).resize( EndA );
    for( size_t aaa = 0; aaa < EndA; ++aaa )
    {
        /*
         * index arithmetic here is confusing but I think I've got it
         * right; you can access column EndA of the previous row because
         * the previous row is one cell longer than the current row.
         */
        for( size_t jjj = aaa + 1; jjj <= EndA; ++jjj )
        {
            float eee = mesa.at( bbb - 1 ).at( jjj ).e + m_se( aaa, jjj );
            if ( eee < mesa.at( bbb ).at( aaa ).e )
            {
                mesa.at( bbb ).at( aaa ) = PerVid( eee, jjj );
            }
        }
    }
}


PixPath PolylineApprox::approx(size_t goal_size)
{
    // trivial cases
    if (goal_size >= m_base_path.size()) return m_base_path; // maybe smaller
    if (goal_size < 3)
    {
        PixPath p(PixPath::reserve(2));
        if (goal_size > 0) p.push_back(m_base_path.front());
        if (goal_size > 1) p.push_back(m_base_path.back());
        return p;
    }

    // table-filling:  mesa.size() must grow until it equals (goal_size-2).
    if (0 == mesa.size()) fill_first_row();
    while (mesa.size() + 2 < goal_size) fill_next_row();

    // total solution
    PerVid final;
    if (mesa.size() + 2 > goal_size)
    {
        /* This branch is only reached if a previous call had a bigger
         * goal_size than the goal_size of this call.
         * When that occurs, the table is so huge that the answer is already
         * cached.
         */
        final = mesa.at(goal_size - 2).at(0);
    }
    else
    {
        /*
         * Total solution requires one more minimization for aaa equal to 0.
         * (We do not need to try multiple aaa's because we know we will take
         * index 0.)
         */
        const size_t Enda = m_base_path.size() - ( goal_size - 2 ) - 1;
        for( size_t jjj = 1; jjj <= Enda; ++jjj )
        {
            const float eee = mesa.at(goal_size - 3).at(jjj).e + m_se(0, jjj);
            if ( eee < final.e )
            {
                final = PerVid( eee, jjj );
            }
        }
    }

    /*
     * Now extract the error and the path from the completed table.
     */
    PixPath answer( PixPath::reserve( goal_size ) );
    answer.push_back( m_base_path.front() );
    answer.push_back( m_base_path[ final.ix ] );
    for (int bbb = goal_size - 3; bbb >= 0; --bbb)
    {
        const PerVid& next = mesa.at( bbb ).at( final.ix );
        final.e += next.e;
        answer.push_back( m_base_path[ final.ix = next.ix ] );
    }

    m_last_error = final.e;
    return answer;
}


PixPath spruce_up_body(
    const PixPath& p_0_plus,///< original path, maybe wrong size
    const PixPath& p_1      ///< the improved path, the right size
)
{
    /*
     * If p_1 is just an expanded version of p_0_plus, then we don't know how
     * to improve it any further, so forget it; we just return p_1.
     */
    if ( p_0_plus.size() <= p_1.size() )
    {
        KJB( ASSERT( p_1.has_subsequence( p_0_plus ) ) );
        return p_1;
    }

    /*
     * The following is an invariant we need below; I don't expect the
     * condition ever to be true.
     */
    if ( p_1.size() < 4 )
    {
        return p_1;
    }

    /*
     * Now look at the hausdorff distance between p_0_plus and p_1.
     * If p_1 is literally right on top of it, we cannot improve on that.
     * This is a "pure" notion:  you absolutely can't improve on perfection.
     */
    const PixPath i0( p_0_plus.interpolate() );
    const PixPath i1( p_1.interpolate() );
    const float hd = i0.hausdorff_distance( i1 );
    KJB( ASSERT( 0 <= hd ) );
    if ( 0 == hd )
    {
        return p_1;
    }

    /*
     * On second thought (actually a separate thought for a good reason)
     * let's look at hausdorff distance again.
     * If the distance is small enough, let's just save ourselves a lot of time
     * and say "p_1 might not be perfect but it is darn good."
     * This is NOT a "pure" notion, it is a heuristic.  That is why it is a
     * separate clause from the above notion.
     */
    const float GOOD_ENOUGH = 5.0f; // heuristic.  Units are pixels (@ 1 m/pix)
    if ( hd <= GOOD_ENOUGH )
    {
        return p_1;
    }

    /*
     * Time for our heuristic:
     * We will try to find four-point subranges of p_1 that are not very good,
     * and try to improve them by moving around the two middle points.
     * Final result will be stored in p_2, which is no worse than p_1.
     */
    KJB( ASSERT( 4 <= p_1.size() ) );
    PixPath p_2( p_1 ); // p_2 is at least as good, maybe better than, p_1
    PixPath p2i( p_2.interpolate() );   // must keep this up to date, danger!
    size_t jjj = 0;     // cursor within p_0_plus
    for( size_t iii = 0; iii + 3 < p_1.size(); ++iii ) // scan through p_2
    {
        const PixPoint p2iii = p_2[ iii ];
        ASSERTO( 1 == p_2.hits( p2iii ) ); // expensive, O(log N)
        PixPath p2prefix = p_2.prefix( iii );
        PixPath p2mid = p_2.subrange( iii, iii + 4 ); // do we need it?
        PixPath p2suffix = p_2.suffix( iii + 4 );

#if ASSIDUOUS_DEBUG_CHECKING_SPRUCE
        do {
        PixPath p2prefix_copy( p2prefix );
        ASSERTO( p2prefix_copy.append( p2mid ).append( p2suffix ) == p_2 );
        ASSERTO( p2prefix_copy == p_2 ); // really useless!
        } while(0);
#endif
        KJB( ASSERT( p2mid.front() == p2iii ) );

        // after debug, you should probably delete all the "expensive" tests,
        // but the next four are especially deletable.
        ASSERTO( 1 == p_0_plus.hits( p_2[ iii     ] ) );
        ASSERTO( 1 == p_0_plus.hits( p_2[ iii + 1 ] ) );
        ASSERTO( 1 == p_0_plus.hits( p_2[ iii + 2 ] ) );
        ASSERTO( 1 == p_0_plus.hits( p_2[ iii + 3 ] ) );

        /*
         * Also we put cursor jjj at the offset within p_0_plus that hits p2iii
         * (same coords., but a different object of course).
         */
        while( jjj < p_0_plus.size() && p_0_plus[ jjj ] != p2iii )
        {
            ++jjj;
        }
        KJB( ASSERT( jjj < p_0_plus.size() ) ); // keep this test around please
        ASSERTO( 1 == p_0_plus.hits( p_0_plus[ jjj ] ) );// expensive, O(log N)
        KJB( ASSERT( p_0_plus[ jjj ] == p2iii ) ); // stupid test (but cheap)

        /*
         * Now we put cursor mmm at the offset within p_0_plus corresponding to
         * iii+3's offset within p_2.
         */
        size_t mmm = jjj;
        while( mmm < p_0_plus.size() && p_0_plus[ mmm ] != p_2[ iii + 3 ] )
        {
            ++mmm;
        }
        KJB( ASSERT( mmm < p_0_plus.size() ) ); // keep this test
        KJB( ASSERT( jjj + 3 <= mmm ) ); // keep this test
        KJB( ASSERT( p_0_plus[ mmm ] == p_2[ iii + 3 ] ) );

        /*
         * Now that jjj and mmm are in position, we can define two small sub-
         * paths:  the stretch of path we wish to optimize, and the ideal by
         * which it is evaluated.  p0mid is the "ideal" subpath, comprising
         * possibly a large number of points that we wish to approximate by a
         * piecewise interpolation of just four points.
         */
        const PixPath p0mid( p_0_plus.subrange( jjj, mmm + 1 ) );
        const PixPath i0mid( p0mid.interpolate() );

        /*
         * Heuristic:  if the Hausdorff distance is small enough, move along.
         */
        float hdbest = i0mid.hausdorff_distance( p2mid.interpolate() );
        if ( hdbest <= GOOD_ENOUGH )
        {
            continue;
        }

        /*
         * Now we are going to try all pairs of points between jjj and mmm,
         * and try to find a four-point sequence better than p2mid.
         * If it finds something better, the better sequence will be in pbest.
         * The endpoints of pbest will unquestionably be p_0_plus[ jjj ] and
         * p_0_plus[ mmm ]; the trick is finding the two in the middle.
         * We search exhausively; the universe we search from is the range of
         * points p_0_plus[ jjj+1 ] to p_0_plus[ mmm-1 ].  We organize our
         * search by increasing gap between the two in the middle.  Gap size
         * is denoted nnn.  The smallest gap is a gap of zero (two consecutive
         * points in p_0_plus) and the largest gap size is mmm-jjj-3.
         */
        PixPath pbest( p_2.subrange( iii, 4 + iii ) );
        bool found_something_better = false;
        const size_t GAPLIMIT = mmm - jjj - 2;  // greater than biggest gap
        for( size_t nnn = 0; nnn < GAPLIMIT; ++nnn ) // varies with gap size
        {
            for( size_t ix1 = jjj + 1, ix2 = ix1 + 1 + nnn; ix2 < mmm;
                                                                ++ix1, ++ix2 )
            {
                PixPoint mid1( p_0_plus[ ix1 ] ), mid2( p_0_plus[ ix2 ] );
                PixPath pcandidate( pbest );
                pcandidate.assign( 1, mid1 );
                pcandidate.assign( 2, mid2 );
                float hd = pcandidate.interpolate().hausdorff_distance(i0mid);
                if ( hd < hdbest )
                {
                    hdbest = hd;
                    pbest.assign( 1, mid1 );
                    pbest.assign( 2, mid2 );
                    found_something_better = true;
                }
            }
        }

        /*
         * If we found something better, put it in place.
         */
        if ( found_something_better )
        {
            KJB( ASSERT( ! pbest[ 1 ].is_unused() ) );
            KJB( ASSERT( ! pbest[ 2 ].is_unused() ) );

            p2prefix.append( pbest ).append( p2suffix ); // clobbers!  it's ok!
            PixPath& preplacement( p2prefix );
            PixPath pri( preplacement.interpolate() );

            /*
             * What follows is an expensive test present here ONLY FOR DEBUG.
             * It is too expensive to leave in place permanently executable.
             * The idea is that the replacement (pri) is either better than the
             * original (if we have optimized a critical section of the trail)
             * or it is at least no worse (if we have optimized a section of
             * trail that was already pretty good and does not define the
             * global Hausdorff distance).  Or (this is the painful truth) it
             * might measure up as a microscopic amount worse, but (we trust)
             * only due to numerical noise.  That is, pri's distance from the
             * ideal, i0, might be a tiny bit bigger than that of the current
             * best guess (p2i), but that difference must be tiny indeed.
             * If it werent for the numerical noise, the left hand value would
             * always be negative or zero.
             */
            ASSERTO( pri.hausdorff_distance( i0 )
                                    - p2i.hausdorff_distance( i0 ) < 1e-6 );

            KJB( ASSERT( p_2.size() == preplacement.size() ) );

            p_2.swap( preplacement );
            p2i.swap( pri ); // must keep this up to date, danger!

            /*
             * Here is the beautiful thing:  iii is still just as valid within
             * preplacement as it is within p_2.
             */
        }
        KJB( ASSERT( p_2.size() == p_1.size() ) );
    }
    return p_2;
}


struct P3
{
    enum { CULL_RED, RED_CULL, RED_RED };
    boost::shared_ptr<kjb::qd::PixPath> gpa, par, self;
    char g_p_s;
};


#if 0
/// @brief cruelly remove least-useful points from p until result is output_sz
PixPath PixPath_compress_2::devil_take_hindmost(
    const PixPath& p,
    size_t output_sz
)
{
    KJB( ASSERT( 2 <= output_sz ) );
    if ( 2 == output_sz )
        return PixPath::fenceposts( p.front(), p.back(), 2 );

    KJB( ASSERT( output_sz <= p.size() ) );
    if ( p.size() == output_sz )
        return p;

    //AMPutil::DebugPrintTee dbt( "compress_2_progress.txt" );
    kjb::File_Ptr_Append c2p( "compress_2_progress.txt" );

    const PixPath pfull( p.interpolate() );

    PixPath pp( p );
    AMPutil::Heartbeat heart( "Path compression", pp.size() - output_sz, 0 );
    //int tencount = 0; //debug

    while( pp.size() > output_sz )
    {
        KJB( ASSERT( 2 < pp.size() ) );
        heart.beat();
        PixPath::const_iterator q = pp.begin() + 1;
        PixPath slimmer_pp( PixPath::reserve() );
        float hd_min = FLT_MAX;
        std::string expelled_point;
        for( size_t iii = 0; iii < pp.size() - 2; ++iii, ++q )
        {
            PixPath candidate( pp.expel( q ) );
            const PixPath canfull( candidate.interpolate() );
            //float hd = p.hausdorff_distance( candidate );
            float hd = pfull.hausdorff_distance( canfull );
            if ( hd < hd_min )
            {
                hd_min = hd;
                slimmer_pp.swap( candidate );
                expelled_point = q -> str();
            }
        }
        KJB( ASSERT( hd_min != FLT_MAX ) );

        /*
        // debug
        std::ostringstream os;
        os << "size " << slimmer_pp.size() << " hausdorff_error " << hd_min
            << " expelled_point " << expelled_point;
        AMPutil::debug_print( os.str() );
        fprintf( c2p, "%s\n", os.str().c_str() );
        // flush the c2p output buffer every ten writes
        if ( 0 == ( tencount = (1 + tencount) % 10 ) )
        {
            int rc = c2p.flush();
            KJB( ASSERT( 0 == rc ) );
        }
        */

        KJB( ASSERT( slimmer_pp.size() == pp.size() - 1 ) );
        pp.swap( slimmer_pp );
    }
    heart.stop();
    return pp;
}
#endif


/**
 * @brief try heuristics to make an improved estimate of a path
 * @return path hopefully even better than input p_1 at approximating p_0_plus.
 *
 * @warning The following function is FULL of kludges.
 */
PixPath PixPath_compress_2::spruce_up(
    const PixPath& p_0_plus,///< original path, maybe wrong size
    const PixPath& p_1      ///< the improved path, the right size
)
{
    try
    {
        return spruce_up_body( p_0_plus, p_1 );
    }
    catch( PixPoint::Unused& )
    {
        kjb_c::p_stderr( "Caught PixPoint::Unused in %s\n", __func__ );
        throw;
    }
}



} // anonymous namespace




namespace kjb {
namespace qd {


PixPath rightsize(
    const PixPath& base_path,
    size_t goal_size,
    float distance_too_close
)
{
    const PixPath_compress_2 p(base_path, goal_size, distance_too_close);
    return p;
}



/**
 * @brief Copy the path, removing adjacent points that are too close together
 *
 * @return path modified so adjacent points are not too close together.
 *
 * This uses a few heuristics to try to make the resulting path have
 * a small Hausdorff distance from the original path.
 * The path might curve back on itself, or even touch itself, but this method
 * does not address that phenomenon.
 */
PixPath PixPath_expander::cull_cozy_points(
    const PixPath& inpath,
    float distance_too_close
)
{
    if ( inpath.size() < 2 )
    {
        return inpath;
    }

    if ( 2 == inpath.size() )
    {
        if ( inpath.front().dist( inpath.back() ) <= distance_too_close )
        {
            KJB_THROW_2( kjb::Runtime_error, "Cannot resize" );
        }
        return inpath;
    }

    const PixPath newpath = cull_squashed_points(
                inpath.cull_redundant_points(),
                distance_too_close
            );

    if ( newpath.size() < 4 )
    {
        return newpath;
    }

    /*
     * There still might be points that are too close together, but now each
     * such point must have breathing room on one side or the other, e.g.,
     * A----------------------b-c-------------D
     * Points b and c are too close but neither is "squashed" (see def. below).
     * Now we sift through the path and decide to kill off either b or c.
     */
    const float dtc2 = distance_too_close * distance_too_close;
    std::vector< bool > skip_me( newpath.size(), false );
    size_t smc = 0; // skip_me cursor, which shadows p below
    size_t keep_count = newpath.size();

    for(const_iterator p = newpath.begin(); p + 1 != newpath.end(); ++smc, ++p)
    {
        KJB( ASSERT( p != newpath.end() ) ); // verify, better late than nvr
        if ( p -> dist2( *( p + 1 ) ) <= dtc2 )
        {
            KJB( ASSERT( 0 < keep_count ) );
            --keep_count;
            float   e1 = newpath.hausdorff_distance( newpath.expel( p ) ),
                    e2 = newpath.hausdorff_distance( newpath.expel( p + 1 ) );
            /*
             * Now we are definitely going to kill one of these two points,
             * preferably the one we will miss less.  However, the endpoints
             * have special immunity from assassination.
             */
            if ( newpath.begin() == p )
            {
                skip_me.at( smc + 1 ) = true;   // p is untouchable so kill p+1
            }
            else if ( newpath.end() == p + 2 )
            {
                skip_me.at( smc ) = true;       // p+1 is untouchable so kill p
            }
            else if ( e1 < e2 )
            {
                skip_me.at( smc ) = true;       // kill p:  it's less useful
            }
            else
            {
                skip_me.at( smc + 1 ) = true;   // kill p+1:  it's less useful
            }
        }
    }

    PixPath newnewpath( PixPath::reserve( keep_count ) );
    const_iterator q = newpath.begin();
    for( size_t smc = 0; smc < skip_me.size(); ++smc, ++q )
    {
        KJB( ASSERT( q != newpath.end() ) );
        if ( ! skip_me[ smc ] )
        {
            newnewpath.push_back( *q );
        }
    }

    return newnewpath;
}


/**
 * A point is SQUASHED if it has two neighbors and both are too close.
 * A squashed point is so close to its neighbors that it cannot tell us
 * very much about the path anyway:  practically all the information in the
 * squashed point is expressed by its two neighbors.  So we don't need it.
 * Note that one or both of the neighbors might also be squashed.
 * This function returns a new path lacking any squashed points.
 * The endpoints cannot possibly be squashed because they don't even have two
 * neighbors.
 *
 * @param inpath input path potentially having squashed points
 * @param distance_too_close distance measure in linear pixel units such that
 *                          two points at this distance are unacceptably close.
 * @return path fairly close to inpath using a Hausdorff measure, but without
 *              squashed points in its sequence.
 *
 * Perhaps "sandwiched" would have been a better image?  Oh well.
 */
PixPath PixPath_expander::cull_squashed_points(
    const PixPath& inpath,
    float distance_too_close
)
{
    const float dtc2 = distance_too_close * distance_too_close;
    std::vector< bool > skip_me( inpath.size(), false );
    const_iterator pre_p = inpath.begin(), p = 1 + pre_p, post_p = 1 + p;
    size_t newpath_size = inpath.size();
    for( int smc = 1; smc < int( skip_me.size() ) - 1;
                                                ++smc, p = post_p, ++post_p )
    {
        KJB( ASSERT( pre_p != inpath.end() && p != inpath.end() ) );
        if ( p -> dist2( *pre_p ) <= dtc2 && p -> dist2( *post_p ) <= dtc2 )
        {
            skip_me[ smc ] = true; // p is squashed by both of its neighbors
            KJB( ASSERT( 0 < newpath_size ) );
            --newpath_size;
        }
        else
        {
            pre_p = p;
        }
    }

    PixPath newpath( PixPath::reserve( newpath_size ) );
    const_iterator q = inpath.begin();
    for( size_t smc = 0; smc < skip_me.size(); ++smc, ++q )
    {
        KJB( ASSERT( q != inpath.end() ) );
        if ( ! skip_me[ smc ] )
        {
            newpath.push_back( *q );
        }
    }

    return newpath;
}


PixPath PixPath_expander::expanded_pp(
    const PixPath& inpath,
    size_t sz,
    float distance_too_close
)
{
    KJB( ASSERT( 2 <= sz ) );

    PixPath inpath_2( cull_cozy_points( inpath, distance_too_close ) );
    const size_t NP2 = inpath_2.size();
    KJB( ASSERT( ! inpath_2.self_intersect() ) );

    if ( sz <= NP2 )
    {
        return inpath_2;            // common case
    }

    // Maybe sz is really big?

    // interpolate inpath_2, except that we forbid self intersection
    PixPath inpath_3( inpath_2.interpolate( true ) );
    KJB( ASSERT( ! inpath_3.self_intersect() ) );

    if ( inpath_3.size() == sz )    // possible although unlikely
    {
        return inpath_3;
    }

    if ( inpath_3.size() < sz ) // also possible, and disastrous
    {
        KJB_THROW_2( kjb::Runtime_error, "Path too short to resize thus" );
    }
    KJB( ASSERT( inpath_3.size() > sz ) ); // common case, given inpath_2 small

    return all_from_list_a_with_some_from_list_b( inpath_2, inpath_3, sz );
}


/**
 * @param path_a essential points for the output
 * @param path_b superset of path_a, augmented by filler points
 * @param sz desired size of the output path
 * @return path of size 'sz' with all points in path_a and the best from path_b
 *
 * All we need to do is return a path with
 * - ALL the points from path a, (the ESSENTIAL POINTS), and
 * - enough of the points from path b that are unique to path b
 *      (the FILLER POINTS),
 *
 * so that the total number of points equals sz.
 *
 * The order is important.  We want a formal subsequence of path_b.
 * Note that path_a is formally a subsequence of path_b.
 * Also, path_a must be a subsequence of the output.
 *
 * Our strategy is to select our filler points at random (without
 * replacement) from the population of potential filler points (the
 * REDUNDANCIES).
 *
 * Actually it would ALSO be nice to choose redundancies that are not
 * too close to essential points.
 */
PixPath PixPath_expander::all_from_list_a_with_some_from_list_b (
    const PixPath& path_a,
    const PixPath& path_b,
    size_t sz
)
{
    KJB( ASSERT( path_a.size() < sz && sz < path_b.size() ) );

    #if 0
        KJB( ASSERT( path_b.has_subsequence( path_a ) ) );

        std::vector< size_t > redundancies;
        redundancies.reserve( path_b.size() );
        KJB( ASSERT( path_a.hits( path_b[0] ) ) );
        KJB( ASSERT( path_a.hits( path_b[ path_b.size() - 1 ] ) ) );
        for( size_t iii = 0; iii < path_b.size(); ++iii )
            if ( ! path_a.hits( path_b[ iii ] ) )
                redundancies.push_back( iii );
        KJB( ASSERT( redundancies.size() < path_b.size() ) );
        KJB( ASSERT( 0 < redundancies.size() ) );
        KJB( ASSERT( sz < path_a.size() + redundancies.size() ) );

        return choose_random_redundancies( path_a, path_b, redundancies, sz );
    #else
        return choose_hermitic_redundancies( path_a, path_b, sz);
    #endif
}


/// @brief randomly select "redundant" points to augment path_a with
PixPath PixPath_expander::choose_random_redundancies(
    const PixPath& path_a,
    const PixPath& path_b,
    std::vector< size_t >& redundancies, // gets clobbered, sorry
    size_t sz
)
{
    /*
     * Take the list of potential filler points, shake well, choose and sort.
     */
    std::random_shuffle( redundancies.begin(), redundancies.end() );
    redundancies.resize( sz - path_a.size() );
    std::sort( redundancies.begin(), redundancies.end() );

    /*
     * Now build the output list like we planned:  all of path a, plus filler.
     */
    PixPath path_out( PixPath::reserve( sz ) );
    size_t rix = 0;
    size_t skip_count = 0;
    for( size_t iii = 0; iii < path_b.size(); ++iii )
    {
        PixPoint pb = path_b[ iii ];
        if ( path_a.hits( pb ) )
        {
            KJB( ASSERT( rix == redundancies.size()
                        || iii != redundancies.at( rix ) ) );
            path_out.push_back( pb );
        }
        else if( rix < redundancies.size() && iii == redundancies.at( rix ) )
        {
            path_out.push_back( pb );
            ++rix;
        }
        else
        {
            ++skip_count;
        }
    }
    KJB( ASSERT( redundancies.size() == rix ) );
    KJB( ASSERT( path_out.size() == sz ) );
    KJB( ASSERT( path_b.size() == path_out.size() + skip_count ) );
    return path_out;
}


/**
 * If you have a list of intervals of various lengths (like city streets),
 * and you want to put points (like bus stops) internal to those intervals,
 * not at the endpoints for some reason, but you insist that the bus stops must
 * be at least 'mingap' apart, then what is the max number of bus stops you can
 * build?  This computes it, along with a little bit of rounding heuristic
 * thrown in.
 * @return max number of bus stops you can build (in above bus stop analogy)
 */
size_t PixPath_expander::insertion_yield(
    const std::vector< float >& pgaps,
    float mingap
)
{
    /*
     * Because floating point values are subject to tiny errors, we have to
     * tweak the given gap values upwards just a bit before we divide; then we
     * use floor to see how many whole mingaps fit inside the gap.
     * Please convince yourself that this procedure it NOT THE SAME AS omitting
     * the tweak and using ceiling instead.  Why?  Consider this_gap = 5.01
     * and mingap = 1.0.
     */
    size_t yield = 0;
    for( std::vector<float>::const_iterator p = pgaps.begin();
                                                    p != pgaps.end(); ++p )
    {
        float this_gap = *p * 1.001;        // tweak it up just a little bit
        int num_whole_mingaps = std::floor( this_gap / mingap );
        yield += std::max( num_whole_mingaps - 1, 0 );
    }
    return yield;
}


/**
 * This is a fancy version of "all from path_a and some from path_b."
 * What makes it fancy?  It is finicky about which points from path_b to
 * select.  It selects points that are relatively far from other selected
 * points -- so the selected points are isolated like hermits, you might say.
 *
 * @return path with all points in path_a, selected "best" from path_b.
 *
 * @pre Points in path_a must be a subsequence of points in path_b
 *
 * @todo This function is unreliable.  "hermit fail 3" has been observed and
 *      when it does, herm.size() ends up smaller than sz at the end = fail.
 *      This can happen when a path defines intersecting segments:  then,
 *      the call to path_b.nearest() can return the same point in two
 *      different segments along a, which causes test herm.hits(pbh) to fail.
 *      A workaround:  you are probably pretty safe if consecutive path_a
 *      points do not define intersecting segments.
 */
PixPath PixPath_expander::choose_hermitic_redundancies(
    const PixPath& path_a,
    const PixPath& path_b,
    size_t sz
)
{
    KJB( ASSERT( sz > path_a.size() ) );
    const size_t deficit = sz - path_a.size();
    KJB( ASSERT( ! path_a.self_intersect() ) );
    KJB( ASSERT( ! path_b.self_intersect() ) );

#if 0
    /*
     * Find all redundant points, that is, points in b that are not in path_a.
     * The hermitic redundancies will be only a subset, determined laboriously.
     */
    PixPath redundancies;
    redundancies.reserve( path_b.size() );
    for const_iterator pa = path_a.begin(), pb = path_b.begin();
                                                pa != path_a.end(); ++pa )
    {
        // next line looks buggy:  should be *pa==*pb, ne pravda li?
        while( pb != path_b.end() && *pa != *pb )
            ++pb;
        KJB( ASSERT( pb != path_b.end() ) ); // assert path_a is subseq of b
        redundancies.push_back( *pb++ );
    }
#else
    KJB( ASSERT( path_b.has_subsequence( path_a ) ) );
#endif

    /*
     * Find the size of gaps between path_a points.
     * Also, find the size of the biggest gap.
     */
    std::vector< float > pgaps_d;
    pgaps_d.reserve( path_a.size() - 1 );
    KJB( ASSERT( 2 <= path_a.size() ) );
    float dmax = -FLT_MAX;
    for ( const_iterator pa = path_a.begin(); pa + 1 != path_a.end(); ++pa )
    {
        float dd = pa -> dist( *( pa + 1 ) );
        pgaps_d.push_back( dd );
        dmax = std::max( dmax, dd );
    }

    /*
     * Try to establish some boundaries on the gaps surrounding redundancies.
     * Compute how many redundancies we could add with those respective bounds.
     */
    float   d_fatgap = dmax / 2.0f,
            d_thingap = dmax / ( 1 + deficit ),
            d_gap;
    size_t  fat_yield = insertion_yield( pgaps_d, d_fatgap ),   // SMALLER num,
            thin_yield =insertion_yield( pgaps_d, d_thingap),   // BIGGER num!
            yield = 0;
    KJB( ASSERT( deficit <= thin_yield ) );
    if ( deficit <= fat_yield )
    {
        d_gap = d_fatgap;
        yield = fat_yield;
    }
    else if ( deficit == thin_yield )
    {
        d_gap = d_thingap;
        yield = thin_yield;
    }
    else
    {
        /*
         * Try to squash the bounds closer together using bisection.
         * We do this SIX TIMES!  For 2**6 times the closeness!!
         * The quantity "six" was chosen by educated guess.
         */
        for( int iii = 0; iii < 6; ++iii )
        {
            d_gap = ( d_fatgap + d_thingap ) / 2.0f;
            yield = insertion_yield( pgaps_d, d_gap );
            KJB( ASSERT( fat_yield <= yield && yield <= thin_yield ) );
            if ( yield < deficit )
            {
                d_fatgap = d_gap;
                fat_yield = yield;
            }
            else if ( deficit < yield )
            {
                d_thingap = d_gap;
                thin_yield = yield;
            }
            else
            {
                KJB( ASSERT( deficit == yield ) );
                break;
            }
        }
        if ( yield < deficit )
        {
            d_gap = d_thingap;
            yield = thin_yield;
        }
    }
    KJB( ASSERT( deficit <= yield ) );

    /*
     * Find the "hermitic" (h) redundancies:  redundancies isolated from others
     * Actually we will intersperse essential points and h-redundancies;
     * Furthermore, we will store the indices of the h-redundancies, since
     * the harvest of h-redundancies might yield more than we need.
     */
    PixPath herm( PixPath::reserve( path_b.size() ) ); // rsv very generously
    std::vector< size_t > hr_indices;   // just a list of indices of h-red's
    hr_indices.reserve( path_b.size() );
    for( const_iterator pa = path_a.begin(); pa + 1 != path_a.end(); ++pa )
    {
        /*
         * As we said, herm stores both essential points and h-redundancies.
         * Here we store one of the essential points.
         */
        herm.push_back( *pa );

        /*
         * Measure the gap between this point and its successor; compare it to
         * the "radius" we computed earlier, stored in d_gap.
         */
        const float this_gap = pa -> dist( *( pa + 1 ) ) * 1.001;// tweak it up
        int whole_hermitage_radii = std::floor( this_gap / d_gap );

        /*
         * Now test whether this gap between the two points along path_a is
         * large enough for a "front yard" and "back yard" (two yards) to
         * isolate the redundant point adequately from its path_a neighbors.
         */
        if ( whole_hermitage_radii < 2 )
        {
            continue;
        }

        /*
         * Looks like we can support at least one hermitic redundancy here.
         * Good!  We must now find the corresponding points along path_b.
         */
        const_iterator pb_fore, pb_aft;
        const PixPoint pbn1 = path_b.nearest( *pa, &pb_fore );
        const PixPoint pbn2 = path_b.nearest( *( pa + 1 ), &pb_aft );
        KJB( ASSERT( *pb_fore == pbn1 && *pb_aft == pbn2 ) );
        KJB( ASSERT( pbn1 == *pa && pbn2 == *( pa + 1 ) ) ); // should be a hit
        for( int hhh = 1; hhh < whole_hermitage_radii; ++hhh )
        {
            float ttt = hhh / float( whole_hermitage_radii );
            KJB( ASSERT( 0 < ttt && ttt < 1 ) );
            // Compute a point along a parametric line connecting pa and pa+1
#if 0
            float goal_x = pa -> x * ( 1 - ttt ) + ( pa + 1 ) -> x * ttt;
            float goal_y = pa -> y * ( 1 - ttt ) + ( pa + 1 ) -> y * ttt;
            PixPoint pbh = path_b.nearest( goal_x, goal_y ); // causes problems
#else
            const kjb::Vector2 da0( to_vector2(*pa) ),
                               da1( to_vector2(*(pa + 1)) );
            const PixPoint pbh( path_b.nearest( da0 * (1 - ttt) + da1 * ttt ));
#endif
            if ( pbh == *pa )
            {
                TEST_PSE(( "hermit fail 1\n" ));
                continue;
            }
            else if ( pbh == *( pa + 1 ) )
            {
                TEST_PSE(( "hermit fail 2\n" ));
                continue;
            }
            else if ( herm.hits( pbh ) )
            {
                TEST_PSE(( "hermit fail 3\n" ));
                continue;
            }
            else
            {
                hr_indices.push_back( herm.size() );
                herm.push_back( pbh );
            }
        }
    }
    KJB( ASSERT( ! herm.hits( path_a.back() ) ) );
    herm.push_back( path_a.back() );
    KJB( ASSERT( herm.has_subsequence( path_a ) ) );

    KJB( ASSERT( sz <= herm.size() ) );

    if ( sz == herm.size() )
    {
        return herm;            // unlikely scenario
    }

    return choose_random_redundancies( path_a, herm, hr_indices, sz );
}


/**
 * @brief this is an error metric for a path bridging from index iii to jjj
 * @param path a PixPath interpreted as a polygonal path of at least two points
 * @param iii index of a path element that is one vertex of a bridge
 * @param jjj index of a distinct path element that is one vertex of a bridge
 * @return error metric, the sum of squares of perpendicular distances
 *
 * This function computes an error metric for a polygonal path.  What we are
 * comparing is to replace a section of path between points of index iii and
 * jjj, by eliminating all vertices strictly between those two vertices.
 * Because we interpret the reduced path as a polygonal path, we assume there
 * is a line segment between path[iii] and path[jjj].
 * For each such vertex v between those two that we eliminate, we compute
 * the square of the perpendicular distance between v and the nearest point on
 * the line through path[iii] and path[jjj].  This function sums all those
 * squared distances and returns the sum.
 *
 * This function conforms to the signature of typedef fp_path_sum_err.
 */
float sosq_error( const PixPath& path, size_t iii, size_t jjj )
{
    if ( jjj < iii )
    {
        std::swap( iii, jjj );
    }

    float sum = 0;
    const PixPoint pi( path[ iii ] ), pj( path[ jjj ] );
    for( size_t mmm = iii + 1; mmm < jjj; ++mmm )
    {
        sum += perp_dist_sq( pi, pj, path[ mmm ] );
    }
    return sum;
}



PixPath polyline_approx(
    const PixPath& base_path,
    size_t goal_size,
    float* error
)
{
#if 0
/*
 * Formerly this used to take an additional parameter, a pointer to an error
 * function to be used.  We no longer accept that as a parameter since we now
 * use a functor with the error function built in that also memoizes previous
 * results of the function; and parameterizing THAT would not be as efficient.
 * So here is the now-unused doxygen documentation for that former parameter:
 *
 * @param errfun additive error function for subranges of the path, such that
 *        (*errfun)(i,j) represents the term of the error measure when we use a
 *        straight-line segment from base_path[i] to base_path[j] and throw
 *        away all points strictly between those two points.  Note that we
 *        assume the total error for any approximation can be computed by
 *        summing *errfun values from start to each point included in the
 *        approximation.
 */

    // Dispense with the nothing-to-do case:
    if ( base_path.size() <= goal_size )
    {
        if ( error ) *error = 0;
        return base_path;
    }

    // initialize functor that we will use to compute error terms
    kjb::qd::Memo_sosq_error se( base_path );

    /*
     * allocate table used to store subsolutions
     *
     * Table definition:  mesa[i][j].e contains the error of an optimal
     * i+2-vertex polygonal approximation of base_path[j] to base_path.back().
     *
     * The table is going to be sort of triangular, so we won't allocate the
     * whole thing at once.  Instead we will push on rows as we compute them.
     * We will allocate row-0 (the longest) now though.
     */
    std::vector< std::vector< PerVid > > mesa( goal_size - 2 );
    mesa[ 0 ].resize( base_path.size() - 1 );

    /*
     * First row corresponds to a 2-vertex polygonal approximation, i.e., a
     * line segment, thus the table's error value is just given by the error
     * function.
     */
    for( size_t aaa = 0; aaa < base_path.size() - 1; ++aaa )
    {
        mesa[ 0 ].at( aaa ) = PerVid( se( aaa, base_path.size() - 1 ),
                                                        base_path.size() - 1 );
    }

    /*
     * Subsequent rows require a minimization.
     */
    for( size_t bbb = 1; bbb < goal_size - 2; ++bbb )
    {
        const size_t EndA = base_path.size() - bbb - 1;
        mesa.at( bbb ).resize( EndA );
        for( size_t aaa = 0; aaa < EndA; ++aaa )
        {
            /*
             * index arithmetic here is confusing but I think I've got it
             * right; you can access column EndA of the previous row because
             * the previous row is one cell longer than the current row.
             */
            for( size_t jjj = aaa + 1; jjj <= EndA; ++jjj )
            {
                float eee = mesa.at( bbb - 1 ).at( jjj ).e + se( aaa, jjj );
                if ( eee < mesa.at( bbb ).at( aaa ).e )
                {
                    mesa.at( bbb ).at( aaa ) = PerVid( eee, jjj );
                }
            }
        }
    }

    /*
     * Total solution requires one more minimization for aaa equal to 0.  (We
     * do not need to try multiple aaa's because we know we will take index 0.)
     */
    PerVid final;
    const size_t Enda = base_path.size() - ( goal_size - 2 ) - 1;
    for( size_t jjj = 1; jjj <= Enda; ++jjj )
    {
        float eee = mesa.at( goal_size - 3 ).at( jjj ).e + se( 0, jjj );
        if ( eee < final.e )
        {
            final = PerVid( eee, jjj );
        }
    }

    /*
     * Now extract the error and the path from the completed table.
     */
    PixPath answer( PixPath::reserve( goal_size ) );
    answer.push_back( base_path.front() );
    answer.push_back( base_path[ final.ix ] );
    for( int bbb = goal_size - 3; 0 <= bbb; --bbb )
    {
        const PerVid& next = mesa.at( bbb ).at( final.ix );
        final.e += next.e;
        answer.push_back( base_path[ final.ix = next.ix ] );
    }

    if ( error )
    {
        *error = final.e;
    }

#else
    PolylineApprox polax(base_path);
    PixPath answer(polax.approx(goal_size));
    if (error) *error = polax.get_last_error();

#endif
    return answer;
}



/**
 * @brief Remove "unnecessary" pixels, using the Perez-Vidal algorithm
 * @param pixelpath Path of pixels to be reduced
 * @return path possibly reduced from this one
 *
 * Input should be a path that is regarded more as pixels than as polygon
 * vertices, i.e., the points should be "small" atomic blocks and you should
 * not be relying on sub-PixPoint resolution.
 *
 * This function sometimes does a better job than the obsolete
 * PixPath::cull_redundant_points(); other times it does a worse job.
 *
 * This function sort of does the opposite of interpolate.  It returns a path
 * p such that interpolate() and p.interpolate() should return equivalent
 * paths.  Note that the input is not necessarily 8-connected.
 * This method does not always reduce the number of points to the minimum.
 *
 * The fundamental flaw in this version is that it assumes if an optimal
 * reduction to size M fails to yield equivalency to the interpolated value,
 * then reduction to any size x < M must similarly so fail.  Untrue.
 * Consider the path (0,0); (1,1); (2,1); (3,2).  A reduction to 2 points will
 * interpolate to the original, but a reduction to points (0,0);(2,1);(3,2)
 * will not, since that interpolates to (0,0);(1,0);(2,1);(3,2).
 * Consequently, the binary search in this function can skip over the optimal
 * solution.  This is merely heuristic.  Also, it often works pretty well.
 */
PixPath reduce_pixels_pv(const PixPath& pixelpath)
{
    const PixPath IPP(pixelpath.interpolate());
    PixPath better(pixelpath);

    PolylineApprox pax(pixelpath);
    //std::cerr <<"initialize, pmax,pmin ="<< pixelpath.size() <<",2\n";
    for (size_t pmin = 2, pmax = pixelpath.size(); pmin + 1 < pmax; )
    {
        size_t ptry = (pmin + pmax) / 2;
        PixPath candidate = pax.approx(ptry);
        if (candidate.interpolate() == IPP)
        {
            pmax = ptry;
            //std::cerr <<"reduce top, pmax,pmin ="<< pmax <<","<< pmin <<'\n';
            better.swap(candidate);
        }
        else
        {
            pmin = ptry;
            //std::cerr <<"increase b, pmax,pmin ="<< pmax <<","<< pmin <<'\n';
        }
    }

    return better;
}


/**
 * @brief another heuristic to reduce pixels, using previous two methods + BFS
 * @param pixelpath path of pixels to be reduced
 * @return smaller path, at least as good as the best of the other two.
 * @see PixPath::cull_redundant_points
 * @see reduce_pixels_pv
 *
 * This method uses the two functions mentioned previously, plus breadth first
 * search (BFS) and searches all combinations of their outputs.  Again, the
 * result should satisfy the invariant that its output of interpolate() equals
 * pixelpath.interpolate().  Also, this method should find the very best
 * that cull and reduce pv can ever do.  They are still heuristics, so this too
 * is heuristic, and another heuristic might do even better.
 *
 * The tree under consideration grows in size as the Fibonacci series, as the
 * levels increase, since the cull
 * function can only be (meaningfully) followed by the reduce function, but
 * reduce can be followed by cull or by another invocation of reduce.
 * The tree is never going to grow very large, though.
 */
PixPath reduce_pixels_bfs(const PixPath& pixelpath)
{
    boost::shared_ptr<PixPath> best;
    std::deque<P3> q;

    // red = abbreviation for "a call to reduce_pixels_pv
    // cull = abbreviation for "a call to cull_redundant_points"

    // first try a cull red combo
    P3 cr1;
    cr1.gpa.reset(new PixPath(pixelpath));
    cr1.par.reset(new PixPath(pixelpath.cull_redundant_points()));
    cr1.self.reset(new PixPath(reduce_pixels_pv(*cr1.par.get())));
    cr1.g_p_s = P3::CULL_RED; // cull first (grandparent to parent), red second
    if (cr1.self -> size() < cr1.gpa -> size()) q.push_back(cr1);
    best = cr1.self;

    // next try a red cull combo
    P3 rc1 = q.back();
    rc1.gpa = cr1.gpa;
    rc1.par.reset(new PixPath(reduce_pixels_pv(pixelpath)));
    rc1.self.reset(new PixPath(rc1.par -> cull_redundant_points()));
    rc1.g_p_s = P3::RED_CULL; // red first, cull second
    if (rc1.self -> size() < rc1.gpa -> size()) q.push_back(rc1);
    if (rc1.self -> size() < best -> size()) best = rc1.self;

    // maybe try a red red combo
    if (rc1.par -> size() < rc1.gpa -> size())
    {
        P3 rr1 = q.back();
        rr1.gpa = rc1.gpa;
        rr1.par = rc1.par;
        rr1.self.reset(new PixPath(reduce_pixels_pv(*rc1.par)));
        rc1.g_p_s = P3::RED_RED; // red first, red second
        if (rr1.self -> size() < rr1.par -> size()) q.push_back(rr1);
        if (rr1.self -> size() < best -> size()) best = rr1.self;
    }

    for (P3 er, ec; !q.empty(); q.pop_front())
    {
        const P3& e = q.front();
        er.gpa = ec.gpa = e.par;
        er.par = ec.par = e.self;
        er.g_p_s = P3::RED_CULL == e.g_p_s ? P3::CULL_RED : P3::RED_RED;
        ec.g_p_s = P3::RED_CULL;

        switch (e.g_p_s)
        {
            case P3::CULL_RED:
                ec.self.reset(new PixPath(ec.par -> cull_redundant_points()));
                if (ec.self -> size() < ec.gpa -> size()) q.push_back(ec);
                if (ec.self -> size() < best -> size()) best = ec.self;
                if (er.par -> size() >= er.gpa -> size()) break;
                // else, fall through . . .
            case P3::RED_CULL:
                er.self.reset(new PixPath(reduce_pixels_pv(*er.par)));
                if (er.self -> size() < er.par -> size()) q.push_back(er);
                if (er.self -> size() < best -> size()) best = er.self;
                break;
            case P3::RED_RED:
                ec.self.reset(new PixPath(ec.par -> cull_redundant_points()));
                if (ec.self -> size() < ec.gpa -> size()) q.push_back(ec);
                if (ec.self -> size() < best -> size()) best = ec.self;

                if (er.par -> size() < er.gpa -> size())
                {
                    er.self.reset(new PixPath(reduce_pixels_pv(*er.par)));
                    if (er.self -> size() < er.par -> size()) q.push_back(er);
                    if (er.self -> size() < best -> size()) best = er.self;
                }
                break;
            default:
                KJB_THROW(Cant_happen);
        }
    }

    return *best;
}


}
}


