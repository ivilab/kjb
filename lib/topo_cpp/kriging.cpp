/**
 * @file
 * @brief implementation of kriging class
 * @author Andrew Predoehl
 *
 * This performs interpolation on NED data using Gaussian processes, a
 * technique historically called "kriging."  This file uses its own
 * implementation rather than the libkjb Gaussian process code (sorry).
 */
/*
 * $Id: kriging.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "l/l_sys_io.h"
#include "l/l_debug.h"
#include "l_cpp/l_exception.h"
#include "i_cpp/i_matrix.h"
#include "l_cpp/l_ew.h"
#include "n_cpp/n_svd.h"
#include "topo_cpp/nedget.h"
#include "topo_cpp/kriging.h"

#include <sstream>
#include <map>

#ifndef NED_USE_VALARRAY
#define NED_USE_VALARRAY 1 /*provides tiny speedup, but not in the bottleneck*/
#endif

#if NED_USE_VALARRAY
#include <valarray>
#endif

#include <boost/scoped_ptr.hpp>



namespace
{



typedef kjb::TopoFusion::pt TopoPt;

// Name stands for "Grid integral latitude, longitude [pair]."
typedef kjb::Ned13_one_degree_grid::IntegralLL GILL;




// extra rows, columns in the training data grid
const int NS_ILL_MARGIN = 3, EW_ILL_MARGIN = 3;


/*
 * This is a SWAG at the Gaussian error assumed in the LIDAR reference data.
 * Currently set to 1 centimeter, i.e., 95% of all elevations will be within
 * plus or minus 2 cm of the stated value.  I tried before to get by without
 * this, but the covariance matrix was occasionally singular.  This will remedy
 * such problems.  I also tried to find a realistic estimate of this value,
 * but the best I could find was a Wikipedia article saying "under 50 mm error"
 * which is vague but the same ballpark as what I propose here.
 */
const double ELEVATION_NOISE_SIGMA_METERS = 1.0e-2;






char get_most_pop_z(const std::map<char, size_t> &zone_histogram)
{
    std::pair<char, size_t> mpz = * zone_histogram.begin();
    for (    std::map<char, size_t>::const_iterator i = zone_histogram.begin();
            i != zone_histogram.end();
            ++i
        )
    {
        if (i -> second > mpz.second) mpz = *i;
    }
    return mpz.first;
}


/*
 * This function can be called by multiple threads, but if all the threads
 * point to the same cache, then the caller had better pass in a pointer to a
 * mutex to lock/unlock when we do ned-grid-cache access.  If that is not the
 * case (e.g., non-shared grid) then no locking need take place.  In that case
 * pmt_grid_cache_serializer should equal NULL.
 */
std::vector< TopoPt > fill_training_elevation_grid(
    const GILL& nw_corner,
    const GILL& se_corner,
    char* most_popular_zone, // required output ptr, which zone predominates?
    kjb::Ned13_grid_cache* cache,
    kjb::Pthread_mutex* pmt_grid_cache_serializer // might equal NULL
)
{
    using kjb::Runtime_error;
    NTX(cache);
    ASSERT(nw_corner.ilat > se_corner.ilat);
    ASSERT(nw_corner.ilon < se_corner.ilon);

    const size_t rows = nw_corner.ilat - se_corner.ilat + 1,
                 cols = se_corner.ilon - nw_corner.ilon + 1;

    std::vector< TopoPt > output;
    output.reserve(rows*cols);
    std::map<char, size_t> zone_histogram;

    // RAII-lock the mutex, if a mutex pointer was provided.
    boost::scoped_ptr< kjb::Mutex_lock > lock;
    if (pmt_grid_cache_serializer)
    {
        lock.reset( new kjb::Mutex_lock(*pmt_grid_cache_serializer) );
    }

    // Scan the square of training data, buffer all the training points in it.
    for (GILL cursor = nw_corner; cursor.ilat >= se_corner.ilat; --cursor.ilat)
    {
        for (cursor.ilon = nw_corner.ilon; cursor.ilon <= se_corner.ilon;
                                                                ++cursor.ilon)
        {
            TopoPt p = ned13_ill_to_utm(cursor);
            const GILL tile = round_nw_to_whole_degrees(cursor);

            // Get LIDAR elevation at the cursor (not interpolated).
            ASSERT( cache -> fetch(tile).is_in_bounds( tile ));
            p.ele = cache -> fetch(tile).elevation_meters(cursor);

            // Do not push the sentinel for "missing data" into output buffer
            if (kjb::NED_MISSING == p.ele) continue;

            // Sanity-check
            if (p.ele < kjb::NED_MIN)
            {
                KJB_THROW_2(Runtime_error, "Impossible elevation: too small");
            }
            if (p.ele > kjb::NED_MAX)
            {
                KJB_THROW_2(Runtime_error, "Impossible elevation:  too large");
            }

            output.push_back(p);
            zone_histogram[p.zone] += 1;
        }
    }

    if (most_popular_zone) *most_popular_zone = get_most_pop_z(zone_histogram);

    return output;
}







#if NED_USE_VALARRAY
typedef std::valarray<double> VDub;

#else
kjb::Vector get_easting(const std::vector< TopoPt >& points)
{
    kjb::Vector easting(points.size());
    for (size_t iii = 0; iii < points.size(); ++iii)
    {
        easting.at(iii) = points[iii].x;
    }
    return easting;
}

kjb::Vector get_northing(const std::vector< TopoPt >& points)
{
    kjb::Vector northing(points.size());
    for (size_t iii = 0; iii < points.size(); ++iii)
    {
        northing.at(iii) = points[iii].y;
    }
    return northing;
}
#endif


kjb::Matrix gp_se_covariance(
    const std::vector< TopoPt >& points,
    float char_sf2
)
{
    const double DIAG_ELT = 1
                  + ELEVATION_NOISE_SIGMA_METERS*ELEVATION_NOISE_SIGMA_METERS;
#if NED_USE_VALARRAY
    size_t k=0;
    const size_t cc = points.size()*(points.size()-1) >> 1;

    VDub    dx(cc), // easting differences
            dy(cc); // northing differences

    for (size_t j = 0; j < points.size()-1; ++j)
    {
        for (size_t i = 1+j; i < points.size(); ++i, ++k)
        {
            dx[k] = points[i].x - points[j].x;
            dy[k] = points[i].y - points[j].y;
        }
    }
    ASSERT(cc == k);

    VDub sev = exp(-0.5 * char_sf2 * (dx*dx + dy*dy));

    k = 0;
    kjb::Matrix sem(points.size(), points.size());
    sem(points.size()-1, points.size()-1) = DIAG_ELT;
    for (size_t j = 0; j < points.size()-1; ++j)
    {
        sem(j, j) = DIAG_ELT;
        for (size_t i = 1+j; i < points.size(); ++i, ++k)
        {
            sem(i, j) = sev[k];
            sem(j, i) = sev[k];
        }
    }
    return sem;

#else
    kjb::Matrix dx(points.size(), points.size()), dy(dx);
    const kjb::Vector   easting(get_easting(points)),
                        northing(get_northing(points));

    for (size_t rrr = 0; rrr < points.size(); ++rrr)
    {
        for (size_t ccc = 0; ccc < points.size(); ++ccc)
        {
            dx.at(rrr, ccc) = easting.at(rrr) - easting.at(ccc);
            dy.at(rrr, ccc) = northing.at(rrr) - northing.at(ccc);
        }
    }

    kjb::ew_square_ow(dx);
    kjb::ew_square_ow(dy);
    dx += dy;
    dx *= -0.5 * char_sf2;
    dx.mapcar(std::exp);

    for (size_t rrr = 0; rrr < points.size(); ++rrr)
    {
        dx(rrr, rrr) = DIAG_ELT;
    }

    return dx;
#endif
}


/*
 * compare query-point to a given set of (training) points, using
 * squared exponential (SE) kernel.
 */
kjb::Vector gp_se_kernel(
    const std::vector< TopoPt >& points, // training points
    const TopoPt& query,
    float char_sf2
)
{
#if USE_VALARRY
    VDub dx(points.size()), dy(points.size());

    for (size_t rrr = 0; rrr < points.size(); ++rrr)
    {
        dx[rrr] = points[rrr].x - query.x;
        dy[rrr] = points[rrr].y - query.y;
    }

    VDub se = exp(-0.5 * char_sf2 * (dx*dx + dy*dy));
    return kjb::Vector(points.size(), & se[0]);
#else
    kjb::Vector dx(points.size()), dy(points.size());

    for (size_t rrr = 0; rrr < points.size(); ++rrr)
    {
        dx.at(rrr) = points[rrr].x - query.x;
        dy.at(rrr) = points[rrr].y - query.y;
    }

    kjb::ew_square_ow(dx);
    kjb::ew_square_ow(dy);
    dx += dy;
    dx *= -0.5 * char_sf2;
    dx.mapcar(std::exp);

    return dx;
#endif
}




// some point(s) IS/ARE NOT in the proper zone, so we must fix it/them.
bool zone_for_real_fixup(
    std::vector< TopoPt >* points,
    size_t bad_loc,
    char proper_zone
)
{
    // KJB(UNTESTED_CODE()); // not true anymore: hit on 19 Aug 2013.
    ASSERT(points -> at(bad_loc).zone != proper_zone);

    for (size_t jjj = bad_loc; jjj < points -> size(); ++jjj)
    {
        TopoPt& p = points -> at(jjj);
        p = kjb::force_zone_to_this(p, proper_zone);
    }
    return false; // False means that not all the points were in the same zone.
}


// if any points are not in the proper zone, fudge them to be so, approx.
bool zone_fixup(
    std::vector< TopoPt >* points, // both input and output parameter
    char proper_zone
)
{
    ASSERT(points);
    for (size_t iii = 0; iii < points -> size(); ++iii)
    {
        if ((*points)[iii].zone != proper_zone)
        {
            return zone_for_real_fixup(points, iii, proper_zone);
        }
    }
    return true; // True means "truly, all the points are in the same zone."
}

struct Print_a_pt {

