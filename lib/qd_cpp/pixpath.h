/**
 * @file
 * @brief Contains definition for class PixPath
 * @author Andrew Predoehl
 */
/*
 * $Id: pixpath.h 20532 2016-03-12 00:54:48Z predoehl $
 */

#ifndef PIXPATH_H_UOFARIZONAVISION
#define PIXPATH_H_UOFARIZONAVISION 1

#include <l/l_error.h>
#include <l/l_global.h>
#include <l_cpp/l_util.h>
#include <m_cpp/m_vector_d.h>
#include <i_cpp/i_pixel.h>
#include <qd_cpp/pixpoint.h>

#include <limits>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>


namespace kjb
{
namespace qd
{





/**
 * @brief Representation of a sequence of pixel coordinates
 *
 * The following class started out as a simple typedef of a vector of PixPoint
 * objects, but then it got a bit fancier.
 * The essential characteristics are as follows:
 * - Integer coordinates
 * - Sequential organization:  this is a directed path
 * - Supports sequence operations including:
 *  - string and file IO
 *  - concatenation
 *  - subsequences
 *  - reversal
 *  - forward and reverse iterators
 *  - push_back, pop_back
 * - Supports geometric operations:
 *  - Test whether a query point is struck; if so, how often?
 *  - Test connectivity
 *  - Test self-intersection (duplicated points in sequence)
 *  - Test whether line segments cross
 *  - Find nearest path point to a query point
 *  - Find closest pair of points (not implemented yet, actually)
 *  - Find closest pair of adjacent points (linear time)
 *  - Arclength
 *  - Hausdorff distance
 *  - Interior angles
 *  - Path bending clockwise, counterclockwise, or neither
 *  - @ref path_bbox
 *  - Estimated curvature using a cubic polynomial fit
 *  - Interpolation a la Bresenham
 *  - Removal of "redundant" points
 *
 * @todo replace map with a multimap containing the indices along the path that
 * hit each point.
 *
 * @section path_bbox Bounding Boxes
 *
 * To test whether a path lies entirely within a given axis-aligned bounding
 * box, there are two ways to do it and they both take linear time.
 * Suppose we have a bounding box defined by two PixPoints, min_min and
 * width_height.  As the names suggest, min_min.x is the minimum valid
 * X-coordinate, and min_min.y is the minimum valid Y-coordinate.  The width
 * of the box is width_height.x pixels, and the height is width_height.y
 * pixels; which means, for example, that any point with X-coordinate of
 * min_min.x + width_height.x is just out of bounds.  Suppose the path we want
 * to test is called "path."
 *
 * @subsection pathbb_ppath Testing using PixPath methods
 *
 * PixPath lets you test containment of two paths' minimal, axis-aligned
 * bounding boxes:
 * @code
 *  PixPath bb;
 *  bb.push_back( min_min );
 *  bb.push_back( min_min + width_height - PixPoint(1,1) );
 *  if ( width_height.is_poz_poz() && path.boundbox_within_boundbox( bb ) )
 *      std::cout << "path stays inside bounding box\n";
 *  else
 *      std::cout << "path leaves bounding box somewhere\n";
 * @endcode
 * We subtract PixPoint(1,1) in the third line because we need the upper right
 * maximum inbounds point, and min_min+width_height is out of bounds.
 * But then we must use method PixPoint::is_poz_poz() in case
 * the width or height is less than 1.
 *
 * @subsection pathbb_ppoint Testing using PixPoint functor
 *
 * We can use functor PixPoint::Is_inbounds to look for an out-of-bounds point:
 * @code
 *  PixPath::const_iterator q = std::find_if( path.begin(), path.end(),
 *          std::not1( PixPoint::Is_inbounds( min_min, width_height ) ) ) );
 *  if ( path.end() == q )
 *      std::cout << "path stays inside bounding box\n";
 *  else
 *      std::cout <<"path leaves bounding box when it hits "<< q->str() <<'\n';
 * @endcode
 *
 * This approach obliges you to include headers
 * &lt;algorithm> and &lt;functional>.
 */
class PixPath
{
    /**
     * @brief Basic sequential relationship between points is represented by
     *          this vector type.
     */
    typedef std::vector< PixPoint > VecPP;

    /// @brief storage for points in the path (the heart of the class).
    VecPP my_vpp;


    typedef std::map< PixPoint, unsigned > PPMap;   ///< type used for ppmap


