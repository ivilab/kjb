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

/* $Id: data.cpp 22559 2019-06-09 00:02:37Z kobus $ */

#include <l/l_sys_def.h>
#include <l/l_sys_debug.h>
#include <l/l_sys_io.h>
#include <l_cpp/l_util.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_filesystem.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include <vector>
#include <string>
#include <map>
#include <ios>
#include <fstream>
#include <ostream>
#include <iostream>
#include <iomanip>

#include "dbn_cpp/data.h"

using namespace kjb;
using namespace kjb::ties;

Data kjb::ties::parse_data(const std::string& fname)
{
    using namespace std;
    ifstream ifs(fname.c_str());
    IFTD(ifs.is_open(), IO_error, "Can't open file %s", (fname.c_str()));

    string line;
    vector<string> tokens;
    // Parse dyid
    getline(ifs, line);
    boost::trim(line);
    boost::split(tokens, line, boost::is_any_of(" :"), boost::token_compress_on);
    KJB(ASSERT(tokens.size() == 2));
    KJB(ASSERT(tokens[0] == "dyid"));
    size_t dyid = boost::lexical_cast<size_t>(tokens[1]);
    
    // create data
    Data data(dyid);

    // parse the number of oscillators 
    getline(ifs, line);
    boost::trim(line);
    boost::split(tokens, line, boost::is_any_of(" :"), 
                               boost::token_compress_on);
    KJB(ASSERT(tokens.size() == 2));
    KJB(ASSERT(tokens[0] == "oscillators"));
    size_t num_oscillators = boost::lexical_cast<size_t>(tokens[1]);

    // parse moderators 
    getline(ifs, line);
    boost::trim(line);
    boost::split(tokens, line, boost::is_any_of(" :"), 
                               boost::token_compress_on);
    string name; 
    KJB(ASSERT(tokens[0] == "moderators"));
    int t = 1;
    while(t < tokens.size())
    {
        name = tokens[t++];
        if(t >= tokens.size()) break;
        for(size_t i = 0; i < num_oscillators; i++)
        {
            double m = boost::lexical_cast<double>(tokens[t++]);
            data.moderators[name].push_back(m);
        }
    }

    // parse outcomes if it's present
    getline(ifs, line);
    boost::trim(line);
    boost::split(tokens, line, boost::is_any_of(" :"), 
                               boost::token_compress_on);
    bool parse_outcomes = false;
    if(tokens[0] == "outcomes")
    {
        t = 1;
        while(t < tokens.size())
        {
            name = tokens[t++];
            if(t >= tokens.size()) break;
            for(size_t i = 0; i < num_oscillators; i++)
            {
                double m = boost::lexical_cast<double>(tokens[t++]);
                data.outcomes[name].push_back(m);
            }
        }
        parse_outcomes = true;
    }

    // parse observables 
    if(parse_outcomes)
    {
        getline(ifs, line);
        boost::trim(line);
        boost::split(tokens, line, boost::is_any_of(" :"), 
                                   boost::token_compress_on);
    }
    bool parse_group = false;
    if(tokens[0] == "group")
    {
        KJB(ASSERT(tokens.size() == 2));
        data.group_index = boost::lexical_cast<size_t>(tokens[1]);
        parse_group = true;
    }

    if(parse_group)
    {
        getline(ifs, line);
        boost::trim(line);
        boost::split(tokens, line, boost::is_any_of(" :"), 
                                   boost::token_compress_on);
    }
    KJB(ASSERT(tokens[0] == "observables"));
    t = 1;
    while(t < tokens.size())
    {
        data.observables[tokens[t++]] = Vector_v(num_oscillators);
    }

    // initialize the max_val and min_val
    data.max_val = -DBL_MAX;
    data.min_val = DBL_MAX;
    // skip the header of time and observables 
    getline(ifs, line);
    while(getline(ifs, line))
    {
        boost::trim(line);
        boost::split(tokens, line, boost::is_any_of(" "), 
                                   boost::token_compress_on);
        t = 0;
        data.times.push_back(boost::lexical_cast<double>(tokens[t++]));
        Obs_map::iterator obs_it = data.observables.begin();
        //size_t osc_index = 0;
        while(t < tokens.size())
        {
            int osc_index = (t - 1) % num_oscillators;
            double val = boost::lexical_cast<double>(tokens[t++]);
            obs_it->second[osc_index].push_back(val);
            if((t-1) % num_oscillators == 0) obs_it++;

            // check the max and min value of the data 
            if(val > data.max_val)
            {
                data.max_val = val;
            }
            if(val < data.min_val)
            {
                data.min_val = val;
            }
        }
    }

    return data;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<size_t> kjb::ties::parse_list
(
    const std::string& id_fname
)
{
    std::vector<size_t> lists;
    std::ifstream ifs(id_fname.c_str());
    IFTD(!ifs.fail(), IO_error, "Can't open file %s", (id_fname.c_str()));
    std::string line;
    while(std::getline(ifs, line))
    {
        if(!line.empty())
            lists.push_back(boost::lexical_cast<size_t>(line));
    }
    return lists;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Data> kjb::ties::parse_data
(
    const std::string& data_dir, 
    const std::string& list_fp,
    const std::string& grouping_var
)
{
    std::vector<Data> data;
    if(grouping_var == "")
    {
        std::vector<size_t> ids;
        ids = parse_list(list_fp);
        data = parse_data_from_dir(data_dir, ids);
    }
    else
    {
        std::string group_fmt(data_dir + "/" + grouping_var + "-%d/");
        std::vector<std::string> group_dirs = get_data_group_dirs(data_dir);
        for(size_t i = 0; i < group_dirs.size(); i++)
        {
            std::string group_list = group_dirs[i] + "/ids.txt";
            std::vector<size_t> ids = parse_list(group_list);
            std::vector<Data> cur_data_group = parse_data_from_dir(group_dirs[i], ids);
            std::copy(cur_data_group.begin(), cur_data_group.end(), std::back_inserter(data));
        }
    }
    return data;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Data> kjb::ties::parse_data_from_dir
(
    const std::string& data_base_dir,
    const std::vector<size_t>& ids
)
{
    size_t num_lss = ids.size();
    std::vector<size_t> default_ids;
    if(ids.empty())
    {
        std::string id_list_fp(data_base_dir + "/ids.txt");
        default_ids = parse_list(id_list_fp);
        num_lss = default_ids.size();
    }
    boost::format data_fmt(data_base_dir + "/%04d.txt"); 
    std::vector<Data> data;
    data.reserve(num_lss);
    for(size_t i = 0; i < num_lss; i++)
    {
        std::string fname = std::string("");
        if(!ids.empty())
        {
            fname = (data_fmt % ids[i]).str();
        }
        else
        {
            fname = (data_fmt % default_ids[i]).str();
        }
        if(kjb_c::is_file(fname.c_str()))
        {
            // Read in data
            //std::cout << "Read in data: " << fname << std::endl;
            data.push_back(parse_data(fname));
        }
        else
        {
            std::cerr << "[WARNING]: " << fname << " does not exist.\n"; 
        }
    }
    return data;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Data> kjb::ties::parse_data
(
    const std::string& data_dir, 
    const std::vector<std::string>& list_fps,
    const std::string& grouping_var
)
{
    std::vector<Data> res;
    std::string group_fmt(data_dir + "/" + grouping_var + "-%d/");
    std::vector<std::string> group_dirs = get_data_group_dirs(data_dir);
    for(size_t i = 0; i < list_fps.size(); i++)
    {
        std::string cur_dir = data_dir;
        if(!group_dirs.empty()) cur_dir = group_dirs[i];
        std::vector<Data> cur_data = parse_data(cur_dir, list_fps[i]);
        std::copy(cur_data.begin(), cur_data.end(), std::back_inserter(res));
    }

    return res;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::write_data
(
    const Data& data, 
    const std::string& fname, 
    size_t exclude_percent
)
{
    using namespace std;

    ofstream ofs(fname.c_str());
    IFTD(ofs.is_open(), IO_error, "Can't open file %s\n", (fname.c_str()));

    // dyid 
    ofs << "dyid: " << data.dyid << std::endl; 

    // if there are no observables, return 
    if(data.observables.empty()) return;

    // number of oscillators 
    size_t num_oscillators = 0;
    if(!data.observables.empty())
    {
        num_oscillators = data.observables.begin()->second.size();
    }
    ofs << "oscillators: " << num_oscillators << std::endl;

    // moderators 
    ofs << "moderators: "; 
    BOOST_FOREACH(const Mod_map::value_type& val, data.moderators)
    {
        ofs << setw(10) << val.first << " ";
        BOOST_FOREACH(const double& mod_val, val.second)
        {
            ofs << setw(15) << setprecision(10) << mod_val << " ";
        }
    }
    ofs << endl;

    // outcomes
    if(!data.outcomes.empty())
    {
        ofs << "outcomes: "; 
        BOOST_FOREACH(const Mod_map::value_type& val, data.outcomes)
        {
            ofs << setw(10) << val.first << " ";
            BOOST_FOREACH(const double& mod_val, val.second)
            {
                ofs << setw(15) << setprecision(10) << mod_val << " ";
            }
        }
        ofs << endl;
    }

    // cluster
    ofs << "group: ";
    ofs << data.group_index << endl;

    // observables 
    ofs << "observables: ";  
    BOOST_FOREACH(const Obs_map::value_type& val, data.observables)
    {
        ofs << val.first << " ";
    }
    ofs << endl;

    ofs << setw(4) << "time";
    BOOST_FOREACH(const Obs_map::value_type& val_type, data.observables)
    {
        for(size_t i = 0; i < num_oscillators; i++)
        {
            ofs << setw(18) << val_type.first << "-" << i;
        }
    }
    ofs << endl;
    size_t num_data = data.times.size();

    //Excluding time points here. 
    assert(exclude_percent < 100);
    int exclude_time_points = num_data * exclude_percent / 100;

    num_data -= exclude_time_points;

    for(size_t i = 0; i < num_data; i++)
    {
        ofs << setw(4) << data.times[i]; 
        BOOST_FOREACH(const Obs_map::value_type& val_type, data.observables)
        {
            BOOST_FOREACH(const Vector& val, val_type.second)
            {
                ofs << setw(20) << setprecision(10) << val[i];
            }
        }
        ofs << endl;
    }
}

