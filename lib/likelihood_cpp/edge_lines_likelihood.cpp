/* $Id: edge_lines_likelihood.cpp 21596 2017-07-30 23:33:36Z kobus $ */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
   |  Author: Jinyan Guan 
 * =========================================================================== */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "likelihood_cpp/edge_lines_likelihood.h"
#include <sample/sample_gauss.h>
#include <cmath>
#include <iostream>

#define KJB_LS_MAX_RGB_VALUE 255
#define M_PI_7 M_PI/7.0

using namespace kjb;

/**
 * @brief Compare the corresponding Line_bin based on the x position of 
 * the starting point of the image edge segment 
 */
bool Line_correspondence::Line_bin::Compare_edge_segment_starting_points::operator()
(
    const Line_correspondence::Line_bin*    line_bin1,
    const Line_correspondence::Line_bin*    line_bin2
) const
{     
    return line_bin1->image_edge_segment.get_start_x() < 
        line_bin2->image_edge_segment.get_start_x();
}

/**
 * @brief Compare the correspondence based on the distance between the 
 * middle points of the model edge and edge segment
 */
bool Line_correspondence::Line_bin::operator< (const Line_correspondence::Line_bin& l_bin) const
{
    if(fabs(distance - l_bin.distance) < 5)
        return angle < l_bin.angle; 
    else 
        return distance < l_bin.distance; 
}

/**@class Line_correspondence
 * Represents the correspondence between an edge segment and a model edge, it greedily 
 * mapps a model edge to an edge segment based on the distance between the middle point
 * of the edge segment to the model edge
 */
Line_correspondence::Line_correspondence
(
    const Edge_segment_set&     iimage_edge_segments,
    double                      angle_sigma,
    double                      dist_sigma,
    double                      max_distance,
    double                      max_angle
) : m_max_distance(max_distance),
    m_max_angle(max_angle),
    m_angle_sigma(angle_sigma),
    m_dist_sigma(dist_sigma),
    image_edge_segments(iimage_edge_segments), 
    m_image_corresponding_model_edges(image_edge_segments.size())
{
    // Some preprocessing (need to think about it after CVPR)
}

 /** 
 * @brief Constructs a new Lin_bin between an image edge and a model edge 
 */
Line_correspondence::Line_bin::Line_bin
(
    const Edge_segment&         iimage_edge_segment,
    const Model_edge&           model_edge_segment,
    unsigned int                imodel_edge_segment_index,
    double                      angle_sigma,
    double                      dist_sigma
) : image_edge_segment(iimage_edge_segment),
    model_edge_segment_index(imodel_edge_segment_index)
{
    Vector center = image_edge_segment.get_centre();
    distance = model_edge_segment.get_distance_from_point(center);
    kjb_c::gaussian_pdf(&gauss_distance, distance, 0, dist_sigma);
    angle = image_edge_segment.get_angle_between_line(model_edge_segment);
    kjb_c::gaussian_pdf(&gauss_angle, angle, 0, angle_sigma);
}

/**
 * @brief Find the corresponding image edges for each model edge based on the m_image_correspondence 
 */
