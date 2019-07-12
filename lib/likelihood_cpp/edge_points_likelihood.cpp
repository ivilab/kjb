/* $Id: edge_points_likelihood.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
   |  Author:  Luca Del Pero, Joseph Schlecht, Kyle Simek
 * =========================================================================== */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "likelihood_cpp/edge_points_likelihood.h"
#include "sample/sample_gauss.h"

#include <cmath>
#include <iostream>
#include <boost/unordered_map.hpp>

#define EDGE_ANGLE_THRESHOLD 0.3

/**
 * Compares two Correspondence points.
 * Returns true if the normal distance between model point
 * and the generative approximation of the edge point
 * (field norm_dist) for the first input correspondence
 * point is smaller than that for the second input point.
 * This is used to sort correspondence points in increasing order
 * according to the norm_dist field.
 *
 * @param p1 The first point to compare
 * @param p2 The second point to compare
 *
 */

using namespace kjb;

bool Correspondence::Point::Compare_Normal_Distance::operator()
(
    const Point* p1,
    const Point* p2
)
const
{
    return p1->norm_dist < p2->norm_dist;
}

/** This operator is defined in terms of the Euclidean
 * distance between model point and edge point along the
 * edge gradient direction. This operator is used to sort
 * correspondence points according to this parameter
 *
 * @param p The point to compare this Correspondence point to
 */
bool Correspondence::Point::operator< (const Point& p) const
{
    return grad_dist < p.grad_dist;
}

bool Correspondence::Matched_point::Compare_Normal_Distance::operator()
(
    const Matched_point* mp1,
    const Matched_point* mp2
)
const
{
    return mp1->norm_dist < mp2->norm_dist;
}

/** This operator is defined in terms of the Euclidean
 * distance between model point and edge point along the
 * edge gradient direction. This operator is used to sort
 * correspondence points according to this parameter
 *
 * @param p The point to compare this Correspondence point to
 */
bool Correspondence::Matched_point::operator< (const Matched_point& mp) const
{
    return grad_dist < mp.grad_dist;
}



/**
 * A point correspondence between a model point and a detected
 * edge point. For efficiency, given an edge point we follow
 * from it the direction defined by the edge gradient,
 * and we consider all points along this direction within a certain
 * distance. This class stores the precomputed information for
 * a possible match between one edge point and one model point.
 * For each point correspondence, we precompute the assignment
 * penalty which depends on the difference in orientation between
 * the model edge orientation and the detected edge orientation.
 * We precompute this penalty for each possible orientation
 * (we actually divide the angle space in n bins).
 * The penalty also depends on the distance between the model point
 * and the generative approximation of the edge point given the
 * actual position of the edge point and the position of the model
 * point. This approximation is perpendicular both to the edge gradient
 *  and to the model edge.
 * The penalties for both distance and angle are computed using
 * gaussian distributions with 0 mean and the input variances.
 *
 * This function assumes that the model point lies along the
 * direction perpendicular to the edge gradient at the edge
 * point. (This is how we compute possible correspondences).
 * For more details please see Figure 6 in
 * http://vision.cs.arizona.edu/~schlecht/research/furniture/papers/schlecht-2009b.pdf
 *
 * @param iedge_pt The detected edge point
 * @param iedge_pt_num The number (id) of the detected edge point
 * @param imodel_pt_row The position in the image of the model edge
 *                      point assigned to this edge point (row)
 * @param imodel_pt_col The position in the image of the model edge
 *                      point assigned to this edge point (col)
 *                      This point is assumed to lie in the direction
 *                      perpendicular to the detected edge gradient
 *                      computed at iedge_pt
 * @param num_angles    The number of bins used to partition the
 *                      angle space (from 0 to 90 degrees, due to symmetry)
 * @param angle_sigma   The standard deviation for the Gaussian
 *                      distribution (with zero mean) used to compute
 *                      the angle penalty
 * @param dist_sigma    The standard deviation for the Gaussian
 *                      distribution (with zero mean) used to compute
 *                      the distance penalty
 */

Correspondence::Point::Point
(
    const kjb::Edge_point & iedge_pt,
    unsigned int            iedge_pt_num,
    unsigned int            imodel_pt_row,
    unsigned int            imodel_pt_col,
    unsigned int            num_angles,
    double                  angle_sigma,
    double                  dist_sigma
) :
    angles(num_angles, 0), gauss_angles(num_angles, 1),
    norm_dists(num_angles, 0), gauss_norm_dists(num_angles, 1)
{
    edge_pt_num   = iedge_pt_num;
    model_pt_row   = imodel_pt_row;
    model_pt_col   = imodel_pt_col;
    edge_pt_row = iedge_pt.get_row();
    edge_pt_col = iedge_pt.get_col();
    edge_pt_drow = iedge_pt.get_drow();
    edge_pt_dcol = iedge_pt.get_dcol();
    model_edge_index = 0;

    /**
     * This function expects imodel_pt to lie on the direction
     * defined by the edge gradient computed at iedge_pt.
     * (v_x, v_y) are the components of the vector between the edge
     * point and the model point. Once (v_x, v_y) is normalized,
     * it represents the direction of the line between model point
     * and edge point, along the direction perpendicular to the detected
     * edge
     */
    double v_x = (double)model_pt_col - (double)edge_pt_col;
    double v_y = (double)model_pt_row - (double)edge_pt_row;

    /** This is the distance between edge point and model point
     * computed along the direction defined by the edge gradient
     * computed in edge_pt
     */
    grad_dist = sqrt(v_x*v_x + v_y*v_y);

    /** Normalize (v_x,v_y) */
    if (grad_dist > 1.0e-16)
    {
        v_x /= grad_dist;
        v_y /= grad_dist;
    }

    ASSERT(num_angles > 0);

    /** We subdivide the angle space (only 90 degrees due to symmetry)
     * in n bins, with n=num_angles */
    dtheta = M_PI_2 / num_angles;
    double theta  = 0.5*dtheta;

    /** For each angle bin we precompute the orientation penalty */
    for (unsigned int i = 0; i < angles.size(); i++)
    {
        angles[ i ] = theta;

        double cos_theta = cos(theta);
        double sin_theta = sin(theta);

        /** (dcol, drow) is the vector between edge point and model point.
         *  It is the unnormalized version of (v_x,v_y).
         */
        double dcol = (double)edge_pt_col - (double)model_pt_col;
        double drow = (double)edge_pt_row - (double)model_pt_row;

        /**
         * We compute here the distance between the model point and the
         * generative approximation of the edge point, which is perpendicular
         * both to the edge gradient and to the model edge.
         * (v_x,v_y) is the direction perpendicular to the edge.
         * Since theta is the difference in orientation between edge and model
         * edge, we can get the direction perpendicular to the model edge by
         * rotating (v_x, v_y) by theta, let's call this (v_xtheta, v_ytheta).
         * We can get the length of the projection of (dcol, drow) onto
         * (v_xtheta, v_ytheta) taking their dot product, since the latter
         * is normalized. This value is actually the distance between the model
         * point and the generative approximation of the edge point as explained
         * above. All of this is done by the following line of code
         * For more details see Figure 6 in
         * http://vision.cs.arizona.edu/~schlecht/research/furniture/papers/schlecht-2009b.pdf
         */
        norm_dists[ i ] = fabs(
                (v_x*cos_theta - v_y*sin_theta)*dcol +
                (v_x*sin_theta + v_y*cos_theta)*drow);

        /** Compute the penalties for this particular bin */
        kjb_c::gaussian_pdf(&gauss_angles[ i ], angles[i], 0, angle_sigma);
        kjb_c::gaussian_pdf(&gauss_norm_dists[ i ], norm_dists[i], 0, dist_sigma);

        theta += dtheta;
    }
}

