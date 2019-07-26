/* $Id: edge_points_likelihood.h 18283 2014-11-25 05:05:59Z ksimek $ */
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

#ifndef KJB_EDGE_POINTS_LIKELIHOOD_H_
#define KJB_EDGE_POINTS_LIKELIHOOD_H_

#include <list>
#include <vector>
#include <edge_cpp/line_segment.h>
#include <edge_cpp/edge.h>
#include <edge_cpp/line_segment_set.h>
#include <likelihood_cpp/model_edge.h>
#include <likelihood_cpp/edge_likelihood_util.h>

/**
 * @file edge_points_likelihood.h This file contains functions
 * used to compute the likelihood of a set of deteceted edges
 * given a set of backprojected model edges (ie, the backprojection
 * of the model gives a high likelihood if it explains the detected
 * edges well). We treat each edge point independently here.
 * The likelihood value is computed in the following way:
 * 1) We first need to find a correspondence between model edge
 *    points and image edge points. We match each model point to
 *    the closest edge point along the direction of the edge gradient
 *    (there is no match is the closest match is too far away).
 *    It follows that each edge point can be explained by several model
 *    point (ie edge point e1 could be the closest edge point to model
 *    point m1 and also the closest edge point to model point m2).
 * 2) For each edge point we compute a penalty. This is a constant
 *    value if the edge point is not explained by any model point (we
 *    explain these points as noise). Otherwise, if an edge point matches
 *    one and only one model point, the penalty is proportional (with a gaussian
 *    falloff) to their distance along the edge gradient, and to the difference
 *    in orientation between model edge and detected edge. If an edge point
 *    is matched to multiple model points, the penalty is the weighted average
 *    of the penalty for each possible match. Last, we penalize model points
 *    that are not matched to any edge point (we explain this as edge points
 *    missed by the edge detector)
 *
 * For more details please see Figure 5 and 6 in:
 * http://vision.cs.arizona.edu/~schlecht/research/furniture/papers/schlecht-2009b.pdf
 */
namespace kjb {

/**
 * Contains pointers to data edge point per cell.
 */
class Correspondence
{
public:

    /**
     * @brief Creates a new correspondence between edge points and
     * model points. For efficiency reasons, given a set of detected edge
     * points in an image, we pre compute the penalties for each possible
     * assignment of each edge point to any possible model point.
     * We do it in the following way:
     * For each edge point, follow the direction of the edge gradient computed
     * at that point (do it until you don't get too far from the edge point).
     * Each of the points in this direction is a potential match if a model
     * point happens to be projected in that position. For each potential match
     * we compute the distance penalty and the orientation penalty. We do it
     * for all possible orientation of the model edge (the angle space is
     * discretized into n bins). The distance penalty is also a function
     * of the difference in orientation: we compute the distance between the
     * model point and the generative approximation of the edge point, such that
     * this latter lies both in the direction perpendicular to the edge gradient
     * and in the direction of the model gradient).
     * For more details please see Figure 5 and 6 in:
     * http://vision.cs.arizona.edu/~schlecht/research/furniture/papers/schlecht-2009b.pdf
     * All of the above is done by the constructor.
     *
     * Once this is done, given a set of model points we can compute the correspondence
     * easily. This is done by function generate_for_model_from_map_and_edges
     */
    Correspondence
    (
        const kjb::Edge_set * data_edges,
        unsigned int          num_rows,
        unsigned int          num_cols,
        unsigned int          num_angles,
        double                angle_sigma,
        double                dist_sigma,
        double                max_dist
    );

    /** @brief Copy constructor*/
    Correspondence(const Correspondence & c);

    /** @brief Assignment operator*/
    Correspondence & operator=(const Correspondence & c);

    /**
     * Correspondence point between a model point and edge point. For efficiency
     * reasons, we store the precomputed distance penalties for
     * distance and difference in orientation for any possible
     * model edge orientation (the angle space is discretized in n bins).
     * Once the model point is known, this will also store the actual penalty
     * for that particular model point.
     */
    class Point
    {
    public:

        /** @brief Constructor*/
        Point
        (
            const kjb::Edge_point    & e_pt,
            unsigned int             iedge_pt_num,
            unsigned int             imodel_pt_row,
            unsigned int             imodel_pt_col,
            unsigned int             num_angles,
            double                   angle_sigma,
            double                   dist_sigma
        );

        /** @brief Copy constructor*/
        Point(const Point & p);

        /** @brief Assignment operator*/
        Point & operator=(const Point & p);

