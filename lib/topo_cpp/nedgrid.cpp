/**
 * @file
 * @brief interface for low-level NED reader functions
 * @author Andrew Predoehl
 *
 * This performs interpolation on NED data using Gaussian processes, a
 * technique historically called "kriging."  This file uses its own
 * implementation rather than the libkjb Gaussian process code (sorry).
 */
/*
 * $Id: nedgrid.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "l/l_sys_io.h"
#include "l/l_debug.h"
#include "l_cpp/l_exception.h"
#include "l_cpp/l_stdio_wrap.h"
#include "l_cpp/l_heartbeat.h"
#include "m_cpp/m_matrix_d.h"
#include "m_cpp/m_vector_d.h"
#include "topo_cpp/LatLong-UTMconversion.h"
#include "topo_cpp/nedget.h"
#include "topo_cpp/nedgrid.h"
#include "topo_cpp/kriging.h"
#include "l_cpp/l_util.h"

#include <cmath>
#include <sstream>
#include <map>

#include <boost/scoped_ptr.hpp>


// Macro symbol DEBUG_VIEW_TRAINING is useful for developers and maintainers.
#define DEBUG_VIEW_TRAINING 0

#if DEBUG_VIEW_TRAINING
#include <i_cpp/i_image.h>
#include <i_cpp/i_collage.h>
#include <i_cpp/i_matrix.h>
#endif


/* Macro symbol NED_THREADS_COUNT is a somewhat crude way to support
 * parallelization of the kriging procedure, using pthreads.
 * If you lack pthreads then obviously we cannot use this feature.
 * Also if you are running a TEST compilation we cannot use this feature
 * because kjb_malloc(), used down deep in the library, is not thread safe
 * when compiled with macro TEST true (which enables static tracking).
 * The value of sixteen was chosen half-capriciously, because a single El Gato
 * node has sixteen cores.
 */
#ifndef NED_THREADS_COUNT
#if defined(KJB_HAVE_PTHREAD) && ! defined(TEST)
#define NED_THREADS_COUNT 16 /* Number of threads used, when we parallelize. */
#else
#define NED_THREADS_COUNT 1 /* NO_LIBS or no threads or development mode */
#endif
#endif


// Here is the standard idiom to test whether running multithread:
#if NED_THREADS_COUNT > 1
#include <l_mt_cpp/l_mt_mutexlock.h>
#endif


