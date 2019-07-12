/* $Id */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
|  Author: Luca Del Pero
* =========================================================================== */

#ifndef VANISHING_POINT_DETECTION_H
#define VANISHING_POINT_DETECTION_H

#include "m/m_incl.h"
#include "i/i_float.h"
#include <i_cpp/i_image.h>
#include "edge/edge_base.h"
#include "edge_cpp/line_segment_set.h"
#include "m_cpp/m_int_vector.h"
#include "edge_cpp/vanishing_point.h"

#include <vector>
#include <list>
#include <string>

#define VPD_VERTICAL_THRESHOLD   0.12
#define VPD_VERTICAL_VP_OUTLIER_THRESHOLD 0.05
#define VPD_VERTICAL_VP_MAX_OUTLIERS_RATIO 0.6 /*0.4 */

#define VPD_MAX_LINE_SEGMENT_FITTING_ERROR 0.5 /* 0.5 */
#define VPD_RANSAC_ROBUST_SUCCESS_PROBABILITY 0.998

/** When we check the consistency of a triplet of vanishing
 * point, we expect the position of the principal point
 * computed for them to be in the image centre with the
 * following tolerance*/
#define VPD_MAX_PRINCIPAL_POINT_POSITION_TOLERANCE 18  /* vero: 30 //16 // 80 */
#define VPD_LOOSE_VPTS_TOLERANCE 60

namespace kjb
{

/**
 * @class Vanishing_point_detector
 *
 * @brief This class computes the position of the three vanishing points
 * from a set of line segments. The ass
 */
class Vanishing_point_detector
{
    public:

    enum {
        VPD_HORIZONTAL_VP_1 = 0,
        VPD_HORIZONTAL_VP_2,
        VPD_VERTICAL_VP,
        VPD_OUTLIER
    } Vanishing_point_index;

        Vanishing_point_detector(const Edge_segment_set & iset,
                unsigned int img_rows, unsigned int img_cols, double vertical_threshold = VPD_VERTICAL_THRESHOLD,
                double max_line_segment_fitting_error = VPD_MAX_LINE_SEGMENT_FITTING_ERROR,
                double ivpts_tolerance = VPD_MAX_PRINCIPAL_POINT_POSITION_TOLERANCE)
                : _num_rows(img_rows), _num_cols(img_cols), use_relaxed_checks(false),
                  vpts_tolerance(ivpts_tolerance), A(3, 4, 1.0), Atranspose(0),
                  AA(0), absolute_conic(3,3,0.0), E_mp(0), D_vp(0),
                  decomposition(0), inverse(0)
        {
            init_from_edge_segment_set(iset, vertical_threshold, max_line_segment_fitting_error );
            kjb_c::get_target_matrix( &E_mp, 4, 4 );
            kjb_c::get_target_matrix( &Atranspose, 4, 3 );
            kjb_c::get_target_matrix( &AA, 4, 4 );
            kjb_c::get_target_vector( &D_vp, 4);
        }

        Vanishing_point_detector(const Line_segment_set & iset,
                unsigned int img_rows, unsigned int img_cols, double vertical_threshold = VPD_VERTICAL_THRESHOLD,
                double ivpts_tolerance = VPD_MAX_PRINCIPAL_POINT_POSITION_TOLERANCE)
                : _num_rows(img_rows), _num_cols(img_cols), use_relaxed_checks(false),
                  vpts_tolerance(ivpts_tolerance), A(3, 4, 1.0), Atranspose(0),
                  AA(0), absolute_conic(3,3,0.0), E_mp(0), D_vp(0),
                  decomposition(0), inverse(0)
        {
            init_from_edge_segment_set(iset, vertical_threshold);
            kjb_c::get_target_matrix( &E_mp, 4, 4 );
            kjb_c::get_target_matrix( &Atranspose, 4, 3 );
            kjb_c::get_target_matrix( &AA, 4, 4 );
            kjb_c::get_target_vector( &D_vp, 4);
        }

