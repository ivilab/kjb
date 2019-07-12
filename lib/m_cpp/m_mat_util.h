/* $Id: m_mat_util.h 21596 2017-07-30 23:33:36Z kobus $ */
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

#ifndef KJB_M_CPP_M_MAT_UTIL_H
#define KJB_M_CPP_M_MAT_UTIL_H

#include "m_cpp/m_matrix.h"
#include "l_cpp/l_int_matrix.h"
#include "m_cpp/m_vector.h"
#include "l_cpp/l_int_vector.h"

#include <iterator>
#include <boost/type_traits.hpp>
#include <boost/concept_check.hpp>
#include <boost/multi_array.hpp>

/**
 * @file  General-purpose matrix utility functions.
 * @note some of these functions exist here because they rely on kjb:Vector being 
 *       defined, and Vector is an incomplete type in m_matrix.h
 */

namespace kjb
{

/**
 * @addtogroup kjbLinearAlgebra
 * @{
 */

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

template<class value_type>
struct Matrix_traits
{};

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

template<>
struct Matrix_traits<int>
{
    typedef kjb::Int_matrix Matrix_type;
};

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

template<>
struct Matrix_traits<double>
{
    typedef kjb::Matrix Matrix_type;
};

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief Build a matrix from a container of row vectors.
 */
template <class Iterator>
typename Matrix_traits<typename std::iterator_traits<Iterator>::value_type::value_type>::Matrix_type
create_matrix_from_rows(Iterator begin, Iterator end)
{
    using namespace boost;
    typedef typename std::iterator_traits<Iterator>::value_type Vector_type;
    typedef typename Vector_type::value_type Value_type;
    typedef typename Matrix_traits<Value_type>::Matrix_type Matrix_type;

    Matrix_type result = create_matrix_from_columns(begin, end);

    return matrix_transpose(result);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief Build a matrix from a container of column vectors.
 */
template <class Iterator>
typename Matrix_traits<typename std::iterator_traits<Iterator>::value_type::value_type>::Matrix_type
create_matrix_from_columns(const Iterator& begin, const Iterator& end)
{
    using namespace boost;
    typedef typename std::iterator_traits<Iterator>::value_type Vector_type;
    typedef typename Vector_type::value_type Value_type;
    typedef typename Matrix_traits<Value_type>::Matrix_type Matrix_type;
    typedef typename Vector_type::size_type Size_type;

    BOOST_CONCEPT_ASSERT((kjb::SimpleVector<Vector_type>));

    if ( begin == end  )
    {
        return Matrix_type();
    }

    const Size_type NUM_COLS = std::distance(begin, end);
    const Size_type NUM_ROWS = (*begin).size();

    for( Iterator it = begin; it != end; ++it)
    {
        const Vector_type& column = *it;
        if ( column.size() != NUM_ROWS )
        {
            KJB_THROW_2( Dimension_mismatch,
                                        "Input vectors differ in length" );
        }
    }

    if ( 0 == NUM_ROWS )
    {
        return Matrix_type();
    }

    Matrix_type result( NUM_ROWS, NUM_COLS );
    
    int col = 0;
    for( Iterator it = begin; it != end; ++it )
    {
        result.set_col( col, it->begin(), it->end());
        col++;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 * @brief Build a matrix from a container of column vectors.
 */
template <class Container_>
inline 
typename Matrix_traits<typename Container_::value_type::value_type>::Matrix_type
create_matrix_from_columns( const Container_& cols )
{
    using namespace ::boost;
    BOOST_CONCEPT_ASSERT((Container<Container_>));
    return create_matrix_from_columns(cols.begin(), cols.end());
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 * @brief Build a matrix from a container of row vectors.
 */
template <class Container_>
inline 
typename Matrix_traits<typename Container_::value_type::value_type>::Matrix_type
create_matrix_from_rows( const Container_& rows )
{
    using namespace ::boost;
    BOOST_CONCEPT_ASSERT((Container<Container_>));
    typedef typename Matrix_traits<typename Container_::value_type::value_type>::Matrix_type Matrix_type;\

    Matrix_type result = create_matrix_from_columns(rows);
    return matrix_transpose(result);
}


typedef boost::multi_array_ref<double, 2> Matrix_stl_view;
/**
 * Get view of the matrix that has a similar interface to an stl-style std::vector<std::vector<double> >.
 * When subscripting, inidices are specified using subscript operators, not the parentheses operator; i.e. you can index using m[1][2] instead of m(1,2);
 * This is a lightweight operation: no copy is made, only metadata is allocated; Naturally, caller is responsible for ensuring that the returned
 * view is outlived by this Matrix.
 */
Matrix_stl_view get_matrix_stl_view(kjb::Matrix& mat);

/** @} */

} // namespace kjb

#endif
