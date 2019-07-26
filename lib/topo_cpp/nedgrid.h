/**
 * @file
 * @brief interface for low-level NED reader functions
 * @author Andrew Predoehl
 *
 * NED stands for the National Elevation Dataset, which is a family of GIS
 * (Geographical Information Systems) databases of elevation data in the
 * U.S.A., also known as a Digital Elevation Models (DEMs).  There are a few
 * NED databases, with different resolutions.  The one used by FindTrails is
 * NED13, which has one-third arcsecond resolution.  NED13 data is available in
 * grids that are approximately square, with an edge length of just over
 * one degree.  As of this writing, a number of such grids are located in
 * /data_3/trails/ned_13as/original.
 */
/*
 * $Id: nedgrid.h 18278 2014-11-25 01:42:10Z ksimek $
 */

#ifndef NEDGRID_H_UOFARIZONA_VISION
#define NEDGRID_H_UOFARIZONA_VISION

#include <l_cpp/l_exception.h>
#include <m_cpp/m_matrix.h>
#include <topo_cpp/layer.h>
#include <string>
#include <deque>
#include <vector>
#include <algorithm>
#include <utility>

#include <boost/scoped_ptr.hpp>

namespace kjb
{


extern const int NED_ELLIPSOID;



/**
 * @brief storage for a single NED13 grid tile, plus convenient access.
 *
 * This does not perform interpolation.
 * Use the "elevation_meters" method to get the elevation of a point in the
 * grid.  "Ah," you ask, "but how to get the point I really want?"
 *
 * To specify locations, there is a helper class defined inside this one:
 * IntegralLL, which is based on integers, and stores latitude and longitude
 * in units of one-third arcsecond; you can increment those values by +/- 1
 * to step through all grid points, until you are out of bounds.
 *
 * Also you can generate an IntegralLL point from decimal degrees, but remember
 * this will quantize your point by truncating, usually to the south and east,
 * although computation artifacts can distort the effect, yielding a point
 * sometimes apparently to the north and west (by a few centimeters).
 *
 * The USGS tiles come with a thin margin around all four edges, so actually
 * these tiles are 1 degree, 4 arcseconds per edge.  Thus you can go to the
 * "northwest corner," and keep going west or north by six more steps.
 * Similarly at the southeast corner.
 *
 * Also see the max and min functions -- you might appreciate them because they
 * disregard missing-data locations.
 */
class Ned13_one_degree_grid
{
public:
    typedef std::vector<std::string> Path; ///< list of dirs to srch for tiles

    struct IntegralLL;

    /**
     * @brief try to load a NED13 tile at a given geo location
     * @param ill  location of a point in the grid (not necessarily at corner)
     * @param path optional list of directories where to look for tile files.
     *
     * If the input path is empty, we by default assume a location on the local
     * vision file server.  If the path is not empty, we do not make that
     * default assumption, and instead each entry in the path should be the
     * name of a directory to look in.
     */
    Ned13_one_degree_grid(const IntegralLL& ill, const Path& path);

    Ned13_one_degree_grid(int, int, const Path&);

#if 0
    void display(const std::string&, int) const;
#endif

    /// is the given location within this grid?
    bool is_in_bounds(const IntegralLL&) const;

    float elevation_meters(const IntegralLL&) const;

    /// @brief determine the maximum elevation value in the grid
    float maximum_elevation() const
    {
        return max_and_min_elevations().first;
    }

    /// @brief determine the minimum elevation, disregarding missing-data flag
    float minimum_elevation() const
    {
        return max_and_min_elevations().second;
    }

    /// @brief determine extremal elevations, disregarding missing-data flag
    std::pair<float, float> max_and_min_elevations() const;

    void swap(Ned13_one_degree_grid&);

    IntegralLL northwest_corner() const;

    /// @brief get the whole-degree location of the southeast corner of grid
    IntegralLL southeast_corner() const;

private:
    /// Northwest corner of grid, not not not counting 2-second bonus margin.
    /// We are using a pointer, because of pimpl idiom.
    boost::scoped_ptr< IntegralLL > nw_corner;

    /// bulk elevation data, in meters above sea level
    std::deque< float > elevation;

    void grid_rc(const IntegralLL&, int*, int*) const;

    bool grid_inbounds(int, int) const;

