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


#include "edge_cpp/vanishing_point_detector.h"
#include "sample/sample_misc.h"
#include "n/n_diagonalize.h"
#include "n/n_cholesky.h"
#include "n/n_invert.h"
#include <cmath>
#include <iostream>

using namespace kjb;

/** This is an approximation of the position of the
 * vanishing points located at infinity
 */
#define INFINITY_APPROXIMATION 100000000

/** Default parameters for a Ransac run. */

/** We need 4 lines (parameters) to find two pairs of horizontal
 * vanishing points (one pair each). */
#define VPD_HORIZONTAL_VANISHING_POINTS_RANSAC_PARAMETERS 4

/** We need 2 lines (parameters) to find the vertical vanishing
 *  point */
#define VPD_VERTICAL_VANISHING_POINTS_RANSAC_PARAMETERS 2

/** The default expected percentage of inliers among the
 * input points. This is a very rough and inaccurate estimate.
 * It is actually a lower bound so that the algorithm
 * is more robust. I figured it is wise to run RANSAC a little
 * longer since very few lines will intersect exactly
 * at their vanishing point due to noise, and if we
 * don't run it long enough we may get a vanishing point
 * that is a little off its correct position
 */
#define VPD_VANISHING_POINTS_ESTIMATED_INLIER_RATIO 0.15

/** When we check the consistency of a triplet of vanishing
 * point, we expect the focal_length
 * computed for them to be ipositive and possibly not too
 * small */
#define VPD_MIN_FOCAL_LENGTH 150 /*150 */

#define VPD_MAX_FOCAL_LENGTH 5000 /*800 */

#define VPD_CHECK_FOCAL_LENGTH 1000 /*800 */

/** All the following defines are used in the
 *  in the function robustly_estimate_vanishing_points.
 *  Please see the comments there for further details
 */

/** For edge detection */
#define VPD_SIGMA 1.2 /*1.2 */
#define VPD_START_CANNY_THRESHOLD 2.55 /*2.55 */
#define VPD_END_CANNY_THRESHOLD 2.04 /*2.04 */
#define VPD_EDGE_PADDING 30

/** Other parameters we need in robustly_estimate_vanishing_points */
#define VPD_BREAK_EDGE_SHORT_WINDOW_SIZE 7
#define VPD_BREAK_EDGE_LARGE_WINDOW_SIZE 60
#define VPD_BREAK_EDGE_SHORT_WINDOW_THRESHOLD 0.85
#define VPD_BREAK_EDGE_LARGE_WINDOW_THRESHOLD 0.97

#define VPD_SMALL_IMAGE_START_MIN_EDGE_LENGTH 5
#define VPD_NORMAL_IMAGE_START_MIN_EDGE_LENGTH 15
#define VPD_BIG_IMAGE_START_MIN_EDGE_LENGTH 30
#define VPD_RELAXED_EDGE_LENGTH 10

#define VPD_EDGE_LENGTH_INCREMENT 5
#define VPD_MAX_EDGE_LENGTH  51

#define VPD_MAX_OUTLIER_RATIO_INCREMENT 0.05
#define VPD_MAX_OUTLIER_RATIO 0.8

#define VPD_RANSAC_OUTLIER_THRESHOLD 0.06 /*0.06 */
#define VPD_RANSAC_STRICT_MAX_OUTLIER_RATIO 0.05 /*0.05 */
#define VPD_RANSAC_MAX_OUTLIER_RATIO 0.3

#define MIN_SEGMENT_LENGTH_ALLOWED 14.998

#define SMALL_IMAGE_MAX_SIZE 280
#define BIG_IMAGE_MIN_SIZE 500

/** This is used to give a reasonable value for
 * the focal length when it cannot be computed */
#define VPD_DEFAULT_FOCAL_LENGTH_VALUE 350


struct ComparePenalty : public std::binary_function<double ,double,bool >
{
    bool operator () (const double& _value, const double & to_compare) const
    {
        return (_value > to_compare);
    }
};

/**
 * Computes the angle between a edge segment and the line between a segment's
 * mid point and a vanishing point
 *
 * @param vp the vanishing point
 * @param edge_segment the edge segment
 */
double Vanishing_point_detector::compute_alpha
(
    const kjb::Vanishing_point &  vp,
    const Line_segment *        edge_segment
)
{
    double alpha;
    if(vp.get_type() == Vanishing_point::REGULAR)
    {
         /** Compute the difference in orientation between the line segment and
          * the line between the line segment midpoint and the vanishing point
          */
         double denominator = vp.get_x() - edge_segment->get_centre_x();

         /** We check that the segment between the line segment midpoint and the
          * vanishing point is vertical. If so, we set the orientation to 90 degrees,
          * otherwise we compute it
          */
         double orientation = M_PI_2;
         if(fabs(denominator) > DBL_EPSILON)
         {
             double midpoint_to_vp_slope = (vp.get_y() - edge_segment->get_centre_y()) / denominator;
             orientation = atan(-midpoint_to_vp_slope);
             if(orientation < 0)
             {
                 orientation += M_PI;
             }
         }
         alpha = fabs(orientation - edge_segment->get_orientation());
    }
    else if( (vp.get_type() ==  Vanishing_point::INFINITY_UP) || (vp.get_type() ==  Vanishing_point::INFINITY_DOWN) )
    {
        /** Vertical vanishing point, the line segment should then be a vertical line,
         * alpha measures how far the line segment is from being vertical,
         * that implies having an orientation of 90 degrees
         */
        alpha = fabs(M_PI_2 - edge_segment->get_orientation());
    }
    else
    {
        /** Horizontal vanishing point, the line segment should then be a vertical line,
         * alpha measures how far the line segment is from being horizontal,
         * that implies having an orientation of zero degrees
         */
        alpha = fabs(edge_segment->get_orientation());
    }

    /** If alpha is bigger than 90 degrees, it means that the smallest angle between the
     * two segments is actually 180-alpha
     */
    if(alpha > M_PI_2)
    {
        return (M_PI - alpha);
    }
    return alpha;

}

/** Computes the RANSAC penalty for non vertical segments.
 *  Each edge segment is assigned to the vanishing
 *  point that minimizes the angle alpha between the edge segment and the line between
 *  the edge segment mid point and the vanishing point, and the angle alpha
 *  contributes to the total penalty, unless it is bigger than the input
 *  threshold
 *
 *  @param outlier_threshold        The threshold above which an edge
 *                                  segment is considered an outlier
 *  @param total_number_of_outliers Will store the number of edge segments labeled
 *                                  as outliers
 *  @param vp1 This is the first horizontal vanishing point for which we
 *             have to compute the penalty
 *  @param vp2 This is the second horizontal vanishing point for which we
 *             have to compute the penalty
 */
double Vanishing_point_detector::compute_ransac_penalty
(
    double                 outlier_threshold,
    unsigned int *         total_number_of_outliers,
    const Vanishing_point & vp1,
    const Vanishing_point & vp2
)
{
    double penalty = 0;
    (*total_number_of_outliers) = 0;
    double alpha = 0.0;
    double alpha2 = 0.0;

    for(unsigned int i = 0; i < _regular_segments.size() ; i++)
    {
        alpha = compute_alpha( vp1,  _regular_segments[i]);
        alpha2 = compute_alpha( vp2,  _regular_segments[i]);

        if( (alpha2 < alpha) )
        {
            alpha = alpha2;
        }

        if(alpha < outlier_threshold)
        {
            alpha /= _regular_segments[i]->get_length();
            penalty += alpha;
        }
        else
        {
            (*total_number_of_outliers)++;
        }
    }

    return penalty;

}

/** Computes the RANSAC penalty for non vertical segments.
 *  Each edge segment is assigned to the vanishing
 *  point that minimizes the angle alpha between the edge segment and the line between
 *  the edge segment mid point and the vanishing point, and the angle alpha
 *  contributes to the total penalty, unless it is bigger than the input
 *  threshold
 *
 *  @param outlier_threshold        The threshold above which an edge
 *                                  segment is considered an outlier
 *  @param total_number_of_outliers Will store the number of edge points labeled
 *                                  as outliers. Notice that this is edge points,
 *                                  not segments. This encourages the use of long segments
 *  @param vp1 This is the first horizontal vanishing point for which we
 *             have to compute the penalty
 *  @param vp2 This is the second horizontal vanishing point for which we
 *             have to compute the penalty
 *  @param vp3 This is the vertical vanishing point for which we
 *             have to compute the penalty
 */
double Vanishing_point_detector::jointly_compute_ransac_penalty
(
    double                 outlier_threshold,
    double *         total_number_of_outliers,
    double max_outlier_threshold,
    const Vanishing_point & vp1,
    const Vanishing_point & vp2,
    const Vanishing_point & vp3
)
{
    (*total_number_of_outliers) = 0;
    double alpha = 0.0;
    double alpha2 = 0.0;
    double alpha3 = 0.0;
    penalties.clear();

    for(unsigned int i = 0; i < _regular_segments.size() ; i++)
    {
        alpha = compute_alpha( vp1,  _regular_segments[i]);
        alpha2 = compute_alpha( vp2,  _regular_segments[i]);
        alpha3 = compute_alpha( vp3,  _regular_segments[i]);

        if( (alpha2 < alpha) )
        {
            alpha = alpha2;
        }
        if( (alpha3 < alpha) )
        {
              alpha = alpha3;
        }

        if(alpha < outlier_threshold)
        {
            alpha /= (_regular_segments[i]->get_length());
            insert_penalty_into_list(alpha);
        }
        else
        {
            (*total_number_of_outliers) =  (*total_number_of_outliers) + _regular_segments[i]->get_length();
        }
    }

    for(unsigned int i = 0; i < _vertical_segments.size() ; i++)
    {
        alpha = compute_alpha( vp1,  _vertical_segments[i]);
        alpha2 = compute_alpha( vp2,  _vertical_segments[i]);
        alpha3 = compute_alpha( vp3,  _vertical_segments[i]);

        if( (alpha2 < alpha) )
        {
            alpha = alpha2;
        }
        if( (alpha3 < alpha) )
        {
              alpha = alpha3;
        }

        if(alpha < outlier_threshold)
        {
            alpha /= (_vertical_segments[i]->get_length());
            insert_penalty_into_list(alpha);
        }
        else
        {
            (*total_number_of_outliers) = (*total_number_of_outliers) + _vertical_segments[i]->get_length();
        }
    }

    return compute_average_inlier_penalty(max_outlier_threshold);
}


double Vanishing_point_detector::groundtruth_compute_ransac_penalty
(
    double outlier_threshold,
    int * total_number_of_outliers,
    const Line_segment_set & vertical_bucket,
    const Line_segment_set & horizontal_bucket1,
    const Line_segment_set & horizontal_bucket2,
    const Vanishing_point & vp1,
    const Vanishing_point & vp2,
    const Vanishing_point & vp3
)
{
    (*total_number_of_outliers) = 0;
    double alpha = 0.0;
    double alpha2 = 0.0;
    double alpha3 = 0.0;
    penalties.clear();
    int tot_num_segments = vertical_bucket.size() + horizontal_bucket1.size() +
    		horizontal_bucket2.size();

    for(unsigned int i = 0; i < vertical_bucket.size() ; i++)
    {
        alpha = compute_alpha( vp1,  &vertical_bucket.get_segment(i));
        if(alpha < outlier_threshold)
        {
            alpha /= (vertical_bucket.get_segment(i).get_length());
            insert_penalty_into_list(alpha);
        }
        else
        {
            (*total_number_of_outliers)++;
        }
    }
    for(unsigned int i = 0; i < horizontal_bucket1.size() ; i++)
    {
        alpha = compute_alpha( vp2,  &horizontal_bucket1.get_segment(i));
        if(alpha < outlier_threshold)
        {
            alpha /= (horizontal_bucket1.get_segment(i).get_length());
            insert_penalty_into_list(alpha);
        }
        else
        {
        	(*total_number_of_outliers)++;
        }
    }
    for(unsigned int i = 0; i < horizontal_bucket2.size() ; i++)
    {
        alpha = compute_alpha( vp3,  &horizontal_bucket2.get_segment(i));
        if(alpha < outlier_threshold)
        {
            alpha /= (horizontal_bucket2.get_segment(i).get_length());
            insert_penalty_into_list(alpha);
        }
        else
        {
        	(*total_number_of_outliers)++;
        }
    }

    std::list<double>::const_iterator it;
    double tot_penalty = 0.0;
    for(it = penalties.begin(); it != penalties.end(); it++)
    {
	   tot_penalty += (*it);
    }
   return tot_penalty / ((double) penalties.size());
}

