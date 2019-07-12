/* $Id: prob_weight_array.h 20243 2016-01-20 22:59:59Z jguan1 $ */

#ifndef PROB_WEIGHT_ARRAY_H_
#define PROB_WEIGHT_ARRAY_H_

/*!
 * @file prob_weight_array.h
 *
 * @author Colin Dawson
 *
 * implements a class to handle possibly unnormalized weight vectors
 * to be manipulated outside the context of a distribution object.
 * Largely syntactic sugar over std algorithms
 */

#include "l_cpp/l_exception.h"
#include <prob_cpp/prob_util.h>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <numeric>
#include <algorithm>
#include <ostream>
#include <cmath>

namespace kjb
{
/// Forward declarations
template<size_t D> class Weight_array;

template<size_t D>
bool operator==(const Weight_array<D>& left, const Weight_array<D>& right);

template<size_t D>
bool operator!=(const Weight_array<D>& left, const Weight_array<D>& right);

/*! @class a size D weight array
 */
template<size_t D>
class Weight_array
{
public:
    typedef Weight_array<D> Self_t;
    typedef double Val_type;
    typedef boost::array<Val_type, D> Data_type;
    typedef boost::array<bool, D> Filter;
    typedef typename Data_type::iterator iterator;
    typedef typename Data_type::const_iterator const_iterator;

public:
    /*! @brief construct an empty Weight_array
     */
    Weight_array()
        : values_(), total_mass_(0), modified_(true), normalized_(false)
    {}

    /*! @brief construct a Weight_array from a value array
     */
    Weight_array(const Data_type& values)
    : values_(values),
      total_mass_(0),
      modified_(true),
      normalized_(false)
    {}

    template<typename InputIter>
    Weight_array(InputIter start, InputIter end)
    : values_(), total_mass_(0), modified_(true), normalized_(false)
    {
        std::copy(start, end, values_.begin());
    }

    /*! @brief get a const_iterator to the first value
     */
    const_iterator cbegin() const {return values_.begin();}

    /*! @brief get a const_iterator to the last value
     */
    const_iterator cend() const {return values_.end();}

    /*! @brief get a const_iterator to the first value
     *
     *  useful for passing to algorithms like copy, transform, etc.
     */
    const_iterator begin() const {return cbegin();}

    /*! @brief get a const_iterator to the last value
     *
     *  useful for passing to algorithms like copy, transform, etc.
     */
    const_iterator end() const {return cend();}

    /*! @brief get an iterator to the first value
     *
     *  useful for passing to algorithms like copy, transform, etc.
     */
    iterator begin() {return values_.begin();}

        /*! @brief get an iterator to the last value
     *
     *  useful for passing to algorithms like copy, transform, etc.
     */
    iterator end() {return values_.end();}

    /*! @brief get total mass
     */
    Val_type total_mass() const;

    /*! @brief normalize the weights
     */
    Self_t& normalize();

    /*! @brief shift values
     */
    Self_t& operator+=(const Self_t& right);

    /*! @brief rescale the weights element-wise
     */
    Self_t& operator*=(const Self_t& right);

    /*! @brief rescale the weights by division element-wise
     */
    Self_t& operator/=(const Self_t& right);

    /*! @brief shift values by a constant
     */
    Self_t& operator+=(const Val_type& offset);

    /*! @brief rescale the weights by a constant
     */
    Self_t& operator*=(const Val_type& scale);

    /*! @brief divide the weights by a constant
     */
    Self_t& operator/=(const Val_type& scale);

    /*! @brief zero out some weights by multiplying by a bool array
     */
    Self_t& operator*=(const Filter& filter);

    /*! @brief access value in position @a position.
     *  @return const reference to value
     */
    const Val_type& operator[](const size_t& position) const;

    template<size_t M, size_t K>
    friend Weight_array<M> convex_combination(
        const Weight_array<K>&                    weights,
        const boost::array<Weight_array<M>, K>&   components
        );
    template<size_t M>
    friend std::ostream& operator<<(
        std::ostream& os, const Weight_array<M>& wa);

    /*! @brief check if two weight arrays are equal
     */
    friend bool operator==<>(const Self_t& left, const Self_t& right);

    /*! @brief check if two weight arrays are unequal
     */
    friend bool operator!=<>(const Self_t& left, const Self_t& right);

