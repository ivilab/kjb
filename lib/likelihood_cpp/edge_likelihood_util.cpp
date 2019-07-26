/* $Id: edge_likelihood_util.cpp 14948 2013-07-18 16:00:33Z delpero $ */
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
   |  Author:  Luca Del Pero, Jinyan Guan
 * =========================================================================== */

#include "likelihood_cpp/edge_likelihood_util.h"
#include "gr_cpp/gr_camera.h"
#include "gr_cpp/gr_polygon.h"
#include "i_cpp/i_image.h"

using namespace kjb;

/** @brief Prepares a model map from a polymesh. Each edge of the polymesh
 * is rendered using its id as a color. The id is obtained by assigning id = 1
 * to the first edge, and sequentially increasing ids to the following
 * edges
 *
 * @param model_map Will prepare the model map from the input mesh
 * @param p         The mesh that will be rendered onto the map
 */
void kjb::prepare_model_map(Int_matrix & model_map, const Polymesh & p)
{
    /** We render the polymesh in a way that occluded edges are not visible,
     *  and such that every edge is rendered with pixel color equal to its id.
     *  The id of the first edge is set to 1, second edge to 2 and so on*/
    p.wire_occlude_render();
    p.wire_render_with_sequential_ids(1);
    Base_gl_interface::capture_gl_view(model_map);
}

/** Prepares a set of model edges from a polymesh. The extrema of
 *  each edge are projected onto the image plane using the current gl projection
 *  and modelview matrices. Each projected edge is stored as a line segment
 *  having as extrema the projections of the extrema of the 3D polymesh edge.
 *  We also check for each edge whether it is part of the silhoutte of
 *  the polymesh or if it is an inner edge (we need the camera to do this
 *  test)
 *
 *  @param edges Will containt the projected edges
 *  @param p     The input polymesh
 *  @param eye   The camera used to render this mesh
 */
void kjb::prepare_model_edges(std::vector<Model_edge> & edges, const Polymesh & p, const Base_gl_interface & eye)
{

    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int adjacent_face_index = 0;
    bool is_silhouette_edge = false;
    bool is_visible_edge = false;
    bool face_visible = false;
    bool adjacent_face_visible = false;

    /** We intentionally copy all the faces here, because we need to
     *  project them. This function could be made for efficient by
     *  projecting each polygon here without calling the function project,
     *  so that we need to retrieve the gl matrices needed for the projection
     *  only once
     */
    std::vector<kjb::Polygon> faces = p.get_faces();

    for( i = 0; i < faces.size(); i++)
    {
        face_visible = eye.Polygon_visibility_test(p.get_face(i));
        faces[i].project();
        const std::vector<kjb::Vector> & vertices = faces[i].get_vertices();

        for( j = 0; j < (vertices.size() - 1); j++)
        {
            /** We retrieve the face adjacent to the ith face along the jth edge.
             * If they are both invisible or both visible, this is clearly not a silhouette
             * edge. If one is visible and the other is not, it clearly is a silhouette
             * edge. We need to distinguish between silhouette and no-silhouette edges
             * because we assume that silhouette edges are usually sharper than inner edges,
             * and the penalty for missing inner edges is thus larger
             */
            adjacent_face_index = p.adjacent_face(i, j);
            adjacent_face_visible = eye.Polygon_visibility_test(p.get_face(adjacent_face_index));

            is_silhouette_edge = ( adjacent_face_visible && !face_visible)|| 
                              (face_visible && !adjacent_face_visible); 
            is_visible_edge = is_silhouette_edge || (face_visible && adjacent_face_visible);
            edges.push_back( Model_edge( (vertices[j])(0), (vertices[j])(1),
                             (vertices[j+1])(0), (vertices[j+1])(1), is_silhouette_edge, is_visible_edge ) );
        }
        if(vertices.size()>0) 
        {
            /** We here compute the line segment between the last and the first vertex of
             * the polygon */
            adjacent_face_index = p.adjacent_face(i, j);
            adjacent_face_visible = eye.Polygon_visibility_test(p.get_face(adjacent_face_index));
            is_silhouette_edge =  (adjacent_face_visible && !face_visible) ||
                              (face_visible && !adjacent_face_visible); 
            is_visible_edge = is_silhouette_edge || (face_visible && adjacent_face_visible);
            
            edges.push_back( Model_edge( (vertices[j])(0), (vertices[j])(1),
                (vertices[0])(0), (vertices[0])(1), is_silhouette_edge, is_visible_edge ) );
        }
    }
}

