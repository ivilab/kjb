/* $Id: model_edge.h 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifndef KJB_MODEL_EDGE_H_
#define KJB_MODEL_EDGE_H_

#include <edge_cpp/line_segment.h>

/**
 * @class Model_edge This class contains an edge resulting
 * from rendering a model. Likelihood algorithms in this library
 * will compute a likelihood value of a set of model edges given
 * the edges detected in the image (ie, how well the model edges
 * explain the detected image edges).
 * The model could be anything, but it must be renderable in terms
 * of a set of edges. Each of these edges is represented
 * as a line segment in the image plane, plus an indicator
 * on whether the model edge is on the silhouette of the model.
 * This last field is relevant only if the algorithm to compute
 * the likelihood from a set of edges needs it.
 */
namespace kjb
{
    class Model_edge : public Line_segment{

    public:

        /** @brief Constructs a model edge from the position of its edge points
         *
         * @param x1 The x coordinate of the first end point
         * @param y1 The y coordinate of the first end point
         * @param x2 The x coordinate of the second end point
         * @param y2 The y coordinate of the second end point
         * @param isilhouette indicates whether this is a silhouette edge or not
         */
        Model_edge(double x1, double y1, double x2, double y2, bool isilhouette = false, bool isvisible = false, bool iflagged = false);

        /** @brief Constructs a model edge from the position of its edge points
         *
         * @param istart The position of the first end point of this segment
         * @param iend   The position of the second end point of this segment
         * @param isilhouette indicates whether this is a silhouette edge or not
         */
        Model_edge(const kjb::Vector & istart, const kjb::Vector & iend, bool isilhouette = false, bool isvisible = false, bool iflagged = false)
        {
            if(istart.size() < 2 || iend.size() < 2)
            {
                KJB_THROW_2(Illegal_argument, "Bad start and end points, expectin x and y coordinates");
            }

            _degenerate = false;
            if( (istart(0) == iend(0)) && (istart(1) == iend(1)) )
            {
                /** In case this is a degnerate segment, we add a little perturbation
                 * to one coordinate so that line_segment will not cause any exception.
                 * This is sort of a hack, but this is such a rare case that we can
                 * afford it
                 */
                init_from_end_points(istart(0), istart(1), iend(0), iend(1) + FLT_EPSILON);
                _degenerate = true;
            }
            else
            {
                init_from_end_points(istart(0), istart(1), iend(0), iend(1));
            }
            _silhouette = isilhouette;
            _visible = isvisible;
            _flagged = iflagged;
        }

        /** @brief Copy constructor
         *
         * @param src The model edge to copy into this one
         */
        Model_edge(const Model_edge & src) :
            Line_segment(src), _silhouette(src._silhouette), _degenerate(src._degenerate), _visible(src._visible), _flagged(src._flagged)
        {

        }

        /** @ brief Assignment operator
         *
         * @param src The model edge to assign to this one
         */
        Model_edge & operator=(const Model_edge & src)
        {
            Line_segment::operator=(src);
            _silhouette = src._silhouette;
            _visible = src._visible;
            _degenerate = src._degenerate;
            _flagged = src._flagged;
            return (*this);
        }

        inline bool operator==(const Model_edge& model_edge) const
        {
            return (get_start() == model_edge.get_start() &&
                        get_end() == model_edge.get_end()) ||
                    (get_end() == model_edge.get_start() &&
                        get_start() == model_edge.get_end());
        }

        /** @brief Reads this Model segment from an input stream. */
        void read(std::istream& in);

        /** @brief Writes this Model segment to an output stream. */
        void write(std::ostream& out) const;

        /** @brief Returns true if this model edge is a silhouette edge */
        inline bool is_silhouette() const { return _silhouette; }

        /** @brief Returns true if this model edge is an visible edge */
        inline bool is_visible() const { return _visible; }

        /** @brief Sets whether this model edge is a silhouette edge or not */
        inline void set_silhouette(bool isilhouette) { _silhouette = isilhouette; }

        /** @brief Sets whether this model edge is flagged or not */
        inline void set_flagged(bool iflagged) { _flagged = iflagged; }

        /** @brief Sets whether this model edge is a visible edge or not */
        inline void set_visible(bool isvisible) { _visible = isvisible; }

        /** @brief Update the detectd parts when projecting an image segment onto the 
         *   model edge
         *  @return the overlapped length between teh image_edge_segment and the 
         *  already detected parts 
         */
        void update_detected_segments
        (
            const Edge_segment& image_edge_segment, 
            double              collinear_distance_threshold,
            Vector&             previous_center,
            bool&               mapped,
            double              noisy_length
        );

        /** @brief Return the detected length of this model segment */
        double get_detected_length(); 

        bool is_flagged() const
        {
            return _flagged;
        }
    
    private:

        /** @brief Tells whether this is a silhouette edge or not */
        bool _silhouette;

        /** @brief Tells whether this is a degenerate segment. This happens in the extremely
         * rare case where we project a 3D line through the camera centre, which projects
         * onto a single point in the image plane
         */
        bool _degenerate;
        
        /** @brief Tells whether this is a visible edge or not */
        bool _visible;

        bool _flagged;
       
        /** @brief The detected line segments along this model edge */
        std::vector<Line_segment> detected_segments;
    };
}

#endif /* KJB_MODEL_EDGE_H_ */
