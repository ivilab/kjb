/**
 * @file
 * @brief definition of Kriging class
 * @author Andrew Predoehl
 */
/*
 * $Id: kriging.h 17611 2014-09-26 20:39:48Z predoehl $
 *
 * Tab size:  4
 */

#ifndef KRIGING_H_INCLUDED_PREDOEHL_UOFARIZONAVISION
#define KRIGING_H_INCLUDED_PREDOEHL_UOFARIZONAVISION 1

#include <l_mt_cpp/l_mt_mutexlock.h>
#include <m_cpp/m_vector.h>
#include <topo_cpp/nedgrid.h>
#include <vector>

namespace kjb
{

/**
 * @brief a class to interpolate elevation values using Gaussian processes
 *
 * This class performs Gaussian process interpolation of elevation values using
 * a technique commonly known as "kriging."  It is a straighforward application
 * of Gaussian processes but the data structures and interface is customized
 * for GIS data, specifically elevation.
 *
 * Objects of this class have a substantial memory footprint.
 * They are copyable but you should avoid copying them unnecessarily.
 *
 * Basic usage:  the idea is that you have access to a sparse grid of known
 * elevation points, in the form of NED13 data, and you wish to
 * densely interpolate points in the center grid cell.  At least, this is how
 * I use it.  To do so, you instantiate this class with a pointer to a NED 13
 * grid cache, the characteristic length of the Gaussian Process (which
 * determines the smoothness of the interpolation) and the center point of the
 * grid, i.e., a representative point nearby where all other interpolations
 * will be performed.  Then you can use the function operator to fetch the
 * estimated elevation.  Also the diff_e and diff_n functions let you fetch the
 * east-west and north-south gradient at a point.
 *
 * @section nedcacser NED grid-cache serialization
 * If you have multiple threads all sharing a single NED grid cache and trying
 * to build these objects, you must help serialize cache access, because the
 * NED grid cache fills itself from the file system, and you don't want two or
 * more parallel attempts to enlarge the cache that way, especially for the
 * same grid file.  In that case, just create a kjb::Pthread_mutex object
 * elsewhere, and all threads should provide to the ctor that mutex pointer.
 * If the above conditions do not apply (e.g., single-threaded or the cache is
 * not shared) then omit the pointer or pass in a NULL value.
 *
 * @section nedseqspacoh Sequential Spatial Coherence
 * If you wish to have all three output values, elevation and the two gradient
 * components, then be sure to call all these functions one after another at
 * the same point, without any intervening queries at a different point,
 * because the object caches its internal state.  This makes the gradient
 * computation incrementally cheap when it immediately precedes or follows
 * the elevation results.  If you fail to maintain this "sequential-spatial
 * coherency," performance will degrade.
 */
class Kriging_interpolator
{
    const double m_char_sf2;  ///< characteristic spatial frequency squared
    char m_utm_zone;          ///< all points must lie in or near this UTM zone
    std::vector< TopoFusion::pt > m_training;
    Vector m_cov_inv_targ;


    mutable bool kcache_valid;

    /// actual query point, zone maybe bad
    mutable TopoFusion::pt m_query_zbad;

    /// query point pushed into proper zone
    mutable TopoFusion::pt m_query_zgood;

    /// cache of kernel resp. to query
    mutable Vector k_cache;


    void refresh_cache(const TopoFusion::pt&) const;

public:
    Kriging_interpolator(
        double,
        const Ned13_one_degree_grid::IntegralLL&,
        Ned13_grid_cache*,
        Pthread_mutex* = 0
    );

    /**
     * @brief interpolate (krige) the elevation at the given query point
     *
     * If you plan to query gradients too, please @see nedseqspacoh
     */
    double operator()(const TopoFusion::pt& utm) const
    {
        refresh_cache(utm);
        return dot( m_cov_inv_targ, k_cache );
    }

    /**
     * @brief compute the east-west slope of the terrain at query pt
     *
     * If you plan to query both gradients, or elevation, please
     * @see nedseqspacoh
     */
    double diff_e(const TopoFusion::pt&) const;

    /**
     * @brief compute the north-south slope of the terrain at query pt
     *
     * If you plan to query both gradients, or elevation, please
     * @see nedseqspacoh
     */
    double diff_n(const TopoFusion::pt&) const;
};


}

#endif /* KRIGING_H_INCLUDED_PREDOEHL_UOFARIZONAVISION */
