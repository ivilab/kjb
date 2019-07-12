/* =========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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

#ifndef MCMCDA_DATA_H_INCLUDED
#define MCMCDA_DATA_H_INCLUDED

#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <iostream>
#include <functional>
#include <l_cpp/l_exception.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <fstream>

namespace kjb {
namespace mcmcda {

/**
 * @brief   A class that holds data for the tracking problem.
 *
 * It is a std::vector of sets of the association type. Each set represents
 * the points at a given time, where the index gives the time step. In
 * other words, if D is of type Data, D[t] is the data points
 * found at time t.
 */
template<class Element>
class Data : private std::vector<std::set<Element> >
{
public:
    typedef boost::function1<Vector, const Element&> Convert;

private:
    typedef std::set<Element> E_set;
    typedef std::vector<E_set> Parent;

public:
    /** @brief  Empty constructor. */
    Data() {}

    /**
     * @brief   Constructor from sequence
     *
     * Copies the track set from iterator sequence
     */
    template<class Iterator>
    Data(Iterator first, Iterator last) : Parent(first, last) {}

    // make these public for use
    using Parent::empty;
    using Parent::begin;
    using Parent::end;
    using Parent::clear;
    using Parent::size;
    using Parent::resize;
    using Parent::reserve;
    using Parent::push_back;
    using Parent::operator[];
    using Parent::operator=;
    typedef typename Parent::iterator iterator;
    typedef typename Parent::const_iterator const_iterator;

    /**
     * @brief   Reads data from files with given names.
     */
    void read(const std::vector<std::string>& filenames)
    {
        IFT(filenames.size() > 1, Illegal_argument,
            "Read data from file: must have at least two time steps.");

        clear();
        resize(filenames.size());
        std::transform(filenames.begin(), filenames.end(), begin(),
            boost::bind(&Data<Element>::read_single_time, this, _1));
    }

    /**
     * @brief   Write data to files with given names.
     */
    void write(const std::vector<std::string>& filenames) const;

    /**
     * @brief   Read data elements from a single time-step, contained in a
     *          single file.
     */
    virtual E_set read_single_time(const std::string& /*filename*/) const
    {
        KJB_THROW_2(Not_implemented, "read_single_time: not defined in general");
        return E_set();
    }

    /**
     * @brief   Read data elements from a single time-step, contained in a
     *          single file.
     */
    virtual void write_single_time(const E_set&, const std::string&) const
    {
        KJB_THROW_2(Not_implemented,
                    "write_single_time: not defined in general");
    }

    /**
     * @brief   Checks whether this data set is completely empty.
     * 
     */
    bool is_completely_empty() const
    {
        using namespace std;

        E_set empty_set;
        const_iterator eset_p = find_if(
                                    begin(),
                                    end(),
                                    boost::bind(
                                        not_equal_to<E_set>(),
                                        _1,
                                        boost::cref(empty_set)));

        return eset_p == end();
    }

    /**
     * @brief   Computes the neighborhood of a point. See MCMCDA
     *          paper for details.
     */
    std::set<const Element*> neighborhood
    (
        const Element& y,
        int t,
        int d,
        int d_bar,
        double v_bar,
        double sg,
        const Convert& to_vector
    ) const;

    /**
     * @brief   Computes the size of the neighborhood of a point. This
     *          should be faster than computing the neighborhood itself
     *          and getting its size().
     */
    int neighborhood_size
    (
        const Element& y,
        int t,
        int d,
        int d_bar,
        double v_bar,
        double sg,
        const Convert& to_vector
    ) const;
};

/**
 * @brief   Returns true if given point is a neighbor of second given point.
 */

template<class Element>
inline bool is_neighbor
(
    const Element& y,
    const Element& y_p,
    int d,
    int d_bar,
    double v_bar,
    double sg,
    const typename Data<Element>::Convert& to_vector
)
{
    if(std::abs(d) > d_bar)
    {
        return false;
    }

    if(vector_distance(to_vector(y), to_vector(y_p))
                            <= std::abs(d)*v_bar + 2*sqrt(sg))
    {
        return true;
    }

    return false;
}

/*============================================================================*
 *                      MEMBER FUNCTION DEFINITIONS                           *
 *----------------------------------------------------------------------------*/

template<class Element>
void Data<Element>::write(const std::vector<std::string>& filenames) const
{
    const size_t sz = size();
    IFT(filenames.size() == sz, Illegal_argument,
        "Write data to file: wrong number of files.");

    for(size_t t = 0; t < sz; t++)
    {
        write_single_time((*this)[t], filenames[t]);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Element>
std::set<const Element*> Data<Element>::neighborhood
(
    const Element& y,
    int t,
    int d,
    int d_bar,
    double v_bar,
    double sg,
    const Convert& to_vector
) const
{
    std::set<const Element*> hood;

    IFTD(t >= 0, Illegal_argument,
         "neighborhood: t=%d cannot be negative.", (t));

    if(t + d >= static_cast<int>(size()) || t + d < 1 || std::abs(d) > d_bar)
    {
        return hood;
    }
    const E_set& Y_t = (*this)[t - 1 + d];
    for(typename E_set::const_iterator p = Y_t.begin();
                                       p != Y_t.end();
                                       p++)
    {
        if(is_neighbor(y, *p, d, d_bar, v_bar, sg, to_vector))
        {
            hood.insert(&(*p));
        }
    }

    return hood;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Element>
int Data<Element>::neighborhood_size
(
    const Element& y,
    int t,
    int d,
    int d_bar,
    double v_bar,
    double sg,
    const Convert& to_vector
) const
{
    int hood_size = 0;

    IFTD(t >= 0, Illegal_argument,
         "neighborhood: t=%d cannot be negative.", (t));

    if(t + d >= static_cast<int>(size()) || t + d < 0 || std::abs(d) > d_bar)
    {
        return hood_size;
    }

    const E_set& Y_t = (*this)[t - 1 + d];
    for(typename E_set::const_iterator p = Y_t.begin();
                                       p != Y_t.end();
                                       p++)
    {
        if(is_neighbor(y, *p, d, d_bar, v_bar, sg, to_vector))
        {
            hood_size++;
        }
    }

    return hood_size;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/** @brief  Useful specialization for kjb::Vector. */
template<>
inline
std::set<Vector> Data<Vector>::read_single_time(const std::string& filename) const
{
    std::set<Vector> data_t;
    Matrix M(filename);
    M.get_all_rows(std::inserter(data_t, data_t.begin()));

    return data_t;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/** @brief  Useful specialization for kjb::Vector. */
template<>
inline
void Data<Vector>::write_single_time
(
    const E_set& data_t,
    const std::string& filename
) const
{
    std::ofstream ofs(filename.c_str());
    if(ofs.fail())
    {
        KJB_THROW_3(IO_error, "can't open file %s", (filename.c_str()));
    }

    std::copy(data_t.begin(), data_t.end(),
              std::ostream_iterator<Vector>(ofs, "\n"));
}

}} //namespace kjb::mcmcda

#endif /*MCMCDA_DATA_H_INCLUDED */

