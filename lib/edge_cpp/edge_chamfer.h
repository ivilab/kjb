/* $Id: edge_chamfer.h 21669 2017-08-05 19:51:00Z kobus $ */
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

#ifndef KJB_CHAMFER_CPP
#define KJB_CHAMFER_CPP

#include <boost/shared_ptr.hpp>
#include <g/g_chamfer.h>
#include <edge_cpp/edge.h>
#include <m_cpp/m_matrix.h>
#include <vector>
#include <string>

#include <l_cpp/l_serialization.h>
#include <l_cpp/l_int_matrix.h>

#ifdef KJB_HAVE_BST_SERIAL
#include <boost/serialization/access.hpp>
#endif

namespace kjb
{

class Chamfer_transform
{
    /*
     * TODO:
     *    Make it possible to use existing framebuffer objects in this (shared_pointers?)
     *    If reduce_module is already loaded before declaring this object, don't re-load it (singleton object).
     */
#ifdef KJB_HAVE_BST_SERIAL
    friend class boost::serialization::access;
#endif

    typedef Chamfer_transform Self;
public:
    Chamfer_transform() :
        m_size(0),
        m_edges(),
        m_num_rows(0),
        m_num_cols(0),
        m_distances(),
        m_edge_map(0)
    {}

    Chamfer_transform(const Edge_set_ptr edges, int size = 3) :
        m_size(size),
        m_edges(edges),
        m_num_rows(edges->num_rows()),
        m_num_cols(edges->num_cols()),
        m_distances(), // this might not be necessary and could be made switchable
        m_edge_map(0)
    {
        kjb_c::Matrix* c_distances = 0;
        kjb_c::Edge_point*** c_edge_map = NULL;

        chamfer_transform_2(
                edges->c_ptr(),
                edges->num_rows(),
                edges->num_cols(),
                size,
                &c_distances,
                &c_edge_map);

        m_distances = Matrix(c_distances);

        // CONVERT DOUBLE-POINTER TO VECTOR OF VECTORS

        // create array of rows
        m_edge_map = std::vector<std::vector<const kjb_c::Edge_point*> >(m_num_rows);
        for(int row = 0; row < m_num_rows; row++)
        {
            // create array of columns
            m_edge_map[row] = std::vector<const kjb_c::Edge_point*>(m_num_cols);
            // populate column
            for(int col = 0; col < m_num_cols; col++)
            {
                m_edge_map[row][col] = c_edge_map[row][col];
            }
        }

        // free c-style 2D array.
        kjb_c::free_2D_ptr_array((void***) c_edge_map);
    }

    Chamfer_transform(const std::string& fname) :
        m_size(0),
        m_edges(),
        m_num_rows(0),
        m_num_cols(0),
        m_distances(),
        m_edge_map(0)
    {
        // use serialization framework to load from file
        load(*this, fname);
    }

    Chamfer_transform(const Self& other);

    ~Chamfer_transform()
    {
    }

    Self& operator=(const Self& other)
    {
        Self tmp(other);
        swap(tmp);
        return *this;
    }
    
    void swap(Self& other)
    {
        using std::swap;

        swap(m_size, other.m_size);
        swap(m_edges, other.m_edges);
        swap(m_num_rows, other.m_num_rows);
        swap(m_num_cols, other.m_num_cols);
        m_distances.swap(other.m_distances);
        m_edge_map.swap(other.m_edge_map);
    }

    const kjb_c::Edge_point& nearest_edge(int row, int col)
    {
        assert(row >= 0);
        assert(row < m_num_rows);
        assert(col >= 0);
        assert(col < m_num_cols);

        return *m_edge_map[row][col];
    }

    double nearest_distance(int row, int col)
    {
        assert(row >= 0);
        assert(row < m_num_rows);
        assert(col >= 0);
        assert(col < m_num_cols);

        return m_distances(row, col);
    }

    int get_num_rows() const { return m_num_rows; }
    int get_num_cols() const { return m_num_cols; }

    const std::vector<std::vector<const kjb_c::Edge_point*> >& edge_map() const { return m_edge_map; }
    const Matrix& distance_map() const { return m_distances; }

    /**
     * This will return two int matrices containing the row and column
     * location of the corresponding points. This data isn't stored 
     * internally, so the result is re-constructed on every call. 
     */
    std::vector<Int_matrix> position_map() const
    {
        std::vector<Int_matrix> result(2);
        position_map(result[0], result[1]);
        return result;
    }

    /**
     * This will return two int matrices containing the row and column
     * location of the corresponding points. This data isn't stored 
     * internally, so the result is re-constructed on every call. 
     */
    void position_map(Int_matrix& row_positions, Int_matrix& col_positions) const
    {
        row_positions = col_positions = Int_matrix(m_num_rows, m_num_cols);

        for(int row = 0; row < m_num_rows; row++)
        for(int col = 0; col < m_num_cols; col++)
        {
            row_positions(row, col) = (*m_edge_map[row][col]).row;
            col_positions(row, col) = (*m_edge_map[row][col]).col;
        }
    }

    size_t get_num_points() const
    {
        return m_edges->get_total_edge_points();
    }
private:
    int m_size;

    // these edges may be owned by a number of other objects,
    // and we don't want to rely on the fact that they won't be 
    // freed while this object exists.
    Edge_set_ptr m_edges;

    int m_num_rows;
    int m_num_cols;

    Matrix m_distances;
    std::vector<std::vector<const kjb_c::Edge_point*> > m_edge_map;

#ifdef KJB_HAVE_BST_SERIAL
    template <class Archive>
    void serialize(Archive& /* ar */, const unsigned int /* version */)
    {
        KJB_THROW(Not_implemented);
//        ar & m_size;
//        ar & m_edges;
//        ar & m_num_rows;
//        ar & m_num_cols;
//
//        ar & m_distances;
//        ar & m_edge_map;
    }
#endif
};

typedef boost::shared_ptr<Chamfer_transform> Chamfer_transform_ptr;

inline void swap(Chamfer_transform& op1, Chamfer_transform& op2)
{
    op1.swap(op2);
}

}
#endif
