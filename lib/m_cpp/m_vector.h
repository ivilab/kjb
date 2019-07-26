/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2007-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Kobus Barnard, Kyle Simek, Andrew Predoehl.                        |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: m_vector.h 21596 2017-07-30 23:33:36Z kobus $ */

#ifndef VECTOR_WRAP_H
#define VECTOR_WRAP_H

/** @file
 *
 * @author Kobus Barnard
 * @author Kyle Simek
 * @author Andrew Predoehl
 * @author Ernesto Brau
 * @author Luca Del Pero
 *
 * @brief Definition for the Vector class, a thin wrapper on the KJB
 *        Vector struct and its related functionality.
 *
 * If you make changes to this file, PLEASE CONSIDER making parallel changes to
 * l_int_vector.h,
 * whose structure closely parallels the structure of this file.
 * Tip:  use vimdiff on both files to show the parallel structure.
 *
 * Although this class has much the same interface as class Int_vector, they
 * are not derived from a common abstract interface, because (1) we want as
 * much speed as possible -- this code should be eligible to put inside a
 * tight inner loop; and (2) I don't know whether that would be useful.
 */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "m/m_vector.h"
#include "m/m_vec_arith.h"
#include "m/m_vec_norm.h"
#include "m/m_vec_stat.h"
#include "m/m_vec_metric.h"
#include "m_cpp/m_serialization.h"
#include "l_cpp/l_exception.h"

#include <algorithm>
#include <iterator>
#include <vector>

#ifdef KJB_HAVE_BST_SERIAL
#include <boost/serialization/access.hpp>
#endif

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

namespace kjb {

class Matrix;
class Vector;
class Int_vector;

class Index_range;

template <class V>
class Generic_vector_view;

typedef Generic_vector_view<Vector> Vector_view;
typedef const Generic_vector_view<const Vector> Const_vector_view;

template <class T>
class Generic_matrix_view;

/**
 * @addtogroup kjbLinearAlgebra
 * @{
 */

/**
 * @brief This class implements vectors, in the linear-algebra sense,
 *        with real-valued elements.
 *
 * For better maintainability, refer to the element type using this class's
 * Value_type typedef, instead of referring to 'double' directly.
 *
 * Most methods of this class are implemented in the C language portion of the
 * KJB library, with this class forming a thin (usually inlined) layer.
 */
class Vector
{
#ifdef KJB_HAVE_BST_SERIAL
    friend class boost::serialization::access;
#endif

public:


    /* Class users:  to make your code as maintanable as possible, use the
     * *_type typedefs below instead of directly referencing the specific
     * type.
     */

    typedef double          Value_type; ///< data type of the elements
    typedef kjb_c::Vector   Impl_type;  ///< the underlying implementation
    typedef Matrix          Mat_type;   ///< the associated matrix type
    typedef Generic_matrix_view<Matrix> Mat_view_type;   ///< view type of the associated matrix type
    typedef Vector          Vec_type;   ///< the associated vector type
    typedef Value_type (*Mapper)(Value_type);   ///< element transformer fun

    // Below are typdefs required to adhere to the stl Container concept
    typedef double          value_type;
    typedef double*         pointer;
    typedef const double*   const_pointer;
    typedef double&         reference;
    typedef double          const_reference;
//    typedef __gnu_cxx::__normal_iterator<value_type*, Vector> iterator;
//    typedef __gnu_cxx::__normal_iterator<const value_type*, Vector> 
//        const_iterator;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;
    typedef std::reverse_iterator<iterator>       reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator; ///< const Iterator type
    typedef int             size_type;
    typedef int             difference_type;


private:
    Impl_type* m_vector;

    static const char* const BAD_SEGMENT;

    /**
     * @brief   Generate debug message for the Index_out_of_bounds exception.
     * @throws  Index_out_of_bounds
     * @pre     The index value has been discovered to be a bad value.
     */
    void throw_bad_bounds( int ) const;

    /**
     * @brief Increase capacity to at least c. 
     *
     * Guaranteed to run in amortized constant time when 
     * called with parameters 1, 2, 3, ...
     */
    void m_ensure_capacity(size_type c);

    /**
     * @brief Template specialization for "Range constructor" dispatch.
     *
     * This is used when it is unknown how large the range is.  
     * If the range has N elements, roughly log N allocations will occur.
     */
    template<typename InputIterator_>
    void m_initialize_dispatch(InputIterator_ begin_,
                InputIterator_ end_, std::input_iterator_tag);

    /**
     * @brief Template specialization for "Range constructor" dispatch.
     *
     * This is used when the range size is available.  
     * Only one allocation occurs with this version.
     */
    template<typename ForwardIterator_>
    void m_initialize_dispatch(ForwardIterator_ begin_, ForwardIterator_ end_, std::forward_iterator_tag)
    {
        const size_type n = std::distance(begin_, end_);
        m_ensure_capacity(n);
        
        m_vector->length = n;

        std::copy(begin_, end_, this->begin());
    }
public:

    /* -------------------------------------------------------------------
     * CONSTRUCTORS
     * ------------------------------------------------------------------- */

