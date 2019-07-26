/**
 * @file
 * @brief Regression tests for the PixPoint and PixPath classes
 * @author Andrew Predoehl
 *
 * This file contains some regression tests for the PixPoint and PixPath class.
 * If any test fails, an assertion will break.  Otherwise the fun will print
 * "success!" and return EXIT_SUCCESS on exit -- nothing fancy.
 */

/*
 * $Id: test_pixpath.cpp 21357 2017-03-30 05:35:22Z kobus $
 */


#include <l/l_sys_lib.h>
#include <l/l_sys_rand.h>
#include <l_cpp/l_stdio_wrap.h>
#include <l_cpp/l_test.h>
#include <qd_cpp/pixpath.h>
#include <qd_cpp/ppath_ac.h>
#include <qd_cpp/pathresize.h>
#include <qd_cpp/svgwrap.h>


//#define SHOW_PRETTY_PICTURES

#ifdef SHOW_PRETTY_PICTURES
    #include <i_cpp/i_image.h>
    #include <i_cpp/i_pixel.h>
#endif

#include <vector>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <iostream>

namespace
{

using kjb::qd::PixPoint;
using kjb::qd::PixPath;

std::string FNAME;

void skipping()
{
#ifdef TEST
    if (kjb_c::is_interactive())
    {
        std::cerr << "Skipping some unit tests, since TEST is not defined.\n";
    }
#endif
}

#ifdef SHOW_PRETTY_PICTURES

const int EDGE_SZ = 500;

const kjb::PixelRGBA    red     ( 100,   0,   0 ),
                        green   (   0, 100,   0 ),
                        blue    (   0,   0, 100 ),
                        gray    ( 100, 100, 100 ),
                        white   ( 200, 200, 200 );


void get_coords( double px, double py, int* cx, int* cy )
{
    assert( cx );
    assert( cy );
    *cx = static_cast<int>( px*10.0 + 50 );
    *cy = static_cast<int>( EDGE_SZ -py*10.0 - 50 );
}




#endif

/*
 * This function tests the following PixPoint methods:
 *  operator<
 *  is_unused
 *  operator==
 *  operator!=
 *  operator+
 *  dist
 *  dist2
 *  L_infinity_distance
 */
int test1()
{
    using kjb::qd::TEST_PIXPOINT_UNUSED;

    PixPoint a, b( 1, 1 ), c( 1, 2 ), d( 2, 1 ), e( 1, 2 );

    TEST_TRUE( a.is_unused() );
    TEST_TRUE( ! a.is_not_unused() );
    TEST_TRUE( ! a.is_used() );

    TEST_TRUE( ! b.is_unused() );
    TEST_TRUE( b.is_not_unused() );
    TEST_TRUE( b.is_used() );

    /* PixPoints should be totally ordered, reflecting row-major order. */
    TEST_TRUE( b < d );
    TEST_TRUE( !( d < b ) );   /* antisymmetry */
    TEST_TRUE( d < c );
    TEST_TRUE( !( c < d ) );
    TEST_TRUE( b < c );        /* transitivity */
    TEST_TRUE( !( c < b ) );

    /* equality */
    TEST_TRUE( c == c );
    TEST_TRUE( c == e );
    TEST_TRUE( e == c );
    TEST_TRUE( e != b );
    TEST_TRUE( e != d );

    /* addition */
    TEST_TRUE( b + c == PixPoint( 2, 3 ) );
    TEST_TRUE( c + PixPoint( 0, 0 ) == c );
    TEST_TRUE( d + e == PixPoint( 3, 3 ) );

#ifdef TEST
    bool caught = !TEST_PIXPOINT_UNUSED;
    try { a + d; } catch( const PixPoint::Unused& ) { caught = true; }
    TEST_TRUE( caught );
#else
    skipping();
#endif

#ifdef TEST
    caught = !TEST_PIXPOINT_UNUSED;
    try { d + a; } catch( const PixPoint::Unused& ) { caught = true; }
    TEST_TRUE( caught );
#endif

    /* subtraction */
    TEST_TRUE( b - b == PixPoint( 0, 0 ) );
    TEST_TRUE( d - b == PixPoint( 1, 0 ) );
    TEST_TRUE( c - PixPoint( 0, 0 ) == c );
    TEST_TRUE( c - d == PixPoint( -1, 1 ) );

#ifdef TEST
    caught = !TEST_PIXPOINT_UNUSED;
    try { a - d; } catch( const PixPoint::Unused& ) { caught = true; }
    TEST_TRUE( caught );
#endif

#ifdef TEST
    caught = !TEST_PIXPOINT_UNUSED;
    try { d - a; } catch( const PixPoint::Unused& ) { caught = true; }
    TEST_TRUE( caught );
#endif

    /* L2 distance squared */
    TEST_TRUE( b.dist2( b ) == 0 );
    TEST_TRUE( b.dist2( c ) == 1 );
    TEST_TRUE( b.dist2( d ) == 1 );
    TEST_TRUE( c.dist2( e ) == 0 );
    TEST_TRUE( c.dist2( d ) == 2 );

    /* L2 distance */
    TEST_TRUE( b.dist( b ) == 0 );
    TEST_TRUE( b.dist( c ) == 1 );
    TEST_TRUE( b.dist( d ) == 1 );
    TEST_TRUE( c.dist( e ) == 0 );
    TEST_APPROX_EQUALITY( c.dist( d ), M_SQRT2 );

    /* L-infinity distance */
    TEST_TRUE( b.L_infinity_distance( b ) == 0 );
    TEST_TRUE( b.L_infinity_distance( c ) == 1 );
    TEST_TRUE( b.L_infinity_distance( d ) == 1 );
    TEST_TRUE( c.L_infinity_distance( d ) == 1 );

    return EXIT_SUCCESS;
}


/*
 * This function tests the following PixPoint methods:
 *  dist2
 *  adjacent8
 *  operator+
 */
int test2()
{
    PixPoint p( 5,5 );
    PixPoint dn = PixPoint( 0,  1 );
    PixPoint ds = PixPoint( 0, -1 );
    PixPoint de = PixPoint(  1, 0 );
    PixPoint dw = PixPoint( -1, 0 );

    PixPoint pn = p + dn;
    PixPoint ps = p + ds;
    PixPoint pe = p + de;
    PixPoint pw = p + dw;
    PixPoint pne = p + dn + de;
    PixPoint pnw = p + dn + dw;
    PixPoint pse = p + ds + de;
    PixPoint psw = p + ds + dw;

    TEST_TRUE( dn.dist2( dn ) == 0 );
    TEST_TRUE( dn.dist2( ds ) == 4 );
    TEST_TRUE( dn.dist2( de ) == 2 );
    TEST_TRUE( dn.dist2( dw ) == 2 );
    TEST_TRUE( ds.dist2( dn ) == 4 );
    TEST_TRUE( ds.dist2( ds ) == 0 );
    TEST_TRUE( ds.dist2( de ) == 2 );
    TEST_TRUE( ds.dist2( dw ) == 2 );
    TEST_TRUE( de.dist2( dn ) == 2 );
    TEST_TRUE( de.dist2( ds ) == 2 );
    TEST_TRUE( de.dist2( de ) == 0 );
    TEST_TRUE( de.dist2( dw ) == 4 );
    TEST_TRUE( dw.dist2( dn ) == 2 );
    TEST_TRUE( dw.dist2( ds ) == 2 );
    TEST_TRUE( dw.dist2( de ) == 4 );
    TEST_TRUE( dw.dist2( dw ) == 0 );

    TEST_TRUE(  dn.adjacent8( dn ) );
    TEST_TRUE( !dn.adjacent8( ds ) );
    TEST_TRUE(  dn.adjacent8( de ) );
    TEST_TRUE(  dn.adjacent8( dw ) );
    TEST_TRUE( !ds.adjacent8( dn ) );
    TEST_TRUE(  ds.adjacent8( ds ) );
    TEST_TRUE(  ds.adjacent8( de ) );
    TEST_TRUE(  ds.adjacent8( dw ) );
    TEST_TRUE(  de.adjacent8( dn ) );
    TEST_TRUE(  de.adjacent8( ds ) );
    TEST_TRUE(  de.adjacent8( de ) );
    TEST_TRUE( !de.adjacent8( dw ) );
    TEST_TRUE(  dw.adjacent8( dn ) );
    TEST_TRUE(  dw.adjacent8( ds ) );
    TEST_TRUE( !dw.adjacent8( de ) );
    TEST_TRUE(  dw.adjacent8( dw ) );

    PixPoint *pp[] ={ &pn, &pne, &pe, &pse, &ps, &psw, &pw, &pnw };

    for( int iii = 1; iii < 8; ++iii )
        TEST_TRUE( pp[iii] -> adjacent8( p ) );

    return EXIT_SUCCESS;
}


/* This function tests saving and loading a path of 100 random points. */
int test3()
{
    PixPath ppp( PixPath::reserve( 100 ) );

    for( int iii = 0; iii < 100; ++iii )
        ppp.push_back( PixPoint( rand(), rand() ) );

    int rc = ppp.save( FNAME );
    assert( kjb_c::NO_ERROR == rc );

    TEST_TRUE( ppp == PixPath::load( FNAME ) );
    return EXIT_SUCCESS;
}


/*
 * This function tests the following PixPath methods:
 *  size
 *  push_back
 *  back, nee last
 *  append
 *  append_no_overlap
 *  connected8
 *  arclength
 *  self_intersect
 *  subrange
 *  prefix
 *  suffix
 *  boundbox_min_min
 *  boundbox_max_max
 *  boundbox_within_boundbox
 *  so_boundbox_holds_within
 *  in_quadrant_I
 *  is_poz_poz
 *  hits
 *  assign
 */
int test4()
{
    PixPath ppp( PixPath::reserve( 4 ) );
    ppp.push_back( PixPoint( 5, 5 ) );
    ppp.push_back( PixPoint( 6, 6 ) );
    ppp.push_back( PixPoint( 7, 7 ) );
    ppp.push_back( PixPoint( 8, 8 ) );

    PixPath qqq( PixPath::reserve( 3 ) );
    qqq.push_back( PixPoint( 1, 1 ) );
    qqq.push_back( PixPoint( 2, 2 ) );
    qqq.push_back( PixPoint( 3, 3 ) );

    PixPath rrr( PixPath::reserve() );
    rrr = qqq;  // deep copy
    rrr.append( ppp );
    rrr.append( qqq );

    PixPath mmm( append_trying_not_to_overlap( ppp, qqq ) );
    TEST_TRUE( mmm.prefix( 4 ) == ppp );
    TEST_TRUE( mmm.suffix( 4 ) == qqq );
    PixPath nnn( append_trying_not_to_overlap( ppp, ppp.reverse() ) );
    TEST_TRUE( 7 == nnn.size() );
    TEST_TRUE( nnn.prefix( 4 ) == ppp );
    TEST_TRUE( nnn.suffix( 3 ).reverse() == ppp );

    TEST_TRUE( ppp.connected8() );
    TEST_TRUE( qqq.connected8() );
    TEST_TRUE( ! rrr.connected8() );

    TEST_APPROX_EQUALITY( ppp.arclength(), 3 * M_SQRT2 );
    TEST_APPROX_EQUALITY( qqq.arclength(), 2 * M_SQRT2 );
    TEST_APPROX_EQUALITY( rrr.arclength(), 16 * M_SQRT2 );

    TEST_TRUE( ! ppp.self_intersect() );
    TEST_TRUE( ! qqq.self_intersect() );
    TEST_TRUE( rrr.self_intersect() );

    TEST_TRUE( 4 == ppp.size() );
    TEST_TRUE( 3 == qqq.size() );
    TEST_TRUE( 10 == rrr.size() );
    TEST_TRUE( ppp == rrr.subrange( 3, 7 ) );
    TEST_TRUE( qqq == rrr.prefix( 3 ) );
    TEST_TRUE( qqq == rrr.suffix( 7 ) );

    TEST_TRUE( PixPoint( 1, 1 ) == rrr.boundbox_min_min() );
    TEST_TRUE( PixPoint( 8, 8 ) == rrr.boundbox_max_max() );

    // reflexive relationship
    TEST_TRUE( ppp.boundbox_within_boundbox( ppp ) );
    TEST_TRUE( rrr.boundbox_within_boundbox( rrr ) );
    TEST_TRUE( qqq.boundbox_within_boundbox( qqq ) );

    // not an antisymmetric relationship, but reminiscient of one
    TEST_TRUE( ppp.boundbox_within_boundbox( rrr ) ); // rrr is "bigger"
    TEST_TRUE( qqq.boundbox_within_boundbox( rrr ) );
    TEST_TRUE( ! qqq.boundbox_within_boundbox( ppp ) );
    TEST_TRUE( ! ppp.boundbox_within_boundbox( qqq ) );
    TEST_TRUE( ! rrr.boundbox_within_boundbox( ppp ) );
    TEST_TRUE( ! rrr.boundbox_within_boundbox( qqq ) );

    TEST_TRUE( qqq.so_boundbox_holds_within( PixPoint( 1, 1 ) ) );
    TEST_TRUE( qqq.so_boundbox_holds_within( PixPoint( 2, 2 ) ) );
    TEST_TRUE( qqq.so_boundbox_holds_within( PixPoint( 1, 2 ) ) );
    TEST_TRUE( qqq.so_boundbox_holds_within( PixPoint( 2, 1 ) ) );
    TEST_TRUE( ! qqq.so_boundbox_holds_within( PixPoint( 3, 3 ) ) );
    TEST_TRUE( ! qqq.so_boundbox_holds_within( PixPoint( 3, 1 ) ) );
    TEST_TRUE( ! qqq.so_boundbox_holds_within( PixPoint( 1, 3 ) ) );
    TEST_TRUE( ! qqq.so_boundbox_holds_within( PixPoint( 0, 0 ) ) );
    TEST_TRUE( ! qqq.so_boundbox_holds_within( PixPoint( 1, 0 ) ) );
    TEST_TRUE( ! qqq.so_boundbox_holds_within( PixPoint( 0, 1 ) ) );
    TEST_TRUE( ! qqq.so_boundbox_holds_within( PixPoint( 2, 3 ) ) );
    TEST_TRUE( ! qqq.so_boundbox_holds_within( PixPoint( 3, 2 ) ) );
    TEST_TRUE( ! qqq.so_boundbox_holds_within( PixPoint( 3, 0 ) ) );
    TEST_TRUE( ! qqq.so_boundbox_holds_within( PixPoint( 2, 0 ) ) );
    TEST_TRUE( ! qqq.so_boundbox_holds_within( PixPoint( 0, 3 ) ) );
    TEST_TRUE( ! qqq.so_boundbox_holds_within( PixPoint( 0, 2 ) ) );

    TEST_TRUE( PixPoint( 1, 1 ).in_quadrant_I() );
    TEST_TRUE( PixPoint( 0, 0 ).in_quadrant_I() );
    TEST_TRUE( PixPoint( 1, 0 ).in_quadrant_I() );
    TEST_TRUE( PixPoint( 0, 1 ).in_quadrant_I() );
    TEST_TRUE( ! PixPoint(  0, -1 ).in_quadrant_I() );
    TEST_TRUE( ! PixPoint( -1,  0 ).in_quadrant_I() );
    TEST_TRUE( ! PixPoint( -1, -1 ).in_quadrant_I() );
    TEST_TRUE( ! PixPoint(  1, -1 ).in_quadrant_I() );
    TEST_TRUE( ! PixPoint( -1,  1 ).in_quadrant_I() );

    TEST_TRUE( PixPoint( 1, 1 ).is_poz_poz() );
    TEST_TRUE( ! PixPoint( 0, 0 ).is_poz_poz() );
    TEST_TRUE( ! PixPoint( 1, 0 ).is_poz_poz() );
    TEST_TRUE( ! PixPoint( 0, 1 ).is_poz_poz() );
    TEST_TRUE( ! PixPoint(  0, -1 ).is_poz_poz() );
    TEST_TRUE( ! PixPoint( -1,  0 ).is_poz_poz() );
    TEST_TRUE( ! PixPoint( -1, -1 ).is_poz_poz() );
    TEST_TRUE( ! PixPoint(  1, -1 ).is_poz_poz() );
    TEST_TRUE( ! PixPoint( -1,  1 ).is_poz_poz() );

    // JJ (judge judy?) defines a bounding box that should perfectly fit ppp.
    PixPoint::Is_inbounds JJ( PixPoint(5,5), /*width,height =*/PixPoint(4,4) );
    TEST_TRUE( ! JJ( PixPoint(5,4) ) );
    TEST_TRUE(   JJ( PixPoint(5,5) ) );
    TEST_TRUE(   JJ( PixPoint(5,6) ) );
    TEST_TRUE(   JJ( PixPoint(5,7) ) );
    TEST_TRUE(   JJ( PixPoint(5,8) ) );
    TEST_TRUE( ! JJ( PixPoint(5,9) ) );
    TEST_TRUE( ! JJ( PixPoint(4,5) ) );
    TEST_TRUE(   JJ( PixPoint(5,5) ) );
    TEST_TRUE(   JJ( PixPoint(6,5) ) );
    TEST_TRUE(   JJ( PixPoint(7,5) ) );
    TEST_TRUE(   JJ( PixPoint(8,5) ) );
    TEST_TRUE( ! JJ( PixPoint(9,5) ) );
    TEST_TRUE( ! JJ( PixPoint(8,4) ) );
    TEST_TRUE(   JJ( PixPoint(8,5) ) );
    TEST_TRUE(   JJ( PixPoint(8,6) ) );
    TEST_TRUE(   JJ( PixPoint(8,7) ) );
    TEST_TRUE(   JJ( PixPoint(8,8) ) );
    TEST_TRUE( ! JJ( PixPoint(8,9) ) );
    TEST_TRUE( ! JJ( PixPoint(4,8) ) );
    TEST_TRUE(   JJ( PixPoint(5,8) ) );
    TEST_TRUE(   JJ( PixPoint(6,8) ) );
    TEST_TRUE(   JJ( PixPoint(7,8) ) );
    TEST_TRUE(   JJ( PixPoint(8,8) ) );
    TEST_TRUE( ! JJ( PixPoint(9,8) ) );
    // Here's an idiom for verifying that ppp lies within JJ's domain:
    TEST_TRUE( ppp.end() == std::find_if( ppp.begin(), ppp.end(),
                                                        std::not1( JJ ) ) );
    // similarly but backward
    TEST_TRUE( ppp.rend() == std::find_if( ppp.rbegin(), ppp.rend(),
                                                        std::not1( JJ ) ) );
    // But the very first point of qqq is outside JJ's domain; rrr also outside
    TEST_TRUE( qqq.begin() == std::find_if( qqq.begin(), qqq.end(),
                                                        std::not1( JJ ) ) );
    TEST_TRUE( rrr.end() != std::find_if( rrr.begin(), rrr.end(),
                                                        std::not1( JJ ) ) );

    TEST_TRUE( qqq.hits( PixPoint( 2, 2 ) ) );
    TEST_TRUE( ! qqq.hits( PixPoint( 4, 4 ) ) );
    TEST_TRUE( 2 == rrr.hits( PixPoint( 3, 3 ) ) );
    TEST_TRUE( 1 == rrr.hits( PixPoint( 5, 5 ) ) );
    TEST_TRUE( 0 == rrr.hits( PixPoint( 4, 4 ) ) );

    TEST_TRUE( kjb_c::ERROR == rrr.append_no_overlap( ppp ) );

    TEST_TRUE( PixPoint( 3, 3 ) == rrr.back() );
    rrr.push_back( PixPoint( 1, 1 ) );
    TEST_TRUE( PixPoint( 1, 1 ) == rrr.back() );
    TEST_TRUE( kjb_c::NO_ERROR == rrr.append_no_overlap( qqq ) );
    TEST_TRUE( 13 == rrr.size() );
    TEST_TRUE( qqq == rrr.suffix( 10 ) );

    TEST_TRUE( PixPoint( 1, 1 ) == rrr[0] );
    TEST_TRUE( PixPoint( 2, 2 ) == rrr[1] );
    TEST_TRUE( PixPoint( 3, 3 ) == rrr[2] );
    TEST_TRUE( PixPoint( 5, 5 ) == rrr[3] );
    TEST_TRUE( PixPoint( 6, 6 ) == rrr[4] );
    TEST_TRUE( PixPoint( 7, 7 ) == rrr[5] );
    TEST_TRUE( PixPoint( 8, 8 ) == rrr[6] );
    TEST_TRUE( PixPoint( 1, 1 ) == rrr[7] );
    TEST_TRUE( PixPoint( 2, 2 ) == rrr[8] );
    TEST_TRUE( PixPoint( 3, 3 ) == rrr[9] );
    TEST_TRUE( PixPoint( 1, 1 ) == rrr[10] );
    TEST_TRUE( PixPoint( 2, 2 ) == rrr[11] );
    TEST_TRUE( PixPoint( 3, 3 ) == rrr[12] );
    TEST_TRUE( 1 == rrr.hits( PixPoint( 6, 6 ) ) );

    rrr.assign( 4, PixPoint( 66, 66 ) );
    TEST_TRUE( PixPoint( 66, 66 ) == rrr[4] );
    TEST_TRUE( 0 == rrr.hits( PixPoint( 6, 6 ) ) );

    TEST_TRUE( 3 == rrr.hits( PixPoint( 2, 2 ) ) );
    rrr.assign( 1, PixPoint( 12, 12 ) );
    TEST_TRUE( 2 == rrr.hits( PixPoint( 2, 2 ) ) );
    rrr.assign( 8, PixPoint( 22, 22 ) );
    TEST_TRUE( 1 == rrr.hits( PixPoint( 2, 2 ) ) );
    rrr.assign( 11, PixPoint( 32, 32 ) );
    TEST_TRUE( 0 == rrr.hits( PixPoint( 2, 2 ) ) );

    TEST_TRUE( 3 == rrr.hits( PixPoint( 1, 1 ) ) );
    rrr.clear();
    TEST_TRUE( 0 == rrr.size() );
    TEST_TRUE( 0 == rrr.hits( PixPoint( 1, 1 ) ) );
    TEST_TRUE( 3 == qqq.size() );

    return EXIT_SUCCESS;
}


// This transposes x, y coordinates, yielding mirrored path about the line y==x
PixPath transpose( const PixPath& p )
{
    PixPath t( PixPath::reserve( p.size() ) );
    for( PixPath::const_iterator q = p.begin(); q != p.end(); ++q )
    {
        t.push_back( PixPoint( q->y, q->x ) );
    }
    return t;
}

// This yields a path mirrored about the y axis
PixPath mirrory( const PixPath& p )
{
    PixPath t( PixPath::reserve( p.size() ) );
    for( PixPath::const_iterator q = p.begin(); q != p.end(); ++q )
    {
        t.push_back( PixPoint( - q->x, q->y ) );
    }
    return t;
}

// This yields a path mirrored about the x axis
PixPath mirrorx( const PixPath& p )
{
    PixPath t( PixPath::reserve( p.size() ) );
    for( PixPath::const_iterator q = p.begin(); q != p.end(); ++q )
    {
        t.push_back( PixPoint( q->x, - q->y ) );
    }
    return t;
}


PixPath operator+( const PixPath& p, const PixPoint& c )
{
    PixPath t( PixPath::reserve( p.size() ) );
    for( PixPath::const_iterator q = p.begin(); q != p.end(); ++q )
    {
        t.push_back( PixPoint( q->x + c.x, q->y + c.y ) );
    }
    return t;
}


/**
 * Test PixPath methods:
 * - PixPath::bresenham_line( PixPoint, Pixpoint )
 * - ow_reverse
 * - reverse
 * - interpolate
 * - cull_redundant_points
 * - expel
 *
 * @todo test all four quadrants
 */
int test5()
{
    using kjb::qd::bresenham_line;

    /*
     *    a   +   +   +   +   +   +   +
     *    9                       q q
     *    8   +   +   +   + q q q +   +
     *    7             q q
     *    6   +   +   +   +   +   +   +
     *    5
     *    4   +   +   +   +   +   +   +
     *    3
     *    2   +   +   +   +   +   +   +
     *    1
     *    O 1 2 3 4 5 6 7 8 9 a b c d e f (hexadecimal)
     */
    const PixPath ppp( bresenham_line( PixPoint( 7, 7 ), PixPoint( 13, 9 ) ) );
    PixPath qqq( PixPath::reserve(7) );
    qqq.push_back( PixPoint( 7, 7 ) );
    qqq.push_back( PixPoint( 8, 7 ) );
    qqq.push_back( PixPoint( 9, 8 ) );
    qqq.push_back( PixPoint( 10, 8 ) );
    qqq.push_back( PixPoint( 11, 8 ) );
    qqq.push_back( PixPoint( 12, 9 ) );
    qqq.push_back( PixPoint( 13, 9 ) );

    TEST_TRUE( ppp == qqq );
    TEST_TRUE( ppp.connected8() );

    qqq.ow_reverse();
    const PixPath qqqrrr( qqq.reverse() );
    size_t index = qqq.size();
    TEST_TRUE( 7 == index );
    PixPath::const_iterator qri = qqqrrr.begin();
    PixPath::const_reverse_iterator criq = qqq.rbegin();
    for( PixPath::const_iterator pr = ppp.begin(); pr != ppp.end(); ++pr ) {
        TEST_TRUE( *pr == qqq[ --index ] );
        TEST_TRUE( *pr == *qri++ );
        TEST_TRUE( *pr == *criq++ );
    }
    TEST_TRUE( qqqrrr.end() == qri );
    TEST_TRUE( qqq.rend() == criq );

    // test variations of steep, gradual, deltax, deltay
    const PixPath   q1s( bresenham_line( PixPoint( 7, 7), PixPoint( 9, 13) ) ),
                    q2s( bresenham_line( PixPoint(-7, 7), PixPoint(-9, 13) ) ),
                    q2g( bresenham_line( PixPoint(-7, 7), PixPoint(-13, 9) ) ),
                    q3s( bresenham_line( PixPoint(-7,-7), PixPoint(-9,-13) ) ),
                    q3g( bresenham_line( PixPoint(-7,-7), PixPoint(-13,-9) ) ),
                    q4s( bresenham_line( PixPoint( 7,-7), PixPoint( 9,-13) ) ),
                    q4g( bresenham_line( PixPoint( 7,-7), PixPoint( 13,-9) ) ),
                    qv1( bresenham_line( PixPoint( 7, 7), PixPoint( 7, 13) ) ),
                    qv2( bresenham_line( PixPoint( 7,-7), PixPoint( 7,-13) ) ),
                    qh1( bresenham_line( PixPoint( 7,-7), PixPoint( 13,-7) ) ),
                    qh2( bresenham_line( PixPoint(-7,-7), PixPoint(-13,-7) ) ),
                    qu1( bresenham_line( PixPoint( 7, 7), PixPoint( 13,13) ) ),
                    qu2( bresenham_line( PixPoint(-7,-7), PixPoint(-13,-13)) );
    TEST_TRUE( transpose(q1s) == ppp );
    TEST_TRUE( mirrory(q2s) == q1s );
    TEST_TRUE( mirrory(q2g) == ppp );
    TEST_TRUE( q3g.reverse() + PixPoint(20,16) == ppp );
    TEST_TRUE( q3s.reverse() + PixPoint(16,20) == q1s );
    TEST_TRUE( transpose(q3s) == q3g );
    TEST_TRUE( mirrorx(q4s) == q1s );
    TEST_TRUE( mirrorx(q4g) == ppp );
    for( int iii = 7, jjj = 0; iii <= 13; ++iii, ++jjj )
    {
        TEST_TRUE(  7 == qv1[jjj].x &&  iii == qv1[jjj].y );
        TEST_TRUE(  7 == qv2[jjj].x && -iii == qv2[jjj].y );
        TEST_TRUE( -7 == qh1[jjj].y &&  iii == qh1[jjj].x );
        TEST_TRUE( -7 == qh2[jjj].y && -iii == qh2[jjj].x );
        TEST_TRUE(  iii == qu1[jjj].x &&  iii == qu1[jjj].y );
        TEST_TRUE( -iii == qu2[jjj].x && -iii == qu2[jjj].y );
    }

    // test interpolate
    PixPath ur_rrr( PixPath::reserve(2) );
    ur_rrr.push_back( PixPoint( 7, 7 ) );
    ur_rrr.push_back( PixPoint( 13, 9 ) );
    const PixPath rrr( ur_rrr );
    TEST_TRUE( ! rrr.connected8() );
    PixPath sss( rrr.interpolate() );
    TEST_TRUE( ! rrr.connected8() );
    TEST_TRUE( 2 == rrr.size() );
    TEST_TRUE( sss.connected8() );
    TEST_TRUE( sss == ppp );

    PixPath ttt = sss.cull_redundant_points();
    TEST_TRUE( sss == ppp ); /* unchanged */
    TEST_TRUE( ttt == rrr );

    PixPath::const_iterator middle = sss.begin() + 3;
    const PixPath vvv( sss.expel( middle ) );
    TEST_TRUE( 6 == vvv.size() );
    TEST_TRUE( PixPoint(  7, 7 ) == vvv[0] );
    TEST_TRUE( PixPoint(  8, 7 ) == vvv[1] );
    TEST_TRUE( PixPoint(  9, 8 ) == vvv[2] );
    TEST_TRUE( PixPoint( 11, 8 ) == vvv[3] );
    TEST_TRUE( PixPoint( 12, 9 ) == vvv[4] );
    TEST_TRUE( PixPoint( 13, 9 ) == vvv[5] );
    TEST_TRUE( vvv.interpolate() == sss );

    // test a "trivial" bresenham line
    const PixPoint ccc( 7, 11 ), ddd;
    PixPath www( bresenham_line( ccc, ccc ) );

    TEST_TRUE( 1 == www.size() );
    TEST_TRUE( ccc == www.front() );
    TEST_TRUE( ccc == www.back() );

#ifdef TEST
    // test a bresenham line based on an unused point
    bool catchy = false;
    try { PixPath xxx( bresenham_line( ccc, ddd ) ); }
    catch( PixPoint::Unused& ) { catchy = true; }
    TEST_TRUE( catchy );

    catchy = false;
    try { PixPath xxx( bresenham_line( ddd, ccc ) ); }
    catch( PixPoint::Unused& ) { catchy = true; }
    TEST_TRUE( catchy );

    catchy = false;
    try { PixPath xxx( bresenham_line( ddd, ddd ) ); }
    catch( PixPoint::Unused& ) { catchy = true; }
    TEST_TRUE( catchy );
#else
    skipping();
#endif

    // test merge_redundant_slopes
    const PixPath yyy( sss.merge_redundant_slopes() );
    TEST_TRUE( yyy == vvv );

    return EXIT_SUCCESS;
}


/* Test PixPath methods:
 *  cubic_fit
 */
int test6()
{
#ifdef KJB_HAVE_LAPACK
    PixPath ppp( PixPath::reserve( 5 ) );
    ppp.push_back( PixPoint( 0, 0 ) );
    ppp.push_back( PixPoint( 10, 10 ) );
    ppp.push_back( PixPoint( 20, 10 ) );
    ppp.push_back( PixPoint( 30, 20 ) );
    ppp.push_back( PixPoint( 40, 0 ) );

    std::vector<float> x, y;
    KJB(EPETE(ppp.cubic_fit( &x, &y )));
    TEST_TRUE( 4 == x.size() );
    TEST_TRUE( 4 == y.size() );

    TEST_APPROX_EQUALITY( x[0], -1.6e-4 );
    TEST_APPROX_EQUALITY( y[0], -4.5e-4 );
    TEST_APPROX_EQUALITY( x[1],  9.8e-3 );
    TEST_APPROX_EQUALITY( y[1],  2.35e-2 );
    TEST_APPROX_EQUALITY( x[2],  6.6e-1 );
    TEST_APPROX_EQUALITY( y[2],  2.33e-1 );
    TEST_APPROX_EQUALITY( x[3], -1.3e-1 );
    TEST_APPROX_EQUALITY( y[3],  5.5e-1 );

#ifdef SHOW_PRETTY_PICTURES
    kjb::Image img = kjb::Image::create_zero_image( EDGE_SZ, EDGE_SZ );

    /*
     * Plot polynomial
     */
    for( double t = -5; t < 62; t += 0.05 ) {
        double px = 0, py = 0, td = 1;
        for( int dr = 0; dr < 4; ++dr ) {
            int dd = 3-dr;
            px += td * x[dd];
            py += td * y[dd];
            td *= t;
        }
        int cx, cy;
        int &cr = cy;   // row alias
        int &cc = cx;   // column alias

        get_coords( px, py, &cx, &cy );
        img.at( cr, cc ) = gray;
    }

    /*
     * Plot original data points plus curvature plus "close point" on polynom
     */
    double t = 0;
    for( PixPath::const_iterator q = ppp.begin(); q != ppp.end(); ++q ) {
        int cx, cy;
        int &cr = cy;   // row alias
        int &cc = cx;   // column alias

        /* update t */
        if ( q != ppp.begin() )
            t += q -> dist( *(q-1) );

        /* paint a green dot on the polynomial curve that should be pretty
         * close to the corresponding data point we are considering.
         */
        double px = 0, py = 0, td = 1;
        for( int dr = 0; dr < 4; ++dr ) {
            int dd = 3-dr;
            px += td * x[dd];
            py += td * y[dd];
            td *= t;
        }
        get_coords( px, py, &cx, &cy );
        //img.paint( cx, cy, green );
        img.at( cr, cc ) = green;
        //img.rectangle( cx, cy, 6, 6, green );

        img.draw_aa_rectangle( cr-4, cc-4, cr+4, cc+4, green );

        /* paint a large square with a color center for the data point and the
         * curvature at the "corresponding" polynomial point.
         */
        get_coords( q -> x, q -> y, &cx, &cy );
        //img.rectangle( cx, cy, 5, 5, white );
        img.draw_aa_rectangle( cr-3, cc-3, cr+3, cc+3, white );

        double  Dx = x[2] + ( 2*x[1] + 3*x[0]*t )*t,
                Dy = y[2] + ( 2*y[1] + 3*y[0]*t )*t,
                D2x = 2*x[1] + 6*x[0]*t,
                D2y = 2*y[1] + 6*y[0]*t,
                n = Dx*D2y - Dy*D2x,
                d1 = Dx*Dx + Dy*Dy,
                d2 = d1 * sqrt(d1),
                k = 16384 * d2 < fabs(n) ? 16384 : n/d2;
        printf("t = %e, n = %e, d2 = %e, k = %e\n", t, n, d2, k);
        float   rrr = std::max( std::min( 128 + 1000*k, 255.0 ), 0.0 ),
                bbb = std::max( std::min( 128 - 1000*k, 255.0 ), 0.0 );
        kjb_c::Pixel pixel = gray;
        pixel.r = rrr;
        pixel.b = bbb;
        //img.rectangle( cx, cy, 3, 3, pixel );
        img.draw_aa_rectangle( cr-2, cc-2, cr+2, cc+2, pixel );
    }

    img.display( "cubic interpolation" );
#endif
#endif

    return EXIT_SUCCESS;
}


/* Test PixPath methods:
 *  hausdorff_distance
 */
int test7()
{
    PixPath p1( PixPath::reserve( 5 ) );
    p1.push_back( PixPoint( 1, 1 ) );
    p1.push_back( PixPoint( 2, 2 ) );
    p1.push_back( PixPoint( 3, 1 ) );
    p1.push_back( PixPoint( 4, 2 ) );
    p1.push_back( PixPoint( 5, 1 ) );

    PixPath p2( PixPath::reserve( 5 ) );
    p2.push_back( PixPoint( 1, -1 ) );
    p2.push_back( PixPoint( 2, -1 ) );
    p2.push_back( PixPoint( 3, -2 ) );
    p2.push_back( PixPoint( 4, -1 ) );
    p2.push_back( PixPoint( 5, -1 ) );

    PixPath p3( PixPath::reserve( 5 ) );
    p3.push_back( PixPoint( 1, 3 ) );
    p3.push_back( PixPoint( 2, 2 ) );
    p3.push_back( PixPoint( 3, 1 ) );
    p3.push_back( PixPoint( 4, 2 ) );
    p3.push_back( PixPoint( 5, 2 ) );

    PixPath p4( p3 );
    p4.ow_reverse();
    TEST_TRUE( p3[0] < p4[0] || p4[0] < p3[0] );

    /*
     * Rule #1 for a metric:  d(x,y)=0 iff x is equivalent to y
     * Note that Hausdorff distance is not oriented, so p3 and p4 are
     * equivalent though the differ in direction.
     *
     * Note:  The Hausdorff distance method actually returns a float, so these
     * exact comparisons are perilous, but I seem to be getting away with it.
     */
    TEST_TRUE( p1.hausdorff_distance( p1 ) == 0 );
    TEST_TRUE( p2.hausdorff_distance( p2 ) == 0 );
    TEST_TRUE( p3.hausdorff_distance( p3 ) == 0 );
    TEST_TRUE( p4.hausdorff_distance( p4 ) == 0 );
    TEST_TRUE( p4.hausdorff_distance( p3 ) == 0 );
    TEST_TRUE( p3.hausdorff_distance( p4 ) == 0 );

    TEST_TRUE( p1.hausdorff_distance( p2 ) != 0 );
    TEST_TRUE( p1.hausdorff_distance( p3 ) != 0 );
    TEST_TRUE( p1.hausdorff_distance( p4 ) != 0 );

    TEST_TRUE( p2.hausdorff_distance( p1 ) != 0 );
    TEST_TRUE( p2.hausdorff_distance( p3 ) != 0 );
    TEST_TRUE( p2.hausdorff_distance( p4 ) != 0 );

    TEST_TRUE( p3.hausdorff_distance( p1 ) != 0 );
    TEST_TRUE( p3.hausdorff_distance( p2 ) != 0 );

    TEST_TRUE( p4.hausdorff_distance( p1 ) != 0 );
    TEST_TRUE( p4.hausdorff_distance( p2 ) != 0 );

    /*
     * Rule #2 for a metric:  d(x,y) = d(y,x); also, we know what it oughta be
     */
    TEST_TRUE( p1.hausdorff_distance( p2 ) == 3 );
    TEST_TRUE( p2.hausdorff_distance( p1 ) == 3 );

    TEST_APPROX_EQUALITY( p1.hausdorff_distance( p3 ), M_SQRT2 );
    TEST_APPROX_EQUALITY( p3.hausdorff_distance( p1 ), M_SQRT2 );

    TEST_APPROX_EQUALITY( p1.hausdorff_distance( p4 ), M_SQRT2 );
    TEST_APPROX_EQUALITY( p4.hausdorff_distance( p1 ), M_SQRT2 );

    TEST_TRUE( p3.hausdorff_distance( p2 ) == 4 );
    TEST_TRUE( p2.hausdorff_distance( p3 ) == 4 );
    return EXIT_SUCCESS;
}


/* Test PixPath methods:
 *  angle_at
 */
int test8()
{
    PixPath p( PixPath::reserve( 4 ) );
    p.push_back( PixPoint( 100, 100 ) );
    p.push_back( PixPoint( 100, 90 ) );
    p.push_back( PixPoint( 120, 90 ) );
    p.push_back( PixPoint( 120, 90 ) );

    TEST_APPROX_EQUALITY( p.angle_at( 1 ), M_PI_2 );

    bool satisfied = false;

    try {
        p.angle_at( 0 );
    }
    catch ( std :: out_of_range ) {
        satisfied = true;
    }

    TEST_TRUE( satisfied );

    satisfied = false;

    try {
        p.angle_at( 3 );
    }
    catch ( std :: out_of_range ) {
        satisfied = true;
    }

    TEST_TRUE( satisfied );

    satisfied = false;

    try
    {
        // degenerate triangle:  angle undefined
        p.angle_at( 2 );
    }
    catch (const kjb::Illegal_argument&)
    {
        satisfied = true;
    }

    TEST_TRUE( satisfied );

    // A straight chunk of PixPath has an included angle of M_PI or nearly so:
    p.push_back( PixPoint( 130, 100 ) ); // this elt has index 4
    p.push_back( PixPoint( 140, 110 ) );
    TEST_APPROX_EQUALITY( p.angle_at( 4 ), M_PI );

    return EXIT_SUCCESS;
}

int test9(){

    PixPath p( PixPath::reserve( 4 ) );
    p.push_back( PixPoint( 1, 1 ) );
    p.push_back( PixPoint( 2, 2 ) );
    p.push_back( PixPoint( 3, 3 ) );
    p.push_back( PixPoint( 4, 4 ) );

    PixPath q( PixPath::reserve( 3 ) );
    q.push_back( PixPoint( 1, 1 ) );
    q.push_back( PixPoint( 1, 1 ) );
    q.push_back( PixPoint( 1, 1 ) );

    bool satisfied = false;

    try
    {
        p + q;
    }
    catch (const kjb::Dimension_mismatch&)
    {
        satisfied = true;
    }

    TEST_TRUE( satisfied );

    q.push_back( PixPoint( 1, 1 ) );

    PixPath result( PixPath::reserve() );
    result = p + q;

    TEST_TRUE( result[0] == PixPoint(2,2));
    TEST_TRUE( result[1] == PixPoint(3,3));
    TEST_TRUE( result[2] == PixPoint(4,4));
    TEST_TRUE( result[3] == PixPoint(5,5));

    return EXIT_SUCCESS;
}


/*
 * This function tests the following PixPathAc methods:
 *  halfway
 *  arclength( size_t )
 */
int test10()
{
    using kjb::qd::PixPathAc;

    PixPathAc p( PixPathAc::reserve( 20 ) );

    p.push_back( PixPoint( 1, 0 ) );

    for( int i = 1; i < 11; ++i )
        p.push_back( PixPoint( 0, 100*i ) );
    TEST_TRUE( 4 == p.halfway() );

    p.ow_reverse();
    TEST_TRUE( 5 == p.halfway() );

    p.ow_reverse();
    for( int i = 11; i < 21; ++i )
        p.push_back( PixPoint( 100*( i - 10 ), 100*i ) );

    TEST_TRUE( 11 == p.halfway() );

    p.ow_reverse();
    TEST_TRUE( 8 == p.halfway() );

    return EXIT_SUCCESS;
}


/* Test PixPath methods:
 *  bracket_cross_at
 *  heading_shift_at
 *  intersect_at_with
 *  do_segments_intersect
 *  is_collinear
 */
int test11()
{
    PixPath p( PixPath::reserve( 3 ) );
    p.push_back( PixPoint( 1, 1 ) );
    p.push_back( PixPoint( 2, 2 ) );
    p.push_back( PixPoint( 3, 3 ) );

    PixPath q( PixPath::reserve( 3 ) );
    q.push_back( PixPoint( 1, 1 ) );
    q.push_back( PixPoint( 2, 2 ) );
    q.push_back( PixPoint( 2, 3 ) );

    PixPath r( PixPath::reserve( 3 ) );
    r.push_back( PixPoint( 1, 1 ) );
    r.push_back( PixPoint( 2, 2 ) );
    r.push_back( PixPoint( 3, 2 ) );

    TEST_TRUE( fabs( p.heading_shift_at( 1 ) ) < 1e-6 );
    TEST_APPROX_EQUALITY( q.heading_shift_at( 1 ), M_PI_4 );
    TEST_APPROX_EQUALITY( r.heading_shift_at( 1 ), -M_PI_4 );

    PixPoint a( 1, 0 ), b( 0, 1 );
    TEST_TRUE( 1 == a.cross( b ) );    // right hand rule tells us so
    TEST_TRUE( -1 == b.cross( a ) );

    TEST_TRUE( 0 == p.bracket_cross_at( 1 ) ); // collinear
    TEST_TRUE( 0 < q.bracket_cross_at( 1 ) ); // counterclockwise
    TEST_TRUE( 0 > r.bracket_cross_at( 1 ) ); // clockwise

    PixPath s( PixPath::reserve( 6 ) );
    s.push_back( PixPoint( 0, 0 ) );
    s.push_back( PixPoint( 2, 2 ) );
    s.push_back( PixPoint( 3, 1 ) );
    s.push_back( PixPoint( 1, 2 ) );
    s.push_back( PixPoint( 4, 3 ) );
    s.push_back( PixPoint( 1, 1 ) );
    unsigned sgix = 10;
    sgix=10; TEST_TRUE(  s.intersect_at_with( 0, 0, &sgix ) &&  0==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 0, 1, &sgix ) &&  1==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 0, 2, &sgix ) && 10==sgix );
    sgix=10; TEST_TRUE(! s.intersect_at_with( 0, 3, &sgix ) && 10==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 0, 4, &sgix ) &&  5==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 1, 0, &sgix ) &&  1==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 2, 0, &sgix ) && 10==sgix );
    sgix=10; TEST_TRUE(! s.intersect_at_with( 3, 0, &sgix ) && 10==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 4, 0, &sgix ) &&  5==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 1, 1, &sgix ) &&  1==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 1, 2, &sgix ) &&  2==sgix );
    sgix=10; TEST_TRUE(! s.intersect_at_with( 1, 3, &sgix ) && 10==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 1, 4, &sgix ) && 10==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 2, 1, &sgix ) &&  2==sgix );
    sgix=10; TEST_TRUE(! s.intersect_at_with( 3, 1, &sgix ) && 10==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 4, 1, &sgix ) && 10==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 2, 2, &sgix ) &&  2==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 2, 3, &sgix ) &&  3==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 2, 4, &sgix ) && 10==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 3, 2, &sgix ) &&  3==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 4, 2, &sgix ) && 10==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 3, 3, &sgix ) &&  3==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 3, 4, &sgix ) &&  4==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 4, 3, &sgix ) &&  4==sgix );
    sgix=10; TEST_TRUE(  s.intersect_at_with( 4, 4, &sgix ) &&  4==sgix );

    TEST_TRUE( ! p.do_segments_intersect() );
    TEST_TRUE( ! q.do_segments_intersect() );
    TEST_TRUE( ! r.do_segments_intersect() );
    TEST_TRUE(   s.do_segments_intersect() );

    unsigned s1, s2;
    TEST_TRUE(   s.do_segments_intersect( &s1, &s2 ) );
    TEST_TRUE( 0 == s1 );
    TEST_TRUE( 2 == s2 );

    p.push_back( PixPoint( 0,0 ) );
    TEST_TRUE( p.do_segments_intersect( &s1, &s2 ) );
    TEST_TRUE( 0 == s1 );
    TEST_TRUE( 2 == s2 );

    q.push_back( PixPoint( 2,4 ) );
    TEST_TRUE( ! q.do_segments_intersect( &s1, &s2 ) );
    q.push_back( PixPoint( 2,3 ) );
    TEST_TRUE( q.do_segments_intersect( &s1, &s2 ) );
    TEST_TRUE( 1 == s1 );
    TEST_TRUE( 3 == s2 );

    r.push_back( PixPoint( 2,2 ) );
    r.push_back( PixPoint( 2,4 ) );
    r.push_back( PixPoint( 2,5 ) );
    TEST_TRUE( r.do_segments_intersect( &s1, &s2 ) );
    TEST_TRUE( 0 == s1 );
    TEST_TRUE( 2 == s2 );

    // Here we test the example inputs shown in the Doxygen docs
    std::vector< PixPath >  ptrue( 5, PixPath::reserve() ),
                            pfalse( 3, PixPath::reserve() );
    const unsigned true_second_intersection[] = { 2, 2, 2, 1, 1 };

    for( int iii = 0; iii < 5; ++iii ) {
        ptrue[ iii ].push_back( PixPoint( 0, 0 ) );
        ptrue[ iii ].push_back( PixPoint( 2, 2 ) );
    }
    ptrue[ 3 ].push_back( PixPoint( 0,0 ) );
    ptrue[ 4 ].push_back( PixPoint( 2,2 ) );
    for( int iii = 0; iii < 3; ++iii ) {
        ptrue[ iii ].push_back( PixPoint( 1, 3 ) );
        pfalse[ iii ].push_back( PixPoint( 0, 0 ) );
    }
    ptrue[ 0 ].push_back( PixPoint( 1, 0 ) );
    ptrue[ 1 ].push_back( PixPoint( 0, 0 ) );
    ptrue[ 2 ].push_back( PixPoint( 1, 1 ) );

    pfalse[ 0 ].push_back( PixPoint( 1, 1 ) );
    pfalse[ 0 ].push_back( PixPoint( 2, 2 ) );
    pfalse[ 1 ].push_back( PixPoint( 2, 2 ) );
    pfalse[ 1 ].push_back( PixPoint( 1, 3 ) );
    pfalse[ 2 ].push_back( PixPoint( 2, 2 ) );

    for( int iii = 0; iii < 5; ++iii ) {
        TEST_TRUE( ptrue[ iii ].do_segments_intersect( &s1, &s2 ) );
        TEST_TRUE( 0 == s1 );
        TEST_TRUE( true_second_intersection[ iii ] == s2 );
    }

    for( int iii = 0; iii < 3; ++iii )
        TEST_TRUE( ! pfalse[ iii ].do_segments_intersect() );



    // a few more related tests
    TEST_TRUE( a.is_collinear( b, PixPoint( -1, 2 ) ) );
    TEST_TRUE( ! a.is_collinear( b, PixPoint( 1, 2 ) ) );
    TEST_TRUE( PixPoint( 2, 1 ).is_collinear(
                                        PixPoint( 4, 2 ), PixPoint( 6, 3 ) ) );
    TEST_TRUE( PixPoint( 2, 1 ).is_collinear(
                                        PixPoint( 2, 1 ), PixPoint( 7, 7 ) ) );
    TEST_TRUE( ! PixPoint( 2, 1 ).is_collinear(
                                        PixPoint( 4, 2 ), PixPoint( 7, 7 ) ) );

    return EXIT_SUCCESS;
}