    /**
     * @brief Pointwise self-intersection is represented with a map.
     *
     * There are two "intersection" concepts that are relevant:
     * - Points
     * - Segments
     *
     * An intersection of points occurs when two entries in the PixPath contain
     * the same value.  This concept is easily accessed by storing a map
     * keyed on the PixPoints, counting the number of locations in the path
     * that has the key value.  If the map contains a value of 2 or more
     * anywhere, we say the path has a self-intersection.
     *
     * To say that again:
     * Map storage tells us whether the path has self-intersection:  key is
     * the pixel coordinates (as a PixPoint), satellite data is the number of
     * times that pixel coordinate is present in the path.  A typical valid
     * path has a map with all points associated with value 1.  Zero never
     * should occur.  Greater than 1 means a self-intersection.
     *
     * If you regard a PixPath as a polygonal path, where the path entries are
     * vertices, but the points on segments btw. vertices are also thought of
     * as part of the path, then you might also care about when segments
     * intersect with each other.  Unfortunately, the map does not help to
     * answer that question.
     *
     * @see self_intersect
     * @see do_segments_intersect
     */
    PPMap ppmap;

    // A private class used for some ugly skulduggery
    struct D2_endpoints;


    float hausdorff_dist_1side( const PixPath& ) const;

#if 0
    /**
     * @brief increase size of path by 1 by adding a new point on longest seg
     *
     * This probably is only valuable if you think of the object as a model of
     * a polygonal path, of which the entries of my_vpp are just vertices.
     * What this does is it tries to add a new vertex into the path
     * by splitting the longest segment in two, with the new vertex in the
     * middle.  But another strong constraint is that the new vertex must be
     * distinct from every existing vertex.
     *
     * Example:  path is (0,0), (2,4) when this starts.  At exit, the path
     * will be (0,0), (1,2), (2,4).
     *
     * @return EXIT_SUCCESS on success.  Otherwise this return EXIT_FAILURE
     * if such a vertex cannot be added.  That happens when
     * the segments are already too short (length less than 2 pixels) or if
     * the path is too crammed with vertices to add a new suitable one.
     *
     * @warning Due to rounding errors, the new vertex might not be exactly on
     * the old longest segment.  So this introduces some distortion.
     */
    int add_a_point_along_longest_segment();

    int untangle_recursive( int depth );    // disabled bc it is sick
#endif

protected:
    /// @brief Default ctor starts as empty, and you can push points onto it.
    PixPath() {}

    /// @brief ctor receives the size to reserve
    PixPath( size_t potential_size )
    {
        my_vpp.reserve( potential_size );
    }

    /// @brief ctor to load from an ASCII file
    PixPath( const std::string& );

    /// @brief ctor makes evenly spaced points between termini
    PixPath( const PixPoint&, const PixPoint&, size_t );

    /// @brief ctor that parses a string of characters
    PixPath( const std::string&, int );

public:
    // default copy ctor is just fine, and is (properly) public.

    /// @brief Reference to an element -- required to use std::back_inserter().
    typedef const PixPoint& const_reference;

    /// @brief Element type -- required to use std::back_inserter() in c++11.
    typedef PixPoint value_type;

#if 0
    /// @brief Vacuous class thrown if an error occurs
    struct Some_error : public kjb::Exception
    {
        Some_error( const std::string& m, const char* f, int l )
        :   kjb::Exception( m, f, l )
        {}
    };
#endif

    /// @brief constant iterator through PixPath points
    typedef VecPP::const_iterator const_iterator;

    /// @brief constant reverse iterator through PixPath points
    typedef VecPP::const_reverse_iterator const_reverse_iterator;

    /// @brief obligatory dtor
    virtual ~PixPath() {}

    /// @brief named ctor creates an empty path but reserves some memory for it
    static PixPath reserve( size_t potential_size = 0 )
    {
        return PixPath( potential_size );
    }

    /// @brief named ctor loads path coordinates from an ASCII file
    static PixPath load( const std::string& filename )
    {
        return PixPath( filename );
    }

