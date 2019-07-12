/* $Id: g2_ransac_line_fitting.cpp 12267 2012-05-07 02:54:58Z ksimek $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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
   |  Author:  Jinyan Guan
 * =========================================================================== }}}*/

#include <prob_cpp/prob_sample.h>
#include <prob_cpp/prob_distribution.h>

#include "g2_cpp/g2_ransac_line_fitting.h"

namespace kjb
{

bool Ransac_line_fitting::run(size_t max_num_iter)
{
    size_t iter = 0;
    bool success = false;
    while(iter < max_num_iter)
    {
        // std::cout<<"=========================================\n";
        // std::cout<<"iter: "<<iter<<std::endl;
        consensus_set.clear();

        // step 1: Randomly select values from data 
        // for line fitting, we need two points 
        size_t num_observations = observations.size();
        Vector probs(static_cast<int>(num_observations), 1.0/num_observations);
        Categorical_distribution<int> dist(probs);  

        size_t index_1 = sample(dist)-1;
        size_t index_2 = sample(dist)-1;
        while(index_2 == index_1)
        {
            index_2 = sample(dist)-1;
        }

        // Construct a line from the randomly selected points 
        Line maybe_model(observations[index_1], observations[index_2]);
        // std::cout<<"index_1: "<<index_1<<std::endl;
        // std::cout<<"index_2: "<<index_2<<std::endl;
        consensus_set.push_back(Vector(observations[index_1]));
        consensus_set.push_back(Vector(observations[index_2]));

        // Find all the points that fit to the maybe_model within the
        // threshold 
        for(size_t i = 0; i < observations.size(); i++)
        {
            if(i == index_1 || i == index_2)
                continue;

            // Check the fitting error 
            if(maybe_model.find_distance_to_point(observations[i]) < threshold)
            {
                consensus_set.push_back(Vector(observations[i]));
            }
        }

        // Check whether we have found a best model
        if(consensus_set.size() >= num_inliers_required)
        { 
            success = true;
            Line fitted_line;
            double error = 0.0;
            // Fit a line to all points in the consensus_set 
            // using least square 
            double sum_x = 0.0; 
            double sum_y = 0.0; 
            double sum_x_square = 0.0;
            double sum_xy = 0.0;
            size_t num_points = consensus_set.size();
            // std::cout<<"Num of points in consensus_set: "<<num_points<<std::endl;
            for(size_t i = 0; i < num_points; i++)
            {
                double x = consensus_set[i](0);
                double y = consensus_set[i](1);
                sum_x += x;
                sum_y += y;
                sum_x_square += x*x;
                sum_xy += x*y;
            }
            double denominator = (num_points*sum_x_square) - (sum_x*sum_x);
            if(fabs(denominator) < FLT_EPSILON)
            {
                // The line is perpendicular to the x axis
                fitted_line.set_a(0.0);
                fitted_line.set_b(-1.0);
                // Use the average x 
                fitted_line.set_c(sum_x/num_points);

                // compute the error 
                // Here we use the difference along the x axis 
                for(size_t i = 0; i < num_points; i++)
                {
                    double line_x = fitted_line.get_c();
                    error += fabs(consensus_set[i](0) - line_x);
                }
            }
            else
            {
                double intercept = ((sum_x_square*sum_y) - (sum_x*sum_xy))/denominator;
                double slope = (num_points*sum_xy - sum_x*sum_y)/denominator;
                fitted_line.set_a(slope);
                fitted_line.set_b(-1.0);
                fitted_line.set_c(intercept);

                // compute the error
                for(size_t i = 0; i < num_points; i++)
                {
                    double x = consensus_set[i](0);
                    double y_p = consensus_set[i](1);
                    double y_at_x = fitted_line.compute_y_coordinate(x);
                    error += fabs(y_p - y_at_x);
                }
            }
            error /= num_points;
            //std::cout<<"error: "<<error<<std::endl;
            if(error < best_error)
            {
                // Update the best error and model
                best_error = error; 
                best_line = fitted_line;
            }
        }
        //std::cout<<"best_error: "<<best_error<<std::endl;
        iter++;
    }

    return success;
}

} // namespace kjb
