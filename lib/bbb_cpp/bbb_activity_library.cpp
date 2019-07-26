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

#include "bbb_cpp/bbb_activity_library.h"
#include "l_cpp/l_exception.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_sample.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>
#include <utility>
#include <algorithm>
#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>

namespace kjb {
namespace bbb {

Activity_library::Activity_library(const std::string& lib_dp)
{
    std::string fpath;
    std::ifstream ifs;
    S_vec strs;
    V_vec vecs;

    //// activities
    // intentional
    fpath = lib_dp + "/iacts.txt";
    ifs.open(fpath.c_str());
    iacts_.assign(
        std::istream_iterator<std::string>(ifs),
        std::istream_iterator<std::string>());
    ifs.close();

    // physical
    fpath = lib_dp + "/pacts.txt";
    ifs.open(fpath.c_str());
    pacts_.assign(
        std::istream_iterator<std::string>(ifs),
        std::istream_iterator<std::string>());
    ifs.close();

    // all
    activities_.resize(iacts_.size() + pacts_.size());
    std::copy(iacts_.begin(), iacts_.end(), activities_.begin());
    std::copy(pacts_.rbegin(), pacts_.rend(), activities_.rbegin());

    //// roles
    fpath = lib_dp + "/roles.txt";
    ifs.open(fpath.c_str());
    roles_.assign(
        std::istream_iterator<std::string>(ifs),
        std::istream_iterator<std::string>());
    ifs.close();

    //// CRP params
    fpath = lib_dp + "/crp_params.txt";
    boost::tie(strs, vecs) = read_strs_and_vecs(fpath);
    for(size_t i = 0; i < strs.size(); ++i)
    {
        crp_params_[strs[i]] = vecs[i][0];
    }

    //// role distributions
    fpath = lib_dp + "/role_dists.txt";
    boost::tie(strs, vecs) = read_strs_and_vecs(fpath);
    for(size_t i = 0; i < strs.size(); ++i)
    {
        role_dists_[strs[i]] = vecs[i];
    }

    //// kernel parameters
    fpath = lib_dp + "/kernel_params.txt";
    boost::tie(strs, vecs) = read_strs_and_vecs(fpath);
    for(size_t i = 0; i < strs.size(); ++i)
    {
        kernel_params_[strs[i]] = std::make_pair(vecs[i][0], vecs[i][1]);
    }

    //// markov chains
    for(size_t i = 0; i < roles_.size(); ++i)
    {
        fpath = lib_dp + "/mc_" + roles_[i] + ".txt";
        Matrix M(fpath);

        Vector x = M.get_row(0);
        Matrix K = M.submatrix(1, 0, M.get_num_rows() - 1, M.get_num_cols());
        role_mcs_[roles_[i]] = std::make_pair(x, K);
    }

    Vector x((int)activities_.size(), 0.0);
    Matrix K(activities_.size(), activities_.size(), 0.0);

    //// I has target?
    fpath = lib_dp + "/act_target.txt";
    ifs.open(fpath.c_str());
    act_target_.clear();
    act_target_.insert(
        std::istream_iterator<std::string>(ifs),
        std::istream_iterator<std::string>());
    ifs.close();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector Activity_library::sample_parameters
(
    const std::string& name,
    const std::string& pt_name,
    const Vector& pt_params
) const
{
    typedef MV_normal_distribution Mvn;
    const double space_sd = 30.0;
    const double noise_sd = 0.5;

    if(name == "FFA")
    {
        return Vector();
    }
    else if(name == "MEET")
    {
        const int d = 2;
        Mvn P(Vector(d, 0.0), Vector(d, space_sd*space_sd));
        return kjb::sample(P);
    }
    else if(name == "MOVE-TO")
    {
        if(pt_name == "MEET")
        {
            const int d = pt_params.get_length();
            Mvn P(pt_params, Vector(d, noise_sd*noise_sd));
            return kjb::sample(P);
        }
        else
        {
            const int d = 2;
            Mvn P(Vector(d, 0.0), Vector(d, space_sd*space_sd));
            return kjb::sample(P);
        }
    }

    //else
    KJB_THROW_2(Runtime_error, "Sample params: activity not recognized");
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::pair<Activity_library::S_vec, Activity_library::V_vec>
    Activity_library::read_strs_and_vecs
(
    const std::string& path
)
{
    S_vec strs;
    V_vec vecs;

    std::ifstream ifs(path.c_str());
    std::string line;
    while(std::getline(ifs, line))
    {
        std::istringstream iss(line);

        std::string str;
        iss >> str;

        std::vector<double> vec;
        double num;
        while(iss >> num)
        {
            vec.push_back(num);
        }

        strs.push_back(str);
        vecs.push_back(vec);
    }

    ifs.close();

    return std::make_pair(strs, vecs);
}

}} // namespace kjb::bbb