/** Prepares a set of model edges from a polymesh. The extrema of
 *  each edge are projected onto the image plane using the current gl projection
 *  and modelview matrices. Each projected edge is stored as a line segment
 *  having as extrema the projections of the extrema of the 3D polymesh edge.
 *  We also check for each edge whether it is part of the silhoutte of
 *  the polymesh or if it is an inner edge (we need the camera to do this
 *  test)
 *
 *  @param edges Will containt the projected edges
 *  @param p     The input polymesh
 *  @param eye   The camera used to render this mesh
 */
void kjb::prepare_model_edges
(
    std::vector<Model_edge> & edges,
    const Polymesh & p,
    const Base_gl_interface & eye,
    const Matrix & M,
    double width,
    double height,
    bool flagged
)
{

    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int adjacent_face_index = 0;
    bool is_silhouette_edge = false;
    bool is_visible_edge = false;
    bool face_visible = false;
    bool adjacent_face_visible = false;

    /** We intentionally copy all the faces here, because we need to
     *  project them. This function could be made for efficient by
     *  projecting each polygon here without calling the function project,
     *  so that we need to retrieve the gl matrices needed for the projection
     *  only once
     */
    std::vector<kjb::Polygon> faces = p.get_faces();
    for( i = 0; i < faces.size(); i++)
    {
        face_visible = eye.Polygon_visibility_test(p.get_face(i));
        faces[i].project(M, width, height);

        const std::vector<kjb::Vector> & vertices = faces[i].get_vertices();

        for( j = 0; j < (vertices.size() - 1); j++)
        {
            /** We retrieve the face adjacent to the ith face along the jth edge.
             * If they are both invisible or both visible, this is clearly not a silhouette
             * edge. If one is visible and the other is not, it clearly is a silhouette
             * edge. We need to distinguish between silhouette and no-silhouette edges
             * because we assume that silhouette edges are usually sharper than inner edges,
             * and the penalty for missing inner edges is thus larger
             */
            adjacent_face_index = p.adjacent_face(i, j);
            adjacent_face_visible = eye.Polygon_visibility_test(p.get_face(adjacent_face_index));

            is_silhouette_edge = ( adjacent_face_visible && !face_visible)||
                              (face_visible && !adjacent_face_visible);
            is_visible_edge = is_silhouette_edge || (face_visible && adjacent_face_visible);
            edges.push_back( Model_edge( (vertices[j])(0), (vertices[j])(1),
                             (vertices[j+1])(0), (vertices[j+1])(1), is_silhouette_edge, is_visible_edge, flagged) );
        }
        if(vertices.size()>0)
        {
            /** We here compute the line segment between the last and the first vertex of
             * the polygon */
            adjacent_face_index = p.adjacent_face(i, j);
            adjacent_face_visible = eye.Polygon_visibility_test(p.get_face(adjacent_face_index));
            is_silhouette_edge =  (adjacent_face_visible && !face_visible) ||
                              (face_visible && !adjacent_face_visible);
            is_visible_edge = is_silhouette_edge || (face_visible && adjacent_face_visible);

            edges.push_back( Model_edge( (vertices[j])(0), (vertices[j])(1),
                (vertices[0])(0), (vertices[0])(1), is_silhouette_edge, is_visible_edge, flagged ) );
        }
    }
}

