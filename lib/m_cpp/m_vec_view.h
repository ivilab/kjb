/* $Id: m_vec_view.h 21596 2017-07-30 23:33:36Z kobus $ */
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

#ifndef KJB_M_CPP_VEC_VIEW
#define KJB_M_CPP_VEC_VIEW

#include "l_cpp/l_exception.h"
#include "l_cpp/l_index.h"

namespace kjb
{

/**
 * @addtogroup kjbLinearAlgebra
 * @{
 */

/**
 * Represents a filtered "view" of another vector.  This object
 * has the same semantics as a real vector, but altering it will
 * alter the "visible" elements of the underlying vector.  Here's 
 * an example
 *
 *   Vector vec(5, 1.0);
 *   Index_range I(2,3);
 *   Vector_view vec_view = vec(I);
 *   vec_view *= -1;
 *   assert(vec[0]) ==  1);
 *   assert(vec[1]) ==  1);
 *   assert(vec[2]) == -1);
 *   assert(vec[3]) == -1);
 *   assert(vec[4]) == 1);
 *
 * Matlab-style indexing is possible, by leveraging the implicit conversion
 * constructor from string to Index_range:
 *  
 *  vec("2:3") *= -1;
 */
template <class Vector_type>
class Generic_vector_view
{
public:
    typedef Generic_vector_view Self;
    typedef typename Vector_type::value_type Value_type;
    typedef int Size_type;

    Generic_vector_view(
            Vector_type& vec,
            const Index_range& indices) :
        indices_(indices),
        vec_(vec)
    {
    }

    Generic_vector_view(Vector_type& vec) :
        indices_(true), // "all" type range
        vec_(vec)
    {
    }

    Generic_vector_view(const Generic_vector_view& src) :
        indices_(src.indices_), // "all" type range
        vec_(src.vec_)
    {
    }

    // ----------------------
    // Vector-LIKE INTERFACE:
    // ----------------------

    bool operator==(const Self& other)
    {
        return equality(other);
    }

    bool operator==(const Vector_type& other)
    {
        return equality(other);
    }

    bool operator!=(const Self& other)
    {
        return !equality(other);
    }

    bool operator!=(const Vector_type& other)
    {
        return !equality(other);
    }

    /**
     * @warning Bounds-checking is enabled, so expect a performance hit
     */
    Value_type& operator[](int i)
    { 
        int idx = indices_[i];
        return vec_.at(idx);
    }

    /**
     * @warning Bounds-checking is enabled, so expect a performance hit
     */
    Value_type  operator[](int i) const 
    { 
        int idx = indices_[i];
        return vec_.at(idx); 
    }

    Size_type size() const
    {
        // "all"-type ranges can't know their size, so we provide it
        if(indices_.all()) return vec_.size();
        return indices_.size();
    }

    Self& operator=(const Vector_type& vec)
    {
        if(&vec_ == &vec)
        {
            // avoid modifying what we're reading from
            // copy first, then assign
            return assign(Vector_type(vec));
        }

        return assign(vec);
    }

    Self& operator=(const Self& vec)
    {
        // do they refer to the same vector?
        if(&vec_ == &vec.vec_)
        {
            // avoid modifying what we're reading from
            // copy first, then assign
            return assign(Vector_type(vec));
        }
        return assign(vec);
    }
 
    /**
     * Set all elements to s
     */
    Self& operator=(const Value_type& s)
    {
        const int n = size();

        for(int i = 0; i < n; i++)
        {
            operator[](i) = s;
        }

        return *this;
    }

    /**
     * If this object has only 1 column or 1 row, set it equal to 
     * v or v', respectively.
     */
    Self& operator=(const std::vector<double>& v)
    {
        return assign(v);
    }

    /**
     * If this object has only 1 column or 1 row, set it equal to 
     * v or v', respectively.
     */
    Self& operator=(const std::vector<float>& v)
    {
        return assign(v);
    }

    Self& operator*=(Value_type s)
    {
        const int N = size();
        for(int i = 0; i < N ; i++)
            operator[](i) *= s;

        return *this;
    }

    /*
     * This doesn't make sense, since the output dimension could be different from input dimension
    Self& operator*=(const Matrix_view& s)
    {
    }
     */