    /**
     * @brief named ctor makes a path of evenly spaced points from a to b
     *
     * @param pt_a  first point of path
     * @param pt_b  last point of path provided sz is at least 2
     * @param sz    number of points in output
     * @return path from a to b
     *
     * If sz >= 2 then the output will have its first point at 'a' and its last
     * point at 'b', and its size() property will equal sz.  The other points
     * will lie more-or-less on the line segment ab, between them, and
     * increasing in distance from point 'a'.
     *
     * Due to rounding, the points
     * might not lie exactly on this line segment.  And if sz is large enough,
     * there can be duplication of points.  If sz < 2 I am not going to bother
     * to define the results.
     */
    static PixPath fenceposts(
        const PixPoint& pt_a,
        const PixPoint& pt_b,
        size_t sz
    )
    {
        return PixPath( pt_a, pt_b, sz );
    }

    /// @brief named ctor builds from string-format list of coordinates
    static PixPath parse( const std::string& coords )
    {
        return PixPath( coords, 0 );
    }

    /**
     * @brief Change a member of the path to a new PixPoint.
     *
     * @param index Index of point (aka vertex) in this path to be clobbered
     * @param newp  New location to store at the given index
     * @pre path must have size() exceeding given param 'index' value.
     * @return reference to this object after the modification
     *
     * I cannot do this by overloading operator[] because I don't know how to
     * make that approach update the map or the set.
     */
    virtual PixPath& assign( unsigned index, const PixPoint& newp )
    {
        // update the vector
        PixPoint oldp = my_vpp.at( index );
        my_vpp.at( index ) = newp;

        /*
         * Now update the map.
         * We could, but do not, delete zero-valued entries.  That is because
         * we expect that in the typical use case, there won't be very many.
         * It could be done similarly to the following:
         *  if ( 0 == ppmap[ oldp ] ) ppmap.erase( oldp );
         */
        if ( ppmap[ oldp ] <= 0 )
        {
            KJB_THROW_2(Cant_happen, "Internal corruption in PixPath::ppmap");
        }
        ppmap[ oldp ] -= 1;
        ppmap[ newp ] += 1;

        return *this;
    }

    /// @brief Vector-like access to points (returning an rvalue).
    const PixPoint& operator[]( unsigned index ) const
    {
        return my_vpp[ index ];
    }

    /// @brief Vector-like appending a new PixPoint on the end of the object
    virtual void push_back( const PixPoint& pp )
    {
        my_vpp.push_back( pp );
        ppmap[ pp ] += 1;
    }

    virtual void pop_back();

    /// @brief Number of points in the path (not the same as its length).
    size_t size() const
    {
        return my_vpp.size();
    }

    /// @brief Swap the contents of two PixPaths
    virtual void swap( PixPath& vpp )
    {
        my_vpp.swap( vpp.my_vpp );      // swap vectors of PixPoints
        ppmap.swap( vpp.ppmap );        // swap map of duplicates
    }

    /// @brief Discard all points
    virtual void clear()
    {
        my_vpp.clear();
        ppmap.clear();
    }

    /// @brief Return an iterator pointing to the first point in the path
    const_iterator begin() const
    {
        return my_vpp.begin();
    }

    /// @brief Rtn iterator pointing to just beyond the last point in the path
    const_iterator end() const
    {
        return my_vpp.end();
    }

    /// @brief return a reverse-moving iterator to the last point in path
    const_reverse_iterator rbegin() const
    {
        return my_vpp.rbegin();
    }

    /// @brief return one-before-first iterator, in symmetry with end().
    const_reverse_iterator rend() const
    {
        return my_vpp.rend();
    }

    /**
     * @brief Append a range from a given other path to the end of this path.
     * @param begin iterator pointing to first point to append
     * @param end iterator pointing one-past-last point to append
     * @pre begin, end must define an empty or increasing sequence in a
     *      different PixPath; behavior is undefined otherwise (probably bad).
     * @warning This is not a const method; it modifies the path!
     * @return A reference to this path (after the modification of course)
     *
     * Be aware that if the first point of suffix is the same as the last point
     * in this path, this method does not care about that, and the result will
     * be a self-intersection at that point.
     */
    virtual PixPath& append( const_iterator begin, const_iterator end );

    /**
     * @brief Append the given path to the tail of this path; overwrites.
     *
     * @param suffix    a path to become the new suffix of this path.
     *
     * @return A reference to this path (after the modification of course)
     *
     * @warning This is not a const method; it modifies the path!
     *
     * Be aware that if the first point of suffix is the same as the last point
     * in the path, this method does not care about that, and the result will
     * be a self-intersection at that point.
     */
    virtual PixPath& append( const PixPath& suffix );