/**
 * Copy constructor
 *
 * @param p the point to copy into this one
 */
Correspondence::Point::Point(const Point & p)
{
    //(*this) = p;
    operator=(p);
}

/**
 * Assignment operator
 *
 * @param p The point to assign this one to
 */
Correspondence::Point & Correspondence::Point::operator=(const Point & p)
{
    if(&p != this)
    {
        edge_pt_row = p.edge_pt_row;
        edge_pt_col = p.edge_pt_col;
        edge_pt_drow = p.edge_pt_drow;
        edge_pt_dcol = p.edge_pt_dcol;
        edge_pt_num = p.edge_pt_num;
        model_pt_row = p.model_pt_row;
        model_pt_col = p.model_pt_col;
        model_edge_index = p.model_edge_index;
        dtheta = p.dtheta;
        angles = p.angles;
        angle = p.angle;
        gauss_angles = p.gauss_angles;
        gauss_angle = p.gauss_angle;
        norm_dists = p.norm_dists;
        norm_dist = p.norm_dist;
        gauss_norm_dists = p.gauss_norm_dists;
        gauss_norm_dist = p.gauss_norm_dist;
        grad_dist = p.grad_dist;
    }

    return (*this);
}

/*
 * Correspondence constructor. For efficiency reasons, we precompute
 * the penalty for any possible assignment between any possible model point
 * and and each edge point in the set of the detected edges.
 * For each edge point, we follow the direction perpendicular to the
 * edge point starting from the edge point (this is the gradient direction
 * computed in the edge point). We follow this direction until we get
 * too far from the edge point. For each possible correspondence
 * we compute the difference penalties for any possible orientation
 * of a model edge (we divide the angle space in n bins).
 *
 * @param data_edges the set of detected edges
 * @param num_rows the height of the image (in pixel)
 * @param num_cols the width of the image (in pixel)
 * @param num_angles the number of bins to be used to subdivide the angle space
 * @param angle_sigma the sigma for the gaussian used to compute the penalty
 *                    associated to the difference in orientation between model
 *                    edge and detected edge
 * @param dist_sigma  the sigma for the gaussian used to compute the penalty
 *                    associated to the distance between the model point and the
 *                    edge point, computed along the edge gradient
 * @param imax_dist   The max distance between an edge point and a model point.
 *                    If their distance is bigger than max_distance, a model point
 *                    cannot be assigned to an edge point
 */
Correspondence::Correspondence
(
    const kjb::Edge_set * data_edges,
    unsigned int          num_rows,
    unsigned int          num_cols,
    unsigned int          num_angles,
    double                angle_sigma,
    double                dist_sigma,
    double                imax_dist
) :
    model_pts_corr(num_rows, std::vector< std::list<Point> >(num_cols)),
    edge_pts_corr(data_edges->get_total_edge_points())
{
    unsigned int i, j;
    unsigned int n = 0;

    max_dist = imax_dist;

    for (i = 0; i < data_edges->num_edges(); i++)
    {
        const kjb::Edge & _edge = data_edges->get_edge(i);
        for (j = 0; j < _edge.get_num_points(); j++)
        {
            /** For each edge point, we draw a line through it along
             * the direction defined by the edge gradient (ie perpendicular
             * to the edge). Each point on this line is a potential match
             * if a model point will happen to be there, For each potential
             * match we compute the distance and difference in orientation
             * penalties (using bins to subdivide the angle space). In this
             * way for any model point these penalties will be already available,
             * and we will not need to compute anymore.
             */
            add_edge_pts_along_line(_edge.get_edge_point(j), n++, num_rows,
                    num_cols, num_angles, angle_sigma, dist_sigma, max_dist);
        }
    }

    /** After running edge_pts_along_line, for each possible position of
     *  a model point in the image, we know a set of edge points it can
     *  be matched to. We sort them here according to the distance between
     *  edge point and model point along the edge gradient (ie the first
     *  edge point in the list in model_pts_corr[i][j] will be the closest
     *  to the potential position (i,j) for a model point).
     *  The sort operator uses the "<" operator defined above
     */
    for (i = 0; i < model_pts_corr.size(); i++)
    {
        for (j = 0; j < model_pts_corr[ i ].size(); j++)
        {
            model_pts_corr[ i ][ j ].sort();
        }
    }

    /** For each edge point, we will need to know to what model point
     * it will be assigned to. This changes according to the set of
     * model points we want to compute the likelihood for. We here reserve
     * some memory so that there will not be any memory allocation when
     * we compute the likelihood
     */
    for (i = 0; i < edge_pts_corr.size(); i++)
    {
        // A good beginning reserve value would be the number of projected
        // model lines on average
        edge_pts_corr[ i ].reserve(15);
    }
}

/**
 * Copy constructor
 *
 * @param c the correspondence to copy into this one
 */
Correspondence::Correspondence(const Correspondence & c)
{
    (*this) = c;
}


/**
 * Assignment operator
 *
 * @param c the correspondence to assign to this one
 */
Correspondence & Correspondence::operator=(const Correspondence & c)
{
    if(&c != this)
    {
        model_pts_corr = c.model_pts_corr;
        max_dist = c.max_dist;
        edge_pts_corr.resize(c.edge_pts_corr.size());

        for(unsigned int i = 0; i < edge_pts_corr.size(); i++)
        {
            edge_pts_corr[ i ].reserve(15);
        }
    }

    return (*this);
}

/**
 * Given an edge point and the gradient direction at that point,
 * this function draws a line along the gradient direction starting
 * from the edge point, going as far as max_dist. In this way we will
 * consider the set of all points along the edge gradient direction
 * and whose distance to edge_pt is smaller than max_dist.
 * Bresenham's line drawing algorithm is used to trace out the path in the matrix
 * along the gradent vector of the edge point.
 */
void Correspondence::add_edge_pts_along_line
(
    const kjb::Edge_point & edge_pt,
    unsigned int            edge_pt_num,
    unsigned int            num_rows,
    unsigned int            num_cols,
    unsigned int            num_angles,
    double                  angle_sigma,
    double                  dist_sigma,
    double                  imax_dist
)
{
    const static float sqrt_2 = 1.414213562373095f;

    unsigned int x_0 = edge_pt.get_col();
    unsigned int y_0 = edge_pt.get_row();

    model_pts_corr[ y_0 ][ x_0 ].push_back(Point(edge_pt, edge_pt_num, y_0,
                x_0, num_angles, angle_sigma, dist_sigma));

    double dx = edge_pt.get_dcol();
    double dy = edge_pt.get_drow();

    /** We use Bresenham algorithm for drawing lines
     * http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
     */
    bool steep = fabs(dy) > fabs(dx);

    if (steep)
    {
        double dt = dy;
        dy = dx;
        dx = dt;

        unsigned int t_0 = y_0;
        y_0 = x_0;
        x_0 = t_0;
    }

    // We assure dx >= 0 (we draw from left to right)
    if (dx < 0)
    {
        dx *= -1;
        dy *= -1;
    }

    // Now we have positive dx (x-step) and negative or positive dy (y-step)

    double error = 0;
    double derr  = fabs(dy) / dx;

    double dist_r = 0;
    double dist_l = 0;

    int row;
    int col;
    int x = 0;
    int y = 0;
    bool done_r = false;
    bool done_l = false;

    /** We draw both on left and right of the edge point (or up and down if it
     * is steep) */
    while (!done_r || !done_l)
    {
        x++;
        error += derr;
        if (error >= 0.5)
        {
            y = (dy > 0) ? y + 1 : y - 1;
            error -= 1.0;

            dist_r += sqrt_2;
            dist_l += sqrt_2;
        }
        else
        {
            dist_r += 1;
            dist_l += 1;
        }

        row = (steep) ? (x_0 + x) : (y_0 + y);
        col = (steep) ? (y_0 + y) : (x_0 + x);
        done_r = row < 0 || row >= (int)num_rows ||
                 col < 0 || col >= (int)num_cols ||
                 dist_r > imax_dist;

        if (!done_r)
        {
            model_pts_corr[ row ][ col ].push_back(Point(edge_pt, edge_pt_num, row,
                        col, num_angles, angle_sigma, dist_sigma));
        }

        row = (steep) ? (x_0 - x) : (y_0 - y);
        col = (steep) ? (y_0 - y) : (x_0 - x);
        done_l = row < 0 || row >= (int)num_rows ||
                 col < 0 || col >= (int)num_cols ||
                 dist_l > imax_dist;

        if (!done_l)
        {
            model_pts_corr[ row ][ col ].push_back(Point(edge_pt, edge_pt_num, row,
                        col, num_angles, angle_sigma, dist_sigma));
        }
    }
}