    std::ostream& m_os;

    Print_a_pt(std::ostream& os)
    :   m_os(os)
    {}

    void operator()(const TopoPt& p)
    {
        long n = p.y + 0.5;
        m_os << '(' << p.x << ", " << n << ", " << int(p.zone) << ")\n";
    }
};




std::string str_rank_deficit(
    const std::vector< TopoPt >& training,
    const kjb::Svd& svd
)
{
    std::ostringstream os;
    os  << "Training data are rank-deficient.\n"
           "Covariance matrix contains " << training.size() << " rows.\n"
           "Estimated rank (from SVD): " << svd.rank << "\n"
           "Training points (easting, northing, zone):\n"
           "(BEGIN TRAINING DUMP)\n";
    std::for_each(training.begin(), training.end(), Print_a_pt(os));
    os << "(END TRAINING DUMP)\n";

    return os.str();
}



/*
 * The Kriging class needs training data centered around training_center_loc
 * and extending out by an amount proportional to the characteristic length.
 *
 * This had a huge bug (a design flaw) fixed in mid September 2014 -- basically
 * we were using the same value for nw & se (just training_center_loc plus and
 * minus NS_ILL_MARGIN and EW_ILL_MARGIN) regardless of size of train_rad_m.
 * It almost worked because that fixed-size enlargement looks like enough if
 * you fail to look very closely.
 */
std::vector< TopoPt > get_kriging_training(
    const GILL& training_center_loc,
    kjb::Ned13_grid_cache* cache,
    char* m_utm_zone,
    double characteristic_freq_m_sq, // ch. frequency, bumps per meter, SQUARED
    kjb::Pthread_mutex* pmt_grid_cache_serializer
)
{
    // grid increments in meters, in north (first) and east (second) directions
    const std::pair<double,double> dne = delta_n_e_meters(training_center_loc);
    ASSERT(dne.first > 0 && dne.second > 0);

    // Training radius, meters (of circle on earth where we need training data)
    // Factor 6 is because squared exponential dies off after about 3 sigmas
    // away from the peak, just as a Gaussian is said to be about "six sigmas"
    // wide.  This value could be adjusted (5 would be almost as good).
    // There's a factor of 2 in there as well, which is why it's 6 not 3.
    const double train_rad_m = 6.0/sqrt(characteristic_freq_m_sq);
    ASSERT(train_rad_m > 0);

    const int
        // Number of north grid steps, each step is 1/3rd arcsecond latitude.
        dn = std::max(NS_ILL_MARGIN, static_cast<int>(train_rad_m/dne.first)),

        // ditto for east steps, longitude
        de = std::max(EW_ILL_MARGIN, static_cast<int>(train_rad_m/dne.second));

    GILL nw(training_center_loc), se(training_center_loc);
    nw.ilat += dn;
    se.ilat -= dn;
    nw.ilon -= de;  // minus a positive delta means push it west a little
    se.ilon += de;

    return fill_training_elevation_grid(
                        nw, se, m_utm_zone, cache, pmt_grid_cache_serializer);
}




} // end anonymous ns