    /**
     * @brief Like append(), with extra avoid-the-duplicates fuss; overwrites.
     *
     * @param suffix    a path to become the new suffix of this path, except
     *                  for the first point of suffix
     *
     * @pre First point of suffix must be equal to the last point of this path.
     *
     * @return kjb_c::NO_ERROR, or kjb_c::ERROR if the precondition is not met.
     *
     * @warning This is not a const method; it modifies the path!
     *
     * Use this when you KNOW the first point of suffix is the same as the last
     * point of this path, and you want to drop one of them before appending.
     * We test the preconditon, then skip suffix[0] and append suffix[1]
     * and all the rest of the suffix points; then we return NO_ERROR.
     *
     * This is pretty useful when trying to "transitively" join a path from a
     * to b, then a path from b to c, and you don't want 'b' in there twice.
     */
    virtual int append_no_overlap( const PixPath& suffix );

    // Return a new path that is copy of a subrange of this path, [begin,end[
    PixPath subrange( unsigned, unsigned ) const;


    /// @brief Variation on subrange:  return a prefix of this path (by value)
    PixPath prefix( unsigned postlast ) const
    {
        return subrange( 0, postlast );
    }

    /**
     * @brief Variation on subrange:  return a suffix of this path (by value)
     *
     * @param first     Index of the element of the path that you want to be
     *                  the first element of the suffix.
     *
     * @return path equivalent to the suffix of this path
     *
     * @warning The parameter is NOT the length of the suffix.  If you want a
     *          suffix of path P with length L, then you should invoke like so:
     * @code    P.suffix( P.size() - L )
     * @endcode
     */
    PixPath suffix( unsigned first ) const
    {
        return subrange( first, my_vpp.size() );
    }

    /**
     * @brief Reveal whether the path has any self-intersections
     * @param[out] where    Optional parameter indicating coordinates of hit
     *
     * @return true iff list of points contains a duplicate list entry.
     *
     * Unfortunately our data structures do not currently tell us quickly
     * what the indices of the colliding points are.
     *
     * This tests whether the path has duplicate points, not whether
     * segments between the points intersect.  For the latter, try
     * do_segments_intersect().
     */
    bool self_intersect( PixPoint* where = 00 ) const;

    /// @brief Is the path all connected, using 8-connectivity?
    bool connected8( bool emit_verbose_debug_output = false ) const;


    /// @brief Return (by value) the first point in the path.
    PixPoint front() const
    {
        return my_vpp.front();
    }

    /// @brief Return (by value) the last point in the path.
    PixPoint back() const
    {
        return my_vpp.back();
    }

    /**
     * @brief Return a vector of arclengths along the polygonal path.
     *
     * @pre Input pointer must not equal null.
     * @return kjb_c::NO_ERROR, or ERROR if the input equals null
     *
     * The pointer input (which must be non-null) points to a vector of output
     * such that the i-th point in the PixPath sequence has an arclength from
     * the first point as the i-th entry of the vector.  Arclength is defined
     * as the length of the polygonal path defined by the PixPath points.
     */
    virtual int arclength( std::vector< float >* ) const;

    /**
     * @brief compute and return polygonal path length of this path
     *
     * @return the length of the polygonal path defined by the points.
     * @throws kjb::KJB_error if the arclength computation fails.
     *
     * This method takes linear time in the number of points of the path.
     * So if you need to access it a lot, consider using PixPathAc, which
     * memoizes the answer.
     */
    virtual float arclength() const;

    /// @brief Return the path as an ASCII string
    std::string str( const std::string& separator = "\n" ) const;

    /// @brief Saves the path to an ASCII file, returns NO_ERROR or ERROR
    int save( const std::string& filename ) const;

    /// @brief Compare PixPaths; true iff the sequence of points is identical
    bool operator==( const PixPath& ) const;

    /// @brief Compare PixPaths; true iff the sequence of points differs
    bool operator!=( const PixPath& other ) const
    {
        return ! operator==( other );
    }

    /**
     * @brief Add, element-by-element, two PixPaths of the same length.
     *
     * @return new PixPath defined by elementwise sum of this and other path.
     *
     * In other words, it's something like this:
     * @code
     * for i = 0 to N-1
     *     output[i] = this[i] + that[i]
     * endfor
     * @endcode
     *
     * @throws Size_mismatch if the paths are not of the same length
     */
    PixPath operator+( const PixPath& ) const;