        Vanishing_point_detector(const Vanishing_point_detector & src)
               : _vertical_segments(src._vertical_segments),  _regular_segments(src._regular_segments),
                 _num_rows(src._num_rows), _num_cols(src._num_cols), use_relaxed_checks(src.use_relaxed_checks),
                 vpts_tolerance(src.vpts_tolerance),
                 A(src.A), Atranspose(0), AA(0), absolute_conic(src.absolute_conic), E_mp(0), D_vp(0),
                 decomposition(0), inverse(0)
        {
            kjb_c::copy_matrix( &E_mp, src.E_mp);
            kjb_c::copy_vector( &D_vp, src.D_vp);
            kjb_c::copy_matrix( &decomposition, src.decomposition);
            kjb_c::copy_matrix( &inverse, src.inverse);
            kjb_c::copy_matrix( &Atranspose, src.Atranspose);
            kjb_c::copy_matrix( &AA, src.AA);

        }

        Vanishing_point_detector & operator=(const Vanishing_point_detector & src)
        {
            _vertical_segments = src._vertical_segments;
            _regular_segments = src._regular_segments;
            _num_rows = src._num_rows;
            _num_cols = src._num_cols;
            use_relaxed_checks = src.use_relaxed_checks;
            vpts_tolerance = src.vpts_tolerance;
            A = src.A;
            absolute_conic = src.absolute_conic;
            kjb_c::copy_matrix( &E_mp, src.E_mp);
            kjb_c::copy_vector( &D_vp, src.D_vp);
            kjb_c::copy_matrix( &decomposition, src.decomposition);
            kjb_c::copy_matrix( &inverse, src.inverse);
            kjb_c::copy_matrix( &Atranspose, src.Atranspose);
            kjb_c::copy_matrix( &AA, src.AA);
            return (*this);
        }

        ~Vanishing_point_detector()
        {
            kjb_c::free_matrix(Atranspose);
            kjb_c::free_matrix(AA);
            kjb_c::free_matrix(E_mp);
            kjb_c::free_vector(D_vp);
            kjb_c::free_matrix(decomposition);
            kjb_c::free_matrix(inverse);
        }

        /** @brief Inits the edge segment that will be used to estimate the vanishing points*/
        void init_from_edge_segment_set(const Edge_segment_set & iset, double vertical_threshold,
                                         double max_line_segment_fitting_error );
        /** @brief Inits the edge segment that will be used to estimate the vanishing points*/
        void init_from_edge_segment_set(const Line_segment_set & iset, double vertical_threshold);

        /** @brief Compute the angle between the line segment and
         * the line through the vanishing point and the line
         * segment mid point
         */
        static double compute_alpha(const kjb::Vanishing_point & vp, const Line_segment * line);


        /** @brief Computes the RANSAC penalty for the horizontal
         * segments given the input horizontal vanishing points
         */
        double compute_ransac_penalty
        (
            double                 outlier_threshold,
            unsigned int *         total_number_of_outliers,
            const Vanishing_point & vp1,
            const Vanishing_point & vp2
        );

        /** @brief Computes the RANSAC penalty for all segments given
         *  the three vanishing points
         */
        double jointly_compute_ransac_penalty
        (
            double                 outlier_threshold,
            double *         total_number_of_outliers,
            double max_outliers_ratio,
            const Vanishing_point & vp1,
            const Vanishing_point & vp2,
            const Vanishing_point & vp3
        );

        double groundtruth_compute_ransac_penalty
        (
            double outlier_threshold,
            int * total_number_of_outliers,
            const Line_segment_set & vertical_bucket,
            const Line_segment_set & horizontal_bucket1,
            const Line_segment_set & horizontal_bucket2,
            const Vanishing_point & vp1,
            const Vanishing_point & vp2,
            const Vanishing_point & vp3
        );

        /** @brief Computes the RANSAC penalty for the vertical
         * segments given the input vertical vanishing point
         */
        double compute_vertical_ransac_penalty
        (
           double                 outlier_threshold,
           unsigned int *         total_number_of_outliers,
           const Vanishing_point & vertical_vp
        );

