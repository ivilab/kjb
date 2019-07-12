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
   |  Author:  Kyle Simek, Ernesto Brau, Jinyan Guan
 * =========================================================================== */

/* $Id: tracking_trajectory.h 21596 2017-07-30 23:33:36Z kobus $ */

#ifndef TRACKING_TRAJECTORY_H
#define TRACKING_TRAJECTORY_H

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "tracking_cpp/tracking_entity.h"
#include "l_cpp/l_exception.h"
#include "l_cpp/l_word_list.h"
#include "l/l_sys_io.h"
#include "m_cpp/m_vector.h"

#include <string>
#include <map>
#include <algorithm>
#include <vector>
#include <fstream>
#include <iterator>
#include <boost/optional.hpp>
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

/** @file   Classes and functions for dealing with generic trajectories. */

namespace kjb {
namespace tracking {

// forward declarations
template <class T>
class Generic_trajectory;
typedef Generic_trajectory<kjb::Vector> Canonical_trajectory;

template <class T>
class Generic_trajectory_map;
typedef Generic_trajectory_map<kjb::Vector> Canonical_trajectory_map;


/**
 * @brief   Represents an element of a trajectory of a particular entity.
 */
template <class T>
struct Generic_trajectory_element
{
    Generic_trajectory_element() {}

    Generic_trajectory_element(const T& value_) : value(value_) {}

    /**
     * @brief  Reads an element from a line of a file.
     */
    bool parse(const std::string& line)
    {
        // NOT_IMPLEMENTED IN GENERAL
        KJB_THROW(Not_implemented);
    }

    /**
     * @brief  Writes this element to a stream.
     */
    void write(std::ofstream& ofs) const
    {
        // Implementer must specialize this function
        KJB_THROW(Not_implemented);
    }

    /**
     * @brief  Writes this element to a stream.
     */
    static void write_invalid(std::ofstream& ofs)
    {
        // Implementer must specialize this function
        KJB_THROW(Not_implemented);
    }

    /** @brief  Swap two trajectories. */
    friend
    void swap
    (
        Generic_trajectory_element<T>& t1,
        Generic_trajectory_element<T>& t2
    )
    {
        using std::swap;
        swap(t1.value, t2.value);
    }