// test12 moved to test_doublecircle.cpp


/* test in PixPath:
 *  longest_segment
 */
int test13()
{
    PixPath pp( PixPath::reserve() );

    bool fine = false;
    try
    {
        pp.longest_segment();
    }
    catch (kjb::Illegal_argument&)
    {
        fine = true;
    }
    TEST_TRUE( fine );

    pp.push_back( PixPoint(0,0) );

    fine = false;
    try
    {
        pp.longest_segment();
    }
    catch (kjb::Illegal_argument&)
    {
        fine = true;
    }
    TEST_TRUE( fine );

    pp.push_back( PixPoint(1,1) );
    pp.push_back( PixPoint(5,5) );
    pp.push_back( PixPoint(8,8) );
    pp.push_back( PixPoint(11,11) );

    float len = -1;
    unsigned s1 = pp.longest_segment( &len );
    TEST_TRUE( 1 == s1 );
    TEST_APPROX_EQUALITY( len, 4 * M_SQRT2 );

    pp.assign( 3, PixPoint(6,6) );

    unsigned s2 = pp.longest_segment( &len );
    TEST_TRUE( 3 == s2 );
    TEST_APPROX_EQUALITY( len, 5 * M_SQRT2 );

    return EXIT_SUCCESS;
}


// test14 moved to test_doublecircle.cpp