namespace
{

#if NED_THREADS_COUNT > 1
kjb::Pthread_mutex grid_cache_serialization;
#endif


typedef boost::scoped_ptr< std::vector< std::string > > AutoPath;

typedef kjb::TopoFusion::pt TopoPt;

// Name stands for "Grid integral latitude, longitude [pair]."
typedef kjb::Ned13_one_degree_grid::IntegralLL GILL;

// Name stands for "multimap keyed on [grid] integral lat-longitude [pairs]."
typedef std::multimap< GILL, std::pair<TopoPt, size_t> > MMILL;

typedef MMILL::const_iterator MMCI;

const bool VERBOSE = false;

const char* WEST_IS_NEGATIVE = "Longitudes in the USA are negative";

const std::string NED13_LOCAL_STORE = "/net/v04/data_3/trails/"
                                          "elevation/ned_13as/llgrid";

const size_t    SIZE_DEGREES = 1,       // USGS NED13 tiles are 1 deg/edge long
                ARCMIN_PER_DEGREE = 60,
                SIZE_ARCMIN = SIZE_DEGREES * ARCMIN_PER_DEGREE, // edge length
                ARCSEC_PER_ARCMIN = 60,
                BONUS_ARCSEC = 4,       // actually, USGS provides 4" extra
                SIZE_ARCSEC = SIZE_ARCMIN*ARCSEC_PER_ARCMIN + BONUS_ARCSEC,
                SIZE_THIRD_ARCSEC = SIZE_ARCSEC * 3, // one-3rd arcseconds/edge
                GRID_VOLUME = SIZE_THIRD_ARCSEC * SIZE_THIRD_ARCSEC,
                THIRD_ARCSEC_PER_DEG = 3*ARCMIN_PER_DEGREE*ARCSEC_PER_ARCMIN;


/*
 * If you request elevation data in a grid that crosses zone boundaries, we
 * will accommodate by "stretching" the standard limits of the zone of the
 * grid center, which is accurate enough for small stretches.  But if you ask
 * to stretch by too much, we will throw an error because the noneuclidean
 * distortion threatens to give bad results.
 * The value here, of 40 kilometers, is based on a Wikipedia article on UTM.
 */
const double TOO_MUCH_ZONE_STRETCH_METERS = 40000;


// nonsense value: it lacks semantic purpose
const GILL whatever = GILL::from_lat_lon(45, -90);



// return true iff the given filename corresponds to a file that exists.
inline bool file_exists(const std::string& filename)
{
    using namespace kjb_c;
    return TRUE == is_file(filename.c_str());
}


// determine the "proper" zone for a given UTM point (which might be fudged).
inline char zone_of(const TopoPt& p)
{
    return kjb::TopoFusion::zone_of(kjb::NED_ELLIPSOID, p);
}


#if DEBUG_VIEW_TRAINING
// validate the input parameters, possibly throwing, possibly printing err msgs
void zone_validation(
    const TopoPt& center,
    const TopoPt& nw_corner,
    const TopoPt& ne_corner
)
{
    if (zone_of(center) != center.zone)
    {
        KJB_THROW_2(kjb::Illegal_argument, "Input point has improper zone");
    }

    /*
     * If you overflow your zone by 40 kilometers or less, we complain but
     * put up with it.  If you exceed 40 km, that is too much.
     * The 40 km threshold is based on the Wikipedia article about UTM coords.
     */
    if (zone_of(nw_corner) != zone_of(ne_corner))
    {
        if (ne_corner.x - nw_corner.x > TOO_MUCH_ZONE_STRETCH_METERS)
        {
            /*
             * You can make large requests, but they must fit within one UTM
             * zone.  When large requests span utm zones, the distortion is
             * too much.  Small leakage into a nearby zone causes but small
             * distortion.  Large leakage, though, is a problem.
             */
            KJB_THROW_2(kjb::Illegal_argument, "Grid request spans UTM zones "
                    "and is too large to give accurate results.");
        }
        else
        {
            KJB(TEST_PSE(("Warning:  elevation grid requested spans UTM "
               "zones.\n\t\tElevation accuracy might be poorer than usual.")));
        }
    }
}



std::vector< TopoPt > get_training_elevation_data(
    const TopoPt& center,
    int ew_size_m,
    int ns_size_m, 
    kjb::Ned13_grid_cache* cache
)
{
    ASSERT(cache);

    TopoPt nw_corner(center);
    nw_corner.x -= ew_size_m / 2;
    nw_corner.y += ns_size_m / 2;
    TopoPt ne_corner(nw_corner);
    ne_corner.x += ew_size_m;

    TopoPt sw_corner(nw_corner);
    sw_corner.y -= ns_size_m;
    TopoPt se_corner(sw_corner);
    se_corner.x = ne_corner.x;

    zone_validation(center, nw_corner, ne_corner); // this might throw

    return fill_training_elevation_grid(
            kjb::utm_to_se_ned13_ill(nw_corner),
            kjb::utm_to_se_ned13_ill(se_corner),
            cache,
            0
        );
}



kjb::Image debug_view_ned_training_data(
    const std::vector< std::vector< TopoPt > >& training
)
{
    const int   ROWS = int(training.size()),
                COLS = int(training.front().size());
    if (0 == training.size()) return kjb::Image();

    kjb::Matrix m[4];

    for (int iii=0; iii<4; ++iii)
    {
        m[iii].resize(ROWS, COLS);
    }

    for (int rrr = 0; rrr < ROWS; ++rrr)
    {
        for (int ccc = 0; ccc < COLS; ++ccc)
        {
            const TopoPt &p = training.at(rrr).at(ccc);
            m[0].at(rrr, ccc) = p.x;
            m[1].at(rrr, ccc) = p.y;
            m[2].at(rrr, ccc) = p.zone;
            m[3].at(rrr, ccc) = p.ele;
        }
    }
    kjb::Image i[4];
    for (int iii=0; iii<4; ++iii)
    {
        i[iii] = kjb::matrix_to_max_contrast_8bit_bw_image(m[iii]);
    }

    return kjb::make_collage(i, 2, 2);
}
#endif



const std::vector< std::string >& path_setup(
    const std::vector< std::string >& path,
    AutoPath& default_path_ap
)
{
    if (path.size()) return path;

    default_path_ap.reset( new std::vector< std::string >() );
    default_path_ap -> push_back( NED13_LOCAL_STORE );
    return * default_path_ap.get();
}


std::string get_filename(
    const std::string& latlon,
    const std::vector< std::string >& path
)
{
    for (size_t iii = 0; iii < path.size(); ++iii)
    {
        std::string zip_fn = path.at(iii) + DIR_STR + latlon + ".zip";
        if (file_exists(zip_fn)) return zip_fn;
    }
    KJB_THROW_2(kjb::IO_error, "File " + latlon + " not found");
}


// generate the filename for the zip archive that we want to draw from
std::string get_latlon(const GILL& nw_corner)
{
    std::ostringstream fn;
    fn  << 'n' << +nw_corner.ilat / THIRD_ARCSEC_PER_DEG
        << 'w' << -nw_corner.ilon / THIRD_ARCSEC_PER_DEG;
    return fn.str();
}





/*
 * In this context a "cell" is a small, more-or-less rectangular region of the
 * client's grid, with a bunch of query points (like 80 to 100), who all share
 * a common reference point in the DEM grid.
 *
 * What does "a common reference point" mean?  That means utm_to_se_ned13_ill()
 * returns the same answer for all of them.
 *
 * std::multimap makes this so easy.  We can store by reference point, and
 * later we can compute all points.
 */
MMILL get_cell_identities(
    const TopoPt& center,
    int eastwest_size_meters,
    int northsouth_size_meters,
    int resolution_meters
)
{
    TopoPt nw(center), cursor(center);
    nw.x -= eastwest_size_meters/2;
    nw.y += northsouth_size_meters/2;

    MMILL cells;
    size_t rmix = 0; // row major (northing-major) index through grid
    for (int y = 0; y < northsouth_size_meters; y += resolution_meters)
    {
        cursor.y = nw.y - y;
        for (int x = 0; x < eastwest_size_meters; x += resolution_meters)
        {
            cursor.x = nw.x + x;
            cells.insert( std::make_pair(   kjb::utm_to_se_ned13_ill(cursor), 
                                            std::make_pair(cursor, rmix)
                ));
            ++rmix;
        }
    }

    KJB(ASSERT(    (northsouth_size_meters/resolution_meters)
                 * (eastwest_size_meters/resolution_meters)
                == int(rmix)
            ));

    return cells;
}








int grid_validate(int ew_sz, int ns_sz, int res)
{
    using namespace kjb_c;
    if (ew_sz < 1)
    {
        add_error("Bad east-west size of %d in ned13_grid()", ew_sz);
        return ERROR;
    }
    if (ns_sz < 1)
    {
        add_error("Bad north-south size of %d in ned13_grid()", ns_sz);
        return ERROR;
    }
    if (res < 1)
    {
        add_error("Bad resolution of %d in ned13_grid()", ns_sz);
        return ERROR;
    }
    return NO_ERROR;
}


bool fetch_key(const MMILL& cells, GILL* key)
{
    if (cells.empty()) return false; // cannot fetch a key
    *key = cells.begin() -> first;
    return true; // true means we successfully fetched a key
}


// check that the indices field is all unique points.
bool sanity_check(const MMILL& cells)
{
    std::vector<size_t> indices;
    indices.reserve(cells.size());

    for (MMCI i = cells.begin(); i != cells.end(); ++i)
    {
        indices.push_back(i -> second.second);
    }
    std::sort(indices.begin(), indices.end());
    return std::adjacent_find(indices.begin(), indices.end()) == indices.end();
}


int interpolate_elevation(
    MMILL* cells,
    kjb::Ned13_grid_cache* cache,
    kjb::Matrix* elev_o,
    kjb::Matrix* elev_de_o,
    kjb::Matrix* elev_dn_o,
    bool verbose
)
{
    const double CSF2=1/kjb::Ned13_gp_reader::characteristic_length_squared();

    kjb::Heartbeat heart("kriging the elevation model", cells -> size(), 4);
    for (GILL key(whatever); fetch_key(*cells, &key); cells -> erase(key))
    {
        // The kriger object performs interpolation near the key DEM point.
        const kjb::Kriging_interpolator
#if NED_THREADS_COUNT > 1
                kriger(CSF2, key, cache, &grid_cache_serialization);
#else
                kriger(CSF2, key, cache);
#endif

        const std::pair<MMCI, MMCI> range = cells -> equal_range(key);

        for (MMCI i = range.first; i != range.second; ++i)
        {
            ASSERT(!(i->first < key) && !(key < i->first));
            if (verbose) heart.beat();
            const std::pair<TopoPt, size_t>& loc = i -> second;
            if (elev_o)    elev_o -> at(loc.second) = kriger(loc.first);
            if (elev_de_o) elev_de_o->at(loc.second)=kriger.diff_e(loc.first);
            if (elev_dn_o) elev_dn_o->at(loc.second)=kriger.diff_n(loc.first);
        }
    }
    if (verbose) heart.stop();

    return kjb_c::NO_ERROR;
}


struct Interpolation_task
{
    MMILL* cells;
    kjb::Ned13_grid_cache* cache;
    kjb::Matrix* elev_o;
    kjb::Matrix* elev_de_o;
    kjb::Matrix* elev_dn_o;
    bool verbose;
};


// thread worker function
void* interp_worker(void* vp)
{
    KJB(NRN(vp));
    Interpolation_task* tp = (Interpolation_task*) vp;
    KJB(ERN(interpolate_elevation(tp->cells, tp->cache, tp->elev_o,
                           tp->elev_de_o, tp->elev_dn_o, tp->verbose)));
    return vp;
}


#if NED_THREADS_COUNT > 1
int multithread_interp_el(
    MMILL* cells,
    kjb::Ned13_grid_cache* cache,
    kjb::Matrix* elev_o,
    kjb::Matrix* elev_de_o,
    kjb::Matrix* elev_dn_o,
    size_t threadcount
)
{
    typedef MMILL::iterator Iter;
    KJB(NRE(cells));

    if (threadcount < 2)
    {
        kjb_c::add_error("threadcount %d expected to be >= 2", threadcount);
        return kjb_c::ERROR;
    }

    // partition *cells
    std::vector< MMILL > vcells(threadcount);
    size_t index = 0;
    while (! cells -> empty())
    {
        for (GILL key(whatever); fetch_key(*cells, &key); cells -> erase(key))
        {
            const std::pair<Iter, Iter> range = cells -> equal_range(key);
            vcells[index].insert(range.first, range.second);
            cells -> erase(range.first, range.second);
            index = (1+index) % threadcount;
        }
    }

    // create tasks
    const Interpolation_task DETA ={0, cache, elev_o, elev_de_o, elev_dn_o, 0};
    std::vector<Interpolation_task> vtask(threadcount, DETA);

    // launch threads
    std::vector< kjb_c::kjb_pthread_t > vtid(threadcount);
    for (size_t i = 0; i < threadcount; ++i)
    {
        vtask.at(i).cells = & vcells.at(i);
        vtask.at(i).verbose = (VERBOSE && 0==i);
        void* vp = (void*) & vtask.at(i);
        KJB(ERE(kjb_pthread_create(& vtid.at(i), 0, interp_worker, vp)));
    }

    // join threads
    for (size_t i = 0; i < threadcount; ++i)
    {
        void* vp;
        KJB(ERE(kjb_pthread_join(vtid.at(i), &vp)));
        KJB(NRE(vp));
    }

    return kjb_c::NO_ERROR;
}
#endif


void validate_southeast_enough(
    const TopoPt& query,
    const GILL& gill_se,
    const TopoPt& q_se)
{
    /*
     * When we quantize an arbitrary UTM query point q to a grid point p, we
     * try to find the point p on the grid minimizing distance(p-q), but also
     * constrained so p lies in the closed southeast quadrant originating
     * at q.  In other words, p lies east or south of q (or exactly on q, if we
     * are so lucky), and no other southeastern grid point is closer to q.
     *
     * When we convert p to UTM representation, usually the UTM coordinates
     * also indicate that p does indeed lie in the closed southeastern
     * quadrant, lying to the east by up to 8 meters or so.
     * But sometimes there is conflicting evidence:  it appears that
     * occasionally, p lies several inches WEST of q, according to the UTM
     * coordinates.  I do not understand exactly why, but it does not bother
     * me much.   This suggests that the utm_to_lat_long() and LLtoUTM()
     * functions are not exact inverses of each other.  An error of a foot or
     * so is not, I think, going to cause a significant problem.  Hopefully.
     *
     * Easting err btw 12.0 - 12.1 cm obsv. in Bright Angel Gorge, Grand Canyon
     * Easting err btw 28.8 - 28.9 cm obsv. near utm(e=219381, n=3639207, z=13)
     */
    const double TOLERABLE_EASTING_ERROR_METERS = 0.289; // around 29 cm

    /* Similar to above, truncation intended to lie to the south has
     * sometimes been reprojected to be a small distance north of the
     * original point.
     *
     * Northing err less than 1 cm observed near Pike's Peak.
     */
    const double TOLERABLE_NORTHING_ERROR_METERS = 0.01; // 1 cm

    const bool is_southish = q_se.y-TOLERABLE_NORTHING_ERROR_METERS <= query.y,
               is_eastish = q_se.x+TOLERABLE_EASTING_ERROR_METERS > query.x;

    if (! is_southish)
    {
        KJB(TEST_PSE(("Query northing %f, SE rounded northing %f\n"
            "northerly error: %f\nQuery SE-round ILL %d, %d\n", 
            query.y, q_se.y, (q_se.y - query.y), gill_se.ilat, gill_se.ilon)));
    }

    if (! is_eastish)
    {
        KJB(TEST_PSE(("Query easting %f, SE rounded easting %f, "
            "Westerly error: %f\nQuery SE-round ILL %d, %d\n", 
            query.x, q_se.x, (query.x - q_se.x), gill_se.ilat, gill_se.ilon)));
    }

    ASSERT(is_southish);
    ASSERT(is_eastish);
}


#if 0
std::ostream* spy_p = 0;
boost::scoped_ptr<std::ostream> spy_o;
#endif


} // end anonymous ns