int Vanishing_point_detector::get_tot_num_segments()
{
    return  (_regular_segments.size() + _vertical_segments.size());
}

/**
 * Computes the RANSAC penalty for vertical segments. Each vertical segment
 *  contributes to the penalty proportionally to the angle alpha between the
 *  segment and the line between its midpoint and the vertical vanishing point
 *
 *  @param outlier_threshold        The threshold above which an edge
 *                                  segment is considered an outlier
 *  @param total_number_of_outliers Will store the number of edge segments labeled
 *                                  as outliers
 *  @param vertical_vp The vertical vanishing point to be used to compute the penalty
 */
double Vanishing_point_detector::compute_vertical_ransac_penalty
(
    double                 outlier_threshold,
    unsigned int *         total_number_of_outliers,
    const Vanishing_point & vertical_vp
)
{
    double penalty = 0;
    (*total_number_of_outliers) = 0;
    double alpha = 0.0;

    for(unsigned int i = 0; i < _vertical_segments.size(); i++)
    {
        alpha = compute_alpha( vertical_vp,
                               _vertical_segments[i]);

        if(alpha < outlier_threshold)
        {
            alpha /= _vertical_segments[i]->get_length();
            penalty += alpha;

        }
        else
        {
            (*total_number_of_outliers)++;
        }
    }

    return penalty;

}

/**
 * Find all the three vanishing points using RANSAC
 *
 * @param success_probability The probability of finding the right solution
 *        This will increase or decrease the number of Ransac iterations.
 *        This does not necessarily do what you expect, because the ratio of
 *        inliers to outliers needed to compute the number of iterations
 *        from this probability is only an estimate
 *        It is wise to run RANSAC a little
 *        longer than you would think, since very few lines will
 *        intersect exactly at their vanishing point due to noise,
 *        and if we don't run it long enough we may get a vanishing point
 *        that is a little off its correct position
 * @param outlier_threshold The threshold above which a line
 *        segment is considered an outlier (ie it does not
 *        converge to any of the 3 vanishing points). It is the
 *        maximun difference in orientation allowed between the
 *        line segment and the segment between the midpoint of the
 *        segment and the vanishing point (in radian)
 * @param max_outliers_ratio The maximum ratio between outliers and
 *        total segments allowed (ie 0.45 means that no more than
 *        45% of the segments can be outliers)
 * @param vpts This will contain the three estimated vanishing
 *        point, the last being the vertical
 * @param vp_outlier_threshold equivalent to outlier_threshold,
 *        but this is used for the computation of the vertical
 *        vanishing point only
 * @param vertical_vp_max_outliers_ratio equivalent to
 *        max_outliers_ratio, but this is used only for the
 *        estimation of the vertical vanishing point
 */
double Vanishing_point_detector::find_vpts_with_ransac
(
    double success_probability,
    double outlier_threshold,
    double         max_outliers_ratio,
    std::vector<Vanishing_point> & vpts,
    double vertical_vp_outlier_threshold,
    double vertical_vp_max_outliers_ratio
)
{

    std::cout << "Num reg segs:" << _regular_segments.size() << std::endl;
    std::cout << "Num ver segs:" << _vertical_segments.size() << std::endl;
    if(vpts.size() != 3)
    {
        vpts.resize(3, Vanishing_point());
    }

    if( !find_vertical_vp_with_ransac(success_probability, vertical_vp_outlier_threshold,
                                      vertical_vp_max_outliers_ratio, vpts[VPD_VERTICAL_VP]) )
    {
        return false;
    }
    return find_horizontal_vanishing_vpts_with_ransac(success_probability, outlier_threshold,
                     max_outliers_ratio,  vpts[VPD_VERTICAL_VP], vpts[VPD_HORIZONTAL_VP_1],
                     vpts[VPD_HORIZONTAL_VP_2] );
}

double Vanishing_point_detector::jointly_find_vpts_with_ransac
(
    double success_probability,
    double outlier_threshold,
    double         max_outliers_ratio,
    std::vector<Vanishing_point> & vpts
)
{

    std::cout << "Num reg segs:" << _regular_segments.size() << std::endl;
    std::cout << "Num ver segs:" << _vertical_segments.size() << std::endl;
    if(vpts.size() != 3)
    {
        vpts.resize(3, Vanishing_point());
    }

    unsigned int iterations = (unsigned int) (log(1-success_probability)/
            log(1 - pow(VPD_VANISHING_POINTS_ESTIMATED_INLIER_RATIO,
                    VPD_HORIZONTAL_VANISHING_POINTS_RANSAC_PARAMETERS) ));

    std::cout << "Num iter: " << iterations << std::endl;
    iterations = 1600000;

    int num_segments = _regular_segments.size();

    int num_vertical_segments = _vertical_segments.size();

    int total_num_segments = num_segments + num_vertical_segments;

    double full_outlier_threshold = 0.0;
    for(int i = 0; i < num_vertical_segments; i++)
    {
        full_outlier_threshold += _vertical_segments[i]->get_length();
    }
    for(int i = 0; i < num_segments; i++)
    {
        full_outlier_threshold += _regular_segments[i]->get_length();
    }

    std::cout << "The full outlier_threshold is :" << full_outlier_threshold << std::endl;
    full_outlier_threshold *= max_outliers_ratio;
    std::cout << "The ratioed full outlier_threshold is :" << full_outlier_threshold << std::endl;

    /**
     * We store the 4 lines from which we compute the vanishing points
     * in here. The first two will be used to compute the first vanishing
     * points, third and fourth two will be used to compute the
     * second vanishing point
     */
    Int_vector sampled_lines(4);
    Int_vector sampled_vertical_lines(2);
    Int_vector global_sampled_lines(6);

    Vector temp_vertical_position(2);
    Vanishing_point temp_vertical;

    Vector temp_vp1_position(2);
    Vanishing_point temp_vp1;

    Vector temp_vp2_position(2);
    Vanishing_point temp_vp2;

    /*if(num_vertical_segments < 2)
    {
        ** Not enough segments to calculate the vertical vanishing point.
         * We just put it up at infinity, which is generally a safe
         * assumption
         *
        std::cout << "WARNING, TOO FEW VERTICAL SEGMENTS" << std::endl;
        vpts[VPD_VERTICAL_VP].set_type(Vanishing_point::INFINITY_UP);
        return  find_horizontal_vanishing_vpts_with_ransac(success_probability, outlier_threshold,
                max_outliers_ratio,  vpts[VPD_VERTICAL_VP], vpts[VPD_HORIZONTAL_VP_1],
                vpts[VPD_HORIZONTAL_VP_2] );
    }*/

     if(num_segments < 4)
     {
         if( !find_vertical_vp_with_ransac(success_probability, outlier_threshold,
                 max_outliers_ratio, vpts[VPD_VERTICAL_VP]) )
         {
             return false;
         }
         std::cout << "WARNING, TOO FEW HORIZONTAL SEGMENTS" << std::endl;
         if(num_segments == 2)
         {
             return guess_horizontal_vpts_from_two_segments(vpts[VPD_HORIZONTAL_VP_1],vpts[VPD_HORIZONTAL_VP_2]);
         }
         else if(num_segments == 3)
         {
             return guess_horizontal_vpts_from_three_segments(vpts[VPD_HORIZONTAL_VP_1],vpts[VPD_HORIZONTAL_VP_2]);
         }
         else
         {
             /** Not enough segments to calculate the vertical vanishing point.*/
             return false;
         }
     }

     double num_outliers;
     double best_num_outliers = 0.0;
     double outliers_ratio = 0.0;
     double best_penalty = 100000000;
     double penalty = 0.0;
     double focal = 0.0;

     bool found = false;

     for(unsigned int i = 0; i < iterations; i++)
     {
         sample_n_from_m_without_repetitions(6, total_num_segments, global_sampled_lines);
         std::vector<const Line_segment *> regular_pointers;
         std::vector<const Line_segment *> vertical_pointers;
         for(unsigned int k = 0; k < 4; k++)
         {
             if(global_sampled_lines[k] < num_segments)
             {
                 regular_pointers.push_back(_regular_segments[global_sampled_lines[k]]);
             }
             else
             {
                 regular_pointers.push_back(_vertical_segments[global_sampled_lines[k] - num_segments ]);
             }
         }

         for(unsigned int k = 0; k < 2; k++)
         {
             if(global_sampled_lines[k + 4] < num_segments)
             {
                 vertical_pointers.push_back(_regular_segments[global_sampled_lines[k + 4]]);
             }
             else
             {
                 vertical_pointers.push_back(_vertical_segments[global_sampled_lines[k + 4] - num_segments ]);
             }
         }

         /** Let's compute the first vanishing point */
         if( !Line::find_line_intersection(regular_pointers[0]->get_line(),
                                           regular_pointers[1]->get_line(),
                                           temp_vp1_position) )
         {
             /** The line segments do not intersect, let's put the vanishing point at infinity */
             temp_vp1.set_type(Vanishing_point::INFINITY_RIGHT);
         }
         else
         {
             temp_vp1.set_position(temp_vp1_position);
         }

         /** Let's compute the first vanishing point */
         if( !Line::find_line_intersection(regular_pointers[2]->get_line(),
                                           regular_pointers[3]->get_line(),
                                           temp_vp2_position) )
         {
             /** The line segments do not intersect, let's put the vanishing point at infinity */
             temp_vp2.set_type(Vanishing_point::INFINITY_RIGHT);
         }
         else
         {
             temp_vp2.set_position(temp_vp2_position);
         }

         if( !Line::find_line_intersection(vertical_pointers[0]->get_line(),
                                           vertical_pointers[1]->get_line(),
                                           temp_vertical_position) )
         {
             /** The line segments do not intersect, let's put the vanishing point at infinity */
             temp_vertical.set_type(Vanishing_point::INFINITY_UP);
             if( (temp_vp1.get_y() < (_num_rows/2.0)) || (temp_vp2.get_y() < (_num_rows/2.0)))
             {
            	 temp_vertical.set_type(Vanishing_point::INFINITY_DOWN);
             }
         }
         else
         {
             temp_vertical.set_position(temp_vertical_position);
             if( (temp_vertical.get_y() > 0 ) && (temp_vertical.get_y() < _num_cols))
             {
                 continue;
             }
         }

         if(!kjb::find_vertical_vanishing_point(temp_vp1, temp_vp2, temp_vertical, _num_cols, _num_rows))
         {
             continue;
         }

         /** Swap the horizontal vpts if needed */

         if(!use_relaxed_checks)
         {
             if(! check_vpts_consistency(temp_vp1, temp_vp2, temp_vertical, focal) )
             {
                 continue;
             }
         }
         else
         {
             if(! relaxed_check_vpts_consistency(temp_vp1, temp_vp2, temp_vertical, focal) )
             {
                 continue;
             }
         }

         penalty = jointly_compute_ransac_penalty(outlier_threshold, &num_outliers, max_outliers_ratio,
                                          temp_vp1, temp_vp2, temp_vertical);
         //penalty /= (double)(total_num_segments - num_outliers);
         outliers_ratio = (double)num_outliers / (double)total_num_segments;

         //if( outliers_ratio < max_outliers_ratio)
         if(full_outlier_threshold > num_outliers)
         {
             if(found)
             {
                 if(best_penalty > penalty)
                 {
                     best_penalty = penalty;
                     best_num_outliers = num_outliers;
                     vpts[VPD_HORIZONTAL_VP_1] = temp_vp1;
                     vpts[VPD_HORIZONTAL_VP_2] = temp_vp2;
                     vpts[VPD_VERTICAL_VP] = temp_vertical;
                 }
             }
             else
             {
                 best_penalty = penalty;
                 best_num_outliers = num_outliers;
                 vpts[VPD_HORIZONTAL_VP_1] = temp_vp1;
                 vpts[VPD_HORIZONTAL_VP_2] = temp_vp2;
                 vpts[VPD_VERTICAL_VP] = temp_vertical;
                 found = true;
             }
         }
     }

     /*if(found)
     {
         std::cout << "The penalty is : " << best_penalty << std::endl;
         std::cout << "The num outliers is  " << best_num_outliers << std::endl;
         std::cout << "Total num segs: " << total_num_segments << std::endl;
     }*/

   return found;
}