    /**
     * @brief Return the corner point of a bounding box, with min x and y.
     *
     * To restate that brief description more coherently, this method computes
     * (in LINEAR TIME!) the smallest axis-aligned bounding box around the path
     * and returns the coordinates of the corner point having the minimum x
     * coordinate and minimum y coordinate.
     *
     * @return PixPoint with x value = min{ x | (x,y) is in this path},
     *          y similar.  Note this point is not necessarily in the path.
     *
     * @throws Too_small if the path contains no points
     * @warning This method takes linear time and does not memoize its results.
     *
     * The method is virtual because someone someday might want memoization.
     */
    virtual PixPoint boundbox_min_min() const;

    /// @brief Similar to boundbox_min_min() but returns maximum x, y coords.
    virtual PixPoint boundbox_max_max() const;

    /**
     * @brief Test whether the axis-aligned bounding box of this path lies
     *      within the axis-aligned bounding box of the given path.
     *
     * @param outer path to be tested as the candidate container of this path
     *
     * @return true iff this path lies within a.a. bounding-box of 'outer' path
     *
     * "Within" here is
     * liberally defined to mean in the interior OR on the boundary.
     */
    bool boundbox_within_boundbox( const PixPath& outer ) const
    {
        return (boundbox_min_min() - outer.boundbox_min_min()).in_quadrant_I()
            && (outer.boundbox_max_max() - boundbox_max_max()).in_quadrant_I();
    }

    /**
     * @brief sort of like boundbox_within_boundbox but works on a single point
     * @param q query point to test for membership in semi-open rectang. region
     *
     * @return true if query point q is a member of semi-open region
     *
     * This was hard to name.  The final choice is meant to be scanned to mean
     * a path p has some semi-open axis-aligned minimal bounding box, and that
     * box holds q within itself (or else the return value is false).
     *
     * The bounding box is semi-open in that points on the min-x and min-y
     * boundaries are (potentially) "in" but points on the max-x and max-y
     * boundaries are definitely not.
     */
    bool so_boundbox_holds_within( const PixPoint& q ) const
    {
        return ( q - boundbox_min_min() ).in_quadrant_I()
            && ( boundbox_max_max() - q ).is_poz_poz();
    }

    /**
     * @brief Return number of times (maybe 0) the path hits a query point.
     * @param query_point point possibly in list of points (aka path vertices)
     * @return true iff query_point is a point in the list (aka a path vertex)
     *
     * This is just a lookup of your query point in the map:  it does NOT do
     * any sort of interpolation between points.  For example, if the path is
     * (0,0) (2,0) then hits(PixPoint(1,0)) returns zero!
     *
     * This takes O( log N ) time for an N point path.  Not O(N).
     * It would be even faster if we used spatial hashing, but we don't;
     * With Boost's help, we could.
     */
    unsigned hits( const PixPoint& query_point ) const
    {
        // You'd think the following is all you need, but no, it breaks const:
        // return ppmap[ query_point ];
        // Instead you have to do something like this:
        PPMap::const_iterator pn = ppmap.find( query_point );
        return ( ppmap.end() == pn ) ? 0 : pn -> second;
    }

    /// @brief return number of common points between query_path and this path
    unsigned hits( const PixPath& query_path ) const;


    PixPoint nearest( const PixPoint&, const_iterator* q = 00 ) const;

    PixPoint nearest( const kjb::Vector2&, const_iterator* q = 00 ) const;


    /**
     * @brief Reverse the order of the points in the path (overwriting!)
     *
     * @return reference to this path after the operation.
     *
     * The implementation relies on the fact that ppmap contents remain the
     * same whether the points are in forward or reverse order.
     * If we ever do a mulimap, quadtree, or kdtree implementation,
     * this will have to get more complicated.
     */
    PixPath& ow_reverse();


    /// @brief return a copy of this list after reversing the front-back order.
    PixPath reverse() const
    {
        PixPath ppp( *this );
        return ppp.ow_reverse();
    }


    /**
     * @brief Fit a cubic polynomial to the given list of x and y points
     *
     * @param[out] x    Where to put the coefficients for the x coordinate fit
     * @param[out] y    Where to put the coefficients for the y coordinate fit
     * @param[in] ref   Index value of the point in the path that you want to
     *                  be the "reference" point corresponding to t=0.
     */
    int cubic_fit( std::vector<float>* x, std::vector<float>* y,
                                                            int ref = 0) const;