#if 0
/*
 * test in PixPath:
 *  untangle_segments
 */
int test15()
{
    PixPath path1( PixPath::reserve( 6 ) );
    /* before                  after
     *
     *  o---o   o---o       o---o   o---o
     *       \ /                |   |
     *        X                 |   |
     *       / \                |   |
     *      o---o               o---o
     */
    path1.push_back( PixPoint( 0, 1 ) );
    path1.push_back( PixPoint( 1, 1 ) );
    path1.push_back( PixPoint( 2, 0 ) );
    path1.push_back( PixPoint( 1, 0 ) );
    path1.push_back( PixPoint( 2, 1 ) );
    path1.push_back( PixPoint( 3, 1 ) );

    int rc1 = path1.untangle_segments();
    TEST_TRUE( EXIT_SUCCESS == rc1 );
    TEST_TRUE( 6 == path1.size() );
    TEST_TRUE( PixPoint( 0, 1 ) == path1[ 0 ] );
    TEST_TRUE( PixPoint( 1, 1 ) == path1[ 1 ] );
    TEST_TRUE( PixPoint( 1, 0 ) == path1[ 2 ] );
    TEST_TRUE( PixPoint( 2, 0 ) == path1[ 3 ] );
    TEST_TRUE( PixPoint( 2, 1 ) == path1[ 4 ] );
    TEST_TRUE( PixPoint( 3, 1 ) == path1[ 5 ] );

    PixPath path2( PixPath::reserve( 12 ) );
    /* before                  after
     *
     *   o-o o-------o o-o      o-o o-------o o-o
     *      \ \     / /           |  \     /  |
     *       \ \   / /            |  |     |  |
     *        \ \ / /             |  \     /  |
     *         o X o              |   o   o   |
     *        / / \ \             |  /     \  |
     *       / /   \ \            |  |     |  |
     *      / /     \ \           |  /     \  |
     *     o-o       o-o          o-o       o-o
     */
    path2.push_back( PixPoint( 1, 2 ) );
    path2.push_back( PixPoint( 2, 2 ) );
    path2.push_back( PixPoint( 4, 1 ) );
    path2.push_back( PixPoint( 2, 0 ) );
    path2.push_back( PixPoint( 3, 0 ) );
    path2.push_back( PixPoint( 7, 2 ) );
    path2.push_back( PixPoint( 3, 2 ) );
    path2.push_back( PixPoint( 7, 0 ) );
    path2.push_back( PixPoint( 8, 0 ) );
    path2.push_back( PixPoint( 6, 1 ) );
    path2.push_back( PixPoint( 8, 2 ) );
    path2.push_back( PixPoint( 9, 2 ) );

    int rc2 = path2.untangle_segments();
    TEST_TRUE( EXIT_SUCCESS == rc2 );
    TEST_TRUE( 12 == path2.size() );
    TEST_TRUE( PixPoint( 1, 2 ) == path2[ 0 ] );
    TEST_TRUE( PixPoint( 2, 2 ) == path2[ 1 ] );
    TEST_TRUE( PixPoint( 2, 0 ) == path2[ 2 ] );
    TEST_TRUE( PixPoint( 3, 0 ) == path2[ 3 ] );
    TEST_TRUE( PixPoint( 4, 1 ) == path2[ 4 ] );
    TEST_TRUE( PixPoint( 3, 2 ) == path2[ 5 ] );
    TEST_TRUE( PixPoint( 7, 2 ) == path2[ 6 ] );
    TEST_TRUE( PixPoint( 6, 1 ) == path2[ 7 ] );
    TEST_TRUE( PixPoint( 7, 0 ) == path2[ 8 ] );
    TEST_TRUE( PixPoint( 8, 0 ) == path2[ 9 ] );
    TEST_TRUE( PixPoint( 8 ,2 ) == path2[ 10 ] );
    TEST_TRUE( PixPoint( 9, 2 ) == path2[ 11 ] );

    // ruin this path by inserting a self-intersection
    path2.push_back( PixPoint( 3, 2 ) );
    // now it cannot be untangled
    int rc22 = path2.untangle_segments();
    TEST_TRUE( EXIT_FAILURE == rc22 );

    PixPath path3( PixPath::reserve( 6 ) );
    /* before           after
     *    o----o           o-o-o
     *         |               |
     *         |o-o            o-o
     *         ||
     *         o
     *
     */
    path3.push_back( PixPoint( 0, 2 ) );
    path3.push_back( PixPoint( 2, 2 ) );
    path3.push_back( PixPoint( 2, 0 ) );
    path3.push_back( PixPoint( 2, 1 ) );
    path3.push_back( PixPoint( 3, 1 ) );

    int rc3 = path3.untangle_segments();
    TEST_TRUE( EXIT_SUCCESS == rc3 );
    TEST_TRUE( 5 == path3.size() );
    TEST_TRUE( PixPoint( 0, 2 ) == path3[ 0 ] );
    TEST_TRUE( PixPoint( 1, 2 ) == path3[ 1 ] );
    TEST_TRUE( PixPoint( 2, 2 ) == path3[ 2 ] );
    TEST_TRUE( PixPoint( 2, 1 ) == path3[ 3 ] );
    TEST_TRUE( PixPoint( 3, 1 ) == path3[ 4 ] );

    PixPath path4( PixPath::reserve( 5 ) );
    /* before           after
     *     o-o           o-o
     *       |             |
     *       |o---o        o-o-o
     *       ||
     *       o
     *
     */
    path4.push_back( PixPoint( 0, 2 ) );
    path4.push_back( PixPoint( 1, 2 ) );
    path4.push_back( PixPoint( 1, 0 ) );
    path4.push_back( PixPoint( 1, 1 ) );
    path4.push_back( PixPoint( 3, 1 ) );

    int rc4 = path4.untangle_segments();
    TEST_TRUE( EXIT_SUCCESS == rc4 );
    TEST_TRUE( 5 == path4.size() );
    TEST_TRUE( PixPoint( 0, 2 ) == path4[ 0 ] );
    TEST_TRUE( PixPoint( 1, 2 ) == path4[ 1 ] );
    TEST_TRUE( PixPoint( 1, 1 ) == path4[ 2 ] );
    TEST_TRUE( PixPoint( 2, 1 ) == path4[ 3 ] );
    TEST_TRUE( PixPoint( 3, 1 ) == path4[ 4 ] );

    PixPath path5( PixPath::reserve() );
    /* before           after
     *     o-o           o-o
     *       |             |
     *       |             o
     *       |             |
     *       |o-o          o-o
     *       ||
     *       o
     *
     */
    path5.push_back( PixPoint( 0, 2 ) );
    path5.push_back( PixPoint( 1, 2 ) );
    path5.push_back( PixPoint( 1, -1 ) );
    path5.push_back( PixPoint( 1, 0 ) );
    path5.push_back( PixPoint( 2, 0 ) );

    int rc5 = path5.untangle_segments();
    TEST_TRUE( EXIT_SUCCESS == rc5 );
    TEST_TRUE( 5 == path5.size() );
    TEST_TRUE( PixPoint( 0, 2 ) == path5[ 0 ] );
    TEST_TRUE( PixPoint( 1, 2 ) == path5[ 1 ] );
    TEST_TRUE( PixPoint( 1, 1 ) == path5[ 2 ] );
    TEST_TRUE( PixPoint( 1, 0 ) == path5[ 3 ] );
    TEST_TRUE( PixPoint( 2, 0 ) == path5[ 4 ] );

    /* ------------------------------------- */

    PixPath path6( PixPath::reserve() );
    path6.push_back( PixPoint( 9, 2 ) );
    path6.push_back( PixPoint( 0, 4 ) );
    path6.push_back( PixPoint(20, 4 ) );
    path6.push_back( PixPoint( 8, 1 ) );
    path6.push_back( PixPoint(10, 0 ) );
    path6.push_back( PixPoint(10, 3 ) );
    PixPath path7( path6 );

    int rc6 = path7.untangle_segments();
    TEST_TRUE( EXIT_FAILURE == rc6 ); // chopped too much

    /* ------------------------------------- */

    PixPath path8( PixPath::reserve() );
    path8.push_back( PixPoint( 0, 2 ) );
    path8.append( path6 );

    int rc7 = path8.untangle_segments();
    TEST_TRUE( EXIT_SUCCESS == rc7 );
    TEST_TRUE( 7 == path8.size() );
    TEST_TRUE( PixPoint( 0, 2 ) == path8[ 0 ] );
    TEST_TRUE( PixPoint( 0, 4 ) == path8[ 1 ] );
    TEST_TRUE( PixPoint(10, 4 ) == path8[ 2 ] ); // interpolated
    TEST_TRUE( PixPoint(20, 4 ) == path8[ 3 ] );
    TEST_TRUE( PixPoint(10, 0 ) == path8[ 4 ] );
    TEST_TRUE( PixPoint( 8, 1 ) == path8[ 5 ] );
    TEST_TRUE( PixPoint(10, 3 ) == path8[ 6 ] );

    /* ------------------------------------- */

    PixPath path9( PixPath::reserve() );
    path9.push_back( PixPoint( 7, 2 ) );
    path9.push_back( PixPoint( 0, 0 ) );
    path9.push_back( PixPoint( 0, 2 ) );
    path9.push_back( PixPoint(10, 0 ) );
    path9.push_back( PixPoint( 5, 4 ) );
    path9.push_back( PixPoint( 2, 2 ) );

    int rc8 = path9.untangle_segments();
    TEST_TRUE( EXIT_SUCCESS == rc8 );
    TEST_TRUE( 6 == path9.size() );
    TEST_TRUE( PixPoint( 7, 2 ) == path9[ 0 ] );
    TEST_TRUE( PixPoint( 6, 2 ) == path9[ 1 ] ); // interpolated
    TEST_TRUE( PixPoint( 5, 2 ) == path9[ 2 ] ); // interpolated
    TEST_TRUE( PixPoint( 4, 2 ) == path9[ 3 ] ); // interpolated
    TEST_TRUE( PixPoint( 3, 2 ) == path9[ 4 ] ); // interpolated
    TEST_TRUE( PixPoint( 2, 2 ) == path9[ 5 ] );

    return EXIT_SUCCESS;
}
#endif