/**
 * This finds the correspondence between a set of model
 * edges and a set of image edges.
 * The returned vector is indexed by edge point. For each edge point
 * we have a list of all the model points that could be matched to
 * it, sorted according to the distance between edge point and
 * model point.
 *
 * @param model_map the model rendered onto the image plane. It is assumed
 * that each edge was rendered with a color equal to its id
 */
const std::vector< std::vector<const Correspondence::Point *> > &
Correspondence::generate_for_model_from_map_and_edges
(
    const Int_matrix & model_map,
    const std::vector<Model_edge> & model_edges,
    const std::vector<kjb::Int_vector> & edge_indexes,
    int edge_counter,
    int & num_sil_miss,
    int & num_inn_miss
)
{
    using std::vector;
    using std::list;

    bool silhouette = 0;
    num_sil_miss = 0;
    num_inn_miss = 0;

    for (unsigned int i = 0; i < edge_pts_corr.size(); i++)
    {
        /** Let's clear the edge point correspondences.
         * Notice that we used reserve before so the memory
         * still remains allocated
         */
        edge_pts_corr[ i ].clear();
    }

    unsigned int i = 0;
    unsigned int j = 0;
    for(int k = 0; k < edge_counter; k++)
    {
        i = edge_indexes[k](0);
        j = edge_indexes[k](1);
        unsigned int edge_id = edge_indexes[k](2) - 1;

        silhouette = model_edges[edge_id].is_silhouette();
        if((!silhouette) && (edge_indexes[k](3) == 1))
        {
            //The special flag "ignore non silhouette" for this point is on, let's skip it
                continue;
        }
        if (silhouette)
        {
            num_sil_miss++;
        }
        else
        {
            num_inn_miss++;
        }

        if(!model_pts_corr[ i ][ j ].empty())
        {
            /** We just match a model point to the closest edge point
             * After a lot of experimentation, this restriction seems to
             * make sense; our model points can only generate one edge point
             * apiece. The most logical edge point it generated is the
             * closest one (in terms of grad_dist). */
            Point* correspondence_pt = &(model_pts_corr[ i ][ j ].front());

            /** Edge gradient at the edge point */
            double dcol = correspondence_pt->edge_pt_dcol;
            double drow = correspondence_pt->edge_pt_drow;

            /** Change in x and y in the model edge */
            double dx = -model_edges[edge_id].get_dx();
            double dy = model_edges[edge_id].get_dy();

            /** We compute the angle between model edge and detected edge
             * taking the arc cosine of their normalized dot product where
             * we normalize by dividing by the norm
             */
            double dot_product = fabs((dcol*dy+ drow*dx)/sqrt(dx*dx + dy*dy));

            if(dot_product < FLT_EPSILON)
            {
                /** This fixes possible precision problems */
                dot_product = FLT_EPSILON;
            }
            else if (dot_product >= 1.0)
            {
                /** This fixes possible precision problems */
                dot_product = 1.0 - DBL_EPSILON;
            }
            double angle = acos(dot_product);

            /** We now check what bin of the discretized angle space we have to use */
            unsigned int angle_bin = (unsigned int) std::floor(angle / correspondence_pt->dtheta);

            /** We now have found the difference in orientation and the distance
             *  for this correspondence between an edge point and a model point.
             *  We then keep track of these values and of the associated penalties
             *  (everything is pre-computed we just have to fetch the right values)
             */
            correspondence_pt->angle = correspondence_pt->angles.at(angle_bin);
            correspondence_pt->gauss_angle = correspondence_pt->gauss_angles.at(angle_bin);
            correspondence_pt->norm_dist = correspondence_pt->norm_dists.at(angle_bin);
            correspondence_pt->gauss_norm_dist = correspondence_pt->gauss_norm_dists.at(angle_bin);
            correspondence_pt->model_edge_index = edge_id;
            if(angle > EDGE_ANGLE_THRESHOLD )
            {
                correspondence_pt->gauss_angle = 1e-30;
                correspondence_pt->gauss_norm_dist = 1;
            }
            edge_pts_corr.at(correspondence_pt->edge_pt_num).push_back(correspondence_pt);
        }
    }


    /** Each edge point has now a list of associated possible matches
     * with model points. For each edge point, we sort these matches
     * according to the normal distance between the model point and
     * the generative approximation of the edge point, which is
     * perpendicular both to the edge model edge and to the detected
     * edge gradient
     */
    for (unsigned int i = 0; i < edge_pts_corr.size(); i++)
    {
        std::sort(edge_pts_corr[ i ].begin(), edge_pts_corr[ i ].end(),
                Point::Compare_Normal_Distance());
    }

    return edge_pts_corr;
}

/**
 * This finds the correspondence between a set of model
 * edges and a set of image edges.
 * The returned vector is indexed by edge point. For each edge point
 * we have a list of all the model points that could be matched to
 * it, sorted according to the distance between edge point and
 * model point.
 *
 * @param model_map the model rendered onto the image plane. It is assumed
 * that each edge was rendered with a color equal to its id
 */
