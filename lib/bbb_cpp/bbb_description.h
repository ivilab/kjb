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

#ifndef B3_DESCRIPTION_H
#define B3_DESCRIPTION_H

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "bbb_cpp/bbb_activity_sequence.h"
#include "bbb_cpp/bbb_intentional_activity.h"
#include "bbb_cpp/bbb_physical_activity.h"
#include "bbb_cpp/bbb_activity_library.h"
#include "bbb_cpp/bbb_traj_set.h"
#include "bbb_cpp/bbb_data.h"
#include "m_cpp/m_vector.h"
#include "l_cpp/l_int_matrix.h"
#include "l_cpp/l_int_vector.h"
#include "l_cpp/l_functors.h"
#include "l_cpp/l_word_list.h"
#include "l/l_sys_io.h"

#include <map>
#include <set>
#include <vector>
#include <utility>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/variant.hpp>
#include <boost/ref.hpp>
#include <boost/tuple/tuple.hpp>

namespace kjb {
namespace bbb {

// forward declaration
class Likelihood;

/**
 * @class   Description
 *
 * Class represents the description of a video.
 */
class Description
{
public:
    typedef std::multimap<const Intentional_activity*, Activity_sequence>
            Act_tree;

public:
    /** @brief  Create a descrpition with the given end times. */
    Description(size_t start, size_t end, const Traj_set& trajs) :
        root_("FFA", start, end, Vector(), trajs)
    {
        IFT(trajs.size() != 0, Illegal_argument,
            "Cannot create description; empty trajectory set");
        IFT(*trajs.rbegin() + 1 == trajs.size(), Illegal_argument,
            "Cannot create description; trajectories indexed incorrectly");
    }

    /** @brief  Create a descrpition which is forced to be of the given kind. */
    Description(const Intentional_activity& root) : root_(root)
    {
        const Traj_set& trajs = root.trajectories();
        IFT(trajs.size() != 0, Illegal_argument,
            "Cannot create description; empty trajectory set");
        IFT(*trajs.rbegin() + 1 == trajs.size(), Illegal_argument,
            "Cannot create description; trajectories indexed incorrectly");
    }

    /** @brief  Copy constructor: deep copy. */
    Description(const Description& desc) : root_(desc.root_)
    {
        copy(desc, root_, desc.root_);
    }

    /** @brief  Assignment: deep copy. */
    Description& operator=(const Description& desc)
    {
        if(&desc != this)
        {
            tree_.clear();
            root_ = desc.root_;
            copy(desc, root_, desc.root_);
        }

        return *this;
    }

    /** @brief  Resets this description and sets the given IA as root. */
    void clear(const Intentional_activity& root)
    {
        root_ = root;
        tree_.clear();
    }

    /** @brief  Add an activity sequence as a child of an IA. */
    const Activity_sequence* add_child_sequence
    (
        const Intentional_activity* act,
        const Activity_sequence& aseq
    )
    {
        // this check is expensive; once we have more confidence in this code
        // we should take it off
        ASSERT(contains(*act));

        return &(tree_.insert(std::make_pair(act, aseq))->second);
    }

    /** @brief  Get the start time of this description. */
    size_t start() const { return root_.start(); }

    /** @brief  Get the end time of this description. */
    size_t end() const { return root_.end(); }

    /** @brief  Get the root activity of this description. */
    const Intentional_activity& root_activity() const { return root_; }

    /** @brief  Get the the children of a given IA node. */
    std::pair<Act_tree::const_iterator, Act_tree::const_iterator> children
    (
        const Intentional_activity& act
    ) const
    {
        return tree_.equal_range(&act);
    }

    /** @brief  Get all the phsical activities of this description. */
    template<class Iter>
    void physical_activities(Iter out, const std::string& name = "") const;

    /** @brief  Get all the phsical activities of this description. */
    template<class Iter>
    void intentional_activities(Iter out, const std::string& name = "") const;