/**
 * Find the vertical vanishing point from the vertical segments using RANSAC
 *
 * @param success_probability The probability of finding the right solution
 *        This will increase or decrease the number of Ransac iterations.
 *        This does not necessarily do what you expect, because the ratio of
 *        inliers to outliers needed to compute the number of iterations
 *        from this probability is only an estimate
 *        It is wise to run RANSAC a little
 *        longer than you would think, since very few lines will
 *        intersect exactly at their vanishing point due to noise,
 *        and if we don't run it long enough we may get a vanishing point
 *        that is a little off its correct position
 * @param outlier_threshold The threshold above which a line
 *        segment is considered an outlier (ie it does not
 *        converge to any of the 3 vanishing points). It is the
 *        maximun difference in orientation allowed between the
 *        line segment and the segment between the midpoint of the
 *        segment and the vanishing point (in radian)
 * @param max_outliers_ratio The maximum ratio between outliers and
 *        total segments allowed (ie 0.45 means that no more than
 *        45% of the segments can be outliers)
 * @param vp This will contain the estimated vertical vanishing point
 */
bool Vanishing_point_detector::find_vertical_vp_with_ransac
(
    double success_probability,
    double outlier_threshold,
    double         max_outliers_ratio,
    Vanishing_point & vp
)
{

    unsigned int iterations = (unsigned int) (log(1-success_probability)/
            log(1 - pow(VPD_VANISHING_POINTS_ESTIMATED_INLIER_RATIO,
                        VPD_VERTICAL_VANISHING_POINTS_RANSAC_PARAMETERS) ));

    unsigned int num_segments = _vertical_segments.size();

     if(num_segments < 2)
     {
        return false;
         /** Not enough segments to calculate the vertical vanishing point.
          * We just put it up at infinity, which is generally a safe
          * assumption
          */
        vp.set_type(Vanishing_point::INFINITY_UP);
        return true;
     }

     Int_vector sampled_lines(2);
     Vector temp_vp_position(2);
     Vanishing_point temp_vp;

     unsigned int num_outliers;
     unsigned int best_num_outliers;
     double outliers_ratio = 0.0;
     double best_penalty = 100000000;

     bool found = false;

     /** Let's start by setting the best penalty to the case where the vertical
      * vanishing point is at infinity. Then we run Ransac
      */
     temp_vp.set_type(Vanishing_point::INFINITY_UP);
     double penalty = compute_vertical_ransac_penalty(outlier_threshold, &num_outliers, temp_vp );
     outliers_ratio = (double)num_outliers / (double)num_segments;

     if( outliers_ratio < max_outliers_ratio)
     {
         vp = temp_vp;
         best_num_outliers = num_outliers;
         best_penalty = penalty / ((double) (num_segments - num_outliers));
         found = true;
     }


     for(unsigned int i = 0; i < iterations; i++)
     {
         sample_n_from_m_without_repetitions(2, num_segments, sampled_lines);
         if( !Line::find_line_intersection(_vertical_segments[ sampled_lines[0] ]->get_line(),
                                           _vertical_segments[ sampled_lines[1] ]->get_line(),
                                           temp_vp_position) )
         {
             /** The line segments do not intersect, let's put the vanishing point at infinity */
             temp_vp.set_type(Vanishing_point::INFINITY_UP);
         }
         else
         {
             temp_vp.set_position(temp_vp_position);
             if( (temp_vp.get_y() > 0 ) && (temp_vp.get_y() < _num_cols))
             {
                 continue;
             }
         }

         penalty = compute_vertical_ransac_penalty(outlier_threshold, &num_outliers, temp_vp );
         penalty /= (double)(num_segments - num_outliers);
         outliers_ratio = (double)num_outliers / (double)num_segments;
         if( outliers_ratio < max_outliers_ratio)
         {
             if(found)
             {
                 if(best_penalty > penalty)
                 {
                     best_penalty = penalty;
                     best_num_outliers = num_outliers;
                     vp = temp_vp;

                 }
             }
             else
             {
                 best_penalty = penalty;
                 best_num_outliers = num_outliers;
                 vp = temp_vp;
                 found = true;
             }
         }

     }

   if(!found)
   {
       /** Not enough segments agree on the vertical vanishing point.
          * We just put it up at infinity, which is generally a safe
          * assumption
          */
       vp.set_type(Vanishing_point::INFINITY_UP);
   }

   return true;

}

double Vanishing_point_detector::geometric_find_vpts_with_ransac
(
    double success_probability,
    double outlier_threshold,
    double         max_outliers_ratio,
    std::vector<Vanishing_point> & vpts
)
{

    std::cout << "Num reg segs:" << _regular_segments.size() << std::endl;
    std::cout << "Num ver segs:" << _vertical_segments.size() << std::endl;
    if(vpts.size() != 3)
    {
        vpts.resize(3, Vanishing_point());
    }

    unsigned int iterations = (unsigned int) (log(1-success_probability)/
            log(1 - pow(VPD_VANISHING_POINTS_ESTIMATED_INLIER_RATIO,
                    VPD_HORIZONTAL_VANISHING_POINTS_RANSAC_PARAMETERS) ));

    std::cout << "Num iter: " << iterations << std::endl;
    iterations = 20000;

    int num_segments = _regular_segments.size();

    int num_vertical_segments = _vertical_segments.size();

    int total_num_segments = num_segments + num_vertical_segments;

    double full_outlier_threshold = 0.0;
    for(int i = 0; i < num_vertical_segments; i++)
    {
        full_outlier_threshold += _vertical_segments[i]->get_length();
    }
    for(int i = 0; i < num_segments; i++)
    {
        full_outlier_threshold += _regular_segments[i]->get_length();
    }

    std::cout << "The full outlier_threshold is :" << full_outlier_threshold << std::endl;
    full_outlier_threshold *= max_outliers_ratio;
    std::cout << "The ratioed full outlier_threshold is :" << full_outlier_threshold << std::endl;

    /**
     * We store the 4 lines from which we compute the vanishing points
     * in here. The first two will be used to compute the first vanishing
     * points, third and fourth two will be used to compute the
     * second vanishing point
     */
    Int_vector sampled_lines(3);

    Vector temp_vertical_position(2);
    Vanishing_point temp_vertical;

    Vector temp_vp1_position(2);
    Vanishing_point temp_vp1;

    Vector temp_vp2_position(2);
    Vanishing_point temp_vp2;
     if(num_segments < 4)
     {
         if( !find_vertical_vp_with_ransac(success_probability, outlier_threshold,
                 max_outliers_ratio, vpts[VPD_VERTICAL_VP]) )
         {
             return false;
         }
         std::cout << "WARNING, TOO FEW HORIZONTAL SEGMENTS" << std::endl;
         if(num_segments == 2)
         {
             return guess_horizontal_vpts_from_two_segments(vpts[VPD_HORIZONTAL_VP_1],vpts[VPD_HORIZONTAL_VP_2]);
         }
         else if(num_segments == 3)
         {
             return guess_horizontal_vpts_from_three_segments(vpts[VPD_HORIZONTAL_VP_1],vpts[VPD_HORIZONTAL_VP_2]);
         }
         else
         {
             /** Not enough segments to calculate the vertical vanishing point.*/
             return false;
         }
     }

     double num_outliers;
     double best_num_outliers = 0.0;
     double outliers_ratio = 0.0;
     double best_penalty = 100000000;
     double penalty = 0.0;
     double focal = 0.0;

     bool found = false;

     for(unsigned int i = 0; i < iterations; i++)
     {
         sample_n_from_m_without_repetitions(3, total_num_segments, sampled_lines);
         std::vector<const Line_segment *> first_two_pointers;
         const Line_segment * third_pointer;
         for(unsigned int k = 0; k < 2; k++)
         {
             if(sampled_lines[k] < num_segments)
             {
                 first_two_pointers.push_back(_regular_segments[sampled_lines[k]]);
             }
             else
             {
                 first_two_pointers.push_back(_vertical_segments[sampled_lines[k] - num_segments ]);
             }
         }

         if(sampled_lines[2] < num_segments)
         {
             third_pointer = _regular_segments[sampled_lines[2]];
         }
         else
         {
             third_pointer = _vertical_segments[sampled_lines[2] - num_segments];
         }

         /** Let's compute the first vanishing point */
         if( !Line::find_line_intersection(first_two_pointers[0]->get_line(),
                                           first_two_pointers[1]->get_line(),
                                           temp_vp1_position) )
         {
             continue;
         }
         else
         {
             temp_vp1.set_position(temp_vp1_position);
         }

         for(double f = 225; f < 1001; f+= 10 )
         {
             if(!find_vanishing_point_given_one_and_line(temp_vp1, f, _num_cols, _num_rows, *third_pointer, temp_vp2))
             {
                 continue;
             }
             if(!find_third_vanishing_point(temp_vp1, temp_vp2, f, _num_rows, _num_cols, temp_vertical))
             {
                 continue;
             }
             if(!kjb::find_vertical_vanishing_point(temp_vp1, temp_vp2, temp_vertical, _num_cols, _num_rows))
             {
                 continue;
             }
             penalty = jointly_compute_ransac_penalty(outlier_threshold, &num_outliers, max_outliers_ratio,
                                              temp_vp1, temp_vp2, temp_vertical);
             outliers_ratio = (double)num_outliers / (double)total_num_segments;

             /*if(penalty > 0.0005)
             {
                 continue;
             }*/
             if(full_outlier_threshold > num_outliers)
             //if( outliers_ratio < max_outliers_ratio)
             {
                 if(found)
                 {
                     if(best_penalty > penalty)
                     {
                         best_penalty = penalty;
                         best_num_outliers = num_outliers;
                         vpts[VPD_HORIZONTAL_VP_1] = temp_vp1;
                         vpts[VPD_HORIZONTAL_VP_2] = temp_vp2;
                         vpts[VPD_VERTICAL_VP] = temp_vertical;
                         best_focal = f;
                     }
                 }
                 else
                 {
                     best_penalty = penalty;
                     best_num_outliers = num_outliers;
                     vpts[VPD_HORIZONTAL_VP_1] = temp_vp1;
                     vpts[VPD_HORIZONTAL_VP_2] = temp_vp2;
                     vpts[VPD_VERTICAL_VP] = temp_vertical;
                     found = true;
                     best_focal = f;
                 }
             }
         }
     }

     /*if(found)
     {
         std::cout << "The penalty is : " << best_penalty << std::endl;
         std::cout << "The num outliers is  " << best_num_outliers << std::endl;
         std::cout << "Total num segs: " << total_num_segments << std::endl;
     }*/

   return found;
}

/**Checks that a set of three vanishing points
 * is consistent. It checks that for each pair of
 * vanishing points (v1, v2) v1'*w*v2 equals to zero,
 * where w is the absolute conic. It then retrieves
 * the principal point and focal length from W and
 * tests that the principal point is not too far from
 * the image centre and the focal length is positive.
 * Returns false if any of these checks fails.
 *
 * @param vp1 the first horizontal vanishing point
 * @param vp2 the second horizontal vanishing point
 * @param vertical_vp the vertical vanishing point
 * @param ifocal_length will contain the focal length
 *        estimated from the vanishing points
 */