void Correspondence::generate_for_model_from_map_and_edges
(
    const Int_matrix & model_map,
    const std::vector<Model_edge> & model_edges,
    const std::vector<kjb::Int_vector> & edge_indexes,
    std::vector<Correspondence::Matched_point> & matched_points,
    std::vector< std::vector<const Correspondence::Matched_point *> > & edge_pts_matched_corr,
    int edge_counter,
    int & num_sil_miss,
    int & num_inn_miss,
    int & num_sil_flagged,
    int & matched_counter,
    double inn_prob,
    double sil_prob
)
{
    using std::vector;
    using std::list;

    bool silhouette = 0;
    bool flagged = false;
    num_sil_miss = 0;
    num_inn_miss = 0;
    num_sil_flagged = 0;
    matched_counter = 0;

    for (unsigned int i = 0; i < edge_pts_matched_corr.size(); i++)
    {
        /** Let's clear the edge point correspondences.
         * Notice that we used reserve before so the memory
         * still remains allocated
         */
        edge_pts_matched_corr[ i ].clear();
    }
    unsigned int i = 0;
    unsigned int j = 0;
    for(int k = 0; k < edge_counter; k++)
    {
        i = edge_indexes[k](0);
        j = edge_indexes[k](1);
        unsigned int edge_id = edge_indexes[k](2) - 1;

        silhouette = model_edges[edge_id].is_silhouette();
        flagged = model_edges[edge_id].is_flagged();
        if((!silhouette) && (flagged))
        {
            //The special flag "ignore non silhouette" for this point is on, let's skip it
                continue;
        }
        if (silhouette)
        {
            num_sil_miss++;
        }
        else if(!flagged)
        {
            num_inn_miss++;
        }
        else
        {
            num_sil_flagged++;
        }


        if(!model_pts_corr[ i ][ j ].empty())
        {
            /** We just match a model point to the closest edge point
             * After a lot of experimentation, this restriction seems to
             * make sense; our model points can only generate one edge point
             * apiece. The most logical edge point it generated is the
             * closest one (in terms of grad_dist). */
            Point* correspondence_pt = &(model_pts_corr[ i ][ j ].front());
            Matched_point * matched_pt = &(matched_points[matched_counter]);

            /** Edge gradient at the edge point */
            double dcol = correspondence_pt->edge_pt_dcol;
            double drow = correspondence_pt->edge_pt_drow;

            /** Change in x and y in the model edge */
            double dx = -model_edges[edge_id].get_dx();
            double dy = model_edges[edge_id].get_dy();

            /** We compute the angle between model edge and detected edge
             * taking the arc cosine of their normalized dot product where
             * we normalize by dividing by the norm
             */
            double dot_product = fabs((dcol*dy+ drow*dx)/sqrt(dx*dx + dy*dy));

            if(dot_product < FLT_EPSILON)
            {
                /** This fixes possible precision problems */
                dot_product = FLT_EPSILON;
            }
            else if (dot_product >= 1.0)
            {
                /** This fixes possible precision problems */
                dot_product = 1.0 - DBL_EPSILON;
            }
            double angle = acos(dot_product);

            /** We now check what bin of the discretized angle space we have to use */
            unsigned int angle_bin = (unsigned int) std::floor(angle / correspondence_pt->dtheta);

            /** We now have found the difference in orientation and the distance
             *  for this correspondence between an edge point and a model point.
             *  We then keep track of these values and of the associated penalties
             *  (everything is pre-computed we just have to fetch the right values)
             */
            matched_pt->angle = correspondence_pt->angles.at(angle_bin);
            matched_pt->gauss_angle = correspondence_pt->gauss_angles.at(angle_bin);
            matched_pt->norm_dist = correspondence_pt->norm_dists.at(angle_bin);
            matched_pt->gauss_norm_dist = correspondence_pt->gauss_norm_dists.at(angle_bin);
            matched_pt->grad_dist = correspondence_pt->grad_dist;
            matched_pt->model_edge_index = edge_id;
            if(angle > EDGE_ANGLE_THRESHOLD )
            {
                if(silhouette)
                {
                    matched_pt->gauss_angle = sil_prob;
                } 
                else if(!flagged)
                {
                    matched_pt->gauss_angle = inn_prob;
                }
                else
                {
                    matched_pt->gauss_angle = 1e-10;
                }
                matched_pt->gauss_norm_dist = 1;
            }

            edge_pts_matched_corr.at(correspondence_pt->edge_pt_num).push_back(matched_pt);
            matched_counter++;
        }
    }

    /** Each edge point has now a list of associated possible matches
     * with model points. For each edge point, we sort these matches
     * according to the normal distance between the model point and
     * the generative approximation of the edge point, which is
     * perpendicular both to the edge model edge and to the detected
     * edge gradient
     */
    for (unsigned int i = 0; i < edge_pts_matched_corr.size(); i++)
    {
        std::sort(edge_pts_matched_corr[ i ].begin(), edge_pts_matched_corr[ i ].end(),
                Matched_point::Compare_Normal_Distance());
    }

}

/**
 * The returned structure is indexed by edge point, then correspondence point
 * containing the model point info.
 */
const std::vector< std::vector<const Correspondence::Point*> > &
Correspondence::generate_for_model
(
    const kjb::Edge_set& model_edge_set
)
{
    using std::vector;
    using std::list;

    ASSERT(model_pts_corr.size() == (model_edge_set.num_rows() ));
    ASSERT(model_pts_corr[0].size() == (model_edge_set.num_cols() ));

    for (unsigned int i = 0; i < edge_pts_corr.size(); i++)
    {
        edge_pts_corr[ i ].clear();
    }

    for (unsigned int e = 0; e < model_edge_set.num_edges(); e++)
    {
        for(unsigned int p = 0; p < model_edge_set.get_edge(e).get_num_points(); p++)
        {
            kjb::Edge_point model_pt = model_edge_set.get_edge(e).get_edge_point(p);
            int row = model_pt.get_row();
            int col = model_pt.get_col();

            if( model_pts_corr[ row ][ col ].empty() )
            {
                // There were no edge points whose gradient vector crosses paths
                // with this model point.
                continue;
            }

            // Only use the closest edge point for now. But this is
            // potentially an area to experiment with.
            //
            // After a lot of experimentation, this restriction seems to
            // make sense; our model points can only generate one edge point
            // apiece. The most logical edge point it generated is the
            // closest one (in terms of grad_dist).
            Point* correspondence_pt = &(model_pts_corr[ row ][ col ].front());

            //TODO CHECK THAT Y COORDINATE IS RIGHT -> seems right
            double data_dcol  = correspondence_pt->edge_pt_dcol;
            double data_drow  = correspondence_pt->edge_pt_drow;
            double model_drow = model_pt.get_drow();
            double model_dcol = model_pt.get_dcol();

            //ASSERT(fabs(model_drow * model_drow + model_dcol * model_dcol - 1.0) < 1e-8);
            //ASSERT(fabs(data_drow * data_drow + data_dcol * data_dcol - 1.0) < 1e-8);
            double dot_product = model_drow * data_drow + model_dcol * data_dcol;

            if(dot_product < FLT_EPSILON)
            {
                /** This fixes possible precision problems */
                dot_product = FLT_EPSILON;
            }
            else if (dot_product >= 1.0)
            {
                /** This fixes possible precision problems */
                dot_product = 1.0 - DBL_EPSILON;
            }

            unsigned int angle_bin = (unsigned int)std::floor(acos(dot_product) / correspondence_pt->dtheta);
            ASSERT(angle_bin < correspondence_pt->angles.size());

            correspondence_pt->angle           = correspondence_pt->angles.at(angle_bin);
            correspondence_pt->gauss_angle     = correspondence_pt->gauss_angles.at(angle_bin);
            correspondence_pt->norm_dist       = correspondence_pt->norm_dists.at(angle_bin);
            correspondence_pt->gauss_norm_dist = correspondence_pt->gauss_norm_dists.at(angle_bin);

            edge_pts_corr.at(correspondence_pt->edge_pt_num).push_back(correspondence_pt);
        }
    }

    // Sort each correspondence point vector by distance.
    for (unsigned int i = 0; i < edge_pts_corr.size(); i++)
    {
        std::sort(edge_pts_corr[ i ].begin(), edge_pts_corr[ i ].end(),
                Point::Compare_Normal_Distance());
    }

    return edge_pts_corr;
}