    /** @brief  Get the ancestors of the given activity. */
    template<class A, class OutIt>
    void ancestors(const A& act, OutIt result, int max_depth = -1) const;

    /** @brief  Return true if the given activity is the root. */
    bool is_root(const Physical_activity&) const { return false; }

    /** @brief  Return true if the given activity is the root. */
    bool is_root(const Intentional_activity& act) const
    {
        return &act == &root_;
    }

    /**
     * @brief   Checks whether the given activity is contained (by address) in
     *          this description.
     */
    template<class A>
    bool contains(const A& act_p) const;

private:
    friend std::ostream& operator<<(std::ostream&, const Description&);
    friend Data sample(const Likelihood& likelihood, const Description& desc);

    /** @brief  Deep-copy a description onto this one. */
    void copy
    (
        const Description& desc,
        const Intentional_activity& local_act,
        const Intentional_activity& desc_act
    );

    Intentional_activity root_;
    Act_tree tree_;
};

/** @brief  Push an description to an output stream. */
std::ostream& operator<<(std::ostream& ost, const Description& desc);

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Iter>
void Description::physical_activities(Iter out, const std::string& name) const
{
    BOOST_FOREACH(const Act_tree::value_type& pr, tree_)
    {
        const Activity_sequence& aseq = pr.second;
        for(size_t j = 0; j < aseq.size(); ++j)
        {
            const Activity_sequence::Activity& act = aseq.activity(j);
            const Physical_activity* pa_p = boost::get<Physical_activity>(&act);
            if(pa_p)
            {
                if(name == "" || name == pa_p->name())
                {
                    *out++ = pa_p;
                }
            }
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Iter>
void Description::intentional_activities(Iter out, const std::string& name) const
{
    *out++ = &root_;
    BOOST_FOREACH(const Act_tree::value_type& pr, tree_)
    {
        if(name == "" || name == pr.first->name())
        {
            *out++ = pr.first;
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class A, class OutIt>
void Description::ancestors(const A& act, OutIt result, int max_depth) const
{
    using namespace boost;

    typedef Activity_sequence::const_iterator Asit;
    typedef Intentional_activity Ia;

    if(is_root(act) || max_depth == 0) { return; }

    const Ia* parent_p = 0;
    BOOST_FOREACH(const Act_tree::value_type& pr, tree_)
    {
        const Activity_sequence& aseq = pr.second;
        Asit act_p = std::find_if(
                            aseq.begin(),
                            aseq.end(),
                            bind(same_activity<A>, _1, cref(act)));

        if(act_p != aseq.end())
        {
            parent_p = pr.first;
            break;
        }
    }

    IFT(parent_p, Illegal_argument, "Cannot get ancestors: activity not found");

    *result++ = parent_p;
    if(max_depth > 0) --max_depth;
    ancestors(*parent_p, result, max_depth);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class A>
bool Description::contains(const A& act) const
{
    using namespace boost;

    if(is_root(act)) return true;
    BOOST_FOREACH(const Act_tree::value_type& pr, tree_)
    {
        const Activity_sequence& aseq = pr.second;
        size_t c = std::count_if(
                            aseq.begin(),
                            aseq.end(),
                            bind(same_activity<A>, _1, cref(act)));

        if(c != 0) return true;
    }

    return false;
}

/** @brief  Extract sparse information from description (for write()). */
class Description_info
{
public:
    struct Activity_info
    {
        size_t id;
        std::string name;
        size_t start;
        size_t end;
        size_t parent;
        size_t role;
        Traj_set trajs;

        bool operator<(const Activity_info& ai) const { return name < ai.name; }

        //bool operator==(const Activity_info& ai) const
        //{
        //    return id == ai.id &&
        //            name == ai.name &&
        //            start == ai.start &&
        //            end == ai.end &&
        //            parent == ai.parent &&
        //            std::equal(trajs.begin(), trajs.end(), ai.trajs.begin());
        //}
    };

public:
    void fill(const Description& desc, const Activity_library& lib)
    {
        index = 0;
        info_set.clear();

        const Intentional_activity rt = desc.root_activity();
        Activity_info ai = {
            0,
            rt.name(),
            rt.start(),
            rt.end(),
            0,
            0,
            rt.trajectories()
        };
        info_set.insert(ai);

        fill_tree(desc.root_activity(), index, desc, lib);
    }

    void extract(Description& desc, const Activity_library& lib) const
    {
        typedef std::multiset<Activity_info>::const_iterator Iter;

        // find root
        Iter ai_p = std::find_if(
                            info_set.begin(),
                            info_set.end(),
                            boost::bind(&Description_info::is_root, this, _1));
        IFT(ai_p != info_set.end(), Runtime_error,
            "Cannot read description; root group must have ID = 0");

        // create root
        desc.clear(Intentional_activity(
                                ai_p->name,
                                ai_p->start,
                                ai_p->end,
                                Vector(),
                                ai_p->trajs));

        // fill other activities
        extract_tree(desc.root_activity(), 0, desc, lib);
    }

private:
    void fill_tree
    (
        const Intentional_activity& act,
        size_t id,
        const Description& desc,
        const Activity_library& lib
    );

    void extract_tree
    (
        const Intentional_activity& act,
        size_t id,
        Description& desc,
        const Activity_library& lib
    ) const;

    template<class AseqIt, class VecIt>
    void get_child_sequences
    (
        size_t id,
        AseqIt seq_out,
        VecIt ids_out,
        const Activity_library& lib
    ) const;

    bool is_root(const Activity_info& ai) const { return ai.id == 0; }

    bool same_traj_set(const Traj_set& ts1, const Traj_set& ts2) const
    {
        return std::equal(ts1.begin(), ts1.end(), ts2.begin());
    }

public:
    std::multiset<Activity_info> info_set;
    size_t index;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class AseqIt, class VecIt>
void Description_info::get_child_sequences
(
    size_t id,
    AseqIt seq_out,
    VecIt ids_out,
    const Activity_library& lib
) const
{
    typedef std::map<size_t, const Activity_info*> Time_info_map;
    std::vector<Time_info_map> ti_maps;
    std::vector<Traj_set> traj_sets;

    BOOST_FOREACH(const Activity_info& ai, info_set)
    {
        if(ai.id != id && ai.parent == id)
        {
            std::vector<Traj_set>::iterator set_p;
            set_p = std::find_if(
                        traj_sets.begin(),
                        traj_sets.end(),
                        boost::bind(
                            &Description_info::same_traj_set,
                            this,
                            boost::cref(ai.trajs),
                            _1));

            size_t seq_idx;
            if(set_p == traj_sets.end())
            {
                traj_sets.push_back(ai.trajs);
                ti_maps.push_back(Time_info_map());
                seq_idx = traj_sets.size() - 1;
            }
            else
            {
                seq_idx = std::distance(traj_sets.begin(), set_p);
            }

            ti_maps[seq_idx][ai.start] = &ai;
        }
    }

    for(size_t j = 0; j < ti_maps.size(); ++j)
    {
        std::vector<size_t> ids;
        Activity_sequence aseq(lib.role_name(ti_maps[j].begin()->second->role));
        BOOST_FOREACH(const Time_info_map::value_type& pr, ti_maps[j])
        {
            if(lib.is_physical(pr.second->name))
            {
                size_t s = pr.second->start;
                size_t e = pr.second->end;
                Trajectory trajectory;
                std::vector<Vector> zeros(2, Vector((int)(e - s + 1), 0.0));
                trajectory.set_dimensions(s, zeros.begin(), zeros.end());
                aseq.add(Physical_activity(
                                    pr.second->name,
                                    trajectory,
                                    pr.second->trajs));
            }
            else if(lib.is_intentional(pr.second->name))
            {
                aseq.add(Intentional_activity(
                                    pr.second->name,
                                    pr.second->start,
                                    pr.second->end,
                                    //5*create_random_vector(2),
                                    Vector(1.0, 1.0),
                                    pr.second->trajs));
            }
            else
            {
                ASSERT(false);
            }

            ids.push_back(pr.second->id);
        }

        *seq_out++ = aseq;
        *ids_out++ = ids;
    }
}

/**
 * @brief   Write description to files.
 *
 * @param   desc    Description to write.
 * @param   path    Path to output directory.
 * @param   lib     Activity library.
 * @param   first   First element in range of trajectory labels.
 * @param   last    One-past-the-end element in range of trajectory labels.
 */
template<class InIt>
void write
(
    const Description& desc,
    const std::string& path,
    const Activity_library& lib,
    InIt first,
    InIt last
)
{
    typedef Description_info::Activity_info Act_info;

    const size_t ntrajs = desc.root_activity().trajectories().size();
    const size_t nframes = desc.end() + 1;

    // check for all kinds of errors
    IFT(ntrajs != 0, Illegal_argument,
        "Cannot write description: contains no trajectories");
    IFT(*desc.root_activity().trajectories().rbegin() == ntrajs - 1,
        Illegal_argument, "Cannot write description: bad trajectory indices.");
    IFT(std::distance(first, last) == ntrajs, Illegal_argument,
        "Cannot write description: bad trajectory indices.");
    IFT(kjb_c::is_directory(path.c_str()), Illegal_argument,
        "Cannot write description: invalid path.");

    // too much std:: in this function
    using namespace std;

    // parse tree
    Description_info dinfo;
    dinfo.fill(desc, lib);

    // write files
    Int_matrix empty_mat(nframes, ntrajs, -1);
    vector<size_t> parents(dinfo.info_set.size());
    vector<size_t> roles(dinfo.info_set.size());

    string cur_name = dinfo.info_set.begin()->name;
    multiset<Act_info>::const_iterator ai_p = dinfo.info_set.begin();
    while(ai_p != dinfo.info_set.end())
    {
        Int_matrix cur_mat = empty_mat;;
        while(ai_p != dinfo.info_set.end() && ai_p->name == cur_name)
        {
            // save activity
            BOOST_FOREACH(size_t j, ai_p->trajs)
            {
                for(size_t t = ai_p->start; t <= ai_p->end; ++t)
                {
                    cur_mat(t, j) = ai_p->id;
                }
            }

            parents[ai_p->id] = ai_p->parent;
            roles[ai_p->id] = ai_p->role;
            ++ai_p;
        }

        // write to file
        ofstream ofs((path + "/" + cur_name + ".txt").c_str());
        IFT(ofs, IO_error, "Cannot write description: error creating file.");

        // write first line
        ofs << -1 << ",";
        InIt notlast = last;
        copy(first, --notlast, ostream_iterator<size_t>(ofs, ","));
        ofs << *notlast << endl;

        // write other lines
        for(size_t t = 0; t < nframes; ++t)
        {
            ofs << t << ",";
            Int_vector r = cur_mat.get_row(t);
            copy(r.begin(), r.end() - 1, ostream_iterator<int>(ofs, ","));
            ofs << *r.rbegin() << endl;
        }

        ofs.close();

        if(ai_p != dinfo.info_set.end()) cur_name = ai_p->name;
    }

    ofstream ofs((path + "/parents.txt").c_str());
    IFT(ofs, IO_error, "Cannot write description: error creating file.");
    copy(parents.begin(), parents.end()-1, ostream_iterator<size_t>(ofs, ","));
    ofs << *parents.rbegin() << endl;

    copy(roles.begin(), roles.end()-1, ostream_iterator<size_t>(ofs, ","));
    ofs << *roles.rbegin() << endl;
    ofs.close();
}

/** @brief  Find group in activity matrix. Utility function for read(). */
template<class InIt>
Description_info::Activity_info find_group
(
    Int_matrix& act_mat,
    size_t tr,
    size_t jc,
    InIt first,
    InIt last
)
{
    Description_info::Activity_info ai;

    // first row and column of matrix are not part of the group info
    IFT(tr != 0 && jc != 0, Illegal_argument,
        "Cannot find group; bad row or col.");

    // group of interest
    ai.id = act_mat(tr, jc);

    // find initial and final frames
    size_t r = tr;
    while(r != 0 && act_mat(r, jc) == ai.id) --r;
    size_t sr = r + 1;

    r = sr;
    while(r != act_mat.get_num_rows() && act_mat(r, jc) == ai.id) ++r;
    size_t er = r - 1;

    // find individuals participating
    for(size_t i = 1; i < act_mat.get_num_cols(); ++i)
    {
        if(act_mat(sr, i) == ai.id)
        {
            // find index
            InIt sz_p = std::find(first, last, act_mat(0, i));
            IFT(sz_p != last, Runtime_error, "Cannot read activity; bad IDs.");
            ai.trajs.insert(*sz_p);

            // clear column area
            for(size_t k = sr; k <= er; ++k)
            {
                act_mat(k, i) = -1;
            }
        }
    }

    ai.start = act_mat(sr, 0);
    ai.end = act_mat(er, 0);

    return ai;
}

/**
 * @brief   Read description from files.
 *
 * @param   desc    Target description to be read.
 * @param   path    Path to input directory.
 * @param   lib     Activity library.
 * @param   first   First element in range of trajectory labels.
 * @param   last    One-past-the-end element in range of trajectory labels.
 */
template<class InIt>
void read
(
    Description& desc,
    const std::string& path,
    const Activity_library& lib,
    InIt first,
    InIt last
)
{
    typedef Description_info::Activity_info Ainfo;
    using boost::tie;
    using std::vector;

    IFT(kjb_c::is_directory(path.c_str()), Illegal_argument,
        "Cannot read description: invalid path.");

    //// first read files and description info
    Description_info dinfo;

    // read parents file
    IFT(kjb_c::is_file((path + "/parents.txt").c_str()), IO_error,
        "Cannot read description; cannot read parents file.");
    Int_matrix parents(path + "/parents.txt");

    // go through each activity and look at corresponding file
    const size_t nacts = lib.num_activities();
    for(size_t i = 0; i < nacts; ++i)
    {
        const std::string& name = lib.activity_name(i);
        //std::string act_fp = path + "/" + name + ".txt";
        //if(!kjb_c::is_file(act_fp.c_str())) continue;

        std::string act_fpg = path + "/" + name + "*.txt";
        Word_list wl(act_fpg.c_str());
        if(wl.size() == 0) continue;

        for(size_t f = 0; f < wl.size(); ++f)
        {
            // parse file and find different groups;
            // we shall first put activities in vectors then worry about tree
            std::string act_fp = wl[f];
            Int_matrix cur_act(act_fp);
            for(size_t t = 1; t < cur_act.get_num_rows(); ++t)
            {
                for(size_t j = 1; j < cur_act.get_num_cols(); ++j)
                {
                    if(cur_act(t, j) != -1)
                    {
                        Ainfo ai = find_group(cur_act, t, j, first, last);
                        ai.name = name;
                        ai.parent = parents(0, ai.id);
                        ai.role = parents(1, ai.id);
                        dinfo.info_set.insert(ai);
                    }
                }
            }
        }
    }

    // fill description
    dinfo.extract(desc, lib);
}

/**
 * @brief   Create a trivial description from data, where everybody is
 *          simply walking independently.
 */
Description make_trivial_description(const Data& data);

}} // namespace kjb::bbb

#endif /*B3_DESCRIPTION_H */