namespace kjb
{

const int NED_ELLIPSOID = TopoFusion::ELLIPSOID_GRS_1980;



TopoPt force_zone_to_this(const TopoPt& p, char use_this_zone)
{
    if (p.zone == use_this_zone) return p; // nothing to do!

    // We can hop at most one zone away; two or more is absurd.
    // Cast to int in case char is unsigned.
    if (std::abs(int(p.zone) - int(use_this_zone)) != 1)
    {
        KJB_THROW_2(Illegal_argument, "point is not in adjacent zone");
    }

    TopoPt p2(p);
    p2.x = kjb::TopoFusion::getNewEasting(p, use_this_zone);
    p2.zone = use_this_zone;
    return p2;
}



int ned13_grid(
    const TopoFusion::pt& center,
    Matrix *elev_o,
    Matrix *elev_de_o,
    Matrix *elev_dn_o,
    int eastwest_size_meters,
    int northsouth_size_meters,
    int resolution_meters,
    Ned13_caching_reader *cache,
    const std::vector< std::string > &path
)
{
    KJB(ERE(grid_validate(eastwest_size_meters, northsouth_size_meters,
                    resolution_meters)));

    const int   EW_COUNT = eastwest_size_meters / resolution_meters,
                NS_COUNT = northsouth_size_meters / resolution_meters;

    // We need a reader; we might have to instantiate one locally, maybe not.
    boost::scoped_ptr< Ned13_caching_reader > pr;
    if (0 == cache)
    {
        // Flavor of reader does not matter; we will not use its intelligence.
        pr.reset(new Ned13_nearest_se_neighbor_reader(path));
        cache = pr.get();
    }

#if DEBUG_VIEW_TRAINING
    return debug_view_ned_training_data(get_training_elevation_data(center,
            eastwest_size_meters, northsouth_size_meters, & cache -> cache));
#endif

    // Enumerate all query points, and store, keyed by a nearby DEM point.
    MMILL cells( get_cell_identities(center, eastwest_size_meters,
                            northsouth_size_meters, resolution_meters));
#ifdef TEST
    ASSERT(sanity_check(cells));
#endif

    // Prepare output matrices.
    if (elev_o)     elev_o    -> resize(NS_COUNT, EW_COUNT);
    if (elev_de_o)  elev_de_o -> resize(NS_COUNT, EW_COUNT);
    if (elev_dn_o)  elev_dn_o -> resize(NS_COUNT, EW_COUNT);

    if (VERBOSE && elev_o)    kjb_c::kjb_puts("fetch elevation\n");
    if (VERBOSE && elev_de_o) kjb_c::kjb_puts("fetch east west gradient\n");
    if (VERBOSE && elev_dn_o) kjb_c::kjb_puts("fetch north south gradient\n");

    // Now, for each key DEM point, interpolate all its nearby query points.
#if NED_THREADS_COUNT > 1
    KJB(ERE( multithread_interp_el(&cells, & cache -> cache,
                        elev_o, elev_de_o, elev_dn_o, NED_THREADS_COUNT ) ));
#else
    KJB(ERE( interpolate_elevation(&cells, & cache -> cache,
                                    elev_o, elev_de_o, elev_dn_o, VERBOSE) ));
#endif

    return kjb_c::NO_ERROR;
}


/// ctor saves the user some typing, but it's just syntactic sugar.
Ned13_one_degree_grid::Ned13_one_degree_grid(
    int north_lat_deg,
    int west_lon_deg,
    const Path& p
)
:   nw_corner(new IntegralLL(GILL::from_lat_lon(north_lat_deg, west_lon_deg)))
{
    Ned13_one_degree_grid neu(*nw_corner, p);
    swap(neu);
}


/// @brief get the whole-degree location of the northwest corner of grid
GILL Ned13_one_degree_grid::northwest_corner() const
{
    return *nw_corner;
}


/// private ctor takes lat,lon in units of one-third arcsecond
Ned13_one_degree_grid::IntegralLL::IntegralLL(
    int lat_third_arcsecond,
    int lon_third_arcsecond
)
:   ilat(lat_third_arcsecond), ilon(lon_third_arcsecond)
{
    if (ABS_OF(ilat) > 90 * int(THIRD_ARCSEC_PER_DEG))
    {
        KJB_THROW_2(Illegal_argument, "Latitude overrange");
    }
    if (ABS_OF(ilon) > 180 * int(THIRD_ARCSEC_PER_DEG))
    {
        KJB_THROW_2(Illegal_argument, "Longitude overrange");
    }
    if (ilon > 0)
    {
        KJB_THROW_2(Illegal_argument, WEST_IS_NEGATIVE);
    }
}


/// named-ctor builds from whole integer degrees, very coarse (miles away)
GILL Ned13_one_degree_grid::IntegralLL::from_lat_lon(
    int north_lat_deg,
    int west_lon_deg
)
{
    return IntegralLL(  north_lat_deg * THIRD_ARCSEC_PER_DEG,
                        west_lon_deg * THIRD_ARCSEC_PER_DEG);
}


/// named-ctor builds fr. floating-point decimal degrees, rounding to southeast
GILL Ned13_one_degree_grid::IntegralLL::from_lat_lon(
    double north_lat_dd,
    double west_lon_dd
)
{
    // Formally, it truncates toward equator and towards Greenwich, England.
    // In North America, that means it truncates southerly and easterly.
    // Truncation is by a stone's-throw, maybe a dozen meters, not too much.
    return IntegralLL(
            static_cast<int>(north_lat_dd * THIRD_ARCSEC_PER_DEG),
            static_cast<int>(west_lon_dd * THIRD_ARCSEC_PER_DEG)
        );
}


void Ned13_one_degree_grid::grid_rc(const IntegralLL& ill, int*r, int*c) const
{
    const int BOARS = 3 * BONUS_ARCSEC / 2;
    *r = BOARS + nw_corner -> ilat - ill.ilat;
    *c = BOARS + ill.ilon - nw_corner -> ilon;
}


bool Ned13_one_degree_grid::grid_inbounds(int row, int col) const
{
    return      row >= 0
            &&  col >= 0
            &&  row < int(SIZE_THIRD_ARCSEC)
            &&  col < int(SIZE_THIRD_ARCSEC);
}


bool Ned13_one_degree_grid::is_in_bounds(const IntegralLL& ill) const
{
    int grid_row, grid_col;
    grid_rc(ill, &grid_row, &grid_col);
    return grid_inbounds(grid_row, grid_col);
}



Ned13_one_degree_grid::Ned13_one_degree_grid(
    const IntegralLL& ill,
    const Path& path
)
:   nw_corner(new GILL(round_nw_to_whole_degrees(ill)))
{
    AutoPath dp; // can store default path
    const std::vector< std::string > &true_path = path_setup(path, dp);
    const std::string latlon = get_latlon(*nw_corner),
                      extension = ".flt",
                      zip_fn = get_filename(latlon, true_path), // might throw

                      // Tiles from 2014 follow this kind of pattern:
                      flt1 = "float" + latlon + "_13",  // not-yet-seen case
                      flt2 = flt1 + extension,          // common case in 2014

                      // Tiles from 2012 contain a subdirectory name too:
                      flt3 = latlon + DIR_STR + flt1,   // rare case, no suffix
                      flt4 = flt3 + extension,          // common case

                      // List of possibilities we will try (common case first).
                      // Patterns 2-4 all are known to exist (not so for flt1).
                      *ff[] = {&flt4, &flt2, &flt3, &flt1, 00};

    /*
     * Extract the floating-point file from that archive, into a temp
     * directory.  The inner file might have a ".flt" suffix, or maybe not.
     * So we try both.
     */
    Temporary_Recursively_Removing_Directory td;
    const std::string **fn = ff;
    for (fn = ff; *fn; ++fn)
    {
        std::string command = "unzip ";
        if (!VERBOSE) command += "-qq ";
        command += zip_fn + " " + **fn + " -d " + td.get_pathname();
        kjb_c::kjb_system(command.c_str());
        if (file_exists(td.get_pathname() + DIR_STR + **fn))
        {
            break; // if unzip was successful (i.e., target file is there now).
        }
    }
    if (00 == *fn)
    {
        KJB_THROW_2(IO_error, "Unable to open zip file " + zip_fn);
    }

    // read the floating point elevation data
    ETX(get_ned_fdeq(td.get_pathname() + DIR_STR + **fn, &elevation));

    if (elevation.size() != GRID_VOLUME)
    {
        std::ostringstream os;
        os << "File " << zip_fn << " size is incorrect "
                "(expected " << GRID_VOLUME << " points, "
                "read " << elevation.size() << ").";
        KJB_THROW_2(IO_error, os.str());
    }
}


#if 0
/// @brief for debug, show tiny square cartoon image of DEM, forking a task.
void Ned13_one_degree_grid::display(
    const std::string& caption,
    int edge_size
)   const
{
    if (kjb_c::kjb_fork()) return;

    // find the min and max
    const std::pair<float, float> EEXT = max_and_min_elevations();
    const float ERANG = EEXT.first - EEXT.second;
    ASSERT(ERANG > 0);

    // turn data into a matrix
    Matrix *mel = new Matrix;
    KJB(EPETE(ned_fdeq_to_matrix(elevation, mel)));

    // force freeing of deq memory, using some violence (ok because post-fork)
    std::deque<float> *e = const_cast<std::deque<float>*>(&elevation);
    do { std::deque<float> empty; e -> swap(empty); } while(0);

    // shrink the matrix somewhat coarsely (quicker than using image scale)
    while( mel -> get_num_rows() > 5 * edge_size )
    {
        const int   RC = mel -> get_num_rows(),
                    CC = mel -> get_num_cols();
        for (int row_rd = 0, row_wr = 0; row_rd+1<RC; row_rd += 2, ++row_wr)
        {
            for (int col_rd = 0, col_wr =0; col_rd+1<CC; col_rd += 2, ++col_wr)
            {
                double x = mel->at(row_rd, col_rd) + mel->at(row_rd, col_rd+1)
                    + mel->at(row_rd+1, col_rd) + mel->at(row_rd+1, col_rd+1);
                mel->at(row_wr, col_wr) = 0.25 * x;
            }
        }
        mel -> resize(RC/2, CC/2);
    }

    // scale and shift matrix to (floating-point) range 0 to 255
    mel -> ow_add_scalar(- EEXT.second);
    *mel *= 255.5f / ERANG;

    // turn matrix into an image, release matrix memory
    Image *iel = new Image(*mel);
    double scale_factor = edge_size;
    scale_factor /= mel -> get_num_rows();
    delete mel;
    mel = 0;

    // resize image, release memory
    iel -> scale(scale_factor);
    iel -> display(caption);
    delete iel;
    iel = 0;

    // child process does not willingly finish
    while(true) { kjb_c::nap(1000); }
}
#endif





/// get the elevation, in meters, at the given point
float Ned13_one_degree_grid::elevation_meters(const IntegralLL& ill) const
{
    int grid_row, grid_col;
    grid_rc(ill, &grid_row, &grid_col);
    ETX_2(!grid_inbounds(grid_row, grid_col), "Out of range");
    return elevation.at(grid_row * SIZE_THIRD_ARCSEC + grid_col);
}


double delta_e_meters(const Ned13_one_degree_grid::IntegralLL& ill) 
{
#if 0
    // define a neighboring point one step east
    GILL ie(ill);
    ie.ilon += 1;

    // convert to UTM
    const TopoPt p0 = ned13_ill_to_utm(ill), pe = ned13_ill_to_utm(ie);

    // can we use the 'pe' point?  (common case is yes)
    if (p0.zone == pe.zone)
    {
        return pe.x - p0.x; // units are meters
    }

    // rare case:  pe crosses into a new zone, so we have to try one step west.
    // KJB(UNTESTED_CODE()); this branch was hit on 2013 August 28.
    ie.ilon -= 2;
    const TopoPt pw = ned13_ill_to_utm(ie);
    ASSERT(p0.zone == pw.zone);
    return p0.x - pw.x;
#else
    return delta_n_e_meters(ill).second;
#endif
}


double delta_n_meters(const Ned13_one_degree_grid::IntegralLL& ill) 
{
    // define a neighboring point one step south
    GILL is(ill);
    is.ilat -= 1;

    // convert to UTM
    const TopoPt p0 = ned13_ill_to_utm(ill), ps = ned13_ill_to_utm(is);

    // return meters of difference
    return p0.y - ps.y;
}


std::pair<double, double> delta_n_e_meters(
    const Ned13_one_degree_grid::IntegralLL& ill
) 
{
    // define a neighboring point one step east and south
    GILL ise(ill);
    ise.ilon += 1;
    ise.ilat -= 1;

    // convert to UTM
    const TopoPt p0 = ned13_ill_to_utm(ill), pse = ned13_ill_to_utm(ise);
    const double delta_n = p0.y - pse.y;

    // can we use the 'pse' point?  (common case is yes)
    if (p0.zone == pse.zone)
    {
        return std::make_pair(delta_n, pse.x - p0.x); // units are meters
    }

    // rare case:  pse crosses into a new zone, so we have to try one step west
    ise.ilon -= 2;
    const TopoPt psw = ned13_ill_to_utm(ise);
    ASSERT(p0.zone == psw.zone);
    return std::make_pair(delta_n, p0.x - psw.x); // units are meters
}


double Ned13_one_degree_grid::IntegralLL::dd_latitude() const
{
    return double(ilat) / THIRD_ARCSEC_PER_DEG;
}


double Ned13_one_degree_grid::IntegralLL::dd_longitude() const
{
    /*
    KJB(TEST_PSE(("%s:%d -- ilon=%d, THIRD_ARCSEC_PER_DEG=%d, ratio = %f\n",
                    __FILE__, __LINE__, ilon, THIRD_ARCSEC_PER_DEG,
                    double(ilon) / THIRD_ARCSEC_PER_DEG))); // debug
    */
    return double(ilon) / THIRD_ARCSEC_PER_DEG;
}


TopoFusion::pt ned13_ill_to_utm(const Ned13_one_degree_grid::IntegralLL& i)
{
    TopoPt p;
    TopoFusion::LLtoUTM(NED_ELLIPSOID, i.dd_latitude(), i.dd_longitude(), p);
    return p;
}


/**
 * @brief convert from UTM coord to grid location, truncating to the southeast
 * @param p UTM point of interest
 * @return NED13 grid location closest to p without going north or west at all.
 * @bug quantization error empirically appears not always to be strictly SE.
 *
 * If possible, try to avoid this function.  Better to work with UTM points.
 * Truncation error is usually around a dozen meters or less.
 */
GILL utm_to_se_ned13_ill(const TopoFusion::pt& p)
{
    double dd_lat, dd_long;
    TopoFusion::utm_to_lat_long(NED_ELLIPSOID, p, dd_lat, dd_long);

    // floor(latitude) can go south, if you are north of the equator.
    // ceiling(longitude) can go east, if you are west of Greenwich.
    return GILL::from_lat_lon(dd_lat, dd_long);
}



std::pair<float, float> Ned13_one_degree_grid::max_and_min_elevations() const
{
    std::deque< float >::const_iterator i = elevation.begin();
    while (i != elevation.end() && NED_MISSING == *i) ++i;
    std::pair<float, float> Ee = std::make_pair(*i, *i);
    if (elevation.end() == i)
    {
        KJB(UNTESTED_CODE());
        KJB(TEST_PSE(("Suspiciously early end to scan in %s (%s:%d)\n",
                                            __func__, __FILE__, __LINE__)));
        return Ee;
    }
    for (++i; i != elevation.end(); ++i)
    {
        if (NED_MISSING == *i) continue;
        Ee.first = std::max(Ee.first, *i);
        Ee.second = std::min(Ee.second, *i);
    }
    return Ee;
}


void Ned13_one_degree_grid::swap(Ned13_one_degree_grid &q)
{ 
    elevation.swap(q.elevation);

    // It's ugly to "swap" northwest corners, but that's too bad.
    const IntegralLL nwc = *nw_corner, qn = * q.nw_corner;
    nw_corner.reset();
    q.nw_corner.reset();
    nw_corner.reset(new IntegralLL(qn));
    q.nw_corner.reset(new IntegralLL(nwc));
}


/// map an input location to the nearest location to northwest of whole degrees
GILL round_nw_to_whole_degrees(const GILL& ill)
{
    if (ill.ilat <= 0 || ill.ilon >= 0)
    {
        KJB_THROW_2(Illegal_argument,
                        "Input must be in northern and western hemispheres");
    }

    // w=whole-degree; la=latitude.  Round towards equator to nearest degree
    int wla = ill.ilat / int(THIRD_ARCSEC_PER_DEG);

    // w=whole-degree; lo=longitude.  Round towards Greenwich to nearest degree
    // For goodness sakes, don't forget the cast to int here!
    int wlo = ill.ilon / int(THIRD_ARCSEC_PER_DEG);

    // if remainder, push north 1 deg., b/c rounding must have been southward.
    ASSERT(wla > 0);
    if (ill.ilat != wla * int(THIRD_ARCSEC_PER_DEG)) wla += 1;

    // if remainder, push west 1 deg., b/c rounding must have been eastward.
    ASSERT(wlo < 0);
    if (ill.ilon != wlo * int(THIRD_ARCSEC_PER_DEG)) wlo -= 1;

    return GILL::from_lat_lon(wla, wlo);
}


GILL Ned13_one_degree_grid::southeast_corner() const
{
    GILL se_corner(*nw_corner);
    se_corner.ilat -= THIRD_ARCSEC_PER_DEG;
    se_corner.ilon += THIRD_ARCSEC_PER_DEG;
    return se_corner;
}



void Ned13_grid_cache::discard_cache()
{
    while(m_grids.size())
    {
        delete m_grids.back();
        m_grids.pop_back();
    }
    most_recent = 00;
}


/**
 * @brief reference to grid, reading from cache or loading if necessary
 * @return reference to grid containing the specified point
 * @param deg_lat degrees of latitude of some point within the grid
 * @param deg_long degrees of longitude (negative in USA) of point within grid
 */
const Ned13_one_degree_grid& Ned13_grid_cache::fetch(
    double deg_lat,
    double deg_long
)
{
    // ceil(latitude) can push you north if you're north of the equator.
    // floor(longitude) can push you west if you're west of Greenwich.
    // The northwest integer latitude,longitude define the tile names.
    return fetch(GILL::from_lat_lon(static_cast<int>(std::ceil(deg_lat)),
                                    static_cast<int>(std::floor(deg_long))));
}


const Ned13_one_degree_grid& Ned13_grid_cache::fetch(
    const Ned13_one_degree_grid::IntegralLL& nw_corner
)
{
    /*
     * We expect very strong locality of reference:  millions of queries to
     * the same grid, uninterrupted.  Thus we optimize the most recently used
     * grid pointer, rather than always doing the linear search of the next
     * clause.
     */
    if (most_recent && nw_corner == most_recent -> northwest_corner())
    {
        return *most_recent;
    }

    // Linear search, when most recent pointer does not contain the grid.
    for (size_t iii = 0; iii < m_grids.size(); ++iii)
    {
        if (nw_corner == m_grids[iii] -> northwest_corner())
        {
            return *(most_recent = m_grids[iii]);
        }
    }

    // If you reach here, it is a cache miss!  So, we load the desired grid.
    m_grids.push_back(00); // Do this first!  It COULD throw bad_alloc.

    // Now we create the new grid and save a pointer to it.
    // Because we've already expanded m_grids, we won't leak memory.
    most_recent = m_grids.back() = new Ned13_one_degree_grid(nw_corner,m_path);

#ifdef TEST
    if (!(nw_corner == most_recent -> northwest_corner()))
    {
        /*
         * It's a bug if the above equality fails.  We complain in dev mode.
         * If you do it once, you probably do it a thousand times.  So you
         * probably are going to see this message a bunch of times, and then
         * run out of RAM and start swapping.
         */
        KJB(TEST_PSE(("Warning: call to Ned13_grid_cache::fetch(p) where p "
                "was expected to be\na grid corner point (i.e., at an "
                "integral-degree north latitude line\nand an integral-degree "
                "west longitude line).  Actual lat and long:\n"
                "N %f degrees, W %f degrees.\n"
                "This bug can cause spurious cache misses.\n",
                nw_corner.dd_latitude(), -nw_corner.dd_longitude()
            )));
    }
#endif

    return *most_recent;
}



float Ned13_nearest_se_neighbor_reader::elevation_meters(
    const TopoFusion::pt& utm
)
{
    const GILL q(utm_to_se_ned13_ill(utm));
    return cache.fetch(round_nw_to_whole_degrees(q)).elevation_meters(q);
}


float Ned13_bilinear_reader::elevation_meters(
    const TopoFusion::pt& utm
)
{
    const GILL  qse(utm_to_se_ned13_ill(utm)),
                nw(round_nw_to_whole_degrees(qse));
    const Ned13_one_degree_grid& g(cache.fetch(nw));
    const float ese = g.elevation_meters(qse);

    TopoPt utm_se = ned13_ill_to_utm(qse);

    // If southeast point lies in a different zone, fudge it into query zone.
    KJB(UNTESTED_CODE()); // hit on 29 Aug 2013 by tp#104 in hdtrails5
    utm_se = force_zone_to_this(utm_se, utm.zone);

    /* Truncation to southeast turns out not to be guaranteed, but here we
     * verify that if utm_se isn't exactly southeast, it must be close enough.
     */
    validate_southeast_enough(utm, qse, utm_se);

    const bool  exact_easting(utm_se.x == utm.x),
                exact_northing(utm_se.y == utm.y);

    if (exact_easting) // uncommon
    {
        KJB(UNTESTED_CODE());
        if (exact_northing) // very uncommon: exact grid hit
        {
            KJB(UNTESTED_CODE());
            return ese;
        }

        GILL qn(qse);  // nearest point to north
        qn.ilat += 1;

        const float en = g.elevation_meters(qn);
        return ese + (utm.y - utm_se.y) * (en - ese) / delta_n_meters(qse);
    }

    if (exact_northing) // uncommon
    {
        KJB(UNTESTED_CODE());
        GILL qw(qse);  // nearest point to west
        qw.ilon -= 1;

        const float ew = g.elevation_meters(qw);
        return ese + (utm.x - utm_se.x) * (ese - ew) / delta_e_meters(qse);
    }

    // common case:  bilinear interpolate between four neighbors
    GILL qsw(qse); qsw.ilon -= 1;
    GILL qne(qse); qne.ilat += 1;
    GILL qnw(qsw); qnw.ilat += 1;

    Matrix2 E;
    E(0,0) = g.elevation_meters(qnw); E(0,1) = g.elevation_meters(qne);
    E(1,0) = g.elevation_meters(qsw); E(1,1) = ese;
    //if (spy_p) *spy_p << "e " << E(0,0) << ' ' << E(0,1) << ' ' << E(1,0) << ' ' << E(1,1) << '\n';

    Vector2 X;
    X.set_x((utm_se.x - utm.x) / delta_e_meters(qse));
    X.set_y(1.0 - X.x());

    Vector2 Y;
    Y.set_x((utm.y - utm_se.y) / delta_n_meters(qse));
    Y.set_y(1.0 - Y.x());
    //if (spy_p) *spy_p << "i " << X.x() << ' ' << X.y() << ' ' << Y.x() << ' ' << Y.y() << '\n';

    return dot(Y, E * X);
}



/*
 * If you need a bunch of nearby elevation estimates, this is the wrong
 * function, because you'll end up inverting the same matrix a bunch of times.
 * Instead, use the grid function.
 */
float Ned13_gp_reader::elevation_meters(const TopoFusion::pt& utm)
{
    const GILL qse(utm_to_se_ned13_ill(utm));
    Kriging_interpolator k(1/characteristic_length_squared(), qse, &cache);
    return k(utm);
}



/**
 * @brief return correction factor between DEM and DOQ coordinate systems.
 * @param dem_p UTM reference point from a DEM (digital elevation model)
 * @return approximate coordinates of the corresponding location in DOQ imagery
 *
 * The DEM and DOQ datasets do not share precisely the same coordinate system.
 *
 * In other words, the UTM triple (e=222222, n=3333333, z=12) indicates two
 * different (nearby) geographic locations in the DEM dataset and the DOQ
 * dataset.  If we try to use the same
 * coordinate notation for both, we will have misregistered data; when the
 * terrain includes cliffs or rivers, the misregistration is obvious.
 * The displacement is in the neighborhood of 100 to 200 meters.
 *
 * I do not understand the source of this displacement.  It might be a
 * systemic error -- maybe MSR uses the Clarke-1866 ellipsoid (the
 * historical basis for UTM coordinates) somehow with the NAD-83 reference
 * positioning.  That does not entirely make sense, but I'm reduced to
 * guessing.  The funny business below is basically
 * an empirical solution to a poorly-understood problem.
 */
TopoFusion::pt dem_to_doq_displacement(const TopoFusion::pt& dem_p)
{
    using namespace kjb::TopoFusion;
    double la, lo;
    utm_to_lat_long(ELLIPSOID_GRS_1980, dem_p, la, lo);
    //utm_to_lat_long(ELLIPSOID_WGS_84, dem_p, la, lo);
    pt msrmap_doq_p;
    LLtoUTM(ELLIPSOID_Clarke_1866, la, lo, msrmap_doq_p);
    return msrmap_doq_p;
}


#if 0
/// @brief control debug channel of inner workings of some nedget routines.
void set_spy_filename(const std::string& filename)
{
    spy_o.reset(filename.size() ? new std::ofstream(filename.c_str()) : 0);
    spy_p = spy_o.get();
}
#endif

}

