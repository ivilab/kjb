/**
 * @file
 * @brief interactive demo of StochasticPriorityQueue
 * @author Andrew Predoehl
 *
 * Call this program with two output filenames, and optional options, i.e.,
 * $ test_spq [OPTIONS] FN1 FN2
 * The output filenames will be overwritten.  The output will be in SVG format.
 * Options are -w C for C in {0,1,2}, different options on weighting, default 0
 *             -n C for positive integer C, number of trajectories, default 25.
 */
/*
 * $Id: test_spq.cpp 20310 2016-02-01 11:32:25Z predoehl $
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* for getopt_long_only */
#endif

#include <l/l_def.h>
#include <i_cpp/i_hsv.h>

#include <qd_cpp/svgwrap.h>
#include <qd_cpp/stoprique.h>

extern "C" {
#include <getopt.h> /* for getopt_long_only */
}

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#ifndef SPQ_EXP
#define SPQ_EXP -3
#endif
#ifndef SPQ_ROOT
#define SPQ_ROOT 2
#endif

namespace {

using kjb::qd::PixPoint;
using kjb::qd::PixPath;
using kjb::Vector2;

const int   GRW = 250,  ///< width of grid
            GRH = GRW,  ///< height of grid
            DELTA = GRW/5;

const double sigma = 40; ///< gaussian "sigma" of hill

/// @brief alternative ways of weighting the edges in the 8-way grid graph
enum Flavor {
    UNIFORM,            ///< weights equal geometric distance in grid units
    WINDY,              ///< UNIFORM plus northwind or southwind of 0.3 units
    HILL,               ///< UNIFORM plus gradient of hill, worse if uphill
    END_FLAVOR,         ///< sentinel indicates end of flavor list
    MIN_FLAVOR=UNIFORM  ///< start of flavor list
};

struct Params {
    enum Flavor weight_flavor;  ///< selection of how graph is to be weighted
    int iter_count;             ///< number of stochastic trajectories per sink
    std::string filename_d,     ///< filename of deterministic output
                filename_s,     ///< filename of stochastic output
                whoami;         ///< filename of this file (from argv[0])
} g_params;                     ///< global params for invocation


/// @brief a table that associates a direction with a small 2-vector
class Compass : public PixPath {

public:
    enum{ NORTH, NORTHEAST, NORTHWEST, EAST, WEST, SOUTH, SOUTHEAST, SOUTHWEST,
        C8MAX, C8MIN=NORTH };

private:
    static PixPath build_compass()
    {
        PixPath compass( PixPath::reserve( C8MAX ) );
        // set up compass
        for( size_t iii = C8MIN; iii < C8MAX; ++iii )
            compass.push_back( PixPoint(0,0) );
        compass.assign( NORTH,      PixPoint( 0, 1) );
        compass.assign( SOUTH,      PixPoint( 0,-1) );
        compass.assign( EAST,       PixPoint( 1, 0) );
        compass.assign( WEST,       PixPoint(-1, 0) );
        compass.assign( NORTHEAST,  PixPoint( 1, 1) );
        compass.assign( NORTHWEST,  PixPoint(-1, 1) );
        compass.assign( SOUTHEAST,  PixPoint( 1,-1) );
        compass.assign( SOUTHWEST,  PixPoint(-1,-1) );
        KJB(ASSERT( ! compass.hits( PixPoint(0,0) ) ));
        return compass;
    }

public:
    Compass()
    :   PixPath( build_compass() )
    {}
};


/// @brief build, store eight directions to guide our steps around grid graph
const Compass compass;


/// @brief convert XY coordinate (not row-column) to row-major
PixPoint::Integer to_rm( const PixPoint& ppp )
{
    return ppp.x + ppp.y * GRW;
}


/// @brief convert row-major coordinate to XY coordinate (not row-column)
PixPoint to_xy( PixPoint::Integer rm )
{
    PixPoint::Integer   yy = rm / GRW,
                        xx = rm - yy * GRW;
    return PixPoint( xx, yy );
}


/**
 * This runs Dijkstra or quasi-Dijkstra on the grid graph, just once,
 * using weights of the flavor defined by g_params.
 * Then you can query the result.  For example, you can query
 * paths through the resulting tree structure, using
 * get_a_route_to.
 *
 * How do you want your priority queue to work?  Your choices here are
 * PQ = StochasticPriorityQueue< int >  for stochastic behavior
 * PQ = Redblack_subtree_sum< int >     for deterministic behavior
 */
template< typename PQ >
class HokieGraph
{
    /// @brief Dijkstra's algorithm in the segment graph
    void single_source_shortest_path();

public:
    typedef float Weight_tp;                ///< Weight of arcs
    typedef std::vector< std::pair< PixPoint::Integer, float > > VNeighb;
    typedef PQ priority_queue;              ///< type of queue
    typedef typename PQ::Key_tp Key_tp;     ///< type of keys in queue
    typedef typename PQ::Loc_tp Loc_tp;     ///< type of locators in queue