        /** Compares two Correspondence
         * Points based on the normal distance between model point
         * and the generative approximation of the edge point
         */
        class Compare_Normal_Distance
        {
        public:
            bool operator() (const Point* p1, const Point* p2) const;
        };


        /** @brief Compares two Correspondence Points based on the
         * normal distance between model point and the edge point
         * along the edge gradient
         */
        bool operator< (const Point& p) const;

        /** @brief Edge point detected in the input image. */
        unsigned int edge_pt_row;

        unsigned int edge_pt_col;

        double edge_pt_drow;

        double edge_pt_dcol;

        /**
         * @brief Index number of the edge point detected,
         * starting with 0.
         */
        unsigned int edge_pt_num;

        /**
         * @brief Row number for the model point corresponding to
         * the edge point.
         */
        unsigned int model_pt_row;

        /**
         * @brief Column number for the model point corresponding
         * to the edge point.
         */
        unsigned int model_pt_col;

        unsigned int model_edge_index;

        /**
         * @brief Size of a bin in the discretized angle space
         */
        double dtheta;

        /**
         * @brief Discretized angles between model point
         * normals and an edge point's gradient.
         *
         * Because of symmetry, ranges 0 to PI/2.
         */
        std::vector<double> angles;

        /**
         * @brief Angle between model point normal and an edge
         * point's gradient, once the correspondence is known
         *
         * Because of symmetry, ranges 0 to PI/2.
         */
        double angle;

        /**
         * @brief Gaussian likelihood of the discretized angles
         * angle. This is proportional to the difference in orienation
         * between model edge and detected edge. We compute it for
         * any bin of the discretized angle space
         */
        std::vector<double> gauss_angles;

        /** @brief Gaussian likelihood of the angle, once the correspondence
         *  is known
         */
        double gauss_angle;

        /**
         * @brief Euclidean distance from the model point to the
         * generative approximation of the edge point along, that lies
         * both in the direction perpendicular to the gradient of the
         * detected edge and in the direction perpendicular to the model
         * edge. This changes with the model edge orientation, and we need
         * to compute it for every angle bin
         */
        std::vector<double> norm_dists;

        /**
         * @brief Euclidean distance from the model point to the
         * generative approximation of the edge point along, that lies
         * both in the direction perpendicular to the gradient of the
         * detected edge and in the direction perpendicular to the model
         * edge. Computed once the correspondence is known
         */
        double norm_dist;

        /**
         * @brief Gaussian likelihood of the Euclidean normal
         * distance.
         */
        std::vector<double> gauss_norm_dists;

        /**
         * @brief Gaussian likelihood of the Euclidean normal
         * distance (once the correspondence is known)
         */
        double gauss_norm_dist;

        /**
         * @brief Distance between the model point and the edge point
         * along the direction of the edge gradient. This does not
         * depend on the model edge orientation
         */
        double grad_dist;
    };

    class Matched_point
    {
    public:

        Matched_point( double iangle, double igauss_angle, double inorm_dist, double igauss_norm_dist, double igrad_dist, int imodel_index)
                : angle(iangle), gauss_angle(igauss_angle), norm_dist(inorm_dist), gauss_norm_dist(igauss_norm_dist),
                  grad_dist(igrad_dist), model_edge_index(imodel_index)
        {

        }

        Matched_point()
        {
            angle = 0.0;
            gauss_angle = 0.0;
            norm_dist = 0.0;
            gauss_norm_dist = 0.0;
            grad_dist = 0.0;
            model_edge_index = 0;
        }

        Matched_point(const Matched_point & mp)
        {
            this->operator=(mp);
        }

           /** @brief Assignment operator*/
        Matched_point & operator=(const Matched_point & src)
        {
            angle = src.angle;
            gauss_angle = src.gauss_angle;
            norm_dist = src.norm_dist;
            gauss_norm_dist = src.gauss_norm_dist;
            grad_dist = src.grad_dist;
            model_edge_index = src.model_edge_index;

            return (*this);
        }

        class Compare_Normal_Distance
        {
        public:
            bool operator() (const Matched_point* mp1, const Matched_point * mp2) const;
        };


        /** @brief Compares two Correspondence Points based on the
         * normal distance between model point and the edge point
         * along the edge gradient
         */
        bool operator< (const Matched_point& mp) const;

        double angle;

        double gauss_angle;

        double norm_dist;

        double gauss_norm_dist;

        double grad_dist;

        int model_edge_index;
    };


