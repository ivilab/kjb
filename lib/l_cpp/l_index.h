/* $Id: l_index.h 21596 2017-07-30 23:33:36Z kobus $ */
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

#ifndef KJB_L_CPP_INDEX_H
#define KJB_L_CPP_INDEX_H

#include "l_cpp/l_exception.h"
#include "l/l_debug.h"
#include "l_cpp/l_util.h"
#include "l_cpp/l_int_vector.h"

#include <boost/concept_check.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <vector>
#include <list>
#include <sstream>
#include <iterator>
#include <algorithm>

#include <limits>

namespace kjb {

/**
 * @addtogroup kjbLinearAlgebra
 * @{
 */

// forward declaration
class Int_vector;

/**
 * Object for specifying sets of matrix and vector indices.  Ranges
 * need not be contiguous or ordered, and values need not be unique.
 * This object is indended to be passed to the subscript operator of Matrix and
 * Vector, which will return sub-matrices and sub-vectors.
 *
 * Contains a special constant, Index_range::all, which is an alias for an
 * Index_range containing all indices of the Matrix/Vector, in ascending order.
 *
 * Objects of this type are rarely created explicitly, but will be implicitly
 * converted in calls to the subscript operator
 *     
 *     v["2:5"] = Vector(1.0, 1.0, 2.0, 2.0);
 *
 * @note This class is designed to be as lightweight as possible in terms of storage.  For example, 
 * Index_range("1:10000000") takes the same space as Index_range("1:2");
 */
class Index_range
{
    typedef Index_range Self;
public:
    static const Index_range ALL;

    struct End_type {};
    static const End_type END;

    /**
     * empty range
     */
    Index_range() :
        ranges_(),
        all_(false)
    {}

    /**
     * Create a single-element range
     */
    Index_range(size_t index) :
        ranges_(),
        all_(false)
    {
        ranges_.push_back(new Single_element(index));
    }

    /**
     * Create a single-element range
     */
    Index_range(int index) :
        ranges_(),
        all_(false)
    {
        ranges_.push_back(new Single_element(index));
    }

    /**
     * Receives matlab-style strings like "1:3,4:2:10,15"
     */
    Index_range(const std::string& str) :
        ranges_(),
        all_(false)
    {
        init_from_str(str);
    }

    Index_range(const char* str) :
        ranges_(),
        all_(false)
    {
        init_from_str(std::string(str));
    }

    Index_range(const Index_range& src ) :
        ranges_(src.ranges_.size()),
        all_(src.all_)
    {
        for(size_t i = 0; i < src.ranges_.size(); i++)
        {
            ranges_[i] = src.ranges_[i]->clone();
        }
    }

    Index_range& operator=(const Index_range& other)
    {
        Index_range tmp(other);
        swap(tmp);
        return *this;
    }

    /**
     * Convert the set of indices into a vector
     * and return it.
     *
     * Index_range can store large ranges symbolically, allowing it
     * to represent an arbitrarilly large number of indices, while memory
     * consumption remains constant.  Calling this method for large ranges
     * will require lots of memory, as all indices will be stored explicitly.
     */
    std::vector<size_t> expand() const
    {
        size_t my_size = size();

        std::vector<size_t> result(my_size);

        for(size_t i = 0; i < my_size; i++)
        {
            result[i] = operator[](i);
        }

        return result;
    }

    void swap(Index_range& other)
    {
        using std::swap;
        ranges_.swap(other.ranges_);
        swap(all_, other.all_);
    }
//
//    /// create from collection of Range_elements
//    template <class Input_iterator>
//    Index_range(Input_iterator begin_, Input_iterator end_) :
//        ranges_(begin_, end_),
//        all_(false)
//    {
//    }

    Index_range(const Int_vector& v);

    template <class Int_type>
    Index_range(typename std::vector<Int_type> v) :
        ranges_(),
        all_(false)
    {
        BOOST_STATIC_ASSERT((boost::is_convertible<Int_type, size_t>::value));

        ranges_.push_back(new Listing_element(v));
    }

    template <class Int_type>
    Index_range(std::list<Int_type> v) :
        ranges_(),
        all_(false)
    {
        BOOST_STATIC_ASSERT((boost::is_convertible<Int_type, size_t>::value));
        ranges_.push_back(new Listing_element(v));
    }