/** Constructs a class for computing the likelihood of a set of image
 * detected edges and a different set of edges, arising from backprojecting
 * a 3D model onto the image plane. Model edge points are matched to
 * image edge points independently, as described in class Correspondence.
 *
 * @param idata_edges The set of edges to compare against, detected from an image
 * @param num_angles  When matching edges, we penalize their difference in orientation.
 *                    The penalty is discretized in num_angles bins
 * @param num_rows    The number of rows of the image
 * @param num_cols    The number of columns of the image
 * @param angle_sigma the sigma for the gaussian used to compute the penalty
 *                    associated to the difference in orientation between model
 *                    edge and detected edge
 * @param dist_sigma  the sigma for the gaussian used to compute the penalty
 *                    associated to the distance between the model point and the
 *                    edge point, computed along the edge gradient
 * @param max_dist    The max distance between an edge point and a model point.
 *                    If their distance is bigger than max_distance, a model point
 *                    cannot be assigned to an edge point
 * @param bg_prob     The probability that a point in an image is background (ie not part
 *                    of an edge).
 * @param inoise_prob The probability of detecting noise in a pixel of the image (ie the
 *                    probability of detecting an edge point where there should not be any)
 * @param silho_miss_prob The probability of not having any image detected edge explaining
 *                        a particular point of the model backprojected onto the image plane.
 *                        The model edge is part of the silhouette of an object, so it should be
 *                        pretty sharp, and the probability of such misdetection should be
 *                        relatively low
 * @param inner_miss_prob Same as above, but the model edge is not part of the silhouette (inner
 *                        edge), so it is likely to be a weak edge, and the probability of such
 *                        misdetection should be higher than that of a silhouette edge
 */
Independent_edge_points_likelihood::Independent_edge_points_likelihood
(
    const kjb::Edge_set * idata_edges,
    int num_angles,
    unsigned int num_rows,
    unsigned int num_cols,
    double angle_sigma,
    double dist_sigma,
    double max_dist,
    double bg_prob,
    double inoise_prob,
    double silho_miss_prob,
    double inner_miss_prob,
    bool idelete_correspondence
) :
    //m_ll_assign(num_rows, num_cols),
    data_edges(idata_edges),
    background_prob(bg_prob),
    noise_prob(inoise_prob),
    silhouette_miss_prob(silho_miss_prob),
    inner_edge_miss_prob(inner_miss_prob),
    M(4,4),
    image_size(num_rows*num_cols),
    matched_points(20000),
    matched_pts(idata_edges->get_total_edge_points())
{
    correspondence = new Correspondence(data_edges, num_rows, num_cols, num_angles, angle_sigma, dist_sigma, max_dist);

    for (unsigned int i = 0; i < matched_pts.size(); i++)
    {
        // A good beginning reserve value would be the number of projected
        // model lines on average
        matched_pts[ i ].reserve(15);
    }

    background_prob = MAX_OF(MIN_LOG_ARG, background_prob);
    noise_prob = MAX_OF(MIN_LOG_ARG, noise_prob);
    silhouette_miss_prob = MAX_OF(MIN_LOG_ARG, silhouette_miss_prob);
    inner_edge_miss_prob = MAX_OF(MIN_LOG_ARG, inner_edge_miss_prob);

    /** This part is for debug purposes */
    /*edge_mask.resize(data_edges.num_edges());
    for(unsigned int i = 0; i < data_edges.num_edges(); i++)
    {
        for(unsigned int j = 0; j < data_edges.edge_length(i); j++)
        {
            edge_mask[i].push_back(false);
        }
    }*/

    num_points_ratio = 1.0/(double)data_edges->get_total_edge_points();
    _num_data_point = (double) (data_edges->get_total_edge_points());
    used_matched_points = 0;
    delete_correspondence = idelete_correspondence;
    std::cout << "The num points is: " << num_points_ratio << std::endl;
}

/**
 * Copy constructor. Notice that this is not efficient. You probably will not need to copy this
 * class very often, however you will experience that copying this class is still faster that creating
 * it twice
 *
 *  @param src The likelihood to copy into this one
 *
 */
Independent_edge_points_likelihood::Independent_edge_points_likelihood
(
    const Independent_edge_points_likelihood& src
) :
    data_edges(src.data_edges), correspondence(src.correspondence),
    matched_points(src.matched_points)
{
    //m_ll_assign = src.m_ll_assign;
    internal_model_map = src.internal_model_map;
    background_prob = src.background_prob;
    noise_prob = src.noise_prob;
    silhouette_miss_prob = src.silhouette_miss_prob;
    inner_edge_miss_prob = src.inner_edge_miss_prob;
    num_points_ratio = src.num_points_ratio;
    _num_data_point = src._num_data_point;
    _num_miss = src._num_miss;
    edge_mask = src.edge_mask;
    M = src.M;
    image_size = src.image_size;
    used_matched_points = src.used_matched_points;
    delete_correspondence = src.delete_correspondence;

    matched_pts.resize(src.matched_pts.size());

    for(unsigned int i = 0; i < matched_pts.size(); i++)
    {
        matched_pts[ i ].reserve(15);
    }
}

void Independent_edge_points_likelihood::reset_edge_mask() const
{
    for(unsigned int i = 0; i < edge_mask.size(); i++)
    {
        for(unsigned int j = 0; j < edge_mask[i].size(); j++)
        {
            (edge_mask[i])[j] = false;
        }
    }
}

double Independent_edge_points_likelihood::operator()
(
    Int_matrix & model_map,
    const std::vector<Model_edge> & model_edges
) const
{
    return 0.0;
}

/** @brief Calculates the log likelihood of the image edge set
 *  given the rendering of the model onto the image plane (model_map)
 *  and the set of the model edges. We assumed that each edge was
 *  rendered onto the image plane with a color equal to its id
 *  (ie its position in the line_segment_set)
 *
 *  @param model_map contains the rendering of the model as described above
 *  @param model_edges the list of model edges as described above
 */