    private:
    /*! @brief set to normalized (if generated from a fn that yields
     *         normalized weights)
     */
    void set_as_normalized()
    {
        total_mass_ = 1;
        modified_ = false;
        normalized_ = true;
    }

    Data_type values_;
    mutable Val_type  total_mass_;
    mutable bool      modified_;
    bool      normalized_;
};

/// Free function declarations

template<size_t D>
Weight_array<D> operator+
(
    const Weight_array<D>& left,
    const Weight_array<D>& right
);

template<size_t D>
Weight_array<D> operator*
(
    const Weight_array<D>& left,
    const Weight_array<D>& right
);

template<size_t D>
Weight_array<D> operator/
(
    const Weight_array<D>& left,
    const Weight_array<D>& right
);

template<size_t D>
Weight_array<D> operator+
(
    const Weight_array<D>&                    w,
    const typename Weight_array<D>::Val_type& offset
);

template<size_t D>
Weight_array<D> operator*(
const Weight_array<D>&                    w,
const typename Weight_array<D>::Val_type& scale
);

template<size_t D>
Weight_array<D> operator/
(
    const Weight_array<D>&                    w,
    const typename Weight_array<D>::Val_type& right
);

template<size_t D, size_t K>
Weight_array<D> convex_combination
(
    const Weight_array<K>&                    weights,
    const boost::array<Weight_array<D>, K>&   components
);

/// Member function definitions

template<size_t D>
typename Weight_array<D>::Val_type Weight_array<D>::total_mass() const
{
    if(modified_)
    {
        total_mass_ = std::accumulate(begin(), end(), 0.0);
        modified_ = false;
    }
    return total_mass_;
}

template<size_t D>
Weight_array<D>& Weight_array<D>::normalize()
{
    if(!normalized_)
    {
        operator/=(total_mass());
        set_as_normalized();
    }
    return *this;
}

template<size_t D>
Weight_array<D>& Weight_array<D>::operator+=(const Weight_array<D>& right)
{
    std::transform(
        begin(), end(), right.begin(), begin(),
        std::plus<Val_type>()
        );
    modified_ = true; normalized_ = false;
    return *this;
}

template<size_t D>
Weight_array<D>& Weight_array<D>::operator*=(const Weight_array<D>& right)
{
    std::transform(
        begin(), end(), right.begin(), begin(),
        std::multiplies<Val_type>()
        );
    modified_ = true; normalized_ = false;
    return *this;
}

template<size_t D>
Weight_array<D>& Weight_array<D>::operator/=(const Weight_array<D>& right)
{
    std::transform(
        begin(), end(), right.begin(), begin(),
        std::divides<Val_type>()
        );
    modified_ = true; normalized_ = false;
    return *this;
}

template<size_t D>
Weight_array<D>& Weight_array<D>::operator+=(
const typename Weight_array<D>::Val_type& offset
)
{
std::transform(
    begin(), end(), begin(),
    boost::bind(std::plus<Val_type>(), _1, offset)
    );
if(modified_ == false) total_mass_ += D*offset;
normalized_ = false;
return *this;
}

template<size_t D>
Weight_array<D>& Weight_array<D>::operator*=(
const typename Weight_array<D>::Val_type& scale
)
{
std::transform(
    begin(), end(), begin(),
    boost::bind(std::multiplies<Val_type>(), _1, scale)
    );
if(modified_ == false) total_mass_ *= scale;
normalized_ = false;
return *this;
}

template<size_t D>
Weight_array<D>& Weight_array<D>::operator/=(
const typename Weight_array<D>::Val_type& scale
)
{
std::transform(
    begin(), end(), begin(),
    boost::bind(std::divides<Val_type>(), _1, scale)
    );
if(modified_ == false) total_mass_ /= scale;
normalized_ = false;
return *this;
}

template<size_t D>
Weight_array<D>& Weight_array<D>::operator*=(
const typename Weight_array<D>::Filter& filter
)
{
std::transform(
    begin(), end(), filter.begin(), begin(),
    boost::bind(std::multiplies<Val_type>(), _1, _2)
    );
modified_ = true;
normalized_ = false;
return *this;
}

template<size_t D>
const typename Weight_array<D>::Val_type& Weight_array<D>::operator[](
const size_t& position
) const
{
if(!(position < D))
{
    KJB_THROW_2(
    Index_out_of_bounds,
    "Attempted to access nonexistent data position in "
    "kjb::Weight_array"
    );
}
return values_[position];
}

/// Free and friend function definitions

namespace
{
template<size_t D>
typename Weight_array<D>::Val_type abs_log_ratio
(
    typename Weight_array<D>::Val_type a,
    typename Weight_array<D>::Val_type b
)
{
    return abs(log(a) - log(b));
}
}

template<size_t D>
bool operator==(const Weight_array<D>& left, const Weight_array<D>& right)
{
    typedef Weight_array<D> Wt_arr;
    typedef typename Wt_arr::Val_type Val_t;
    typedef typename Wt_arr::Data_type Data_t;
    if(left.total_mass() == 0 && right.total_mass() == 0) return true;
    if(left.total_mass() == 0 || right.total_mass() == 0) return false;
    Wt_arr left_normed = left;
    Wt_arr right_normed = right;
    left_normed.normalize();
    right_normed.normalize();
    Data_t diff_arr ;
    std::transform(
        left_normed.begin(), left_normed.end(),
        right_normed.begin(),
        diff_arr.begin(),
        abs_log_ratio<D>
        );
    Val_t total_abs_diff =
        std::accumulate(
        diff_arr.begin(), diff_arr.end(), static_cast<Val_t>(0)
        );
    return total_abs_diff < D * 0.0000000000000001;
}

template<size_t D>
bool operator!=(const Weight_array<D>& left, const Weight_array<D>& right)
{
    return !(left == right);
}

template<size_t D>
Weight_array<D> operator+
(
    const Weight_array<D>& left,
    const Weight_array<D>& right
)
{
    Weight_array<D> result = left;
    result += right;
    return result;
}

template<size_t D>
Weight_array<D> operator*
(
        const Weight_array<D>& left,
        const Weight_array<D>& right
)
{
    Weight_array<D> result = left;
    result *= right;
    return result;
}

template<size_t D>
Weight_array<D> operator/
(
        const Weight_array<D>& left,
        const Weight_array<D>& right
)
{
    Weight_array<D> result = left;
    result /= right;
    return result;
}

template<size_t D>
Weight_array<D> operator+
(
    const Weight_array<D>&                    w,
    const typename Weight_array<D>::Val_type& offset
)
{
    Weight_array<D> result = w;
    result += offset;
    return result;
};

template<size_t D>
Weight_array<D> operator*
(
    const Weight_array<D>&                    w,
    const typename Weight_array<D>::Val_type& scale
)
{
    Weight_array<D> result = w;
    result *= scale;
    return result;
};

template<size_t D>
Weight_array<D> operator/
(
    const Weight_array<D>&                    w,
    const typename Weight_array<D>::Val_type& normalization
)
{
    Weight_array<D> result = w;
    result /= normalization;
    return result;
};

template<size_t D, size_t K>
Weight_array<D> convex_combination
(
    const Weight_array<K>&                    weights,
    const boost::array<Weight_array<D>, K>&   components
)
{
    typedef Weight_array<D> WA;
    typedef typename WA::Val_type Val_t;
    WA result;
    boost::array<WA, K> rescaled_components = components;
    std::for_each(
        rescaled_components.begin(), rescaled_components.end(),
        boost::bind(&WA::normalize, _1)
        );
    std::transform(
        rescaled_components.begin(), rescaled_components.end(),
        weights.begin(),
        rescaled_components.begin(),
        boost::bind(
        static_cast<WA& (WA::*)(const Val_t&)>(&WA::operator*=),
        _1, _2
        )
        );
    result =
        std::accumulate(
        rescaled_components.begin(), rescaled_components.end(),
        result
        );
    result.normalize();
    return result;
}

template<size_t D> std::ostream& operator<<
(
    std::ostream& os,
    const Weight_array<D>& wa
)
{
    os << "(";
    for(typename Weight_array<D>::const_iterator it = wa.begin();
        it != wa.end(); ++it)
    {
        os << (*it) << ",";
    }
    os << ")";
    return os;
}

template<typename T, size_t D> std::ostream& operator<<
(
    std::ostream& os, 
    const boost::array<T, D>& arr
)
{
    os << "(";
    for(typename boost::array<T,D>::const_iterator it = arr.begin();
        it != arr.end(); ++it)
    {
        os << (*it) << ",";
    }
    os << ")";
    return os;
}
};//namespace kjb

#endif