    Index_range(size_t first, size_t end, size_t interval = 1) :
        ranges_(),
        all_(false)
    {
        ranges_.push_back(new Interval_element(first, end, interval));
    }


    Index_range(size_t first, End_type end, size_t interval = 1) :
        ranges_(),
        all_(false)
    {
        ranges_.push_back(new Interval_element(first, end, interval));
    }

    Index_range& concat(Index_range& other)
    {
        for(size_t i = 0; i < other.ranges_.size(); i++)
        {
            ranges_.push_back(other.ranges_[i]->clone());
        }
        return *this;
    }

    size_t max() const
    {
        size_t max = 0;
        for(size_t i = 0; i < ranges_.size(); i++)
        {
            size_t cur_max = ranges_[i]->max();

            if(cur_max > max)
            {
                max = cur_max;
            }
        }
        return max;
    }
        

    ~Index_range()
    {
        for(size_t i = 0; i < ranges_.size(); i++)
        {
            delete ranges_[i];
        }
    }

    explicit Index_range(bool all_elements) :
        ranges_(),
        all_(all_elements)
    {}

    size_t operator[](size_t i) const
    {
        if(all()) return i;


        // iterate through range elements until the i-th index is found.
        size_t range_i = 0;
        for(; range_i < ranges_.size(); range_i++)
        {
            const size_t range_size = ranges_[range_i]->size();
            if(i < range_size)
                break;

            i -= range_size;
        }

        if(range_i == ranges_.size())
        {
            // didn't find range that contains the index i
            KJB_THROW(Index_out_of_bounds);
        }

        return (*ranges_[range_i])[i];
    }

    size_t size() const
    {
        if(all())
        {
            KJB_THROW_2(Runtime_error, "Can't know the size of an \"all\" range.  Determining the size is the responsibility of the caller.");
        }

        size_t total = 0;

        for(size_t i = 0; i < ranges_.size(); i++)
        {
            total += ranges_[i]->size();
        }

        return total;
    }

    bool all() const { return all_; }

private:
    /// Basic element of a range.  A full range is an ordered colection of Range_elements
    class Range_element
    {
    public:
        virtual ~Range_element() {}
        virtual size_t operator[](size_t i) const = 0;
        virtual size_t size() const = 0;
        virtual Range_element* clone() const = 0;
        virtual size_t max() const = 0;
    };

    /// trivial range consisting of a single element
    class Single_element : public Range_element
    {
    public:
        Single_element(size_t i) : value_(i) {}

        virtual size_t operator[](size_t i) const
        {
            if(i == 0)
                return value_;
            KJB_THROW(Index_out_of_bounds);
        }
        virtual size_t size() const { return 1; }


        virtual Range_element* clone() const
        {
            return new Single_element(*this);
        }

        virtual size_t max() const { return value_; }
    private:
        size_t value_;
    };


    /// Range element consisting of a listing of specific indices
    class Listing_element : public Range_element
    {
    public:
        Listing_element(Int_vector v);

        Listing_element(std::vector<size_t> v) :
            indices_(v.begin(), v.end())
        {}

        Listing_element(std::list<size_t> v) :
            indices_(v.begin(), v.end())
        {}

        virtual Range_element* clone() const
        {
            return new Listing_element(*this);
        }

        virtual size_t operator[](size_t i) const
        {
            return indices_[i];
        }
        virtual size_t size() const
        {
            return indices_.size();
        }

        virtual size_t max() const
        {
            return *std::max_element(indices_.begin(), indices_.end());
        }
    private:
        std::vector<size_t> indices_;


    };

    /// Range element defined by a beginning, end, and step interval

    class Interval_element : public Range_element
    {
        static const int UNDEFINED_END_INDEX = INT_MAX;
    public:

        Interval_element(int begin, int end, int interval) :
            begin_(begin),
            end_(end),
            interval_(interval),
            size_(0)
        {
            size_ = compute_size();
        }

        Interval_element(int begin, End_type, int interval) :
            begin_(begin),
            end_(UNDEFINED_END_INDEX),
            interval_(interval),
            size_(0)
        {
            size_ = compute_size();
        }

        virtual Range_element* clone() const
        {
            return new Interval_element(begin_, end_, interval_);
        }

