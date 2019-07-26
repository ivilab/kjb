/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
| Authors:
|     Jinyan Guan
|
* =========================================================================== */

/* $Id: data.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_DATA_H
#define KJB_TIES_DATA_H

#include <m_cpp/m_vector.h>
#include <l_cpp/l_filesystem.h>

#include <vector>
#include <string>
#include <cmath>
#include <cfloat>
#include <algorithm>
#include <fstream>
#include <iostream>

#include <boost/assign/std/vector.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include "dbn_cpp/typedefs.h"

namespace kjb {
namespace ties {

// Double_v indexed by [oscillator_index]
typedef std::map<std::string, Double_v> Mod_map;

// Vector_v indexed by [oscillator_index][time-index]
typedef std::map<std::string, Vector_v> Obs_map;

typedef boost::bimap<size_t, std::string> Group_map;
typedef Group_map::value_type Group_map_entry;
typedef Group_map::left_map Group_map_by_index;
typedef Group_map::left_map::const_iterator Group_index_iterator;
typedef Group_map::right_map Group_map_by_name;
typedef Group_map::right_map::const_iterator Group_name_iterator;

/**
 * @struct  A struct represents the date over time.
 */
struct Data
{
    Data(size_t id = -1) 
        : dyid(id), group_index(0), max_val(DBL_MAX), min_val(-DBL_MAX)  {}

    size_t dyid;
    size_t group_index;

    Mod_map moderators;
    Obs_map observables;
    Mod_map outcomes;

    Double_v times;

    std::vector<int> regimes;
    double outcome;
    double max_val;
    double min_val;

    /**
     * @brief  Returns true if the type of data is invalid (such as missing)
     */
    size_t invalid_num_data(const std::string& type, size_t index) const
    {
        return std::count(observables.at(type).at(index).begin(), 
                          observables.at(type).at(index).end(), INT_MAX);
    }
};

/**
 * @brief  Returns true if the data is invalid (such as missing)
 */
inline bool invalid_data(double value)
{
    return fabs(INT_MAX - value) < DBL_EPSILON;
}

/** @brief  Reads in the couple list from a file. */
std::vector<size_t> parse_list
(
    const std::string& id_list_fp
);

/** @brief  Reads in the couple list from a list of files. */
inline std::vector<size_t> parse_list
(
    const std::vector<std::string>& id_list_fps
)
{
    std::vector<size_t> ids;
    for(size_t i = 0; i < id_list_fps.size(); i++)
    {
        std::vector<size_t> id = parse_list(id_list_fps[i]);
        std::copy(id.begin(), id.end(), std::back_inserter(ids));
    }
    return ids;
}

/** @brief  Parse in data from a input file. */
Data parse_data
(
    const std::string& fname
);

/** 
 * @brief  Parse in all the data from directory data_dir with IDs in list_fp.
 * 
 **/
std::vector<Data> parse_data_from_dir
(
    const std::string& data_base_dir,
    const std::vector<size_t>& ids = std::vector<size_t>()
);

/** 
 * @brief  Parse in all the data from directory data_dir with IDs in list_fp.
 * 
 **/
std::vector<Data> parse_data
(
    const std::string& data_dir, 
    const std::string& list_fp,
    const std::string& grouping_var = std::string("")
);

/** 
 * @brief  Parse in all the data from directory data_dir with IDs in list_fp.
 * 
 **/
std::vector<Data> parse_data
(
    const std::string& data_dir, 
    const std::vector<std::string>& list_fps,
    const std::string& grouping_var = std::string("")
);

/** @brief  Writes data to an output file. */
void write_data
(
    const Data& data, 
    const std::string& fname,
    size_t exclude_percent = 0
);

/** @brief  Write the group information. */
inline void write_group(const Group_map& group_map, const std::string& output_dir)
{
    std::string group_info_fp(output_dir + "/group_info.txt");
    std::ofstream group_ofs(group_info_fp.c_str());
    IFTD(group_ofs.is_open(), IO_error, 
            "Can't open file %s", (group_info_fp.c_str()));
    for(Group_index_iterator it = group_map.left.begin();
            it != group_map.left.end();
            it++)
    {
        group_ofs << it->first << ": " << it->second << std::endl;
    }
}

/**
 * @brief   Write a vector of data into the output directory
 */
inline void write_data
(
    const std::vector<Data>& all_data, 
    const std::string& out_dir,
    size_t exclude_percent = 0
)
{
    boost::format out_fmt(out_dir + "/%04d.txt");
    ETX(kjb_c::kjb_mkdir(out_dir.c_str()));
    for(size_t i = 0; i < all_data.size(); i++)
    {
        size_t dyid = all_data[i].dyid;
        std::string out_fname = (out_fmt % dyid).str();
        write_data(all_data[i], out_fname, exclude_percent);
    }
    std::string id_list_fp(out_dir + "/ids.txt");
    std::ofstream id_ofs(id_list_fp.c_str());
    BOOST_FOREACH(const Data& data, all_data)
    {
        id_ofs << data.dyid << std::endl;
    }
    std::string invalid_fp(out_dir + "/invalid.txt");
    std::ofstream ofs(invalid_fp.c_str());
    ofs << INT_MAX << std::endl;
}

/**
 * @brief   Parse in a Group_map from an input stream 
 */
inline std::istream& operator>> 
(
    std::istream& in,
    Group_map& group_map
)
{
    std::string line;
    while(getline(in, line))
    {
        std::vector<std::string> tokens;
        boost::trim(line);
        boost::split(tokens, line, boost::is_any_of(": \""), 
                    boost::token_compress_on);
        assert(tokens.size() == 2);
        size_t group_index = boost::lexical_cast<size_t>(tokens[0]);
        std::string group_id = tokens[1];
        group_map.insert(Group_map_entry(group_index, group_id));
    }
    return in;
}

inline std::vector<std::string> get_group_dirs
(
    const std::string& dir,
    const Group_map& group_map
)
{
    std::vector<std::string> dirs;
    BOOST_FOREACH(Group_map::left_const_reference p, group_map.left)
    {
        std::string cur_dp(dir + "/" + p.second);
        dirs.push_back(cur_dp);
    }
    return dirs;
}

/**
 * @brief   Get the group directories 
 */
inline std::vector<std::string> get_data_group_dirs
(
    const std::string& data_dp
)
{
    IFTD(kjb_c::is_directory(data_dp.c_str()), Illegal_argument, 
            "Data directory %s does not exit.", (data_dp.c_str()));
    std::string group_info_fp(data_dp + "/group_info.txt");
    if(!kjb_c::is_file(group_info_fp.c_str()))
    {
        return std::vector<std::string>();
    }
    std::ifstream group_ifs(group_info_fp.c_str());
    IFTD(group_ifs.is_open(), IO_error, 
            "Can't open file %s", (group_info_fp.c_str()));
    Group_map group_map;
    group_ifs >> group_map;
    return get_group_dirs(data_dp, group_map);
}

inline void print_group(const Group_map& group_map)
{
    //BOOST_FOREACH(const Group_map_entry& value, group_map.left)
    for(Group_index_iterator it = group_map.left.begin();
            it != group_map.left.end();
            it++)
    {
        std::cout << it->first << ": " << it->second << std::endl;
    }
}

}}
#endif