    double dot(const Self& s)
    {
        // do they refer to the same vector?
        if(&vec_ == &s.vec_)
        {
            // avoid modifying what we're reading from
            // copy first, then assign
            return dot_with_(Vector_type(s));
        }
        return dot_with_(s);
    }

    double dot(const Vector_type& s)
    {
        // do they refer to the same vector?
        if(&vec_ == &s)
        {
            // avoid modifying what we're reading from
            // copy first, then assign
            return dot_with_(Vector_type(s));
        }
        return dot_with_(s);
    }

    Self& operator/=(Value_type s)
    {
        const int N = size();
        for(int i = 0; i < N ; i++)
            operator[](i) /= s;

        return *this;
    }

    Self& operator+=(const Self& s)
    {
        // do they refer to the same vector?
        if(&vec_ == &s.vec_)
        {
            // avoid modifying what we're reading from
            // copy first, then assign
            return plus_equals(Vector_type(s));
        }
        return plus_equals(s);
    }

    Self& operator+=(const Vector_type& s)
    {
        // do they refer to the same vector?
        if(&vec_ == &s)
        {
            // avoid modifying what we're reading from
            // copy first, then assign
            return plus_equals(Vector_type(s));
        }

        return plus_equals(s);
    }

    Self& operator-=(const Self& s)
    {
        // do they refer to the same vector?
        if(&vec_ == &s.vec_)
        {
            // avoid modifying what we're reading from
            // copy first, then assign
            return minus_equals(Vector_type(s));
        }
        return minus_equals(s);
    }

    Self& operator-=(const Vector_type& s)
    {
        return minus_equals(s);
    }
protected:
    /**
     * generic version of operator=().
     * Made private to avoid unwanted template instantiation.
     */
    template <class Generic_vector>
    bool equality(const Generic_vector& mat)
    {
        const int N = size();
        if(mat.size () != N)
            KJB_THROW(Dimension_mismatch);

        for(int i = 0; i < N; i++)
        {
            if(operator[](i) != mat[i])
            {
                return false;
            }
        }

        return true;
    }

    /**
     * generic version of operator=().
     * Made private to avoid unwanted template instantiation.
     */
    template <class Generic_vector>
    Self& assign(const Generic_vector& vec)
    {
        const int N = size();
        if(vec.size() != N)
        {
            KJB_THROW(Dimension_mismatch);
        }

        for(int i = 0; i < N; i++)
        {
            operator[](i) = vec[i];
        }

        return *this;
    }

    /**
     * generic version of operator+=.
     * Made private to avoid unwanted template instantiation
     */
    template <class Generic_vector>
    Self& plus_equals(const Generic_vector& vec)
    {
        const int N = size();
        if(vec.size() != N)
        {
            KJB_THROW(Dimension_mismatch);
        }

        for(int i = 0; i < N; i++)
        {
            operator[](i) += vec[i];
        }

        return *this;

    }

    /**
     * generic version of dot().
     * Made private to avoid unwanted template instantiation
     */
    template <class Generic_vector>
    double dot_with_(const Generic_vector& vec)
    {
        const int N = size();
        if(vec.size() != N)
        {
            KJB_THROW(Dimension_mismatch);
        }

        double result = 0;
        for(int i = 0; i < N; i++)
        {
            result += operator[](i) * vec[i];
        }

        return result;
    }

    /**
     * generic version of operator-=().
     * Made private to avoid unwanted template instantiation
     */
    template <class Generic_vector>
    Self& minus_equals(const Generic_vector& vec)
    {
        const int N = size();
        if(vec.size() != N)
        {
            KJB_THROW(Dimension_mismatch);
        }

        for(int i = 0; i < N; i++)
        {
            operator[](i) -= vec[i];
        }

        return *this;

    }
protected:
    Index_range indices_;
    Vector_type& vec_;

};

// "template typedef"
// see: http://www.gotw.ca/gotw/079.htm
template <class Vector_type>
struct Generic_const_vector_view
{
    typedef const Generic_vector_view<const Vector_type> Type;
};

/** @} */

} // namespace kjb

#endif
