/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2007-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Kobus Barnard, Luca Del Pero, Kyle Simek, Andrew Predoehl.         |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: l_int_vector.h 21596 2017-07-30 23:33:36Z kobus $ */

#ifndef L_CPP_INT_VECTOR_WRAP_H
#define L_CPP_INT_VECTOR_WRAP_H

/**
 * @file
 * @author Kobus Barnard
 * @author Luca Del Pero
 * @author Kyle Simek
 * @author Andrew Predoehl
 * @author Ernesto Brau
 *
 * @brief Definition for the Int_vector class, a thin wrapper on the KJB
 *        Int_vector struct and its related functionality.
 *
 * If you make changes to this file, PLEASE CONSIDER making parallel changes to
 * m_vector.h,
 * whose structure closely parallels the structure of this file.
 * Tip:  use vimdiff on both files to show the parallel structure.
 *
 * Although this class has much the same interface as class Vector, they
 * are not derived from a common abstract interface, because (1) we want as
 * much speed as possible -- this code should be eligible to put inside a
 * tight inner loop; and (2) I don't know whether that would be useful.
 */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "l/l_debug.h"
#include "l/l_int_vector.h"
#include "l/l_sys_io.h"
#include "l/l_sys_lib.h"

#include "l_cpp/l_util.h"
#include "l_cpp/l_exception.h"
#include <vector>


#ifdef KJB_HAVE_BST_SERIAL
#include <boost/serialization/access.hpp>
#endif

#include <boost/concept_check.hpp>


// forward declarations
namespace boost {
namespace archive {
    class text_iarchive;
    class text_oarchive;
} // namespace archive
} // namespace boost

namespace kjb {

/**
 * @addtogroup kjbLinearAlgebra
 * @{
 */

class Int_matrix;

/**
 * @brief This class implements vectors, in the linear-algebra sense,
 *        restricted to integer-valued elements.
 *
 * For better maintainability, refer to the element type using this class's
 * Value_type typedef, instead of referring to 'int' directly.
 *
 * Most methods of this class are implemented in the C language portion of the
 * KJB library, with this class forming a thin (usually inlined) layer.
 */
class Int_vector
{
#ifdef KJB_HAVE_BST_SERIAL
    friend class boost::serialization::access;
#endif

public:


    /* Class users:  to make your code as maintanable as possible, use the
     * *_type typedefs below instead of directly referencing the specific
     * type.
     */

    typedef int                 Value_type; ///< data type of the elements
    typedef kjb_c::Int_vector   Impl_type;  ///< the underlying implementation
    typedef Int_matrix          Mat_type;   ///< the associated matrix type
    typedef Int_vector          Vec_type;   ///< the associated vector type
    typedef Value_type (*Mapper)(Value_type);   ///< element transformer fun

