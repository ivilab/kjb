/* $Id $ */

/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief Declarations for Line class
 *
 * Copyright (c) 2003-2008 by members of University of Arizona Computer Vision
 * group. For use outside the University of Arizona Computer Vision group
 * please contact Kobus Barnard.
 */

#ifndef EDGE_FEATURES_MANAGER_H
#define EDGE_FEATURES_MANAGER_H

#include "l_cpp/l_exception.h"
#include "l_cpp/l_readable.h"
#include "l_cpp/l_writeable.h"
#include "edge_cpp/line_segment_set.h"
#include "edge_cpp/edge.h"
#include "edge_cpp/manhattan_world.h"

#define FM_DEFAULT_EDGE_SIGMA 1.0
#define FM_DEFAULT_EDGE_BEGIN 2.55
#define FM_DEFAULT_EDGE_END 2.04
#define FM_DEFAULT_EDGE_PADDING 30
#define FM_DEFAULT_VP_SUCCESS_PROBABILITY 0.999
#define FM_DEFAULT_VP_ASSIGNMENT_THRESHOLD 0.16

namespace kjb {
/**
 * @class Features manager
 *
 * @brief Allows to manipulate basic 2d image features (edges, fitted line
 *        segments and manhattan worl). This is mostly useful for Manhattan
 *        world scenes so it might be renamed.
 *        This class contains the following features:
 *        - edges          detected from an image
 *        - edge segments  line segments fitted to the image edges
 *        - manhattan world features for a Manhattan world scene
 *
 *        This class mostly offers IO functions. Features can be written (read)
 *        to a file. Only the available features will be written (read).
 *
 *        The class also manages options for feature detection
 *
 * For more details on each feature, it is recommended to check
 * the relevant file.
 * For edges: edge_cpp/edge.h
 * For edge_segments: edge_cpp/line_segment.h
 * For Manhattan scene features: edge_cpp/manhattan_world.h
 */
class Features_manager : public Readable, public Writeable
{
    public:

        /** @brief Constructs a features manager. All options are set to default values
         *  All the requested features will be detected */
        Features_manager
        (
            const kjb::Image & img,
            bool idetect_edges = true,
            bool ifit_edge_segments = true,
            bool icreate_manhattan_world = true
        );

        /** @brief Constructs a features manager. All options are set to default values
         *  All the requested features will be detected */
        Features_manager
        (
            bool read_image,
            const std::string & img_path,
            bool idetect_edges = true,
            bool ifit_edge_segments = true,
            bool icreate_manhattan_world = true
        );

        /** @brief Constructs a features manager. If one input pointer
         * is NULL, it will be assumed that that feature is not available */
        Features_manager
        (
            kjb::Edge_set * edges,
            Edge_segment_set * edge_segments,
            Manhattan_world * manhattan_world
        );

        /** @brief Constructs a features manager.
         *  All the requested features will be detected */
        Features_manager
        (
            const kjb::Image & img,
            float iblurring_sigma,
            float ibegin_threshold,
            float iend_threshold,
            unsigned int ipadding,
            bool iuse_fourier,
            double ivanishing_point_detection_success_probability = FM_DEFAULT_VP_SUCCESS_PROBABILITY,
            double ioutlier_threshold_for_vanishing_points_assignment = FM_DEFAULT_VP_ASSIGNMENT_THRESHOLD,
            bool idetect_edges = true,
            bool ifit_edge_segments = true,
            bool icreate_manhattan_world = true
        );

        /** @brief Constructs a features manager by reading it from an input stream */
        Features_manager(std::istream & in) : Readable(), Writeable(),
                 edges(0), edge_segments(0), manhattan_world(0),
                _edges_available(false),_edge_segments_available(false),
                _manhattan_world_available(false)
        {
            read(in);
        }

        /** @brief Constructs a features manager by reading it from a file */
        Features_manager(const char * filename) : Readable(), Writeable(),
                 edges(0), edge_segments(0), manhattan_world(0),
                _edges_available(false),_edge_segments_available(false),
                _manhattan_world_available(false)
        {
            Readable::read(filename);
        }

        ~Features_manager()
        {
            delete manhattan_world;
            delete edge_segments;
            delete edges;
        }

        /** @brief Sets the parameters needed for edge detection */
        void set_edge_detection_parameters
        (
            float iblurring_sigma,
            float ibegin_threshold,
            float iend_threshold,
            unsigned int ipadding,
            bool iuse_fourier
        );

        /** @brief Sets the parameters needed to detect Manhattan world features
         *         (vanishing points for three orthogonal directions)
         */
        void set_manhattan_world_parameters
        (
            double ivanishing_point_detection_success_probability,
            double ioutlier_threshold_for_vanishing_points_assignment
        );

        /** @brief Reads this Features_manager from an input stream. */
        void read(std::istream& in);

        void read(const char* fname)
        {
            Readable::read(fname);
        }

        /** @brief Writes this Features_manager to an output stream. */
        void write(std::ostream& out) const;

