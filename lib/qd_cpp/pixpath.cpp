/**
 * @file
 * @brief Implementation of the PixPath class
 * @author Andrew Predoehl
 */

/*
 * $Id: pixpath.cpp 21596 2017-07-30 23:33:36Z kobus $
 */


#include "l/l_sys_debug.h"  /* For ASSERT */
#include "l/l_sys_io.h"
#include "m_cpp/m_vector.h"     /* see PixPath::cubic_fit      */
#include "m_cpp/m_matrix.h"     /* see PixPath::cubic_fit      */
#include "curv/curv_lib.h"      /* see PixPath::cubic_fit      */
#include "l_cpp/l_exception.h"  /* THROWn stuff                */
#include "l_cpp/l_stdio_wrap.h" /* File_Ptr_Write, _Smart_Read */
#include "qd_cpp/pixpath.h"

using kjb::qd::PixPoint;
using kjb::qd::PixPath;

namespace
{




inline
bool const_iterator_less_x(
    const PixPath::const_iterator& p1,
    const PixPath::const_iterator& p2
)
{
    return p1 -> x  <  p2 -> x;
}



// Assume input pointers are not equal to null, and vector has positive size,
// but we don't assume the map is accurate.  Test; if not, throw.
// Otherwise, eliminate the last element from the path.
template< typename VT, typename MT >
void back_pop_and_validate( VT* vec, MT* map )
{
    typename MT::iterator iii = map -> find( vec -> back() );

    if ( map -> end() == iii || iii -> second < 1 )
    {
        KJB_THROW_2(kjb::Cant_happen, "Internal corruption in PixPath::ppmap");
    }

    vec -> pop_back();
    iii -> second -= 1; // decrease map count by one
}

bool are_both_nonzero_but_opposite_signs( double u, double v )
{
    if ( u < 0 ) return 0 < v;
    return v < 0 && 0 < u;
}



} // end anonymous ns


namespace kjb
{
namespace qd
{


/**
 * @brief Distance-squared between a PixPath's two termini.
 *
 * This struct stores the distance squared between two paths' beginnings,
 * ends, and respective begin-end and end-begin.
 */
struct PixPath::D2_endpoints {
    float   bb, ///< Distance squared between both paths' beginnings
            be, ///< Distance squared between path 1's begin, path 2's end
            eb, ///< Distance squared between path 1's end, path 2's begin
            ee; ///< Distance squared between both paths' ends.

    /// @brief vacuous class thrown if either path is empty in ctor
    class bad_args {};

    /// @brief ctor to compare endpoint distances of two paths
    D2_endpoints( const PixPath& v1, const PixPath& v2 )
    {
        if ( 0 == v1.size() || 0 == v2.size() )
        {
            throw bad_args();
        }
        bb = v1.front().dist2( v2.front() );
        be = v1.front().dist2( v2.back()  );
        eb = v1.back() .dist2( v2.front() );
        ee = v1.back() .dist2( v2.back()  );
    }

    /// @brief Test whether some value is no larger than bb,ee,be and eb.
    bool min( float xx ) const
    {
        return xx <= bb && xx <= be && xx <= eb && xx <= ee;
    }

    /// @brief Return true if the bb distance-sq. is (or ties) the minimum
    bool min_bb() const
    {
        return min(bb);
    }

    /// @brief Return true if the be distance-sq. is (or ties) the minimum
    bool min_be() const
    {
        return min(be);
    }

    /// @brief Return true if the eb distance-sq. is (or ties) the minimum
    bool min_eb() const
    {
        return min(eb);
    }