int test16()
{
    PixPath pos(
            PixPath::fenceposts( PixPoint( 17, 4 ), PixPoint( 23, 16 ), 7 ) );
    TEST_TRUE( 7 == pos.size() );
    TEST_TRUE( PixPoint( 17, 4 ) == pos[ 0 ] );
    TEST_TRUE( PixPoint( 18, 6 ) == pos[ 1 ] );
    TEST_TRUE( PixPoint( 19, 8 ) == pos[ 2 ] );
    TEST_TRUE( PixPoint( 20,10 ) == pos[ 3 ] );
    TEST_TRUE( PixPoint( 21,12 ) == pos[ 4 ] );
    TEST_TRUE( PixPoint( 22,14 ) == pos[ 5 ] );
    TEST_TRUE( PixPoint( 23,16 ) == pos[ 6 ] );

    TEST_TRUE( 1 == pos.hits( PixPoint( 17, 4 ) ) );
    TEST_TRUE( 1 == pos.hits( PixPoint( 18, 6 ) ) );
    TEST_TRUE( 1 == pos.hits( PixPoint( 19, 8 ) ) );
    TEST_TRUE( 1 == pos.hits( PixPoint( 20,10 ) ) );
    TEST_TRUE( 1 == pos.hits( PixPoint( 21,12 ) ) );
    TEST_TRUE( 1 == pos.hits( PixPoint( 22,14 ) ) );
    TEST_TRUE( 1 == pos.hits( PixPoint( 23,16 ) ) );

    return EXIT_SUCCESS;
}