/** Prepares a set of model edges from a polymesh. The extrema of
 *  each edge are projected onto the image plane using the current gl projection
 *  and modelview matrices. Each projected edge is stored as a line segment
 *  having as extrema the projections of the extrema of the 3D polymesh edge.
 *  We also check for each edge whether it is part of the silhoutte of
 *  the polymesh or if it is an inner edge (we need the camera to do this
 *  test)
 *
 *  @param edges Will containt the projected edges
 *  @param p     The input polymesh
 *  @param eye   The camera used to render this mesh
 */
void kjb::prepare_model_edges
(
    std::vector<Model_edge> & edges,
    const Polymesh & p,
    const Base_gl_interface & eye,
    const Matrix & M,
    double width,
    double height
)
{

    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int adjacent_face_index = 0;
    bool is_silhouette_edge = false;
    bool is_visible_edge = false;
    bool face_visible = false;
    bool adjacent_face_visible = false;

    /** We intentionally copy all the faces here, because we need to
     *  project them. This function could be made for efficient by
     *  projecting each polygon here without calling the function project,
     *  so that we need to retrieve the gl matrices needed for the projection
     *  only once
     */
    std::vector<kjb::Polygon> faces = p.get_faces();
    for( i = 0; i < faces.size(); i++)
    {
        face_visible = eye.Polygon_visibility_test(p.get_face(i));
        faces[i].project(M, width, height);

        const std::vector<kjb::Vector> & vertices = faces[i].get_vertices();

        for( j = 0; j < (vertices.size() - 1); j++)
        {
            /** We retrieve the face adjacent to the ith face along the jth edge.
             * If they are both invisible or both visible, this is clearly not a silhouette
             * edge. If one is visible and the other is not, it clearly is a silhouette
             * edge. We need to distinguish between silhouette and no-silhouette edges
             * because we assume that silhouette edges are usually sharper than inner edges,
             * and the penalty for missing inner edges is thus larger
             */
            adjacent_face_index = p.adjacent_face(i, j);
            adjacent_face_visible = eye.Polygon_visibility_test(p.get_face(adjacent_face_index));

            is_silhouette_edge = ( adjacent_face_visible && !face_visible)||
                              (face_visible && !adjacent_face_visible);
            is_visible_edge = is_silhouette_edge || (face_visible && adjacent_face_visible);
            edges.push_back( Model_edge( (vertices[j])(0), (vertices[j])(1),
                             (vertices[j+1])(0), (vertices[j+1])(1), is_silhouette_edge, is_visible_edge, false) );
        }
        if(vertices.size()>0)
        {
            /** We here compute the line segment between the last and the first vertex of
             * the polygon */
            adjacent_face_index = p.adjacent_face(i, j);
            adjacent_face_visible = eye.Polygon_visibility_test(p.get_face(adjacent_face_index));
            is_silhouette_edge =  (adjacent_face_visible && !face_visible) ||
                              (face_visible && !adjacent_face_visible);
            is_visible_edge = is_silhouette_edge || (face_visible && adjacent_face_visible);

            edges.push_back( Model_edge( (vertices[j])(0), (vertices[j])(1),
                (vertices[0])(0), (vertices[0])(1), is_silhouette_edge, is_visible_edge, false ) );
        }
    }
}

void kjb::prepare_model_map(Int_matrix & model_map, const std::vector<const Polymesh *> & ps)
{
    for(unsigned int i = 0; i < ps.size(); i++)
    {
        ps[i]->wire_occlude_render();
    }
    unsigned int id = 0;
    for(unsigned int i = 0; i < ps.size(); i++)
    {
        id = ps[i]->wire_render_with_sequential_ids(id + 1);
    }
    Base_gl_interface::capture_gl_view(model_map);
    /*kjb_c::KJB_image * cap = 0;
    Base_gl_interface::capture_gl_view(&cap);
    kjb::Image img(cap);
    img *= 1000000;
    img.write("~/src/kjb/room/modelmap.jpg");*/

}

