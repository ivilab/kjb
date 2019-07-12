/* $Id: edge_lines_likelihood.h 18283 2014-11-25 05:05:59Z ksimek $ */
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
#ifndef EDGE_LINES_LIKELIHOOD_H_INCLUDED
#define EDGE_LINES_LIKELIHOOD_H_INCLUDED

#include <edge_cpp/edge.h>

#include <camera_cpp/perspective_camera.h>

#include <likelihood_cpp/model_edge.h>
#include <likelihood_cpp/edge_likelihood_util.h>
#include <list>

/**
 * @file collinear_edge_lines_likelihood.h This file contains functions used 
 * to compute the likelihood of a set of detected collinear edge segments given
 * a set of backprojected model edges. We treat the collinear edge segments are 
 * conditionally independent of each other given the model. We approaximate the
 * likelihood by the ignoring the noise and expected non-edge points. 
 */
namespace kjb {

/**@class Line_correspondence
 * Represents the correspondence between an edge segment and a model edge, it greedily 
 * mapps a model edge to an edge segment based on the distance between the middle point
 * of the edge segment to the model edge
 */

class Line_correspondence
{
    public:

        /**
         * @brief Creates a new correspondence between collinear edge segments
         * and model edge lines
         */
        Line_correspondence
        (
            const Edge_segment_set&             image_edge_segments,
            double                              angle_sigma,
            double                              dist_sigma,
            double                              max_distance,
            double                              max_angle
        );

        ~Line_correspondence(){};

         /**
         * @class Line Represents the correspondence between a model edge 
         * and a collinear edge segment in the image
         */        
        class Line_bin 
        {
             public:
                
                 /** 
                 * @brief Constructs a new bin between an Model_edge 
                 * and a Edge_segment
                 */
                Line_bin 
                (
                    const Edge_segment&         iimage_edge_segment, 
                    const Model_edge&           model_edge_segment, 
                    unsigned int                imodel_edge_segment_index,
                    double                      angle_sigma,
                    double                      dist_sigma
                );
                
                /**
                 * @brief Compare the corresponding Line_bin based on the x position of 
                 * the starting point of the image edge segment 
                 */
                class Compare_edge_segment_starting_points
                {     
                    public:
                        bool operator() (const Line_bin* line_bin1, const Line_bin* line_bin2) const;
                };

                /**
                 * @brief Compare the correspondence based on the distance between the 
                 * middle points of the model edge and edge segment
                 */
                bool operator< (const Line_bin& l_bin) const;

                const Edge_segment& image_edge_segment;

                unsigned int model_edge_segment_index;

                /** 
                 * @brief Euclidean distance from the middle point of the image edge segment 
                 * to the model edge 
                 */
                float distance; 

                /** @brief normal pdf of distance */
                double gauss_distance;
                                     
                /**
                 * @brief Angle between an image edge segment and a model edge  
                 * Because of symmetry, ranges 0 to PI/2.
                 */
                float angle;

                /** @brief normal pdf of the angle. */
                double gauss_angle;
        }; 

        /**
         * @brief Return a map of model_to_image_correspondence
         * Indexed by image edge segments and sorted by the gauss_distance  
         */
        inline const std::vector<std::vector<Line_bin*> >& get_model_corresponding_image_edges ( ) const
        {
            return m_model_corresponding_image_edges;
        }

        inline double get_max_distance() const
        {
            return m_max_distance;
        }

        inline std::vector<const Edge_segment*> get_noisy_edge_segments()
        {
            return noisy_image_edge_segments;
        }

        /**
         * @brief Find the correspoinding image edges for each model edge based on
         * m_image_edge_correspondence, the correspondence will be stored in 
         * m_model_edge_correspondence
         */ 
        void find_model_corresponding_image_edges
        (
            const std::vector<Model_edge>&  model_edge_segments
        ); 

    private: 

        /**
         * @brief The maximum distance allows an edge segment mapped to a model edge  */
        double m_max_distance;
   
        /**
         * @brief The maximum angle allows an edge segment mapped to a model edge  */
        double m_max_angle;
        
        double m_angle_sigma;

        double m_dist_sigma;

        const Edge_segment_set& image_edge_segments;

        /** 
         * @brief vector stores the correspondence between model line and image 
         * edge segment, indexed by the image edge segment 
         * 
         */
        std::vector<std::list<Line_bin> > m_image_corresponding_model_edges;