    const PixPoint m_source;                ///< pixel location of src seg
    std::vector< int > m_parent;            ///< Dijkstra tree structure
    std::vector< Key_tp > rm_to_dist;       ///< Dijkstra distance array

    void ft_adj8_neighbs( PixPoint::Integer, VNeighb* );


    /// @brief ctor needs the source segment and the Posterior compound
    HokieGraph( const PixPoint& source )
    :   m_source( source )
    {
        single_source_shortest_path();
    }

    /// @return access to the parent-of array defining the Dijkstra tree
    const std::vector< int >& get_parents() const
    {
        return m_parent;
    }

    /// @return access to the distance-to array computed by Dijkstra
    const std::vector< Key_tp >& get_distance() const
    {
        return rm_to_dist;
    }

    PixPath get_a_route_to( const PixPoint& ) const;
};



// weight is pure geometric distance
void get_weight_0( const PixPoint&, std::vector< float >* weight )
{
    KJB(ASSERT( weight ));

    for( size_t iii = Compass::C8MIN; iii < Compass::C8MAX; ++iii )
	{
        KJB(ASSERT( 0 < compass[ iii ].dist2() ));
        // geometric distance as baseline
        weight -> at( iii ) = compass[ iii ].dist();
    }
}


// northwind blowing in the left, right third columsn, southwind in middle col
void get_weight_1( const PixPoint& ppp, std::vector< float >* weight )
{
    get_weight_0( ppp, weight );

    /*
     * Create a "wind" from North, then South, then North in 3 vertical bands.
     *
     * kind of a stupid way to do it but whatevs.
     * If you make the effect +0.5 then southeast becomes
     * cheaper than east.
     */
    const double northwind =
                    ( 3 * ppp.x < GRW || 2 * GRW < 3 * ppp.x ? 0.3 : -0.3 );

    weight -> at( Compass::NORTH        ) += northwind;
    weight -> at( Compass::NORTHEAST    ) += northwind;
    weight -> at( Compass::NORTHWEST    ) += northwind;
    weight -> at( Compass::SOUTH        ) -= northwind;
    weight -> at( Compass::SOUTHEAST    ) -= northwind;
    weight -> at( Compass::SOUTHWEST    ) -= northwind;
}


// hill in the center of the grid
void get_weight_2( const PixPoint& ppp, std::vector< float >* weight )
{
    const Vector2 qqq( kjb::qd::to_vector2(ppp - PixPoint( GRW/2, GRH/2 ) ) );
    const double qm2 = qqq.magnitude_squared(),
                 mahald = qm2 / (sigma * sigma),
                 pdf = exp( -0.5 * mahald ), // but not a real pdf, that's cruel
                 mag_dpdf = -pdf * sqrt(mahald);
    const Vector2 grad( qqq * -pdf / sigma );

    get_weight_0( ppp, weight );

    for( size_t iii = Compass::C8MIN; iii < Compass::C8MAX; ++iii )
	{
        KJB(ASSERT( 0 < compass[ iii ].dist2() ));
#if 0
        // compute magnitude of gradient in this direction
        double hill_effect = kjb::dot(grad, kjb::qd::to_vector2(compass[iii]));
        // reduce the effect for going downhill (equiv. to penalty for uphill)
        if ( hill_effect < 0 ) hill_effect *= 0.8;
        weight -> at( iii ) += hill_effect;
#else
        // unit vector version of qqq (except it is zero at exact center point)
        const Vector2 uq = qm2 > 0 ? qqq/sqrt(qm2) : qqq;
        double hill_effect = mag_dpdf * kjb::dot(uq, kjb::qd::to_vector2(compass[iii]));
        //KJB(ASSERT(       1==compass[iii].dist2() ||  2==compass[iii].dist2() ));
        hill_effect *= sqrt(compass[iii].dist2() );
        hill_effect *= 0.8; // shrink the effect, it might be too strong.
        weight -> at( iii ) += hill_effect;
#endif
        // combine hill effect with mere geometric distance
        KJB(ASSERT( 0 < weight -> at( iii ) ));
    }
}


inline
void get_weight( const PixPoint& ppp, std::vector< float >* weight )
{
    if ( UNIFORM == g_params.weight_flavor )
        get_weight_0( ppp, weight );
    else if ( WINDY == g_params.weight_flavor )
        get_weight_1( ppp, weight );
    else if ( HILL == g_params.weight_flavor )
        get_weight_2( ppp, weight );
    else
        KJB(ASSERT( false ));
}


/// @brief vec of pixels adj to rm w/in the image boundaries, plus distances.
template< typename PQ >
void HokieGraph< PQ >::ft_adj8_neighbs(
    PixPoint::Integer rm,
    VNeighb* neighbs
)
{
    KJB(ASSERT( neighbs ));
    neighbs -> clear();
    const PixPoint ppp( to_xy( rm ) );
    const bool  firstrow = ( ppp.y <= 0 ),
                lastrow = ( GRH <= ppp.y + 1 ),
                firstcol = ( ppp.x <= 0 ),
                lastcol = ( GRH <= ppp.x + 1 );

    std::vector< float > weight( Compass::C8MAX );
    get_weight( ppp, & weight );

    // go west?
    if ( ! firstcol )
        neighbs -> push_back( std::make_pair( rm - 1, weight[ Compass::WEST]));

    // go east?
    if ( ! lastcol )
        neighbs -> push_back( std::make_pair( rm + 1, weight[ Compass::EAST]));

    // go north? northwest? northeast?
    if ( ! firstrow ) {
        const PixPoint::Integer north = rm - GRW;
        neighbs -> push_back( std::make_pair( north, weight[ Compass::NORTH]));
        if ( ! firstcol )
            neighbs -> push_back( std::make_pair( north - 1,
                                                weight[ Compass::NORTHWEST ]));
        if ( ! lastcol )
            neighbs -> push_back( std::make_pair( north + 1,
                                                weight[ Compass::NORTHEAST ]));
    }

    // go south? southwest? southeast?
    if ( ! lastrow ) {
        const PixPoint::Integer south = rm + GRW;
        neighbs -> push_back( std::make_pair( south, weight[ Compass::SOUTH]));
        if ( ! firstcol )
            neighbs -> push_back( std::make_pair( south - 1,
                                                weight[ Compass::SOUTHWEST ]));
        if ( ! lastcol )
            neighbs -> push_back( std::make_pair( south + 1,
                                                weight[ Compass::SOUTHEAST ]));
    }
    std::random_shuffle( neighbs -> begin(), neighbs -> end() );
}



// this implementation of dijkstra's algorithm navigates through pixels
template< typename PQ >
void HokieGraph< PQ >::single_source_shortest_path()
{
    typedef VNeighb::const_iterator VNCI;
    const int NO_PIX = -1;
    Loc_tp NO_LOC = 0;

    m_parent.assign( GRW * GRH, NO_PIX );
    // set up the distance array
    rm_to_dist.assign( GRW * GRH,
                        std::numeric_limits< typename PQ::Key_tp >::max() );
    const size_t rmsrc = m_source.x + m_source.y * GRW;
    rm_to_dist.at( rmsrc ) = 0;

    // fill the priority queue, store the resulting locators
    std::vector< Loc_tp > rm_to_loc( GRW * GRH, NO_LOC );
    PQ pq;
    for ( size_t iii = 0; iii < GRW * GRH; ++iii )
	{
        Loc_tp loc;
        if ( rmsrc == iii )
		{
            loc = pq.insert( 0, iii );
		}
        else
		{
            loc = pq.ins_max_key( iii );
		}
        rm_to_loc[ iii ] = loc;
        KJB(ASSERT( loc != NO_LOC ));
    }

    VNeighb neighbs;
    while ( pq.size() )
	{
        // extract min elt
        const Loc_tp lowloc = pq.Dijkstra_extraction();
        KJB(ASSERT( lowloc != NO_LOC ));
        int vert_rm = NO_PIX;
        Key_tp energy;
        pq.access_loc( lowloc, & energy, & vert_rm );
        KJB(ASSERT( vert_rm != NO_PIX ));
        bool hit = pq.erase_loc( lowloc );
        KJB(ASSERT( hit ));
        KJB(ASSERT( lowloc == rm_to_loc.at( vert_rm ) )); // loc records shd be ok
        /*
         * We have to erase this vertex's locator to keep track of the fact
         * that it is no longer a member of the priority queue.
         */
        rm_to_loc.at( vert_rm ) = NO_LOC;
        // relax neighbors
        ft_adj8_neighbs( vert_rm, & neighbs );
        for ( VNCI qqq = neighbs.begin(); qqq != neighbs.end(); ++qqq ) {
            /* disregard the supposed neighbor if
             * pixel has already been extracted earlier from pq
             */
            const PixPoint::Integer neighb_rm( qqq -> first );
            if ( NO_LOC == rm_to_loc.at( neighb_rm ) )
                continue;
            // Extract the neighbor's key, i.e., its distance from origin.
            const Loc_tp neighb_loc( rm_to_loc.at( neighb_rm ) );
            Key_tp neighb_key;
            int nrm = neighb_rm + 1; // fill with a wrong value
            bool hit2 = pq.access_loc( neighb_loc, & neighb_key, & nrm );
            KJB(ASSERT( hit2 ));
            // propose a new route to the neighbor: via vert_rm! Great eh?
            const Key_tp d_new = rm_to_dist.at( vert_rm ) + qqq -> second;
            // if that is great, then update (otherwise, awkwardly ignore)
            if ( d_new < neighb_key ) {
                // decrease key
                rm_to_dist.at( neighb_rm ) = d_new;
                bool itsok = pq.rekey_loc( neighb_loc, d_new );
                KJB(ASSERT( itsok ));
                // update parent array
                m_parent.at( neighb_rm ) = vert_rm;
            }
        }
    }
}



/// @brief trace a connected pixel path in Dijkstra tree of pixel graph
template< typename PQ >
PixPath HokieGraph< PQ >::get_a_route_to( const PixPoint& destination ) const
{
    PixPath course( PixPath::reserve() );
    for( PixPoint ppp = destination; ppp != m_source;
                                ppp = to_xy( m_parent.at( to_rm( ppp ) ) ) )
        course.push_back( ppp );
    KJB(ASSERT( course.back() != m_source ));
    course.push_back( m_source );
    return course.ow_reverse();
}



/// @brief emit a test message and return a failure code
int fail( const char* msg )
{
    std::cerr << '(' << g_params.whoami << ") Error: " << msg << '\n';
    return EXIT_FAILURE;
}



std::string generate_svg_preamble()
{
    std::ostringstream intro;
    intro   << kjb::qd::SvgWrap::xml_header()
            <<  "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
                "width=\"" << GRW*3 << "px\" "
                "height=\"" << GRH*3 << "px\" "
                "viewBox=\"0 0 " << GRW << ' ' << GRH << "\">\n";

    // draw (if necessary) a graphical cue that hints at the weight structure
    if ( WINDY == g_params.weight_flavor )
        intro   // northwind box along west edge
                << "<rect x=\"0\" y=\"0\" width=\"" << GRW/3 << "\" "
                    "height=\"" << GRH << "\" fill=\"LightCyan\" />\n"
                // northwind box along east edge
                << "<rect x=\"" << 2*GRW/3 << "\" y=\"0\" "
                    "width=\"" << GRW/3 << "\" "
                    "height=\"" << GRH << "\" fill=\"LightCyan\" />\n";
    else if ( HILL == g_params.weight_flavor )
        intro   // hill contour at two-sigma
                << "<circle cx=\"" << GRW/2 << "\" cy=\"" << GRH/2 << "\" "
                    "r=\"" << 2*sigma << "\" fill=\"Azure\" />\n"
                // hill contour at one-sigma
                << "<circle cx=\"" << GRW/2 << "\" cy=\"" << GRH/2 << "\" "
                    "r=\"" << 1*sigma << "\" fill=\"LightCyan\" />\n"
                /* hill contour at one sigma
                << "<circle cx=\"" << GRW/2 << "\" cy=\"" << GRH/2 << "\" "
                    "r=\"" << 1*sigma << "\" fill=\"aliceblue\" />\n"*/;
    else
	{
        KJB(ASSERT( UNIFORM == g_params.weight_flavor ));
	}

    return intro.str();
}



int path_test( std::ostream& df, std::ostream& sf, const PixPath& termini )
{
    using kjb::qd::Redblack_subtree_sum;
    using kjb::qd::StochasticPriorityQueue;
    using kjb::qd::SvgWrap;

    const PixPoint source( DELTA/2, GRH/2 ) /*, terminus( 225,125 )*/;

    // deterministic
    HokieGraph< Redblack_subtree_sum< int > > hg1( source );

    // Get opening verbage of SVG, including graphical hint at weight structure
    const std::string preamble( generate_svg_preamble() );

    // build a color table
    std::vector< kjb_c::Pixel > colortab;
    for( size_t iii = 0; iii < termini.size(); ++iii )
	{
        const kjb::PixelHSVA ppp( iii / ( 1.1 * termini.size() ), // hue
                                    0.8, 0.8 ); // saturation, value
        colortab.push_back( ppp );
    }

    // generate deterministic tree
    df << preamble;
    for( size_t iii = 0; iii < termini.size(); ++iii )
	{
        PixPath pp1( hg1.get_a_route_to( termini[ iii ] ) );
        SvgWrap svg( pp1.merge_redundant_slopes() );
        svg.set_segs(1).set_text(0).set_xml(0).set_svg(0);
        svg.set_color( colortab.at( iii ) );
		std::ostringstream sid;
		sid << "<g id='sink_" << iii << "_distance_"
			<< hg1.get_distance().at( to_rm( termini[ iii ] ) )
			<< "'>\n";
		df	<< sid.str()
			<< svg()
			<< "\n</g>\n";
        std::cout << "opt to " << termini[ iii ].str() << " = "
                << hg1.get_distance().at( to_rm( termini[ iii ] ) ) << '\n';
    }
    df << "</svg>" << std::endl; // endl because we want a flush() now.

    // generate trajectories of stochastic tree
    typedef HokieGraph< StochasticPriorityQueue< int, SPQ_EXP, SPQ_ROOT > > Hgspq;
    sf << preamble;
    for ( int jjj = 0; jjj < g_params.iter_count; ++jjj )        // trajectories
	{
		Hgspq hg3( source );
        for ( size_t iii = 0; iii < termini.size(); ++iii )      // termini
		{
            PixPath pp3( hg3.get_a_route_to( termini[ iii ] ) );
            SvgWrap svg( pp3.merge_redundant_slopes() );
            svg.set_segs(1).set_text(0).set_xml(0).set_svg(0);
            svg.set_color( colortab.at( iii ) );
            //Hgspq::priority_queue q;
			std::ostringstream sid;
			sid << "<g id='sink_" << iii << "_distance_"
				<< hg3.get_distance().at( to_rm( termini[ iii ] ) )
				<< "'>\n";
            sf	<< sid.str()
				<< svg()
				<< "\n</g>\n";
				//<< "\n" /*"<!-- power_law() = " << q.power_law() << " -->\n"*/;
            std::cout   << "other-" << jjj << " to "
                        << termini[ iii ].str()
#if 0
						<< " = "
                        << hg3.get_distance().at( to_rm( termini[ iii ] ) )
                        /* << " power_law()=" << q.power_law() */
#endif
                        << '\n';
        }
    }
    sf << "</svg>\n";

    return EXIT_SUCCESS;
}



int read_options( int argc, char** argv )
{
    g_params.weight_flavor = UNIFORM;   // default weighting is uniform weights
    g_params.iter_count = 25;           // default number of iterations is 25

    const struct option opz[ 3 ] = {    { "weight", 1, 00, 'w' },
                                        { "iters", 1, 00, 'i' },
                                        { 0,0,0,0 } };

    for( int ccc;
            EOF != ( ccc = getopt_long_only( argc, argv, "i:w:", opz, 0 )); )
	{

        if ( 'w' == ccc )
		{
            int flav;
            if (    1 != sscanf( optarg, "%d", & flav )
                ||  flav < MIN_FLAVOR
                ||  END_FLAVOR <= flav
               )
			{
                return fail( "Weight choice is not valid (must be 0, 1, 2)." );
			}
            g_params.weight_flavor = static_cast< enum Flavor >( flav );
        }
        else if ( 'i' == ccc )
		{
            if (    1 != sscanf( optarg, "%d", & g_params.iter_count )
                ||  g_params.iter_count < 0
               )
			{
                return fail( "Iters count is invalid or negative." );
			}
        }
        else
		{
            return fail(
                    "Bad invocation.\n"
                    "Synopsis:\ttest_spq [OPTIONS] file1 file2\n"
                    "\nfile1 is the filename in which to store SVG "
                    "visualization of deterministic\n\tpaths.\n"
                    "\nfile2 is the filename in which to store SVG "
                    "visualization of stochastic paths.\n"
                    "\nOptions:\t--weight W \tW must be 0, 1, or 2.\n"
                    "\t\t\t\t0 means uniform weighting in grid (default).\n"
                    "\t\t\t\t1 means there is a \"wind\" blowing.\n"
                    "\t\t\t\t2 means there is a \"hill\" in the middle.\n"
                    "\t\t--iters N \tN must be a non-negative integer, "
                    "equal to the\n"
                    "\t\t\t\tdesired number of stochastic path trajectories\n"
                    "\t\t\t\trequested.  Default is N=25.\n"
                    "\t\t-w W\t\tSame as --weight W.\n"
                    "\t\t-i N\t\tSame as --iters N.\n"
                );
		}
    }

    // after the options, there should be exactly two filenames
    if ( argc - optind != 2 )   // optind is a global provided by getopt.h
	{
        return fail( "program needs two args -- filenames for SVG outputs" );
	}

    // filenames should be distinct, except that they both may be /dev/null.
    g_params.filename_d = argv[ optind ];
    g_params.filename_s = argv[ 1 + optind ];
    if (        g_params.filename_d == g_params.filename_s
            &&  g_params.filename_d != "/dev/null"
        )
	{
        return fail( "filenames are not distinct" );
	}
    return EXIT_SUCCESS;
}


}

