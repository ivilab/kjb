/* $Id: edge_likelihood_util.h 14948 2013-07-18 16:00:33Z delpero $ */
/**
 * This work is licensed under a Creative Commons
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 *
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 *
 * You are free:
 *
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 *
 * Under the following conditions:
 *
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 *
 *    Noncommercial. You may not use this work for commercial purposes.
 *
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 *
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 *
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 *
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */

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
   |  Author: Luca Del Pero
 * =========================================================================== */

#ifndef KJB_EDGE_LIKELIHOOD_UTIL_H_
#define KJB_EDGE_LIKELIHOOD_UTIL_H_

#include <edge_cpp/line_segment.h>
#include <edge_cpp/edge.h>
#include <edge_cpp/line_segment_set.h>
#include <likelihood_cpp/model_edge.h>
#include <gr_cpp/gr_polymesh_renderer.h>

/**
 * @file edge_likelihood_util.h
 */
namespace kjb {

    /**
     * @brief   Prepares a model map from a polymesh.
     *
     * Each edge of the polymesh is rendered using its
     * id as a color. The id is obtained by assigning id = 1
     * to the first edge, and sequentially increasing ids to
     * the following edges
     */
    void prepare_model_map(Int_matrix & model_map, const Polymesh & p);

    /**
     * @brief   Prepares a set of model edges from a polymesh.
     *
     *  The extrema of each edge are projected onto the image plane using the current gl projection
     *  and modelview matrices. Each projected edge is stored as a line segment
     *  having as extrema the projections of the extrema of the 3D polymesh edge.
     *  We also check for each edge whether it is part of the silhoutte of
     *  the polymesh or if it is an inner edge (we need the camera to do this
     *  test).
     */
    void prepare_model_edges(std::vector<Model_edge> & edges, const Polymesh & p, const Base_gl_interface & eye);

    /**
     * @brief   Prepares a model map from a vector of polymeshes.
     *
     * Each edge of the polymesh is rendered using its id as a color. The
     * id is obtained by assigning id = 1 to the first edge, and sequentially
     * increasing ids to the following edges.
     */
    void prepare_model_map(Int_matrix & model_map, const std::vector<const Polymesh *> & ps);

    /**
     * @brief   Prepares a set of model edges from a vector of polymeshes.
     *
     * The extrema of each edge are projected onto the image plane using the
     * current gl projection and modelview matrices. Each projected edge is stored
     * as a line segment having as extrema the projections of the extrema of the
     * 3D polymesh edge. We also check for each edge whether it is part of the silhoutte of
     * the polymesh or if it is an inner edge (we need the camera to do this
     * test).
     */
    void prepare_model_edges(std::vector<Model_edge> & edges, const std::vector<const Polymesh *> & ps, const Base_gl_interface & eye);

    void prepare_model_edges
    (
        std::vector<Model_edge> & edges,
        const std::vector<const Polymesh *> & ps,
        const Base_gl_interface & eye,
        const Matrix & M,
        double width,
        double height,
        const std::vector<bool> & flagged
    );

    void prepare_model_edges
    (
        std::vector<Model_edge> & edges,
        const std::vector<const Polymesh *> & ps,
        const Base_gl_interface & eye,
        const Matrix & M,
        double width,
        double height
    );

    void prepare_model_edges
    (
        std::vector<Model_edge> & edges,
        const Polymesh & p,
        const Base_gl_interface & eye,
        const Matrix & M,
        double width,
        double height,
        bool flagged
    );

    void prepare_model_edges
    (
        std::vector<Model_edge> & edges,
        const Polymesh & p,
        const Base_gl_interface & eye,
        const Matrix & M,
        double width,
        double height
    );

    /**
     * @brief   Prepares a model map from a polymesh.
     *
     * Each face of the polymesh is rendered using its
     * id as a color. The id is obtained by assigning id = 1
     * to the first face, and sequentially increasing ids to
     * the following faces.
     */
    void prepare_solid_model_map(Int_matrix & model_map, const std::vector<const Polymesh *> & ps);

    /**
     * @brief   Prepares a model map from a vector of polymeshes.
     *
     * Each face of the polymesh is rendered using its
     * id as a color. The id is obtained by assigning id = 1
     * to the first face, and sequentially increasing ids to
     * the following faces.
     */
    void prepare_solid_model_map(Int_matrix & model_map, const Polymesh & p);

    void draw_model_edges(kjb::Image & img, const std::vector<Model_edge> & edges);

    /**
     * @brief Prepares a set of rendered model edges from the model_map.
     */
    void prepare_rendered_model_edges(std::vector<Model_edge> & edges, const Int_matrix & model_map);

    // Used by the prepare_renderend_model_edges() method to sort the points of the model edge
    struct compare_point_x_location
    {
        public:
            bool operator() (const Vector& p1, const Vector& p2)
            {
                return p1[0] < p2[0];
            }
    };
}

#endif /* KJB_EDGE_LIKELIHOOD_UTIL_H_ */