bool Vanishing_point_detector::check_vpts_consistency
(
    const Vanishing_point & vp1,
    const Vanishing_point & vp2,
    const Vanishing_point & vertical_vp,
    double & ifocal_length
)
{
    double centre_x = _num_cols/2.0;
    double centre_y = _num_rows/2.0;

    double vp1x = vp1.get_x();
    double vp1y = vp1.get_y();

    double vp2x = vp2.get_x();
    double vp2y = vp2.get_y();

     if(vp1y >= (_num_rows -50) )
     {
         return false;
     }
     if(vp2y >= (_num_rows -50) )
     {
        return false;
     }

     /* Simple geometric constraints, for when the vertical vp is at infinity or close */
     if( vertical_vp.is_at_infinity() ||
        ((fabs(vertical_vp.get_y()) > 10000 ) && (fabs(vp1x) > 10000 ))
        || (fabs(vertical_vp.get_y()) > 10000 && (fabs(vp2x) > 10000 )) )
     {
        if(vp1.is_at_infinity())
        {
            return false;
        }
        if(vp2.is_at_infinity())
        {
            return false;
        }
        if(fabs(vp1x - centre_x) > 1500 )
        {
            if(fabs(vp2x - centre_x) > 30)
            {
                return false;
            }
            if(fabs(vp1y - centre_y) > 30)
            {
                return false;
            }
        }

        if(fabs(vp2x - centre_x) > 1500 )
        {
            if(fabs(vp1x - centre_x) > 30)
            {
                return false;
            }
            if(fabs(vp2y - centre_y) > 30)
            {
                return false;
            }
        }
     }
     else
     {
         if( fabs(vp1y - vp2y) > 110)
         {
             return false;
         }
     }


    /** The vertical vanishing point is at infinity, we perform basic geometric checks */
    if( ((vertical_vp.get_type() == Vanishing_point::INFINITY_UP) || (vertical_vp.get_type() == Vanishing_point::INFINITY_DOWN) ) )
    {

        /* Simple geometric constraints: if we have a vertical vp at infinity,
         * and another one far away on the x axis, the third one has to be
         * very close to the image center
         */

        /** This is how one computes the focal length when the vertical vanishing point is at infinity */
        double root_arg = -(vp1.get_y()-centre_y)*(vp2.get_y()-centre_y) -(vp1.get_x() - centre_x)*(vp2.get_x()-centre_x);

        if(root_arg > 0)
        {
            ifocal_length = sqrt(root_arg);
            if(ifocal_length < VPD_MIN_FOCAL_LENGTH)
                return false;
            if(ifocal_length > VPD_MAX_FOCAL_LENGTH)
                return false;
        }
        else
        {
            return false;
            ifocal_length = VPD_DEFAULT_FOCAL_LENGTH_VALUE;
        }

        /** One cannot have three vanishing points at infinity */
        if(vp1.is_at_infinity() && vp2.is_at_infinity())
            return false;

        /* In this scenario, the horizontal vpts must be roughly on a line parallel to the image x axis */
        if(fabs(vp1y - vp2y) > 30)
            return false;

        /* Last, they cannot be both on the same side (left to right) of the image */
        if( (vp1x < centre_x) && (vp2x < centre_x) )
            return false;
        if( (vp1x > centre_x) && (vp2x > centre_x) )
            return false;

        return true;
    }

    if(vp1.is_at_infinity())
    {
        if(vp2.is_at_infinity())
        {
            /** Horizontal vanishing points cannot be both at infinity! */
            return false;
        }
        if(vp2x > centre_x)
        {
            vp1x = -INFINITY_APPROXIMATION;
            vp1y = vp2y;
        }
        else
        {
            vp1x = INFINITY_APPROXIMATION;
            vp1y = vp2y;
        }
    }
    else if(vp2.is_at_infinity())
    {
        if(vp1x > centre_x)
        {
            vp2x = -INFINITY_APPROXIMATION;
            vp2y = vp1y;
        }
        else
        {
            vp2x = INFINITY_APPROXIMATION;
            vp2y = vp1y;
        }
    }

    //Vertical vanishing point
    double vp3x = vertical_vp.get_x();
    double vp3y = vertical_vp.get_y();
    if(vertical_vp.get_type() == Vanishing_point::INFINITY_UP)
    {
        vp3x = centre_x;
        vp3y = -INFINITY_APPROXIMATION;
    }
    else if(vertical_vp.get_type() == Vanishing_point::INFINITY_DOWN)
    {
        vp3x = centre_x;
        vp3y = INFINITY_APPROXIMATION;
    }

    double vpdist = sqrt( ((vp1x - vp2x)*(vp1x - vp2x)) + ((vp1y - vp2y)*(vp1y - vp2y)));
    if(vpdist < 50 )
    {
        return false;
    }

    vp1x -= centre_x;
    vp2x -= centre_x;
    vp3x -= centre_x;
    vp1y -= centre_y;
    vp2y -= centre_y;
    vp3y -= centre_y;

    /*if( (fabs(vp1x) > 20000) || (fabs(vp2x) > 20000) || (fabs(vp3y) > 20000))
    {
        return false;
    }*/
    /** We solve for the absolute conic by enforcing the
     * constraint that each  pair of
     * vanishing points (v1, v2) v1'*w*v2 equals to zero,
     * where w is the absolute conic. We solve the linear
     * system of equation in the least square sense
     * (Hartley and Zisserman, pp. 224-226)
     */
    A(0,0) = vp1x*vp2x + vp1y*vp2y;
    A(0,1) = vp1x + vp2x;
    A(0,2) = vp1y + vp2y;
    A(0,3) = 1.0;

    A(1,0) = vp2x*vp3x + vp2y*vp3y;
    A(1,1) = vp2x + vp3x;
    A(1,2) = vp2y + vp3y;
    A(1,3) = 1.0;

    A(2,0) = vp1x*vp3x + vp1y*vp3y;
    A(2,1) = vp1x + vp3x;
    A(2,2) = vp1y + vp3y;
    A(2,3) = 1.0;

    ETX( kjb_c::get_matrix_transpose( &Atranspose, A.get_c_matrix() ) );
    ETX(kjb_c::multiply_matrices(&AA, Atranspose, A.get_c_matrix()));

    if( kjb_c::diagonalize_symmetric(AA, &E_mp, &D_vp) != kjb_c::NO_ERROR)
    {
        /** These three vanishing points are no good, we cannot estimate an absolute
         * conic from them (the system admits no solution).
         */
        return false;
    }

    /** We arrange the parameters of the absolute conic we estimated
     * in the absolute conic matric (Hartley and Zisserman, page 226
     */
    absolute_conic(0,0) = E_mp->elements[0][3];
    absolute_conic(0,1) = 0.0;
    absolute_conic(0,2) = E_mp->elements[1][3];
    absolute_conic(1.0) = 0.0;
    absolute_conic(1,1) = E_mp->elements[0][3];
    absolute_conic(1,2) = E_mp->elements[2][3];
    absolute_conic(2,0) = E_mp->elements[1][3];
    absolute_conic(2,1) = E_mp->elements[2][3];
    absolute_conic(2,2) = E_mp->elements[3][3];


    /** We retrieve the intrinsic camera matrix K from w = inv(K*K') */
    if( kjb_c::cholesky_decomposition(&decomposition, absolute_conic.get_c_matrix()) != kjb_c::NO_ERROR)
    {
        /** If we cannot decompose the matrix it means that the triplet of vanishing points
         *  is not a valid configuration */
        return false;
    }

    if( kjb_c::get_matrix_inverse( &inverse, decomposition ) != kjb_c::NO_ERROR)
    {
        return false;
    }
    /** We retrieve the principal point position and focal length
     *  from K.
     *  We divide everything so that K(2,2) equals to 1,
     *  such that K is in his canonical form. Notice that
     *  we are working with K transpose here, since the
     *  Cholesky decomposition we used returns a lower
     *  triangular matrix.
     */
    double focal_length = inverse->elements[0][0]/inverse->elements[2][2];
    double px = inverse->elements[2][0]/inverse->elements[2][2];
    double py = inverse->elements[2][1]/inverse->elements[2][2];

    if(focal_length < VPD_MIN_FOCAL_LENGTH)
        return false;
    if(focal_length > VPD_MAX_FOCAL_LENGTH)
        return false;

    /* This is the most important check. Given that the triplet was orthogonal,
     * we want to make sure that the resulting principal point is close to the image center */
    if(fabs(px) > vpts_tolerance)
         return false;

    if(fabs(py) > vpts_tolerance)
         return false;

    ifocal_length = focal_length;
    return true;
}

bool Vanishing_point_detector::groundtruth_check_vpts_consistency
(
    const Vanishing_point & vp1,
    const Vanishing_point & vp2,
    const Vanishing_point & vertical_vp,
    double & ifocal_length,
    double & opx,
    double & opy
)
{
	opx = 0.0;
	opy = 0.0;
    double centre_x = _num_cols/2.0;
    double centre_y = _num_rows/2.0;

    double vp1x = vp1.get_x();
    double vp1y = vp1.get_y();

    double vp2x = vp2.get_x();
    double vp2y = vp2.get_y();

     /* Simple geometric constraints, for when the vertical vp is at infinity or close */
     if( vertical_vp.is_at_infinity() ||
        ((fabs(vertical_vp.get_y()) > 1000000 ) && (fabs(vp1x) > 1000000 ))
        || (fabs(vertical_vp.get_y()) > 1000000 && (fabs(vp2x) > 1000000 )) )
     {
        if(vp1.is_at_infinity())
        {
            if(vp2.is_at_infinity())
            {
        	    return false;
            }
            else
            {
            	ifocal_length = 1000.0;
            	return true;
            }
        }
        if(vp2.is_at_infinity())
        {
        	if(vp1.is_at_infinity())
			{
				return false;
			}
			else
			{
				ifocal_length = 1000.0;
				return true;
			}
        }
     }

    /** The vertical vanishing point is at infinity, we perform basic geometric checks */
    if( ((vertical_vp.get_type() == Vanishing_point::INFINITY_UP) || (vertical_vp.get_type() == Vanishing_point::INFINITY_DOWN) ) )
    {

        /* Simple geometric constraints: if we have a vertical vp at infinity,
         * and another one far away on the x axis, the third one has to be
         * very close to the image center
         */

        /** This is how one computes the focal length when the vertical vanishing point is at infinity */
        double root_arg = -(vp1.get_y()-centre_y)*(vp2.get_y()-centre_y) -(vp1.get_x() - centre_x)*(vp2.get_x()-centre_x);
        if(root_arg < 0)
        {
        	return false;
        }
        if(root_arg > 0)
        {
            ifocal_length = sqrt(root_arg);
            if(ifocal_length < VPD_MIN_FOCAL_LENGTH)
                return false;
        }
        else
        {
            return false;
            ifocal_length = VPD_DEFAULT_FOCAL_LENGTH_VALUE;
        }
        std::cout << "INFINITY VERTICAL focal length OK" << std::endl;
        return true;
    }

    if(vp1.is_at_infinity())
    {
        if(vp2.is_at_infinity())
        {
            /** Horizontal vanishing points cannot be both at infinity! */
            return false;
        }
        double root_arg = -(vertical_vp.get_y()-centre_y)*(vp2.get_y()-centre_y) -(vertical_vp.get_x() - centre_x)*(vp2.get_x()-centre_x);
        if(root_arg < 0)
        {
        	return false;
        }
        ifocal_length = sqrt(root_arg);
        return true;
    }
    else if(vp2.is_at_infinity())
    {
    	 double root_arg = -(vertical_vp.get_y()-centre_y)*(vp1.get_y()-centre_y) -(vertical_vp.get_x() - centre_x)*(vp1.get_x()-centre_x);
		 if(root_arg < 0)
		 {
		    return false;
		 }
    	 ifocal_length = sqrt(root_arg);
    	 return true;
    }

    //Vertical vanishing point
    double vp3x = vertical_vp.get_x();
    double vp3y = vertical_vp.get_y();
    if(vertical_vp.get_type() == Vanishing_point::INFINITY_UP)
    {
        vp3x = centre_x;
        vp3y = -INFINITY_APPROXIMATION;
    }
    else if(vertical_vp.get_type() == Vanishing_point::INFINITY_DOWN)
    {
        vp3x = centre_x;
        vp3y = INFINITY_APPROXIMATION;
    }

    double vpdist = sqrt( ((vp1x - vp2x)*(vp1x - vp2x)) + ((vp1y - vp2y)*(vp1y - vp2y)));
    if(vpdist < 50 )
    {
        return false;
    }

    vp1x -= centre_x;
    vp2x -= centre_x;
    vp3x -= centre_x;
    vp1y -= centre_y;
    vp2y -= centre_y;
    vp3y -= centre_y;

    /** We solve for the absolute conic by enforcing the
     * constraint that each  pair of
     * vanishing points (v1, v2) v1'*w*v2 equals to zero,
     * where w is the absolute conic. We solve the linear
     * system of equation in the least square sense
     * (Hartley and Zisserman, pp. 224-226)
     */
    A(0,0) = vp1x*vp2x + vp1y*vp2y;
    A(0,1) = vp1x + vp2x;
    A(0,2) = vp1y + vp2y;
    A(0,3) = 1.0;

    A(1,0) = vp2x*vp3x + vp2y*vp3y;
    A(1,1) = vp2x + vp3x;
    A(1,2) = vp2y + vp3y;
    A(1,3) = 1.0;

    A(2,0) = vp1x*vp3x + vp1y*vp3y;
    A(2,1) = vp1x + vp3x;
    A(2,2) = vp1y + vp3y;
    A(2,3) = 1.0;

    ETX( kjb_c::get_matrix_transpose( &Atranspose, A.get_c_matrix() ) );
    ETX(kjb_c::multiply_matrices(&AA, Atranspose, A.get_c_matrix()));

    if( kjb_c::diagonalize_symmetric(AA, &E_mp, &D_vp) != kjb_c::NO_ERROR)
    {
        /** These three vanishing points are no good, we cannot estimate an absolute
         * conic from them (the system admits no solution).
         */
        return false;
    }

    /** We arrange the parameters of the absolute conic we estimated
     * in the absolute conic matric (Hartley and Zisserman, page 226
     */
    absolute_conic(0,0) = E_mp->elements[0][3];
    absolute_conic(0,1) = 0.0;
    absolute_conic(0,2) = E_mp->elements[1][3];
    absolute_conic(1.0) = 0.0;
    absolute_conic(1,1) = E_mp->elements[0][3];
    absolute_conic(1,2) = E_mp->elements[2][3];
    absolute_conic(2,0) = E_mp->elements[1][3];
    absolute_conic(2,1) = E_mp->elements[2][3];
    absolute_conic(2,2) = E_mp->elements[3][3];


    /** We retrieve the intrinsic camera matrix K from w = inv(K*K') */
    if( kjb_c::cholesky_decomposition(&decomposition, absolute_conic.get_c_matrix()) != kjb_c::NO_ERROR)
    {
        /** If we cannot decompose the matrix it means that the triplet of vanishing points
         *  is not a valid configuration */
        return false;
    }

    if( kjb_c::get_matrix_inverse( &inverse, decomposition ) != kjb_c::NO_ERROR)
    {
        return false;
    }
    /** We retrieve the principal point position and focal length
     *  from K.
     *  We divide everything so that K(2,2) equals to 1,
     *  such that K is in his canonical form. Notice that
     *  we are working with K transpose here, since the
     *  Cholesky decomposition we used returns a lower
     *  triangular matrix.
     */
    double focal_length = inverse->elements[0][0]/inverse->elements[2][2];
    double px = inverse->elements[2][0]/inverse->elements[2][2];
    double py = inverse->elements[2][1]/inverse->elements[2][2];

    if(focal_length < VPD_MIN_FOCAL_LENGTH)
        return false;
    std::cout << "Principal: " << px << "  " << py << std::endl;

    opx = px;
    opy = -py;
    ifocal_length = focal_length;
    return true;
}

