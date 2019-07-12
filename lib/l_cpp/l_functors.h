/* $Id: l_functors.h 18278 2014-11-25 01:42:10Z ksimek $ */
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
   |  Author:  Ernesto Brau
 * =========================================================================== */

#ifndef L_FUNCTORS_H_INCLUDED
#define L_FUNCTORS_H_INCLUDED

/**
 * @file    A collection of useful functors that are not present in the STL
 *          or boost (I think).
 *
 * @author  Ernesto Brau
 */

#include <algorithm>

namespace kjb {

/**
 * @brief   Generator that increments (++) its state everytime it is called.
 *          Useful for creating sequences of contigous values.
 */
template<class T>
class Increment
{
public:
    typedef T result_type;

private:
    T value;

public:
    Increment(const T& initial_value = T()) : value(initial_value)
    {}

    T operator()()
    {
        return value++;
    }

    ~Increment()
    {}
};

/**
 * @brief   Generator that increases (using +=) itself by the
 *          given value everytime it is called.
 */
template<class T>
class Increase_by
{
public:
    typedef T result_type;

private:
    T value;
    T inc_value;

public:
    Increase_by(const T& initial_value, const T& increase_value) :
        value(initial_value), inc_value(increase_value)
    {}

    T operator()()
    {
        T v = value;
        value += inc_value;
        return v;
    }

    ~Increase_by()
    {}
};

/**
 * @brief   Predicate that compares the kth element of a indexable type.
 */
template<class T>
class Index_less_than
{
public:
    typedef bool result_type;

private:
    int m_index;

public:
    Index_less_than(int index) : m_index(index)
    {}

    bool operator()(const T& lhs, const T& rhs) const
    {
        return lhs[m_index] < rhs[m_index];
    }

    ~Index_less_than()
    {}
};

/**
 * @brief   Selects a coordinate from a vector type
 */
template<class V>
class Select_coordinate
{
public:
    typedef double result_type;

private:
    int coord;

public:
    Select_coordinate(int coordinate) : coord(coordinate)
    {}

    double operator()(const V& v) const
    {
        return v[coord];
    }

    ~Select_coordinate()
    {}
};

/**
 * @brief   Identity function
 */
template<class T>
class Identity
{
public:
    typedef T result_type;

public:
    Identity(){}

    T operator()(const T& t) const
    {
        return t;
    }

    ~Identity(){}
};

/**
 * @brief   Predicate that returns true every nth call.
 */
template<class T>
class Every_nth_element
{
public:
    typedef T argument_type;
    typedef bool result_type;

private:
    int m_n;
    mutable int m_i;

public:
    Every_nth_element(int n, int start = 1) : m_n(n), m_i(start)
    {}

    bool operator()(const T& /*t*/) const
    {
        if(m_i++ == m_n)
        {
            m_i = 1;
            return true;
        }

        return false;
    }

    ~Every_nth_element()
    {}
};

/**
 * @brief   Predicate that returns true if address of element equals given.
 */
template<class T>
class Compare_address
{
public:
    typedef T argument_type;
    typedef bool result_type;

private:
    const T* m_Tp;

public:
    Compare_address(const T* Tp) : m_Tp(Tp) {}

    bool operator()(const T& t) const
    {
        return &t == m_Tp;
    }
};

/**
 * @brief   Functor that returns the address of a given object.
 */
template<class T>
class Get_address
{
public:
    typedef T argument_type;
    typedef const T* result_type;

public:
    const T* operator()(const T& t) const
    {
        return &t;
    }
};

/**
 * binary operator that returns the minimum of two elements
 * Useful in conjunction with std::accumulate to find the minimum element of 
 * a collection of elements.
 */
template <class T>
class minimum
{
public:
    const T operator()(const T& op1, const T& op2) { return std::min(op1, op2); }
};

/**
 * binary operator that returns the maximum of two elements.
 * Useful in conjunction with std::accumulate to find the maximum element of 
 * a collection of elements.
 */
template <class T>
class maximum
{
public:
    const T operator()(const T& op1, const T& op2) { return std::max(op1, op2); }
};



} //namespace kjb

#endif /* L_FUNCTORS_H_INCLUDED */