    /**
     * @brief Generates a correspondence set between the detected edge point
     * and the given model points. model_map contains the model rendered onto
     * the image plane. Each model edge is expected to be rendered with a
     * color value equal to its id (that is the position of the edge
     * in the input line_segment_set)
     */
    const std::vector< std::vector<const Correspondence::Point *> > &
    generate_for_model_from_map_and_edges
    (
        const Int_matrix & model_map,
        const std::vector<Model_edge> & model_edges,
        const std::vector<kjb::Int_vector> & edge_indexes,
        int edge_counter,
        int & num_sil_miss,
        int & num_inn_miss
    );

    void generate_for_model_from_map_and_edges
    (
        const Int_matrix & model_map,
        const std::vector<Model_edge> & model_edges,
        const std::vector<kjb::Int_vector> & edge_indexes,
        std::vector<Correspondence::Matched_point> & matched_points,
        std::vector< std::vector<const Correspondence::Matched_point *> > & edge_pts_matched_corr,
        int edge_counter,
        int & num_sil_miss,
        int & num_inn_miss,
        int & num_flagged_miss,
        int & matched_counter,
        double inn_prob,
        double sil_prob
    );

    const std::vector< std::vector<const Correspondence::Point*> > &
    generate_for_model
    (
        const kjb::Edge_set& model_edge_set
    );

    /** @brief Returns the max distance allowed between a model point
     * and edge point for them to be considered matched
     */
    double get_max_dist() const { return max_dist; };

    /** @brief Gets the correspondences associated to model points */
    const std::vector< std::vector< std::list<Point> > > & get_model_corr() const {return model_pts_corr;};

private:

    /**
     * @brief For the input edge point, we follow the direction of the
     * gradient computed in the edge point (we use Bresenham's line
     * drawing algorithm, stopping when we get to far from the edge
     * point). Each point on this line represents a possible match
     * in case a model point is backprojected in that position.
     * For each of these possible matches we pre-compute the penalties
     * for each possible orientation of the model edge
     */
    void add_edge_pts_along_line
    (
        const kjb::Edge_point & iedge_pt,
        unsigned int            iedge_pt_num,
        unsigned int            num_rows,
        unsigned int            num_cols,
        unsigned int            num_angles,
        double                  angle_sigma,
        double                  dist_sigma,
        double                  max_dist
    );

    /**
     * The two vectors together can be seen as a matrix.
     * (model_pts_corr[i])[j] represents a possible location
     * of a model point at pixel (i,j) in the image plane.
     * For each of these locations, we have a list of all the
     * edge points that could have been generated by that
     * model point. This list is sorted according to the distance
     * between model point and edge point
     */
    std::vector< std::vector< std::list<Point> > > model_pts_corr;

    /**
     * This vector is indexed by edge point number. For each edge point
     * we have a vector containing the model points that could have
     * generated it, sorted according to the distance between model point and
     * edge point
     */
    std::vector< std::vector<const Point*> > edge_pts_corr;

    /** The max distance allowed between a model point
     * and edge point for them to be considered matched */
    double max_dist;

};

class Independent_edge_points_likelihood
{

public:
    enum Likelihood_mode {
        FROM_MAP_AND_EDGES = 0,
        FROM_EDGE_POINTS
    };

    Independent_edge_points_likelihood(
        const kjb::Edge_set * data_edges,
        int num_angles,
        unsigned int num_rows,
        unsigned int num_cols,
        double angle_sigma,
        double dist_sigma,
        double max_dist,
        double bg_prob,
        double noise_prob,
        double silho_miss_prob,
        double inner_miss_prob,
        bool idelete_correspondence = true
    );

     /** @brief Copy constructor: Notice that this is not efficient. You probably will not need to copy this
      *         class very often, however you will experience that copying this class is still faster that creating
      *         it twice */
    Independent_edge_points_likelihood(const Independent_edge_points_likelihood& src);

    ~Independent_edge_points_likelihood()
    {
        if(delete_correspondence)
        {
            delete correspondence;
        }
    }

   /**
    * @brief    Calculates the log likelihood of the image edge set (set
    *           in the constructor) given the model edges.
    */
   double operator()(const Edge_set& model_edge_set) const;

    /** @brief Calculates the log likelihood of the image edge set
     *  given the rendering of the model onto the image plane (model_map)
     *  and the set of the model edges. We assumed that each edge was
     *  rendered onto the image plane with a color equal to its id
     *  (ie its position in the line_segment_set) */
    double old_operator
    (
        Int_matrix & model_map,
        const std::vector<Model_edge> & model_edges,
        const std::vector<kjb::Int_vector> & external_edge_indexes,
        int external_edge_counter
    ) const;

