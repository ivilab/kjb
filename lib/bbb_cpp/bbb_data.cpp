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

#include "bbb_cpp/bbb_data.h"
#include "bbb_cpp/bbb_trajectory.h"
#include "m_cpp/m_vector.h"
#include "l_cpp/l_exception.h"

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <utility>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>

using namespace kjb;
using namespace kjb::bbb;

void kjb::bbb::read(Data& data, const std::string& fname)
{
    typedef std::multimap<size_t, std::pair<size_t, Vector> > Line_map;

    std::ifstream ifs(fname.c_str());
    IFT(ifs, Runtime_error, "Cannot read data: cannot open file");

    // skip header
    std::string line;
    std::getline(ifs, line);

    // read trajectory lines
    Line_map line_map;
    size_t num_trajs = 0;
    while(std::getline(ifs, line))
    {
        using namespace boost;

        std::vector<std::string> toks;
        split(toks, line, is_any_of(" ,"), token_compress_on);

        //IFTD(toks.size() == 5, Runtime_error,
        IFTD(toks.size() == 4, Runtime_error,
            "Cannot parse data file; wrong format on line %d",
            (line_map.size() + 2));

        size_t id = lexical_cast<size_t>(toks[0]);
        size_t f = lexical_cast<size_t>(toks[1]);
        double x = lexical_cast<double>(toks[2]);
        double z = lexical_cast<double>(toks[3]);
        //std::string label = toks[4];

        //if(strcasecmp(label.c_str(), "person") != 0) continue;

        if(line_map.count(id) == 0) num_trajs++;
        line_map.insert(std::make_pair(id, std::make_pair(f, Vector(x, z))));
    }

    if(line_map.empty()) return;

    // build trajectories
    std::vector<Trajectory> trajs;
    std::vector<size_t> ids;
    trajs.reserve(num_trajs);
    ids.reserve(num_trajs);
    Line_map::const_iterator pr_p = line_map.begin();
    while(pr_p != line_map.end())
    {
        size_t id = pr_p->first;

        // first count them and find start time
        size_t sz = 0;
        size_t st = std::numeric_limits<size_t>::max();
        Line_map::const_iterator pr_q = pr_p;
        for(Line_map::const_iterator pr_q = pr_p;
            pr_q != line_map.end() && pr_q->first == id;
            ++pr_q)
        {
            ++sz;
            if(pr_q->second.first < st)
            {
                st = pr_q->second.first;
            }
        }
        
        // fill in trajectory
        std::vector<Vector> poss(sz);
        while(pr_p != line_map.end() && pr_p->first == id)
        {
            const size_t f = pr_p->second.first;
            const Vector& x = pr_p->second.second;
            poss[f - st] = x;
            ++pr_p;
        }

        //trajs[id - 1].set_positions(st, poss.begin(), poss.end());
        trajs.push_back(Trajectory());
        trajs.back().set_positions(st, poss.begin(), poss.end());
        ids.push_back(id);
    }

    // fill in data
    data.set(trajs.begin(), trajs.end(), ids.begin());
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::bbb::write(const Data& data, const std::string& fname)
{
    std::ofstream ofs(fname.c_str());
    IFT(ofs, Runtime_error, "Cannot write data: cannot open file");

    // write header
    ofs << "TrackId,"
        << "Frame,"
        << "PosX,"
        << "PosZ,"
        << "Label" << std::endl;

    // write trajectories
    const size_t num_trajs = data.size();
    for(size_t i = 0; i < num_trajs; i++)
    {
        const Trajectory& traj = data.trajectory(i);
        const size_t id = data.id(i);

        const size_t s = traj.start();
        const size_t e = traj.end();
        for(size_t f = s; f <= e; f++)
        {
            Vector x = traj.pos(f);
            ofs << id << ","
                << f << ","
                << x[0] << ","
                << x[1] << ","
                << "Person" << std::endl;
        }
    }

    ofs.close();
}