void kjb::prepare_model_edges(std::vector<Model_edge> & edges, const std::vector<const Polymesh *> & ps, const Base_gl_interface & eye)
{
    for(unsigned int i = 0; i < ps.size(); i++)
    {
        prepare_model_edges(edges, *(ps[i]),eye);
    }
}

void kjb::prepare_model_edges
(
    std::vector<Model_edge> & edges,
    const std::vector<const Polymesh *> & ps,
    const Base_gl_interface & eye,
    const Matrix & M,
    double width,
    double height,
    const std::vector<bool> & flagged
)
{
    for(unsigned int i = 0; i < ps.size(); i++)
    {
        prepare_model_edges(edges, *(ps[i]), eye, M, width, height, flagged[i]);
    }
}

void kjb::prepare_model_edges
(
    std::vector<Model_edge> & edges,
    const std::vector<const Polymesh *> & ps,
    const Base_gl_interface & eye,
    const Matrix & M,
    double width,
    double height
)
{
    for(unsigned int i = 0; i < ps.size(); i++)
    {
        prepare_model_edges(edges, *(ps[i]), eye, M, width, height);
    }
}

void kjb::prepare_solid_model_map(Int_matrix & model_map, const Polymesh & p)
{
    p.solid_render_with_sequential_ids(1);
    Base_gl_interface::capture_gl_view(model_map);
}

void kjb::prepare_solid_model_map(Int_matrix & model_map, const std::vector<const Polymesh *> & ps)
{
    unsigned int id = 0;
    for(unsigned int i = 0; i < ps.size(); i++)
    {
        id = ps[i]->solid_render_with_sequential_ids(id + 1);
    }
    Base_gl_interface::capture_gl_view(model_map);
    model_map /= 16777216;
}

void kjb::draw_model_edges(kjb::Image & img, const std::vector<Model_edge> & edges)
{
    for(unsigned int i = 0; i < edges.size(); i++)
    {
        edges[i].draw(img, 255.0, 0.0, 0.0);
    }
}

/**
 * @brief Prepares a set of renderend model edges from the model_map. 
 * TODO: here, we assume each rendered model is a silhoutte edge, which is not true. Will think a better way to determine 
 * the silhouette after CVPR. 
 */
void kjb::prepare_rendered_model_edges(std::vector<Model_edge> & model_edges, const Int_matrix & model_map)
{
    model_edges.clear();

    //find the number of model edges in the model map 
    int num_edges = 0;
    for (int i = 0; i < model_map.get_num_rows(); i++)
    {
        for (int j = 0; j < model_map.get_num_cols(); j++)
        {
            if(model_map(i, j) > num_edges)
                num_edges = model_map(i, j);
        }
    }

    std::vector<std::vector<Vector> > vertices; 
    vertices.resize(num_edges);
    model_edges.reserve(num_edges);

    for(int i = 0; i < model_map.get_num_rows(); i++) 
    {
        for (int j = 0; j < model_map.get_num_cols(); j++)
        {
            unsigned int edge_id = model_map(i,j);
            if(edge_id >= 1) 
            {
                Vector p(3);
                p(0) = j; 
                p(1) = i;
                p(2) = 1.0;
                vertices[edge_id-1].push_back(p);
            }
        }
    }

    //construct the model edge set
//    std::vector<std::vector<Vector> >::iterator iter;
    for(unsigned int i = 0; i < vertices.size(); i++)
    {
        if(vertices[i].size() > 1)
        {
            if(vertices[i].front()[0] > vertices[i].back()[0] && vertices[i][1][1] <= vertices[i][0][1]) 
                std::sort(vertices[i].begin(), vertices[i].end(), compare_point_x_location());   
            Vector start = vertices[i].front();
            Vector end = vertices[i].back();
            model_edges.push_back(Model_edge(start[0], start[1], end[0], end[1], true, true));
        } 
    }

}