void Line_correspondence::find_model_corresponding_image_edges
(
    const std::vector<Model_edge>&  model_edge_segments
)
{
    //Debug 
//    std::cout<<"model_edge_segments: "<<model_edge_segments.size()<<std::endl;

    for (unsigned int i = 0; i < m_image_corresponding_model_edges.size(); i++)
    {
        m_image_corresponding_model_edges[i].clear();
    }
    
    for (unsigned int i = 0; i < m_model_corresponding_image_edges.size(); i++)
    {
        m_model_corresponding_image_edges[i].clear();
    }
    
    noisy_image_edge_segments.clear();
    
    m_model_corresponding_image_edges.reserve(model_edge_segments.size());
    noisy_image_edge_segments.reserve(image_edge_segments.size());
    
    //TODO: Need to change to proper error handling  
//    ASSERT(model_edge_segments.size() > 0);   

    for (unsigned int i = 0; i < image_edge_segments.size(); i++)
    {
        const Edge_segment& image_edge = image_edge_segments.get_segment(i);
        
        //For each image edge image, calculate the Euclidian distance from its middle point to the model edge
        for (unsigned int j = 0; j < model_edge_segments.size(); j++)
        {
            const Model_edge& model_edge = model_edge_segments[j];
            m_image_corresponding_model_edges[i].push_back(Line_bin(image_edge, model_edge, j, m_angle_sigma, m_dist_sigma));
        }
        
        //Sort the m_image_to_model_corrspondence according to the Euclidean distance from the middle point
        //of the image edge segment to the model edge 
        m_image_corresponding_model_edges[i].sort();
//        std::sort(m_image_corresponding_model_edges[i].begin(), m_image_corresponding_model_edges[i].end()); 
    }

    
    //Find the corresponding image edges for each model edge
    m_model_corresponding_image_edges.resize(model_edge_segments.size());
  
    for (unsigned int i = 0; i < m_image_corresponding_model_edges.size(); i++)
    {
        //An image edge segment is only mapped to its closed model edge 
        unsigned int model_index = m_image_corresponding_model_edges[i].front().model_edge_segment_index;
//        std::cout<<i<<"'s image edge will be mapped to model edge: "<<model_index<<std::endl;
        //Debug
//        unsigned int j = 0;
//        for(std::list<Line_bin>::iterator iter = m_image_corresponding_model_edges[i].begin();
//                iter != m_image_corresponding_model_edges[i].end(); iter++, j++)
//        {
//            unsigned int model_index = iter->model_edge_segment_index;
//            double dist = iter->distance;
//            double angle = iter->angle;
//            std::cout<<j<<" model index: " << model_index <<" dist: "<<dist << angle <<" angle: "<<angle<<std::endl;
//            std::cout<<"model edge: "<<model_edge_segments[model_index].get_start()<<" ";
//            std::cout<<model_edge_segments[model_index].get_end()<<std::endl;
//            const Edge_segment& image_edge = iter->image_edge_segment;
//            std::cout<<"image edge center: "<<image_edge.get_centre()<<std::endl;
//        }
//        std::cout<<"################################"<<std::endl;

        //Check whether the distance between the image edge and the model edge is too far away
        if(m_image_corresponding_model_edges[i].front().distance < m_max_distance &&
                m_image_corresponding_model_edges[i].front().angle < m_max_angle )
            //      m_image_corresponding_model_edges[i].front().image_edge_segment.get_length() > model_edge_length*0.5) 
        { 
            m_model_corresponding_image_edges[model_index].push_back(&m_image_corresponding_model_edges[i].front());
        }
        else // This is a noisy image edge  
        {
//            std::cout<<"image edge " <<i <<"is a noisy image for model image "<<m_image_corresponding_model_edges[i].front().model_edge_segment_index<<std::endl;  
            noisy_image_edge_segments.push_back(&(m_image_corresponding_model_edges[i].front().image_edge_segment));
        }
    }

    //Sort the corresponding image edge segments of each model edge segment based
    //on the starting point of the image edge segments 
   
    for (unsigned int i = 0; i < m_model_corresponding_image_edges.size(); i++)
    {
        //If a model edge has any corresponding image edge segments 
        if(m_model_corresponding_image_edges[i].size()>1)    
        {
            std::sort(m_model_corresponding_image_edges[i].begin(), m_model_corresponding_image_edges[i].end(),
                Line_bin::Compare_edge_segment_starting_points());
        }
    }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/**
 * @brief Constructs a new Edge_lines_likelihood 
 * @param dist_sigma    The standard diviation of the probility density function 
 *                      of the distance between the middle point of the image 
 *                      edge segment and the model edge
 * @param angle_sigma   The sstandard diviation of the probability density function 
 *                      of the angle between the image edge segment and the model edge
 * @param prob_of_missing   The probability of an edge element being missing
 * @param prob_of_noise     The probability of an edge element being noise
 * @param max_distance      The maximum distance allows to  edge segments of a model edge within
 */
Edge_lines_likelihood::Edge_lines_likelihood
(
    const Edge_segment_set&  iimage_edge_segments,
    double                   angle_sigma,
    double                   dist_sigma,
    double                   prob_of_missing,
    double                   prob_of_noise,
    double                   max_distance,
    double                   max_angle,
    double                   collinear_distance_threshold
) : image_edge_segments(iimage_edge_segments),
    m_prob_of_missing(prob_of_missing),
    m_prob_of_noise(prob_of_noise),
    m_line_correspondence(iimage_edge_segments, angle_sigma, dist_sigma, max_distance, max_angle),
    m_collinear_distance_threshold(collinear_distance_threshold)
{
    m_prob_of_missing = MAX_OF(m_prob_of_missing, MIN_LOG_ARG);
    m_prob_of_noise = MAX_OF(m_prob_of_missing, MIN_LOG_ARG);
}


/**
 * @brief Calculates and return the likelihood for a set of model edges */
double Edge_lines_likelihood::operator()
(
    std::vector<Model_edge>& model_edges
)
{
    //If there's no model edge rendered, all the image edge segments are noisy edges
    if (model_edges.size() == 0)
    {
//        double noise_edge_length = 0.0;
//        for (unsigned int i = 0; i < image_edge_segments.size(); i++)
//        {
//            noise_edge_length += image_edge_segments.get_segment(i).get_length();
//        }
//
//        return m_prob_of_noise*log(noise_edge_length);
        return log(-DBL_EPSILON);
    }

    double detected_edge_length_total = 0.0; 
    double missing_edge_length_total = 0.0;
    double noisy_edge_length_total = 0.0;
    double log_ll = 0.0;
    //for debug
    double model_length_total = 0.0;
    
    //Step 1: find the corresponding edge segments of each model edge 
    m_line_correspondence.find_model_corresponding_image_edges(model_edges);
        
    const std::vector<std::vector<Line_correspondence::Line_bin*> >& model_corresponding_edges = m_line_correspondence.get_model_corresponding_image_edges(); 
    ASSERT(model_corresponding_edges.size() == model_edges.size());

    //Step 2: Calculate the density function of the likelihood
    for (unsigned int i = 0; i < model_corresponding_edges.size(); i++)
    {
        Model_edge& model_segment = model_edges[i];
        double density = 0.0; 
        unsigned int num_image_edges = model_corresponding_edges[i].size();

        model_length_total += model_segment.get_length();
//        std::cout<<i<<"'s Model Edge segment "<<std::endl;

        if(num_image_edges > 0)
        {
            Vector previous_center(3); 
            bool mapped = false;
            unsigned int num_mapped = 0;

            for (unsigned int j = 0; j < num_image_edges; j++)
            {
                Line_correspondence::Line_bin* corr = model_corresponding_edges[i][j];
                const Edge_segment& image_edge = corr->image_edge_segment;
                double noisy_length = 0.0;              
                model_segment.update_detected_segments(image_edge, m_collinear_distance_threshold, previous_center, mapped, noisy_length);  

                if(mapped)
                {
                    double s = corr->gauss_distance*corr->gauss_angle;
//                    std::cout<<"corr->distance: "<<corr->distance <<" corr->angle: "<<corr->angle<<std::endl;
//                    std::cout<<"corr->gauss_distance: "<<corr->gauss_distance <<" corr->gauss_angle: "<<corr->gauss_angle<<std::endl;
//                    std::cout<<"s: "<<s<<std::endl; 
                    density += s > MIN_LOG_ARG ? log(s) : log(MIN_LOG_ARG);
                    if(std::isnan(density))
                        std::cout<<"density is Nan"<<std::endl;
                            
                    num_mapped++;
                }
                noisy_edge_length_total += noisy_length;
//              std::cout<<j <<"'s image edge segment" <<std::endl; 
            }
            //Add the average density of the image edge segments to the likelihood
//            std::cout<<"Num of image segments: "<<num_image_edges<<std::endl;
//            std::cout<<"num_mapped: "<<num_mapped<<std::endl;
            if(num_mapped > 0)
            {
//                std::cout<<"average density is : "<<(density/num_mapped)<<std::endl; 
                log_ll += density/num_mapped;
            }

            double detected = model_segment.get_detected_length();
            double missing = model_segment.get_length() - detected;
            detected_edge_length_total += detected; 
            missing_edge_length_total += missing;
        }
        // There's no image edge associated with this model edge, count it as a missing image edge,
        // the missing length is approxiated by the length of the model edge segment 
        else
        {
            missing_edge_length_total += model_segment.get_length();
        }
//        std::cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<std::endl;
    }
    // Count the nonmapped image edge segments as noisy edge length 
    std::vector<const Edge_segment*> noisy_edges = m_line_correspondence.get_noisy_edge_segments();
    for(unsigned int i = 0; i < noisy_edges.size(); i++)
    {
        noisy_edge_length_total += noisy_edges[i]->get_length();
    }

//    std::cout<<"density: "<<log_ll<<std::endl;   
//    std::cout<<"Missing edge length: "<<missing_edge_length_total<<std::endl;
//    std::cout<<"noisy edge length: "<<noisy_edge_length_total<<std::endl;
//    std::cout<<"detected edge length: "<<detected_edge_length_total<<std::endl;
//    std::cout<<"Model length total: "<<model_length_total<<std::endl;
    log_ll += detected_edge_length_total*log(1.0-m_prob_of_missing);
    log_ll += missing_edge_length_total*log(m_prob_of_missing);
    log_ll += noisy_edge_length_total*log(m_prob_of_noise);

    return log_ll;

}

//For debug purposes, draw the correspondence 
void Edge_lines_likelihood::draw_correspondence
(
    Image&          model_edge_image, 
    Image&          data_edge_image,
    const           std::vector<Model_edge>& model_edges, 
    double          width
)
{
    if(model_edges.size() == 0)
        KJB_THROW_2(Illegal_argument,"model_edges's size is 0 for Edge_lines_likelihood");

    m_line_correspondence.find_model_corresponding_image_edges(model_edges);
    
    std::vector<std::vector<Line_correspondence::Line_bin*> > model_corresponding_edges =
        m_line_correspondence.get_model_corresponding_image_edges(); 
  
    std::vector< std::vector< Line_correspondence::Line_bin*> >::iterator iter_outer; 
    std::vector<Line_correspondence::Line_bin*>::iterator iter_inner;
    
    for (iter_outer = model_corresponding_edges.begin(); 
            iter_outer != model_corresponding_edges.end();
            iter_outer++)
    {
        if((*iter_outer).size() > 0)
        {
            const Model_edge& model_edge = model_edges[(*iter_outer).front()->model_edge_segment_index];
        
            //Create a random color to draw the model_edge
            double tempr =(double)kjb_c::kjb_rand()*KJB_LS_MAX_RGB_VALUE;
            double tempg =(double)kjb_c::kjb_rand()*KJB_LS_MAX_RGB_VALUE;
            double tempb =(double)kjb_c::kjb_rand()*KJB_LS_MAX_RGB_VALUE;

            if( (tempr < 50.0) && (tempg < 50.0) && (tempb < 50.0) )
            {
                tempr = 255;
            }
            
            model_edge.draw(model_edge_image, tempr, tempg, tempb, width);
            
            //Draw the corresponding image edge segments 
            for (iter_inner = (*iter_outer).begin(); iter_inner != (*iter_outer).end(); iter_inner++)
            {
//                std::cout<<" Distance: "<<(*iter_inner)->distance<<std::endl;
                const Edge_segment& image_edge = (*iter_inner)->image_edge_segment;
                image_edge.draw(data_edge_image, tempr, tempg, tempb, width);

            }
        }
    }
}