double Independent_edge_points_likelihood::old_operator
(
    Int_matrix & model_map,
    const std::vector<Model_edge> & model_edges,
    const std::vector<kjb::Int_vector> & external_edge_indexes,
    int external_edge_counter
) const
{
    using namespace std;

    unsigned int num_background_pts = image_size;
    unsigned int num_pts = 0;
    unsigned int num_noise_pts = 0;
    unsigned int num_missed_pts = 0;
    int num_silhouette_miss_pts = 0;
    int num_inner_edge_miss_pts = 0;

    double log_likelihood = 0;

    /*reset_edge_mask(); //for debug purposes
    m_ll_assign.zero_out(m_ll_assign.get_num_rows(), m_ll_assign.get_num_cols()); // for debug purposes */

    /** Find the correspondences between the data edges and the model input data */
    const vector< vector<const Correspondence::Point*> > & correspondence_pts =
                 correspondence->generate_for_model_from_map_and_edges(model_map, model_edges, external_edge_indexes,
                         external_edge_counter, num_silhouette_miss_pts, num_inner_edge_miss_pts );
    clock_t clock2 = clock();

    /** Edge points correspondences are stored sequentially in correspondence_pts,
     * indexed by their edge point number, there is no concept of what edge they
     * belong to. This is why we need to use separate counters (e for correspondence_pts,
     * edge_index and p for the edge_set).
     */
    unsigned int e = 0;
    for(unsigned int edge_index = 0; edge_index < data_edges->num_edges(); edge_index++ )
    {
        for(unsigned int e_pt_index = 0; e_pt_index < data_edges->get_edge(edge_index).get_num_points(); e_pt_index++)
        {
            kjb::Edge_point edge_pt = data_edges->get_edge(edge_index).get_edge_point(e_pt_index);

            if ( correspondence_pts[ e ].size() > 0)
            {

                //(edge_mask[edge_index])[e_pt_index] = true; // This line for debug purposes

                /** If we are here, it means that for this edge point we found at least a
                 *  model point that could have generated it. The first one we fetch
                 *  here should be the closes because the vector is sorted by distance */
                const Correspondence::Point* correspondence_pt = correspondence_pts[ e ][0];
                ASSERT(correspondence_pt->edge_pt_row == edge_pt.get_row());
                ASSERT(correspondence_pt->edge_pt_col == edge_pt.get_col());

                /** This is the number of model points that could explain the detected
                 *  edge point. We will average their penalties. This makes sense because
                 *  we don't really know if the closest model point is the right correspondence,
                 *  so we average the penalties over the model points nearby. Notice that at this
                 *  point, a model point was already assigned to a single edge point (the closest),
                 *  but it often happens that an edge point is the closest to multiple model
                 *  points
                 */
                unsigned int k = correspondence_pts[ e ].size();
                double   s = 0;

                correspondence_pt = correspondence_pts[ e ][ 0 ];
                if(model_edges[correspondence_pt->model_edge_index].is_silhouette())
                {
                    num_silhouette_miss_pts--;
                }
                else
                {
                    num_inner_edge_miss_pts--;
                }
                s += correspondence_pt->gauss_angle * correspondence_pt->gauss_norm_dist;
                for (unsigned int j = 1; j < k; j++)
                {
                    correspondence_pt = correspondence_pts[ e ][ j ];

                    /** Fetch the precomputed penalties for orientation difference and distance */
                    s += correspondence_pt->gauss_angle * correspondence_pt->gauss_norm_dist;
                 }
                 s /= (double)k;

                if(s > MIN_LOG_ARG)
                {
                    log_likelihood += log(s);
                }
                else
                {
                    log_likelihood += log(MIN_LOG_ARG);
                }

                num_pts++;
                //m_ll_assign( edge_pt.get_row(), edge_pt.get_col()) = 1;
            }
            else
              {
                /** No model point explains this edge point. We thus explain the latter as noise */
                num_noise_pts++;
                //m_ll_assign( edge_pt.get_row(), edge_pt.get_col()) = 2;
            }
            e++;
        } /** End for loop over p */
    } /** End for loop over e */
    num_missed_pts = num_silhouette_miss_pts + num_inner_edge_miss_pts;

    /** Everything that is not a correspondence, noise or a miss, is background
     *  Backgorund was initialized to the number of pixels in the image */
    num_background_pts -= num_pts + num_noise_pts + num_missed_pts;

    /** Finish by adding to the likelihood the contributions of background
     *  points, noise points, missed silhouette points and missed inner points */
    /**double points_contribution = log_likelihood;*/
    log_likelihood += num_background_pts * log(background_prob);
    log_likelihood += num_noise_pts * log(noise_prob);
    log_likelihood += num_silhouette_miss_pts * log(silhouette_miss_prob);
    log_likelihood += num_inner_edge_miss_pts * log(inner_edge_miss_prob);
    _num_miss = (double) (num_silhouette_miss_pts + num_inner_edge_miss_pts);

    /** TODO CHECK THAT LIKELHIOOD VALUE IS OK
    #if defined FURNITURE_HAVE_ISINF && defined FURNITURE_HAVE_ISNAN
    ASSERT(!std::isinf(ll) && !std::isnan(ll));
    #endif
     */

    return log_likelihood;
}

double Independent_edge_points_likelihood::new_operator
(
    Int_matrix & model_map,
    const std::vector<Model_edge> & model_edges,
    const std::vector<kjb::Int_vector> & external_edge_indexes,
    int external_edge_counter
) const
{
    using namespace std;

    unsigned int num_background_pts = image_size;
    unsigned int num_pts = 0;
    unsigned int num_noise_pts = 0;
    unsigned int num_missed_pts = 0;
    int num_silhouette_miss_pts = 0;
    int num_inner_edge_miss_pts = 0;
    int num_flagged_miss = 0;

    double log_likelihood = 0;

    /*reset_edge_mask(); //for debug purposes
    m_ll_assign.zero_out(m_ll_assign.get_num_rows(), m_ll_assign.get_num_cols()); // for debug purposes */

    /** Find the correspondences between the data edges and the model input data */
     correspondence->generate_for_model_from_map_and_edges(model_map, model_edges, external_edge_indexes, matched_points,
             matched_pts, external_edge_counter, num_silhouette_miss_pts,
             num_inner_edge_miss_pts, num_flagged_miss, used_matched_points ,
             inner_edge_miss_prob, silhouette_miss_prob);

    /** Edge points correspondences are stored sequentially in correspondence_pts,
     * indexed by their edge point number, there is no concept of what edge they
     * belong to. This is why we need to use separate counters (e for correspondence_pts,
     * edge_index and p for the edge_set).
     */
    unsigned int e = 0;
    for(unsigned int edge_index = 0; edge_index < data_edges->num_edges(); edge_index++ )
    {
        for(unsigned int e_pt_index = 0; e_pt_index < data_edges->get_edge(edge_index).get_num_points(); e_pt_index++)
        {
            kjb::Edge_point edge_pt = data_edges->get_edge(edge_index).get_edge_point(e_pt_index);

            if ( matched_pts[ e ].size() > 0)
            {

                //(edge_mask[edge_index])[e_pt_index] = true; // This line for debug purposes

                /** If we are here, it means that for this edge point we found at least a
                 *  model point that could have generated it. The first one we fetch
                 *  here should be the closes because the vector is sorted by distance */
                const Correspondence::Matched_point* matched_pt = matched_pts[ e ][0];
                //ASSERT(correspondence_pt->edge_pt_row == edge_pt.get_row());
                //ASSERT(correspondence_pt->edge_pt_col == edge_pt.get_col());

                /** This is the number of model points that could explain the detected
                 *  edge point. We will average their penalties. This makes sense because
                 *  we don't really know if the closest model point is the right correspondence,
                 *  so we average the penalties over the model points nearby. Notice that at this
                 *  point, a model point was already assigned to a single edge point (the closest),
                 *  but it often happens that an edge point is the closest to multiple model
                 *  points
                 */
                unsigned int k = matched_pts[ e ].size();
                double   s = 0;

                matched_pt = matched_pts[ e ][ 0 ];
                if(model_edges[matched_pt->model_edge_index].is_silhouette())
                {
                    num_silhouette_miss_pts--;
                }
                else
                {
                    num_inner_edge_miss_pts--;
                }
                s += matched_pt->gauss_angle * matched_pt->gauss_norm_dist;
                for (unsigned int j = 1; j < k; j++)
                {
                    matched_pt = matched_pts[ e ][ j ];

                    /** Fetch the precomputed penalties for orientation difference and distance */
                    s += matched_pt->gauss_angle * matched_pt->gauss_norm_dist;
                 }
                 s /= (double)k;

                if(s > MIN_LOG_ARG)
                {
                    log_likelihood += log(s);
                }
                else
                {
                    log_likelihood += log(MIN_LOG_ARG);
                }

                num_pts++;
                //m_ll_assign( edge_pt.get_row(), edge_pt.get_col()) = 1;
            }
            else
              {
                /** No model point explains this edge point. We thus explain the latter as noise */
                num_noise_pts++;
                //m_ll_assign( edge_pt.get_row(), edge_pt.get_col()) = 2;
            }
            e++;
        } /** End for loop over p */
    } /** End for loop over e */
    num_missed_pts = num_silhouette_miss_pts + num_inner_edge_miss_pts;

    /** Everything that is not a correspondence, noise or a miss, is background
     *  Backgorund was initialized to the number of pixels in the image */
    num_background_pts -= num_pts + num_noise_pts + num_missed_pts;

    /** Finish by adding to the likelihood the contributions of background
     *  points, noise points, missed silhouette points and missed inner points */
    /**double points_contribution = log_likelihood;*/
    log_likelihood += num_background_pts * log(background_prob);
    log_likelihood += num_noise_pts * log(noise_prob);
    log_likelihood /= _num_data_point;
    double accurate_innner_miss = log(inner_edge_miss_prob)/_num_data_point;
    double accurate_silhouette_miss = log(silhouette_miss_prob)/_num_data_point;
    log_likelihood += num_silhouette_miss_pts * accurate_silhouette_miss;
    log_likelihood += num_inner_edge_miss_pts * accurate_innner_miss;
    _num_miss = (double) (num_silhouette_miss_pts + num_inner_edge_miss_pts);

    /** TODO CHECK THAT LIKELHIOOD VALUE IS OK
    #if defined FURNITURE_HAVE_ISINF && defined FURNITURE_HAVE_ISNAN
    ASSERT(!std::isinf(ll) && !std::isnan(ll));
    #endif
     */

    return log_likelihood;
}