        /** 
         * @brief A vector stores the correspondence between model line and image 
         * edge segment, indexed by the model edge segment 
         * 
         */
        std::vector<std::vector<Line_bin*> > m_model_corresponding_image_edges; 

        /** @brief A vector stores the noisy image edge segments. */
        std::vector<const Edge_segment*> noisy_image_edge_segments; 

};

/**
 * @class Computes the likelihood between the image edges and the backprojected model
 * edges by considering the detected edge segments are conditional independent
 * given the model 
 */
class Edge_lines_likelihood
{
    public:
        
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
        Edge_lines_likelihood
        (
             const Edge_segment_set&  iimage_edge_segments,
             double                   angle_sigma,
             double                   dist_sigma,
             double                   prob_of_missing,
             double                   prob_of_noise,
             double                   max_distance,
             double                   max_angle,
             double                   collinear_distance_threshold
        );
        
        /** Template function to compute the likelihood for a generic model.
         *  The input model must generate a set of of model edges, 
         *
         *  @param model The model to compute the likelihood for
         */
        template <class T> double compute_likelihood(T & model);
    
        template <class T, class U>
        double compute_likelihood(T & model, U & camera);
        
        /**
         * @brief Calculates and return the likelihood for a set of model edges */
        double operator()(std::vector<Model_edge>& model_edges);

        //For debug purposes, draw the correspondence 
        void draw_correspondence
        (
            Image&          model_edge_image, 
            Image&          data_edge_image,
            const           std::vector<Model_edge>& model_edges, 
            double          width
        );

    private:

        /** @brief This contains the set of edges we want to compare to the detected image
         *          edges. We expect each edge point in this map to be rendered with a color
         *          that enables us to retrieve the edge information from a vector (ie
         *          pixel color = id of the edge the pixel comes from). This id must
         *          be sequential (1 = first edge, 2 = second edge and so on).
         *          This is stored here, for efficiency region, so that we don't
         *          have to reallocate the matrix every time we need to compute the
         *          likelihood
         */
        kjb::Int_matrix model_map;

        /** @brief The detected edge line segments from the input image. The likelihood is computed
         *      *  by comparing a new set of edges against this one. This is not
         *           *  owned by this class (memory allocation not handled)*/
        const Edge_segment_set& image_edge_segments;
        

        /** @brief The probability of an image edge segment missing some part*/        
        double m_prob_of_missing;

        /** @brief The probability of an image edge segment being noise */ 
        double m_prob_of_noise;

        /** @brief The correspondence between the image edge segment and model edge segment */        
        Line_correspondence m_line_correspondence;

        /** @brief The distance threshold to determine whether the segment are collinear */
        double m_collinear_distance_threshold;
};

    /** Template function to compute the likelihood for a generic model.
     *  The input model must be able to generate a model map and a set
     *  of model edges, or an edgeset (those are the two kind of features
     *  we can compute a likelihood for so far). Most of the time, you
     *  will likely write a specialization for this function, this
     *  is mostly a guideline
     *
     *  @param model The model to compute the likelihood for
     *  @param mode   Mode for likelihood computation. 0 means that the likelihood
     *                will be computed by generating a model map and a set of
     *                edges from the model and the camera, 1 means that we will generate
     *                a set of edge points.
     */
    template <class T>
    double Edge_lines_likelihood::compute_likelihood(T & model)
    {
        std::vector<Model_edge> model_edges;
        model.prepare_model_map(model_map);
        model.prepare_rendered_model_edges(model_edges, model_map);
        return this->operator()(model_edges);
    }
    
    /** @brief Template function to compute the likelihood for a generic model under
     *  a given camera.
     *  The input model, with the camera, must be able to generate a model map and a set
     *  of model edges, or an edgeset (those are the two kind of features
     *  we can compute a likelihood for so far). Most of the time, you
     *  will likely write a specialization for this function, this
     *  is mostly a guideline
     *
     *  @param model The model to compute the likelihood for
     *  @param camera The camera to be used to render the model
     *  @param mode   Mode for likelihood computation. 0 means that the likelihood
     *                will be computed by generating a model map and a set of
     *                edges from the model and the camera, 1 means that we will generate
     *                a set of edge points.
     */
    template <class T, class U>
    double Edge_lines_likelihood::compute_likelihood(T & model, U & camera)
    {
        std::vector<Model_edge> model_edges;
        model.prepare_model_map(model_map);
        model.prepare_rendered_model_edges(model_edges, model_map);
        return this->operator()(model_edges);
    }
}

#endif