    // teasers:  this object is not copyable or assignable (swappable, though)
    Ned13_one_degree_grid(const Ned13_one_degree_grid&);
    Ned13_one_degree_grid& operator=(const Ned13_one_degree_grid&);
};



/**
 * @brief integer-valued, latitude-style and longitude-style coordinates
 *
 * This class defines the geographic location of grid points in the NED-13
 * data.  The two fields, which are public integers, correspond to latitude
 * and longitude.  A single unit corresponds to one-third of an arcsecond of
 * distance, and in the USA that corresponds to something like 8 meters east
 * distance per unit increment in longitude (ilon), and 10 meters north
 * distance per unit increment in latitude (ilat).  You can get more exact
 * estimates for these values using the functions delta_e_meters() and
 * delta_n_meters().
 *
 * The reason this class is available is so that the user can traverse the NED
 * grid, by instantiating this class and manually adjusting the ilat and ilon
 * fields.  You can convert these grid values to UTM coordinates using
 * function ned13_ill_to_utm().  You can also convert these grid values to
 * floating-point latitude and longitude using member methods dd_latitude()
 * and dd_longitude().  The latter two methods are less computationally
 * burdensome (a few floating point arithmetic ops) than conversion to UTM
 * (which requires projecting a point onto the GRS-1980 ellipsoid).
 *
 * If you have a UTM coordinate, which are ideally continuous, and you want
 * to know where it lies in a NED grid,
 * you can get its nearest neighbor to the southeast via utm_to_se_ned13_ill().
 * In other words, the grid point should be quantized from your UTM coordinate
 * with the smallest possible nonnegative* deviations to the east and south.
 * (I believe the above definition is unambiguous, at least in the USA.)
 * The above star next to "nonnegative" means I have to add that, for reasons
 * unknown but probably just numerical noise, the UTM-to-LL conversion is not
 * 100% reliably quantized to the southeast.  Sometimes it goes north or west
 * by a few centimeters.  It is better to work with UTM representations
 * as much as possible.
 *
 * This class is copyable, assignable, and comparable (with operator==) with
 * no problem, like most tiny concrete classes.  It won't work with a 16-bit
 * int, but who uses them anymore?
 */
struct Ned13_one_degree_grid::IntegralLL
{
    int     ilat,   ///< latitude, in one-third-arcsecond units (north=+)
            ilon;   ///< longitude, in 1/3rd-arcsecond units (west = neg)

    /// latitude, in "decimal" degrees
    double dd_latitude() const;

    /// longitude, in "decimal" degrees; negative values within the USA.
    double dd_longitude() const;

    // named ctor constructs from float n latitude, e longitude degrees,
    static IntegralLL from_lat_lon(double, double);

    // named ctor constructs from integer n latitude, e longitude degrees
    static IntegralLL from_lat_lon(int, int);

private:
    IntegralLL(int, int);
};

/// @brief distance, in meters, of the next grid point to the east
double delta_e_meters(const Ned13_one_degree_grid::IntegralLL&);

/// @brief distance, in meters, of the next grid point to the north
double delta_n_meters(const Ned13_one_degree_grid::IntegralLL&);

/// @brief distances, in meters, of next grid pt north (first), east (second)
std::pair<double, double> delta_n_e_meters(
    const Ned13_one_degree_grid::IntegralLL&
);

/// @brief convert to UTM coordinates (no elevation)
TopoFusion::pt ned13_ill_to_utm(const Ned13_one_degree_grid::IntegralLL&);


Ned13_one_degree_grid::IntegralLL utm_to_se_ned13_ill(const TopoFusion::pt&);


/// @brief test two coordinate locations for equality
inline bool operator==(
    const Ned13_one_degree_grid::IntegralLL& p,
    const Ned13_one_degree_grid::IntegralLL& q
)
{
    return p.ilat == q.ilat && p.ilon == q.ilon;
}


/// @brief impose an order on coordinate locations (geographically silly).
inline bool operator<(
    const kjb::Ned13_one_degree_grid::IntegralLL& p,
    const kjb::Ned13_one_degree_grid::IntegralLL& q
)
{
    return p.ilat == q.ilat ? p.ilon < q.ilon : p.ilat < q.ilat;
}


Ned13_one_degree_grid::IntegralLL round_nw_to_whole_degrees(
    const Ned13_one_degree_grid::IntegralLL&
);



/// @brief This caches a bunch (potentially) of one-degree grids for you
class Ned13_grid_cache
{
    std::vector< Ned13_one_degree_grid* > m_grids;
    Ned13_one_degree_grid* most_recent;
    const std::vector< std::string > m_path;