/**Checks that a set of three vanishing points
 * is consistent, using basic geometric constraints.
 * This is mostly for debug purposes,
 * or for specific applications. For a thorough check use
 * check_vpts_consistency.
 *
 * @param vp1 the first horizontal vanishing point
 * @param vp2 the second horizontal vanishing point
 * @param vertical_vp the vertical vanishing point
 * @param ifocal_length will contain the focal length
 *        estimated from the vanishing points
 *
 */
bool Vanishing_point_detector::relaxed_check_vpts_consistency
(
    const Vanishing_point & vp1,
    const Vanishing_point & vp2,
    const Vanishing_point & vertical_vp,
    double &ifocal_length
)
{
    double centre_x = _num_cols/2.0;
    double centre_y = _num_rows/2.0;

    if( fabs(vertical_vp.get_y() - centre_y) < 10 )
    {
        /** We do not want a vertical vanishing point in the image center */
        return false;
    }


    if(!vp1.is_at_infinity() && !vp2.is_at_infinity())
    {
        /** The two horizontal vanishing points cannot be both on the same side
         * of the image center */
        double tolerance = 10.0;
        if( (vp1.get_x() < (_num_cols/2.0 - tolerance)) && (vp2.get_x() < (_num_cols/2.0 - tolerance)) )
        {
            return false;
        }
        if( (vp1.get_x() > (_num_cols/2.0 + tolerance)) && (vp2.get_x() > (_num_cols/2.0 + tolerance)) )
        {
            return false;
        }

        /** We do not want them both in the image */
        if( (vp1.get_x() > 0) && (vp1.get_x() < _num_cols) )
        {
            if( (vp2.get_x() > 0) && (vp2.get_x() < _num_cols) )
            {
                return false;
            }
        }


        /** A horizontal vanishing point cannot be too high or too low,
         *  this check is useful in a very few corner cases */
        if( (fabs(vp1.get_y()) > (_num_rows+11.0)) || (fabs(vp2.get_y()) > (_num_rows+11.0)) ) //11
        {
            return false;
        }

        //Right 430
        double threshold = 430;
        if(_num_cols > 1000)
        {
            threshold = 650;
        }
        /** Let's now check that they are not both too far */
        if( (vp1.get_x() < - threshold) && (vp2.get_x() > (_num_cols + threshold) ) )
        {
            return false;
        }
        if( (vp2.get_x() < - threshold) && (vp1.get_x() > (_num_cols + threshold) ) )
        {
            return false;
        }

        if(fabs(vp2.get_x() - vp1.get_x() ) < 250) //250
        {
            return false;
        }
        if(fabs(vp2.get_y() - vp1.get_y()) > ((double)_num_rows*0.53)) //Good 0.53
        {
            return false;
        }
    }


    /** Now compute the focal length */
    if( !(vp1.is_at_infinity()) && !(vp2.is_at_infinity()) )
    {
        double root_arg = -(vp1.get_y()-centre_y)*(vp2.get_y()-centre_y) -(vp1.get_x() - centre_x)*(vp2.get_x()-centre_x);
        if(root_arg > 0)
        {
            ifocal_length = sqrt(root_arg);
        }
        else
        {
            ifocal_length = VPD_DEFAULT_FOCAL_LENGTH_VALUE;
        }
    }
    else
    {
        ifocal_length = VPD_DEFAULT_FOCAL_LENGTH_VALUE;
    }

    return true;
}


/**
 * Find the two horizontal vanishing points using RANSAC from
 * the horizontal line segments
 *
 * @param success_probability The probability of finding the right solution
 *        This will increase or decrease the number of Ransac iterations.
 *        This does not necessarily do what you expect, because the ratio of
 *        inliers to outliers needed to compute the number of iterations
 *        from this probability is only an estimate
 *        It is wise to run RANSAC a little
 *        longer than you would think, since very few lines will
 *        intersect exactly at their vanishing point due to noise,
 *        and if we don't run it long enough we may get a vanishing point
 *        that is a little off its correct position
 * @param outlier_threshold The threshold above which a line
 *        segment is considered an outlier (ie it does not
 *        converge to any of the 3 vanishing points). It is the
 *        maximun difference in orientation allowed between the
 *        line segment and the segment between the midpoint of the
 *        segment and the vanishing point (in radian)
 * @param max_outliers_ratio The maximum ratio between outliers and
 *        total segments allowed (ie 0.45 means that no more than
 *        45% of the segments can be outliers)
 */
bool Vanishing_point_detector::find_horizontal_vanishing_vpts_with_ransac
(
    double success_probability,
    double outlier_threshold,
    double         max_outliers_ratio,
    const Vanishing_point & vp_vertical,
    Vanishing_point & vp1,
    Vanishing_point & vp2
)
{
    unsigned int iterations = (unsigned int) (log(1-success_probability)/
            log(1 - pow(VPD_VANISHING_POINTS_ESTIMATED_INLIER_RATIO,
                    VPD_HORIZONTAL_VANISHING_POINTS_RANSAC_PARAMETERS) ));

    unsigned int num_segments = _regular_segments.size();

     if(num_segments < 4)
     {
         return false;
         if(num_segments == 2)
         {
             return guess_horizontal_vpts_from_two_segments(vp1, vp2);
         }
         else if(num_segments == 3)
         {
             return guess_horizontal_vpts_from_three_segments(vp1, vp2);
         }

         /** Not enough segments to calculate the vertical vanishing point.
          */
         return false;
     }

     /**
      * We store the 4 lines from which we compute the vanishing points
      * in here. The first two will be used to compute the first vanishing
      * points, third and fourth two will be used to compute the
      * second vanishing point
      */
     Int_vector sampled_lines(4);

     Vector temp_vp1_position(2);
     Vanishing_point temp_vp1;

     Vector temp_vp2_position(2);
     Vanishing_point temp_vp2;

     unsigned int num_outliers;
     unsigned int best_num_outliers;
     double outliers_ratio = 0.0;
     double best_penalty = 100000000;
     double penalty = 0.0;
     double focal = 0.0;

     bool found = false;

     for(unsigned int i = 0; i < iterations; i++)
     {
         sample_n_from_m_without_repetitions(4, num_segments, sampled_lines);

         /** Let's compute the first vanishing point */
         if( !Line::find_line_intersection(_regular_segments[ sampled_lines[0] ]->get_line(),
                                           _regular_segments[ sampled_lines[1] ]->get_line(),
                                           temp_vp1_position) )
         {
             /** The line segments do not intersect, let's put the vanishing point at infinity */
             temp_vp1.set_type(Vanishing_point::INFINITY_RIGHT);
         }
         else
         {
             temp_vp1.set_position(temp_vp1_position);
         }

         /** Let's compute the first vanishing point */
         if( !Line::find_line_intersection(_regular_segments[ sampled_lines[2] ]->get_line(),
                                   _regular_segments[ sampled_lines[3] ]->get_line(),
                                   temp_vp2_position) )
         {
             /** The line segments do not intersect, let's put the vanishing point at infinity */
             temp_vp2.set_type(Vanishing_point::INFINITY_RIGHT);
         }
         else
         {
             temp_vp2.set_position(temp_vp2_position);
         }

         if(!use_relaxed_checks)
         {
             if(! check_vpts_consistency(temp_vp1, temp_vp2, vp_vertical, focal) )
             {
                 continue;
             }
         }
         else
         {
             if(! relaxed_check_vpts_consistency(temp_vp1, temp_vp2, vp_vertical, focal) )
             {
                 continue;
             }
         }

         penalty = compute_ransac_penalty(outlier_threshold, &num_outliers,
                                          temp_vp1, temp_vp2);
         penalty /= (double)(num_segments - num_outliers);
         outliers_ratio = (double)num_outliers / (double)num_segments;

         if( outliers_ratio < max_outliers_ratio)
         {
             if(found)
             {
                 if(best_penalty > penalty)
                 {
                     best_penalty = penalty;
                     best_num_outliers = num_outliers;
                     vp1 = temp_vp1;
                     vp2 = temp_vp2;

                 }
             }
             else
             {
                 best_penalty = penalty;
                 best_num_outliers = num_outliers;
                 vp1 = temp_vp1;
                 vp2 = temp_vp2;
                 found = true;
             }
         }
     }

   return found;

}

/**
 * Computes the focal length from a set of three input vanishing points
 *
 * @param v_pts The input vanishing points
 */
double Vanishing_point_detector::compute_focal_length(const std::vector<Vanishing_point> & v_pts)
{
    double focal;
    if(v_pts.size() != 3)
    {
        KJB_THROW_2(Illegal_argument, "Compute focal length, expected three vanishing points");
    }

    if(_regular_segments.size() < 4)
    {
        return VPD_DEFAULT_FOCAL_LENGTH_VALUE;
    }

    if(!check_vpts_consistency(v_pts[VPD_HORIZONTAL_VP_1], v_pts[VPD_HORIZONTAL_VP_2], v_pts[VPD_VERTICAL_VP], focal))
    {
        if(!relaxed_check_vpts_consistency(v_pts[VPD_HORIZONTAL_VP_1], v_pts[VPD_HORIZONTAL_VP_1], v_pts[VPD_VERTICAL_VP], focal))
        {
            KJB_THROW_2(Illegal_argument, "Compute focal length, invalid set of vanishing points");
        }
    }

    return focal;

}

/**
 * Init this vanishing point detector from the input segments.
 * Only segments whose fitting error is reasonably small will be kept.
 * Segments will be divided into two different classes according to
 * their slope (vertical and non vertical, the former will be used to
 * estimate the vertical vanishing point).
 *
 * @param vertical_threshold Segments whose slope is larger than this threshold
 *        will be labeled as vertical segments
 * @param iset the set of edge segments
 * @param max_line_segment_fitting_error only segments whose fitting error is smaller
 *        than this threshold will be used
 */