    /**
     * @brief   Allocate vector of given length; contents are uninitialized.
     *
     * This also serves as a default ctor
     */
    explicit Vector(int length = 0)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::get_target_vector(&m_vector, length) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Allocate vector of given length; contents are uninitialized.
     * @warning Native type for length is int, not unsigned.
     *
     * This ctor is almost mandatory since we want to construct from a size_t,
     * which on most platforms is unsigned int or unsigned long int.
     */
    explicit Vector(unsigned length)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::get_target_vector(&m_vector, static_cast<int>(length)) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Allocate vector of given length; contents are uninitialized.
     * @warning Native type for length is int, not unsigned long.
     *
     * This ctor is almost mandatory since we want to construct from a size_t,
     * which on most platforms is unsigned int or unsigned long int.
     */
    explicit Vector(unsigned long length)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::get_target_vector(&m_vector, static_cast<int>(length)) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Allocate vector of given length; initialize all elts to 'num'.
     */
    Vector(int length, Value_type num)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::get_initialized_vector(&m_vector, length, num) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Initialize from an array of doubles, of given length.
     */
    Vector(int length, const Value_type* data)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::get_target_vector(&m_vector, length) );
        std::copy( data, data + length, m_vector -> elements );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Ctor builds a vector from one with integer values.
     */
    Vector( const Int_vector& );

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Initialize from an array of floats, of given length.
     */
    Vector(int length, const float* data)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::get_target_vector(&m_vector, length) );
        std::copy( data, data + length, m_vector -> elements );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Conversion ctor:  claim ownership of an existing vector
     *          pointer (i.e., make a shallow copy).
     *
     * This method is the proper way to say, ''Here is a kjb_c::Vector
     * struct that I am responsible for deleting, and I must make sure that
     * it gets destroyed when it goes out of scope.''  This is a good way
     * to wrap a vector "dynamically," after it has already been created.
     *
     * Anyplace you find yourself using free_vector() in your C++ code,
     * you should consider using instead this Vector class with this ctor.
     *
     * If the input pointer equals NULL then a zero length vector is
     * constructed.
     *
     * @warning Don't create two Vector objects from the same source this
     *          way or you'll get a double deletion bug.
     */
    explicit Vector(Impl_type* vec_ptr)
        : m_vector( vec_ptr )
    {
        if ( 0 == vec_ptr )
        {
            // Test program was HERE.
            ETX( kjb_c::get_target_vector(&m_vector, 0) );
        }
        //else
        //{
        //    // Test program was HERE.
        //}
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Conversion ctor: Create from stl-style vector
     *
     */
    Vector(const std::vector<double>& src);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Create a single-element vector.
     */
    explicit Vector(Value_type val)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::get_initialized_vector(&m_vector, 1, val) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /** 
     * @brief Create a vector from a view of another vector 
     */
    Vector (const Vector_view& view);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /** 
     * @brief Create a vector from a view of another vector 
     */
    Vector (const Const_vector_view& view);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Create a two-vector.
     */
    Vector(Value_type val1, Value_type val2)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::get_target_vector(&m_vector, 2) );
        m_vector->elements[0] = val1;
        m_vector->elements[1] = val2;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Create a three-vector.
     */
    Vector(Value_type val1, Value_type val2, Value_type val3)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::get_target_vector(&m_vector, 3) );
        m_vector->elements[0] = val1;
        m_vector->elements[1] = val2;
        m_vector->elements[2] = val3;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Create a four-vector.
     */
    Vector(Value_type val1, Value_type val2, Value_type val3, Value_type val4)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::get_target_vector(&m_vector, 4) );
        m_vector->elements[0] = val1;
        m_vector->elements[1] = val2;
        m_vector->elements[2] = val3;
        m_vector->elements[3] = val4;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Ctor copies contents (i.e., deep copy) of an existing vector.
     *
     * @warning This method should be seldom used:  kjb_c::Vector objects
     *          should rarely be left in an unwrapped state.
     *
     * This kind of conversion is relatively expensive, thus we restrict its
     * use only to explicit invocation.
     */
    explicit Vector(const Impl_type& vec_ref)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::copy_vector(&m_vector, &vec_ref) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Ctor builds from a one-row or one-column Matrix (making
     *          a deep copy).
     */
    explicit Vector(const Mat_type& src);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Ctor builds from a one-row or one-column view of a Matrix (making
     *          a deep copy).
     *
     * This allows syntax like:
     *  Matrix m(100, 100);
     *  Vector v1 = m["1,3,5,7,9", 0];
     *  Vector v2 = m[":", 0];
     */
    explicit Vector(const Mat_view_type& src);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Construct a vector by reading contents from a named file.
     */
    Vector(const std::string& file_name);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Copy ctor -- calls the kjb_c function to copy a vector.
     *
     * Although this method is essential, I hope it is seldom actually used.
     * Most of the time it should be unnecessary or optimized away somehow.
     */
    Vector( const Vector& vec_ref )
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::copy_vector(&m_vector, vec_ref.m_vector) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

#ifdef KJB_HAVE_CXX11
    /**
     * @brief   Move ctor
     */
    Vector(Vector&& vec_ref)
        : m_vector( nullptr )
    {
        m_vector = vec_ref.m_vector;
        vec_ref.m_vector = 0;
    }