    PixPath interpolate( bool forbid_self_intersection = false ) const;

    PixPath cull_redundant_points() const;

    PixPath merge_redundant_slopes() const;

    /// @brief Duplicate this path, except that we omit the indicated point.
    PixPath expel( const_iterator ) const;


    /// @brief Duplicate this path, except we omit the point with given index
    PixPath expel( size_t index_of_unwanted_point ) const
    {
        return expel( begin() + index_of_unwanted_point );
    }


    /**
     * @brief Compute the Hausdorff metric between two sets of points
     * @param that other points to compute this one's Hausdorff distance from.
     * @return Hausdorff distance, in units of linear pixel length
     *
     * This method does not interpolate for you!  If you want to compare the
     * gap between polygonal paths, it's up to you to interpolate the input.
     */
    virtual float hausdorff_distance( const PixPath& that ) const
    {
        return std::max( hausdorff_dist_1side( that ),
                            that.hausdorff_dist_1side( *this ) );
    }

    // Find the closest pair of points, ignoring adjacency, time is O(N log N).
    float closest_pair( const_iterator* pa = 0, const_iterator* pb = 0) const;

    /**
     * @brief Find a pair of adjacent points at least as close as any other
     *
     * @param[out]  pa  Optional output parameter:  iterator pointing to one of
     *                  the pair, such that *pa and *(pa+1) have minimum
     *                  distance.
     *
     * @return distance between an adjacent pair of points with min. distance
     *
     * @warning Keep in mind that the result is unlikely to be unique!
     * Since the coordinates are integers, it is VERY COMMON
     * for multiple pairs of adjacent points to have the same distance; thus it
     * is a bad idea to think of the result as THE closest adjacent pair.
     */
    float closest_adjacent_pair( const_iterator* pa = 0 ) const;

    /// @brief tests whether a given path is a subsequence of this path
    bool has_subsequence( const PixPath& subseq ) const;

    double angle_at( unsigned ) const;


    /**
     * @brief cross product of line segs (index-1,index+1) x (index,index+1)
     *
     * @param index must be the array index of an interior point
     *
     * Let point q be the point on this path with the given index.
     * Let points p, r be the predecessor and successor points.
     *
     * This algorithm is drawn from Cormen, Leiserson, Rivest, Stein chap 33
     * section 1.
     *
     * @return Zero if points p, q, r are collinear;
     * @return Positive if path from p to q to r is counterclockwise;
     * @return Negative if path from p to q to r is clockwise.
     * @return Abs. value of result is twice the area of the triangle p-q-r.
     */
    PixPoint::Integer bracket_cross_at( unsigned index ) const
    {
        PixPoint    bracket = my_vpp.at( index + 1 ) - my_vpp.at( index - 1 ),
                    here = my_vpp.at( index + 1 ) - my_vpp.at( index );
        return bracket.cross( here );
    }


    /**
     * @brief compute shift in "heading" angle at an interior point
     *
     * This calls method angle_at(), which can throw!
     *
     * @param index Index of INTERIOR point:  must be positive and less than
     *              (size()-1).
     * @return angle in radians of "shift" away from going in a straight line.
     *
     * Example:  path (0,0) (1,1) (2,2) (2,3) (3,4) has the following shifts:
     * - Undefined for index 0 and index 4 (throws std::out_of_range)
     * - Index 1:  heading shift 0
     * - Index 2:  heading shift M_PI_4
     * - Index 3:  heading shift -M_PI_4
     */
    double heading_shift_at( unsigned index ) const
    {
        double theta = M_PI - angle_at( index );
        return bracket_cross_at( index ) < 0 ? -theta : theta;
    }