/*
 * test sosq_error, in header <pathresize.h>
 */
int test17()
{
    using kjb::qd::sosq_error;

    PixPath path1( PixPath::reserve( 3 ) );
    path1.push_back( PixPoint(0,0) );
    path1.push_back( PixPoint(1,1) );
    path1.push_back( PixPoint(2,0) );
    float e1 = sosq_error( path1, 0, 2 );
    TEST_APPROX_EQUALITY( e1, 1 );

    path1.assign( 1, PixPoint(0,1) );
    float e2 = sosq_error( path1, 0, 2 );
    TEST_APPROX_EQUALITY( e2, 1 );

    path1.assign( 1, PixPoint(2,1) );
    float e3 = sosq_error( path1, 0, 2 );
    TEST_APPROX_EQUALITY( e3, 1 );

    path1.assign( 1, PixPoint(2,-1) );
    float e4 = sosq_error( path1, 0, 2 );
    TEST_APPROX_EQUALITY( e4, 1 );

    path1.assign( 1, PixPoint(2,-2) );  // error will be 4, that is, -2 squared
    float e5 = sosq_error( path1, 0, 2 );
    TEST_APPROX_EQUALITY( e5, 4 ); // verify square quality

    path1.push_back( PixPoint(3,1) ); // index 3, contributes 1 to error
    path1.push_back( PixPoint(4,2) ); // index 4, contributes 4 to error
    path1.push_back( PixPoint(5,3) ); // index 5, contributes 9 to error
    path1.push_back( PixPoint(6,0) ); // index 6 is new endpoint
    float e6 = sosq_error( path1, 0, 6 );
    TEST_APPROX_EQUALITY( e6, 4+0+1+4+9 ); // verify that they are summed

#if 0 /* class Memo_sosq_error went private */
    // should be the same functionality but memoizes its answers
    kjb::qd::Memo_sosq_error sf( path1 );
    float e7 = sf( 0, 6 );
    TEST_APPROX_EQUALITY( e7, e6 );

    for( size_t iii = 0; iii < 6; ++iii )
    {
        TEST_APPROX_EQUALITY( 0, sf( iii, iii ) );
    }

    TEST_APPROX_EQUALITY( 4, sf( 0, 2 ) ); // non-memoized result
    TEST_APPROX_EQUALITY( 4, sf( 0, 2 ) ); // memoized result
    TEST_APPROX_ZERO( sf( 2, 5 ) );
    TEST_APPROX_ZERO( sf( 2, 5 ) ); // mem
    TEST_APPROX_EQUALITY( 14, sf( 2, 6 ) );
    TEST_APPROX_EQUALITY( 14, sf( 2, 6 ) ); // mem
#endif

    return EXIT_SUCCESS;
}



