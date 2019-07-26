/* $Id: l_iterator.h 15196 2013-08-15 23:05:59Z ernesto $ */
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

#ifndef KJB_L_ITERATOR
#define KJB_L_ITERATOR

/**
 * @file    A collection of general-purpose iterators
 * @author  Kyle Simek
 * @author  Ernesto Brau
 */

#include <boost/concept_check.hpp>
#include <iterator>
namespace kjb
{

/**
 * Const version of Circular_iterator
 *
 * @see Circular_iterator
 */
template<class const_iterator>
class const_circular_iterator
    : public std::iterator<std::bidirectional_iterator_tag, 
                           typename std::iterator_traits<const_iterator>::value_type> 
{ 
    typedef const_circular_iterator Self;

protected:
    const_iterator   begin;
    const_iterator   end;
    const_iterator   iter;

public:
    typedef typename std::iterator_traits<const_iterator>::value_type value_type;

    const_circular_iterator() : begin(), end(), iter() {}

    const_circular_iterator(const_iterator b, const_iterator e) :
        begin(b),
        end(e),
        iter(begin)
    {}

    template <class Container>
    explicit const_circular_iterator(const Container& c) :
        begin(c.begin()),
        end(c.end()),
        iter(begin) 
    {}

    const_circular_iterator(const Self& other) :
        begin(other.begin),
        end(other.end),
        iter(other.iter)
    {}

    Self& operator=(const Self& other)
    {
        if(&other != this)
        {
            Self tmp = other;
            this->swap(tmp);
        }
        return *this;
    }

    const const_iterator& get_iterator() const
    {
        return iter;
    }

    void swap(Self& other)
    {
        using std::swap;
        swap(begin, other.begin);
        swap(end, other.end);
        swap(iter, other.iter);
    }

    Self& operator--()
    {
        // todo: use iterator tags to determine if we have a reverse
        // iterator or a random-access iterator
        /* const_iterator prev = begin;

        if(iter == begin)
        {
            iter = end;
        }

        for(const_iterator cur = begin; cur != iter; cur++)
        {
            prev = cur;
        }

        iter = prev;

        return(*this); */

        if(iter == begin)
        {
            iter = end;
        }
        iter--;
        return *this;
    }

    Self operator--(int)
    {
        Self t = *this;
        --(*this);
        return(t);
    }

    Self& operator++()
    {     
        ++iter;
        if(iter == end)
        {
            iter = begin;
        }
        return(*this);
    }

    Self operator++(int)
    {
        Self t = *this;
        ++(*this);
        return(t);
    }

    const value_type& operator*() const
    {
        return (*iter);
    }

    const value_type* operator->() const
    {
        return (iter.operator->());
    }

    bool operator==(const Self& rhs) const
    {
        return (iter == rhs.iter);
    }

    bool operator==(const const_iterator& rhs) const
    {
        return iter == rhs;
    }

    bool operator!=(const Self& rhs) const
    {
        return !operator==(rhs); 
    }

    bool operator!=(const const_iterator& rhs) const
    {
        return !operator==(rhs); 
    }

    void reset()
    {
        iter = begin;
    }
};

/**
 * An very simple iterator that continues at the beginning when it reaches the
 * end.  This is useful when you need to cycle through elements of a 
 * collection by avoiding messy modulo operations when stepping backward.
 *
 * @note  You should NOT pass this to any algorithm that expects the
 * iterator to eventally end.  This iterator would appear to represent
 * and infinitely long array that repeats every N elements.
 *
 * @author Kyle Simek
 * @see const_circular_iterator
 */
template<class iterator>
class circular_iterator
    : public std::iterator<std::bidirectional_iterator_tag, 
                           typename std::iterator_traits<iterator>::value_type> 
{ 
    typedef circular_iterator Self;

protected:
    iterator   begin;
    iterator   end;
    iterator   iter;

public:
    typedef typename std::iterator_traits<iterator>::value_type value_type;

    circular_iterator() : begin(), end(), iter() {};
    circular_iterator(iterator b, iterator e) : begin(b), end(e), iter(b) {};

    circular_iterator(const circular_iterator& other) :
        begin(other.begin), 
        end(other.end), 
        iter(other.iter) 
    {}

    circular_iterator(Self& other) :
        begin(other.begin), 
        end(other.end), 
        iter(other.iter)
    {}

    template <class Container_>
    explicit circular_iterator(Container_& c) : 
        begin(c.begin()),
        end(c.end()),
        iter(c.begin()) 
    {}


    Self& operator=(const Self& other)
    {
        if(&other != this)
        {
            Self tmp = other;
            this->swap(tmp);
        }
        return *this;
    }

    const iterator& get_iterator() const
    {
        return iter;
    }

    void swap(Self& other)
    {
        using std::swap;
        swap(begin, other.begin);
        swap(end, other.end);
        swap(iter, other.iter);
    }


    Self& operator--()
    {
        /* iterator prev = begin;

        if(begin == iter)
        {
            iter = end;
        }

        // todo: use iterator tags to determine if we have a reverse iterator or a random-access iterator
        for(iterator cur = begin; cur != iter; cur++)
        {
            prev = cur;
        }

        iter = prev;

        return(*this); */

        if(iter == begin)
        {
            iter = end;
        }
        iter--;
        return *this;
    }

    Self operator--(int)
    {
        Self t = *this;
        this->operator--();
        return(t);
    }

    Self& operator++()
    {     
        ++iter;
        if(iter == end)
        {
            iter = begin;
        }
        return(*this);
    }

    Self operator++(int)
    {
        Self t(*this);
        ++(*this);
        return(t);
    }

    value_type& operator*() const
    {
        return (*iter);
    }

    value_type* operator->() const
    {
        return (iter.operator->());
    }

    bool operator==(const Self& rhs) const
    {
        return (iter == rhs.iter);
    }

    bool operator==(const iterator& rhs) const
    {
        return iter == rhs;
    }

    bool operator!=(const Self& rhs) const
    {
        return !operator==(rhs); 
    }

    bool operator!=(const iterator& rhs) const
    {
        return !operator==(rhs); 
    }

    void reset()
    {
        iter = begin;
    }
};

template<class const_iterator> inline
const_circular_iterator<const_iterator> make_const_circular_iterator(const_iterator begin, const_iterator end)
{
    return const_circular_iterator<const_iterator>(begin, end);
}

template<class Container> inline
const_circular_iterator<typename Container::const_iterator> make_const_circular_iterator(const Container& container)
{
    return const_circular_iterator<typename Container::const_iterator>(container);
}

template<class iterator> inline
circular_iterator<iterator> make_circular_iterator(iterator begin, iterator end)
{
    return circular_iterator<iterator>(begin, end);
}

template<class Container> inline
circular_iterator<typename Container::iterator> make_circular_iterator(Container& container)
{
    return circular_iterator<typename Container::iterator>(container);
}


}
#endif