    T value;
};


/**
 * @brief   Represents a trajectory. Vector of optionals to trajectory elements.
 */
template <class T>
class Generic_trajectory :
    public std::vector<boost::optional<Generic_trajectory_element<T> > >
{
public:
    typedef Generic_trajectory_element<T> Trajectory_element;
    typedef std::vector<boost::optional<Trajectory_element> > Base;

public:

    Generic_trajectory(){}

    Generic_trajectory(size_t size) : Base(size) {}

    Generic_trajectory(double h, double w, double g) :
        height(h), width(w), girth(g) {}

    Generic_trajectory(size_t sz, double h, double w, double g) :
        Base(sz), height(h), width(w), girth(g) {}

    template <class F>
    Canonical_trajectory to_canonical(F to_vector) const;

    /** @brief  Reads a single trajectory from the given file. */
    size_t parse(std::ifstream& ifs);

    /**
     * @brief  Writes a single trajectory to a file.
     */
    void write(const std::string& filename) const;

    /** @brief  Parses the header information from a header line. */
    bool parse_header(const std::string& /*line*/)
    {
        return false;
    }

    /**
     * @brief   Writes the header of a trajectory. Does nothing by
     *          default. Please specialize.
     */
    void write_header(std::ofstream& /*ofs*/) const {}

    /**
     * @brief  Returns index of first valid element.
     */
    int start_time() const;

    /**
     * @brief   Returns index of last valid element.
     */
    int end_time() const;

    /**
     * @brief   Get a vector of the values of this trajectory.
     */
    std::vector<T> get_values() const
    {
        std::vector<T> vals;
        transform(std::back_inserter(vals), boost::lambda::_1);
        return vals;
    }

    /**
     * @brief   Fill a sequence of values obtained from this trajectory.
     */
    template <class OutIt, class Trans>
    void transform(OutIt outp, Trans f) const;

    /** @brief  Swap two trajectories. */
    friend void swap(Generic_trajectory<T>& t1, Generic_trajectory<T>& t2)
    {
        using std::swap;
        swap(t1.id, t2.id);
        swap(t1.height, t2.height);
        swap(t1.width, t2.width);
        swap(t1.girth, t2.girth);
        swap(static_cast<Base&>(t1), static_cast<Base&>(t2));
    }

public:
    Entity_id id;
    double height;
    double width;
    double girth;
};

typedef Generic_trajectory<kjb::Vector> Canonical_trajectory;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class T>
template <class F>
Canonical_trajectory
Generic_trajectory<T>::to_canonical(F to_vector) const
{
    //typedef typename Base::value_type Map_element;
    //typedef typename Canonical_trajectory::Base::value_type Out_element;

    Canonical_trajectory out(
            this->size(),
            this->height,
            this->width,
            this->girth);
    out.id = this->id;

    for(size_t i = 0; i < this->size(); ++i)
    {
        if((*this)[i])
            out[i] = to_vector((*this)[i]->value);
    }

    return out;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class T>
size_t Generic_trajectory<T>::parse(std::ifstream& ifs)
{
    std::string line;
    while(std::getline(ifs, line))
    {
        if(line == "")
        {
            continue;
        }

        if(!parse_header(line))
        {
            Trajectory_element elem;
            if(elem.parse(line))
            {
                this->push_back(elem);
            }
            else
            {
                this->push_back(boost::none);
            }
        }
    }

    return this->size();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class T>
void Generic_trajectory<T>::write(const std::string& filename) const
{
    std::ofstream ofs(filename.c_str()); 
    if(!ofs)
    {
        KJB_THROW_3(IO_error, "Couldn't write to file: %s", (filename.c_str()));
    }

    write_header(ofs);

    for(size_t f = 0; f < this->size(); f++)
    {
        if((*this)[f])
        {
            (*this)[f]->write(ofs);
        }
        else
        {
            Generic_trajectory_element<T>::write_invalid(ofs);
        }
    }

    ofs.close();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class T>
int Generic_trajectory<T>::start_time() const
{   
    size_t sz = this->size();
    for(size_t i = 0; i < sz; i++)
    {
        if((*this)[i])
        {
            return i + 1;
        }
    }

    return -1;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class T>
int Generic_trajectory<T>::end_time() const
{   
    size_t sz = this->size();
    for(int i = static_cast<int>(sz) - 1; i >= 0; i--)
    {
        if((*this)[i])
        {
            return i + 1;
        }
    }

    return -1;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class T>
template <class OutIt, class Trans>
void Generic_trajectory<T>::transform(OutIt outp, Trans f) const
{
    int stime = start_time();
    int etime = end_time();

    if(stime == -1 || etime == -1)
    {
        return;
    }

    size_t stm = stime;
    size_t etm = etime;
    for(size_t i = stm - 1; i <= etm - 1; i++)
    {
        ASSERT((*this)[i]);
        *outp++ = f((*this)[i]->value);
    }
}


/**
 * @brief   Represents a set of trajectories; it is a map from entity to
 *          trajectory.
 */
template <class T>
class Generic_trajectory_map :
    public std::map<Entity_id, Generic_trajectory<T> > 
{
public:
    typedef Generic_trajectory<T> Trajectory;
    typedef Generic_trajectory_element<T> Trajectory_element;
    typedef std::map<Entity_id, Trajectory> Base;

public:
    /** @brief   Constructor. */
    Generic_trajectory_map(size_t duration = 0) : duration_(duration) {}

    /** @brief convert to a "canonical" trajectory of kjb::Vectors.  Used for evaluation. */
    template <class F>
    Canonical_trajectory_map to_canonical(F to_vector) const;

    /** @brief  Reads all trajectories in a directory. */
    void parse(const std::string& path, const std::string& entity);

    /**
     * @brief  Writes this trajectory map to files in directory given by the
     *         string.
     */
    void write(const std::string& dirname) const;

    /** @brief  Returns the duration of this set of trajectories. */
    size_t duration() const
    {
        return duration_;
    }

    /** @brief  Returns the duration of this set of trajectories. */
    size_t& duration()
    {
        return duration_;
    }

    /** @brief  Swap two trajectories. */
    friend
    void swap(Generic_trajectory_map<T>& t1, Generic_trajectory_map<T>& t2)
    {
        using std::swap;
        swap(t1.duration_, t2.duration_);
        swap(static_cast<Base&>(t1), static_cast<Base&>(t2));
    }

private:
    size_t duration_;
};
typedef Generic_trajectory_map<kjb::Vector> Canonical_trajectory_map;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class T>
template <class F>
Canonical_trajectory_map
Generic_trajectory_map<T>::to_canonical(F to_vector) const
{
    typedef typename Base::value_type Map_element;
    //typedef typename Canonical_trajectory_map::Base::value_type Out_map_element;

    Canonical_trajectory_map out_map(this->duration_);

    BOOST_FOREACH(const Map_element& element, *this)
    {
        out_map[element.first] = element.second.to_canonical(to_vector);
    }

    return out_map;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class T>
void Generic_trajectory_map<T>::parse
(
    const std::string& path,
    const std::string& entity
)
{
    using namespace boost;

    if(entity.empty()) return;

    std::string pattern = path + "/" + entity + "_*.txt";
    Word_list word_list(pattern.c_str(), NULL);

    for(size_t i = 0; i < word_list.size(); i++)
    {
        std::string path = word_list[i];

        // file name format is <type>_<index>.txt
        // use this format to extract the entity_id
        std::string::size_type fname_st = path.rfind('/') + 1;
        std::string fname = path.substr(fname_st);

        std::vector<std::string> parts;
        split(parts, fname, is_any_of("_."), token_compress_on);

        IFTD(parts.size() == 3, Illegal_argument,
            "Invalid filename format for track file:", (fname.c_str()));

        Entity_type type = boost::lexical_cast<Entity_type>(parts[0]);
        size_t index = boost::lexical_cast<size_t>(parts[1]);

        Entity_id id(type, index);
        IFT(this->count(id) == 0, Illegal_argument, "Duplicate entity");

        std::ifstream ifs(path.c_str());
        IFTD(ifs, IO_error, "File not found: %s", (path.c_str()));

        Trajectory traj;
        traj.id = id;
        traj.height = get_entity_type_average_height(type);
        traj.width = get_entity_type_average_width(type);
        traj.girth = get_entity_type_average_girth(type);

        if(this->size() == 0)
        {
            //set_duration(traj.parse(ifs));
            duration() = traj.parse(ifs);
        }
        else
        {
            size_t num_frames = traj.parse(ifs);
            IFT(duration() == num_frames, Runtime_error,
                "Trajectories have differing duration.");
        }

        (*this)[id] = traj;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class T>
void Generic_trajectory_map<T>::write(const std::string& dirname) const
{
    kjb_c::kjb_mkdir(dirname.c_str());
    for(typename Generic_trajectory_map<T>::const_iterator it = Base::begin();
                                                           it != Base::end();
                                                           it++)
    {
        boost::format fmt("%s/%s_%d.txt");
        std::string filename = (
            fmt % dirname
                % boost::lexical_cast<std::string>(it->first.type)
                % it->first.index).str();

        ASSERT(duration() == it->second.size());
        it->second.write(filename);
    }
}

}} //namespace kjb::tracking

#endif /*TRACKING_TRAJECTORY_H */

