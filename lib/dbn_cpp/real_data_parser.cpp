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

/* $Id: real_data_parser.cpp 22559 2019-06-09 00:02:37Z kobus $ */

#include "l/l_sys_io.h"
#include "l/l_init.h"
#include <l_cpp/l_exception.h>

#include <boost/assign/std/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>

#ifdef KJB_HAVE_BST_POPTIONS
#include <boost/program_options.hpp>
#else
#error "Need boost program options"
#endif
#include <boost/exception/diagnostic_information.hpp>
#include <boost/foreach.hpp>

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <ostream>
#include <iomanip>

#include "dbn_cpp/real_data_parser.h"
#include "dbn_cpp/util.h"

#define GROUP_HASH 59393925

using namespace kjb;
using namespace kjb::ties;

std::vector<Data> kjb::ties::parse_real_data
(
    const std::vector<std::string>& moderators,
    const std::vector<std::string>& observables,
    const std::string& input_file,
    //const std::string& output_dir,
    const std::string& distinguisher,
    Group_map& group_map,
    const std::string& grouping_var,
    size_t num_oscillators,
    bool take_diff_mod,
    bool take_ave_mod
)
{
    using namespace std;
    std::vector<Data> data_all;
    try
    {
        ifstream ifs(input_file.c_str());
        IFTD(ifs.is_open(), IO_error, "Error opening input file: %s.\n", 
                (input_file.c_str()));

        typedef boost::tokenizer<boost::escaped_list_separator<char> > Tokenizer;
        map<size_t, Data> info;
        // start parsing input file
        string line; 
        // get the first line (header info)
        getline(ifs,line);
        std::vector<std::string> headers;
        boost::trim(line);
        std::cout << "header: " << line << std::endl;
        Tokenizer tok(line);
        headers.assign(tok.begin(),tok.end());
        size_t num_fields = headers.size();
        std::map<string, size_t> header_map;
        for(size_t i = 0; i < num_fields; i++)
        {
            assert(headers[i] != "");
            header_map[headers[i]] = i;
        }
        // Make sure the .csv file has the following field:
        // Dyad/dyad, Time/time, string for the distinguisher
        if(header_map.find("Dyad") == header_map.end() 
                && header_map.find("dyad") == header_map.end())
        {
            KJB_THROW_2(Runtime_error, "Please provide couple's ID "
                    "(column with \"Dyad\" or \"dyad\" as header in your .csv file\n");
        }
        if(header_map.find("Time") == header_map.end() 
                && header_map.find("time") == header_map.end())
        {
            KJB_THROW_2(Runtime_error, "Please provide time counter "
                    "(column with \"Time\" or \"time\" as header in your .csv file\n");
        }
        if(header_map.find(distinguisher) == header_map.end())
        {
            KJB_THROW_3(Illegal_argument, 
                    "distinguisher %s is not provided in the file.",
                    (distinguisher.c_str())); 
        }
        // If grouping_var is provided, make sure the headers has the
        // corresponding string
        if(grouping_var != "")
        {
            if(header_map.find(grouping_var) == header_map.end())
            {
                KJB_THROW_3(Illegal_argument, 
                            "The provided grouping-variable %s "
                            " is not present in data",
                            (grouping_var.c_str()));
            }
        }

        // get the index for the Dyad/dyad
        size_t dyid_index;
        if(header_map.find("Dyad") != header_map.end())
        {
            dyid_index = header_map.at("Dyad");
        }
        else
        {
            assert(header_map.find("dyad") != header_map.end());
            dyid_index = header_map.at("dyad");
        }

        // Get the index for the Time/time
        size_t time_index;
        if(header_map.find("time") != header_map.end())
        {
            time_index = header_map.at("time");
        }
        else
        {
            assert(header_map.find("Time") != header_map.end());
            time_index = header_map.at("Time");
        }

        // Get the distinguisher index
        size_t distinguisher_index = header_map.at(distinguisher);

        // Get the observable indices
        std::vector<size_t> obs_indices(observables.size());
        for(size_t i = 0; i < observables.size(); i++)
        {
            const string& name = observables[i];
            std::map<string, size_t>::const_iterator it = header_map.find(name);
            if(it == header_map.end())
            {
                KJB_THROW_3(Illegal_argument,
                        "the observable %s is not provided in the file.",
                        (name.c_str()));
            }
            obs_indices[i] = header_map[name];
        }

        // Get the moderator indices
        std::vector<size_t> mod_indices(moderators.size());
        for(size_t i = 0; i < moderators.size(); i++)
        {
            const string& name = moderators[i];
            std::map<string, size_t>::const_iterator it = header_map.find(name);
            if(it == header_map.end())
            {
                KJB_THROW_3(Illegal_argument, 
                        "moderator %s is not provided in the file.", 
                        (name.c_str())); 
            }
            size_t index = header_map[name];
            mod_indices[i] = index;
        }

        int prev_time = -1;
        group_map.clear();

        while(getline(ifs, line))
        {
            vector<string> tokens;
            boost::trim(line);
            Tokenizer tok(line);
            tokens.assign(tok.begin(),tok.end());
            assert(tokens.size() == headers.size());
            string dyid_str = tokens[dyid_index];

            string time_str = tokens[time_index];

            if(dyid_str == "NA" || time_str == "NA") continue;
            if(dyid_str == "")
            {
                std::cerr << "[WARNING]: line (" << line << ") doesn't have dyid\n";
                continue;
            }
            if(time_str == "")
            {
                std::cerr << "[WARNING]: " << dyid_str << " has empty time points\n";
                continue;
            }
            size_t dyid = boost::lexical_cast<size_t>(dyid_str);
            //added for parsing data_all.csv file (couple 5 and 25 has the same
            //  distinguisher value ( they're probably the same sex couple)
            //  which will be excluded from the analysis anyway
            //if(dyid == 5 || dyid == 505 || dyid == 25 || dyid == 525) continue;
            double time = boost::lexical_cast<double>(time_str);
            int person_id = boost::lexical_cast<int>(tokens[distinguisher_index]);

            // get the observables over time
            std::vector<double> obs_vals(observables.size());
            for(size_t i = 0; i < observables.size(); i++)
            {
                size_t index = obs_indices[i];
                if(tokens[index] != "NA" && tokens[index] != "")
                {
                    obs_vals[i] = boost::lexical_cast<double>(tokens[index]);
                    const string& name = observables[i];
                    string msg = "observable " + name + " has value " + 
                                 boost::lexical_cast<string>(INT_MAX) +
                                 " (INT_MAX, which has used to represent"
                                 " missing values in the system at time " + 
                                 boost::lexical_cast<string>(time) + "\n"; 
                    IFT(obs_vals[i] < INT_MAX, IO_error, msg); 
                }
                else
                {
                    obs_vals[i] = INT_MAX;
                }
            }

            // get the moderator values
            vector<double> mod_vals(moderators.size());
            bool valid_moderator = true;
            for(size_t i = 0; i < moderators.size(); i++)
            {
                size_t index = mod_indices[i];
                if(tokens[index] == " " || tokens[index] == "NA" || tokens[index] == "")
                {
                    valid_moderator = false;
                    const string& name = moderators[i];
                    std::cerr << "Dyid " << dyid << " " << distinguisher << " " 
                              << person_id << " does not have valid moderator "
                              << name << "\n";
                    break;
                }
                mod_vals[i] = boost::lexical_cast<double>(tokens[index]);
            }
            if(!valid_moderator)
            {
                std::cerr << "Remove Dyad " << dyid 
                          << " due to invalid moderator value\n";
                continue;
            }

            // insert the values to data
            size_t key = dyid;
            size_t group_index = 0;
            if(grouping_var != "")
            {
                std::string group_str = grouping_var + "-" 
                    + tokens[header_map.at(grouping_var)];
                // find the group index 
                if(group_map.empty())
                {
                    // insert the new entry
                    group_map.insert(Group_map_entry(group_index, group_str));
                }
                else
                {
                    // check to see if the group_str is already in the map
                    Group_name_iterator it = group_map.right.find(group_str);
                    if(it == group_map.right.end())
                    {
                        // go back to the previous entry to get the group index
                        it--;
                        // increase the group index
                        group_index = it->second + 1;
                        // insert the pair <group_index, group_str> to the map
                        group_map.insert(Group_map_entry(group_index, group_str));
                    }
                    else
                    {
                        group_index = it->second;
                    }
                }

                key = dyid + group_index * GROUP_HASH;
            }
            map<size_t, Data>::iterator data_it = info.find(key);
            bool new_entry = false;
            if(data_it != info.end())
            {
                // dyid is already in the collection
                Data& data = (*data_it).second;
                // check the consistency of the group variable
                if(grouping_var != "")
                {
                    if(data.group_index != group_index)
                    {
                        new_entry = true;
                        // check to see if the dyad has already in the map
                        string msg = "Dyid " + boost::lexical_cast<string>(dyid) + 
                                     "'s grouping variable changes " +
                                     "at time " + boost::lexical_cast<string>(time);

                    }
                    else
                    {
                        new_entry = false;
                    }
                }
                if(!new_entry)
                {
                    for(size_t j = 0; j < obs_vals.size(); j++)
                    {
                        Obs_map::iterator o_it = data.observables.find(observables[j]);
                        assert(o_it != data.observables.end());
                        if(person_id >= o_it->second.size())
                        {
                            o_it->second.resize(person_id + 1);
                            o_it->second[person_id].push_back(obs_vals[j]);
                            prev_time = -1;
                        }
                        else
                        {
                            o_it->second[person_id].push_back(obs_vals[j]);
                            // check to see if time is strictly increasing
                            // Only check the time for the first observable 
                            if(j == 0)
                            {
                                string msg = "Time is not strictly increasing at time " +
                                             boost::lexical_cast<string>(time) + 
                                             " for dyid " + boost::lexical_cast<string>(dyid) + 
                                             " partner " + boost::lexical_cast<string>(person_id) + 
                                             " (previous time is " + 
                                             boost::lexical_cast<string>(prev_time) + ")";
                                IFT(time > prev_time, IO_error, msg);
                                prev_time = time;
                            }
                        }
                    }

                    for(size_t j = 0; j < mod_vals.size(); j++)
                    {
                        Mod_map::iterator m_it = data.moderators.find(moderators[j]);
                        assert(m_it != data.moderators.end());
                        // second person
                        if(person_id >= m_it->second.size())
                        {
                            m_it->second.push_back(mod_vals[j]);
                        }
                        // first person 
                        else
                        {
                            string msg = "moderator " + m_it->first + " for dyid " + 
                                         boost::lexical_cast<string>(dyid) +  " partner " +
                                         boost::lexical_cast<string>(person_id) + " at time " + 
                                         boost::lexical_cast<string>(time) + " is not same as " +
                                         "previous times";

                            IFT(fabs(m_it->second[person_id] - mod_vals[j]) < FLT_EPSILON,
                                        IO_error, msg);
                            m_it->second[person_id] = mod_vals[j];
                        }
                    }
                    // only add time when it's partner_0
                    // @TODO this might need to be changed since different partners in the
                    // group might have different time, and we need to keep the time
                    // which is the shortest among the partners. partner_0 might not
                    // have the smallest time points. 
                    if(person_id == 0) 
                    {
                        data.times.push_back(time);
                    }
                }
            }
            if(data_it == info.end() || new_entry)
            {
                // create a new data
                Data new_data(dyid);
                prev_time = time;
                if(grouping_var != "")
                {
                    std::string group_str = grouping_var + "-" 
                        + tokens[header_map.at(grouping_var)];
                    size_t group_index = 0;
                    if(group_map.empty())
                    {
                        // insert the new entry
                        group_map.insert(Group_map_entry(group_index, group_str));
                    }
                    else
                    {
                        // check to see if the group_str is already in the map
                        Group_name_iterator it = group_map.right.find(group_str);
                        if(it == group_map.right.end())
                        {
                            // go back to the previous entry to get the group index
                            it--;
                            // increase the group index
                            group_index = it->second + 1;
                            // insert the pair <group_index, group_str> to the map
                            group_map.insert(Group_map_entry(group_index, group_str));
                        }
                        else
                        {
                            group_index = it->second;
                        }
                    }
                    new_data.group_index = group_index; 
                }

                // check to see if person_id starts with 0
                bool skip = false;
                if(person_id != 0)
                {
                    std::string msg = "Remove dyid " + 
                                       boost::lexical_cast<string>(dyid) + " " + 
                                       distinguisher + " " +  
                                       boost::lexical_cast<string>(person_id) + "\n";
                    std::cerr << msg << std::endl;
                    std::cerr << "Please check your .csv file to see if Dyad " 
                              << boost::lexical_cast<string>(dyid)
                              << " " << distinguisher << " 0"
                              << " is provided.\n";
                    continue;
                }
                // observations
                for(size_t i = 0; i < observables.size(); i++)
                {
                    const std::string& name = observables[i];
                    new_data.observables[name] = Vector_v(1);
                    new_data.observables[name].reserve(num_oscillators);
                    new_data.observables.at(name)[person_id].push_back(obs_vals[i]);
                }
                // check to see if the observables are the same 
                double obs_val = obs_vals[0];

                // moderators 
                for(size_t i = 0; i < moderators.size(); i++)
                {
                    const std::string& name = moderators[i];
                    new_data.moderators[name] = Double_v(1);
                    //new_data.moderators[name].reserve(num_oscillators);
                    new_data.moderators.at(name)[person_id] = mod_vals[i];
                }

                new_data.times.push_back(time);

                size_t key = grouping_var == "" ? dyid : dyid + 
                                    new_data.group_index * GROUP_HASH;
                
                //info.insert(pair<size_t, Data>(dyid, new_data));
                info.insert(pair<size_t, Data>(key, new_data));
            }
        }
       
        map<size_t, Data>::iterator it = info.begin();
        for(; it != info.end(); it++)
        {
            Data& data = it->second;
            size_t dyid = data.dyid;
            bool miss_data = false;
            bool valid_num_oscs = true;
            bool duplicated_partner = false;
            for(Obs_map::iterator it = data.observables.begin(); 
                    it != data.observables.end(); it++)
            {
                Vector_v& obs = it->second;
                if(obs.size() != num_oscillators)
                {
                    std::cerr << "Skipping dyid " << dyid << " since it only has " 
                              << obs.size() << " partners" << " which is fewer than " 
                              << num_oscillators << std::endl;
                    valid_num_oscs = false;
                    break;
                }

                size_t min_length = INT_MAX;
                BOOST_FOREACH(const Vector& val, obs)
                {
                    if(val.size() < min_length)
                    {
                        min_length = val.size();
                    }
                }
                // trim the obscillators 
                size_t p = 0;
                BOOST_FOREACH(Vector& val, obs)
                {
                    if(val.size() > min_length)
                    {
                        std::cerr << "Couple " << data.dyid << " partner " 
                                  << p++ << " has longer observables than "
                                  << min_length << std::endl;
                        val.erase(val.begin() + min_length, val.end());
                    }
                }
                // trim the time if the time is greater than the min length
                if(data.times.size() > min_length)
                {
                    data.times.erase(data.times.begin() + min_length, 
                                     data.times.end());
                }

                // count valid data points
                size_t min_valid_length = INT_MAX;
                BOOST_FOREACH(const Vector& val, obs)
                {
                    size_t valid = 0;
                    BOOST_FOREACH(double v, val)
                    {
                        if(!invalid_data(v)) valid++;
                    }
                    if(valid < min_valid_length) min_valid_length = valid;
                }
                if(min_valid_length < data.times.size() * 0.6)
                {
                    // remove the current data 
                    std::cout << "Remove dyad: " << dyid << " (obs "
                              << it->first << " has too many missing data )"
                              << std::endl;
                    miss_data = true;
                }
                size_t same_vals = 0;
                IFT(obs.size() >= 2, Runtime_error, "At least two partner is required\n");
                for(size_t t = 0; t < min_length; t++)
                {
                    if(fabs(obs[0][t] - obs[1][t]) < FLT_EPSILON)
                    {
                        same_vals++;
                    }
                }
                if(same_vals == min_length)
                {
                    duplicated_partner = true;
                    std::cout << "Remove dyad: " << dyid 
                        << " Partner has duplicated values" << std::endl;
                }
            }
            if(valid_num_oscs && !data.times.empty())
            {
                // check to see if data has valid moderator values
                Mod_map::iterator m_it = data.moderators.begin();
                bool invalid = false;
                for(; m_it != data.moderators.end(); m_it++)
                {
                    BOOST_FOREACH(const double& val, m_it->second)
                    {
                        if(invalid_data(val))
                        {
                            invalid = true;
                            break;
                        }
                    }
                    if(invalid) break;
                }
                if(invalid || miss_data || duplicated_partner) continue;


                // Take diff of two moderators 
                if(take_diff_mod && !take_ave_mod)
                {
                    std::cout << "Take diff\n";
                    Mod_map::iterator m_it = data.moderators.begin();
                    for(; m_it != data.moderators.end(); m_it++)
                    {
                        if(m_it->second.size() == 2)
                        {
                            std::cout << m_it->second[0] << " " << m_it->second[1] << " -> ";
                            double diff = m_it->second[0] - m_it->second[1];
                            m_it->second.clear();
                            m_it->second.resize(2, diff);
                            std::cout << diff << "\n";
                        }
                    }
                }
                else if(take_ave_mod && !take_diff_mod) // Take average of two moderators
                {
                    std::cout << "Take ave\n";
                    Mod_map::iterator m_it = data.moderators.begin();
                    for(; m_it != data.moderators.end(); m_it++)
                    {
                        if(m_it->second.size() == 2)
                        {
                            double ave = (m_it->second[0] + m_it->second[1])/2.0;
                            m_it->second.clear();
                            m_it->second.resize(2, ave);
                        }
                    }
                }
                else if(take_diff_mod && take_ave_mod)
                {
                    std::cout << "Take ave diff\n";
                    Mod_map::iterator m_it = data.moderators.begin();
                    for(; m_it != data.moderators.end(); m_it++)
                    {
                        if(m_it->second.size() == 2)
                        {
                            double diff = m_it->second[0] + m_it->second[1];
                            double ave = (m_it->second[0] + m_it->second[1])/2.0;
                            m_it->second[0] = diff;
                            m_it->second[1] = ave;
                        }
                    }
                }
                
                // valid data 
                data_all.push_back(data);
            }

        }
        return data_all;
    }
    catch(Option_exception& e)
    {
        cerr << e.get_msg() << endl;
        exit(1);
    }
    catch(IO_error& e)
    {
        cerr << e.get_msg() << endl;
        exit(1);
    }
    catch(Exception& e)
    {
        cerr << "KJB Exception: " << e.get_msg() << endl;
        exit(1);
    }
    catch(exception& e)
    {
        cerr << "STD Exception: " << e.what() << endl;
        exit(1);
    }
    catch(boost::exception& e)
    {
        cerr << "boost exception: "
            << boost::diagnostic_information(e) << endl;
        exit(1);
    }
    catch(...)
    {
        cerr << "Unknown error occurred" << endl;
        exit(1);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::parse_outcome_data
(
    std::vector<Data>& data,
    const std::string& input_file,
    const std::string& distinguisher,
    const std::vector<std::string>& outcome_strs
)
{
    using namespace std;
    // build a map <Dyid, index>
    std::map<size_t, size_t> dyid_map;
    for(size_t i = 0; i < data.size(); i++)
    {
        dyid_map[data[i].dyid] = i;
    }

    std::vector<size_t> invalid_indices;
    try
    {
        ifstream ifs(input_file.c_str());
        IFTD(ifs.is_open(), IO_error, "Error opening input file: %s.\n", 
                (input_file.c_str()));

        typedef boost::tokenizer<boost::escaped_list_separator<char> > Tokenizer;
        // Start parsing input file
        string line; 
        // Get the first line (header info)
        getline(ifs,line);
        std::vector<std::string> headers;
        boost::trim(line);
        Tokenizer tok(line);
        headers.assign(tok.begin(),tok.end());
        size_t num_fields = headers.size();
        std::map<string, size_t> header_map;
        for(size_t i = 0; i < num_fields; i++)
        {
            assert(headers[i] != "");
            header_map[headers[i]] = i;
        }

        // Make sure the .csv file has the field Dyad/dyad
        if(header_map.find("Dyad") == header_map.end() 
                && header_map.find("dyad") == header_map.end())
        {
            KJB_THROW_2(Runtime_error, "Please provide couple's ID "
                "(column with \"Dyad\" or \"dyad\" as header "
                "in your outcome .csv file\n");
        }
        // Make sure the .csv file has the distinguisher 
        if(header_map.find(distinguisher) == header_map.end())
        {
            KJB_THROW_3(Illegal_argument, 
                    "distinguisher %s is not provided in the file.",
                    (distinguisher.c_str())); 
        }
        // get the index for the Dyad/dyad
        size_t dyid_index;
        if(header_map.find("Dyad") != header_map.end())
        {
            dyid_index = header_map.at("Dyad");
        }
        else
        {
            assert(header_map.find("dyad") != header_map.end());
            dyid_index = header_map.at("dyad");
        }
        // Get the distinguisher index
        size_t distinguisher_index = header_map.at(distinguisher);

        // Get the outcome indices
        std::vector<size_t> outcome_indices(outcome_strs.size());
        for(size_t i = 0; i < outcome_strs.size(); i++)
        {
            const string& name = outcome_strs[i];
            std::map<string, size_t>::const_iterator it = header_map.find(name);
            if(it == header_map.end())
            {
                KJB_THROW_3(Illegal_argument,
                        "the outcome %s is not provided in the outcome .csv file. ",
                        (name.c_str()));
            }
            outcome_indices[i] = header_map[name];
        }

        while(getline(ifs, line))
        {
            // get the tokens of the current line
            vector<string> tokens;
            boost::trim(line);
            Tokenizer tok(line);
            tokens.assign(tok.begin(),tok.end());
            assert(tokens.size() == headers.size());

            // find the outcome values
            string dyid_str = tokens[dyid_index];
            if(dyid_str == "")
            {
                std::cerr << "[WARNING]: line (" << line << ") doesn't have dyid\n";
                continue;
            }
            size_t dyid = boost::lexical_cast<size_t>(dyid_str);
            std::map<size_t, size_t>::const_iterator id_it = dyid_map.find(dyid);
            if(id_it == dyid_map.end())
            {
                std::cerr << "[WARNING]: dyid " << dyid << " in the outcome .csv file "
                          << "does not exist in the data." << std::endl;
                continue;
            }
            int person_id = boost::lexical_cast<int>(tokens[distinguisher_index]);

            // Get the outcome vals
            vector<double> outcome_vals(outcome_strs.size());
            bool valid_outcome = true;
            for(size_t i = 0; i < outcome_strs.size(); i++)
            {
                size_t index = outcome_indices[i];
                if(tokens[index] == " " || tokens[index] == "NA" || tokens[index] == "")
                {
                    valid_outcome = false;
                    std::cerr << "Dyid " << dyid << " " << distinguisher << " " 
                              << person_id << " does not have valid outcome "
                              << outcome_strs[i] << "\n";
                    break;
                }
                outcome_vals[i] = boost::lexical_cast<double>(tokens[index]);
            }
            if(!valid_outcome)
            {
                std::cerr << "Reomve Dyad " << dyid 
                          << " due to invalid outcome value\n";
                continue;
            }

            // insert the outcome vals into data
            size_t data_index = id_it->second;
            assert(data_index < data.size());
            Data& cur_data = data[data_index];
            for(size_t j = 0; j < outcome_vals.size(); j++)
            {
                const string& name = outcome_strs[j];
                cur_data.outcomes[name].push_back(outcome_vals[j]);
            }
        }

        // remove invalid 
        size_t i = 0;
        size_t num_oscillators = data.front().observables.begin()->second.size();
        std::cout << "num oscillators: " << num_oscillators << std::endl;
        std::cout << "data: " << data.size() << std::endl;
        while(i < data.size())
        {
            size_t num_outcome = data[i].outcomes.empty() ? 0 :
                                 data[i].outcomes.begin()->second.size();
            std::cout << data[i].dyid << " " << num_outcome << std::endl;
            if(num_outcome != num_oscillators)
            {
                data[i] = data.back();
                data.pop_back();
            }
            else
            { 
                i++;
            }
        }
        std::cout << "after data: " << data.size() << std::endl;
    }
    catch(Option_exception& e)
    {
        cerr << e.get_msg() << endl;
        exit(1);
    }
    catch(IO_error& e)
    {
        cerr << e.get_msg() << endl;
        exit(1);
    }
    catch(Exception& e)
    {
        cerr << "KJB exception: " << e.get_msg() << endl;
        exit(1);
    }
    catch(exception& e)
    {
        cerr << "STD exception: " << e.what() << endl;
        exit(1);
    }
    catch(boost::exception& e)
    {
        cerr << "boost boost exception: "
            << boost::diagnostic_information(e) << endl;
        exit(1);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Data_group kjb::ties::get_data_group
(
    const std::vector<Data>& data_all, 
    const std::string& output_dir,
    const Group_map& group_map
)
{
    using namespace std;
    Data_group data_all_groups;
    BOOST_FOREACH(const Data& data, data_all)
    {
        size_t group_index = data.group_index;
        if(data_all_groups.find(group_index) == data_all_groups.end())
        {
            data_all_groups[group_index] = vector<Data>();
        }
        Data_group::iterator entry_it = data_all_groups.find(group_index);
        entry_it->second.push_back(data);
    }

    if(output_dir != "")
    {
        ofstream touch((output_dir + "/invocation.txt").c_str());
        touch << "Program invocation:\n" << "get_data_group" << ' ' 
              << ' ' << output_dir << '\n';
        Data_group::const_iterator entry_it = data_all_groups.begin();
        for(; entry_it != data_all_groups.end(); entry_it++)
        {
            size_t group_index = entry_it->first;
            string group_tag = group_map.left.find(group_index)->second; 
            string group_out_dir = output_dir + "/" + group_tag;
            write_data(entry_it->second, group_out_dir);
            // output the id lists
            string id_list_fp(group_out_dir + "/ids.txt");
            ofstream id_ofs(id_list_fp.c_str());
            BOOST_FOREACH(const Data& data, entry_it->second)
            {
                id_ofs << data.dyid << endl;
            }
        }
        std::string invalid_fp(output_dir + "/invalid.txt");
        std::ofstream ofs(invalid_fp.c_str());
        ofs << INT_MAX << std::endl;
    }
    return data_all_groups;
}