void Vanishing_point_detector::init_from_edge_segment_set
(
    const Edge_segment_set & iset,
    double vertical_threshold,
    double max_line_segment_fitting_error
)
{
    _vertical_segments.clear();
    _regular_segments.clear();
    std::cout << "Set size:" << iset.size() << std::endl;
    for(unsigned int i = 0; i < iset.size(); i++)
    {
        //if((iset.get_segment(i).get_least_squares_fitting_error() < max_line_segment_fitting_error))
        {
            if( fabs(iset.get_segment(i).get_orientation() - M_PI_2) < vertical_threshold)
            {
                _vertical_segments.push_back(  &(iset.get_segment(i) )  );
            }
            else
            {
                if(iset.get_segment(i).get_length() >= MIN_SEGMENT_LENGTH_ALLOWED)
                {
                    _regular_segments.push_back(  &(iset.get_segment(i)  ) );
                }
            }
        }

    }

}

/**
 * Init this vanishing point detector from the input segments.
 * Only segments whose fitting error is reasonably small will be kept.
 * Segments will be divided into two different classes according to
 * their slope (vertical and non vertical, the former will be used to
 * estimate the vertical vanishing point).
 *
 * @param vertical_threshold Segments whose slope is larger than this threshold
 *        will be labeled as vertical segments
 * @param iset the set of edge segments
 * @param max_line_segment_fitting_error only segments whose fitting error is smaller
 *        than this threshold will be used
 */
void Vanishing_point_detector::init_from_edge_segment_set
(
    const Line_segment_set & iset,
    double vertical_threshold
)
{
    _vertical_segments.clear();
    _regular_segments.clear();
    for(unsigned int i = 0; i < iset.size(); i++)
    {
            if( fabs(iset.get_segment(i).get_orientation() - M_PI_2) < vertical_threshold)
            {
                if(iset.get_segment(i).get_length() >= MIN_SEGMENT_LENGTH_ALLOWED)
                {
                    if(_num_rows < 400)
                    {
                        _vertical_segments.push_back(  &(iset.get_segment(i) )  );
                    }
                    else if(iset.get_segment(i).get_length() > 60)
                    {
                        _vertical_segments.push_back(  &(iset.get_segment(i) )  );
                    }
                }
            }
            else
            {
                if(iset.get_segment(i).get_length() >= MIN_SEGMENT_LENGTH_ALLOWED)
                {
                    _regular_segments.push_back(  &(iset.get_segment(i)  ) );
                }
            }
    }
    if(_num_rows > 399)
    {
        if(_vertical_segments.size() < 10)
        {
              _vertical_segments.clear();
              for(unsigned int i = 0; i < iset.size(); i++)
              {
                  if( fabs(iset.get_segment(i).get_orientation() - M_PI_2) < vertical_threshold)
                  {
                      if(iset.get_segment(i).get_length() >= 30)
                      {
                          _vertical_segments.push_back(  &(iset.get_segment(i) )  );
                      }
                  }
              }
         }
    }

}

bool Vanishing_point_detector::guess_horizontal_vpts_from_two_segments(Vanishing_point & vp1, Vanishing_point & vp2)
{
    if(_regular_segments.size() != 2)
    {
        return false;
    }
    kjb::Vector direction;
    const kjb::Vector & centre = _regular_segments[0]->get_centre();
    _regular_segments[0]->get_direction(direction);

    kjb::Vector vp1_position(2);
    vp1_position(0) = centre(0) + direction(0);
    vp1_position(1) = centre(1) + direction(1);
    vp1.set_position(vp1_position);

    kjb::Vector direction2;
    const kjb::Vector & centre2 = _regular_segments[1]->get_centre();
    _regular_segments[1]->get_direction(direction2);

    kjb::Vector vp2_position(2);
    vp2_position(0) = centre2(0) + direction2(0);
    vp2_position(1) = centre2(1) + direction2(1);
    vp2.set_position(vp2_position);
    return true;
}

bool Vanishing_point_detector::guess_horizontal_vpts_from_three_segments(Vanishing_point & vp1, Vanishing_point & vp2)
{
    if(_regular_segments.size() != 3)
    {
        return false;
    }

    kjb::Vector ints1_2;
    _regular_segments[0]->get_intersection(*_regular_segments[1], ints1_2);
    kjb::Vector ints1_3;
    _regular_segments[0]->get_intersection(*_regular_segments[2], ints1_3);
    kjb::Vector ints2_3;
    _regular_segments[1]->get_intersection(*_regular_segments[2], ints2_3);

    double dist1_2 = _regular_segments[0]->get_distance_from_point(ints1_2) +
                     _regular_segments[1]->get_distance_from_point(ints1_2);
    double dist1_3 = _regular_segments[0]->get_distance_from_point(ints1_3) +
                     _regular_segments[2]->get_distance_from_point(ints1_3);
    double dist2_3 = _regular_segments[1]->get_distance_from_point(ints2_3) +
                     _regular_segments[2]->get_distance_from_point(ints2_3);

    if( (dist1_2 > dist1_3) && (dist1_2 > dist2_3) )
    {
        kjb::Vector vp1_position(2);
        vp1_position(0) = ints1_2(0);
        vp1_position(1) = ints1_2(1);
        vp1.set_position(vp1_position);

        kjb::Vector direction;
        const kjb::Vector & centre = _regular_segments[2]->get_centre();
        _regular_segments[2]->get_direction(direction);

        kjb::Vector vp2_position(2);
        vp2_position(0) = centre(0) + direction(0);
        vp2_position(1) = centre(1) + direction(1);
        vp2.set_position(vp2_position);
    }
    else if( (dist1_3 > dist1_2) && (dist1_3 > dist2_3) )
    {
        kjb::Vector position(2);
        position(0) = ints1_3(0);
        position(1) = ints1_3(1);
        vp1.set_position(position);

         kjb::Vector direction;
        const kjb::Vector & centre = _regular_segments[1]->get_centre();
        _regular_segments[1]->get_direction(direction);

        kjb::Vector vp2_position(2);
        vp2_position(0) = centre(0) + direction(0);
        vp2_position(1) = centre(1) + direction(1);
        vp2.set_position(vp2_position);
    }
    else
    {
        kjb::Vector position(2);
        position(0) = ints2_3(0);
        position(1) = ints2_3(1);
        vp1.set_position(position);

         kjb::Vector direction;
        const kjb::Vector & centre = _regular_segments[0]->get_centre();
        _regular_segments[0]->get_direction(direction);

        kjb::Vector vp2_position(2);
        vp2_position(0) = centre(0) + direction(0);
        vp2_position(1) = centre(1) + direction(1);
        vp2.set_position(vp2_position);
    }

    return true;
}

/*
 * Sample a set of n integers from the intervale [0,m] without repetitions
 *
 * @param n The number of samples without repetitions
 * @param m Defines the sampling interval [0,m]
 * @param iv will store the sampled parameters.
 */
void Vanishing_point_detector::sample_n_from_m_without_repetitions(unsigned int n, unsigned int m, Int_vector & iv)
{
    std::vector<bool> _assigned(m, false);

    if(n > m)
    {
        throw kjb::Illegal_argument("Sample n from m without repetitions, n is bigger than m!!!");
    }

    if( ( (unsigned int)iv.size() ) != n)
    {
        iv.resize(n, 0);
    }

    for(unsigned int i = 0; i < n; i++)
    {
        unsigned int position = (unsigned int) kjb_c::kjb_rand_int(0, m-i-1);
        unsigned int counter = 0;
        for(unsigned int j = 0; j < m; j++ )
        {
            if( !_assigned[j])
            {
                if(counter == position)
                {
                    _assigned[j] = true;
                    iv(i) = j;
                    break;
                }
                else
                {
                    counter++;
                }
            }
        }
    }
}

void Vanishing_point_detector::insert_penalty_into_list(double ipenalty)
{
    std::list<double>::iterator result = std::find_if( penalties.begin(), penalties.end(), std::bind2nd( ComparePenalty(), ipenalty ) );
    if(result == penalties.end())
    {
        penalties.push_back(ipenalty);
    }
    else
    {
        penalties.insert(result, ipenalty);
    }
}