double Independent_edge_points_likelihood::operator()
(
    Int_matrix & model_map,
    const std::vector<Model_edge> & model_edges,
    const std::vector<kjb::Int_vector> & external_edge_indexes,
    int external_edge_counter
) const
{
    using namespace std;

    unsigned int num_background_pts = image_size;
    unsigned int num_pts = 0;
    unsigned int num_noise_pts = 0;
    unsigned int num_missed_pts = 0;
    int num_silhouette_miss_pts = 0;
    int num_inner_edge_miss_pts = 0;
    int num_flagged_miss = 0;

    double log_likelihood = 0;

    /*reset_edge_mask(); //for debug purposes
    m_ll_assign.zero_out(m_ll_assign.get_num_rows(), m_ll_assign.get_num_cols()); // for debug purposes */

    /** Find the correspondences between the data edges and the model input data */
     correspondence->generate_for_model_from_map_and_edges(model_map, model_edges, external_edge_indexes, matched_points,
             matched_pts, external_edge_counter, num_silhouette_miss_pts,
             num_inner_edge_miss_pts, num_flagged_miss, used_matched_points, inner_edge_miss_prob,
             silhouette_miss_prob);

    /** Edge points correspondences are stored sequentially in correspondence_pts,
     * indexed by their edge point number, there is no concept of what edge they
     * belong to. This is why we need to use separate counters (e for correspondence_pts,
     * edge_index and p for the edge_set).
     */
    unsigned int e = 0;
    for(unsigned int edge_index = 0; edge_index < data_edges->num_edges(); edge_index++ )
    {
        for(unsigned int e_pt_index = 0; e_pt_index < data_edges->get_edge(edge_index).get_num_points(); e_pt_index++)
        {
            kjb::Edge_point edge_pt = data_edges->get_edge(edge_index).get_edge_point(e_pt_index);

            if ( matched_pts[ e ].size() > 0)
            {

                //(edge_mask[edge_index])[e_pt_index] = true; // This line for debug purposes

                /** If we are here, it means that for this edge point we found at least a
                 *  model point that could have generated it. The first one we fetch
                 *  here should be the closes because the vector is sorted by distance */
                const Correspondence::Matched_point* matched_pt = matched_pts[ e ][0];
                //ASSERT(correspondence_pt->edge_pt_row == edge_pt.get_row());
                //ASSERT(correspondence_pt->edge_pt_col == edge_pt.get_col());

                /** This is the number of model points that could explain the detected
                 *  edge point. We will average their penalties. This makes sense because
                 *  we don't really know if the closest model point is the right correspondence,
                 *  so we average the penalties over the model points nearby. Notice that at this
                 *  point, a model point was already assigned to a single edge point (the closest),
                 *  but it often happens that an edge point is the closest to multiple model
                 *  points
                 */
                unsigned int k = matched_pts[ e ].size();
                double   s = 0;

                matched_pt = matched_pts[ e ][ 0 ];
                if(model_edges[matched_pt->model_edge_index].is_silhouette())
                {
                    num_silhouette_miss_pts--;
                }
                else if(!model_edges[matched_pt->model_edge_index].is_flagged())
                {
                    num_inner_edge_miss_pts--;
                }
                else
                {
                    num_flagged_miss--;
                }
                s += matched_pt->gauss_angle * matched_pt->gauss_norm_dist;
                for (unsigned int j = 1; j < k; j++)
                {
                    matched_pt = matched_pts[ e ][ j ];

                    /** Fetch the precomputed penalties for orientation difference and distance */
                    s += matched_pt->gauss_angle * matched_pt->gauss_norm_dist;
                 }
                 s /= (double)k;

                if(s > MIN_LOG_ARG)
                {
                    log_likelihood += log(s);
                }
                else
                {
                    log_likelihood += log(MIN_LOG_ARG);
                }

                num_pts++;
                //m_ll_assign( edge_pt.get_row(), edge_pt.get_col()) = 1;
            }
            else
              {
                /** No model point explains this edge point. We thus explain the latter as noise */
                num_noise_pts++;
                //m_ll_assign( edge_pt.get_row(), edge_pt.get_col()) = 2;
            }
            e++;
        } /** End for loop over p */
    } /** End for loop over e */
    num_missed_pts = num_silhouette_miss_pts + num_inner_edge_miss_pts;

    /** Everything that is not a correspondence, noise or a miss, is background
     *  Backgorund was initialized to the number of pixels in the image */
    num_background_pts -= num_pts + num_noise_pts + num_missed_pts;

    /** Finish by adding to the likelihood the contributions of background
     *  points, noise points, missed silhouette points and missed inner points */
    /**double points_contribution = log_likelihood;*/
    log_likelihood += num_background_pts * log(background_prob);
    //double avg_ll = log_likelihood / ((double) _num_data_point - num_noise_pts);
    log_likelihood += num_noise_pts * log(noise_prob);
    log_likelihood += num_silhouette_miss_pts * log(silhouette_miss_prob);
    log_likelihood += num_inner_edge_miss_pts * log(inner_edge_miss_prob);
    log_likelihood += num_flagged_miss * log(1e-10);
    _num_miss = (double) (num_silhouette_miss_pts + num_inner_edge_miss_pts + num_flagged_miss);
    //std::cout << "Num data points:" << _num_data_point << std::endl;
    /*std::cout << "Original ll: " << avg_ll << std::endl;
    std::cout << "Num inner miss:" << num_inner_edge_miss_pts << std::endl;
    std::cout << "Inner ll " << num_inner_edge_miss_pts*log(inner_edge_miss_prob) << std::endl;
    std::cout << "Num sil miss:" << num_silhouette_miss_pts << std::endl;
    std::cout << "Sil ll " << num_silhouette_miss_pts*log(silhouette_miss_prob) << std::endl;
    std::cout << "Num noise:" << num_noise_pts << std::endl;
    std::cout << "Noise ll " << num_noise_pts*log(noise_prob) << std::endl;*/

    /** TODO CHECK THAT LIKELHIOOD VALUE IS OK
    #if defined FURNITURE_HAVE_ISINF && defined FURNITURE_HAVE_ISNAN
    ASSERT(!std::isinf(ll) && !std::isnan(ll));
    #endif
     */

    return log_likelihood;
}

/** @brief Calculates the log likelihood of the image edge set
 *  by comparing it to another edge set. Usually this data comes
 *  from rendering a model onto the image plane
 *
 *  @param model_edge_set The edge set to compare to
 */