    /// @brief release all grid objects in the cache
    void discard_cache();

    // teasers
    Ned13_grid_cache(const Ned13_grid_cache&); // not copyable
    Ned13_grid_cache& operator=(const Ned13_grid_cache&); // not assignable

public:
    Ned13_grid_cache(
        const std::vector< std::string >& path = std::vector<std::string>()
    )
    :   most_recent(00),
        m_path(path.begin(), path.end())
    {}

    /// @brief dtor cleans up (releases all grid objects in the cache)
    virtual ~Ned13_grid_cache()
    {
        discard_cache();
    }

    /// @brief return reference to grid with the given exact NW corner point
    const Ned13_one_degree_grid& fetch(
        const Ned13_one_degree_grid::IntegralLL&
    );

    const Ned13_one_degree_grid& fetch(double, double);

    /// @brief return reference to grid, from cache or loading if necessary
    const Ned13_one_degree_grid& fetch(const TopoFusion::pt& p)
    {
        return fetch(round_nw_to_whole_degrees(utm_to_se_ned13_ill(p)));
    }

    /**
     * @brief return the number of grids in the cache
     *
     * This is useful to prevent your cache from getting TOO big, because
     * those tile files are quite large.  So, you can poll the size
     * occasionally, and throw away the cache when it gets excessive.
     */
    size_t cache_size() const
    {
        return m_grids.size();
    }
};


/// @brief interface to an elevation-info object that caches its NED13 grids
struct Ned13_caching_reader
{
    Ned13_caching_reader(
        const std::vector< std::string >& path = std::vector<std::string>()
    )
    :   cache(path)
    {}

    virtual float elevation_meters(const TopoFusion::pt&) = 0;

    virtual ~Ned13_caching_reader() {}