    double new_operator
    (
        Int_matrix & model_map,
        const std::vector<Model_edge> & model_edges,
        const std::vector<kjb::Int_vector> & external_edge_indexes,
        int external_edge_counter
    ) const;
    
    double operator()
    (
        Int_matrix & model_map,
        const std::vector<Model_edge> & model_edges,
        const std::vector<kjb::Int_vector> & external_edge_indexes,
        int external_edge_counter
    ) const;

    double operator()
    (
        Int_matrix & model_map,
        const std::vector<Model_edge> & model_edges
    ) const;

    /** @brief Returns the assignment matrix */
    //const kjb::Int_matrix & get_assign() const { return m_ll_assign; };

    /** @brief Returns the correspondence manager */
    Correspondence * get_correspondence() { return correspondence; }

    /** @brief Template function to compute the likelihood for a generic model.
     *  The input model must be able to generate a model map and a set
     *  of model edges, or an edgeset (those are the two kind of features
     *  we can compute a likelihood for so far). Most of the time, you
     *  will likely write a specialization for this function, this
     *  is mostly a guideline
     */
    template <class T>
    double compute_likelihood(T & model, unsigned int mode = FROM_MAP_AND_EDGES);

    /** @brief Template function to compute the likelihood for a generic model under
     *  a given camera.
     *  The input model, with the camera, must be able to generate a model map and a set
     *  of model edges, or an edgeset (those are the two kind of features
     *  we can compute a likelihood for so far). Most of the time, you
     *  will likely write a specialization for this function, this
     *  is mostly a guideline
     */
    template <class T, class U>
    double compute_likelihood(T & model, U & camera, unsigned int mode = FROM_MAP_AND_EDGES);

    /** @brief Template function to compute the likelihood for a generic model under
     *  a given camera.
     *  The input model, with the camera, must be able to generate a model map and a set
     *  of model edges, or an edgeset (those are the two kind of features
     *  we can compute a likelihood for so far). Most of the time, you
     *  will likely write a specialization for this function, this
     *  is mostly a guideline. Here we expect to receive the model_map as an input too
     */
    template <class T, class U>
    double compute_likelihood(T & model, U & camera, Int_matrix & model_map, unsigned int mode = FROM_MAP_AND_EDGES);

    /** @brief Template function to compute the likelihood for a generic model under
     *  a given camera.
     *  The input model, with the camera, must be able to generate a model map and a set
     *  of model edges, or an edgeset (those are the two kind of features
     *  we can compute a likelihood for so far). Most of the time, you
     *  will likely write a specialization for this function, this
     *  is mostly a guideline. Here we expect to receive the model_map as an input too
     */
    template <class T, class U, class V>
    double compute_likelihood(T & model, U & camera, Int_matrix & model_map, V * auxiliary_structure, unsigned int mode = FROM_MAP_AND_EDGES);

    template <class T, class U, class V>
    double compute_likelihood
    (
        T & model,
        U & camera,
        Int_matrix & model_map,
        V * auxiliary_structure,
        const std::vector<kjb::Int_vector> & external_edge_indexes,
        int external_edge_counter,
        int single_object = -1,
        unsigned int mode = FROM_MAP_AND_EDGES
    );

    /** For debug purposes */
    void draw_edge_set_using_mask(Image& img);

    /** Finds the maximum value of the likelihood */
    double find_log_max(double dist_sigma, double angle_sigma);

    inline double get_num_points_ratio()
    {
        return num_points_ratio;
    }

    inline double get_total_components_ratio()
    {
        /*std::cout << "Num data points:" << _num_data_point << std::endl;
        std::cout << "Num miss: " << _num_miss << std::endl;*/
        return (1.0/(_num_miss + _num_data_point));
    }

    const kjb::Edge_set * get_data_edges() const
    {
        return data_edges;
    }

    bool get_delete_correspondence() const
    {
        return delete_correspondence;
    }

    void set_delete_correspondence(bool idelete_correspondence)
    {
        delete_correspondence = idelete_correspondence;
    }


private:

    /** For debug purposes */
    void reset_edge_mask() const;

    /** @brief Copy constructor: This is private and not meant to be used */
   Independent_edge_points_likelihood& operator=(const Independent_edge_points_likelihood&)
   {
       return (*this);
   }