        /** @brief Checks that a set of three vanishing points
         * is consistent. It checks that for each pair of
         * vanishing points (v1, v2) v1'*w*v2 equals to zero,
         * where w is the absolute conic. It then retrieves
         * the principal point and focal length from W and
         * tests that the principal point is not too far from
         * the image centre and the focal length is positive.
         * Returns false if any of these checks fails.
         */
        bool check_vpts_consistency
        (
            const Vanishing_point & vp1,
            const Vanishing_point & vp2,
            const Vanishing_point & vertical_vp,
            double & focal_length
        );

        bool groundtruth_check_vpts_consistency
        (
            const Vanishing_point & vp1,
            const Vanishing_point & vp2,
            const Vanishing_point & vertical_vp,
            double & ifocal_length,
            double & opx,
            double & opy
        );

        /** @brief Checks that a set of three vanishing points
         * is consistent, using basic geometric constraints.
         * For a more thourough check use check_vpts_consistency */
        bool relaxed_check_vpts_consistency
        (
            const Vanishing_point & vp1,
            const Vanishing_point & vp2,
            const Vanishing_point & vertical_vp,
            double & focal_length
        );

        /** @brief Finds an estimate for all three vanishing points using RANSAC */
        double find_vpts_with_ransac
        (
            double success_probability,
            double outlier_threshold,
            double         max_outliers_ratio,
            std::vector<Vanishing_point> & vpts,
            double vertical_vp_outlier_threshold = VPD_VERTICAL_VP_OUTLIER_THRESHOLD,
            double vertical_vp_max_outliers_ratio = VPD_VERTICAL_VP_MAX_OUTLIERS_RATIO
        );

        /** @brief Finds an estimate for the vertical vanishing point */
        bool find_vertical_vp_with_ransac
        (
            double success_probability,
            double outlier_threshold,
            double         max_outliers_ratio,
            Vanishing_point & vp
        );

        /** @brief Finds an estimate for the two horizontal vanishing points.
         *  IMPORTANT: It relies on the current estimate of the vertical
         *  vanishing point
         */
        bool find_horizontal_vanishing_vpts_with_ransac
        (
            double success_probability,
            double outlier_threshold,
            double         max_outliers_ratio,
            const Vanishing_point & vertical_vp,
            Vanishing_point & vp1,
            Vanishing_point & vp2
        );

        /** @brief Finds an estimate of the three vanishing points at once */
        double jointly_find_vpts_with_ransac
        (
            double success_probability,
            double outlier_threshold,
            double         max_outliers_ratio,
            std::vector<Vanishing_point> & vpts
        );

        double find_vpts_from_groundtruth_assignments
        (
            double outlier_threshold,
            const Line_segment_set & vertical_bucket,
            const Line_segment_set & horizontal_bucket1,
            const Line_segment_set & horizontal_bucket2,
            std::vector<Vanishing_point> & vpts
        );

        /** @brief Sets the type of geometric constraints to be used */
        inline bool get_use_relaxed_checks() const
        {
            return use_relaxed_checks;
        }

        /** @brief Returns true if realxed checks are used */
        inline void set_use_relaxed_checks(bool icheck)
        {
            use_relaxed_checks = icheck;
        }
        /** @brief samples n integers without repetitions from the set [0,m]
         */
        static void sample_n_from_m_without_repetitions(unsigned int n, unsigned int m, Int_vector & iv);

        /** @brief Computes the focal length from the vanishing points position */
        double compute_focal_length(const std::vector<Vanishing_point> & v_pts);

        /** If only two line segments are available, we assume that each of them converges
         * to a different vanishing point */
        bool guess_horizontal_vpts_from_two_segments(Vanishing_point & vp1, Vanishing_point & vp2);

        /** If only three line segments are available, we assume that one vp is at the intersection of
         *  two of them, while the third segment converges to the other vp */
        bool guess_horizontal_vpts_from_three_segments(Vanishing_point & vp1, Vanishing_point & vp2);

        int get_tot_num_segments();

        void set_vpts_tolerance(double ivpts_tolerance)
        {
            vpts_tolerance = ivpts_tolerance;
        }