double Independent_edge_points_likelihood::operator()
(
    const Edge_set& model_edge_set
) const
{
    using boost::unordered_map;
    using std::vector;

    unsigned int num_rows = model_edge_set.num_rows();
    unsigned int num_cols = model_edge_set.num_cols();

    // model and data edge sets should come from same size image.
    ASSERT(data_edges->num_rows() == model_edge_set.num_rows());
    ASSERT(data_edges->num_cols() == model_edge_set.num_cols());

    //m_ll_assign.zero_out(m_ll_assign.get_num_rows(), m_ll_assign.get_num_cols());

    // "Hash table"
    typedef unordered_map<int, unordered_map< int, const kjb::Edge_point* > > Edge_map;
    typedef unordered_map<int, unordered_map< int, const kjb::Edge_point* > >::iterator Edge_map_iterator;
    typedef unordered_map< int, const kjb::Edge_point*>::iterator Edge_map_iterator_iterator;

    Edge_map map;

    for(unsigned int e = 0; e < model_edge_set.num_edges(); e++)
    {
        for(unsigned int p = 0; p < model_edge_set.get_edge(e).get_num_points(); p++)
        {
            kjb::Edge_point edge_pt = model_edge_set.get_edge(e).get_edge_point(p);

            ASSERT(edge_pt.get_row() < num_rows);
            ASSERT(edge_pt.get_col() < num_cols);
            map[edge_pt.get_row()][edge_pt.get_col()] = &edge_pt;
        }
    }

    const vector< vector<const Correspondence::Point*> > & correspondence_pts =
        correspondence->generate_for_model(model_edge_set);

    double likelihood = 0;

    bool silhouette;

    unsigned int num_background_pts = num_rows * num_cols;
    unsigned int num_pts = 0;
    unsigned int num_noise_pts = 0;
    unsigned int num_missed_pts = 0;
    unsigned int num_silhouette_miss_pts = 0;
    unsigned int num_inner_edge_miss_pts = 0;


    /** Edge points correspondences are stored sequentially in correspondence_pts,
     * indexed by their edge point number, there is no concept of what edge they
     * belong to. This is why we need to use separate counters (e for correspondence_pts,
     * edge_index and p for the edge_set).
     */
    unsigned int e = 0;
    for(unsigned int edge_index = 0; edge_index < data_edges->num_edges(); edge_index++ )
    {
        for(unsigned int e_pt_index = 0; e_pt_index < data_edges->get_edge(edge_index).get_num_points(); e_pt_index++)
        {
            kjb::Edge_point edge_pt = data_edges->get_edge(edge_index).get_edge_point(e_pt_index);

            if (correspondence_pts[ e ].size() > 0)
            {
                const Correspondence::Point* correspondence_pt = correspondence_pts[ e ][0];
                ASSERT(correspondence_pt->edge_pt_row == edge_pt.get_row());
                ASSERT(correspondence_pt->edge_pt_col == edge_pt.get_col());

                unsigned int k = correspondence_pts[ e ].size();
                double s = 0;

                unsigned int closest_correspondence_pt = 0;
                double min_distance = correspondence_pts[ e ][ 0 ]->norm_dist;

                for (unsigned int j = 0; j < k; j++)
                {
                    correspondence_pt = correspondence_pts[ e ][ j ];

                    s += (1.0/k) * correspondence_pt->gauss_angle * correspondence_pt->gauss_norm_dist;

                    if (correspondence_pt->norm_dist < min_distance)
                    {
                        closest_correspondence_pt = j;
                        min_distance = correspondence_pt->norm_dist;
                    }
                }

                likelihood += log((s > MIN_LOG_ARG) ? s : MIN_LOG_ARG);

                if (k > 1)
                {
                    num_missed_pts += k-1;

                    for (unsigned int j = 0; j < k; j++)
                    {
                        if (j == closest_correspondence_pt)
                        {
                            continue;
                        }

                        correspondence_pt = correspondence_pts[ e ][ j ];

                        unsigned int row = correspondence_pt->model_pt_row;
                        unsigned int col = correspondence_pt->model_pt_col;

                        ASSERT(map.count(row) > 0 && map[row].count(col) > 0);

                        silhouette = map[ row ][ col ]->get_silhouette();
                        if (silhouette)
                        {
                            num_silhouette_miss_pts++;
                            //m_ll_assign(row , col) = 3;
                        }
                        else
                        {
                            num_inner_edge_miss_pts++;
                            //m_ll_assign(row , col) = 4;
                        }
                    }
                }

                for (unsigned int j = 0; j < k; j++)
                {
                    correspondence_pt = correspondence_pts[ e ][ j ];

                    // WHY CONST?
                    //const unsigned int row = correspondence_pt->model_pt_row;
                    //const unsigned int col = c_pt->m_pt_col;

                    ASSERT(map.count( correspondence_pt->model_pt_row) > 0);

                    map[ correspondence_pt->model_pt_row ].erase(  correspondence_pt->model_pt_col);
                }

                num_pts++;
                //m_ll_assign(edge_pt.get_row() , edge_pt.get_col() ) = 1;
            }
            else
            {
                num_noise_pts++;
                //m_ll_assign(edge_pt.get_row() , edge_pt.get_col() ) = 2;
            }

            e++;
        }
    }

    for(Edge_map_iterator it = map.begin(); it != map.end(); it++)
    {
        Edge_map_iterator_iterator itt = it->second.begin();

        for(; itt != it->second.end(); itt++)
        {
            num_missed_pts++;

            //unsigned int row = itt->second->get_row();
            //unsigned int col = itt->second->get_col();

            bool silhouette = itt->second->get_silhouette();
            if (silhouette)
            {
                num_silhouette_miss_pts++;
                //m_ll_assign(row, col) = 3;
            }
            else
            {
                num_inner_edge_miss_pts++;
                //m_ll_assign(row, col) = 4;
            }
        }
    }

    num_background_pts -= num_pts + num_noise_pts + num_missed_pts;

    unsigned int N = num_background_pts + num_pts + num_missed_pts + num_noise_pts;
    ASSERT(N == num_rows*num_cols);
    ASSERT(num_missed_pts == (num_silhouette_miss_pts + num_inner_edge_miss_pts));

    likelihood += num_background_pts * log(background_prob);
    likelihood += num_noise_pts * log(noise_prob);
    likelihood += num_silhouette_miss_pts * log(silhouette_miss_prob);
    likelihood += num_inner_edge_miss_pts * log(inner_edge_miss_prob);
    _num_miss = (double) (num_silhouette_miss_pts + num_inner_edge_miss_pts);

    /*
    #if defined FURNITURE_HAVE_ISINF && defined FURNITURE_HAVE_ISNAN
        ASSERT(!std::isinf(ll) && !std::isnan(ll));
    #endif
    */

    return likelihood;
}

/**
 * For debug purposes
 */
void Independent_edge_points_likelihood::draw_edge_set_using_mask(Image& img)
{
    for(unsigned int i = 0; i < edge_mask.size(); i++)
    {
        for(unsigned int j = 0; j < edge_mask[i].size(); j++)
        {
            if( (edge_mask[i])[j])
            {
                data_edges->get_edge(i).get_edge_point(j).draw(img, 0, 0, 255);
            }
            else
            {
                data_edges->get_edge(i).get_edge_point(j).draw(img, 255, 0, 0);
            }
        }
    }
}

double Independent_edge_points_likelihood::find_log_max(double angle_sigma, double dist_sigma)
{
    double single_angle_penalty = 0.0;
    kjb_c::gaussian_pdf(&single_angle_penalty, 0, 0, angle_sigma);
    double single_dist_penalty = 0.0;
    kjb_c::gaussian_pdf(&single_dist_penalty, 0, 0, dist_sigma);
    double single_penalty = log(single_angle_penalty) + log(single_dist_penalty);
    return (single_penalty*data_edges->get_total_edge_points());
}

