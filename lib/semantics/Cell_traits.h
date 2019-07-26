#ifndef CELL_TRAITS_H_
#define CELL_TRAITS_H_

/*!
 * @file Cell_traits.h
 *
 * @author Colin Dawson 
 * $Id: Cell_traits.h 17097 2014-07-05 00:13:22Z cdawson $ 
 */

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

namespace semantics
{

template<class T>
class Prior_cell;

template<size_t N>
class Categorical_event;

template<class T, size_t N>
class Marginal_cell;


template<class T, size_t N>
struct Cell_traits
{
    typedef Marginal_cell<T, N-1> Margin_type;
};

template<class T>
struct Cell_traits<T,1>
{
    typedef Prior_cell<T> Margin_type;
};

}; //namespace semantics

#endif