/*
 * This one tests function polyline_approx()
 */
int test18()
{
    PixPath aaa( PixPath::reserve( 7 ) );
    aaa.push_back( PixPoint( 100,100 ) );
    aaa.push_back( PixPoint( 200,202 ) );
    aaa.push_back( PixPoint( 300,300 ) );
    aaa.push_back( PixPoint( 400,303 ) );
    aaa.push_back( PixPoint( 500,300 ) );
    aaa.push_back( PixPoint( 600,198 ) );
    aaa.push_back( PixPoint( 700,100 ) );

    float err = 0;
    PixPath bbb( kjb::qd::polyline_approx( aaa, 4, &err ) );
    TEST_TRUE( err < 50 );
    TEST_TRUE( aaa.has_subsequence( bbb ) );
    TEST_TRUE( bbb[0] == aaa[0] );
    TEST_TRUE( bbb[1] == aaa[2] );
    TEST_TRUE( bbb[2] == aaa[4] );
    TEST_TRUE( bbb[3] == aaa[6] );

    return EXIT_SUCCESS;
}


PixPoint::Integer rand_ppi( int size )
{
    return ( rand()/float(RAND_MAX) ) * size;
}
PixPoint randpt( int size )
{
    return PixPoint( rand_ppi( size ), rand_ppi( size ) );
}