    /// @brief Return true if the ee distance-sq. is (or ties) the minimum
    bool min_ee() const
    {
        return min(ee);
    }
};


PixPath PixPath::operator+( const PixPath& other ) const
{
    if( size() != other.size() )
    {
        KJB_THROW_2(Dimension_mismatch, "Size mismatch in PixPath::operator+");
    }

    PixPath result( /*reserve*/ size() );

    for(const_iterator j1=begin(), j2=other.begin(); j1 != end(); ++j1, ++j2 )
    {
        PixPoint sum( *j1 + *j2 );
        result.push_back( sum );
    }
    return result;
}


PixPath& PixPath::append( const_iterator begin, const_iterator end )
{
    while ( begin != end )
    {
        push_back( *begin++ );
    }
    return *this;
}


PixPath& PixPath::append( const PixPath& suffix )
{
    // bizarre self-appending case
    if ( this == &suffix )
    {
        PixPath autocopy( *this );
        return append( autocopy );
    }

    // common case
    my_vpp.reserve( size() + suffix.size() );
    return append( suffix.begin(), suffix.end() );
}


int PixPath::append_no_overlap( const PixPath& suffix )
{
    // bizarre self-appending case
    if ( this == &suffix )
    {
        PixPath autocopy( *this );
        return append_no_overlap( autocopy );
    }

    if ( 0 == size() || 0 == suffix.size() )
    {
        kjb_c::set_error( "Empty input" );
        return kjb_c::ERROR;
    }
    if ( back() != suffix.front() )
    {
        kjb_c::set_error( "Mismatch in input" );
        return kjb_c::ERROR;
    }

    append( suffix.begin()+1, suffix.end() );
    return kjb_c::NO_ERROR;
}


/**
 * @brief Returns a subrange of this PixPath object
 *
 * @param index_first   We omit all points with indices less than this value.
 * @param index_postlast    We omit all points with indices greater than or
 *                          equal to this value.
 *
 * @return return path equivalent to a subrange of this path.
 *
 * The indices are indicated from
 * the first index (zero-based of course) to one-past-the-last desired index.
 *
 * The above description is phrased to imply that this routine does not ever
 * fail:  it is equivalent if index_postlast equals size() or size() + 1000.
 * If index_first is huge, or
 * if index_postlast <= index_first, the result is simply an empty PixPath.
 *
 * @warning Notice that the second argument is NOT the length!  If you want a
 *          subrange of path P starting at index i and of length L, you can
 *          compute it like so:  \code P.subrange( i, i + L ); \endcode
 */
PixPath PixPath::subrange( unsigned index_first, unsigned index_postlast )const
{
    PixPath range;
    index_postlast = std::min( (size_t) index_postlast, size() );
    if ( index_first < index_postlast )
    {
        range.my_vpp.reserve( index_postlast - index_first );
        for( unsigned iii = index_first; iii < index_postlast; ++iii )
        {
            range.push_back( my_vpp[ iii ] );
        }
    }
    return range;
}



/**
 * @brief ctor builds a sequence of points as a Bresenham straight line.
 *
 * The term "Bresenham straight line" is meant to indicate that the
 * sequence of points will have no gaps with respect to 8-way grid
 * connectivity.
 *
 * This is my transposition of Scott Morris's implementation of the
 * Bresenham line algorithm, from seg to PixPath.
 *
 * This constructs the path as a straight line joining point a to point b (both
 * of which are included), connected via 8-connectivity.
 * If a==b then the result is a one-point path.
 *
 * @post front() is defined and equals a
 * @post back() is defined and equals b
 * @post size() is at least 1
 * @throws PixPoint::Unused if a or b is unused
 *
 * @param a First point of path (passed by value because it serves as a cursor)
 * @param b Last point of path.  It must not be unused.
 */
PixPath bresenham_line( const PixPoint& ca, const PixPoint& b )
{
    if ( ca.is_unused() || b.is_unused() )
    {
        KJB_THROW_2( PixPoint::Unused, "Bresenham builder bad input" );
    }

    PixPath result( PixPath::reserve( 1 + ca.L_infinity_distance( b ) ) );

    result.push_back( ca );

    /* PixPoint a is used as a local variable, a cursor from start to end.
     */
    PixPoint a( ca ); // for neatness, I wanted both params to be const refs.
    const PixPoint::Integer deltax = ABS_OF( b.x - a.x ),
                            deltay = ABS_OF( b.y - a.y ),
                            dx = 2 * deltax,
                            dy = 2 * deltay,
                            stepx = b.x < a.x  ?  -1  :  +1, // west or east?
                            stepy = b.y < a.y  ?  -1  :  +1; // north or south?

    PixPoint::Integer fraction = dx > dy ? dy - deltax : dx - deltay;

    KJB( ASSERT( 0 <= dx && 0 <= dy ) );
    KJB( ASSERT( stepx * dx == 2 * (b.x - a.x) ) );
    KJB( ASSERT( stepy * dy == 2 * (b.y - a.y) ) );

    if ( deltax > deltay )
    {
        while( a.x != b.x )
        {
            if ( 0 <= fraction )
            {
                a.y += stepy;
                fraction -= dx;
            }
            a.x += stepx;
            fraction += dy;
            result.push_back( a );
        }
    }
    else
    {
        while( a.y != b.y )
        {
            if ( 0 <= fraction )
            {
                a.x += stepx;
                fraction -= dy;
            }
            a.y += stepy;
            fraction += dx;
            result.push_back( a );
        }
    }
    KJB( ASSERT( a == b ) );

    return result;
}


// Does this path contain a self-intersection?  Just curious.
bool PixPath::self_intersect( PixPoint* where ) const
{
    for( PPMap::const_iterator qqq = ppmap.begin(); qqq != ppmap.end(); ++qqq )
    {
        if ( 1 < qqq -> second )
        {
            if ( where ) *where = qqq -> first;
            return true;
        }
    }
    return false;
}


bool PixPath::connected8( bool emit_db_output ) const
{
    if ( size() < 2 )
    {
        return true;
    }

    if ( emit_db_output )
    {
        for( size_t iii = 1; iii < size(); ++iii )
        {
            const PixPoint& me = my_vpp[ iii ], &prev = my_vpp [ iii - 1 ];
            float d2 = prev.dist2( me );
            std::ostringstream dbs;
            dbs << "index " << iii << ", "
                   "d2 " << d2 << " to last pixel; "
                   "self = " << me.str() << "   "
                   "previous = " << prev.str() << "\n";
            kjb_c::pso( "%s", dbs.str().c_str() );
            if ( d2 > 2 ) break;
        }
    }

    for( size_t iii = 1; iii < size(); ++iii )
    {
        if ( my_vpp[ iii - 1 ].dist2( my_vpp[ iii ] ) > 2 )
        {
            return false;
        }
    }

    return true;
}



int PixPath::arclength( std::vector<float>* alvec ) const
{
    KJB( NRE( alvec ) );
    alvec -> clear();

    if ( 0 == size() )
    {
        kjb_c::set_error( "Null path has undefined arclength." );
        return kjb_c::ERROR;
    }

    alvec -> reserve( size() );
    alvec -> push_back( 0 );
    // inchworm through the path, accumulating straight-line distances
    for (const_iterator p_back=begin(), p=p_back+1; p != end(); p_back=p, ++p)
    {
        alvec -> push_back( alvec -> back() + p -> dist( *p_back ) );
    }
    return kjb_c::NO_ERROR;
}


std::string PixPath::str( const std::string& separator ) const
{
    std::string result;
    for( size_t iii = 0; iii < size(); ++iii )
    {
        result += my_vpp[ iii ].str() + separator;
    }
    return result;
}


int PixPath::save( const std::string& filename ) const
{
    try
    {
        kjb::File_Ptr_Write fpw( filename );
        KJB( ERE( kjb_c::kjb_fputs( fpw, str().c_str() ) ) );
    }
    catch( kjb::IO_error& )
    {
        return kjb_c::ERROR;
    }
    return kjb_c::NO_ERROR;
}


// Ctor to load a PixPath from a file (ASCII format)
// Note this does not use str_to_PixPoint, but maybe it should.
PixPath::PixPath( const std::string& filename )
{
    kjb::File_Ptr_Smart_Read fi( filename );
    int ct = 0;
    /* Don't use a float here, use a double, since our regression tests use
     * random integers that exercise all 32 bits; and a float only has about
     * 23 bits of mantissa, so a float here will cause a failure.
     */
    for( double xx, yy; 0 < (ct = fscanf( fi, "%le %le\n", &xx, &yy )); )
    {
        /*
         * ct might be 1 or 2, anything else freaks us out.
         */
        if ( ct != 1 && ct != 2 )
        {
            KJB_THROW_2(IO_error, "IO error reading from file (bad scan)" );
        }
        /*
         * If we only read 1 number, probably that's because there was a comma.
         */
        if ( 1 == ct )
        {
            ct = fscanf( fi, ", %le\n", &yy );
            if ( ct != 1 )
            {
                KJB_THROW_2(IO_error, "IO error reading from file (sep)" );
            }
        }
        int ix = int(xx), iy = int(yy);
        push_back( PixPoint( ix, iy ) );
        KJB( ASSERT( double(ix) == xx ) );
        KJB( ASSERT( double(iy) == yy ) );
    }
    KJB( ASSERT( 0 == ct || EOF == ct ) );
}


/* Ctor parses a string of coords.  Int arg does nothing except differentiates
 * this signature from that of the ctor that reads from a named file.
 * This ignores ignores the letter L and M and ignores commas in the input
 * string.
 */
PixPath::PixPath( const std::string& coords, int )
{
    std::string inp( coords ); // we need an editable copy

    // Take off every L and M and comma, for great justice.
    // Comma is perhaps self-explanatory; L and M are based on SVG path syntax
    for( size_t iii = 0;
            ( iii = inp.find_first_of( "LM,", iii ) ) != std::string::npos; )
    {
        inp[ iii ] = ' ';
    }

    std::istringstream iss( inp );
    for( int xxx, yyy; iss >> xxx >> yyy; )
    {
        push_back( PixPoint( xxx, yyy ) );
    }

    if ( ! iss.eof() )
    {
        KJB_THROW_2(Serialization_error, "Error parsing SVG-style input");
    }
}



PixPoint PixPath::boundbox_min_min() const
{
    if ( 0 == size() )
    {
        KJB_THROW_2(Illegal_argument, "Too small for bounding box min" );
    }

    PixPoint minmin = front();
    for( const_iterator p = begin(); p != end(); ++p )
    {
        minmin.x = std::min( minmin.x, p -> x );
        minmin.y = std::min( minmin.y, p -> y );
    }
    return minmin;
}


PixPoint PixPath::boundbox_max_max() const
{
    if ( 0 == size() )
    {
        KJB_THROW_2(Illegal_argument, "Too small for bounding box max" );
    }

    PixPoint maxmax = front();
    for( const_iterator p = begin(); p != end(); ++p )
    {
        maxmax.x = std::max( maxmax.x, p -> x );
        maxmax.y = std::max( maxmax.y, p -> y );
    }
    return maxmax;
}



/**
 * @brief Return the PixPath member closest to a given query point
 *
 * @param[in]   pp  Query point
 * @param[out]  qq  (Optional) pointer to const_iterator that, if non-nil on
 *                  input, is
 *                  set to indicate the nearest member of PixPath to pp.
 *                  If more than one member hits pp, this returns the iterator
 *                  pointing to the member nearest the front of the list.
 * @return      a PixPoint member of the PixPath such that no other member of
 *              the PixPath is closer to the query point pp.
 * @pre         The PixPath must contain at least one point
 *
 * If the PixPath is empty, the results are formally undefined, but informally
 *  consist of an error message and an unused PixPoint.
 * This simple implementation performs a linear scan, which means it is slow.
 * A fancy implementation could maybe be much faster.
 *
 * @todo (a repeat of an earlier point) replace map with multimap so we could
 *      find indices of hitpoints in log time.
 */
PixPoint PixPath::nearest( const PixPoint& pp, const_iterator* qq ) const
{
    if ( 0 == size() )
    {
        KJB_THROW_2(Illegal_argument, "Too small for nearest" );
    }

    /* linear scan for closest point
     * KEY:  q denotes an iterator, p denotes a PixPoint, d2 is a scalar
     */
    const_iterator near_q = my_vpp.begin();
    float near_d2 = near_q -> dist2( pp );
    for (const_iterator new_q=1+my_vpp.begin(); new_q != my_vpp.end(); ++new_q)
    {
        float new_d2 = new_q -> dist2( pp );
        if ( new_d2 < near_d2 )
        {
            near_q = new_q;
            near_d2 = new_d2;
        }
    }
    if ( qq )
    {
        *qq = near_q;
    }
    return *near_q;
}


/// @brief Return the PixPath member closest to the given real xy coordinates
PixPoint PixPath::nearest( const kjb::Vector2& xy, const_iterator* qq ) const
{
    typedef Vector::Value_type DPVT;

    if ( 0 == size() )
    {
        KJB_THROW_2(Illegal_argument, "Too small for nearest" );
    }

    /* linear scan for closest point
     * KEY:  q denotes an iterator, p denotes a PixPoint, d2 is a scalar
     */
    const_iterator near_q = end();
    DPVT near_d2 = std::numeric_limits< DPVT >::max();
    for (const_iterator new_q = my_vpp.begin(); new_q != my_vpp.end(); ++new_q)
    {
        const kjb::Vector2 dif( xy - to_vector2(*new_q) );
        DPVT new_d2 = dif.magnitude_squared();
        if ( new_d2 < near_d2 )
        {
            near_q = new_q;
            near_d2 = new_d2;
        }
    }
    KJB( ASSERT( near_q != end() ) );
    if ( qq )
    {
        *qq = near_q;
    }
    return *near_q;
}




PixPath& PixPath::ow_reverse()
{
    VecPP newvpp( my_vpp.rbegin(), my_vpp.rend() );
    my_vpp.swap( newvpp );
    return *this;
}




/**
 * This fits a cubic polynomial to the vector of points.  The coeffients are
 * returned in \p x and \p y (which must both be non-nil).
 * The polynomial fit for the x points is thus
 * cubic_x(t) = (*x)[0] + t (*x)[1] + t^2 (*x)[2] + t*3 (*x)[3].
 * Similarly for y.
 * The values of t are the Euclidean distances along the polygonal path,
 * i.e., the shortest possible distances between points.  Note that the
 * interpolation will not in general fall exactly on these points
 * The point corresponding to t = 0 is by default the first point, but it may
 * be shifted to any of the points in the PixPath by providing the index in
 * parameter \p ref (which must be valid or ERROR is returned).
 * The fit is effected using the Moore-Penrose pseudoinverse, which assumes
 * Gaussian errors.  This can spell trouble if your errors are not Gaussian.
 *
 * @return kjb_c::ERROR or NO_ERROR indicating outcome of computation.
 *
 * @throws Too_small if this path has fewer than 4 points (which means the
 *          system is underdetermined).
 */
int PixPath::cubic_fit(
    std::vector< float >* x,
    std::vector< float >* y,
    int ref
) const
{
    if ( size() < 4 )
    {
        KJB_THROW_2(Illegal_argument, "Too small for cubic fit" );
    }

    if ( 00 == x || 00 == y || ref < 0 || int( size() ) <= ref )
    {
        kjb_c::set_error( "Empty input in cubic_fit()" );
        return kjb_c::ERROR;
    }

    const size_t DIMS = 2;      // a PixPoint is a two-dimensional entity
    const int CUBE = 3;         // a third-degree polynomial . . .
    const int WIDTH = 1+CUBE;   // . . . has four coefficients.

    /*
     * Build a matrix of target values.
     */
    kjb::Matrix xy( size(), DIMS );
    for( size_t rrr = 0; rrr < size(); ++rrr )
    {
        xy( rrr, 0 ) = my_vpp[ rrr ].x;
        xy( rrr, 1 ) = my_vpp[ rrr ].y;
    }

#if 0
    /*
     * Build a possibly overdetermined Vandermonde matrix to fit a cubic poly
     */
    kjb::Matrix van( size(), WIDTH, 1.0 );

    /*
     * Row 0 is a special case; set it to [1 0 0 0].  The 1 is already there.
     */
    for( int ccc = 1; ccc < WIDTH; ++ccc )
        van( 0, ccc ) = 0;

    /*
     * All subsequent rows get set to [1 t t^2 t^3],
     * where t is the polygonal distance from first pixel to current pixel.
     */
    for( int rrr = 1; rrr < size(); ++rrr )
    {
        const PixPoint& ppp = my_vpp[ rrr ];
        const PixPoint& predecessor = my_vpp[ rrr-1 ];
        const double position = van( rrr-1, 1 ) + ppp.dist( predecessor );
        for( int ccc = 1; ccc < WIDTH; ++ccc )
            van( rrr, ccc ) = position * van( rrr, ccc-1 );
    }

    kjb::Matrix mpi_van = van.get_pseudoinverse();

    /*
     * Compute the "best" coefficients, assuming our errors are Gaussian (ha).
     */
    kjb::Matrix fit = mpi_van * xy;

#else
    kjb::Vector t( size() );
    t[0] = 0;
    for( size_t iii = 1; iii < size(); ++iii )
    {
        const PixPoint& ppp = my_vpp[ iii ];
        const PixPoint& predecessor = my_vpp[ iii-1 ];
        t[ iii ] = t[ iii-1 ] + ppp.dist( predecessor );
    }
    if ( 0 < ref )
    {
        double tref = t[ ref ];
        for( size_t iii = 0; iii < size(); ++iii )
        {
            t[ iii ] -= tref;
        }
    }

    double err;
    kjb_c::Matrix* c_mp = 00;
    KJB( ERE( fit_parametric_cubic_known_time(
                    t.get_c_vector(), xy.get_c_matrix(), 00, &c_mp, &err ) ) );
    kjb::Matrix fit( c_mp );
#endif

    KJB( ASSERT( fit.get_num_rows() == WIDTH ) );
    KJB( ASSERT( fit.get_num_cols() == int( DIMS ) ) );

    /*
     * Stuff the coefficients into the output vectors.
     */
    x -> resize( WIDTH );
    y -> resize( WIDTH );

    for( int rrr = 0; rrr < WIDTH; ++rrr )
    {
        x -> at( rrr ) = fit( rrr, 0 );
        y -> at( rrr ) = fit( rrr, 1 );
    }

    return kjb_c::NO_ERROR;
}


bool PixPath::operator==( const PixPath& other ) const
{
    if ( size() != other.size() )
    {
        return false;
    }
    for (const_iterator p = begin(), q = other.begin();
                                            p != end() && q != other.end(); )
    {
        if ( *p++ != *q++ )
        {
            return false;
        }
    }
    return true;
}


/**
 * @brief This interpolates pixels between unconnected pixels
 *
 * This function uses the Bresenham line drawing algorithm to "draw"
 * intermediate pixels between non-adjacent pixels on the path.
 * The result is a PixPath that is potentially much longer.
 * Points are only added, not removed, not even duplicates.
 *
 * @param forbid_self_intersection  Optional.  If omitted or false, the output
 * will be fully 8-connected.
 * If true, the output path will not contain self-intersections.
 * The output will contain all the points of this path, in the same sequence,
 * and as many interpolated pixels as possible to make it 8-connected,
 * except when that would cause self-intersection.
 * I think it best not to document the
 * sequential order of omitted would-be interpolated pixels -- it might change.
 * This method will run a wee bit slower if the parameter is true.
 *
 * @throws Self_intersection iff parameter forbid_self_intersection is true and
 *                              this path already has a self-intersection.
 *
 * @return a new path hitting points "between" original points.
 * @post This path will always be a subsequence of the output.
 */
PixPath PixPath::interpolate( bool forbid_self_intersection ) const
{
    if ( size() < 2 )
    {
        return *this;
    }

    if ( forbid_self_intersection && self_intersect() )
    {
        KJB_THROW_2(Illegal_argument, "Self intersection in interpolate" );
    }

    PixPath fatter;
    std::vector< bool > originals;  // stores pedigree of each point in fatter

    // insert the first point
    fatter.push_back( my_vpp[ 0 ] );
    originals.push_back( true );

    // insert the non-first points, and we can always look backwards
    for ( const_iterator q = begin() + 1; q != end(); ++q )
    {
        KJB( ASSERT(    originals.size()==fatter.size()
                    ||  !forbid_self_intersection       ) );
        if ( ( q - 1 ) -> adjacent8( *q ) )
        {
            fatter.push_back( *q );
            originals.push_back( true );
        }
        else
        {
            const PixPath newseg( bresenham_line( *( q - 1 ), *q ) );
            ETX( fatter.append_no_overlap( newseg ) );
            for( size_t iii = 2; // for the two original endpoints of newseg
                    forbid_self_intersection && iii < newseg.size(); ++iii )
            {
                originals.push_back( false );
            }
            originals.push_back( true );
        }
    }
    KJB( ASSERT( fatter.connected8() ) );

    // If output must not have self intersection, clean up the mess (ick)
    if ( forbid_self_intersection && fatter.self_intersect() )
    {
        PixPath fat2( /*reserve*/ fatter.size() );
        KJB( ASSERT( originals.size() == fatter.size() ) );
        const_iterator pfat1 = fatter.begin();
        std::vector< bool >::const_iterator por = originals.begin();
        for( ; por != originals.end(); ++por, ++pfat1 )
        {
            if ( *por || 1 == fatter.hits( *pfat1 ) ) // original or innocent
            {
                fat2.push_back( *pfat1 );
            }
            else if ( 0 == hits( *pfat1 ) && 0 == fat2.hits( *pfat1 ) )
            {
                fat2.push_back( *pfat1 );
            }
        }
        KJB( ASSERT( ! fat2.self_intersect() ) );
        fatter.swap( fat2 );
    }

    KJB( ASSERT( fatter.has_subsequence( *this ) ) );
    return fatter;
}



/**
 * @brief Remove "unnecessary" points (ones that can be bresenham interpolated)
 * @return path possibly reduced from this one
 * @see merge_redundant_slopes.
 * @see reduce_pixels_pv
 *
 * This function sort of does the opposite of interpolate.  It returns a path
 * p such that interpolate() and p.interpolate() should return equivalent
 * paths.  Note that the input is not necessarily 8-connected.
 * This method does not always reduce the number of points to the minimum.
 * Sometimes the function reduce_pixels_pv() works better, other times worse.
 *
 * The criterion for "redundant" is limited to PixPoint resolution; if you
 * render (or do whatever) with resolution finer than a PixPoint, the output
 * will not be equivalent to the input.
 * For example, PixPoint(0,0) PixPoint(1,1) PixPoint(2,1) PixPoint(3,2)
 * would under certain circumstances be culled to its endpoints, but if your
 * use exploits the fact of the slope of
 * +1 between the first and last pair,
 * the results of this function will be unsatisfactory (having culled too
 * much).  For a more conservative culling algorithm, try
 * merge_redundant_slopes().
 *
 * @code
 * Bresenham result keeps pixels 0, 3 (discards 1, 2).
 * If all you can show are blocky pixels, then it is fine to drop 1 and 2.
 * .........###....
 * .........#3#....
 * .........###....
 * ...######.......
 * ...#1##2#.......
 * ...######.......
 * ###.............
 * #0#.............
 * ###.............
 * @endcode
 *
 * @code
 * If you have sub-PixPoint resolution you will not get an equivalent path.
 * If you drop pixels 1, 2 then connecting 0 to 3 won't look the same.
 * ................
 * ..........3.....
 * ........./......
 * ......../.......
 * ....1--2........
 * .../............
 * ../.............
 * .0..............
 * ................
 * @endcode
 */
PixPath PixPath::cull_redundant_points() const
{
    if ( size() < 3 )
    {
        return *this;
    }

    PixPath thinner;
    const_iterator  q_past = begin(),
                    q_present = q_past + 1,
                    q_future = q_present + 1;
    thinner.push_back( *q_past );
    for( ; q_future != end() ; ++q_future )
    {
        KJB( ASSERT( q_past != end() ) );
        KJB( ASSERT( q_present != end() ) );

        PixPath fat( bresenham_line( *q_past, *q_present ) );
        ETX( fat.append_no_overlap( bresenham_line( *q_present, *q_future )) );
        PixPath thin( bresenham_line( *q_past, *q_future ) );
        if ( fat != thin )
        {
            thinner.push_back( *q_present );
            q_past = q_present;
        }
        q_present = q_future;
    }
    KJB( ASSERT( back() != thinner.back() ) );
    thinner.push_back( back() );
    KJB( ASSERT( thinner.size() <= size() ) );

    /* tail recursion */
    return thinner.size() == size() ? *this : thinner.cull_redundant_points();
}


PixPath PixPath::expel( const_iterator bad_point ) const
{
    PixPath abridged( /*reserve*/ size() );
    for( const_iterator iii = begin(); iii != end(); ++iii )
    {
        if ( iii != bad_point )
        {
            abridged.push_back( *iii );
        }
    }
    return abridged;
}


/**
 * @brief helper function for computing Hausdorff distance between two objects
 *
 * @param other a path for which each point is associated with the nearest
 *              point in "this" path; then we return the size of the maximum
 *              in all those pairings.
 *
 * @return max { dist(p,q) | p in other, q = argmin { dist(p,v) | v in this } }
 *
 * Compute one side of the Hausdorff distance:  if "this" path has a crazy
 * outlier point at coordinates (BIG1,BIG2) far from everyone, but the rest
 * of "that" path lies close to "this," the one-sided computation of the
 * instant method still might be a small number, since each point in that
 * path might have a near neighbor in this one.  Nevertheless, the true
 * Hausdorff distance *ought to* be a large number, because of the outlier.
 * To get the real HD, call this method twice, switching the positions of
 * this and that path, and return the larger value.  You *must* do this if
 * you want a result that is, topologically, a valid distance metric, since
 * any topologist will tell you a valid metric d satisfies d(x,y)=d(y,x).
 */
float PixPath::hausdorff_dist_1side( const PixPath& other ) const
{
    if ( this == &other )
    {
        return 0;
    }

    const float IMPOSSIBLE = -1.0f;
    float hd2 = IMPOSSIBLE; // squared hausdorff distance register
    for( const_iterator q = other.begin(); q != other.end(); ++q )
    {
        hd2 = std::max( hd2, nearest( *q ).dist2( *q ) );
    }
    KJB( ASSERT( hd2 != IMPOSSIBLE ) );
    return sqrt( hd2 );
}



float PixPath::closest_adjacent_pair( const_iterator* pa ) const
{
    if ( size() < 2 )
    {
        KJB_THROW_2(Illegal_argument, "Too small to have a pair" );
    }

    const_iterator qmin = begin();
    KJB( ASSERT( qmin + 1 != end() ) );
    float d2min = std::numeric_limits< float >::max();
    for( const_iterator qc = begin(); qc + 1 != end(); ++qc )
    {
        float d2 = qc -> dist2( *( qc + 1 ) );
        if ( d2 < d2min )
        {
            qmin = qc;
            d2min = d2;
        }
    }

    if ( pa != 0 )
    {
        *pa = qmin;
    }

    KJB( ASSERT( qmin != end() && qmin + 1 != end() ) );
    return qmin -> dist( *( qmin + 1 ) );
}


bool PixPath::has_subsequence( const PixPath& subseq ) const
{
    if ( size() < subseq.size() )
    {
        return false;
    }
    if ( size() == subseq.size() )
    {
        return operator==( subseq );
    }

    for (const_iterator p=begin(), q=subseq.begin(); q!=subseq.end(); ++p, ++q)
    {
        while( p != end() && *p != *q )
        {
            ++p;
        }
        if ( p == end() )
        {
            return false;
        }
    }
    return true;
}


/**
 * @brief Compute included angle at some interior point in a path
 *
 * @param index Array index of the interior point where to do the computation.
 *
 * The angle we are talking about is the included angle between two line
 * segments defined by a central (interior) point, and the central point's
 * prececessor and successor points.  You specify the index of the central
 * point:  it must not be the front or back point, because those points lack
 * a predecessor and successor point, respectively.
 *
 * This does not tell you whether the angle is clockwise or counterclockwise.
 * Consider using bracket_cross_at() if you need to know.
 *
 * @return Angle, in radians, in range [0, M_PI].  For a straight piece of
 *          trail, the angle will be close to M_PI.
 *
 * @pre index must be an interior point, not equal to zero or size()-1.
 * @pre The points at index + 1 and index - 1 must be distinct from the point
 *      at index.
 *
 * @throws Degenerate_triangle if the interior point is equal to its
 * predecessor or successor point:  the angle is undefined in that case, of
 * course.
 *
 * @throws std::out_of_range if the specified index is not that of an interior
 * point
 *
 * @section pp_aa_impl Implementation
 *
 * The basic idea is that for vectors @f$ p, q @f$ the dot product
 * @f$ p \cdot q @f$ satisfies
 * @f[
 *           p \cdot q = \vert p \vert \cdot \vert q \vert \cos \theta
 * @f]
 * where @f$ \theta @f$ is the included angle.
 * So all we have to do is compute @f$ p @f$ and @f$ q @f$ at the given
 * interior point, then use a little algebra.
 */
double PixPath::angle_at( unsigned index ) const
{
    PixPoint o( my_vpp.at( index ) ),
             p( my_vpp.at( index+1 ) - o ),
             q( my_vpp.at( index-1 ) - o );

    if ( PixPoint( 0, 0 ) == p || PixPoint( 0, 0 ) == q )
    {
        KJB_THROW_2(Illegal_argument, "Degenerate triangle" );
    }

    const kjb::Vector2 dp( to_vector2(p) ), dq( to_vector2(q) );
    double cosine = kjb::dot(dp, dq) / dp.magnitude() / dq.magnitude();

    // The cosine value must be clamped -- empirically proved to be necessary
    return acos( std::min( std::max( cosine, -1.0 ), +1.0 ) );
}



bool PixPath::intersect_at_with(
    unsigned preindex1,
    unsigned preindex2,
    unsigned* endpoint_intersector_index
)   const
{
    const PixPoint  &p1 = my_vpp.at( preindex1      ),
                    &p2 = my_vpp.at( preindex1 + 1  ),
                    &p3 = my_vpp.at( preindex2      ),
                    &p4 = my_vpp.at( preindex2 + 1  );

    const PixPoint::Integer left12 = std::min( p1.x, p2.x ),
                            right12 = std::max( p1.x, p2.x ),
                            btm12 = std::min( p1.y, p2.y ),
                            top12 = std::max( p1.y, p2.y ),

                            left34 = std::min( p3.x, p4.x ),
                            right34 = std::max( p3.x, p4.x ),
                            btm34 = std::min( p3.y, p4.y ),
                            top34 = std::max( p3.y, p4.y );

    // Quick bounding-box check for miss
    if (    right34 < left12
        ||  right12 < left34
        ||  top12 < btm34
        ||  top34 < btm12
       )
    {
        return false; // bounding boxes are disjoint, so segments must be too.
    }

    const PixPoint  v34 = p4 - p3,  // direction of segment from 3 to 4
                    v12 = p2 - p1,
                    v13 = p3 - p1,
                    v32 = p2 - p3,
                    v14 = p4 - p1;

    PixPoint::Integer   c341 = v13.cross( v34 ) * -1,   // 3-4-1 clockwise?
                        c342 = v32.cross( v34 ),        // 3-4-2 clockwise?
                        c123 = v13.cross( v12 ),        // 1-2-3 clockwise?
                        c124 = v14.cross( v12 );        // 1-2-4 clockwise?

    // does segment 12 straddle LINE 34 ?
    bool straddle34 = ( 0 < c341 && c342 < 0 )   ||   ( c341 < 0 && 0 < c342 ),

    // does segment 34 straddle LINE 12 ?
         straddle12 = ( 0 < c123 && c124 < 0 )   ||   ( c123 < 0 && 0 < c124 );

    if ( straddle12 && straddle34 )
    {
        return true;    // they share a common interior point
    }

    /*
     * Now check if either endpoint intersects the segments
     */
    if (        0 == c341
            &&  left34 <= p1.x && p1.x <= right34
            &&  btm34 <= p1.y && p1.y <= top34
        )
    {
        // endpoint p1 intersects segment 34
        if ( endpoint_intersector_index )
        {
            *endpoint_intersector_index = preindex1;
        }
        return true;
    }

    if (        0 == c342
            &&  left34 <= p2.x && p2.x <= right34
            &&  btm34 <= p2.y && p2.y <= top34
        )
    {
        // endpoint p2 intersects segment 34
        if ( endpoint_intersector_index )
        {
            *endpoint_intersector_index = preindex1 + 1;
        }
        return true;
    }

    if (        0 == c123
            &&  left12 <= p3.x && p3.x <= right12
            &&  btm12 <= p3.y && p3.y <= top12
        )
    {
        // endpoint p3 intersects segment 12
        if ( endpoint_intersector_index )
        {
            *endpoint_intersector_index = preindex2;
        }
        return true;
    }

    if (        0 == c124
            &&  left12 <= p4.x && p4.x <= right12
            &&  btm12 <= p4.y && p4.y <= top12
        )
    {
        // endpoint p4 intersects segment 12
        if ( endpoint_intersector_index )
        {
            *endpoint_intersector_index = preindex2 + 1;
        }
        return true;
    }

    return false;
}


bool PixPath::do_segments_intersect(
    unsigned* s1,
    unsigned* s2,
    unsigned* count
) const
{
    if ( count )
    {
        *count = 0;
    }

    if ( size() < 3 )
    {
        return false;
    }

    for( size_t iii = 0; iii < size() - 2; ++iii )
    {
        if ( 0 == bracket_cross_at( 1 + iii ) )
        {
            // p1,p2,p3 are collinear, so test whether the angle is zero or pi
            const PixPoint  &p1 = my_vpp.at( iii ),
                            &p2 = my_vpp.at( iii +  1 ),
                            &p3 = my_vpp.at( iii +  2 );

            // dot product is positive iff ray p2p1 identical to ray p2p3
            if ( 0 <= ( p1 - p2 ).idot( p3 - p2 ) )
            {
                if ( s1 && s2 )
                {
                    *s1 = iii;
                    *s2 = 1 + iii;
                }

                if ( 00 == count )
                {
                    return true;
                }
                *count += 1;
            }
        }

        for( size_t jjj = 2 + iii; jjj < size() - 1; ++jjj )
        {
            if ( intersect_at_with( iii, jjj ) )
            {
                if ( s1 && s2 )
                {
                    *s1 = iii;
                    *s2 = jjj;
                }

                if ( 00 == count )
                {
                    return true;
                }
                *count += 1;
            }
        }
    }

    // at first glance the following might look superfluous, but note the star.
    return count ? 0 < *count : false;
}




unsigned PixPath::longest_segment( float* length ) const
{
    if ( size() < 2 )
    {
        KJB_THROW_2(Illegal_argument, "Too small, no segments" );
    }
    const_iterator argmax = end();
    float max2 = -1.0f;
    for (const_iterator ppp2=begin(), ppp1=ppp2++; ppp2 != end(); ppp1=ppp2++)
    {
        float d2 = ppp1 -> dist2( * ppp2 );
        if ( max2 < d2 )
        {
            argmax = ppp1;
            max2 = d2;
        }
    }
    KJB( ASSERT( argmax + 1 < end() ) );

    if ( length )
    {
        *length = sqrt( max2 );
    }

    return argmax - begin();
}





PixPath::PixPath( const PixPoint& pa, const PixPoint& pb, size_t sz )
:   my_vpp( sz, pa )
{
    if ( sz <= 2 ) // if size is 0, 1, or 2
    {
        if ( 1 <= sz ) // if size is 1 or 2
        {
            if ( 2 == sz )
            {
                my_vpp[ 1 ] = pb;
                ppmap[ pb ] = 1;
            }
            ppmap[ pa ] = 1;
        }
        return;                     // finished with size 0, 1, 2
    }

    my_vpp[ sz - 1 ] = pb;
    ppmap[ pa ] = 1;
    ppmap[ pb ] = 1;
    const kjb::Vector2 da( to_vector2(pa) ), delta( to_vector2(pb - pa) );
    const_iterator ctor_end = my_vpp.end() - 1;
    size_t ctr = 0;
    double normf( 1.0 / ( sz - 1 ) );
    for( VecPP::iterator iii = my_vpp.begin() + 1; iii != ctor_end; )
    {
        PixPoint newpt( reckless_round( da + delta * ( ++ctr * normf ) ) );
        *iii++ = newpt;
        ppmap[ newpt ] += 1;
    }
}


kjb::Vector2 PixPath::centroid() const
{
    if ( 0 == size() )
    {
        KJB_THROW_2(Illegal_argument, "Too small, no centroid" );
    }
    // compute sum of points or throw if one of the points is bad
    kjb::Vector2 centr(0, 0);
    for( const_iterator ppp = begin(); ppp != end(); ++ppp )
    {
        if ( ppp -> is_unused() )
        {
            KJB_THROW_2( PixPoint::Unused, "bad input" );
        }
        centr += kjb::Vector2(ppp->x, ppp->y);
    }
    return centr / size();
}


unsigned PixPath::member_near_centroid() const
{
    const_iterator nearp = end();
    nearest( centroid(), & nearp );
    KJB( ASSERT( nearp != end() ) );
    return nearp - begin();
}


unsigned PixPath::hits( const PixPath& query_path ) const
{
    unsigned total = 0;
    for( const_iterator ppp = query_path.begin(); ppp != query_path.end(); )
    {
        total += hits( *ppp++ );
    }
    return total;
}



/**
 * @brief return a path with interior points removed but preserve all slopes.
 * @see cull_redundant_points
 * @return a path no larger and with unchanged line segment slopes.
 *
 * We view the path as a polygonal path.  If interior point ppp has predecessor
 * nnn and successor qqq.  If ppp lies exactly on the closed line segment
 * (nnn,qqq) then ppp is regarded as redundant and can be removed.  This method
 * returns a path will all such entries removed.  It is suitable for
 * subpixel rendering schemes.
 *
 * In many circumstances, this method is too conservative, and the potential
 * user really wants to cull more aggressively such as provided by
 * cull_redundant_points().  If your rendering of PixPoints only has pixel
 * resolution, you probably want that method.
 *
 * Implementation notes:
 *
 * Throw out *q_present if vecs dpresent, dfuture have same direction.
 * "Same direction" means collinear and arrows pointing the same way.
 * "Collinear" means zero cross product of dpresent, dfuture.
 * "Arrows pointing same way" means dpresent, dfuture have non-negative
 * dot product (i.e., positive or zero).
 * Cross and dot products are zero if *q_present equals *q_past or
 * *q_present.
 * Thus we KEEP *q_present if the cross product is nonzero or if dot
 * product is negative.  This is confusing, isn't it?  Let's summarize.
 *
 * <table>
 * <tr><td></td>    <td>Case 1</td>  <td>Case 2</td>   <td>Case 3</td>
 * </tr><tr>
 * <td>Cross-product of dpresent, dfuture</td>
 *                      <td>nonzero</td><td>zero</td><td>zero</td></tr>
 * <tr><td>Dot-product of dpresent, dfuture</td>
 *      <td>(any)</td><td>negative</td><td>zero or positive</td></tr>
 * <tr><td>Action on *q_present</td>
 *                      <td>keep</td><td>keep</td><td>drop</td></tr>
 * <tr><td>Conclusion about *q_present</td>
 *  <td>It is the elbow of an ordinary bend.
 *      Dot product truly could be pos, neg, or zero.</td>
 *  <td>It is not between *q_past, *q_future, despite being collinear.
 *      In fact *q_future must lie on the line segment *q_past, *q_present,
 *      although worrying about that fact is someone else's job, not this
 *      method's.</td>
 *  <td>*q_present is redundant.</td></tr>
 * </table>
 */
PixPath PixPath::merge_redundant_slopes() const
{
    if ( size() < 3 )
    {
        return *this;
    }

    PixPath thinner;
    const_iterator q_past = begin();
    const_iterator q_present = q_past + 1;
    const_iterator q_future = q_present + 1;
    PixPoint dpresent = *q_present - *q_past;
    thinner.push_back( *q_past );

    for( ; q_future != end() ; q_present = q_future++ )
    {
        KJB( ASSERT( q_past != end() ) );
        KJB( ASSERT( q_present != end() ) );
        PixPoint dfuture = *q_future - *q_present;

        // This test seems confusing:  see the above Doxygen/HTML chart.
        if ( dfuture.cross( dpresent ) != 0 || dfuture.idot( dpresent ) < 0 )
        {
            // keep value of q_present, store it in q_past
            thinner.push_back( *q_present );
            q_past = q_present;
            dpresent = dfuture;
        }
        else
        {
            dpresent = *q_future - *q_past; // eliminate all trace of q_present
        }
    }
    KJB( ASSERT( back() != thinner.back() ) );
    thinner.push_back( back() );
    return thinner;
}


/**
 * @brief Find the distance between closest pair of points, ignoring adjacency
 * @return distance between closest pair of points in the set of points
 * @param[out] pa   If this and pa are set to a non-null location, then at exit
 *                  *pa will hold an iterator pointing to one of the pair of
 *                  closest points.
 * @param[out] pb   Like pa, but if assigned, *pb will hold an iterator
 *                  pointing to the other point in the closest pair.
 * @throws Too_small if the object does not contain a pair of points (or if
 *                  all, or all but one, are unused).
 * @see PixPoint::is_unused()
 * @see PixPath::closest_adjacent_pair()
 *
 * This returns the distance between the closest pair of distinct entries in
 * the object, provided that the object contains at least two entries that are
 * not unused.  "Distinct entries" means that the entries must be stored in
 * distinct indices of this object (indices differ by at least one); it does
 * not mean that the values differ.  Thus, zero is a valid return value, and it
 * occurs when the object contains two copies of the same point.
 *
 * If pa is omitted or pb is omitted, or either equals null, then neither will
 * be assigned.
 *
 * This algorithm takes time O(N log N) where N is the object size.
 * It is described in Chapter 15 of Goodrich & Tamassia's book,
 * <em>Data Structures and Algorithms in Java</em>, John Wiley, 1998.
 *
 * Note again this method ignores adjacency (i.e., where in the list the two
 * points are contained).  If you want to constrain the answer so that the two
 * points are successive entries in the list, use
 * PixPath::closest_adjacent_pair().
 */
float PixPath::closest_pair( const_iterator* pa, const_iterator* pb ) const
{
    // Set up x_array: it holds iterators to all points, ordered L to R.
    std::vector< PixPath::const_iterator > x_array;
    x_array.reserve( size() );
    for( const_iterator qq = begin(); qq != end(); ++qq )
    {
        if ( qq -> is_not_unused() )
        {
            x_array.push_back( qq );
        }
    }
    if ( x_array.size() < 2 )
    {
        KJB_THROW_2(Illegal_argument, "Too small, no pair" );
    }
    std::sort(x_array.begin(), x_array.end(), const_iterator_less_x);

    /*
     * Set up band:  it holds a vertically ordered strip, always infinitely
     * tall, and as wide as the distance between closest points.  After this
     * initialization, it can be thought of as (temporarily) infinitely wide,
     * a semi-open plane containing the leftmost point.  As the main loop
     * proceeds, the band will monotonically narrow, remaining infinitely tall.
     * (Obviously this "infinity" talk is mostly moonshine -- there are only a
     * finite number of points to look at, after all.)
     *
     * The main loop below starts with iii==1 because after this
     * initialization, we will have already inserted the x_array[0] point.
     */
    typedef std::multimap< PixPoint::Integer, PixPath::const_iterator > Band;
    Band band;
    band.insert( std::make_pair( x_array.front() -> y, x_array.front() ) );

    // Main loop:  Traverse x_array and retain the closest pair.
    float min_dist = std::numeric_limits< float >::max();
    for ( size_t iii = 1; iii < x_array.size(); ++iii ) // don't start at 0
    {
        // Invariant 1:  band holds at least 1 entry, corresponding to *(qq-1).
        KJB( ASSERT( ! band.empty() ) );
        /*
         * Invariant 2:  qq is the iterator of the new point to be added to
         * band.  It might be half of the pair we seek!  We will check.
         */
        const PixPath::const_iterator& qq = x_array[ iii ];

        // Shift the left edge of band towards *qq, possibly emptying band.
        while ( ! band.empty() && band.begin()->second->x < qq->x - min_dist )
        {
            band.erase( band.begin() );
        }

        // Search within band, in the min-distance-radius of qq.
        for( Band::const_iterator rr = band.lower_bound( qq->y - min_dist );
                rr != band.end() && rr -> first < qq -> y + min_dist; ++rr )
        {
            const float qr_dist = qq -> dist( *( rr -> second ) );
            if ( qr_dist < min_dist )
            {
                min_dist = qr_dist;
                if ( pa && pb )
                {
                    *pa = rr -> second;
                    *pb = qq;
                }
            }
        }

        // insert the new point
        band.insert( std::make_pair( qq -> y, qq ) );
    }
    return min_dist;
}


/**
 * @brief scan a string value into a PixPoint
 * @param iss   stream that next contains some representation of a PixPoint
 * @param sep   optional separator string; default is a comma, "," between.
 * @return PixPoint constructed from value, or UNUSED PixPoint if failure.
 *
 * If separator string 'sep' is empty then we assume the two values are
 * separated only by whitespace.  Ditto if the first character of 'sep' is
 * whitespace.  Otherwise we pull characters out of the input and expect
 * them to match to successive characters of 'sep.'
 */
PixPoint str_to_PixPoint( std::istream& iss, const std::string& sep )
{
    PixPoint::Integer x, y;
    // read x value
    if (!( iss >> x ))
    {
        return PixPoint(); // unused, because x value was absent
    }
    // read separator
    if ( sep.size() && ! std::isspace( sep[ 0 ] ) )
    {
        char ccc;
        size_t iii = 0;
        while( iii < sep.size() && iss >> ccc && ccc == sep[ iii ] )
        {
            ++iii;
        }
        if ( iii != sep.size() )
        {
            return PixPoint(); // unused, because separator was absent
        }
    }
    if (!( iss >> y ))
    {
        return PixPoint(); // unused, because y value was absent
    }
    return PixPoint( x, y );
}


float PixPath::arclength() const
{
    std::vector< float > al;
    ETX( arclength( &al ) );
    return al.back();
}



/**
 * @brief apppend first to second, without duplicating aaa.back(), bbb.front()
 * @return concatenation of two paths, either sum of sizes or one less.
 * @param aaa first path in chain -- output will have contents of aaa as prefix
 * @param bbb last path in chain -- output will have contents of bbb as suffix
 *
 * Unlike PixPath::append(), which chains two paths regardless of overlap, and
 * unline PixPath::append_no_overlap(), which requires two paths to overlap,
 * this routine returns a new path and tries not to overlap aaa.back() with
 * bbb.front(), but does not make a precondition as to whether they are equal.
 * If aaa.back() and bbb.front() have distinct values, this does as append().
 * Otherwise, this does as append_no_overlap().
 *
 * Implementation note:  the first parameter is deliberately pass-by-value.
 */
PixPath append_trying_not_to_overlap( PixPath aaa, const PixPath& bbb )
{
    if ( 0 == aaa.size() ) return bbb;
    if ( 0 == bbb.size() ) return aaa;
    return aaa.append(bbb.begin() + (aaa.back()==bbb.front()?1:0), bbb.end() );
}

void PixPath::pop_back()
{
    if ( 0 == size() )
    {
        KJB_THROW_2(Illegal_argument, "Tried to pop from empty PixPath" );
    }
    back_pop_and_validate( &my_vpp, &ppmap );
}


/**
 * @brief return just a basic SVG path element with just a d attribute.
 *
 * @returns something like "<path d='M 0,0 L 1,2 3,4' />" unless degenerate.
 *
 * A path with zero or one pixpoint will elicit an empty string or a
 * rather vacuous result.
 */
std::string svg_path(const PixPath& path)
{
	std::ostringstream p;
	p << "<path d='M";
	size_t count = 0;
	for (PixPath::const_iterator i = path.begin(); i != path.end(); ++i)
	{
		if (1 == count) p << " L"; 
		p << ' ' << i -> str(",");
		++count;
		if (0 == count % 10) p << '\n';
	}
	p << "'/>\n";
	return p.str();
}


} // end namespace qd
} // end namespace kjb