#endif /* KJB_HAVE_CXX11 */

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /* ---------------------------------------------------------------------
     * "STL CONSTRUCTORS"
     *
     * These constructors are required for compliance with STL's 
     * "Sequence" concept
     *
     * ------------------------------------------------------------------- */

    /**
     * @brief "Range constructor"
     *
     * @note Template tricks are used to do fast allocation if possible.  
     *   That is, if N = end_ - begin_ is possible, a vector of size N is
     *   allocated at once.  If not, push_back() is called N times, 
     *   requiring log N allocations.
     */
    template<typename InputIterator_>
    Vector(InputIterator_ begin_, InputIterator_ end_)
        : m_vector(0)
    {
        ETX( kjb_c::get_target_vector(&m_vector, 8) );
        m_vector->length = 0;

        typedef typename std::iterator_traits<InputIterator_>::
            iterator_category IterCategory_;
        m_initialize_dispatch(begin_, end_, IterCategory_());
    }


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Destructor -- which just calls the KJB destructor.
     */
    ~Vector()
    {
        // Test program was HERE.
        kjb_c::free_vector(m_vector);
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Return the length of the vector.
     */
    int get_length() const
    {
        // Test program was HERE.
        return m_vector -> length;
    }

    /**
     * @brief  Alias to get_length().  Required to comply with stl Container concept. 
     */
    size_type size() const
    {
        return get_length();
    }

    /**
     * @brief  Maximum size vector can ever have.  Currently defined as INT_MAX. 
     */
    size_type max_size() const
    {
        return INT_MAX;
    }

    /**
     * @brief  Returns true iff size is zero.  Required to comply with stl Container concept. 
     */
    bool empty() const
    {
        return size() == 0;
    }

  // iterators
  /**
   *  Returns a read/write iterator that points to the first
   *  element in the %Vector.  Iteration is done in ordinary
   *  element order.
   */
    iterator begin()
    {
        return iterator(m_vector->elements);
    }

  /**
   *  Returns a read-only (constant) iterator that points to the
   *  first element in the %Vector.  Iteration is done in ordinary
   *  element order.
   */
    const_iterator begin() const
    {
        return const_iterator(m_vector->elements);
    }

  /**
   *  Returns a read/write iterator that points one past the last
   *  element in the %Vector.  Iteration is done in ordinary
   *  element order.
   */
    iterator end()
    {
        return iterator(m_vector->elements + m_vector->length);
    }

  /**
   *  Returns a read-only (constant) iterator that points one past
   *  the last element in the %Vector.  Iteration is done in
   *  ordinary element order.
   */
    const_iterator end() const
    {
        return const_iterator(m_vector->elements + m_vector->length);
    }

  /**
   *  Returns a read/write reverse iterator that points to the
   *  last element in the %Vector.  Iteration is done in reverse
   *  element order.
   */
    reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }

  /**
   *  Returns a read-only (constant) reverse iterator that points
   *  to the last element in the %Vector.  Iteration is done in
   *  reverse element order.
   */
    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }

  /**
   *  Returns a read/write reverse iterator that points
   *  to one before the first element in the %Vector.  Iteration
   *  is done in reverse element order.
   */
    reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

  /**
   *  Returns a read-only (constant) reverse iterator that points
   *  to one before the first element in the %Vector.  Iteration
   *  is done in reverse element order.
   */
    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

  /**
   *  After calling, the vector will be able to contain _capacity_
   *  elements without needing to resize itself.
   */
    void reserve(int capacity)
    {
        m_ensure_capacity(capacity);
    }
    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Clobber current vector; resize and fill with random values.
     * @return  an lvalue to this object
     * @see     kjb_c::kjb_rand()
     *
     * Random values are uniformly distributed between 0 and 1.
     */
    Vector& randomize(int length)
    {
        // Test program was HERE.
        ETX( kjb_c::get_random_vector(&m_vector, length) );
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Clobber current vector contents with random values.
     * @return  an lvalue to this object
     * @see     kjb_c::kjb_rand()
     *
     * Random values are uniformly distributed between 0 and 1.
     */
    Vector& randomize()
    {
        // Test program was HERE.
        return randomize( get_length() );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    ///**
     //* @brief  Clone of randomize(int)
     //* @deprecated The name is misleading -- do not use in new code.
     //*/
    //Vector& init_random( int length )
    //{
        //// Test program was HERE.
        //return randomize( length );
    //}

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Clobber current vector; resize and fill with zeroes.
     * @return  an lvalue to this object
     */
    Vector& zero_out(int length)
    {
        // Test program was HERE.
        ETX( kjb_c::get_zero_vector(&m_vector, length) );
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Clobber current vector contents with zeroes.
     * @return  an lvalue to this object
     */
    Vector& zero_out()
    {
        // Test program was HERE.
        return zero_out( get_length() );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    ///**
     //* @brief  Clone of zero_out(int)
     //* @deprecated The name is misleading -- do not use in new code.
     //*/
    //Vector& init_zero( int length )
    //{
        //// Test program was HERE.
        //return zero_out( length );
    //}

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Convert to a 1-vector and set its value to the argument.
     */
    Vector& set(Value_type val)
    {
        get_target_vector(&m_vector, 1);
        this->operator[](0) = val;

        return *this;
    }

    /**
     * @brief   Convert to a 2-vector and set its values to the arguments.
     */
    Vector& set(Value_type val1, Value_type val2)
    {
        get_target_vector(&m_vector, 2);
        this->operator[](0) = val1;
        this->operator[](1) = val2;

        return *this;
    }

    /**
     * @brief   Convert to a 3-vector and set its values to the arguments.
     */
    Vector& set(Value_type val1, Value_type val2, Value_type val3)
    {
        get_target_vector(&m_vector, 3);
        this->operator[](0) = val1;
        this->operator[](1) = val2;
        this->operator[](2) = val3;

        return *this;
    }

    /**
     * @brief   Convert to a 4-vector and set its values to the arguments.
     */
    Vector& set(Value_type val1, Value_type val2, Value_type val3, Value_type val4)
    {
        get_target_vector(&m_vector, 4);
        this->operator[](0) = val1;
        this->operator[](1) = val2;
        this->operator[](2) = val3;
        this->operator[](3) = val4;

        return *this;
    }

    /* -------------------------------------------------------------------
     * ASSIGNMENT OPERATORS
     * ------------------------------------------------------------------- */

    /**
     * @brief   Assignment operator:  assign from a single-row matrix-view
     */
    Vector& operator=(const Mat_view_type& vec_ref);

    /**
     * @brief   Assignment operator:  assign from a kjb_c::Vector,
     *          a C struct.
     */
    Vector& operator=(const Impl_type& vec_ref)
    {
        if ( m_vector != &vec_ref )
        {
            // Test program was HERE.
            ETX( kjb_c::copy_vector(&m_vector, &vec_ref) );
        }
        //else
        //{
        //    // Test program was HERE.
        //}
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

#ifdef KJB_HAVE_CXX11
    /**
     * @brief   Move assignment
     */
    Vector& operator=(Vector&& other)
    {
        if(this == &other)
            return *this;

        kjb_c::free_vector( m_vector );
        m_vector = other.m_vector;
        other.m_vector = 0;
        return *this;
    }
#endif /* KJB_HAVE_CXX11 */

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Assignment operator:  assign from a kjb::Vector,
     *          a C++ object.
     */
    Vector& operator=(const Vector& src)
    {
        // Test program was HERE.
        // call assignment operator for kjb_c::Vector
        return operator=( *src.m_vector );
    }


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Copy elements from input vector iv into this vector, starting
     *          at some offset, leaving the rest of this vector unchanged.
     *
     * @param   iv      The input vector
     * @param   offset  The index of the first target location of this vector.
     * @param   begin   The index of the first element to copy from iv.
     * @param   length  The number of elements to be copied from iv
     *
     * @return  the lvalue for this vector
     * @throws  Illegal_argument if the source or destination is too small to
     *          perform the specified copy, or if an argument is negative.
     *
     * Just to be absolutely explicit, it works like so, provided all these
     * locations below are valid.
     *
     * | (*this)[ offset              ] = iv[ begin              ];
     * |                                :
     * |                                :
     * | (*this)[ offset + length - 1 ] = iv[ begin + length - 1 ];
     */
    Vector& replace(const Vector& iv, int offset, int begin, int length);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Get const pointer to the underlying kjb_c::Vector C
     *          struct.
     */
    const Impl_type* get_c_vector() const
    {
        // Test program was HERE.
        return m_vector;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Get pointer to the underlying kjb_c::Vector C
     *          struct.
     * @warning:  This should only be used if you know what you're doing.  generally, it should only be used to write wrapper functions for c functions.
     */
    Impl_type*& get_underlying_representation_with_guilt()
    {
        // Test program was HERE.
        return m_vector;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Get pointer to the underlying kjb_c::Vector C
     *          struct.
     * @warning This should NOT be used except inside wrapper functions.  This function breaks encapsulation, and has the potential to completely screw up the C++ matrix object, so be sure you know what you're doing before calling this!!  This method's name is intentionally long and scary to discourage its use outside of wrapper code.
     *
     * @deprecated: don't use in new wrapper code.  For new wrapper code, use
     *          get_underlying_representation_with_guilt() instead.
     */
    Impl_type* get_underlying_representation_unsafe()
    {
        // Test program was HERE.
        return m_vector;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Swap the representations of two vectors.
     */
    void swap( Vector& other )
    {
        // Test program was HERE.
        std::swap( m_vector, other.m_vector );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Resize vector, retaining previous values.
     */
    Vector& resize(int new_length, Value_type pad = Value_type(0) );

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Subscript vector like a C array, e.g., A[10], returning
     *          an lvalue.
     * @warning No bounds checking!
     * @see     at() method, which provides bounds checking.
     */
    Value_type& operator[](int i)
    {
        // Test program was HERE.
        return m_vector -> elements[ i ];
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Subscript vector like a C array, e.g., A[10], returning
     *          an rvalue.
     * @warning No bounds checking!
     * @see     at() method, which provides bounds checking.
     */
    const Value_type& operator[](int i) const
    {
        // Test program was HERE.
        return m_vector -> elements[ i ];
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Fortran-style vector subscript, e.g., A(10), to get an lvalue.
     * @warning No bounds checking!
     * @see     at() method, which provides bounds checking.
     */
    Value_type& operator()(int i)
    {
        // Test program was HERE.
        return operator[]( i );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Fortran-style vector subscript, e.g., A(10), to get an rvalue.
     * @warning No bounds checking!
     * @see     at() method, which provides bounds checking.
     */
    const Value_type& operator()(int i) const
    {
        // Test program was HERE.
        return operator[]( i );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * Access a subvector using Matlab-like syntax.  Returned object has
     * Vector semantics, but operates on the elements of this vector indexed
     * by the idx parameter.
     *
     * @note Accepted input types are int, Index_range.all, string, Int_vector,
     * or vector<int>
     * @warning Vector_view is fairly lightweight, but constructing Vector_view
     * and Index_range adds overhead compared tointeger-based indexing.  In addition,
     * bounds checking occurs at EVERY index, so this shouldn't be used where
     * performance is a concern.
     */
    Vector_view operator[](const Index_range& idx);

    Const_vector_view operator[](const Index_range& idx) const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Test whether a subscript is valid, throw an exception if not.
     * @throws  Index_out_of_bounds with a message, if the index is bad.
     */
    void check_bounds( int i ) const
    {
        // Test program was HERE.
        if ( i < 0 || get_length() <= i )
        {
            throw_bad_bounds( i );
        }
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Safely subscript vector, e.g., A.at(10), to get an lvalue.
     *
     * "Safely" here means we do bounds checking and throw if out of bounds.
     */
    Value_type& at( int i )
    {
        // Test program was HERE.
        check_bounds( i );
        return operator[]( i );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Safely subscript vector, e.g., A.at(10), to get an rvalue.
     *
     * "Safely" here means we do bounds checking and throw if out of bounds.
     */
    const Value_type& at( int i ) const
    {
        // Test program was HERE.
        check_bounds( i );
        return operator[]( i );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief A copy of t is inserted before position
     */
    iterator insert(iterator position, value_type t);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief Insert N copies of t before position.
     */
    void insert(iterator position, size_type N, value_type t);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief Insert range [begin, end) before position.
     */
    template<typename InputIterator>
    void insert(iterator position, InputIterator begin_, InputIterator end_);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief Resizes to N, filling all values with t.
     */
    void assign(size_type N, value_type t);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief Assign this vector to the values in sequence (begin_, and_)
     */
    template<typename InputIterator>
    void assign(InputIterator begin_, InputIterator end_);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief erase element at position and shift all elements after p up by 1.
     * @pre !this->empty()
     * @warning Calling this will never free allocated memory.
     */
    iterator erase(iterator position);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief erase all elements beginning with begin_ up to but not including end_
     *
     * @pre [begin_, end_) is a valid range in *this
     * @warning Calling this will never free allocated memory.
     */
    void erase(iterator begin_, iterator end_);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief delete all elements from vector
     *
     * @warning Calling this will never free allocated memory.
     */
    void clear()
    {
        m_vector->length = 0;
    }


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */
    /**
     * @brief Returns the first element of the vector
     * @pre !this->empty()
     * */
    reference front()
    {
        return operator[](0);
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief Returns the first element of the vector
     * @pre !this->empty()
     * */
    const_reference front() const
    {
        return operator[](0);
    }
    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief Returns the last element of the vector
     * @pre !this->empty()
     * */
    reference back()
    {
        return operator[](size() - 1);
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief Returns the last element of the vector
     * @pre !this->empty()
     * */
    const_reference back() const
    {
        return operator[](size() - 1);
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief inserts an element at the back of the vector in amortized constant time.
     */
    void push_back(Value_type x)
    {
        m_ensure_capacity(m_vector->length + 1);

        m_vector->length++;
        ASSERT(m_vector->length <= m_vector->max_length);
        m_vector->elements[ (m_vector->length - 1) ] = x;
    }

    /**
     * @brief Returns the last element of the vector
     * @pre !this->empty()
     * @warning This does not release any allocated memory.
     * */
    void pop_back()
    {
        m_vector->length--;
    }

    /**
     * @brief   Write vector as a row to a file, or to standard output.
     * @see     kjb_c::write_row_vector_full_precision()
     * @warning This function doesn't handle "empty" vectors correctly.  For this reason, write_col is currently preferred.
     *
     * If filename equals NULL or filename[0] is a null character
     * then the output is directed to standard output.
     */
    void write_row( const char* filename = 0 ) const
    {
        // Test program was HERE.
        ETX(kjb_c::write_row_vector_full_precision( m_vector, filename ));
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Write vector as a column to a file, or to standard output.
     * @see     kjb_c::write_col_vector_with_header()
     *
     * If filename equals NULL or filename[0] is the null character
     * then the output is directed to standard output.
     */
    void write_col( const char* filename = 0 ) const
    {
        // Test program was HERE.
        ETX(kjb_c::write_col_vector_with_header( m_vector, filename ));
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */
    /**
     * @brief   Read vector from a file, or from standard input.
     *
     * If filename equals NULL or filename[0] is the null character
     * then the output is directed to standard output.
     */
    void read( const char* filename = 0 )
    {
        // Test program was HERE.
        ETX(kjb_c::read_vector( &m_vector, filename ));
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */
    /**
     * @brief   Write vector to a file, or to standard output.
     *
     * If filename equals NULL or filename[0] is the null character
     * then the output is directed to standard output.
     *
     * @note    The write() method is required for the KjbReadableWritable concept. @see KjbReadableWritable_concept
     */
    void write( const char* filename = 0 ) const
    {
        // Test program was HERE.
        write_col(filename);
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /* ---------------------------------------------------------------------
     * ARITHMETIC OPERATORS
     *
     * Modifying operators; i.e., operators that modify the object. These
     * are member functions, as per the standard implementation. Operators
     * that do not modify the vector are non-members. Also, their corresponding
     * named methods (subtract, add, etc).
     * ------------------------------------------------------------------- */

    /**
     * @brief   Scalar multiply self, in-place, e.g., v *= 6.
     */
    Vector& operator*= (Value_type op2)
    {
        // Test program was HERE.
        ETX( kjb_c::ow_multiply_vector_by_scalar(m_vector, op2) );
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Scalar multiply self, in-place, just like v *= 6.
     */
    Vector& multiply(Value_type op2)
    {
        // Test program was HERE.
        return operator*=( op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief    Element wise multiplication of two vectors
     */
    Vector& ew_multiply(const Vector & op2)
    {
        ETX( kjb_c::ow_multiply_vectors(m_vector, op2.m_vector) );
        return *this;
    }


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Scalar division of self, in-place.
     */
    Vector& operator/= (Value_type op2)
    {
        // Test program was HERE.
        ETX( kjb_c::ow_divide_vector_by_scalar(m_vector, op2) );
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Scalar division of self, in-place, just like v /= 2.
     */
    Vector& divide (Value_type op2)
    {
        // Test program was HERE.
        return operator/=( op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief    Element wise division of two vectors; result is stored in first
     */
    Vector& ew_divide(const Vector & op2)
    {
        ETX( kjb_c::ow_divide_vectors(m_vector, op2.m_vector) );
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Add vector to self, in-place, e.g., v += delta_v.
     */
    Vector& operator+= (const Vector& op2)
    {
        // Test program was HERE.
        ETX( kjb_c::ow_add_vectors(m_vector, op2.m_vector) );
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Add scalar to self, in-place, e.g., v -= delta_v.
     */
    Vector& operator+= (double x);;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Add vector to self, in-place, just like v += delta_v.
     */
    Vector& add (const Vector& op2)
    {
        // Test program was HERE.
        return operator+=( op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Subtract vector from self, in-place, e.g., v -= delta_v.
     */
    Vector& operator-= (const Vector& op2)
    {
        // Test program was HERE.
        ETX( kjb_c::ow_subtract_vectors(m_vector, op2.m_vector) );
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Subtract scalar from self, in-place, e.g., v -= delta_v.
     */
    Vector& operator-= (double x);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Subtract vector from self, in-place, just like v -= delta_v.
     */
    Vector& subtract (const Vector& op2)
    {
        // Test program was HERE.
        return operator-=( op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Negate self, in-place, just like v *= (-1).
     */
    Vector& negate()
    {
        // Test program was HERE.
        return operator*=( -1 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief Transform the elements of a vector
     *
     * Apply a transformation fun to each element of the vector, in place,
     * and return an lvalue of the modified vector.
     */
    Vec_type& mapcar( Mapper );

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Find minimum element in the vector, both its value and index.
     *
     * This will search for the minimum value in the vector, and return that
     * value.  Also this method can work like an argmin function, and
     * emit the index of a location containing that value.
     *
     * @param[out] min_index   Pointer to a location to store the index of the
     *                     minimum value.  If it equals null, then this method
     *                     behaves the same as min().
     *
     * @throw  Result_error if the vector has size of zero
     *
     * @return The value of the minimum, or ERROR if size is zero.
     *
     * The vector might store the minimum value in more than one location.
     * In that case, the value in *min_index will store one of the locations,
     * but it is undefined which one.
     */
    Value_type min( int* min_index ) const
    {
        if ( 0 == get_length() )
        {
            // Test program was HERE.
            KJB_THROW_2( Result_error,
                                "Vector is empty; min is undefined" );
        }
        if ( 0 == min_index )
        {
            // Test program was HERE.
            //return kjb::min(*this);
            return kjb_c::min_vector_element(m_vector);
        }
        // Test program was HERE.
        Value_type min_val;
        *min_index = kjb_c::get_min_vector_element(m_vector, &min_val);
        return min_val;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Find maximum element in the vector, both its value and index.
     *
     * This will search for the maximum value in the vector, and return that
     * value.  Also this method can work like an argmax function, and
     * emit the index of a location containing that value.
     *
     * @param[out] max_index   Pointer to a location to store the index of the
     *                     maximum value.  If it equals null, then this method
     *                     behaves the same as max().
     *
     * @throw  Result_error if the vector has size of zero
     *
     * @return The value of the maximum
     *
     * The vector might store the maximum value in more than one location.
     * In that case, the value in *max_index will store one of the locations,
     * but it is undefined which one.
     */
    Value_type max( int* max_index ) const
    {
        if ( 0 == get_length() )
        {
            // Test program was HERE.
            KJB_THROW_2( Result_error,
                                "Vector is empty; max is undefined" );
        }
        if ( 0 == max_index )
        {
            // Test program was HERE.
            //return kjb::max(*this);
            return kjb_c::max_vector_element(m_vector);
        }
        // Test program was HERE.
        Value_type max_val;
        *max_index = kjb_c::get_max_vector_element(m_vector, &max_val);
        return max_val;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief returns the sum of the element-wise differences between this vector
     * and the input vector
     *
     * @param op2 the input vector
     */
    double get_max_abs_difference(const Vector & op2) const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */



    /* -------------------------------------------------------------------
     * VECTOR-SPECIFIC METHODS
     * ------------------------------------------------------------------- */

    /**
     * @brief Construct an "equivalent" skew-symmetric matrix from 3-vector.
     *
     * The result is useful for computing a cross product.
     * Only implemented for dimension = 3.  The formula is as follows:
     *
     * <pre>
                            (  0 -c  b )
        Hat of (a b c)' :=  (  c  0 -a )
                            ( -b  a  0 ).</pre>
     */
    kjb::Matrix hat() const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Compute cross product of op1 and op2.
     *
     * Store result in this vector.  Only defined for dimension = 3.
     */
    Vector& cross_with(const Vector& op2);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief Return this vector's magnitude.
     */
    Value_type magnitude() const
    {
        // Test program was HERE.
        return kjb_c::vector_magnitude(m_vector);
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief Return this vector's squared magnitude.
     */
    Value_type magnitude_squared() const
    {
        // Test program was HERE.
        return kjb_c::sum_vector_squared_elements(m_vector);
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief Return this vector's elements sum.
     */
    Value_type sum_vector_elements() const
    {
        // Test program was HERE.
        return kjb_c::sum_vector_elements(m_vector);
    }

    /**
     * @brief   Square each element of the vector in place
     *
     */
    Vector& square_elements()
    {
        ETX(kjb_c::ow_square_vector_elements(m_vector));
        return *this;
    }

    Vector& ew_reciprocal()
    {
        ETX(kjb_c::ow_invert_vector(m_vector));
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief Normalize vector in place by choice of method (default by
     *        magnitude).
     */
    Vector& normalize(kjb_c::Norm_method method =kjb_c::NORMALIZE_BY_MAGNITUDE)
    {
        // Test program was HERE.
        kjb_c::normalize_vector(&m_vector, m_vector, method);
        return *this;
    }

    /**
     * @brief non=mutating version of "normalize()"
     */
    Vector normalized(kjb_c::Norm_method method =kjb_c::NORMALIZE_BY_MAGNITUDE) const
    {
        Vector tmp(*this);
        tmp.normalize(method);
        return tmp;
    }

private:
    template <class View_type>
    void init_from_view_(const View_type& vec_view);

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        return kjb_serialize(ar, *this, version);
    }

};


/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/* ---------------------------------------------------------------------
 * TEMPLATE MEMBER FUNCTIONS
 *
 * Template member function definitions go here.
 * ------------------------------------------------------------------- */

template<typename InputIterator_>
void Vector::m_initialize_dispatch
(
    InputIterator_          begin_,
    InputIterator_          end_,
    std::input_iterator_tag
)
{
    for(; begin_ != end_; ++begin_)
    {
        push_back(*begin_);
    }
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

template<typename InputIterator>
void Vector::insert
(
    iterator        position,
    InputIterator   begin_,
    InputIterator   end_
)
{
    const size_type N = end_ - begin_;
    const size_type offset = position - begin();
    ASSERT(offset <= size());

    m_ensure_capacity(m_vector->length + N);
    position = begin() + offset;

    iterator old_end = this->end();

    m_vector->length += N;

    // corner case: empty vector.  in this case, position=0 is a valid 
    // iterator, but needs to be handled specially.
    if(position == 0 && m_vector->length == N)
    {
        position = m_vector->elements;
    }

    // shift old data
    std::copy(position, old_end, position + N);

    // copy new data
    std::copy(begin_, end_, position);
}

template<typename InputIterator>
void Vector::assign(InputIterator begin_, InputIterator end_)
{
    resize(0);
    insert(begin(), begin_, end_);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

template <class View_type>
void Vector::init_from_view_(const View_type& vec_view)
{
    Vector result(vec_view.size());
    for( int i = 0; i < vec_view.size(); ++i)
    {
        result[i] = vec_view[i];
    }
    swap( result );
}

/* ---------------------------------------------------------------------
 * "NAMED CONSTRUCTORS"
 *
 * The "named constructor idiom" is when we use functions to create
 * objects.  The benefit is that the names indicate the purpose of the
 * method.  The following functions act as named constructors.
 * ------------------------------------------------------------------- */

/**
 * @brief   Construct a vector with values drawn from a uniform distribution over [0,1]
 */
inline
Vector create_random_vector(int length)
{
    // Test program was HERE.
    kjb_c::Vector* result = 0;
    ETX( kjb_c::get_random_vector( &result, length ) );
    return Vector(result);
}


/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Construct a vector with values drawn from a standard Gaussian distribution (mean 0, variance 1);
 */
Vector create_gauss_random_vector(int length);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief       Construct a vector containing zeroes.
 * @deprecated  Better to use Vector(length, 0.0) instead.
 */
inline
Vector create_zero_vector(int length)
{
    // Test program was HERE.
    return Vector(length, 0.0);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Construct a vector by deep-copying a section of another vector.
 * @see     kjb_c::copy_vector_segment()
 *
 * @note The stl-style "fill" constructor is a more generic alternative to this:
 *  Vector v(v2.begin()+start, v2.begin()+start+length)
 */
inline
Vector create_vector_from_vector_section(const Vector& iv, int begin, int length)
{
    // Test program was HERE.
    kjb_c::Vector* outvec = 0;
    ETX_2(kjb_c::copy_vector_segment(&outvec, iv.get_c_vector(), begin, length),
                            "Failure in create_vector_from_vector_section : allocation error or bad indices");
    return Vector(outvec);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/* ---------------------------------------------------------------------
 * ARITHMETIC OPERATORS
 *
 * Non-modifying operators. These were members before, but I think most of us
 * agree that they should be non-members. The only members are the
 * modifying operators, such as +=, *=, /=, etc.
 * ------------------------------------------------------------------- */

/**
 * @brief   Return product of op1 (as a row-vec) times matrix on the
 *          right.
 *
 * Semantics could be expressed as op1^T * M, where
 * M is a matrix with the same number of rows as the length of
 * op1^T.
 *
 *
 */
Vector operator*(const Vector& op1, const Matrix& op2);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Compute product of this matrix (on the left) and a column
 *          vector (on the right).
 * @throws  Dimension_mismatch if the number of columns of this matrix
 *          does not equal the length of right factor op2.
 */
Vector operator*( const Matrix& op1, const Vector& op2 );

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Scalar multiply, resulting in a new vector, e.g., v * 6.
 */
inline
Vector operator*(const Vector& op1, Vector::Value_type op2)
{
    // Test program was HERE.
    return Vector(op1) *= op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Scalar multiplication of a vector
 *
 * This is an inline to allow the scalar to appear on the left side
 * of the operator.
 */
inline
Vector operator*( Vector::Value_type op1, const Vector& op2 )
{
    // Test program was HERE.
    return op2 * op1;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Scalar division, yielding a new vector.
 */
inline
Vector operator/(const Vector& op1, Vector::Value_type op2)
{
    // Test program was HERE.
    return Vector(op1) /= op2;

}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Add two vectors, creating a new vector, e.g., v + w.
 */
inline
Vector operator+ (const Vector& op1, const Vector& op2)
{
    // Test program was HERE.
    return Vector(op1) += op2;
}

/**
 * @brief   Add a scalar to each element of a vector, creating a new vector
 */
inline
Vector operator+ (const Vector& op1, const Vector::Value_type op2)
{
    // Test program was HERE.
    return Vector(op1) += op2;
}
    
/**
 * @brief   Add a scalar to each element of a vector, creating a new vector
 */
inline
Vector operator+ (const Vector::Value_type& op1, const Vector& op2)
{
    // Test program was HERE.
    return Vector(op2) += op1;
}
    
/**
 * @brief   Subtract a scalar from each element of a vector, creating a new vector
 */
inline
Vector operator- (const Vector& op1, const Vector::Value_type op2)
{
    // Test program was HERE.
    return Vector(op1) -= op2;
}
    
/**
 * @brief   Subtract a scalar from each element of a vector, creating a new vector
 */
inline
Vector operator- (const Vector::Value_type& op1, const Vector& op2)
{
    // Test program was HERE.
    return Vector(op2) += op1;
}
    
/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Subtract one vector from another, resulting in a new vector,
 *          e.g., v - w.
 */
inline
Vector operator- (const Vector& op1, const Vector& op2)
{
    // Test program was HERE.
    return Vector(op1) -= op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Return the negation of a vector, as in (-v), resulting in a
 *          new vector.
 */
inline
Vector operator- (const Vector& op1)
{
    // Test program was HERE.
    return op1 * (-1.0);
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/**
 * @brief   Return the vector obtained by squaring the vector elementwise
 */
inline
Vector square_elements(const Vector& v)
{
    return Vector(v).square_elements();
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/**
 * @brief   Return the elementwise multiplicative inverse of a vector
 */
inline
Vector ew_reciprocal(const Vector& v)
{
    return Vector(v).ew_reciprocal();
}
     

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/* ---------------------------------------------------------------------
 * COMPARISON OPERATORS
 *
 * Comparison operators (==, !=).
 * ------------------------------------------------------------------- */

/**
 * @brief   Test for exact equality between vectors.
 * @warning Exact comparison of floating-point values is perilous.
 */
bool operator==(const Vector& op1, const Vector::Impl_type& op2);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Test for exact equality between vectors.
 *
 * This is an inline to allow the KJB C struct appear on the left side
 * of the ==.
 */
inline
bool operator==(const Vector::Impl_type& op1, const Vector& op2)
{
    // Test program was HERE.
    return op2 == op1;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Test for (any) inequality between vectors.
 * @warning Exact comparison of floating-point values is perilous.
 */
inline
bool operator!=(const Vector& op1, const Vector::Impl_type &op2)
{
    // Test program was HERE.
    return !(op1 == op2);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Test for inequality between vectors.
 *
 * This is an inline to allow the KJB C struct appear on the left side
 * of the ==.
 */
inline
bool operator!=(const Vector::Impl_type& op1, const Vector& op2)
{
    // Test program was HERE.
    return op2 != op1;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Test for exact equality between vectors.
 * @warning Exact comparison of floating-point values is perilous.
 */
inline
bool operator==(const Vector& op1, const Vector& op2)
{
    // Test program was HERE.
    return op1 == *op2.get_c_vector();
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Test for (any) inequality between vectors.
 * @warning Exact comparison of floating-point values is perilous.
 */
inline
bool operator!=(const Vector& op1, const Vector &op2)
{
    // Test program was HERE.
    return op1 != *op2.get_c_vector();
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/* ---------------------------------------------------------------------
 * ORDERING OPERATORS
 * These seem silly, but they're required to comply with the 
 * stl "Forward container" concept. [I can easily think of
 * situations where this is necessary. Silly? I think not!]
 * ------------------------------------------------------------------- */

/**
 * @brief   Test weak lexicographic ordering between vectors.
 */
bool operator<(const Vector& op1, const Vector &op2);

/**
 * @brief   Test lexicographic ordering between vectors.
 */
inline
bool operator<=(const Vector& op1, const Vector& op2)
{
    return (op1 < op2) || (op1 == op2);
}

/**
 * @brief   Test lexicographic ordering between vectors.
 */
inline
bool operator>(const Vector& op1, const Vector& op2)
{
    return !(op1 < op2) && !(op1 == op2); 
}

/**
 * @brief   Test lexicographic ordering between vectors.
 */
inline
bool operator>=(const Vector& op1, const Vector& op2)
{
    return !(op1 < op2);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Display vector contents in an ASCII format.
 *
 * This routine is mostly for debugging; consider one of the many
 * KJB output routines for more output in a more standardized form.
 */
std::ostream& operator<<(std::ostream& out, const Vector& m);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Write vector to an output stream so it can be read with read_vector
 *
 * If you aren't using boost serialization, this is the preferred way to
 * serialize a vector for later reading with stream_read_vector.  
 *
 * @warning In general, using operator<< is NOT recommended for serialization, because the vector
 * length is not written
 */
std::ostream& stream_write_vector(std::ostream& ost, const Vector& m);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Read vector from an input stream.
 *
 * Vector is assumed to have been written using stream_write_vector().
 *
 * If you aren't using boost serialization, this is the preferred way to
 * de-serialize a vector.
 */
std::istream& stream_read_vector(std::istream& ist, Vector& m);


/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/* ---------------------------------------------------------------------
 * OTHER STUFF
 * ------------------------------------------------------------------- */

/**
 * @brief   Return the maximum of the absolute values of the elementwise
 *          difference between two vectors, provided they have the same
 *          length.
 * @throws  Dimension_mismatch, if the lengths differ.
 */
inline
Vector::Value_type max_abs_difference( const Vector& op1, const Vector& op2 )
{
    if ( op1.get_length() != op2.get_length() )
    {
        // Test program was HERE.
        KJB_THROW( Dimension_mismatch );
    }
    //else
    //{
    //    // Test program was HERE.
    //}
    return kjb_c::max_abs_vector_difference( op1.get_c_vector(), op2.get_c_vector() );
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief Returns dot product of op1 and op2.
 */
inline
Vector::Value_type dot(const Vector& op1, const Vector& op2)
{
    // Test program was HERE.
    Vector::Value_type result;
    ETX( kjb_c::get_dot_product(op1.get_c_vector(), op2.get_c_vector(), &result) );
    return result;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Compute cross product of op1 and op2.
 *
 * Store result in this vector.  Only defined for dimension = 3.
 */
Vector cross(const Vector& op1, const Vector& op2);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Compute L1-norm of vector
 *
 * This function is required for some KJB algorithms, and is 
 * required to adhere to the ModelParameters concept.
 */
inline
double norm1(const Vector& op)
{
    double total = 0;

    for(Vector::const_iterator it = op.begin(); it != op.end(); ++it)
        total += fabs(*it);

    return total;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Compute L2-norm of vector
 *
 * This function is required for some KJB algorithms, and is 
 * required to adhere to the ModelParameters concept.
 */
inline
double norm2(const Vector& op1)
{
    return op1.magnitude();
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Compute the Euclidian distance between two vectors
 *
 * A routine that computes the distance between two vectors
 */
inline
double vector_distance(const Vector& op1, const Vector& op2)
{
    return kjb_c::vector_distance(op1.get_c_vector(), op2.get_c_vector());
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    
/**
 * @brief   Compute the square of the Euclidian distance between two vectors
 *
 * A routine that computes the square of the distance between two vectors
 */
inline
double vector_distance_squared(const Vector& op1, const Vector& op2)
{
    return kjb_c::vector_distance_sqrd(op1.get_c_vector(), op2.get_c_vector());
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Return the maximum element in the vector.
 */
inline
Vector::Value_type max(const Vector& vec)
{
    // Test program was HERE.
    return kjb_c::max_vector_element(vec.get_c_vector());
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Return the minimum element in the vector.
 */
inline
Vector::Value_type min(const Vector& vec)
{
    // Test program was HERE.
    return kjb_c::min_vector_element(vec.get_c_vector());
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Concatenate two vectors.
 */
inline Vector cat_vectors(const Vector& first, const Vector& second)
{
    Vector result = first;
    result.resize(first.size() + second.size());
    std::copy(second.begin(), second.end(), result.begin() + first.size());
    return result;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Concatenate the vectors in the given sequence of vectors.
 */
template<class InputIterator>
Vector cat_vectors(InputIterator first, InputIterator last)
{
    int size = 0;

    for(InputIterator p = first; p != last; p++)
    {
        size += p->get_length();
    }

    Vector ret(size);

    size = 0;
    for(InputIterator p = first; p != last; p++)
    {
        for(int i = 0; i < p->get_length(); i++)
        {
            ret[size + i] = (*p)[i];
        }
        size += p->get_length();
    }

    return ret;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Separate the given vector into vectors of the give size; the
 *          vectors will be output into the output iterator.
 */
template<class OutputIterator>
OutputIterator separate_vector(const Vector& vec, int sz, OutputIterator result)
{
    int n = vec.get_length() / sz;

    if(n == 0)
    {
        KJB_THROW_2(Illegal_argument, "separate_vectors: cannot desired vector size is too large.");
    }

    for(int i = 0; i < n; i++)
    {
        *result++ = create_vector_from_vector_section(vec, i * sz, sz);
    }

    return result;
}

/**
 * @brief   Treat a std::vector of kjb::Vectors as a matrix and get its
 *          'transpose'.
 */
std::vector<Vector> get_transpose(const std::vector<Vector>& m);

/// @brief Build an Int_vector that is the element-wise floor of a real Vector
Int_vector floor( const Vector& realv );

/*
 * @brief generates a Vector of uniformly spaced points along a vector (including endpoints)
 * Ideally it should be matlab's "linspace"
 * @param  double a - start point, double b - end point, double n - number of points
 * @author Josh Bowdish
 */
kjb::Vector create_uniformly_spaced_vector(double a,double b, unsigned n);


/** @} */

/** @brief  Non-member swap. */
inline void swap(Vector& v1, Vector& v2) { v1.swap(v2); }

} 

//namespace kjb


#endif /*VECTOR_WRAP_H */