    /** @brief The assignment matrix. It is as big as the input image,
     * and each element can have 4 possible values:
     * m_ll_assign(i,j) = 0 No model edge point was backprojected at pixel (i,j),
     *                      and no edge point was detected at pixel (i,j) in the image.
     *                      We call this background
     * m_ll_assign(i,j) = 1 A model edge point was backprojected at location (i,j)
     *                      and it is explained by at least a detected edge point,
     *                      to which it was matched. We call this match
     * m_ll_assign(i,j) = 2 No model edge point was backprojected at location (i,j),
     *                      but an edge point was detected from the image as this location.
     *                      We call this noise
     * m_ll_assign(i,j) = 3 A model edge point was backprojected at location (i,j).
     *                      but no edge point was detected from the image in the neighborhood of
     *                      this point. Further the model edge is part of the silhouette of
     *                      an object (sharp edge). We call this missed_silhouette
     * m_ll_assign(i,j) = 4 A model edge point was backprojected at location (i,j).
     *                      but no edge point was detected from the image in the neighborhood of
     *                      this point. Further the model edge is an inner edge of an
     *                      an object (not part of the silhouette).
     *                      We call this missed_inner_edge
     */
    //mutable kjb::Int_matrix m_ll_assign;

    /** @brief This contains the set of edges we want to compare to the detected image
     *          edges. We expect each edge point in this map to be rendered with a color
     *          that enables us to retrieve the edge information from a vector (ie
     *          pixel color = id of the edge the pixel comes from). This id must
     *          be sequential (1 = first edge, 2 = second edge and so on).
     *          This is stored here, for efficiency region, so that we don't
     *          have to reallocate the matrix every time we need to compute the
     *          likelihood
     */
    kjb::Int_matrix internal_model_map;

    /** The edges detected from the input image. The likelihood is computed
     *  by comparing a new set of edges against this one. This is not
     *  owned by this class (memory allocation not handled)*/
    const kjb::Edge_set * data_edges;

    /** @brief Used to precompute correspondences between the set of data_edges
     * and any possible set of new edges. See description above*/
    mutable Correspondence * correspondence;

    /** @brief The probability that a point in an image is background (ie not part
     * of an edge). */
    double background_prob;

    /** @brief The probability of detecting noise in a pixel of the image (ie the
     *  probability of detecting an edge point where there should not be any) */
    double noise_prob;

    /** @brief The probability of not having any image detected edge explaining
     * a particular point of the model backprojected onto the image plane.
     * The model edge is part of the silhouette of an object, so it should be
     * pretty sharp, and the probability of such misdetection should be relatively
     * low */
    double silhouette_miss_prob;

    /** @brief The probability of not having any image detected edge explaining
    * a particular point of the model backprojected onto the image plane.
    * The model edge is part of an inner edge of an object (no silhouette), so it is
    * likely to be a weak edge, and the probability of such misdetection should be higher
    * than that for a silhouette edge */
    double inner_edge_miss_prob;

    /** (1.0/num_data_edge_points) */
    double num_points_ratio;

    mutable double _num_miss;

    double _num_data_point;

    /** This is only for debug purposes */
    mutable std::vector<std::vector<bool> > edge_mask;

    mutable int edge_counter;

    kjb::Matrix M;

    int image_size;

    mutable std::vector<Correspondence::Matched_point> matched_points;

    mutable std::vector< std::vector<const Correspondence::Matched_point *> > matched_pts;

    mutable int used_matched_points;

    bool delete_correspondence;
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
double Independent_edge_points_likelihood::compute_likelihood(T & model, unsigned int mode)
{
    if(mode == FROM_MAP_AND_EDGES)
    {
        std::vector<Model_edge> model_edges;
        model.prepare_model_map(internal_model_map);
        model.prepare_model_edges(model_edges);
        return this->operator()(internal_model_map, model_edges);
    }
    else if(mode == FROM_EDGE_POINTS)
    {
        kjb::Edge_set edge_points;
        model.prepare_edge_points(edge_points);
        return this->operator()(edge_points);
    }
    else
    {
        KJB_THROW_2(Illegal_argument, "Likelihood mode not supported");
    }
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
double Independent_edge_points_likelihood::compute_likelihood(T & model, U & camera, unsigned int mode)
{
    if(mode == FROM_MAP_AND_EDGES)
    {
        std::vector<Model_edge> model_edges;
        model.prepare_model_map(internal_model_map, camera);
        model.prepare_model_edges(model_edges, camera);
        return this->operator()(internal_model_map, model_edges);
    }
    else if(mode == FROM_EDGE_POINTS)
    {
        kjb::Edge_set edge_points;
        model.prepare_edge_points(edge_points, camera);
        return this->operator()(edge_points);
    }
    else
    {
        KJB_THROW_2(Illegal_argument, "Likelihood mode not supported");
    }
}

} //namespace kjb


#endif /* KJB_EDGE_POINTS_LIKELIHOOD_H_ */