// Test PixPath::closest_pair
int test19()
{
    // simple concrete demonstration
    PixPath qqq( PixPath::reserve( 5 ) );
    qqq.push_back( PixPoint( 10, 30 ) );
    qqq.push_back( PixPoint( 10, 10 ) );
    qqq.push_back( PixPoint( 40, 10 ) );
    qqq.push_back( PixPoint(-10, 20 ) );
    qqq.push_back( PixPoint( 13, 6 ) );
    // note that qqq[1] and qqq[4] are the closest pair

    PixPath::const_iterator q1, q2, a1 = qqq.begin()+1, a2 = qqq.begin()+4;
    float qd = qqq.closest_pair( &q1, &q2 );
    TEST_APPROX_EQUALITY( qd, 5 );
    TEST_TRUE( ( q1==a1 && q2==a2 ) || ( q1==a2 && q2==a1 ) );

    const int SZ = 50;
    const int MM = 2 * SZ;
    PixPath ppp( PixPath::reserve( MM ) );

    // special case: empty PP
    bool caught = false;
    try
    {
        ppp.closest_pair();
    }
    catch (kjb::Illegal_argument&)
    {
        caught = true;
    }
    TEST_TRUE( caught );

    // special case: singleton PP
    caught = false;
    srand( 1234 );
    ppp.push_back( randpt( SZ ) );
    try
    {
        ppp.closest_pair();
    }
    catch (kjb::Illegal_argument&)
    {
        caught = true;
    }
    TEST_TRUE( caught );

    // iterate awhile to test the closest pair awhile
    for ( int iii = 0; iii < MM; ++iii ) {
        while ( int( ppp.size() ) < SZ )
            ppp.push_back( randpt( SZ ) );
        // standard quadratic search
        float mind_quad = std::numeric_limits< float >::max();
        for ( size_t ix1 = 0; ix1 < ppp.size(); ++ix1 )
            for ( size_t ix2 = 1 + ix1; ix2 < ppp.size(); ++ix2 )
                mind_quad = std::min( mind_quad, ppp[ ix1 ].dist( ppp[ ix2 ]));
        PixPath::const_iterator pa, pb;
        float mind_nlgn = ppp.closest_pair( &pa, &pb );
        TEST_APPROX_EQUALITY( mind_quad, mind_nlgn );
        TEST_APPROX_EQUALITY( mind_nlgn, pa -> dist( *pb ) );
        ppp.clear();
    }

    return EXIT_SUCCESS;
}


int test20()
{
    using kjb::qd::str_to_PixPoint;

    PixPoint a1 = str_to_PixPoint( "" );
    TEST_TRUE( a1.is_unused() );

    PixPoint a2 = str_to_PixPoint( "17,42" );
    TEST_TRUE( ! a2.is_unused() );
    TEST_TRUE( a2 == PixPoint( 17, 42 ) );
    PixPoint a3 = str_to_PixPoint( "17, 42" );
    TEST_TRUE( a2 == a3 );
    PixPoint a4 = str_to_PixPoint( "17 , 42" );
    TEST_TRUE( a2 == a4 );
    PixPoint a5 = str_to_PixPoint( "17   ,    42" );
    TEST_TRUE( a2 == a5 );

    TEST_TRUE( str_to_PixPoint( "cat, dog" ).is_unused() );
    TEST_TRUE( str_to_PixPoint( "17 monkey 42 ", "monkey" ) == a2 );
    TEST_TRUE( str_to_PixPoint( "17monkey42 ", "monkey" ) == a2 );
    TEST_TRUE( str_to_PixPoint( "17   42", "" ) == a2 );
    TEST_TRUE( str_to_PixPoint( "17   42", "\n" ) == a2 );

    // you screwed up:  comma is actual separator but you indicated whitespace
    TEST_TRUE( str_to_PixPoint( "17, 42", "" ).is_unused() );
    TEST_TRUE( str_to_PixPoint( "17, 42", " " ).is_unused() );
    TEST_TRUE( str_to_PixPoint( "17, 42", "\n" ).is_unused() );

    TEST_TRUE( str_to_PixPoint( "17, " ).is_unused() );
    TEST_TRUE( str_to_PixPoint( "17 " ).is_unused() );
    TEST_TRUE( str_to_PixPoint( "17" ).is_unused() );
    TEST_TRUE( str_to_PixPoint( "1" ).is_unused() );

    TEST_TRUE( str_to_PixPoint( "17, dog" ).is_unused() );
    TEST_TRUE( str_to_PixPoint( "cat, 42" ).is_unused() );
    TEST_TRUE( str_to_PixPoint( "17 monk 42 ", "monkey" ).is_unused() );
    TEST_TRUE( str_to_PixPoint( "17 monkey 42 ", "monk" ).is_unused() );
    TEST_TRUE( str_to_PixPoint( "17monkey42 ", " monkey" ).is_unused() );

    // surprising maybe?
    TEST_TRUE( str_to_PixPoint( "17, 42x" ) == a2 );
    TEST_TRUE( str_to_PixPoint( "17O42l", "O" ) == a2 );
    TEST_TRUE( str_to_PixPoint( "17monkey42monkey", "monkey" ) == a2 );

    return EXIT_SUCCESS;
}


int test21()
{
    PixPath p( PixPath::reserve() );
    kjb::qd::SvgWrap s( p );
    bool caught = false;
    try
    {
        s.set_id( "\"" );
    }
    catch( const kjb::Illegal_argument& )
    {
        caught = true;
    }
    TEST_TRUE( caught );

    caught = false;
    try
    {
        s.set_id( "\x1" );
    }
    catch( const kjb::Illegal_argument& )
    {
        caught = true;
    }
    TEST_TRUE( caught );
    return EXIT_SUCCESS;
}


int test22()
{
    PixPath p( PixPath::parse( "M 123,456 L 234,345 321,432 " ) );
    TEST_TRUE( 3 == p.size() );
    TEST_TRUE( PixPoint( 123, 456 ) == p[ 0 ] );
    TEST_TRUE( PixPoint( 234, 345 ) == p[ 1 ] );
    TEST_TRUE( PixPoint( 321, 432 ) == p[ 2 ] );

    bool caught = false;
    try
    {
        PixPath::parse( "M 123,456 L beef jerky" );
    }
    catch (kjb::Serialization_error&)
    {
        caught = true;
    }
    TEST_TRUE( caught );

    return EXIT_SUCCESS;
}


/*
 * tests for size(), hits(), pop_back(), front(), back(), begin(), rbegin()
 */