        virtual size_t operator[](size_t i) const
        {
            int result = begin_ + i * interval_;
            if(interval_ > 0)
            {
                if(end_ < result)
                    KJB_THROW(Index_out_of_bounds);
            }
            else
            {
                if(result < end_)
                    KJB_THROW(Index_out_of_bounds);
            }

            return result;
        }

        virtual size_t size() const
        {
            if(end_ == UNDEFINED_END_INDEX)
                KJB_THROW_2(Runtime_error, "Can't get size of interval with unknown end.  Calling context must handle this.");

            return size_;
        }

        virtual size_t max() const
        {
            if(end_ == UNDEFINED_END_INDEX)
                KJB_THROW_2(Runtime_error, "Can't get max of interval with unknown end.  Calling context must handle this.");

            KJB(UNTESTED_CODE());
            return begin_ + interval_ * (size() - 1);
        }

    protected:
        size_t compute_size() const
        {
            int diff = end_ - begin_;
            return diff / interval_ + 1;
        }

        int begin_;
        int end_;
        int interval_;
        size_t size_;
    };

    /// An interval element that is constructed from a string containing matlab-like syntax for index ranges.
    class Matlab_range : public Interval_element
    {
    public:
        Matlab_range(const std::string& str) :
            Interval_element(0, 0, 1)
        {
            using namespace std;
            // TODO: a real tokenizer would clean up this code

            // subsplit on colons
            // convert colons to whitespce..
            string tmp = str;
            replace(tmp.begin(), tmp.end(), ':', ' ');

            // ...then split on whitespace
            istringstream iss(tmp);

            // thow exceptions, except on eof
//            iss.exceptions(istringstream::failbit | istringstream::badbit);
            iss.exceptions(istringstream::failbit);

            int values[3];
            int i = 0;
            while(!iss.eof())
            {
                if(i >= 3) KJB_THROW_2(Runtime_error, "Parse error: token count exceeded 3.");

                iss >> values[i];
                i++;
            }

            switch(i)
            {
                case 0:
                    KJB_THROW_2(Runtime_error, "Parse error: no tokens found.");
                case 1:
                    begin_ = values[0];
                    end_ = begin_;
                    interval_ = 1;
                    break;
                case 2:
                    begin_ = values[0];
                    end_ = values[1];
                    interval_ = 1;
                    break;
                case 3:
                    begin_ = values[0];
                    interval_ = values[1];
                    end_ = values[2];
                    break;
                default:
                    KJB_THROW(Cant_happen);
            }


            if(interval_ == 0)
            {
                KJB_THROW_2(Illegal_argument, "Parse error: interval_ must be != 0.");
            }

            if(interval_ > 0)
            {
                if(end_ < begin_)
                    KJB_THROW_2(Illegal_argument, "Parse error: sign(interval) != sign(end - begin).");
            }
            else
            {
                if(begin_ < end_)
                    KJB_THROW_2(Illegal_argument, "Parse error: sign(interval) != sign(end - begin).");

            }

            size_ = compute_size();
        }

        virtual Range_element* clone() const
        {
           return new Matlab_range(*this);
        }

    };

private:
    void init_from_str(const std::string& str)
    {
        using namespace std;

        if(str == ":")
        {
            all_ = true;
            return;
        }

        string tmp = str;

        // convert delimeters to whitespace
        replace(tmp.begin(), tmp.end(), ',', ' ');
        replace(tmp.begin(), tmp.end(), ';', ' ');

        // split on whitespace
        istringstream iss(tmp);

        vector<string> tokens;
        copy(istream_iterator<string>(iss), 
             istream_iterator<string>(),
             back_inserter<std::vector<string> >(tokens));

        // convert tokens to Matlab ranges
        ranges_.reserve(tokens.size());
        for(size_t i = 0; i < tokens.size(); i++)
        {
            ranges_.push_back(new Matlab_range(tokens[i]));
        }

    }

protected:
    std::vector<Range_element*> ranges_;
    bool all_;
};

inline void swap(Index_range& r1, Index_range& r2)
{
    r1.swap(r2);
}

/**
 * Read Index range from a text stream.  Assumes string has no whitespace
 */
inline std::istream& operator>>(std::istream& ist, Index_range& ir)
{
    std::string line;
    ist >> line;

    Index_range result(line);
    result.swap(ir);

    return ist;
}

/** @} */

} // namespace kjb
#endif
