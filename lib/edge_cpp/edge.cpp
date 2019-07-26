/* $Id: edge.cpp 11995 2012-03-29 20:19:58Z ksimek $ */
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
   |  Author:  Kyle Simek
 * =========================================================================== */

#include <edge_cpp/edge.h>

namespace kjb
{

/**For debug purposes, checks that all the c++ wrappers are
 * consistent with the underlying c structures
 */
bool kjb::Edge_set::is_edge_set_consistenct()
{

    double small_epsilon = 1e-80;

    for(unsigned int i = 0; i < m_edge_set->num_edges; i++)
    {
         Edge edge = get_edge(i);
         kjb_c::Edge c_edge = m_edge_set->edges[i];
         if(edge.get_num_points() != c_edge.num_points)
         {
             return false;
         }

         for(unsigned int j = 0; j < c_edge.num_points; j++)
         {
               Edge_point ept = edge.get_edge_point(j);
               kjb_c::Edge_point c_ept = c_edge.points[j];
               if(ept.get_col() != c_ept.col)
               {
                   return false;
               }
               if(ept.get_row() != c_ept.row)
               {
                   return false;
               }
               if(fabs(ept.get_dcol() - c_ept.dcol ) > small_epsilon )
               {
                   return false;
               }
               if(fabs(ept.get_drow() - c_ept.drow ) > small_epsilon )
               {
                   return false;
               }
               if(fabs(ept.get_gradient_magnitude() - c_ept.mag ) > small_epsilon )
               {
                   return false;
               }
           }
    }

    return true;
}

Image edges_to_image(const Edge_set& edges, bool invert, size_t remove_borders)
{
    float edge_color = (invert ? 255.0 : 0.0);
    float bg_color = (invert ? 0.0 : 255.0);
    Image result = Image::create_initialized_image(edges.num_rows(), edges.num_cols(), 
            bg_color, bg_color, bg_color);
    edges.draw(result, edge_color, edge_color, edge_color);

    if(remove_borders > 0)
    {
        std::vector<size_t> rows;
        for(size_t row = 0; row < remove_borders; ++row)
            rows.push_back(row);
        for(size_t row = 0; row < remove_borders; ++row)
            rows.push_back(result.get_num_rows() - remove_borders + row);

        std::vector<size_t> cols;
        for(size_t col = 0; col < remove_borders; ++col)
            cols.push_back(col);
        for(size_t col = 0; col < remove_borders; ++col)
            cols.push_back(result.get_num_cols() - remove_borders + col);

        for(size_t i = 0; i < rows.size(); ++i)
        for(size_t j = 0; j < result.get_num_cols(); ++j)
        for(size_t c = 0; c < 3; ++c)
            result(rows[i],j,c) = bg_color;

        for(size_t i = remove_borders; i < result.get_num_rows() - remove_borders; ++i)
        for(size_t j = 0; j < cols.size(); ++j)
        for(size_t c = 0; c < 3; ++c)
            result(i,cols[j],c) = bg_color;
    }
    return result;
}

Edge_set_ptr edge_image_to_edge_points(const Image& i, bool oriented)
{
    if(oriented)
    {
        //TODO: Running edge detection on an image that is already edges 
        //      is very slow and unnecessary, but suffices as a reference
        //      implementation.  A faster implementation would look at 
        //      edges in the neighborhood and fit a line to them .
        return Canny_edge_detector(20, 20, 1.0)(i);
    }

    std::vector<kjb_c::Edge_point> edges;
    edges.reserve(512);

    for(int row = 0; row < i.get_num_rows(); row++)
    {
        for(int col = 0; col < i.get_num_cols(); col++)
        {
            const kjb_c::Pixel& px =  i(row, col);
            if(px.r == 0.0 || px.g == 0.0 || px.g == 0.0)
            {
                kjb_c::Edge_point pt;
                pt.row = row;
                pt.col = col;
                pt.dcol = 0.0;
                pt.drow = 0.0;
                pt.mag = 0.0;
                edges.push_back(pt);
            }

        }
    }

//    boost::shared_ptr<Edge_set> result(new Edge_set());
    boost::shared_ptr<Edge_set> result(new Edge_set(edges, i.get_num_rows(), i.get_num_cols()));
    return result;
}

} // namespace kjb