int test23()
{
    PixPath aaa( PixPath::reserve( 7 ) );
    aaa.push_back( PixPoint( 100,100 ) );
    aaa.push_back( PixPoint( 200,202 ) );
    aaa.push_back( PixPoint( 300,300 ) );
    aaa.push_back( PixPoint( 400,303 ) );
    aaa.push_back( PixPoint( 400,303 ) );
    aaa.push_back( PixPoint( 400,303 ) );

    TEST_TRUE( 6 == aaa.size() );
    TEST_TRUE( 3 == aaa.hits( PixPoint(400,303) ) );
    aaa.pop_back();
    TEST_TRUE( 5 == aaa.size() );
    TEST_TRUE( 2 == aaa.hits( PixPoint(400,303) ) );
    aaa.pop_back();
    TEST_TRUE( 4 == aaa.size() );
    TEST_TRUE( 1 == aaa.hits( PixPoint(400,303) ) );
    aaa.pop_back();
    TEST_TRUE( 3 == aaa.size() );
    TEST_TRUE( 0 == aaa.hits( PixPoint(400,303) ) );

    TEST_TRUE( PixPoint(300,300) == aaa.back() );
    TEST_TRUE( PixPoint(100,100) == aaa.front() );
    TEST_TRUE( PixPoint(300,300) == *aaa.rbegin() );
    TEST_TRUE( PixPoint(100,100) == *aaa.begin() );

    return EXIT_SUCCESS;
}


// test function copy_pixpoint_array()
int test24()
{
    PixPath aaa( PixPath::reserve() );
    aaa.push_back( PixPoint( 100,100 ) );
    aaa.push_back( PixPoint( 200,202 ) );
    aaa.push_back( PixPoint( 300,300 ) );
    aaa.push_back( PixPoint( 400,303 ) );
    aaa.push_back( PixPoint( 400,303 ) );
    aaa.push_back( PixPoint( 400,303 ) );
    TEST_TRUE( 6 == aaa.size() );

    PixPath bbb( copy_pixpoint_array( aaa.begin(), aaa.end() ) ),
            ccc( copy_pixpoint_array( aaa.rbegin(), aaa.rend() ) ),
            ddd( copy_pixpoint_array( aaa.begin()+2, aaa.end()-1 ) ),
            eee( copy_pixpoint_array( aaa.begin()+3, aaa.end()-3 ) );
    TEST_TRUE( bbb == aaa );
    TEST_TRUE( ccc.reverse() == aaa );
    TEST_TRUE( ddd == aaa.subrange(2,5) );
    TEST_TRUE( 0 == eee.size() );

    PixPoint fff[3] = {PixPoint(200,202),PixPoint(300,300),PixPoint(400,303)};
    TEST_TRUE( copy_pixpoint_array(fff, fff+3) == aaa.subrange(1,4) );

    return EXIT_SUCCESS;
}


#if 0
// test DoubleSegment methods
// parallelogram_area()
// is_collinear
// length
// length2
// is_intersecting_open
// is_intersecting_closed
/*   DISABLED B/C DOUBLESEGMENT IS UNDER SUSPICION OF REDUNDANCY */
int test25()
{
    typedef kjb::qd::DoubleSegment  DS;
    typedef kjb::qd::DoublePoint    DP;

    /*
     * By using small integer coordinate values, I hope to avoid any
     * floating-point roundoff errors.
     */
    const DS    s1( PixPoint(3, 4), PixPoint(4, 4) ),
                s2( PixPoint(4, 4), PixPoint(5, 5) ),
                s3( PixPoint(3, 3), PixPoint(4, 5) ),
                s4( PixPoint(1, 1), PixPoint(1, 1) );

    TEST_APPROX_EQUALITY( 0, s1.parallelogram_area( PixPoint(4,4) ) );
    TEST_APPROX_EQUALITY( 0, s1.parallelogram_area( PixPoint(44,4) ) );
    TEST_APPROX_EQUALITY( 0, s1.parallelogram_area( PixPoint(-4,4) ) );
    TEST_APPROX_EQUALITY( 1, s1.parallelogram_area( PixPoint(3,5) ) );
    TEST_APPROX_EQUALITY( 1, s1.parallelogram_area( PixPoint(4,5) ) );
    TEST_APPROX_EQUALITY( 1, s1.parallelogram_area( PixPoint(2,5) ) );

    TEST_TRUE( s1.is_collinear( PixPoint(3,4) ) );
    TEST_TRUE( s1.is_collinear( PixPoint(5,4) ) );
    TEST_TRUE( s1.is_collinear( PixPoint(0,4) ) );
    TEST_TRUE( ! s1.is_collinear( PixPoint(3,3) ) );
    TEST_TRUE( ! s1.is_collinear( PixPoint(0,3) ) );

    TEST_TRUE( s2.is_collinear( DP::zero() ) );
    TEST_TRUE( s2.is_collinear( PixPoint(10,10) ) );
    TEST_TRUE( ! s2.is_collinear( PixPoint(3,4) ) );

    TEST_TRUE(   s1.is_intersecting_open( s3 ) );
    TEST_TRUE( ! s1.is_intersecting_open( s2 ) );
    TEST_TRUE( ! s3.is_intersecting_open( s2 ) );
    TEST_TRUE(   s3.is_intersecting_open( s1 ) );
    TEST_TRUE(   s1.is_intersecting_closed( s3 ) );
    TEST_TRUE(   s1.is_intersecting_closed( s2 ) );
    TEST_TRUE( ! s3.is_intersecting_closed( s2 ) );
    TEST_TRUE(   s3.is_intersecting_closed( s1 ) );

    TEST_TRUE(   s1.is_intersecting_open( s1 ) );
    TEST_TRUE(   s2.is_intersecting_open( s2 ) );
    TEST_TRUE(   s3.is_intersecting_open( s3 ) );
    TEST_TRUE( ! s4.is_intersecting_open( s4 ) );      // open s4 is empty
    TEST_TRUE(   s1.is_intersecting_closed( s1 ) );
    TEST_TRUE(   s2.is_intersecting_closed( s2 ) );
    TEST_TRUE(   s3.is_intersecting_closed( s3 ) );
    TEST_TRUE(   s4.is_intersecting_closed( s4 ) );    // not empty closed

    TEST_TRUE( ! s1.is_intersecting_open( PixPoint(3, 4) ) );
    TEST_TRUE( ! s1.is_intersecting_open( PixPoint(4, 4) ) );
    TEST_TRUE( ! s1.is_intersecting_open( PixPoint(5, 4) ) );
    TEST_TRUE( ! s1.is_intersecting_open( PixPoint(2, 4) ) );
    TEST_TRUE( ! s1.is_intersecting_open( PixPoint(0, 0) ) );
    TEST_TRUE(   s1.is_intersecting_closed( PixPoint(3, 4) ) );
    TEST_TRUE(   s1.is_intersecting_closed( PixPoint(4, 4) ) );
    TEST_TRUE( ! s1.is_intersecting_closed( PixPoint(5, 4) ) );
    TEST_TRUE( ! s1.is_intersecting_closed( PixPoint(2, 4) ) );
    TEST_TRUE( ! s1.is_intersecting_closed( PixPoint(0, 0) ) );

    // we are still hoping for exact computations!
    DP m( PixPoint( 7, 8 ) );
    m = m * 0.5;
    TEST_TRUE(   s1.is_intersecting_open( m ) );
    TEST_TRUE( ! s2.is_intersecting_open( m ) );
    TEST_TRUE(   s3.is_intersecting_open( m ) );

    TEST_TRUE(!s1.is_intersecting_open(DS(PixPoint(1,4),PixPoint(2,4))));
    TEST_TRUE(!s1.is_intersecting_open(DS(PixPoint(2,4),PixPoint(3,4))));
    TEST_TRUE(!s1.is_intersecting_open(DS(PixPoint(4,4),PixPoint(5,4))));
    TEST_TRUE( s1.is_intersecting_open(DS(PixPoint(2,4),PixPoint(5,4))));
    TEST_TRUE( s1.is_intersecting_open(DS(PixPoint(3,4), m)) );
    TEST_TRUE( s1.is_intersecting_open(DS(m, PixPoint(4,4))) );
    TEST_TRUE( s1.is_intersecting_open(DS(m, PixPoint(5,4))) );
    TEST_TRUE(!s1.is_intersecting_closed(DS(PixPoint(1,4),PixPoint(2,4))));
    TEST_TRUE( s1.is_intersecting_closed(DS(PixPoint(2,4),PixPoint(3,4))));
    TEST_TRUE( s1.is_intersecting_closed(DS(PixPoint(4,4),PixPoint(5,4))));
    TEST_TRUE( s1.is_intersecting_closed(DS(PixPoint(2,4),PixPoint(5,4))));
    TEST_TRUE( s1.is_intersecting_closed(DS(PixPoint(3,4), m)) );
    TEST_TRUE( s1.is_intersecting_closed(DS(m, PixPoint(4,4))) );
    TEST_TRUE( s1.is_intersecting_closed(DS(m, PixPoint(5,4))) );

    TEST_APPROX_EQUALITY( 1, s1.length() );
    TEST_APPROX_EQUALITY( 2, s2.length2() );
    TEST_APPROX_EQUALITY( M_SQRT2, s2.length() );
    TEST_APPROX_EQUALITY( 5, s3.length2() );
    TEST_APPROX_EQUALITY( 0, s4.length2() );

    return EXIT_SUCCESS;
}
#endif



/// @brief launcher for our PixPath unit tests
int main2( int, const char* const* )
{
    typedef int (*PTest)(void);

    PTest suite[] = {   test1, test2, test3, test4, test5, test6, test7,
                        test8, test9, test10, test11, /*test12,*/ test13,
                        /*test14, test15,*/ test16, test17, test18, test19,
                        test20, test21, test22, test23, test24, /*test25,*/
                        00 };

    kjb::Temporary_File tf;
    FNAME = tf.get_filename();

    srand( 7654321 );
    for( PTest* p = suite; *p; ++p )
    {
        /*std::cerr << "test sequence " << p - suite << '\n';*/
        int rc = (*p)();
        if ( rc != EXIT_SUCCESS )
        {
            kjb_c::p_stderr( "Failure in test index %d.\n", p - suite );
            return rc;
        }
    }

    if ( kjb_c::is_interactive() )
    {
        kjb_c::pso( "Success!\n" );
    }

    RETURN_VICTORIOUSLY();
}

} // anonymous namespace

int main( int argc, const char* const* argv )
{
    KJB(EPETE(kjb_init()));
    int rc = main2( argc, argv );
    kjb_c::kjb_cleanup();
    return rc;
}