namespace kjb
{

/**
 * @brief Create object that can interpolate points in a small neighborhood.
 *
 * The "small" neighborhood size is roughly a 10x10 meters.  You create the
 * interpolator object using this ctor, then you can interpolate any point
 * about 7 meters away or less, from 'training_center_loc' the center point,
 * and this will give you a well-behaved elevation estimate, and incline
 * (gradient) estimate too if you want it.  The interpolation method is called
 * "kriging" in the geostatistics literature, but in the machine learning
 * community we would call that "using a Gaussian process for regression."
 * The GP uses the popular squared-exponential kernel (a/k/a covariance).
 *
 * @param characteristic_spatial_frequency_squared  This value represents the
 *        spatial variability of the correlation of the GP model.  Smaller
 *        means slower-varying, and a smoother output.  Don't make it too
 *        smooth.  For NED13 data, 1/400 is a nice value, 1/100 is probably
 *        too quick, and 1/900 is probably too smooth.  The inverse square root
 *        is a value known as the "characteristic length."
 * @param training_center_loc The centerpoint, as a NED13 grid location (i.e.,
 *        an earth coordinate using latitude and longitude on a one-third
 *        arcsecond grid.)  It does not have to be the exact center, but for
 *        best results it should be closer to your query points (which are
 *        sent to the functor operator).  The value in 'training_center_loc'
 *        represents the center of the square of training data used to train
 *        the Gaussian process model for this object.
 * @param cache A NED13 grid caching object, able to read NED13 elevation data.
 * @param pmt_grid_cache_serializer Optional mutex pointer -- set equal to NULL
 *        in single-threaded applications.  If multiple threads will be
 *        building these objects, and if the threads are sharing a common NED
 *        grid cache, you should pass in a pointer to a common mutex --
 *        @see nedcacser for more details
 */
Kriging_interpolator::Kriging_interpolator(
    double characteristic_spatial_frequency_squared,
    const GILL& training_center_loc,
    Ned13_grid_cache* cache,
    Pthread_mutex* pmt_grid_cache_serializer // might equal NULL
)
:   m_char_sf2(characteristic_spatial_frequency_squared),
    m_utm_zone(0),
    m_training(
            get_kriging_training(
                    training_center_loc,
                    cache,
                    &m_utm_zone,
                    m_char_sf2,
                    pmt_grid_cache_serializer
                )
        ),
    kcache_valid(false)
{
    ETX(0 == m_utm_zone);

    // If training data land in two zones, fudge them back to one zone.
    zone_fixup(&m_training, m_utm_zone);
    ASSERT(zone_fixup(&m_training, m_utm_zone));

    // Compute SVD of the covariance matrix of training.
    Svd svd(gp_se_covariance(m_training, m_char_sf2));
    if (svd.rank != int(m_training.size()))
    {
        KJB_THROW_2(Runtime_error, str_rank_deficit(m_training, svd));
    }

    // Compute the vector of target values.
    Vector targets(m_training.size());
    for (size_t iii = 0; iii < m_training.size(); ++iii)
    {
        targets[iii] = m_training[iii].ele;
    }

    /*
     * Compute covariance inverse times target vector.
     * We left-multiply vector times matrix to avoid computing the transpose.
     */
    Vector ut(targets * svd.u());
    ut.ew_divide(svd.d());
    Vector cv_i_t(ut * svd.vt());
    m_cov_inv_targ.swap(cv_i_t);
}


void Kriging_interpolator::refresh_cache(const TopoPt& q) const
{
    if (    !kcache_valid
        ||  q.x != m_query_zbad.x
        ||  q.y != m_query_zbad.y
        ||  q.zone != m_query_zbad.zone
       )
    {
        m_query_zbad = q;
        m_query_zgood = force_zone_to_this(m_query_zbad, m_utm_zone);
        Vector k(gp_se_kernel(m_training, m_query_zgood, m_char_sf2));
        k_cache.swap(k);
        kcache_valid = true;
    }
}



double Kriging_interpolator::diff_e(const TopoPt& utm) const
{
    refresh_cache(utm);
    Vector dk(k_cache);
    for (int i = 0; i < k_cache.get_length(); ++i)
    {
        dk(i) *= m_char_sf2 * (m_training[i].x - m_query_zgood.x);
    }
    return dot( m_cov_inv_targ, dk );
}


double Kriging_interpolator::diff_n(const TopoPt& utm) const
{
    refresh_cache(utm);
    Vector dk(k_cache);
    for (int i = 0; i < k_cache.get_length(); ++i)
    {
        dk(i) *= m_char_sf2 * (m_training[i].y - m_query_zgood.y);
    }
    return dot( m_cov_inv_targ, dk );
}


}