/// @brief launch the demonstration of Dijkstra + stochastic priority queue
int main( int argc, char** argv )
{
    // assume the program was invoked with at least its own filename
    KJB(ASSERT( 0 < argc ));
    g_params.whoami = argv[ 0 ];

    // Read options and filenames
    int rc1 = read_options( argc, argv );
    if ( rc1 != EXIT_SUCCESS )
	{
        return rc1;
	}

    // set up the termini (the endpoints of the tree)
    PixPath termini( PixPath::reserve( 11 ) );
    for( int xxx = DELTA; xxx < GRW; xxx += DELTA )
	{
        termini.push_back( PixPoint( xxx, DELTA/2 ) );
        termini.push_back( PixPoint( xxx, GRH-DELTA/2 ) );
    }
#if 0
    for( int yyy = 3*DELTA/2; yyy < GRH-DELTA; yyy += DELTA )
        termini.push_back( PixPoint( GRW-25, yyy ) );
#else
	termini.push_back( PixPoint( GRW-25, GRH/2 - 3*DELTA/2 ) );
	termini.push_back( PixPoint( GRW-25, GRH/2 + 3*DELTA/2 ) );
	termini.push_back( PixPoint( GRW-25, GRH/2             ) );
	/*
    for( int yyy = -1; yyy <= 1; ++yyy )
	{
        termini.push_back( PixPoint( GRW-25, GRH/2 + yyy * 3*DELTA/2 ) );
	}
	*/
#endif
    termini.ow_reverse();

    // open the output files, df for deterministic & sf for stochastic
    std::ofstream df( g_params.filename_d.c_str() ),
                  sf( g_params.filename_s.c_str() );
    if ( ! df )
	{
        return fail( "could not open first output file" );
	}
    if ( ! sf )
	{
        return fail( "could not open second output file" );
	}

    return path_test( df, sf, termini );
}

