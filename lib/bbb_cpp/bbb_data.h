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

/* $Id$ */

#ifndef B3_DATA_H
#define B3_DATA_H

#include "bbb_cpp/bbb_trajectory.h"
#include "l_cpp/l_functors.h"

#include <algorithm>
#include <vector>
#include <iterator>
#include <string>
#include <cstring>
#include <functional>
#include <boost/bind.hpp>

namespace kjb {
namespace bbb {

class Data
{
public:
    /** @brief  Empty data set. */
    Data() {}

    /** @brief  Data set containing given trajectories. */
    template<class TrIt>
    Data(TrIt first, TrIt last) : trajectories_(first, last)
    {
        ids_.resize(trajectories_.size());
        std::generate(ids_.begin(), ids_.end(), Increment<size_t>(0));
    }

    /** @brief  Data set containing given trajectories. */
    template<class TrIt, class IdIt>
    Data(TrIt first, TrIt last, IdIt first_id) : trajectories_(first, last)
    {
        IdIt last_id = first_id;
        std::advance(last_id, std::distance(first, last));
        ids_.assign(first_id, last_id);
    }

    /** @brief  Set trajectories. */
    template<class TrIt, class IdIt>
    void set(TrIt first, TrIt last, IdIt first_id)
    {
        trajectories_.assign(first, last);

        IdIt last_id = first_id;
        std::advance(last_id, std::distance(first, last));
        ids_.assign(first_id, last_id);
    }

public:
    /** @brief  Number of trajectories. */
    size_t size() const
    {
        return trajectories_.size();
    }

    /** @brief  Get kth trajectory. */
    const Trajectory& trajectory(size_t k) const
    {
        return trajectories_[k];
    }

    /** @brief  Get kth trajectory. */
    size_t id(size_t k) const
    {
        return ids_[k];
    }

    /** @brief  Get kth trajectory. */
    size_t index(size_t id) const
    {
        std::vector<size_t>::const_iterator sz_p;
        sz_p = std::find(ids_.begin(), ids_.end(), id);
        return sz_p == ids_.end() ? size() : *sz_p;
    }

    /** @brief  Iterator to first trajectory. */
    std::vector<Trajectory>::const_iterator begin() const
    {
        return trajectories_.begin();
    }

    /** @brief  Iterator to trajectory at one-past-the end. */
    std::vector<Trajectory>::const_iterator end() const
    {
        return trajectories_.end();
    }

    /** @brief  Iterator to first ID. */
    std::vector<size_t>::const_iterator ibegin() const
    {
        return ids_.begin();
    }

    /** @brief  Iterator to ID at one-past-the end. */
    std::vector<size_t>::const_iterator iend() const
    {
        return ids_.end();
    }

    /** @brief  Dimensionality of the data. */
    size_t dimensions() const
    {
        if(trajectories_.empty()) return 0;

        return begin()->dimensions();
    }

    /** @brief  Start frame of the data. */
    size_t start_frame() const
    {
        IFT(!trajectories_.empty(), Runtime_error, "Data is empty.");

        std::vector<Trajectory>::const_iterator tr_p;
        tr_p = min_element(
                    trajectories_.begin(),
                    trajectories_.end(),
                    boost::bind(
                        std::less<size_t>(),
                        boost::bind(&Trajectory::start, _1),
                        boost::bind(&Trajectory::start, _2)));

        return tr_p->start();
    }

    /** @brief  End frame of the data. */
    size_t end_frame() const
    {
        std::vector<Trajectory>::const_iterator tr_p;
        tr_p = max_element(
                    trajectories_.begin(),
                    trajectories_.end(),
                    boost::bind(
                        std::less<size_t>(),
                        boost::bind(&Trajectory::end, _1),
                        boost::bind(&Trajectory::end, _1)));

        return tr_p->end();
    }

private:
    std::vector<Trajectory> trajectories_;
    std::vector<size_t> ids_;
};

/** @brief  Read from file. */
void read(Data& data, const std::string& fname);

/** @brief  Write to file. */
void write(const Data& data, const std::string& fname);

}} // namespace kjb::bbb

#endif /*B3_DATA_H */