    // Below are typdefs required to adhere to the stl Container concept
    typedef int             value_type;
    typedef int*            pointer;
    typedef const int*      const_pointer;
    typedef int&            reference;
    typedef int           const_reference;
    //typedef __gnu_cxx::__normal_iterator<value_type*, Int_vector> iterator;
    //typedef __gnu_cxx::__normal_iterator<const value_type*, Int_vector> 
    //    const_iterator;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;
    typedef std::reverse_iterator<iterator>       reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator; ///< const Iterator type
    typedef int             size_type;
    typedef int             difference_type;


private:
    Impl_type* m_vector;



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
    explicit Int_vector(int length = 0)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::get_target_int_vector(&m_vector, length) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Allocate vector of given length; contents are uninitialized.
     * @warning Native type for length is int, not unsigned.
     *
     * This ctor is almost mandatory since we want to construct from a size_t,
     * which on most platforms is unsigned int or unsigned long int.
     */
    explicit Int_vector(unsigned length)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::get_target_int_vector(&m_vector, static_cast<int>(length)) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Allocate vector of given length; contents are uninitialized.
     * @warning Native type for length is int, not unsigned long.
     *
     * This ctor is almost mandatory since we want to construct from a size_t,
     * which on most platforms is unsigned int or unsigned long int.
     */
    explicit Int_vector(unsigned long length)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::get_target_int_vector(&m_vector, static_cast<int>(length)) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Allocate vector of given length; initialize all elts to 'num'.
     */
    Int_vector(int length, Value_type num)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::get_initialized_int_vector(&m_vector, length, num) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Initialize from an array of int, of given length.
     */
    Int_vector(int length, const Value_type* data)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::get_target_int_vector(&m_vector, length) );
        std::copy( data, data + length, m_vector -> elements );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Conversion ctor:  claim ownership of an existing int vector
     *          pointer (i.e., make a shallow copy).
     *
     * This method is the proper way to say, ''Here is a kjb_c::Int_vector
     * struct that I am responsible for deleting, and I must make sure that
     * it gets destroyed when it goes out of scope.''  This is a good way
     * to wrap a vector "dynamically," after it has already been created.
     *
     * Anyplace you find yourself using free_int_vector() in your C++ code,
     * you should consider using instead this Vector class with this ctor.
     *
     * If the input pointer equals NULL then a zero length vector is
     * constructed.
     *
     * @warning Don't create two Int_vector objects from the same source this
     *          way or you'll get a double deletion bug.
     */
    Int_vector(Impl_type* vec_ptr)
        : m_vector( vec_ptr )
    {
        if ( 0 == vec_ptr )
        {
            // Test program was HERE.
            ETX( kjb_c::get_target_int_vector(&m_vector, 0) );
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
    Int_vector(const std::vector<int>& src)
        : m_vector( NULL )
    {
        Int_vector result(src.begin(), src.end());
        swap(result);
    }


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Ctor copies contents (i.e., deep copy) of an existing vector.
     *
     * @warning This method should be seldom used:  kjb_c::Int_vector objects
     *          should rarely be left in an unwrapped state.
     *
     * This kind of conversion is relatively expensive, thus we restrict its
     * use only to explicit invocation.
     */
    explicit Int_vector(const Impl_type& vec_ref)
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::copy_int_vector(&m_vector, &vec_ref) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Ctor builds from a one-row or one-column Int_matrix (making
     *          a deep copy).
     */
    explicit Int_vector(const Mat_type& src);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Construct a vector by reading contents from a named file.
     */
    Int_vector(const std::string& file_name);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Copy ctor -- calls the kjb_c function to copy an int vector.
     *
     * Although this method is essential, I hope it is seldom actually used.
     * Most of the time it should be unnecessary or optimized away somehow.
     */
    Int_vector( const Int_vector& vec_ref )
        : m_vector(0)
    {
        // Test program was HERE.
        ETX( kjb_c::copy_int_vector(&m_vector, vec_ref.m_vector) );
    }

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
    Int_vector(InputIterator_ begin_, InputIterator_ end_)
        : m_vector(0)
    {
        ETX( kjb_c::get_target_int_vector(&m_vector, 8) );
        m_vector->length = 0;

        typedef typename std::iterator_traits<InputIterator_>::
            iterator_category IterCategory_;
        m_initialize_dispatch(begin_, end_, IterCategory_());
    }


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Destructor -- which just calls the KJB destructor.
     */
    ~Int_vector()
    {
        // Test program was HERE.
        kjb_c::free_int_vector(m_vector);
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
     * @see     rand(3)
     *
     * Random values are uniformly distributed between 0 and RAND_MAX.
     */
    Int_vector& randomize(int length);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Clobber current vector contents with random values.
     * @return  an lvalue to this object
     * @see     rand(3)
     *
     * Random values are uniformly distributed between 0 and RAND_MAX.
     */
    Int_vector& randomize()
    {
        // Test program was HERE.
        return randomize( get_length() );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */


    /* -------------------------------------------------------------------
     * ASSIGNMENT OPERATORS
     * ------------------------------------------------------------------- */

    /**
     * @brief   Assignment operator:  assign from a kjb_c::Int_vector,
     *          a C struct.
     */
    Int_vector& operator=(const Impl_type& vec_ref)
    {
        if ( m_vector != &vec_ref )
        {
            // Test program was HERE.
            ETX( kjb_c::copy_int_vector(&m_vector, &vec_ref) );
        }
        //else
        //{
        //    // Test program was HERE.
        //}
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Assignment operator:  assign from a kjb::Int_vector,
     *          a C++ object.
     */
    Int_vector& operator=(const Int_vector& src)
    {
        // Test program was HERE.
        // call assignment operator for kjb_c::Int_vector
        return operator=( *src.m_vector );
    }


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Get const pointer to the underlying kjb_c::Int_vector C
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
    Impl_type* get_underlying_representation_with_guilt()
    {
        // Test program was HERE.
        return m_vector;
    }
    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Swap the representations of two vectors.
     */
    void swap( Int_vector& other )
    {
        // Test program was HERE.
        std::swap( m_vector, other.m_vector );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Resize vector, retaining previous values.
     */
    Int_vector& resize(int new_length, Value_type pad = Value_type(0) );

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
    Value_type operator[](int i) const
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
    Value_type operator()(int i) const
    {
        // Test program was HERE.
        return operator[]( i );
    }

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
    Value_type at( int i ) const
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
        KJB(ASSERT(m_vector->length <= m_vector->max_length));
        m_vector->elements[m_vector->length - 1] = x;
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
     * @see     kjb_c::write_row_int_vector_full_precision()
     *
     * If filename equals NULL or filename[0] is a null character
     * then the output is directed to standard output.
     */
    void write_row( const char* filename = 0 ) const
    {
        // Test program was HERE.
        ETX(kjb_c::write_row_int_vector( m_vector, filename ));
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Write vector as a column to a file, or to standard output.
     * @see     kjb_c::write_col_int_vector_with_header()
     *
     * If filename equals NULL or filename[0] is the null character
     * then the output is directed to standard output.
     */
    void write_col( const char* filename = 0 ) const
    {
        // Test program was HERE.
        ETX(kjb_c::write_col_int_vector_with_header( m_vector, filename ));
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
        write_row( filename );
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
    Int_vector& operator*= (Value_type op2)
    {
        // Test program was HERE.
        ETX( kjb_c::ow_multiply_int_vector_by_int_scalar(m_vector, op2) );
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Scalar multiply self, in-place, just like v *= 6.
     */
    Int_vector& multiply(Value_type op2)
    {
        // Test program was HERE.
        return operator*=( op2 );
    }


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Scalar integer division of self, in-place.
     */
    Int_vector& operator/= (Value_type op2);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Scalar integer division of self, in-place, just like v /= 2.
     */
    Int_vector& divide (Value_type op2)
    {
        // Test program was HERE.
        return operator/=( op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Add vector to self, in-place, e.g., v += delta_v.
     */
    Int_vector& operator+= (const Int_vector& op2)
    {
        // Test program was HERE.
        ETX( kjb_c::ow_add_int_vectors(m_vector, op2.m_vector) );
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Add vector to self, in-place, just like v += delta_v.
     */
    Int_vector& add (const Int_vector& op2)
    {
        // Test program was HERE.
        return operator+=( op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Subtract vector from self, in-place, e.g., v -= delta_v.
     */
    Int_vector& operator-= (const Int_vector& op2)
    {
        // Test program was HERE.
        ETX( kjb_c::ow_subtract_int_vectors(m_vector, op2.m_vector) );
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Subtract vector from self, in-place, just like v -= delta_v.
     */
    Int_vector& subtract (const Int_vector& op2)
    {
        // Test program was HERE.
        return operator-=( op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Negate self, in-place, just like v *= (-1).
     */
    Int_vector& negate()
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
     * @brief   Return the value of the minimum element in the vector.
     */
    Value_type min() const
    {
        // Test program was HERE.
        return kjb_c::min_int_vector_element(m_vector);
    }

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
                                "Int_vector is empty; min is undefined" );
        }
        if ( 0 == min_index )
        {
            // Test program was HERE.
            return min();
        }
        // Test program was HERE.
        Value_type min_val;
        *min_index = kjb_c::get_min_int_vector_element(m_vector, &min_val);
        return min_val;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief   Return the value of the maximum element in the vector.
     */
    Value_type max() const
    {
        // Test program was HERE.
        return kjb_c::max_int_vector_element(m_vector);
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
                                "Int_vector is empty; max is undefined" );
        }
        if ( 0 == max_index )
        {
            // Test program was HERE.
            return max();
        }
        // Test program was HERE.
        Value_type max_val;
        *max_index = kjb_c::get_max_int_vector_element(m_vector, &max_val);
        return max_val;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */



    /* -------------------------------------------------------------------
     * VECTOR-SPECIFIC METHODS
     * ------------------------------------------------------------------- */

    /**
     * @brief   Compute (in place) cross product of this vector and op2.
     *
     * Store result in this vector.  Only defined for dimension = 3.
     */
    Int_vector& cross_with(const Int_vector& op2);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

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
    kjb::Int_matrix hat() const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief Return this vector's magnitude.
     */
    double magnitude() const
    {
        // Test program was HERE.
        return kjb_c::int_vector_magnitude(m_vector);
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

    /**
     * @brief Return this vector's squared magnitude.
     */
    long int magnitude_squared() const
    {
        // Test program was HERE.
        return kjb_c::sum_int_vector_squared_elements(m_vector);
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

private:
    void serialize(boost::archive::text_iarchive &ar, const unsigned int version);
    void serialize(boost::archive::text_oarchive &ar, const unsigned int version);
};

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/* ---------------------------------------------------------------------
 * TEMPLATE MEMBER FUNCTIONS
 *
 * Template member function definitions go here.
 * ------------------------------------------------------------------- */

template<typename InputIterator_>
void Int_vector::m_initialize_dispatch
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
void Int_vector::insert
(
    iterator        position,
    InputIterator   begin_,
    InputIterator   end_
)
{
    const size_type N = end_ - begin_;

    m_ensure_capacity(m_vector->length + N);

    m_vector->length += N;

    reverse_iterator rit;
    for(rit = rbegin(); rit.base() != position; rit++)
    {
        *rit = *rit + N; // 
    }

    iterator dest_it = position; 
    InputIterator src_it = begin_;
    for(; src_it != end_; src_it++)
    {
        *dest_it++ = *src_it;
    }
}

/* ---------------------------------------------------------------------
 * "NAMED CONSTRUCTORS"
 *
 * The "named constructor idiom" is when we use functions to create
 * objects.  The benefit is that the names indicate the purpose of the
 * method.  The following functions act as named constructors.
 * ------------------------------------------------------------------- */

/**
 * @brief   Construct a vector with random contents.
 */
inline
Int_vector create_random_int_vector(int length)
{
    // Test program was HERE.
    Int_vector result( length );
    result.randomize();
    return result;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Construct a vector by deep-copying a section of another vector.
 * @see     kjb_c::copy_int_vector_section()
 */
inline
Int_vector create_vector_from_vector_section(const Int_vector& iv, int begin, int length)
{
    // Test program was HERE.
    kjb_c::Int_vector* outvec = 0;
    ETX_2( kjb_c::copy_int_vector_section(&outvec,
                       iv.get_c_vector(), begin, length), "Failure in create_vector_from_vector_section : allocation error or bad indices");
    return Int_vector(outvec);
}

/* ---------------------------------------------------------------------
 * ARITHMETIC OPERATORS
 *
 * Modifying operators; i.e., operators that modify the object. These
 * are member functions, as per the standard implementation. Operators
 * that do not modify the vector are non-members. Also, their corresponding
 * named methods (subtract, add, etc).
 * ------------------------------------------------------------------- */

/**
 * @brief   Return product of this (as a row-vec) times matrix on the
 *          right.
 *
 * Semantics could be expressed as vT * M, where vT is this vector, as a
 * row, and M is a matrix with the same number of rows as the length of
 * vT.
 *
 * @note    Prior to 1 Feb. 2010, this method was badly flawed.
 */
Int_vector operator*(const Int_vector& op1, const Int_matrix& op2);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Compute product of this matrix (on the left) and a column
 *          vector (on the right).
 * @throws  Dimension_mismatch if the number of columns of this matrix
 *          does not equal the length of right factor op2.
 */
Int_vector operator*( const Int_matrix& op1, const Int_vector& op2 );

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Scalar multiply, resulting in a new vector, e.g., v * 6.
 */
inline
Int_vector operator* (const Int_vector& op1, Int_vector::Value_type op2)
{
    // Test program was HERE.
    return Int_vector(op1) *= op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Scalar multiplication of a vector
 *
 * This is an inline to allow the scalar to appear on the left side
 * of the operator.
 */
inline
Int_vector operator*( Int_vector::Value_type op1, const Int_vector& op2 )
{
    // Test program was HERE.
    return op2 * op1;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Scalar integer division, yielding a new vector.
 */
inline
Int_vector operator/(const Int_vector& op1, Int_vector::Value_type op2)
{
    // Test program was HERE.
    return Int_vector(op1) /= op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Add two vectors, creating a new vector, e.g., v + w.
 */
inline
Int_vector operator+ (const Int_vector& op1, const Int_vector& op2)
{
    // Test program was HERE.
    return Int_vector(op1) += op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Subtract one vector from another, resulting in a new vector,
 *          e.g., v - w.
 */
inline
Int_vector operator- (const Int_vector& op1, const Int_vector& op2)
{
    // Test program was HERE.
    return Int_vector(op1) -= op2;
}

/**
 * @brief   Return the negation of a vector, as in (-v), resulting in a
 *          new vector.
 */
inline
Int_vector operator- (const Int_vector& op1)
{
    // Test program was HERE.
    return op1 * (-1);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Display vector contents in an ASCII format.
 *
 * This routine is mostly for debugging; consider one of the many
 * KJB output routines for more output in a more standardized form.
 */
std::ostream& operator<<(std::ostream& out, const Int_vector& m);

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
bool operator==(const Int_vector& op1, const Int_vector::Impl_type& op2);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Test for exact equality between vectors.
 *
 * This is an inline to allow the KJB C struct appear on the left side
 * of the ==.
 */
inline
bool operator==(const Int_vector::Impl_type& op1, const Int_vector& op2)
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
bool operator!=(const Int_vector& op1, const Int_vector::Impl_type &op2)
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
bool operator!=(const Int_vector::Impl_type& op1, const Int_vector& op2)
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
bool operator==(const Int_vector& op1, const Int_vector& op2)
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
bool operator!=(const Int_vector& op1, const Int_vector &op2)
{
    // Test program was HERE.
    return op1 != *op2.get_c_vector();
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/* ---------------------------------------------------------------------
 * ORDERING OPERATORS
 * Ordering operators (<, >=, etc).
 * ------------------------------------------------------------------- */

/**
 * @brief   Test weak lexicographic ordering between vectors.
 */
bool operator<(const Int_vector& op1, const Int_vector &op2);

/**
 * @brief   Test lexicographic ordering between vectors.
 */
inline
bool operator<=(const Int_vector& op1, const Int_vector& op2)
{
    return (op1 < op2) || (op1 == op2);
}

/**
 * @brief   Test lexicographic ordering between vectors.
 */
inline
bool operator>(const Int_vector& op1, const Int_vector& op2)
{
    return !(op1 < op2) && !(op1 == op2); 
}

/**
 * @brief   Test lexicographic ordering between vectors.
 */
inline
bool operator>=(const Int_vector& op1, const Int_vector& op2)
{
    return !(op1 < op2);
}

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
Int_vector::Value_type max_abs_difference( const Int_vector& op1, const Int_vector& op2 )
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
    return kjb_c::max_abs_int_vector_difference( op1.get_c_vector(), op2.get_c_vector() );
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief Returns dot product of this and op2.
 */
inline
long int dot(const Int_vector& op1, const Int_vector& op2)
{
    // Test program was HERE.
    long int result;
    ETX( kjb_c::get_int_dot_product(op1.get_c_vector(), op2.get_c_vector(), &result) );
    return result;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Compute cross product of op1 and op2.
 *
 * Store result in this vector.  Only defined for dimension = 3.
 */
Int_vector cross(const Int_vector& op1, const Int_vector& op2);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  */

/**
 * @brief   Compute l2-norm of vector
 *
 * This function is required for some KJB algorithms, and is 
 * required to adhere to the ModelParameters concept.
 */
inline
double norm2(const Int_vector& op1)
{
    return op1.magnitude();
}

/**
 * @brief   Compute the Euclidian distance between two vectors
 *
 * A routine that computes the distance between two vectors
 */
inline
double vector_distance(const Int_vector& op1, const Int_vector& op2)
{
    Int_vector temp = op1 - op2;
    return temp.magnitude();
}

/**
 * @brief   Compute the square of the Euclidian distance between two vectors
 *
 * A routine that computes the square of the distance between two vectors
 */
inline
double vector_distance_squared(const Int_vector& op1, const Int_vector& op2)
{
    Int_vector temp = op1 - op2;
    return temp.magnitude_squared();
}

/** @} */

} //namespace

#endif