    /**
     * @brief Test whether two segments of this path intersect
     *
     * @param preindex1 Defines first segment, from preindex1 to 1+preindex1.
     * @param preindex2 Defines second segment, from preindex2 to 1+preindex2.
     *
     * @param[out]  endpoint_intersector_index
     *              Optional output parameter revealing a vertex (if any) that
     *              coincides .  If omitted or null, or if the return value is
     *              false, this parameter is ignored.  If no path vertex
     *              coincides with an interior point of a segment, this
     *              parameter is also ignored.  Otherwise, on exit this points
     *              a location holding the value of the index of the vertex
     *              that lands on the other segment.
     *
     * @pre preindex1 and preindex2 must be smaller than size()-1.
     * @return true if the "closed" segments have a nonempty intersection
     *
     * Segments are "closed" -- they include their terminal vertices as well as
     * their interior points.
     *
     * In order to use the output parameter effectively, it must be set to a
     * value of size() or greater.  Then a change in this value can be
     * detected.
     *
     * There is no obligation to make preindex1 and preindex2 distinct.  If
     * they differ by zero or one then this routine is sure to return true.
     *
     * Algorithm is from Cormen, Leiserson, Rivest, Stein sec. 33.1.
     */
    bool intersect_at_with(
        unsigned preindex1,
        unsigned preindex2,
        unsigned* endpoint_intersector_index = 00
    )   const;


    /**
     * @brief quadratic time test whether any 2 path segments cross each other
     *
     * @param[out] s1   Optional; if it and s2 are not equal to null, at exit
     *                  it points to
     *                  a location containing the index of the first point of
     *                  the first segment in the intersection.  If omitted or
     *                  left equal to null, it and s2 are ignored.
     * @param[out] s2   Optional pointer to index of second segment in
     *                  intersection.  Can be omitted or set equal to null.
     * @param[out] count    Optional pointer to number of intersections.  This
     *                  can affect running time:  if omitted or set equal to
     *                  null, this method returns on the first intersection it
     *                  finds.  Otherwise, the method counts all intersections.
     *
     * @return true iff two polygonal-path segments intersect.
     *
     * If you omit or set to null either of s1 or s2, then both are regarded as
     * omitted.
     *
     * Segments are defined between each pair of adjacent points.
     * Of course in every polygonal path of 3 or more vertices,
     * adjacent segments share a
     * common "hinge" endpoint -- but those commonalities do not count.
     * Yet if there is a zero-length segment in the path, then this returns
     * true.
     *
     * True examples:
     *  - (0,0) (2,2) (1,3) (1,0)
     *      - output *s1=0, *s2=2
     *  - (0,0) (2,2) (1,3) (0,0)
     *      - output *s1=0, *s2=2
     *  - (0,0) (2,2) (1,3) (1,1)
     *      - output *s1=0, *s2=2
     *  - (0,0) (2,2) (0,0)
     *      - output *s1=0, *s2=1
     *  - (0,0) (2,2) (2,2)
     *      - output *s1=0, *s2=1
     *
     * False examples:
     *  - (0,0) (1,1) (2,2)
     *  - (0,0) (2,2) (1,3)
     *  - (0,0) (2,2)
     *
     * If there are multiple intersections and s1 and s2 do not equal null,
     * we do not define which intersection s1, s2 will indicate.
     * The current implementation might still behave like so:  if count equals
     * null, s1 and s2 might indicate intersecting segments closest to begin(),
     * and if count does not equal null, s1 and s2 might indicate intersecting
     * segments closest to end().  This description could be stale, however.
     *
     * If you have a lot of segments and only a few intersections expected,
     * consider using the Bentley-Ottman implementation, declared in
     * header file qd_cpp/intersection.h.  Under such conditions, a quadratic
     * time test will be expensive.
     */
    bool do_segments_intersect(
        unsigned* s1 = 00,
        unsigned* s2 = 00,
        unsigned* count = 00
    )   const;