double Vanishing_point_detector::compute_average_inlier_penalty(double outlier_threshold)
{
    std::list<double>::const_iterator it;
    double tot_penalty = 0.0;
    int counter = 0;
    int num_inliers = get_tot_num_segments()*(1 - outlier_threshold);
    for(it = penalties.begin(); it != penalties.end(); it++)
    {
        //std::cout << *it << std::endl;
        if(counter == num_inliers)
        {
            break;
        }
        tot_penalty += (*it);
        counter++;
    }
    return tot_penalty / ((double) num_inliers);
    //return (tot_penalty / ((double)penalties.size() ));
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      robustly_estimate_vanishing_points
 * Estimates the vanishing points for the three orthogonal directions
 * of a Manhattan world scene (where most of all planes are aligned
 * with three main orthogonal directions). This function works ONLY
 * under the Manhattan world assumption.
 *
 * Vanishing point estimation is quite robust. We start by looking
 * for vanishing points by allowing only a few outliers and keeping
 * all the edges. If we don;t find a triplet of vanishing point
 * during this first iteration, we start by removing short edges
 * that are most likely noise. We progressively remove longer edges.
 * If this still does not work we start increasing the allowed
 * number of outliers until we find a solution.
 *
 * If you want a faster estimation you can simply use the
 * find_vpts_with_ransac function of class Vanishing_point_detector
 * that looks for vanishing point only with once using the input
 * edges and the specified number of outliers
 *
 * Last, I figured it is wise to run RANSAC a little
 * longer than you would think, since very few lines will
 * intersect exactly at their vanishing point due to noise,
 * and if we don't run it long enough we may get a vanishing point
 * that is a little off its correct position
 *
 * @param vpts Will contain the estimated vanishing point
 * @param focal_length The focal length estimated from the vanishing points
 * @param img  The image for which we want to estimate the vanishing points
 * @param success_probability The probability of finding the right solution
 *        This will increase or decrease the number of Ransac iterations.
 *        This does not necessarily do what you expect, because the ratio of
 *        inliers to outliers needed to compute the number of iterations
 *        from this probability is only an estimate.
 *        It is wise to run RANSAC a little
 *        longer than you would think, since very few lines will
 *        intersect exactly at their vanishing point due to noise,
 *        and if we don't run it long enough we may get a vanishing point
 *        that is a little off its correct position
 *
 * =============================================================================
 */
bool kjb::robustly_estimate_vanishing_points
(
    std::vector<Vanishing_point> & vpts,
    double & focal_length,
    const kjb::Image & img,
    double success_probability,
    bool jointly_estimate,
    std::vector<Vanishing_point> right_ones
)
{
    using namespace std;

    if(vpts.size() != 3)
    {
        vpts.resize(3, Vanishing_point());
    }

    /** Start by using all edges longer than this threshold */
    unsigned int start_length = VPD_NORMAL_IMAGE_START_MIN_EDGE_LENGTH;

    unsigned int num_rows = img.get_num_rows();
    unsigned int num_cols = img.get_num_cols();

    unsigned int max_size = num_rows;
    if(num_cols > max_size)
    {
        max_size = num_cols;
    }

    /** If the image is very small we allow short edges to be taken
     * into account in the vanishing point estimation.
     * If the image is too big we keep only long edges
     */
    if(max_size < SMALL_IMAGE_MAX_SIZE)
    {
        start_length = VPD_SMALL_IMAGE_START_MIN_EDGE_LENGTH;
    }
    else if (max_size > BIG_IMAGE_MIN_SIZE)
    {
        start_length = VPD_BIG_IMAGE_START_MIN_EDGE_LENGTH;
    }

    /** Edge detection and removal of short edges */
    Canny_edge_detector edge_detector(VPD_SIGMA,VPD_START_CANNY_THRESHOLD, VPD_END_CANNY_THRESHOLD, VPD_EDGE_PADDING, true);
    Edge_set_ptr ptr = edge_detector(img);

    /** Let's recursively break edges that are not too straight*/
    ptr->break_edges_at_corners(VPD_BREAK_EDGE_SHORT_WINDOW_THRESHOLD, VPD_BREAK_EDGE_SHORT_WINDOW_SIZE);
    ptr->break_edges_at_corners(VPD_BREAK_EDGE_LARGE_WINDOW_THRESHOLD, VPD_BREAK_EDGE_LARGE_WINDOW_SIZE);
    /** After breaking we will have created many other short edges. We remove them according to the value
     * of start_length*/
    ptr->remove_short_edges(start_length);
    Edge_segment_set edge_set( &(*ptr));
    edge_set.remove_frame_segments(ptr->get_num_rows(),ptr->get_num_cols(),*ptr);

    Image cp(img);
    Image cp2(img);
    for(unsigned int i = 0; i < edge_set.size(); i++)
    {
        if(edge_set.get_segment(i).get_least_squares_fitting_error() < 0.5) //0.5
        {
            edge_set.get_segment(i).randomly_color(cp);
        }
    }
    ptr->randomly_color(cp2);
    cp.write("uffa.jpg");
    cp2.write("uffa2.jpg");

    Edge_set_ptr ptr_temp(new kjb::Edge_set(*ptr));

    bool found = false;
    Vanishing_point_detector vpd(edge_set, img.get_num_rows(), img.get_num_cols());

    /** First let's try with a strict outlier threshold and keeping small edges */
    found = vpd.jointly_find_vpts_with_ransac(success_probability,VPD_RANSAC_OUTLIER_THRESHOLD,
                                              VPD_RANSAC_STRICT_MAX_OUTLIER_RATIO, vpts);
    if(found && (right_ones.size() == 3))
    {
         double tempp;
         double pp = vpd.jointly_compute_ransac_penalty(VPD_RANSAC_OUTLIER_THRESHOLD, &tempp, VPD_RANSAC_STRICT_MAX_OUTLIER_RATIO, right_ones[0], right_ones[1], right_ones[2]);
         std::cout << "The right ones penalty is " << pp << std::endl;
         std::cout << "The right num outliers is:" << tempp << std::endl;
         std::cout << "The tot number of segs is: "<< vpd.get_tot_num_segments() << std::endl;
    }

    double addon = 0.05;
    int thecounter = 0;
    /** Let's now try by progressively remove longer edges */
    while((!found) && (thecounter < 18))
    {
        thecounter++;
        Edge_segment_set edge_set_temp( &(*ptr_temp));
        vpd.init_from_edge_segment_set(edge_set_temp, VPD_VERTICAL_THRESHOLD, VPD_MAX_LINE_SEGMENT_FITTING_ERROR);
        found = vpd.jointly_find_vpts_with_ransac(success_probability,VPD_RANSAC_OUTLIER_THRESHOLD,
                                                  VPD_RANSAC_STRICT_MAX_OUTLIER_RATIO + addon, vpts);
        if(found && (right_ones.size() == 3))
        {
             double tempp;
             double pp = vpd.jointly_compute_ransac_penalty(VPD_RANSAC_OUTLIER_THRESHOLD, &tempp, VPD_RANSAC_STRICT_MAX_OUTLIER_RATIO + addon,right_ones[0], right_ones[1], right_ones[2]);
             std::cout << "The right ones penalty is " << pp << std::endl;
             std::cout << "The right num outliers is:" << tempp << std::endl;
             std::cout << "The tot number of segs is: "<< vpd.get_tot_num_segments() << std::endl;
        }
        if(!found)
        {
            addon += 0.05;
        }
    }

    if(found)
    {
        std::cout << "Last, compute focal length" << std::endl;
        focal_length = vpd.compute_focal_length(vpts);
        std::cout << "The focal is:" << focal_length << std::endl;
    }

    return found;

}

double kjb::robustly_estimate_vanishing_points_Kovesi
(
    std::vector<Vanishing_point> & vpts,
    double & focal_length,
    const Image & img,
    const Line_segment_set & kovesi,
    double start_threshold,
    double vpts_tolerance,
    double success_probability
)
{
    using namespace std;

    if(vpts.size() != 3)
    {
        vpts.resize(3, Vanishing_point());
    }

    unsigned int num_rows = img.get_num_rows();
    unsigned int num_cols = img.get_num_cols();

    unsigned int max_size = num_rows;
    if(num_cols > max_size)
    {
        max_size = num_cols;
    }

    bool found = false;
    Vanishing_point_detector vpd(kovesi, img.get_num_rows(), img.get_num_cols());
    vpd.set_vpts_tolerance(vpts_tolerance);

    /** First let's try with a strict outlier threshold and keeping small edges */
    //found = vpd.jointly_find_vpts_with_ransac(success_probability,VPD_RANSAC_OUTLIER_THRESHOLD,
    //                                        start_threshold, vpts);

    double addon = 0.00;
    int thecounter = 0;
    if(vpd.get_num_vertical_segments() < 2)
    {
        thecounter = 13;
    }
    /** Let's now try by progressively remove longer edges */
    while((!found) && (thecounter < 13))
    {
        thecounter++;
        found = vpd.jointly_find_vpts_with_ransac(success_probability,VPD_RANSAC_OUTLIER_THRESHOLD,
                                               start_threshold + addon, vpts);
        if(!found)
        {
            addon += 0.05;
        }
    }

    if(found)
    {
        focal_length = vpd.compute_focal_length(vpts);
        std::cout << "The focal is:" << focal_length << std::endl;
        double tempp = 0.0;
        double pp = vpd.jointly_compute_ransac_penalty(VPD_RANSAC_OUTLIER_THRESHOLD, &tempp, VPD_RANSAC_STRICT_MAX_OUTLIER_RATIO + addon, vpts[0], vpts[1], vpts[2]);
        std::cout << "The outlier threshold is:" << tempp << std::endl;
        std::cout << "The right pen  is" << pp << std::endl;
        return start_threshold + addon;
    }
    else
    {
        addon = 0.0;
        thecounter = 0;
        /** Let's now try by progressively remove longer edges */
        while((!found) && (thecounter < 12))
        {
            thecounter++;
            found = vpd.geometric_find_vpts_with_ransac(success_probability,VPD_RANSAC_OUTLIER_THRESHOLD,
                                                   start_threshold + addon, vpts);
            if(!found)
            {
                addon += 0.05;
            }
        }
        if(found)
        {
            focal_length = vpd.get_best_focal();
            std::cout << "The focal is:" << focal_length << std::endl;
            //double tempp = 0.0;
            //double pp = vpd.jointly_compute_ransac_penalty(VPD_RANSAC_OUTLIER_THRESHOLD, &tempp, VPD_RANSAC_STRICT_MAX_OUTLIER_RATIO + addon, vpts[0], vpts[1], vpts[2]);
            //std::cout << "The outlier threshold is:" << tempp << std::endl;
            //std::cout << "The right pen  is" << pp << std::endl;
            return start_threshold + addon;
        }
    }

    return -1.0;

}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      robustly_estimate_vanishing_points
 * Estimates the vanishing points for the three orthogonal directions
 * of a Manhattan world scene (where most of all planes are aligned
 * with three main orthogonal directions). This function works ONLY
 * under the Manhattan world assumption.
 * This function uses less constraints coming from geometry
 * and uses the data more, relying on the assumption that there is
 * less noise. This is convenient with SYNTHETIC data only!!!
 *
 * @param vpts Will contain the estimated vanishing point
 * @param focal_length The focal length estimated from the vanishing points
 * @param img  The image for which we want to estimate the vanishing points
 * @param success_probability The probability of finding the right solution
 *        This will increase or decrease the number of Ransac iterations.
 *        This does not necessarily do what you expect, because the ratio of
 *        inliers to outliers needed to compute the number of iterations
 *        from this probability is only an estimate.
 *        It is wise to run RANSAC a little
 *        longer than you would think, since very few lines will
 *        intersect exactly at their vanishing point due to noise,
 *        and if we don't run it long enough we may get a vanishing point
 *        that is a little off its correct position
 *
 */
bool kjb::relaxed_vanishing_point_estimation
(
    std::vector<Vanishing_point> & vpts,
    double & focal_length,
    const kjb::Image & img,
    double success_probability
)
{
    using namespace std;

    if(vpts.size() != 3)
    {
        vpts.resize(3, Vanishing_point());
    }

    unsigned int num_rows = img.get_num_rows();
    unsigned int num_cols = img.get_num_cols();

    unsigned int max_size = num_rows;
    if(num_cols > max_size)
    {
        max_size = num_cols;
    }

    /** Edge detection and removal of short edges */
    Canny_edge_detector edge_detector(VPD_SIGMA,VPD_START_CANNY_THRESHOLD, VPD_END_CANNY_THRESHOLD, VPD_EDGE_PADDING, true);
    Edge_set_ptr ptr = edge_detector(img, true);
    ptr->remove_short_edges(VPD_RELAXED_EDGE_LENGTH);

    /** Let's recursively break edges that are not too straight*/
    ptr->break_edges_at_corners(VPD_BREAK_EDGE_SHORT_WINDOW_THRESHOLD, VPD_BREAK_EDGE_SHORT_WINDOW_SIZE);
    ptr->break_edges_at_corners(VPD_BREAK_EDGE_LARGE_WINDOW_THRESHOLD, VPD_BREAK_EDGE_LARGE_WINDOW_SIZE);
    /** After breaking we will have created many other short edges. We remove them according to the value
     * of start_length*/
    ptr->remove_short_edges(VPD_RELAXED_EDGE_LENGTH);
    Edge_segment_set edge_set( &(*ptr));
    edge_set.remove_overlapping_segments(*ptr);

    Edge_set_ptr ptr_temp(new kjb::Edge_set(*ptr));

    bool found = false;
    Vanishing_point_detector vpd(edge_set, img.get_num_rows(), img.get_num_cols(), 0.12, 0.1); //0.12
    vpd.set_use_relaxed_checks(true);

    /** First let's try with a strict outlier threshold and keeping small edges */
    found = vpd.find_vpts_with_ransac(success_probability,0.08,
                                          0.001, vpts);
    if(!found)
    {
        found = vpd.find_vpts_with_ransac(success_probability,0.08,
                                                  0.2001, vpts);
    }

    if(!found)
    {
        found = vpd.find_vpts_with_ransac(success_probability,0.08,
                                                  0.4001, vpts); //0.4001
    }

    if(found)
    {
        focal_length = vpd.compute_focal_length(vpts);
    }
    return found;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      assign_to_vanishing_point
 * Assigns a segment to the vanishing point that minimizes the
 * penalty (or label them as outliers in case no vanishing points gives
 * a penalty smaller than the input threshold).
 *
 * @param outlier_threshold The threshold above which a line
 *        segment is considered an outlier (ie it does not
 *        converge to any of the 3 vanishing points). It is the
 *        maximun difference in orientation allowed between the
 *        line segment and the segment between the midpoint of the
 *        segment and the vanishing point (in radian)
 * @param the edge segment
 * @param ivpts The vanishing points to choose from
 *
 * =============================================================================
 */
unsigned int kjb::assign_to_vanishing_point
(
    double outlier_threshold,
    const Line_segment * isegment,
    const std::vector<Vanishing_point> & ivpts
)
{
    unsigned int _index = 0;
    double alpha = 0.0;
    double temp_alpha = 0.0;
    bool found = false;

    found = false;
    for(unsigned int i = 0; i < ivpts.size(); i++)
    {
        temp_alpha = Vanishing_point_detector::compute_alpha(  ivpts[i], isegment);
        if(found)
        {
            if(temp_alpha < alpha)
            {
                alpha = temp_alpha;
                _index = i;
            }
        }
        else if(temp_alpha < outlier_threshold)
        {
            alpha = temp_alpha;
            _index = i;
            found = true;
        }
    }

    if(!found)
    {
        _index = Vanishing_point_detector::VPD_OUTLIER;
    }

    return _index;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      robustly_estimate_vanishing_points
 * Estimates the vanishing points for the three orthogonal directions
 * of a Manhattan world scene (where most of all planes are aligned
 * with three main orthogonal directions). This function works ONLY
 * under the Manhattan world assumption.
 *
 * Vanishing point estimation is quite robust. We start by looking
 * for vanishing points by allowing only a few outliers and keeping
 * all the edges. If we don;t find a triplet of vanishing point
 * during this first iteration, we start by removing short edges
 * that are most likely noise. We progressively remove longer edges.
 * If this still does not work we start increasing the allowed
 * number of outliers until we find a solution.
 *
 * If you want a faster estimation you can simply use the
 * find_vpts_with_ransac function of class Vanishing_point_detector
 * that looks for vanishing point only with once using the input
 * edges and the specified number of outliers
 *
 * Last, I figured it is wise to run RANSAC a little
 * longer than you would think, since very few lines will
 * intersect exactly at their vanishing point due to noise,
 * and if we don't run it long enough we may get a vanishing point
 * that is a little off its correct position
 *
 * @param vpts Will contain the estimated vanishing point
 * @param focal_length The focal length estimated from the vanishing points
 * @param img  The image for which we want to estimate the vanishing points
 * @param success_probability The probability of finding the right solution
 *        This will increase or decrease the number of Ransac iterations.
 *        This does not necessarily do what you expect, because the ratio of
 *        inliers to outliers needed to compute the number of iterations
 *        from this probability is only an estimate.
 *        It is wise to run RANSAC a little
 *        longer than you would think, since very few lines will
 *        intersect exactly at their vanishing point due to noise,
 *        and if we don't run it long enough we may get a vanishing point
 *        that is a little off its correct position
 *
 * =============================================================================
 */
bool kjb::robustly_estimate_vertical_vanishing_point
(
    Vanishing_point & vertical,
    const kjb::Image & img,
    double success_probability
)
{
    using namespace std;


    /** Start by using all edges longer than this threshold */
    unsigned int start_length = VPD_NORMAL_IMAGE_START_MIN_EDGE_LENGTH;

    unsigned int num_rows = img.get_num_rows();
    unsigned int num_cols = img.get_num_cols();

    unsigned int max_size = num_rows;
    if(num_cols > max_size)
    {
        max_size = num_cols;
    }

    /** If the image is very small we allow short edges to be taken
     * into account in the vanishing point estimation.
     * If the image is too big we keep only long edges
     */
    if(max_size < SMALL_IMAGE_MAX_SIZE)
    {
        start_length = VPD_SMALL_IMAGE_START_MIN_EDGE_LENGTH;
    }
    else if (max_size > BIG_IMAGE_MIN_SIZE)
    {
        start_length = VPD_BIG_IMAGE_START_MIN_EDGE_LENGTH;
    }

    /** Edge detection and removal of short edges */
    Canny_edge_detector edge_detector(VPD_SIGMA,VPD_START_CANNY_THRESHOLD, VPD_END_CANNY_THRESHOLD, VPD_EDGE_PADDING, true);
    Edge_set_ptr ptr = edge_detector(img);
    /** We can be a little more strict when removing short edges here,
     * after this first pass, most of the short edges are caused by noise.
     * This is why we use start_length*2
     */
    ptr->remove_short_edges(start_length*2);

    /** Let's recursively break edges that are not too straight*/
    ptr->break_edges_at_corners(VPD_BREAK_EDGE_SHORT_WINDOW_THRESHOLD, VPD_BREAK_EDGE_SHORT_WINDOW_SIZE);
    ptr->break_edges_at_corners(VPD_BREAK_EDGE_LARGE_WINDOW_THRESHOLD, VPD_BREAK_EDGE_LARGE_WINDOW_SIZE);
    /** After breaking we will have created many other short edges. We remove them according to the value
     * of start_length*/
    ptr->remove_short_edges(start_length);
    Edge_segment_set edge_set( &(*ptr));
    edge_set.remove_frame_segments(ptr->get_num_rows(),ptr->get_num_cols(),*ptr);

    Edge_set_ptr ptr_temp(new kjb::Edge_set(*ptr));

    bool found = false;
    double length = start_length;
    Vanishing_point_detector vpd(edge_set, img.get_num_rows(), img.get_num_cols());

    /** First let's try with a strict outlier threshold and keeping small edges */
    found =  vpd.find_vertical_vp_with_ransac(success_probability,VPD_RANSAC_OUTLIER_THRESHOLD,
                                          VPD_RANSAC_STRICT_MAX_OUTLIER_RATIO, vertical);

    /** Let's now try by progressively remove longer edges */
    while((!found) && (length < VPD_MAX_EDGE_LENGTH))
    {
        Edge_segment_set edge_set_temp( &(*ptr_temp));
        vpd.init_from_edge_segment_set(edge_set_temp, VPD_VERTICAL_THRESHOLD, VPD_MAX_LINE_SEGMENT_FITTING_ERROR);
        found =  vpd.find_vertical_vp_with_ransac(success_probability,VPD_RANSAC_OUTLIER_THRESHOLD,
                                                  VPD_RANSAC_STRICT_MAX_OUTLIER_RATIO, vertical);
        if(!found)
        {
            length += VPD_EDGE_LENGTH_INCREMENT;
            ptr_temp->remove_short_edges(length);
        }
    }

    /** If we still have not found the vanihsing points, let's allow more outliers
     *  in the scene*/
    if(!found)
    {
        vpd.init_from_edge_segment_set(edge_set, VPD_VERTICAL_THRESHOLD, VPD_MAX_LINE_SEGMENT_FITTING_ERROR);
        double max_outliers_ratio = VPD_RANSAC_MAX_OUTLIER_RATIO;
        while((!found) &&(max_outliers_ratio < VPD_MAX_OUTLIER_RATIO))
        {
            max_outliers_ratio += VPD_MAX_OUTLIER_RATIO_INCREMENT;
            found =  vpd.find_vertical_vp_with_ransac(success_probability,VPD_RANSAC_OUTLIER_THRESHOLD,
                    max_outliers_ratio, vertical);
        }
    }

    return found;

}

bool kjb::detect_vanishing_points
(
    std::vector<Vanishing_point> & vpts,
    double & focal_length,
    const std::string & img_path
)
{
    Image img(img_path);

    double scale_factor = -10.0;
    if( (img.get_num_rows() > 600) && (img.get_num_cols() > 400))
    {
        using namespace kjb_c;
        /* If the image is very big, we will just scale so that the number
         * of columns is a little above 401 pixels, remember that
         * this is the threshold above which we prune all edges that are
         * shorter than 30. */
        scale_factor = 401.0/((double)img.get_num_cols());
        kjb_c::KJB_image * cimg = 0;
        EPETE(scale_image_size
        (
            &cimg,
            img.c_ptr(),
            scale_factor
        ));

        Image tempimg(cimg);
        img = tempimg;
        img.write("./tempscaled12345678.jpg");
    }

    Line_segment_set ls;
    int max_length = 30;
    if(img.get_num_rows() < 400)
    {
        max_length = 15;
    }
    if(scale_factor < 0.0)
    {
        detect_long_connected_segments(ls, img_path, max_length);
    }
    else
    {
        detect_long_connected_segments(ls, "./tempscaled12345678.jpg", max_length);
    }

    double th = robustly_estimate_vanishing_points_Kovesi(vpts, focal_length, img, ls);
    if(th > 0.0)
    {
        if(scale_factor > 0.0)
        {
            for(unsigned int i = 0; i < 3; i++)
            {
                if(!vpts[i].is_at_infinity())
                {
                    vpts[i].set_x(vpts[i].get_x()/scale_factor);
                    vpts[i].set_y(vpts[i].get_y()/scale_factor);
                }
            }
        }
    }
    return (th > 0.0);
}


double Vanishing_point_detector::find_vpts_from_groundtruth_assignments
(
	double outlier_threshold,
	const Line_segment_set & vertical_bucket,
	const Line_segment_set & horizontal_bucket1,
	const Line_segment_set & horizontal_bucket2,
    std::vector<Vanishing_point> & vpts
)
{

    if(vpts.size() != 3)
    {
        vpts.resize(3, Vanishing_point());
    }

    unsigned int iterations = 50;

    double full_outlier_threshold = 0.0;

    if( (horizontal_bucket1.size() < 2) || (horizontal_bucket2.size() < 2))
    {
        vpts[0].set_type(Vanishing_point::INFINITY_UP);
        vpts[1].set_type(Vanishing_point::INFINITY_RIGHT);
        vpts[0].set_x(_num_cols/2.0);
        vpts[0].set_y(_num_rows/2.0);
        return true;
    }
    for(int i = 0; i < vertical_bucket.size(); i++)
    {
        full_outlier_threshold += vertical_bucket.get_segment(i).get_length();
    }
    for(int i = 0; i < horizontal_bucket1.size(); i++)
    {
        full_outlier_threshold += horizontal_bucket1.get_segment(i).get_length();
    }
    for(int i = 0; i < horizontal_bucket2.size(); i++)
    {
        full_outlier_threshold += horizontal_bucket2.get_segment(i).get_length();
    }

    /**
     * We store the 4 lines from which we compute the vanishing points
     * in here. The first two will be used to compute the first vanishing
     * points, third and fourth two will be used to compute the
     * second vanishing point
     */
    Int_vector horizontal_segments1(2);
    Int_vector horizontal_segments2(2);
    Int_vector vertical_segments(6);

    Vector temp_vertical_position(2);
    Vanishing_point temp_vertical;

    Vector temp_vp1_position(2);
    Vanishing_point temp_vp1;

    Vector temp_vp2_position(2);
    Vanishing_point temp_vp2;

    int num_outliers;
    double best_num_outliers = 0.0;
    double outliers_ratio = 0.0;
    double best_penalty = 100000000;
    double penalty = 0.0;
    double focal = 0.0;

    bool found = false;

    double opx = 0.0;
    double opy = 0.0;
    for(unsigned int i = 0; i < iterations; i++)
     {
         sample_n_from_m_without_repetitions(2, horizontal_bucket1.size(), horizontal_segments1);
         sample_n_from_m_without_repetitions(2, horizontal_bucket2.size(), horizontal_segments2);
         sample_n_from_m_without_repetitions(2, vertical_bucket.size(), vertical_segments);

         /** Let's compute the first vanishing point */
         if( !Line::find_line_intersection(horizontal_bucket1.get_segment(horizontal_segments1[0]).get_line(),
        		                           horizontal_bucket1.get_segment(horizontal_segments1[1]).get_line(),
                                           temp_vp1_position) )
         {
             /** The line segments do not intersect, let's put the vanishing point at infinity */
             temp_vp1.set_type(Vanishing_point::INFINITY_RIGHT);
         }
         else
         {
             temp_vp1.set_position(temp_vp1_position);
         }

         /** Let's compute the first vanishing point */
         if( !Line::find_line_intersection(horizontal_bucket2.get_segment(horizontal_segments2[0]).get_line(),
        		                           horizontal_bucket2.get_segment(horizontal_segments2[1]).get_line(),
                                           temp_vp2_position) )
         {
             /** The line segments do not intersect, let's put the vanishing point at infinity */
             temp_vp2.set_type(Vanishing_point::INFINITY_RIGHT);
         }
         else
         {
             temp_vp2.set_position(temp_vp2_position);
         }

         if( !Line::find_line_intersection(vertical_bucket.get_segment(vertical_segments[0]).get_line(),
        		                           vertical_bucket.get_segment(vertical_segments[1]).get_line(),
                                           temp_vertical_position) )
         {
             /** The line segments do not intersect, let's put the vanishing point at infinity */
             temp_vertical.set_type(Vanishing_point::INFINITY_UP);
         }
         else
         {
             temp_vertical.set_position(temp_vertical_position);
             if( (temp_vertical.get_y() > 0 ) && (temp_vertical.get_y() < _num_cols))
             {
                 continue;
             }
         }

         double temppx = 0.0;
         double temppy = 0.0;
         if(!use_relaxed_checks)
         {
             if(! groundtruth_check_vpts_consistency(temp_vp1, temp_vp2, temp_vertical, focal, temppx, temppy) )
        	 {
                 std::cout << "VPTS NOT CONSISTENT" << std::endl;
            	 continue;
             }
         }

         penalty = groundtruth_compute_ransac_penalty(outlier_threshold, &num_outliers,
        		 vertical_bucket, horizontal_bucket1, horizontal_bucket2, temp_vertical, temp_vp1, temp_vp2);
         if(num_outliers == 0)
         {
             if(found)
             {
                 if(best_penalty > penalty)
                 {
                     best_penalty = penalty;
                     best_num_outliers = num_outliers;
                     vpts[VPD_HORIZONTAL_VP_1] = temp_vp1;
                     vpts[VPD_HORIZONTAL_VP_2] = temp_vp2;
                     vpts[VPD_VERTICAL_VP] = temp_vertical;
                     opx = temppx;
                     opy = temppy;
                 }
             }
             else
             {
                 best_penalty = penalty;
                 best_num_outliers = num_outliers;
                 vpts[VPD_HORIZONTAL_VP_1] = temp_vp1;
                 vpts[VPD_HORIZONTAL_VP_2] = temp_vp2;
                 vpts[VPD_VERTICAL_VP] = temp_vertical;
                 found = true;
                 opx = temppx;
				 opy = temppy;
             }
         }
     }

   return found;
}