        double geometric_find_vpts_with_ransac
        (
            double success_probability,
            double outlier_threshold,
            double         max_outliers_ratio,
            std::vector<Vanishing_point> & vpts
        );

        double get_best_focal()
        {
            return best_focal;
        }

        int get_num_vertical_segments() const
        {
            return _vertical_segments.size();
        }


    private:

        void insert_penalty_into_list(double ipenalty);

        double compute_average_inlier_penalty(double outlier_threshold);

        /** @brief The vertical line segments  */
        std::vector<const Line_segment *>  _vertical_segments;

        /** @brief The non vertical line segments  */
        std::vector<const Line_segment *>  _regular_segments;

        /** @brief Row range for corner positions. */
        unsigned int _num_rows;

        /** @brief Column range for corner positions. */
        unsigned int _num_cols;

        /** @brief If true, less strict geometric constraints are used
         * when validating a triplet of vanishing points. */
        bool use_relaxed_checks;

        /** @brief Temporary data structure for storing the ordered list of penalties */
        std::list<double> penalties;

        double vpts_tolerance;

        double best_focal;

        /** The following members are created in the class constructor so
         *  that we don't have to reallocate them at every RANSAC iteration
         */
        Matrix A;
        kjb_c::Matrix * Atranspose;
        kjb_c::Matrix * AA;
        Matrix absolute_conic;

        kjb_c::Matrix * E_mp;
        kjb_c::Vector * D_vp;
        kjb_c::Matrix * decomposition;
        kjb_c::Matrix * inverse;

};

/**
 * @brief Estimates the vanishing points for the three orthogonal directions
 * of a Manhattan world scene (where most of all planes are aligned
 * with three main orthogonal directions). This function works ONLY
 * under the Manhattan world assumption.
 *
 */
bool robustly_estimate_vanishing_points
(
    std::vector<Vanishing_point> & vpts,
    double & focal_length,
    const kjb::Image & img,
    double success_probability = VPD_RANSAC_ROBUST_SUCCESS_PROBABILITY,
    bool jointly_estimate = false,
    std::vector<Vanishing_point> right_ones = std::vector<Vanishing_point>(0)
);

bool robustly_estimate_vertical_vanishing_point
(
    Vanishing_point & vertical,
    const kjb::Image & img,
    double success_probability = VPD_RANSAC_ROBUST_SUCCESS_PROBABILITY
);

/**
 * @brief This function uses less constraints coming from geometry
 * and uses the data more, relying on the assumption that there is
 * less noise. This is convenient with synthetic data
 */
bool relaxed_vanishing_point_estimation
(
    std::vector<Vanishing_point> & vpts,
    double & focal_length,
    const kjb::Image & img,
    double success_probability
);

/** @brief Assigns an edge segments to the vanishing point that
 *  minimizes the angle between the segment and the line between
 *  the midpoint of the segment and the vanishing point.
 *  If this angle is too big for all vanishing points, the segment
 *  is labeled as an outlier. This checks using different outlier
 *  thresholds and can be time consuming (ie 1-5 seconds).
 *
 * If you want a faster estimation you can simply use the
 * find_vpts_with_ransac function of class Vanishing_point_detector
 * that looks for vanishing point only with once using the input
 * edges and the specified number of outliers
 */
unsigned int assign_to_vanishing_point
(
    double outlier_threshold,
    const Line_segment * isegment,
    const std::vector<Vanishing_point> & ivpts
);

double robustly_estimate_vanishing_points_Kovesi
(
    std::vector<Vanishing_point> & vpts,
    double & focal_length,
    const Image & img,
    const Line_segment_set & kovesi,
    double start_threshold = 0.05,
    double vpts_tolerance = VPD_MAX_PRINCIPAL_POINT_POSITION_TOLERANCE,
    double success_probability = VPD_RANSAC_ROBUST_SUCCESS_PROBABILITY
);

bool detect_vanishing_points
(
    std::vector<Vanishing_point> & vpts,
    double & focal_length,
    const std::string & img_path
);

}
#endif /*VANISHING_POINT_DETECTION */
