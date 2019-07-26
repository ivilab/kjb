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

/* $Id: real_data_parser.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_REAL_DATA_PARSER_H 
#define KJB_TIES_REAL_DATA_PARSER_H

#include <vector>
#include <string>
#ifdef KJB_HAVE_CXX11
#include <unordered_map>
#else
#include <map>
#endif

#include "dbn_cpp/data.h"

namespace kjb {
namespace ties {

#ifdef KJB_HAVE_CXX11
typedef std::unordered_map<int, std::vector<Data> > Data_group;
#else
typedef std::map<int, std::vector<Data> > Data_group;
#endif

/**
 * @brief   Parse a .csv file that contains information about the observations,
 *          moderators, distinguisher, categorical-moderators
 */
std::vector<Data> parse_real_data
(
    const std::vector<std::string>& moderators,
    const std::vector<std::string>& observables,
    const std::string& input_file,
    //const std::string& output_dir,
    const std::string& distinguisher,
    Group_map& group_map,
    const std::string& grouping_var = std::string(""),
    size_t num_oscillators = 2,
    bool take_diff_mod = false,
    bool take_ave_mod = false
);

/**
 * @brief   Parse in a .csv file that contains the outcome variables
 */
void parse_outcome_data
(
    std::vector<Data>& data,
    const std::string& input_file,
    const std::string& distinguisher,
    const std::vector<std::string>& outcome_strs
);


Data_group get_data_group
(
    const std::vector<Data>& data_all, 
    const std::string& output_dir,
    const Group_map& group_map
);

}} // namespace kjb::ties

#endif // KJB_TIES_REAL_DATA_PARSER_H