    /**
     * @brief return index of first point of longest segment
     *
     * This performs a linear-time search through the pixpath for a longest
     * segment.  Not necessarily unique.
     *
     * @param[out] length   optional output parameter if you would like to know
     *                      the length of the longest segments.
     *
     * @return index value j where path[j] and path[1+j] define a segment at
     * least as long as any other segment.
     *
     * @throws Too_small if the path has fewer than 2 points.
     */
    unsigned longest_segment( float* length = 00 ) const;


#if 0
    // something is wrong with this method.
    /**
     * @brief use a heuristic to try to change the path so no segments cross.
     * @return EXIT_SUCCESS or EXIT_FAILURE indicating success or failure.
     *
     * Find where the path has intersecting segments
     *
     * If the points of the path are not pairwise-unique then this returns
     * EXIT_FAILURE.  If no pair of segments intersects, then this returns
     * EXIT_SUCCESS.  Otherwise, we process intersecting pairs according to
     * two cases:
     *
     * @section ins1 Intersection Case 1 of 2:
     *
     * @code
     * example with ascii art: . . .---(pi)
     *                                    \    e  f
     *                             (pj)----+---o--o-. . .
     *                              |       \
     *                              o--o--o--o
     *                              a  b  c  d
     * @endcode
     *
     * That picture is meant to suggest that the segment between path[pi] and
     * path[1+pi] intersects with the segment between path[pj] and path[1+pj].
     * The o's between are meant to indicate that pi and pj probably differ by
     * more than 1, in fact the original point sequence has substring
     * . . ., path[pi], d, c, b, a, path[pj], e, f, . . . .
     *
     * In this case, the solution is to reverse the subrange from d to path[pj]
     * so the above substring is replaced with the following:
     * . . ., path[pi], path[pj], a, b, c, d, e, f, . . . .
     *
     * which would resemble
     * @code
     *                         . . .---(pi)
     *                                /        e  f
     *                             (pj)        o--o-. . .
     *                              |         /
     *                              o--o--o--o
     *                              a  b  c  d
     * @endcode
     *
     * It would be wrong to think this will always reduce the number of
     * segment intersections.  The reason why is that the new segments between
     * path[pi],path[pj] and between d,e could slice through nests of curled
     * up path.  If you keep doing this over and over, though, I conjecture
     * that eventually you would run out of intersections, i.e., you would have
     * a net improvement.  I have no freaking clue whether that is true.
     *
     * @section ins2 Intersection Case 2 of 2:
     *
     * IN THE CASE OF pj == 1+pi, we basically perform a migrate operation,
     * eliminating the old value at pj, and introducing a new point in the
     * longest segment.  The new point shall be distinct from all other
     * points in the path.
     *
     * @code
     * Example: . . .---(pi)
     *                   |
     *                   |
     *                   | a   b    c                   d
     *                   |o----o----o-------------------o- . . .
     *                   ||
     *                  (pj)
     * @endcode
     *
     * This picture is inaccurate:  really there is no gap between segments
     * pi-pj and pj-a.  But I drew a small gap to show the sequence of points.
     * The idea here is to shift all the points so as to eliminate the hinge.
     * We generate a new point at the middle, if possible, of the longest
     * segment.  If the longest segment is c-d, the result would look like
     * this:
     *
     * @code
     * Example: . . .---(pi)
     *                   |
     *                   |
     *                   | a  b    c        c'         d
     *                   o----o----o--------o----------o- . . .
     * @endcode
     *
     * Whereas if the longest segment preceded pi, then we would introduce the
     * new point earlier.  If there already IS a point at the midpoint of the
     * longest segment, we choose another point stochastically along that
     * segment.  Also, pi-pj or pi-a could be the longest segment.   To avoid
     * such distractions, what we really want is
     * to introduce a new point into the longest segment of the
     * path induced by excluding pj.  And unless pi-a is the longest segment
     * in that induced path,
     * we will have to shift points up or down to annihilate the stupid value
     * at location pj.
     *
     * @todo currently this is not working exactly as expected.
     */
    int untangle_segments()
    {
        KJB(ASSERT( ! self_intersect() ));
        return untangle_recursive( 0 );
    }
#endif


     /// @brief return the "mean" of the points (nonempty, none may be unused)
     kjb::Vector2 centroid() const;

     /// @brief return index of a member point nearest centroid() (qv)
     unsigned member_near_centroid() const;

}; // end class PixPath



// Create a line using Bresenham's algorithm
PixPath bresenham_line( const PixPoint&, const PixPoint& );


// docs with implementation
std::string svg_path(const PixPath&);


PixPath append_trying_not_to_overlap( PixPath, const PixPath& );



/// @brief build a PixPath from a range of PixPoints, using random iterators
template< typename Ran >
inline PixPath copy_pixpoint_array( Ran begin, Ran end )
{
    PixPath result( PixPath::reserve( end-begin ) );
    std::copy( begin, end, std::back_inserter( result ) );
    return result;
}




}
}

namespace std
{

    /**
     * @brief Swap the contents of two PixPath objects
     */
    template<>
    inline void swap(
        kjb::qd::PixPath& p1,
        kjb::qd::PixPath& p2
    )
    {
        p1.swap( p2 );
    }



    /*
     * I trust that I don't need to write the same specialization for PixPathAc
     * since swap is virtual in PixPath.  Not sure.
     */
}



#endif