    Ned13_grid_cache cache;
};


/**
 * @brief support NED 13 elevation queries by using nearest southeast neighbor
 *
 * Like all Ned13_reader classes, this supports queries off the NED 13 grid.
 * This particular class returns, for non-grid points, the nearest neighbor
 * lying in the (closed) quadrant to the southeast of the query point.
 * This is basically the coarsest form of "interpolation."
 * Elevation estimates are discontinous at grid lines.
 */
struct Ned13_nearest_se_neighbor_reader
:   public Ned13_caching_reader
{
    /// basic query operation:  estimate the elevation at a point
    float elevation_meters(const TopoFusion::pt&);

    Ned13_nearest_se_neighbor_reader(
        const std::vector< std::string >& path = std::vector<std::string>()
    )
    :   Ned13_caching_reader(path)
    {}
};


/**
 * @brief support NED 13 elevation queries using bilinear interpolation
 *
 * Like all Ned13_reader classes, this supports queries off the NED 13 grid.
 * This particular class returns, for non-grid points, a bilinear-interpolated
 * estimate, i.e., based on the nearest neighbors in the four directions.
 * The elevation estimates are everywhere continuous, but the slopes are
 * discontinuous at grid lines.
 */
struct Ned13_bilinear_reader
:   public Ned13_caching_reader
{
    /// basic query operation:  estimate the elevation at a point
    float elevation_meters(const TopoFusion::pt&);

    Ned13_bilinear_reader(
        const std::vector< std::string >& path = std::vector<std::string>()
    )
    :   Ned13_caching_reader(path)
    {}
};


struct Ned13_gp_reader
:   public Ned13_caching_reader
{
    /// basic query operation:  estimate the elevation at a point
    float elevation_meters(const TopoFusion::pt&);

    Ned13_gp_reader(
        const std::vector< std::string >& path = std::vector<std::string>()
    )
    :   Ned13_caching_reader(path)
    {}

    /**
     * @brief characteristic length of squared exponential kernel function
     *
     * Since it is about 10 m between points, sigma should be more than 10 m.
     * Let's try a sigma of around 25.
     * This function returns sigma squared.  If you forget to square, you will
     * regret it.
     *
     * 20 m (or 400 m squared) works well for elevation but shows artifacts
     * in the incline map.  Let's try 25 m.
     * 8 Sep 2013:  25 m still shows artifacts.  Let's try 40 m.
     * 15 Sep 2014:  Found, fixed a huge bug that basically ignored this value.
     *               Values from 20 squared to 30 squared work nicely.
     *               40 squared looks like it might be too much.
     *
     * Implementation note: This value interacts with 'train_rad_m' in
     *                      nedget.cpp; the product determines how wide a sweep
     *                      of training data is used.  If you use 20 squared
     *                      here, you should increase train_rad_m to about 7 or
     *                      8 to grab enough training data, otherwise you will
     *                      see artifacts (i.e., elevations 140 m apart are not
     *                      conditionally independent).  If you increase this
     *                      value to 22.4 or 30 squared, you can reduce
     *                      train_rad_m to 6.
     */
    static float characteristic_length_squared() { return 500; }
};


/**
 * @brief generate a grid describing an elevation model near some location
 *
 * @param[in]  center    UTM center point for grid
 * @param[out] elev_o     output pointer: where to store elevation values
 * @param[out] elev_de_o  output pointer for elevation eastern gradient
 * @param[out] elev_dn_o  output pointer for elevation northern gradient
 * @param[in]  eastwest_size_meters  size of the grid, affecting column count
 * @param[in]  northsouth_size_meters size of the grid, affecting row count
 * @param[in]  resolution_meters number of meters between pixel rows or columns
 * @param[in]  cache optional cache object possibly storing DEM tiles
 * @param[in]  path optional vector of directory names to search for tiles
 * @returns kjb_c::ERROR or kjb_c::NO_ERROR as appropriate
 *
 * Pixels are assumed square, as measured in meters.
 *
 * The cache can be any of the reader classes, and as you know they offer
 * polymorphic behavior to interpolate elevation at query points between known
 * NED13 data.  However, this function does not inherit that behavior.  It
 * interpolates its own way (at present, by using a Gaussian process/kriging).
 *
 * The cache input is not const because the cache might change as we read
 * elevation points from it, since it remembers the grids it queries.
 *
 * Any of the Matrix pointers elev_o, elev_de_o, elev_dn_o may be passed in
 * with a value of NULL to indicate that the corresponding output is unneeded.
 *
 * @see ned_visualize_grid for a nice visualization of the output
 */
int ned13_grid(
    const TopoFusion::pt& center,
    Matrix *elev_o,
    Matrix *elev_de_o,
    Matrix *elev_dn_o,
    int eastwest_size_meters = 2000,
    int northsouth_size_meters = 2000,
    int resolution_meters = 1,
    Ned13_caching_reader *cache = 0,
    const std::vector< std::string > &path = std::vector< std::string >()
);



/**
 * @brief generate a grid of elevation values between points at a location
 *
 * @param center    UTM center point for grid
 * @param eastwest_size_meters  size of the grid, affecting column count
 * @param northsouth_size_meters size of the grid, affecting row count
 * @param resolution_meters number of meters between pixel rows or columns
 * @param cache optional cache object possibly storing DEM tiles
 * @param path optional vector of directory names to search for tiles
 *
 * Pixels are assumed square, as measured in meters.
 *
 * The cache can be any of the reader classes, and as you know they offer
 * polymorphic behavior to interpolate elevation at query points between known
 * NED13 data.  However, this function does not inherit that behavior.  It
 * interpolates its own way (at present, by using a Gaussian process/kriging).
 *
 * @bug (August 2013) this returns bad results when the requested grid spans
 *      more than a single one-degree grid.
 */
inline
Matrix ned13_grid(
    const TopoFusion::pt& center,
    int eastwest_size_meters = 2000,
    int northsouth_size_meters = 2000,
    int resolution_meters = 1,
    Ned13_caching_reader *cache = 0,
    const std::vector< std::string > &path = std::vector< std::string >()
)
{
    Matrix e;
    ETX(ned13_grid(center, &e, 0, 0, eastwest_size_meters,
                    northsouth_size_meters, resolution_meters, cache, path));
    return e;
}


TopoFusion::pt dem_to_doq_displacement(const TopoFusion::pt&);


void set_spy_filename(const std::string&);

TopoFusion::pt force_zone_to_this(const TopoFusion::pt&, char use_this_zone);

}


namespace std
{

//template<> // <-- overloading is preferred to template specialization in this context.
//                  (also, this doesn't compile in c++11)
inline void swap(kjb::Ned13_one_degree_grid &p, kjb::Ned13_one_degree_grid &q)
{
    p.swap(q);
}

}

#endif /* NEDGRID_H_UOFARIZONA_VISION */