        void write(const char* fname) const
        {
            Writeable::write(fname);
        }

        /** @brief Returns a pointer to the edge set if available */
        inline const kjb::Edge_set & get_edges()
        {
            if(!_edges_available)
            {
                KJB_THROW_2(KJB_error,"Edges are not available!");
            }
            return *edges;
        }

        /** @brief Returns a pointer to the edge segments set if available */
        inline const Edge_segment_set & get_edge_segments()
        {
            if(!_edge_segments_available)
            {
                KJB_THROW_2(KJB_error,"Edge segments are not available!");
            }
            return *edge_segments;
        }

        /** @brief Returns a pointer to Manhattan world if available */
        inline Manhattan_world & get_manhattan_world() const
        {
            if(!_manhattan_world_available)
            {
                KJB_THROW_2(KJB_error,"Manhattan world is not available!");
            }
            return *manhattan_world;
        }

        /** @brief Returns true if edges are available*/
        inline bool edges_available()
        {
            return _edges_available;
        }

        /** @brief Returns true if edge segments are available*/
        inline bool edge_segments_available()
        {
            return _edge_segments_available;
        }

        /** @brief Returns true if Manhattan world is available*/
        inline bool manhattan_world_available()
        {
            return _manhattan_world_available;
        }

        void set_manhattan_focal_length(double ifocal);

        void set_manhattan_world(Manhattan_world * mw);

        double get_outlier_threshold_for_vanishing_points_assignment()
        {
            return outlier_threshold_for_vanishing_points_assignment;
        }

        void remove_frame_segments();

        void reset_manhattan_world_vpts(const std::vector<Vanishing_point> & vpts, double focal)
        {
            manhattan_world->reset_vanishing_points(vpts);
            manhattan_world->set_focal_length(focal);
            manhattan_world->assign_segments_to_vpts(*edge_segments);
            manhattan_world->create_corners();
        }

    private:

        /** Copy constructor is private so that we cannot copy this class */
        Features_manager(const Features_manager & src) :
                    Readable(), Writeable() { (*this) = src; }

        /** Assignment operator is private so that we cannot assign this class */
        Features_manager & operator=(const Features_manager & /* src */)
        {
            return (*this);
        }

        /** @brief Detects the requested features from an image */
        bool detect_features
        (
            bool idetect_edges,
            bool ifit_edge_segments,
            bool icreate_manhattan_world,
            const kjb::Image & img
        );

        /** @brief Detects the requested features from an image */
        bool detect_features
        (
            bool idetect_edges,
            bool ifit_edge_segments,
            bool icreate_manhattan_world,
            const std::string & img_path
        );

        /** @brief detect edges on an image using the Canny algorithm */
        void detect_edges(const kjb::Image & img);

        /** @brief fit line segments to the image edges */
        void fit_edge_segments_to_edges();

        /** @brief Detects Manhattan world features
         *  (vanishing points for three orthogonal directions) */
        bool create_manhattan_world(const kjb::Image & img);

        /** @brief Detects Manhattan world features
         *  (vanishing points for three orthogonal directions) */
        bool create_manhattan_world(const std::string & img_path);

        /** Features */

        /** @brief image edges */
        kjb::Edge_set         * edges;

        /** @brief line segments fit to image edges */
        Edge_segment_set * edge_segments;

        /** @brief manhattan world features */
        Manhattan_world * manhattan_world;

        /** True if edges are available */
        bool _edges_available;

        /** True if edge segments are available */
        bool _edge_segments_available;

        /** True if Manhattan world features are available */
        bool _manhattan_world_available;



        /** Options for features extraction */

        /** Edge detection */

         /** @brief  Gaussian blurring sigma.  Determines scale of edges to detect.
          *          Also useful to remove noise*/
        float blurring_sigma;

        /** @brief  Starting edge threshold hysteresis in the Canny edge detection algorithm.
          * Lower value gives more edges. */
        float begin_threshold;

        /** @brief  Ending edge threshold for hysteresis in the Canny edge detection algorithm
          * Lower value gives longer edges. */
        float end_threshold;

        /** @brief  Amount of padding to add to images before detecting edges.
          *          Images are padded by repeating the values occurring at image boundaries.
          *          Adding padding can prevent edges detected at image boundary,
          *          but often this is set to zero (default). */
        unsigned int padding;

        /** @brief Specifies whether to use Fast Fourier transform for convolution or not */
        bool use_fourier;

        /** Manhattan World */

        /** @brief The probability that Ransac will successfully detect the
        *         vanishing points */
        double vanishing_point_detection_success_probability;

        /** @brief  ioutlier_threshold_for_vanishing_points_assignment
         *         the threshold above which a line segment is considered an outlier.
         *         We try to assign each segment to a vanishing point, and if none
         *         of them gives a penalty smaller than the threshold the segment
         *         will be considered as an outlier
         */
        double outlier_threshold_for_vanishing_points_assignment;

};

Features_manager * detect_hoiem_features_manager
(
    const std::string & img_path
);

}

#endif
